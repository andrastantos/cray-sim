// -o test.img -s 781000s -cp E:\personal\cray-xmp\from_marcel\cray_j90\UNICOS_exe\uni_12\bin\rcp  rcp
// -o transfer.img -minor 60 -s 781000s -f root -p root -cp E:\personal\cray-xmp\from_marcel\cray_j90\UNICOS_exe\rbf_11.cpio rbf_11.cpio -co E:\personal\cray-xmp\from_marcel\cray_j90\UNICOS_exe\uex_11.cpio uex_11.cpio - cp E : \personal\cray - xmp\from_marcel\cray_j90\UNICOS_exe\uni_12.cpio uni_12.cpio - cp E : \personal\cray - xmp\from_marcel\cray_j90\UNICOS_exe\uni_22.cpio uni_22.cpio
#define _CRT_SECURE_NO_WARNINGS
#include <fstream>
#include <vector>
#include <stdint.h>
#include <string>
#include "../sim_lib/utils.h"
#include <boost/filesystem.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/tokenizer.hpp>
#include <map>

/////////////////////////////////////////////////////////////////////////////////
/*
NOTES:
- The FS seems to contain several copies of the superblock, starting at block 1
- 



- The FS seems to be using INOMAGIC4 as the INode magic
- That being said, it doesn't seem to care about the magic and happily accepts anything
- INodes always occupy a full block
- For small files, the data can be stored in the INode block as well
  This is signaled by all in_addr fileds being zero
- For files that don't fit in the INode block, the INode block is not used
  for data storage. Consequently, when a file grows outside of the INode block,
  the contents is copied over to a separate block, which is linked in in_addrp[0]
- Block 0 is reserved and not used
- Block 1 is the superblock
- Block 2 is the root INode
- The allocation map (FREEMAP) can be anywhere on the disk, but all previous
  blocks must be marked full in it. This means that the best place to put
  the map is from Block 3 onward
- Blocks are marked by 1 as occupied and as by 0 as free in the FREEMAP
- MSB corresponds to the lowest index block, otherwise storage in map is consequtive
- in_addr entires even though marked as daddr_t, are actually a struct of 2 32-bit values:
  - upper 32 bit is the starting block for the allocation
  - lower 32 bit is the length of the allocation in blocks
- Major and minor device numbers are stored in in_addr[0]. Lower 8 bits: minor, next 8 bits: major
- It seems that up to 10 allocation blocks are used directly
- Single-redirects are marked by the StartBlock being 0, and BlockCnt point to the single-indirected allocation block.
UNKNWONS:
- double- and tripple- re-directs for block allocations

A file (/etc/MKDSKDEV) gets created with the following content:
umask 077
/etc/mknod expander b 2 0
/etc/mknod dump b 0 1
/etc/mknod root b 0 2
/etc/mknod usr b 0 3
- We probably should create these devices once we know how
*/
/////////////////////////////////////////////////////////////////////////////////

#pragma pack(push,1)

const size_t cBlockSize = 512; // in QWORDs
const size_t BSIZE = cBlockSize * 8;

const size_t SUPERB = 1; // physical block number of the super block


template <size_t tOfs, typename tFieldType, size_t tStart = 0, size_t tSize = std::numeric_limits<tFieldType>::digits> class IntField_T {
public:
	explicit IntField_T(uint64_t *aBase) : mPtr(aBase + tOfs) {}
	void SetBase(uint64_t *aBase) { mPtr = aBase + tOfs; }
	operator tFieldType() {
		uint64_t Val = SwapBytes(*mPtr);
		return tFieldType(GetBitsReverse(Val, tStart, tStart + tSize - 1));
//		return tFieldType(GetBits(*mPtr, tStart, tStart + tSize - 1));
	}
	IntField_T &operator = (tFieldType aValue) {
		uint64_t Val = SwapBytes(*mPtr);
		Val = SetBitsReverse(Val, tStart, tStart + tSize - 1, uint64_t(aValue));
		*mPtr = SwapBytes(Val);
//		*mPtr = SetBits(*mPtr, tStart, tStart + tSize - 1, uint64_t(aValue));
		return *this;
	}
protected:
	uint64_t *mPtr;
};

template <size_t tOfs> class StrField_T {
public:
	explicit StrField_T(uint64_t *aBase) : mPtr(aBase + tOfs) {}
	void SetBase(uint64_t *aBase) { mPtr = aBase + tOfs; }
	operator std::string() {
		std::string RetVal;
		RetVal.reserve(8);
		char *RawStr = (char*)(mPtr);
		for (int i = 0; i < sizeof(int64_t); ++i) {
			if (RawStr[i] == 0) break;
			RetVal.push_back(RawStr[i]);
		}
		return RetVal;
	}
	StrField_T &operator = (const std::string &aValue) {
		size_t Length = aValue.length();
		CRAY_ASSERT(Length <= sizeof(int64_t));
		*mPtr = 0;
		memcpy(mPtr, &(aValue[0]), Length);
//		for (int i = 0; i < Length; ++i) {
//			((char*)(mPtr))[sizeof(int64_t) - i - 1] = aValue[i];
//		}
		return *this;
	}
	StrField_T &operator = (const char *aValue) {
		size_t Length = strlen(aValue);
		CRAY_ASSERT(Length <= sizeof(int64_t));
		*mPtr = 0;
		memcpy(mPtr, aValue, Length);
//		for (int i = 0; i < Length; ++i) {
//			((char*)(mPtr))[sizeof(int64_t) - i - 1] = aValue[i];
//		}
		return *this;
	}
protected:
	uint64_t *mPtr;
};

template <size_t tOfs, typename tElement, size_t tSize> class ArrField_T {
public:
	explicit ArrField_T(uint64_t *aBase) : mPtr(aBase + tOfs) {}
	void SetBase(uint64_t *aBase) { mPtr = aBase + tOfs; }
	tElement operator[](size_t aOfs) {
		CRAY_ASSERT(aOfs < tSize);
		return tElement(mPtr + aOfs);
	}
	tElement operator[](int aOfs) {
		CRAY_ASSERT(aOfs >= 0);
		return operator[](size_t(aOfs));
	}
	static constexpr size_t size = tSize;
protected:
	uint64_t *mPtr;
};
	
template <size_t tOfs, typename tElement> class SubField_T {
public:
	explicit SubField_T(uint64_t *aBase) : mPtr(aBase + tOfs) {}
	void SetBase(uint64_t *aBase) { mPtr = aBase + tOfs; }
	tElement get() {
		return tElement(mPtr);
	}
protected:
	uint64_t *mPtr;
};

struct extent_t {
	explicit extent_t(uint64_t *aStore): nblks(aStore), blk(aStore) {}

	IntField_T<0, uint32_t, 0>             nblks;  // Number of blocks
	IntField_T<0, uint32_t, 32>            blk;    // Start block number
};

const size_t NC1MAXIREG = 4; // Maximum inode regions per partition

struct nc1fdev_sb {
	explicit nc1fdev_sb(uint64_t *aStore): fd_name(aStore), fd_sblk(aStore), fd_nblk(aStore), fd_ireg(aStore) {}
	StrField_T<0>                       fd_name;    // Physical device name
	IntField_T<1, uint32_t,  0>         fd_sblk;    // Start block number
	IntField_T<1, uint32_t, 32>         fd_nblk;    // Number of blocks
	ArrField_T<2, extent_t, NC1MAXIREG> fd_ireg;    // Inode regions
};

typedef uint64_t CTime_t;
typedef uint64_t CIno_t;
typedef uint64_t CBlkNo_t;

const size_t NC1MAXPART = 64; // Maximum number of partitions

const size_t NC1NSUPER = 10;  // Copies of s.b. per partition

const uint64_t FsMAGIC_NC1 = 0x6e6331667331636eULL; // s_magic number
const uint64_t FsSECURE = 0xcd076d1771d670cdULL;    // s_secure: secure file system
const uint64_t DbMAGIC_NC1 = 0x6e6331646231636e;    // db_magic number

