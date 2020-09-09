@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Input:
@@       OR[280] : Device desc.
@@       OR[281] : 68 WORD long Output buffer
@@       OR[282] : Directory name string pointer
@@       OR[283] : File name string pointer
@@
@@       OR[284] : Return value
@@       OR[285] : Local 2k buffer
@@
@@ Disk layout so far:
@@ Sector 3: contains some info on whether the disk is initialized:
@@          first word: 0xffff
@@          next words: Volume label string
@@ Sector 4: free sector chain array - probably sorted by small-to-large, though it might not be the case
@@          offset 7: number of entries in the array
@@          offset 8: first sector in the chain
@@          offset 9: number of sectors in the chain
@@ Sector 5: first directory entry
@@          offset 0: link to next directory entry sector, or 0 for the last such sector
@@          offset 8: first directory entry
@@          offset 68: second directory entry
@@          offset 128: 3rd directory entry
@@          ....
@@ Each directory entry is 60 words long:
@@          offset 0: flags
@@               bit-15: 1 - entry valid
@@               bit-14: 0 - 0 for XFMACC to accept the entry otherwise error-code 1025 is returned - set to 1 by XFMIO if the directory entry (sector allocation chains) got modified
@@               bit-13: 0 - 0 for XFMACC to accept the entry otherwise error-code 1026 is returned
@@          offset 1...8: directory name (max 16 chars long)
@@          offset 9...16: file name (max 16 chars long)
@@          offset 17...20: Creation date string (probably in MM/DD/YY format)
@@          offset 21...24: Creation time string (probably in HH:MM:SS format)
@@          offset 25: File size in QWORDS - high WORD
@@          offset 26: File size in QWORDS - low WORD
@@          offset 27:
@@          offset 28:
@@          offset 29:
@@          offset 30: number of sectors in the file
@@          offset 31: number of chains in this directory entry
@@          offset 32: first sector in first chain - each chain is a contiguous set of sectors allocated for this file
@@          offset 33: number of sectors in first chain
@@          ...
@@          offset 58: first sector in 14th chain
@@          offset 59: number of sectors in 14th chain
@@
@@ Each file handle is 68 words long:
@@          offset 0: directory entry sector number
@@          offset 1: directory entry sector offset
@@          offset 2: current sector index (not disk sector, but nth sector of file) ????
@@          offset 3:
@@          offset 4:
@@          offset 5:
@@          offset 6:
@@          offset 7:
@@          offset 8...67: corresponding directory entry

0x0000 (0x000000) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0001 (0x000002) 0x291C-       f:00024 d: 284 | OR[284] = A                   
0x0002 (0x000004) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0003 (0x000006) 0x291D-       f:00024 d: 285 | OR[285] = A                   

@ Allocate 2k buffer
0x0004 (0x000008) 0x101A-       f:00010 d:  26 | A = 26 (0x001A)               
0x0005 (0x00000A) 0x292A-       f:00024 d: 298 | OR[298] = A                   
0x0006 (0x00000C) 0x111D-       f:00010 d: 285 | A = 285 (0x011D)              
0x0007 (0x00000E) 0x292B-       f:00024 d: 299 | OR[299] = A                   
0x0008 (0x000010) 0x112A-       f:00010 d: 298 | A = 298 (0x012A)              
0x0009 (0x000012) 0x5800-       f:00054 d:   0 | B = A                         
0x000A (0x000014) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x000B (0x000016) 0x7C09-       f:00076 d:   9 | R = OR[9]                     
0x000C (0x000018) 0x8602-       f:00103 d:   2 | P = P + 2 (0x000E), A # 0     
0x000D (0x00001A) 0x700B-       f:00070 d:  11 | P = P + 11 (0x0018)           
0x000E (0x00001C) 0x1007-       f:00010 d:   7 | A = 7 (0x0007)                
0x000F (0x00001E) 0x292A-       f:00024 d: 298 | OR[298] = A                   
0x0010 (0x000020) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x0011 (0x000022) 0x292B-       f:00024 d: 299 | OR[299] = A                   
0x0012 (0x000024) 0x112A-       f:00010 d: 298 | A = 298 (0x012A)              
0x0013 (0x000026) 0x5800-       f:00054 d:   0 | B = A                         
0x0014 (0x000028) 0x1800-0x2518 f:00014 d:   0 | A = 9496 (0x2518)             
0x0016 (0x00002C) 0x7C09-       f:00076 d:   9 | R = OR[9]                     
0x0017 (0x00002E) 0x7213-       f:00071 d:  19 | P = P - 19 (0x0004)           