enum FsFlags_e {
	Fs_PANIC      = 0000000001, // not used
	Fs_RRFILE     = 0000000002, // Round robin file allocation
	Fs_RRALLDIR   = 0000000004, // Round robin all directories
	Fs_RR1STDIR   = 0000000010, // Round robin 1st level directories
	Fs_UPDATE     = 0000001000, // File system update in progress
	Fs_WUPDAT     = 0000002000, // File system wakeup after update
	Fs_RRALLUDATA = 0000020000, // Round robin all user file data
	Fs_NOIPREF    = 0000040000, // Inode alloc. preference disabled
	Fs_PANICLESS  = 0000100000, // Attempt to continue on error
	Fs_SCRUB      = 0000200000, // Enable/Disable filesystem scrub
	Fs_SFS        = 0010000000, // Shared File system
	Fs_TESTCOND1  = 0100000000, // Test condition #1
	Fs_TESTCOND2  = 0200000000, // Test condition #2
	Fs_TESTCOND3  = 0400000000  // Test condition #3
};

enum DbFlags_e {
	Db_CHECKED = 0000000040, // Flag used in the Dynamic Blk
	Db_MOUNTED = 0000000100, // Flag used in the Dynamic Blk
};

struct nc1filsys {
	nc1filsys(uint64_t *aStore) : 
		s_magic(aStore),
		s_fname(aStore),
		s_fpack(aStore),
		s_dev(aStore),

		s_fsize(aStore),
		s_isize(aStore),
		s_bigfile(aStore),
		s_bigunit(aStore),

		s_secure(aStore),
		s_maxlvl(aStore),
		s_minlvl(aStore),
		s_valcmp(aStore),

		s_time(aStore),
		s_dboff(aStore),
		s_root(aStore),
		s_error(aStore),

		s_mapoff(aStore),
		s_mapblks(aStore),
		s_nscpys(aStore),
		s_npart(aStore),

		s_ifract(aStore),
		s_sfs(aStore),
		s_flag(aStore),
		s_part(aStore),
		s_iounit(aStore),
		s_numiresblks(aStore),
		s_priparts(aStore),
		s_priblock(aStore),
		s_prinblks(aStore),
		s_secparts(aStore),
		s_secblock(aStore),
		s_secnblks(aStore),
		s_sbdbparts(aStore),
		s_rootdparts(aStore),
		s_nudparts(aStore),
		s_nsema(aStore),
		s_priactive(aStore),
		s_sfs_arbiterid(aStore)
	{}
	IntField_T< 0, uint64_t>                s_magic;             // magic number to indicate file system type
	StrField_T< 1>                          s_fname;             // file system name
	StrField_T< 2>                          s_fpack;             // file system pack name
	IntField_T< 3, uint64_t>                s_dev;               // major/minor device, for verification
							                     
	IntField_T< 4, CBlkNo_t>                s_fsize;             // size in blocks of entire volume
	IntField_T< 5, uint64_t>                s_isize;             // Number of total inodes
	IntField_T< 6, uint64_t>                s_bigfile;           // number of bytes at which a file is big
	IntField_T< 7, CBlkNo_t>                s_bigunit;           // minimum number of blocks allocated for big files
								            
	IntField_T< 8, uint64_t>                s_secure;            // security: secure FS label
	IntField_T< 9, uint64_t>                s_maxlvl;            // security: maximum security level
	IntField_T<10, uint64_t>                s_minlvl;            // security: minimum security level
	IntField_T<11, uint64_t>                s_valcmp;            // security: valid security compartments
								            
	IntField_T<12, CTime_t>                 s_time;              // last super block update 
	IntField_T<13, CBlkNo_t>                s_dboff;             // Dynamic block number
	IntField_T<14, CIno_t>                  s_root;              // root inode
	IntField_T<15, uint64_t>                s_error;             // Type of file system error detected
								            
	IntField_T<16, CBlkNo_t>                s_mapoff;            // Start map block number
	IntField_T<17, CBlkNo_t>                s_mapblks;           // Last map block number
	IntField_T<18, uint64_t>                s_nscpys;            // Number of copies of s.b per partition
	IntField_T<19, uint64_t>                s_npart;             // Number of partitions
								            
	IntField_T<20, uint64_t>                s_ifract;            // Ratio of inodes to blocks
	SubField_T<21, extent_t>                s_sfs;               // SFS only blocks
	IntField_T<22, uint64_t>                s_flag;              // Flag word
	ArrField_T<23, nc1fdev_sb, NC1MAXPART>  s_part;              // Partition descriptors
	static constexpr size_t s_part_end = 23 + 6 * NC1MAXPART;
	IntField_T<s_part_end +  0, uint64_t>   s_iounit;            // Physical block size
	IntField_T<s_part_end +  1, uint64_t>   s_numiresblks;       // number of inode reservation blocks per region (currently 1) ... 0 = 1*(AU) words, n = (n+1)*(AU) words
	IntField_T<s_part_end +  2, uint64_t>   s_priparts;          // bitmap of primary partitions
	IntField_T<s_part_end +  3, uint64_t>   s_priblock;          // block size of primary partition(s) ... 0 = 1*512 words, n = (n+1)*512 words
	IntField_T<s_part_end +  4, uint64_t>   s_prinblks;          // number of 512 wds blocks in primary
	IntField_T<s_part_end +  5, uint64_t>   s_secparts;          // bitmap of secondary partitions
	IntField_T<s_part_end +  6, uint64_t>   s_secblock;          // block size of secondary partition(s) ... 0 = 1*512 words, n = (n+1)*512 words
	IntField_T<s_part_end +  7, uint64_t>   s_secnblks;          // number of 512 wds blocks in secondary
	IntField_T<s_part_end +  8, uint64_t>   s_sbdbparts;         // bitmap of partitions with file system data including super blocks, dynamic block and free block bitmaps (only primary partitions may contain these)
	IntField_T<s_part_end +  9, uint64_t>   s_rootdparts;        // bitmap of partitions with root directory (only primary partitions)
	IntField_T<s_part_end + 10, uint64_t>   s_nudparts;          // bitmap of no-user-data partitions (only primary partitions)
	IntField_T<s_part_end + 11, uint64_t>   s_nsema;             // SFS: # fs semaphores to allocate
	IntField_T<s_part_end + 12, uint64_t>   s_priactive;         // bitmap of primary partitions which contain active (up to date) dynamic blocks and free block bitmaps. All bits set indicate that all primary partitions are active, and no kernel manipulation of active flag is allowed.
	IntField_T<s_part_end + 13, uint64_t>   s_sfs_arbiterid;     // SFS Arbiter ID
};

enum FdFlags_e {
	FDNC1_DOWN    = 0001,  // Slice is not available
	FDNC1_RDONLY  = 0002,  // Slice is read only
	FDNC1_NOALLOC = 0004,  // Slice is not available for allocation
	FDNC1_SBDB    = 0010,  // Slice has valid FS tables
	FDNC1_RTDIR   = 0020,  // Slice has valid ROOT Inode and directory
	FDNC1_SECALL  = 0100   // Slice sector allocated
};

struct nc1fdev_db {
	nc1fdev_db(uint64_t *aStore) : fd_flag(aStore), fd_ireg(aStore) {}
	IntField_T< 0, FdFlags_e, 0, 64>                        fd_flag;             // flag word
	ArrField_T< 1, IntField_T< 0, uint64_t>, NC1MAXIREG>    fd_ireg;             // Inode regions (number of available inodes per region)
};

struct lockinfo_t {
	lockinfo_t(uint64_t *aStore) : hi_proc(aStore), hi_syscall(aStore), hi_pid(aStore), hi_time(aStore) {}
	IntField_T< 0, uint64_t>      hi_proc;
	IntField_T< 1, uint32_t, 0>   hi_syscall;
	IntField_T< 1, uint32_t, 32>  hi_pid;
	IntField_T< 2, uint64_t>      hi_time;
};

const uint64_t BITMAPID = 05252525;
const uint64_t FREEMAP = 0x00465245454D4150ULL;


struct CFileTime_t {
	CFileTime_t(uint64_t *aStore) : msec(aStore), usec(aStore) {}
	IntField_T< 0, uint32_t,  0, 34>   msec;
	IntField_T< 0, uint32_t, 34, 30>   usec;
};

struct	map {
	map(uint64_t *aStore) : 
		bmp_name(aStore),
		bmp_id(aStore),
		bmp_base(aStore),
		bmp_begin(aStore),
		bmp_next(aStore),
		bmp_total(aStore),
		bmp_avail(aStore),
		bmp_want(aStore),
		bmp_first(aStore),
		bmp_last(aStore),
		bmp_units(aStore),
		bmp_tnobim(aStore)
	{}

	IntField_T< 0, uint64_t>      bmp_name;            // Name of map
	IntField_T< 1, uint64_t>      bmp_id;              // Bitmap identifier
	IntField_T< 2, uint64_t>      bmp_base;            // Base
	IntField_T< 3, uint64_t*>     bmp_begin;           // Beginning address of map body
	IntField_T< 4, uint64_t>      bmp_next;            // Offset to current unit
	IntField_T< 5, uint64_t>      bmp_total;           // Total number of units in map
	IntField_T< 6, uint64_t>      bmp_avail;           // Number of units currently available
	IntField_T< 7, uint64_t>      bmp_want;            // Map wait indicator
	IntField_T< 8, uint64_t>      bmp_first;           // Offset to first unit of bounded map
	IntField_T< 9, uint64_t>      bmp_last;            // Offset to last unit of bounded map
	IntField_T<10, uint64_t>      bmp_units;           // Number of units per bit
	IntField_T<11, uint64_t>      bmp_tnobim;          // Total number of bits in map
};

struct	nc1dblock {
	nc1dblock(uint64_t *aStore) :
		db_magic(aStore),
		db_tfree(aStore),
		db_ifree(aStore),
		db_ninode(aStore),

		db_state(aStore),
		db_time(aStore),
		db_type(aStore),
		db_spart(aStore),

		db_ifptr(aStore),
		db_actype(aStore),
		db_flag(aStore),
		db_res1(aStore),

		db_res2(aStore),
		db_res3(aStore),
		db_res4(aStore),
		db_res5(aStore),

		db_res6(aStore),
		db_res7(aStore),
		db_res8(aStore),
		db_res9(aStore),

		db_res10(aStore),
		db_part(aStore),
		db_lockinf(aStore),
		db_dpfptr(aStore),
		db_dsfptr(aStore),
		db_sfree(aStore),
		db_fpm(aStore)
	{}
	IntField_T< 0, uint64_t>                 db_magic;       // magic number to indicate file system type
	IntField_T< 1, CBlkNo_t>                 db_tfree;       // total available blocks
	IntField_T< 2, uint64_t>                 db_ifree;       // total free inodes
	IntField_T< 3, uint64_t>                 db_ninode;      // total allocated inodes

	IntField_T< 4, uint64_t>                 db_state;       // file system state
	IntField_T< 5, CTime_t>                  db_time;        // last dynamic block update
	IntField_T< 6, uint64_t>                 db_type;        // type of new file system
	IntField_T< 7, uint64_t>                 db_spart;       // Partition from which system mounted

	IntField_T< 8, uint64_t>                 db_ifptr;       // Inode allocation pointer
	IntField_T< 9, uint64_t>                 db_actype;      // device accounting type (for billing)
	IntField_T<10, DbFlags_e, 0, 64>         db_flag;        // Flag word
	IntField_T<11, uint64_t>                 db_res1;        // reserved

	IntField_T<12, uint64_t>                 db_res2;        // reserved
	IntField_T<13, uint64_t>                 db_res3;        // reserved
	IntField_T<14, uint64_t>                 db_res4;        // reserved
	IntField_T<15, uint64_t>                 db_res5;        // reserved

	IntField_T<16, uint64_t>                 db_res6;        // reserved
	IntField_T<17, uint64_t>                 db_res7;        // reserved
	IntField_T<18, uint64_t>                 db_res8;        // reserved
	IntField_T<19, uint64_t>                 db_res9;        // reserved

	IntField_T<20, uint64_t>                 db_res10;       // reserved
	ArrField_T<21, nc1fdev_db, NC1MAXPART>   db_part;        // Partition descriptors
	static constexpr size_t db_part_end = 21 + 5 * NC1MAXPART;
	SubField_T<db_part_end + 0, lockinfo_t>  db_lockinf;     // proc of the process locking the filesystem
	IntField_T<db_part_end + 3, uint64_t>    db_dpfptr;      // primary partitions allocation pointer
	IntField_T<db_part_end + 4, uint64_t>    db_dsfptr;      // secondary partitions allocation pointer
	IntField_T<db_part_end + 5, CBlkNo_t>    db_sfree;       // secondary parts free blocks
    SubField_T<db_part_end + 6, map>         db_fpm;         // Free blk map hdr - primary part
};

struct cdirect {
	cdirect(uint64_t *aStore) :
		cd_ino(aStore),
		cd_sino(aStore),
		cd_reserved(aStore),
		cd_signature(aStore),
		cd_reclen(aStore),
		cd_namelen(aStore),
		cd_name((char *)(aStore + 3))
	{}
	void SetBase(uint64_t *aBase) {
		cd_ino.SetBase(aBase);
		cd_sino.SetBase(aBase);
		cd_reserved.SetBase(aBase);
		cd_signature.SetBase(aBase);
		cd_reclen.SetBase(aBase);
		cd_namelen.SetBase(aBase);
		cd_name = ((char *)(aBase + 3));
	}
	IntField_T<0 , uint64_t>          cd_ino;              // Inode for name
	IntField_T<1, uint64_t>           cd_sino;             // Reserved for future use
	IntField_T<2, uint32_t, 0, 10>    cd_reserved;
	IntField_T<2, uint32_t, 10, 22>   cd_signature;        // Name signature
	IntField_T<2, uint32_t, 32, 22>   cd_reclen;           // Record length (bytes)
	IntField_T<2, uint32_t, 54, 10>   cd_namelen;          // Length of name (bytes)
	char *cd_name;                                         // NON-null terminated name - actual length comes from cd_namelen above
};

size_t UsedLength(cdirect &aEntry) {
	return ((aEntry.cd_namelen + 7) / 8 + 3) * 8;
}

bool IsLast(cdirect &aEntry) {
	return UsedLength(aEntry) < aEntry.cd_reclen;
}

struct DirEntry_s {
	DirEntry_s(const char *aFileName, size_t aINodeIdx): mEntry(nullptr) {
		size_t NameLen = strlen(aFileName);
		size_t EntrySize = 3 + (NameLen + sizeof(uint64_t) - 1) / sizeof(uint64_t);
		mStorage.resize(EntrySize);
		std::fill(mStorage.begin(), mStorage.end(), 0);
		mEntry.SetBase(&mStorage[0]);
		mEntry.cd_ino = aINodeIdx;
		mEntry.cd_signature = CalcHash(aFileName);
		mEntry.cd_reclen = uint32_t(EntrySize * sizeof(uint64_t));
		mEntry.cd_namelen = uint32_t(NameLen);
		memcpy(mEntry.cd_name, aFileName, NameLen); // Name is NOT null-terminated
	}
	uint32_t CalcHash(const char *aFileName) {
		size_t Length = strlen(aFileName);
		std::vector<uint64_t> Buffer((Length + 7) / 8);
		std::fill(Buffer.begin(), Buffer.end(), 0);
		memcpy(&(Buffer[0]), aFileName, Length);
		uint64_t XHash = 0;
		for (auto &Word : Buffer) {
			XHash ^= SwapBytes(Word);
		}
		uint64_t XHash2 = XHash + (XHash >> 7) + (XHash >> 17) + (XHash >> 27) + (XHash >> 37) + (XHash >> 47);
		XHash2 = XHash2 & 0x3fffff;
		return uint32_t(XHash2);
	}
	void *Storage() { return &mStorage[0]; }
	size_t size() { return mStorage.size() * sizeof(uint64_t); }

	std::vector<uint64_t> mStorage;
	cdirect mEntry;
};

const size_t SFSLK_SZ = 5;           // The # of on - disk inode lock words

enum AllocType_e {
	C1_EXTENT = 1,
	C2_TRACK = 2
};

struct cdinode {
	cdinode(uint64_t *aStore) :
		cdi_rsrvd_1(aStore),
		cdi_mode(aStore),
		cdi_msref(aStore),
		cdi_ms(aStore),
		cdi_nlink(aStore),
		cdi_uid(aStore),
		cdi_gid(aStore),
		cdi_acid(aStore),
		cdi_gen(aStore),
		cdi_size(aStore),
		