@ Read sector 5 into buffer
0x0018 (0x000030) 0x1005-       f:00010 d:   5 | A = 5 (0x0005)                
0x0019 (0x000032) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x001A (0x000034) 0x74C8-       f:00072 d: 200 | R = P + 200 (0x00E2)          

@ This is a loop here
0x001B (0x000036) 0x1008-       f:00010 d:   8 | A = 8 (0x0008)                
0x001C (0x000038) 0x291F-       f:00024 d: 287 | OR[287] = A                   
@ Loop until go over the buffer
0x001D (0x00003A) 0x211F-       f:00020 d: 287 | A = OR[287]                   
0x001E (0x00003C) 0x1E00-0x0800 f:00017 d:   0 | A = A - 2048 (0x0800)         
0x0020 (0x000040) 0x822A-       f:00101 d:  42 | P = P + 42 (0x004A), C # 0   
@ Read buffer index OR[287]
0x0021 (0x000042) 0x211D-       f:00020 d: 285 | A = OR[285]                   
0x0022 (0x000044) 0x251F-       f:00022 d: 287 | A = A + OR[287]               
0x0023 (0x000046) 0x2920-       f:00024 d: 288 | OR[288] = A                   
0x0024 (0x000048) 0x3120-       f:00030 d: 288 | A = (OR[288])                 
@ Make sure bit-15 is not set
0x0025 (0x00004A) 0x080F-       f:00004 d:  15 | A = A > 15 (0x000F)           
0x0026 (0x00004C) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x0027 (0x00004E) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x0028 (0x000050) 0x8602-       f:00103 d:   2 | P = P + 2 (0x002A), A # 0     
0x0029 (0x000052) 0x701E-       f:00070 d:  30 | P = P + 30 (0x0047)           
0x002A (0x000054) 0x743F-       f:00072 d:  63 | R = P + 63 (0x0069)           @ Compare directory entry file name OR[288]: directory entry, OR[282]: primary file name, OR[283]: secondary file name, return OR[290]: 0 - file name doesn't match 1 - file name(s) match
0x002B (0x000056) 0x2122-       f:00020 d: 290 | A = OR[290]                   
0x002C (0x000058) 0x8602-       f:00103 d:   2 | P = P + 2 (0x002E), A # 0    
0x002D (0x00005A) 0x701A-       f:00070 d:  26 | P = P + 26 (0x0047)           
@ File names match too: copy directory entry to output buffer, from offset 8
0x002E (0x00005C) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x002F (0x00005E) 0x1408-       f:00012 d:   8 | A = A + 8 (0x0008)            
0x0030 (0x000060) 0x2923-       f:00024 d: 291 | OR[291] = A                   
0x0031 (0x000062) 0x2120-       f:00020 d: 288 | A = OR[288]                   
0x0032 (0x000064) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x0033 (0x000066) 0x2123-       f:00020 d: 291 | A = OR[291]                   
0x0034 (0x000068) 0x290E-       f:00024 d: 270 | OR[270] = A                   
0x0035 (0x00006A) 0x103C-       f:00010 d:  60 | A = 60 (0x003C)               
0x0036 (0x00006C) 0x290F-       f:00024 d: 271 | OR[271] = A                   
0x0037 (0x00006E) 0x7006-       f:00070 d:   6 | P = P + 6 (0x003D)            
0x0038 (0x000070) 0x310D-       f:00030 d: 269 | A = (OR[269])                 
0x0039 (0x000072) 0x390E-       f:00034 d: 270 | (OR[270]) = A                 
0x003A (0x000074) 0x2D0D-       f:00026 d: 269 | OR[269] = OR[269] + 1         
0x003B (0x000076) 0x2D0E-       f:00026 d: 270 | OR[270] = OR[270] + 1         
0x003C (0x000078) 0x2F0F-       f:00027 d: 271 | OR[271] = OR[271] - 1         
0x003D (0x00007A) 0x210F-       f:00020 d: 271 | A = OR[271]                   
0x003E (0x00007C) 0x8E06-       f:00107 d:   6 | P = P - 6 (0x0038), A # 0    
@ Copy some other info into the return buffer too:
@ Sector number into first WORD
0x003F (0x00007E) 0x211E-       f:00020 d: 286 | A = OR[286]                   
0x0040 (0x000080) 0x3919-       f:00034 d: 281 | (OR[281]) = A                 
@ Sector offset where directory entry was found to second WORD
0x0041 (0x000082) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x0042 (0x000084) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x0043 (0x000086) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0044 (0x000088) 0x211F-       f:00020 d: 287 | A = OR[287]                   
0x0045 (0x00008A) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0046 (0x00008C) 0x700D-       f:00070 d:  13 | P = P + 13 (0x0053)           @ Return from overlay
@ Increment to next directory entry(?) and loop
0x0047 (0x00008E) 0x103C-       f:00010 d:  60 | A = 60 (0x003C)               
0x0048 (0x000090) 0x2B1F-       f:00025 d: 287 | OR[287] = A + OR[287]         
0x0049 (0x000092) 0x722C-       f:00071 d:  44 | P = P - 44 (0x001D)           
@ Haven't found the entry in this sector - read the first WORD of the current sector which links us to the next directory sector and (if it's non-0) continue looping
0x004A (0x000094) 0x311D-       f:00030 d: 285 | A = (OR[285])                 
0x004B (0x000096) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x004C (0x000098) 0x211E-       f:00020 d: 286 | A = OR[286]                   
0x004D (0x00009A) 0x8602-       f:00103 d:   2 | P = P + 2 (0x004F), A # 0    
0x004E (0x00009C) 0x7002-       f:00070 d:   2 | P = P + 2 (0x0050)            
0x004F (0x00009E) 0x7235-       f:00071 d:  53 | P = P - 53 (0x001A)           
@ Not found and reached end of directory structure: return error code 1092 from overlay
0x0050 (0x0000A0) 0x1800-0x0444 f:00014 d:   0 | A = 1092 (0x0444)             
0x0052 (0x0000A4) 0x291C-       f:00024 d: 284 | OR[284] = A                   
@ Free buffer if allocated
0x0053 (0x0000A6) 0x211D-       f:00020 d: 285 | A = OR[285]                   
0x0054 (0x0000A8) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0056), A # 0     
0x0055 (0x0000AA) 0x7009-       f:00070 d:   9 | P = P + 9 (0x005E)            
0x0056 (0x0000AC) 0x101B-       f:00010 d:  27 | A = 27 (0x001B)               
0x0057 (0x0000AE) 0x292A-       f:00024 d: 298 | OR[298] = A                   
0x0058 (0x0000B0) 0x211D-       f:00020 d: 285 | A = OR[285]                   
0x0059 (0x0000B2) 0x292B-       f:00024 d: 299 | OR[299] = A                   
0x005A (0x0000B4) 0x112A-       f:00010 d: 298 | A = 298 (0x012A)              
0x005B (0x0000B6) 0x5800-       f:00054 d:   0 | B = A                         
0x005C (0x0000B8) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x005D (0x0000BA) 0x7C09-       f:00076 d:   9 | R = OR[9]                     
@ Set return value from OR[284] and return from overlay
0x005E (0x0000BC) 0x2005-       f:00020 d:   5 | A = OR[5]                     
0x005F (0x0000BE) 0x1406-       f:00012 d:   6 | A = A + 6 (0x0006)            
0x0060 (0x0000C0) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0061 (0x0000C2) 0x211C-       f:00020 d: 284 | A = OR[284]                   
0x0062 (0x0000C4) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0063 (0x0000C6) 0x102A-       f:00010 d:  42 | A = 42 (0x002A)               
0x0064 (0x0000C8) 0x292A-       f:00024 d: 298 | OR[298] = A                   
0x0065 (0x0000CA) 0x112A-       f:00010 d: 298 | A = 298 (0x012A)              
0x0066 (0x0000CC) 0x5800-       f:00054 d:   0 | B = A                         
0x0067 (0x0000CE) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0068 (0x0000D0) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Compare directory entry file name
@@ Input:
@@      OR[288]: pointer to directory entry in memory
@@      OR[282]: pointer to primary file name to search for
@@      OR[283]: pointer to secondary file name to search for
@@
@@ Return:
@@      OR[290]: 0 - file name doesn't match
@@               1 - file name(s) match
@@
@@ NOTE: there's some detail around strings not being equally long. Maybe we return matches in some cases where the shorter string is an exact match of the beginning of the longer too...
@ OR[292] starts at offset 1
0x0069 (0x0000D2) 0x2120-       f:00020 d: 288 | A = OR[288]                   
0x006A (0x0000D4) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x006B (0x0000D6) 0x2924-       f:00024 d: 292 | OR[292] = A                   
@ OR[294] goes from 0 to 16
0x006C (0x0000D8) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x006D (0x0000DA) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x006E (0x0000DC) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x006F (0x0000DE) 0x1610-       f:00013 d:  16 | A = A - 16 (0x0010)           
0x0070 (0x0000E0) 0x8433-       f:00102 d:  51 | P = P + 51 (0x00A3), A = 0    
0x0071 (0x0000E2) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x0072 (0x0000E4) 0x0801-       f:00004 d:   1 | A = A > 1 (0x0001)            
0x0073 (0x0000E6) 0x251A-       f:00022 d: 282 | A = A + OR[282]               
0x0074 (0x0000E8) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x0075 (0x0000EA) 0x310D-       f:00030 d: 269 | A = (OR[269])                 
0x0076 (0x0000EC) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x0077 (0x0000EE) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x0078 (0x0000F0) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x0079 (0x0000F2) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x007A (0x0000F4) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x007B (0x0000F6) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x007C (0x0000F8) 0x8604-       f:00103 d:   4 | P = P + 4 (0x0080), A # 0    
0x007D (0x0000FA) 0x210D-       f:00020 d: 269 | A = OR[269]                   
0x007E (0x0000FC) 0x0808-       f:00004 d:   8 | A = A > 8 (0x0008)            
0x007F (0x0000FE) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x0080 (0x000100) 0x210D-       f:00020 d: 269 | A = OR[269]                   
0x0081 (0x000102) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x0082 (0x000104) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x0083 (0x000106) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x0084 (0x000108) 0x0801-       f:00004 d:   1 | A = A > 1 (0x0001)            
0x0085 (0x00010A) 0x2524-       f:00022 d: 292 | A = A + OR[292]               
0x0086 (0x00010C) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x0087 (0x00010E) 0x310D-       f:00030 d: 269 | A = (OR[269])                 
0x0088 (0x000110) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x0089 (0x000112) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x008A (0x000114) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x008B (0x000116) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x008C (0x000118) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x008D (0x00011A) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x008E (0x00011C) 0x8604-       f:00103 d:   4 | P = P + 4 (0x0092), A # 0    
0x008F (0x00011E) 0x210D-       f:00020 d: 269 | A = OR[269]                   
0x0090 (0x000120) 0x0808-       f:00004 d:   8 | A = A > 8 (0x0008)            
0x0091 (0x000122) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x0092 (0x000124) 0x210D-       f:00020 d: 269 | A = OR[269]                   
0x0093 (0x000126) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x0094 (0x000128) 0x2928-       f:00024 d: 296 | OR[296] = A                   
0x0095 (0x00012A) 0x2127-       f:00020 d: 295 | A = OR[295]                   
0x0096 (0x00012C) 0x2728-       f:00023 d: 296 | A = A - OR[296]               
0x0097 (0x00012E) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0099), A # 0    
0x0098 (0x000130) 0x7005-       f:00070 d:   5 | P = P + 5 (0x009D)            
0x0099 (0x000132) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x009A (0x000134) 0x2922-       f:00024 d: 290 | OR[290] = A                   
0x009B (0x000136) 0x0200-       f:00001 d:   0 | EXIT                          
0x009C (0x000138) 0x7005-       f:00070 d:   5 | P = P + 5 (0x00A1)            
0x009D (0x00013A) 0x2127-       f:00020 d: 295 | A = OR[295]                   
0x009E (0x00013C) 0x8402-       f:00102 d:   2 | P = P + 2 (0x00A0), A = 0     
0x009F (0x00013E) 0x7002-       f:00070 d:   2 | P = P + 2 (0x00A1)            
0x00A0 (0x000140) 0x7003-       f:00070 d:   3 | P = P + 3 (0x00A3)            
0x00A1 (0x000142) 0x2D26-       f:00026 d: 294 | OR[294] = OR[294] + 1         
0x00A2 (0x000144) 0x7234-       f:00071 d:  52 | P = P - 52 (0x006E)           
@ File name matches, now look at the secondary file name???
0x00A3 (0x000146) 0x2120-       f:00020 d: 288 | A = OR[288]                   
0x00A4 (0x000148) 0x1409-       f:00012 d:   9 | A = A + 9 (0x0009)            
0x00A5 (0x00014A) 0x2925-       f:00024 d: 293 | OR[293] = A                   
0x00A6 (0x00014C) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00A7 (0x00014E) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x00A8 (0x000150) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x00A9 (0x000152) 0x1610-       f:00013 d:  16 | A = A - 16 (0x0010)           
0x00AA (0x000154) 0x8435-       f:00102 d:  53 | P = P + 53 (0x00DF), A = 0    
0x00AB (0x000156) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x00AC (0x000158) 0x0801-       f:00004 d:   1 | A = A > 1 (0x0001)            
0x00AD (0x00015A) 0x251B-       f:00022 d: 283 | A = A + OR[283]               
0x00AE (0x00015C) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x00AF (0x00015E) 0x310D-       f:00030 d: 269 | A = (OR[269])                 
0x00B0 (0x000160) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x00B1 (0x000162) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x00B2 (0x000164) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x00B3 (0x000166) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00B4 (0x000168) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00B5 (0x00016A) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x00B6 (0x00016C) 0x8604-       f:00103 d:   4 | P = P + 4 (0x00BA), A # 0    
0x00B7 (0x00016E) 0x210D-       f:00020 d: 269 | A = OR[269]                   
0x00B8 (0x000170) 0x0808-       f:00004 d:   8 | A = A > 8 (0x0008)            
0x00B9 (0x000172) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x00BA (0x000174) 0x210D-       f:00020 d: 269 | A = OR[269]                   
0x00BB (0x000176) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x00BC (0x000178) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x00BD (0x00017A) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x00BE (0x00017C) 0x0801-       f:00004 d:   1 | A = A > 1 (0x0001)            
0x00BF (0x00017E) 0x2525-       f:00022 d: 293 | A = A + OR[293]               
0x00C0 (0x000180) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x00C1 (0x000182) 0x310D-       f:00030 d: 269 | A = (OR[269])                 
0x00C2 (0x000184) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x00C3 (0x000186) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x00C4 (0x000188) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x00C5 (0x00018A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00C6 (0x00018C) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00C7 (0x00018E) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x00C8 (0x000190) 0x8604-       f:00103 d:   4 | P = P + 4 (0x00CC), A # 0    
0x00C9 (0x000192) 0x210D-       f:00020 d: 269 | A = OR[269]                   
0x00CA (0x000194) 0x0808-       f:00004 d:   8 | A = A > 8 (0x0008)            
0x00CB (0x000196) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x00CC (0x000198) 0x210D-       f:00020 d: 269 | A = OR[269]                   
0x00CD (0x00019A) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x00CE (0x00019C) 0x2928-       f:00024 d: 296 | OR[296] = A                   
0x00CF (0x00019E) 0x2127-       f:00020 d: 295 | A = OR[295]                   
0x00D0 (0x0001A0) 0x2728-       f:00023 d: 296 | A = A - OR[296]               
0x00D1 (0x0001A2) 0x8602-       f:00103 d:   2 | P = P + 2 (0x00D3), A # 0    
0x00D2 (0x0001A4) 0x7005-       f:00070 d:   5 | P = P + 5 (0x00D7)            
0x00D3 (0x0001A6) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00D4 (0x0001A8) 0x2922-       f:00024 d: 290 | OR[290] = A                   
0x00D5 (0x0001AA) 0x0200-       f:00001 d:   0 | EXIT                          
0x00D6 (0x0001AC) 0x7007-       f:00070 d:   7 | P = P + 7 (0x00DD)            
0x00D7 (0x0001AE) 0x2127-       f:00020 d: 295 | A = OR[295]                   
0x00D8 (0x0001B0) 0x8402-       f:00102 d:   2 | P = P + 2 (0x00DA), A = 0     
0x00D9 (0x0001B2) 0x7004-       f:00070 d:   4 | P = P + 4 (0x00DD)            
0x00DA (0x0001B4) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x00DB (0x0001B6) 0x2922-       f:00024 d: 290 | OR[290] = A                   
0x00DC (0x0001B8) 0x0200-       f:00001 d:   0 | EXIT                          
0x00DD (0x0001BA) 0x2D26-       f:00026 d: 294 | OR[294] = OR[294] + 1         
0x00DE (0x0001BC) 0x7236-       f:00071 d:  54 | P = P - 54 (0x00A8)           

0x00DF (0x0001BE) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x00E0 (0x0001C0) 0x2922-       f:00024 d: 290 | OR[290] = A                   
0x00E1 (0x0001C2) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ call XDISK to read buffer
@@ Input:
@@     OR[285]: buffer
@@     OR[286]: sector
@@
0x00E2 (0x0001C4) 0x1028-       f:00010 d:  40 | A = 40 (0x0028)               
0x00E3 (0x0001C6) 0x292A-       f:00024 d: 298 | OR[298] = A                   
0x00E4 (0x0001C8) 0x1800-0x00A6 f:00014 d:   0 | A = 166 (0x00A6)              
0x00E6 (0x0001CC) 0x292B-       f:00024 d: 299 | OR[299] = A                   
0x00E7 (0x0001CE) 0x2118-       f:00020 d: 280 | A = OR[280]                   @ Device desc. address
0x00E8 (0x0001D0) 0x292C-       f:00024 d: 300 | OR[300] = A                   
0x00E9 (0x0001D2) 0x1004-       f:00010 d:   4 | A = 4 (0x0004)                @ Function code 4 - read
0x00EA (0x0001D4) 0x292D-       f:00024 d: 301 | OR[301] = A                   
0x00EB (0x0001D6) 0x211D-       f:00020 d: 285 | A = OR[285]                   @ Buffer address
0x00EC (0x0001D8) 0x292E-       f:00024 d: 302 | OR[302] = A                   
0x00ED (0x0001DA) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                @ ???
0x00EE (0x0001DC) 0x292F-       f:00024 d: 303 | OR[303] = A                   
0x00EF (0x0001DE) 0x211E-       f:00020 d: 286 | A = OR[286]                   @ Sector number
0x00F0 (0x0001E0) 0x2930-       f:00024 d: 304 | OR[304] = A                   
0x00F1 (0x0001E2) 0x112A-       f:00010 d: 298 | A = 298 (0x012A)              
0x00F2 (0x0001E4) 0x5800-       f:00054 d:   0 | B = A                         
0x00F3 (0x0001E6) 0x1800-0x2518 f:00014 d:   0 | A = 9496 (0x2518)             
0x00F5 (0x0001EA) 0x7C09-       f:00076 d:   9 | R = OR[9]                     
0x00F6 (0x0001EC) 0x291C-       f:00024 d: 284 | OR[284] = A                   
0x00F7 (0x0001EE) 0x211C-       f:00020 d: 284 | A = OR[284]                   
0x00F8 (0x0001F0) 0x8602-       f:00103 d:   2 | P = P + 2 (0x00FA), A # 0    
0x00F9 (0x0001F2) 0x7002-       f:00070 d:   2 | P = P + 2 (0x00FB)            
0x00FA (0x0001F4) 0x72A7-       f:00071 d: 167 | P = P - 167 (0x0053)          
0x00FB (0x0001F6) 0x0200-       f:00001 d:   0 | EXIT                          
0x00FC (0x0001F8) 0x0000-       f:00000 d:   0 | PASS                          
0x00FD (0x0001FA) 0x0000-       f:00000 d:   0 | PASS                          
0x00FE (0x0001FC) 0x0000-       f:00000 d:   0 | PASS                          
0x00FF (0x0001FE) 0x0000-       f:00000 d:   0 | PASS                          