		cdi_moffset(aStore),
		cdi_blocks(aStore),
		cdi_extcomp(aStore),
		cdi_secrsvd1(aStore),
		cdi_smallcompart(aStore),
		cdi_slevel(aStore),
		cdi_intcls(aStore),
		cdi_secflg(aStore),
		cdi_intcat(aStore),
		
		cdi_privs(aStore),
		cdi_acl(aStore),
		cdi_cpart(aStore),
		cdi_dmport(aStore),
		cdi_dmstate(aStore),
		cdi_dmkey(aStore),
		cdi_allocf(aStore),
		cdi_alloc(aStore),
		cdi_cblks(aStore),
		cdi_dmmid(aStore),
		
		cdi_at(aStore),
		cdi_mt(aStore),
		cdi_ct(aStore),
		cdi_cbits(aStore),
		     
		cdi_addr(aStore),

		cdi_slock(aStore),
		cdi_rsrvd_5(aStore),
		cdi_applac(aStore),
		cdi_nindir(aStore),

		cdi_rsrvd(aStore),
		cdi_sitebits(aStore)
	{}
	IntField_T< 0, uint32_t,  0,  8>                     cdi_rsrvd_1;              // Reserved for expansion of cdi_mode
	IntField_T< 0, uint32_t,  8, 24>                     cdi_mode;                 // mode and type of file (4-bits still free)
	IntField_T< 0, uint8_t,  32,  1>                     cdi_msref;                // Modification signature is referenced flag
	IntField_T< 0, uint16_t, 33, 14>                     cdi_ms;                   // Modification signature
	IntField_T< 0, uint32_t, 47, 17>                     cdi_nlink;                // #of links to file (can hold > 100,000)
	IntField_T< 1, uint32_t,  0>                         cdi_uid;                  // Owner's user-ID
	IntField_T< 1, uint32_t, 32>                         cdi_gid;                  // Owner's group-ID
	IntField_T< 2, uint32_t,  0>                         cdi_acid;                 // Account-ID
	IntField_T< 2, uint32_t, 32>                         cdi_gen;                  // Inode generation number
	IntField_T< 3, uint64_t>                             cdi_size;                 // Number of bytes in the file
									                     
	IntField_T< 4, uint64_t>                             cdi_moffset;              // Modification offset for current signature
	IntField_T< 5, uint64_t,  0, 52>                     cdi_blocks;               // Quotas: #of blocks actually allocated
	IntField_T< 5, uint8_t,  52,  1>                     cdi_extcomp;              // Security: extended compartments flag
	IntField_T< 5, uint16_t, 53, 11>                     cdi_secrsvd1;             // Security: reserved
	IntField_T< 6, uint64_t>                             cdi_smallcompart;         // Security: compartments info ... Compartments if [0..63]
	IntField_T< 7, uint8_t,  0>                          cdi_slevel;               // Security: security level
	IntField_T< 7, uint8_t,  8>                          cdi_intcls;               // Security: integrity class
	IntField_T< 7, uint16_t, 16>                         cdi_secflg;               // Security: flag settings
	IntField_T< 7, uint32_t, 32>                         cdi_intcat;               // Security: integrity category
									                     
	IntField_T< 8, CBlkNo_t>                             cdi_privs;                // Extent descriptor or Block descriptor for Privilege Assignment List location
	IntField_T< 9, CBlkNo_t>                             cdi_acl;                  // Extent descriptor or Block descriptor for Security: ACL location		     */
	IntField_T<10, uint8_t,   0>                         cdi_cpart;                // Next partition from cbits to use
	IntField_T<10, uint8_t,   8,  3>                     cdi_dmport;               // DMF daemon number
	IntField_T<10, uint8_t,  11,  5>                     cdi_dmstate;              // DMF file state
	IntField_T<10, uint64_t, 16, 48>                     cdi_dmkey;                // Data-Migration: key
	IntField_T<11, uint8_t,   0,  4>                     cdi_allocf;               // Data-Block allocation flags
	IntField_T<11, uint8_t,   4,  4>                     cdi_alloc;                // Data-Block allocation technique
	IntField_T<11, uint32_t,  8, 24>                     cdi_cblks;                // Number of blocks to allocate per part
	IntField_T<11, uint32_t, 32>                         cdi_dmmid;                // Data-Migration: machine-ID
									                     
	SubField_T<12, CFileTime_t>                          cdi_at;                   // Access time
	SubField_T<13, CFileTime_t>                          cdi_mt;                   // Modification time (secs)
	SubField_T<14, CFileTime_t>                          cdi_ct;                   // Time of last inode modification (secs)
	IntField_T<15, uint64_t>                             cdi_cbits;                // bit mask, file placement within cluster
									                          
	ArrField_T<16, extent_t, 8>                          cdi_addr;                 // Extent descriptor or block descriptor

	ArrField_T<24, IntField_T<0, uint64_t> , SFSLK_SZ>   cdi_slock;                // Reserved for SFS lock structure
	IntField_T<24 + SFSLK_SZ + 1, uint16_t, 0>           cdi_rsrvd_5;
	IntField_T<24 + SFSLK_SZ + 1, uint32_t, 16>          cdi_applac;               // Application accounting tag
	IntField_T<24 + SFSLK_SZ + 1, uint16_t, 14>          cdi_nindir;               // # of indirect extent blocks

	IntField_T<24 + SFSLK_SZ + 1, uint64_t>              cdi_rsrvd;                // Reserved by the Kernel group
	IntField_T<24 + SFSLK_SZ + 2, uint64_t>              cdi_sitebits;             // Word reserved for site use

	static constexpr size_t size = 32;
};


#pragma pack(pop)

typedef boost::container::static_vector<uint64_t, cBlockSize> Block_t;

enum Modes_e {
	IFMT         = 00170000,    // type of file
	  IFDIR      = 00040000,    // directory
	  IFCHR      = 00020000,    // character special
	  IFBLK      = 00060000,    // block special
	  IFREG      = 00100000,    // regular
	  IFMPC      = 00030000,    // multiplexed char special
	  IFMPB      = 00070000,    // multiplexed block special
	  IFIFO      = 00010000,    // fifo special
	  IFOFD      = 00110000,    // off line, with data
	  IFOFL      = 00120000,    // off line, with no data
	  IFLNK      = 00130000,    //  symbolic link
	  IFSOCK     = 00140000,    // TCP/IP CODE: UNIX domain socket
	IFBIG        = 00400000,    // File is large
	ISFSDEFERTM  = 01000000,    // SFS deferred a/c/mtime semantics
	IRESTART     = 02000000,    // File is a chkpnt/restart file

	MODEMSK      = 00006777,    // file creation mode mask
	PERMMSK      = 00000777     // file permissions mode mask
};

class DiskImage_c {
public:
	typedef std::map<size_t, Block_t> SparseStorage_t;
	SparseStorage_t mStorage;

	size_t mDiskSizeInBlocks;

	void Finalize() {
//		Block_t &Block = GetBlock(0);
//		std::fill(Block.begin(), Block.end(), std::numeric_limits<uint64_t>::max());

		nc1filsys SuperBlock = GetSuperBlock();
		Block_t &RawSuperBlock = GetBlock(SUPERB);
		size_t SuperBlockCnt = SuperBlock.s_nscpys;
		for (size_t i = 1; i < SuperBlockCnt; ++i) {
			Block_t &SuperBlockCopy = GetBlock(SUPERB + i);
			SuperBlockCopy = RawSuperBlock;
		}
	}

	void WriteToDisk(const char *aFileName) {
		std::ofstream File(aFileName, std::ios::out | std::ios::binary);
		Block_t EmptyBlock;
		EmptyBlock.resize(cBlockSize);
		std::fill(EmptyBlock.begin(), EmptyBlock.end(), 0);
		for (size_t BlockIdx = 0; BlockIdx < mDiskSizeInBlocks; ++BlockIdx) {
			auto Block = mStorage.find(BlockIdx);
			// TODO swap bytes in words!
			if (Block == mStorage.end()) {
				File.write((char*)(&EmptyBlock[0]), EmptyBlock.size()*sizeof(uint64_t));
			} else {
				for (auto &Word : Block->second) {
					//Word = SwapBytes(Word);
					File.write((char*)(&Word), sizeof(Word));
				}
			}
			if (File.fail()) throw Generic_x("Can't write to disk image");
		}
	}

	Block_t &GetBlock(size_t aBlockIdx) {
		auto Block = mStorage.find(aBlockIdx);
		if (Block != mStorage.end()) return Block->second;

		Block_t EmptyBlock;
		EmptyBlock.resize(cBlockSize);
		std::fill(EmptyBlock.begin(), EmptyBlock.end(), 0);
		mStorage[aBlockIdx] = EmptyBlock;
		return mStorage[aBlockIdx];
	}

	nc1filsys GetSuperBlock() {
		Block_t &RawSuperBlock = GetBlock(SUPERB);
		return nc1filsys(&RawSuperBlock[0]);
	}

	nc1dblock GetDynamicBlock() {
		nc1filsys SuperBlock = GetSuperBlock();
		Block_t &RawDynamicBlock = GetBlock(SuperBlock.s_dboff);
		return nc1dblock(&RawDynamicBlock[0]);
	}

	void CreateSuperBlock(const char *aFsName, const char *aPackName, const char *aPartName, uint8_t aMajorDev, uint8_t aMinorDev, size_t aINodeRatio = 4) {
		size_t MapSize = mDiskSizeInBlocks / 64;
		size_t MapBlockCnt = (MapSize + cBlockSize - 1) / cBlockSize;
		size_t MapStart = SUPERB + NC1NSUPER;

		size_t DynamicBlockStart = MapStart + MapBlockCnt;

		size_t INodeCnt = std::min(mDiskSizeInBlocks / aINodeRatio, cBlockSize * 64);
		size_t INodeBlockCnt = 1 + (INodeCnt * cdinode::size + cBlockSize - 1) / cBlockSize;
		size_t INodeStart = DynamicBlockStart + 1;

		size_t SFSStart = INodeStart + INodeBlockCnt;
		size_t SFSBlockCnt = 1;

		nc1filsys SuperBlock = GetSuperBlock();
		SuperBlock.s_magic = FsMAGIC_NC1;                                     // magic number to indicate file system type
		SuperBlock.s_fname = aFsName;                                         // file system name
		SuperBlock.s_fpack = aPackName;                                       // file system pack name
		SuperBlock.s_dev = aMajorDev << 8 | aMinorDev;                        // major/minor device, for verification
		SuperBlock.s_fsize = mDiskSizeInBlocks;                               // size in blocks of entire volume
		SuperBlock.s_isize = INodeCnt;                                        // Number of total inodes
		SuperBlock.s_bigfile = 0x8000;                                        // number of bytes at which a file is big
		SuperBlock.s_bigunit = 21;                                            // minimum number of blocks allocated for big files
		SuperBlock.s_secure = FsSECURE;                                       // security: secure FS label
//		SuperBlock.s_maxlvl = 0;                                              // security: maximum security level
//		SuperBlock.s_minlvl = 0;                                              // security: minimum security level
//		SuperBlock.s_valcmp = 0;                                              // security: valid security compartments
		SuperBlock.s_time = 895339878;                                        // last super block update (sometime in 2001)
		SuperBlock.s_dboff = DynamicBlockStart;                               // Dynamic block number
		SuperBlock.s_root = 2;                                                // root inode
		SuperBlock.s_error = 0;                                               // Type of file system error detected
		SuperBlock.s_mapoff = MapStart;                                       // Start map block number
		SuperBlock.s_mapblks = MapBlockCnt;                                   // Last map block number - ??? This is more like the number of map blocks
		SuperBlock.s_nscpys = NC1NSUPER;                                      // Number of copies of s.b per partition
		SuperBlock.s_npart = 1;                                               // Number of partitions
		SuperBlock.s_ifract = aINodeRatio;                                    // Ratio of inodes to blocks
		SuperBlock.s_sfs.get().nblks = uint32_t(SFSBlockCnt);                 // SFS only blocks
		SuperBlock.s_sfs.get().blk = uint32_t(SFSStart);                      // SFS only blocks
		SuperBlock.s_flag = Fs_RRFILE | Fs_RRALLUDATA;                        // Flag word
		SuperBlock.s_part[0].fd_name = aPartName;                             // Physical device name
		SuperBlock.s_part[0].fd_sblk = 0;                                     // Start block number
		SuperBlock.s_part[0].fd_nblk = uint32_t(mDiskSizeInBlocks);           // Number of blocks
		SuperBlock.s_part[0].fd_ireg[0].nblks = uint32_t(INodeBlockCnt);      // Inode regions
		SuperBlock.s_part[0].fd_ireg[0].blk = uint32_t(INodeStart);           // Inode regions
		SuperBlock.s_iounit = 0;                                              // Physical block size
		SuperBlock.s_numiresblks = 0;                                         // number of inode reservation blocks per region (currently 1) - 0 = 1*(AU) words, n = (n+1)*(AU) words
		SuperBlock.s_priparts = 1ULL << 63;                                   // bitmap of primary partitions
		SuperBlock.s_priblock = 0;                                            // block size of primary partition(s) - 0 = 1*512 words, n = (n+1)*512 words
		SuperBlock.s_prinblks = mDiskSizeInBlocks;                            // number of 512 wds blocks in primary
		SuperBlock.s_secparts = 0;                                            // bitmap of secondary partitions
		SuperBlock.s_secblock = 0;                                            // block size of secondary partition(s) - 0 = 1*512 words, n = (n+1)*512 words
		SuperBlock.s_secnblks = 0;                                            // number of 512 wds blocks in secondary
		SuperBlock.s_sbdbparts = 1ULL << 63;	                              // bitmap of partitions with file system data, including super blocks, dynamic block, and free block bitmaps (only primary partitions may contain these)
		SuperBlock.s_rootdparts = 1ULL << 63;                                 // bitmap of partitions with root directory (only primary partitions)
		SuperBlock.s_nudparts = 0;                                            // bitmap of no-user-data partitions (only primary partitions)
		SuperBlock.s_nsema = 0;                                               // SFS: # fs semaphores to allocate
		SuperBlock.s_priactive = 1ULL << 63;                                  // bitmap of primary partitions which contain active (up to date) dynamic blocks and  free block bitmaps. All bits set indicate that all primary partitions are active, and no kernel manipulation of active flag is allowed.
		SuperBlock.s_sfs_arbiterid = 0;                                       // SFS Arbiter ID
																	          
		nc1dblock DynamicBlock = GetDynamicBlock();					          
		DynamicBlock.db_magic = DbMAGIC_NC1;                                  // magic number to indicate file system type
		DynamicBlock.db_tfree = mDiskSizeInBlocks;                            // total available blocks - start from full disk as the 'Allocate' calls below will make sure that all superblock, dynmaic blocks, etc. are accounted for here
		DynamicBlock.db_ifree = INodeCnt;                                     // total free inodes
		DynamicBlock.db_ninode = (uint64_t)(-int(SuperBlock.s_root));         // total allocated inodes (INode 0 and 1 are not accounted for, so offset by 2)
		DynamicBlock.db_state = 0;                                            // file system state
		DynamicBlock.db_time = 895339878;                                     // last dynamic block update (sometime in 2001)
		DynamicBlock.db_type = 0;                                             // type of new file system - ??? what is this ???
		DynamicBlock.db_spart = 0;                                            // Partition from which system mounted
		DynamicBlock.db_ifptr = 0;                                            // Inode allocation pointer
		DynamicBlock.db_actype = 0;                                           // device accounting type (for billing)
		DynamicBlock.db_flag = Db_CHECKED;                                    // Flag word
		DynamicBlock.db_part[0].fd_flag = FdFlags_e(
			FDNC1_SBDB | 
			FDNC1_RTDIR | 
			FDNC1_SECALL
		);                                                                    // Partition descriptors: flag word
		DynamicBlock.db_part[0].fd_ireg[0] = INodeCnt;                        // Partition descriptors: Inode regions
//		DynamicBlock.db_lockinf;                                              // proc of the process locking the filesystem
//		DynamicBlock.db_dpfptr;                                               // primary partitions allocation pointer
//		DynamicBlock.db_dsfptr;                                               // secondary partitions allocation pointer
//		DynamicBlock.db_sfree;                                                // secondary parts free blocks
		DynamicBlock.db_fpm.get().bmp_name = FREEMAP;                         // Free blk map hdr - primary part. - Name of map
		DynamicBlock.db_fpm.get().bmp_id = BITMAPID;                          // Free blk map hdr - primary part. - Bitmap identifier
		DynamicBlock.db_fpm.get().bmp_base = 0;                               // Free blk map hdr - primary part. - Base
		DynamicBlock.db_fpm.get().bmp_begin = 0;                              // Free blk map hdr - primary part. - Beginning address of map body
		DynamicBlock.db_fpm.get().bmp_next = 0;                               // Free blk map hdr - primary part. - Offset to current unit
		DynamicBlock.db_fpm.get().bmp_total = mDiskSizeInBlocks;              // Free blk map hdr - primary part. - Total number of units in map
		DynamicBlock.db_fpm.get().bmp_avail = mDiskSizeInBlocks;              // Free blk map hdr - primary part. - Number of units currently available
		DynamicBlock.db_fpm.get().bmp_want = 0;                               // Free blk map hdr - primary part. - Map wait indicator
		DynamicBlock.db_fpm.get().bmp_first = 0;                              // Free blk map hdr - primary part. - Offset to first unit of bounded map
		DynamicBlock.db_fpm.get().bmp_last = 0;                               // Free blk map hdr - primary part. - Offset to last unit of bounded map
		DynamicBlock.db_fpm.get().bmp_units = 1;                              // Free blk map hdr - primary part. - Number of units per bit
		DynamicBlock.db_fpm.get().bmp_tnobim = mDiskSizeInBlocks;             // Free blk map hdr - primary part. - Total number of bits in map

		Block_t &SFSBlock = GetBlock(SFSStart);
		SFSBlock[0] = SwapBytes(uint64_t(0x7366733131736673ULL));
		SFSBlock[0xCB] = SwapBytes(uint64_t(0x7366733131736673ULL));

		AllocateBlock(0); // Mark sector 0 as occupied

		// Mark all the superblock copies occupied
		for (size_t i = 0; i < NC1NSUPER; ++i) {
			AllocateBlock(SUPERB + i);
		}
		// Mark the map occupied
		for (size_t i = 0; i < MapBlockCnt; ++i) {
			AllocateBlock(MapStart + i);
		}
		// Mark the dynamic block occipied
		AllocateBlock(DynamicBlockStart);
		// Mark all SFS-only blocks occupied
		for (size_t i = 0; i < SFSBlockCnt; ++i) {
			AllocateBlock(SFSStart + i);
		}
		// Mark the inodes occupied
		for (size_t i = 0; i < INodeBlockCnt; ++i) {
			AllocateBlock(INodeStart + i);
		}

		// Make sure no INode can be smaller then the root
		for (size_t i = 0; i < SuperBlock.s_root; ++i) {
			AllocateINodeIdx(i);
		}
	}

	uint64_t GetMapBit(size_t aBitIdx) {
		//uint64_t MapBit = uint64_t(1) << ((63-aIdx) % 64);
		return uint64_t(1) << ((7 - (aBitIdx % 8)) + ((aBitIdx / 8) * 8));
	}

	void AllocateInMap(size_t aIdx, size_t aMapStart) {
		size_t MapOfs = aIdx / 64;
		size_t MapPage = MapOfs / cBlockSize;
		MapOfs = MapOfs % cBlockSize;
		uint64_t MapBit = GetMapBit(aIdx % 64);
		Block_t &AllocationMap = GetBlock(aMapStart + MapPage);
		CRAY_ASSERT((AllocationMap[MapOfs] & MapBit) == uint64_t(0));
		AllocationMap[MapOfs] |= MapBit;
	}

	size_t GetFreeEntry(size_t aMapStart, size_t aMapBlockCnt) {
		for (uint32_t Page = 0; Page < aMapBlockCnt; ++Page) {
			Block_t Map = GetBlock(aMapStart + Page);
			for (size_t Idx = 0; Idx < Map.size(); ++Idx) {
				if (Map[Idx] != UINT64_MAX) {
					for (size_t Bit = 0; Bit < 64; ++Bit) {
						uint64_t MapBit = GetMapBit(Bit);
						if ((Map[Idx] & MapBit) == 0) {
							size_t RetVal = (Page * cBlockSize + Idx) * 64 + Bit;
							return RetVal;
						}
					}
				}
			}
		}
		throw Generic_x("Disk is full");
	}

	void AllocateBlock(size_t aBlock) {
		nc1filsys SuperBlock = GetSuperBlock();
		nc1dblock DynamicBlock = GetDynamicBlock();

		AllocateInMap(aBlock, SuperBlock.s_mapoff);

		DynamicBlock.db_fpm.get().bmp_avail = DynamicBlock.db_fpm.get().bmp_avail - 1;
		DynamicBlock.db_tfree = DynamicBlock.db_tfree - 1;
	}

	size_t GetFreeBlock() {
		nc1filsys SuperBlock = GetSuperBlock();

		size_t MapStart = SuperBlock.s_mapoff;
		size_t MapBlockCnt = SuperBlock.s_mapblks;

		size_t RetVal = GetFreeEntry(MapStart, MapBlockCnt);
		AllocateBlock(RetVal);
		return RetVal;
	}

	void AllocateINodeIdx(size_t aINodeIdx) {
		nc1filsys SuperBlock = GetSuperBlock();
		nc1dblock DynamicBlock = GetDynamicBlock();

		size_t MapStart = SuperBlock.s_part[0].fd_ireg[0].blk;

		AllocateInMap(aINodeIdx, MapStart);

		DynamicBlock.db_ifree = DynamicBlock.db_ifree - 1;
		DynamicBlock.db_ninode = DynamicBlock.db_ninode + 1;
		DynamicBlock.db_part[0].fd_ireg[0] = DynamicBlock.db_part[0].fd_ireg[0] - 1;
	}

	uint32_t GetFreeINodeIdx() {
		nc1filsys SuperBlock = GetSuperBlock();

		size_t MapStart = SuperBlock.s_part[0].fd_ireg[0].blk;

		size_t RetVal = GetFreeEntry(MapStart, 1);
		AllocateINodeIdx(RetVal);
		return uint32_t(RetVal);
	}

	cdinode GetINode(uint64_t aINodeIdx) {
		nc1filsys SuperBlock = GetSuperBlock();

		size_t INodeStartBlock = SuperBlock.s_part[0].fd_ireg[0].blk + 1;
		size_t INodeBlock = INodeStartBlock + (aINodeIdx / (cBlockSize / cdinode::size));
		size_t INodeOfs = (aINodeIdx % (cBlockSize / cdinode::size)) * cdinode::size;
		Block_t &RawBlock = GetBlock(INodeBlock);

		return cdinode(&(RawBlock[INodeOfs]));
	}

	uint32_t MakeRoot() {
		return MkDir(0, nullptr); // Make root dir	
	}

	const static uint32_t RT_UID = 0;
	const static uint32_t RT_GID = 0;
	const static uint32_t RT_MODE = 0755;

	void AppendToDir(uint32_t aINode, DirEntry_s &aDirEntry) {
		cdinode INode = GetINode(aINode);
		CRAY_ASSERT((INode.cdi_mode & IFMT) == IFDIR);
		size_t BlockDescIdx = 0;
		size_t BlockDescIdxMax = INode.cdi_addr.size;
		uint32_t CurrBlock = INode.cdi_addr[BlockDescIdx].blk;
		uint32_t CurrBlockCnt = INode.cdi_addr[BlockDescIdx].nblks;
		uint32_t CurrEndBlock = CurrBlock + CurrBlockCnt;
		size_t CurrBlockSize = CurrBlockCnt * BSIZE;
		// For now we can only handle directories of one allocation block
		CRAY_ASSERT(INode.cdi_size == CurrBlockSize);
		CRAY_ASSERT(CurrBlockCnt <= 1);
		uint64_t *InsertLocation = nullptr;
		uint64_t *EndLocation = nullptr;
		Block_t *Block = nullptr;
		if (CurrBlockCnt == 0) {
			uint32_t NewBlock = uint32_t(GetFreeBlock());
			CRAY_ASSERT(CurrBlock == 0);
			CRAY_ASSERT(BlockDescIdx == 0);
			INode.cdi_addr[BlockDescIdx].blk = NewBlock;
			INode.cdi_addr[BlockDescIdx].nblks = 1;
			INode.cdi_size = INode.cdi_addr[BlockDescIdx].nblks * BSIZE;
			INode.cdi_blocks = INode.cdi_addr[BlockDescIdx].nblks;
			Block = &GetBlock(NewBlock);
			InsertLocation = &((*Block)[0]);
			EndLocation = InsertLocation + cBlockSize;
		} else {
			// Find the last entry in the directory
			Block = &GetBlock(CurrBlock);
			InsertLocation = &((*Block)[0]);
			EndLocation = InsertLocation + cBlockSize;
			do {
				cdirect Dir(InsertLocation);
				InsertLocation += UsedLength(Dir) / sizeof(uint64_t);
				CRAY_ASSERT(InsertLocation <= EndLocation);
				if (IsLast(Dir)) {
					// Patch up the current entry to occupy as little space as possible
					Dir.cd_reclen = uint32_t(UsedLength(Dir));
					break;
				}
			} while (true);
		}
		CRAY_ASSERT(InsertLocation + UsedLength(aDirEntry.mEntry) / sizeof(uint64_t) <= EndLocation);
		memcpy(InsertLocation, &(aDirEntry.mStorage[0]), UsedLength(aDirEntry.mEntry));
		cdirect Dir(InsertLocation);
		Dir.cd_reclen = uint32_t((EndLocation - InsertLocation) * sizeof(uint64_t));
	}

	void AppendToFile(uint32_t aINode, const void *aData, size_t aDataSize) {
		uint8_t *Data = (uint8_t*)(aData);
		cdinode INode = GetINode(aINode);
		size_t NewSize = INode.cdi_size + aDataSize;
		// Make sure file is regular file or directory
		CRAY_ASSERT(((INode.cdi_mode & IFMT) == IFDIR) || ((INode.cdi_mode & IFMT) == IFREG));
		// Walk the existing block till we find the end
		size_t BytesInFileLeft = INode.cdi_size;
		size_t BlockDescIdx = 0;
		size_t BlockDescIdxMax = INode.cdi_addr.size;
		uint32_t CurrBlock = INode.cdi_addr[BlockDescIdx].blk;
		uint32_t CurrBlockCnt = INode.cdi_addr[BlockDescIdx].nblks;
		uint32_t CurrEndBlock = CurrBlock + CurrBlockCnt;
		size_t CurrBlockSize = CurrBlockCnt * BSIZE;
		size_t TotalBlocks = 0;
		while (BytesInFileLeft > CurrBlockSize) {
			BytesInFileLeft -= CurrBlockSize;
			++BlockDescIdx;
			TotalBlocks += CurrBlockCnt;
			CRAY_ASSERT(BlockDescIdx < BlockDescIdxMax);
			CurrBlock = INode.cdi_addr[BlockDescIdx].blk;
			CurrBlockCnt = INode.cdi_addr[BlockDescIdx].nblks;
			CurrEndBlock = CurrBlock + CurrBlockCnt;
			CurrBlockSize = CurrBlockCnt * BSIZE;
			// TODO: how to handle indirect blocks
		}
		// We've found he allocation chunk that contains the end of the file, find the block in it...
		CurrBlock += uint32_t(BytesInFileLeft / BSIZE);
		TotalBlocks += CurrBlockCnt;
		if (CurrBlock != CurrEndBlock) {
			BytesInFileLeft %= BSIZE;
			// We've found the end of the blocks, we have to figure out how many more to allocate (if any)
			// First, fill the last block
			size_t RoomLeft = std::min(BSIZE - BytesInFileLeft, aDataSize);
			memcpy((uint8_t*)(&GetBlock(CurrBlock)[0]) + BytesInFileLeft, Data, RoomLeft);
			Data += RoomLeft;
			aDataSize -= RoomLeft;
			CurrBlock++;
		}
		if (aDataSize > 0) {
			CRAY_ASSERT(INode.cdi_blocks == TotalBlocks);
		}
		while (aDataSize > 0) {
			// Find out how many blocks we need
			uint32_t BlocksNeeded = uint32_t((aDataSize+BSIZE-1) / BSIZE);
			// If there are any unused, but allocated blocks, use them first
			while (CurrBlock != CurrEndBlock) {
				size_t RoomLeft = std::min(BSIZE, aDataSize);
				memcpy(&GetBlock(CurrBlock)[0], Data, RoomLeft);
				Data += RoomLeft;
				aDataSize -= RoomLeft;
				CurrBlock++;
				BlocksNeeded--;
				if (aDataSize == 0) break;
			}
			// Try to allocate more blocks
			if (BlocksNeeded != 0) {
				uint32_t NewBlock = uint32_t(GetFreeBlock());
				// Either add the range to the inode or merge it with the current range, if possible
				if (CurrEndBlock == NewBlock) {
					// merge
					INode.cdi_addr[BlockDescIdx].nblks = INode.cdi_addr[BlockDescIdx].nblks + 1;
				} else {
					// add
					if (CurrBlock == 0 && CurrBlockCnt == 0) {
						CRAY_ASSERT(BlockDescIdx == 0);
					} else {
						BlockDescIdx++;
					}
					CRAY_ASSERT(BlockDescIdx <= BlockDescIdxMax);
					INode.cdi_addr[BlockDescIdx].blk = NewBlock;
					INode.cdi_addr[BlockDescIdx].nblks = 1;
				}
				CurrBlock = NewBlock;
				CurrEndBlock = CurrBlock + 1;
				CurrBlockSize = BSIZE;
				CurrBlockCnt = 1;
				TotalBlocks += CurrBlockCnt;
			}
		}
		INode.cdi_blocks = TotalBlocks;
		INode.cdi_size = NewSize;
	}

	uint32_t MkDir(uint32_t aParentINodeIdx, const char *aName, uint32_t aUID = RT_UID, uint32_t aGID = RT_GID, uint32_t aMode = RT_MODE) {
		nc1filsys SuperBlock = GetSuperBlock();

		uint32_t MyINodeIdx = (aParentINodeIdx != 0) ? GetFreeINodeIdx() : uint32_t(SuperBlock.s_root);
		if (aParentINodeIdx == 0) AllocateINodeIdx(MyINodeIdx);
		cdinode MyINode = GetINode(MyINodeIdx);

		MyINode.cdi_mode = IFDIR | aMode;
		MyINode.cdi_nlink = 2;
		MyINode.cdi_uid = aUID;
		MyINode.cdi_gid = aGID;
		MyINode.cdi_size = 0;
		MyINode.cdi_blocks = 0;
		MyINode.cdi_nindir = 0;
		MyINode.cdi_alloc = C1_EXTENT;

		DirEntry_s MyDirEntry(".", MyINodeIdx);
		AppendToDir(MyINodeIdx, MyDirEntry);
		DirEntry_s UpDirEntry("..", (aParentINodeIdx != 0) ? aParentINodeIdx : MyINodeIdx);
		AppendToDir(MyINodeIdx, UpDirEntry);

		// Add name to parent
		if (aParentINodeIdx != 0) {
			DirEntry_s RefDirEntry(aName, MyINodeIdx);
			AppendToDir(aParentINodeIdx, RefDirEntry);
			// Increment parent's ref-count
			cdinode Parent = GetINode(aParentINodeIdx);
			Parent.cdi_nlink = Parent.cdi_nlink + 1;
		}
		return MyINodeIdx;
	}

	uint32_t MkFile(uint32_t aParentINodeIdx, const char *aName, uint32_t aUID = RT_UID, uint32_t aGID = RT_GID, uint32_t aMode = RT_MODE) {
		CRAY_ASSERT(aParentINodeIdx != 0);

		uint32_t MyINodeIdx = GetFreeINodeIdx();

		cdinode MyINode = GetINode(MyINodeIdx);

		MyINode.cdi_mode = IFREG | aMode;
		MyINode.cdi_nlink = 1;
		MyINode.cdi_uid = aUID;
		MyINode.cdi_gid = aGID;
		MyINode.cdi_size = 0;
//		MyINode.cdi_blocks = 0;
		MyINode.cdi_nindir = 0;
		MyINode.cdi_alloc = C1_EXTENT;

		// Add name to current
		DirEntry_s RefDirEntry(aName, MyINodeIdx);
		AppendToDir(aParentINodeIdx, RefDirEntry);

		return MyINodeIdx;
	}
//	uint32_t MkPath(const char *aPath, uint32_t aUID = RT_UID, uint32_t aGID = RT_GID, uint32_t aMode = RT_MODE) {
//		std::string Path(aPath);
//
//		typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
//		boost::char_separator<char> Separators("/");
//		tokenizer PathElements(Path, Separators);
//
//		nc1filsys SuperBlock = GetSuperBlock();
//		size_t INodeIdx = SuperBlock.s_root;
//		for (auto &PathElement : PathElements) {
//			if (PathElement.length() == 0) continue;
//			
//		}
//	}

	void CopyFile(uint32_t aParentINodeIdx, const char *aName, const char *aLocalFileName, uint32_t aUID = RT_UID, uint32_t aGID = RT_GID, uint32_t aMode = RT_MODE, bool aIsBinary = true) {
		std::ifstream Strm(aLocalFileName, std::ios::in | (aIsBinary ? std::ios::binary : std::ios_base::openmode(0)));
		if (!Strm.good()) throw Generic_x() << "Can't open local file: " << aLocalFileName;
		uint32_t INodeIdx = MkFile(aParentINodeIdx, aName, aUID, aGID, aMode);
		while (!Strm.eof()) {
			std::vector<char> Buffer(1024 * 1024);
			Strm.read(&(Buffer[0]), Buffer.size());
			if (Strm.bad()) throw Generic_x() << "Can't read local file: " << aLocalFileName;
			size_t BytesRead = Strm.gcount();
			std::fill(Buffer.begin() + BytesRead, Buffer.end(), 0);

//			size_t WordCnt = ((BytesRead + 7) / 8);
//			uint64_t *IntBuffer = (uint64_t *)(&Buffer[0]);
//			for (size_t Idx = 0; Idx < WordCnt; ++Idx) {
//				IntBuffer[Idx] = SwapBytes(IntBuffer[Idx]);
//			}

			AppendToFile(INodeIdx, &(Buffer[0]), BytesRead);
		}
	}

	void MakeFs(
		size_t mDiskSizeInBytes, 
		const char *aFsName, 
		const char *aPackName, 
		const char *aPartName,
		uint8_t aMajorDev,
		uint8_t aMinorDev,
		size_t aINodeRatio = 4
	) {
		mDiskSizeInBlocks = (mDiskSizeInBytes + BSIZE - 1) / BSIZE; // Round up to blocks

		CreateSuperBlock(aFsName, aPackName, aPartName, aMajorDev, aMinorDev, aINodeRatio);
		uint32_t RootINodeIdx = MakeRoot();
//		uint32_t EtcINodeIdx = MkDir(RootINodeIdx, "etc");
		MkDir(RootINodeIdx, "lost+found");
//		MkDir(RootINodeIdx, "usr");
//		MkDir(RootINodeIdx, "drop");
	}
};

int PrintUsage(const char *aExecName, const char *ErrorStr) {
	if (ErrorStr != nullptr) std::cout << "Error: " << ErrorStr << std::endl;
	std::cout << "Usage: " << aExecName << " -o <Image file name> -s <image size> [-f <file system name>] [-p <partition name>] [-cp <local file> <image file> ...]" << std::endl;
	std::cout << std::endl;
	std::cout << "\t" << "Options:" << std::endl;
	std::cout << "\t" << "-major: specifies major device number (defaults to 34)" << std::endl;
	std::cout << "\t" << "-minor: specifies minor device number (defaults to 0)" << std::endl;
	std::cout << "\t" << "-o: specifies the output image name" << std::endl;
	std::cout << "\t" << "-f: specifies the file system name" << std::endl;
	std::cout << "\t" << "-s: size of the image size in optional units. Unit can be kb, mb or gb or b for 4kb blocks." << std::endl;
	std::cout << "\t" << "-p: specifies the pack name" << std::endl;
	std::cout << "\t" << "-cp: specifies file to be copied onto the image (for now image file names can't contain path info)" << std::endl;
	return 1;
}

int main(int argc, const char **argv) {
	struct FileCopyInfo_s {
		std::string LocalFileName;
		std::string ImageFileName;
	};
	CommandLine_c CommandLine(argc,argv);
	try {
		std::string ImgFileName;
		std::string FsName;
		std::string PartitionName;
		size_t ImageSize = 0;
		std::vector<FileCopyInfo_s> FileCopyList;
		uint8_t MajorDev = 34;
		uint8_t MinorDev = 0;

		while(CommandLine.HasMoreParams()) {
			std::string CurParam = CommandLine.GetNextParam();
			if (CurParam == "-o") {
				if (!ImgFileName.empty()) throw Generic_x("Image file name is already specified");
				ImgFileName = CommandLine.GetNextParam();
			} else if (CurParam == "-s") {
				char *EndPtr;
				ImageSize = std::strtoul(CommandLine.GetNextParam().c_str(), &EndPtr, 10);
				if (EndPtr[0] != 0) {
					if (strcmp(EndPtr, "kb") == 0) {
						ImageSize *= 1024;
					} else if (strcmp(EndPtr, "mb") == 0) {
						ImageSize *= 1024 * 1024;
					} else if (strcmp(EndPtr, "gb") == 0) {
						ImageSize *= 1024 * 1024 * 1024;
					} else if (strcmp(EndPtr, "s") == 0) {
						ImageSize *= 1024 * 4;
					} else {
						throw Generic_x("Image size must be an unsigned integer with optional kb, mb or gb or b at the end to denote unit");
					}
				}
			} else if (CurParam == "-major") {
				char *EndPtr;
				MajorDev = uint8_t(std::strtoul(CommandLine.GetNextParam().c_str(), &EndPtr, 10));
				if (EndPtr[0] != 0) {
					throw Generic_x("Major device must be an unsigned integer");
				}
			} else if (CurParam == "-minor") {
				char *EndPtr;
				MinorDev = uint8_t(std::strtoul(CommandLine.GetNextParam().c_str(), &EndPtr, 10));
				if (EndPtr[0] != 0) {
					throw Generic_x("Minor device must be an unsigned integer");
				}
			} else if (CurParam == "-f") {
				if (!FsName.empty()) throw Generic_x("File system name is already specified");
				FsName = CommandLine.GetNextParam();
			} else if (CurParam == "-cp") {
				FileCopyInfo_s FileCopyInfo;
				FileCopyInfo.LocalFileName = CommandLine.GetNextParam();
				FileCopyInfo.ImageFileName = CommandLine.GetNextParam();
				FileCopyList.emplace_back(FileCopyInfo);
			} else if (CurParam == "-p") {
				if (!PartitionName.empty()) throw Generic_x("Partition name is already specified");
				PartitionName  = CommandLine.GetNextParam();
			} else {
				throw Generic_x() << "Unknown command line argument: " << CurParam;
			}
		}
		if (ImgFileName.empty()) throw Generic_x("Image file name was not specified");
		if (FsName.empty()) FsName = "rootfs";
		if (PartitionName.empty()) PartitionName = "root";
		if (FsName.size() > 7) throw Generic_x("File system name is too long");
		if (PartitionName.size() > 7) throw Generic_x("Pack name is too long");
		if (ImageSize == 0) throw Generic_x("Image size must be specified");
		if (ImageSize < 1024*1024) throw Generic_x("Image size must be at least a megabyte");
		std::cout << "Generating image..." << std::endl;
		DiskImage_c DiskImage;
		DiskImage.MakeFs(ImageSize, FsName.c_str(), FsName.c_str(), PartitionName.c_str(), MajorDev, MinorDev);
		for (auto &FileCopyInfo : FileCopyList) {
			std::cout << "Copying file " << FileCopyInfo.LocalFileName << " to image as " << FileCopyInfo.ImageFileName << " ..." << std::flush;
			DiskImage.CopyFile(2, FileCopyInfo.ImageFileName.c_str(), FileCopyInfo.LocalFileName.c_str());
			std::cout << " done." << std::endl;
		}
		DiskImage.Finalize();
		std::cout << "Writing image to disk..." << std::flush;
		DiskImage.WriteToDisk(ImgFileName.c_str());
		std::cout << " done" << std::endl;
	}
	catch(std::exception &Ex) {
		return PrintUsage(argv[0], Ex.what());
	}
	return 0;
}
