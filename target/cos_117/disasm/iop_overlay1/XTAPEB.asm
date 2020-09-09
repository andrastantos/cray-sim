@ Input:
@ OR[280] - device desc
@ OR[281] - function code?

@ Command code	Status bit	Error string code	Error handler address		1st string	2nd string				
@ 70			0x4000		0x0288				0x0137						0x061F		0x0601			0x0c32		0x0bf6			@?S: DATA LATE						DURING ?S ?SRECOVERED
@ 70			0x0400		0x028b				0x0137						0x063F		0x0601			0x0c72		0x0bf10			@?S: DATA PARITY ERROR				DURING ?S ?SRECOVERED
@ 70			0x0200		0x0286				0x0285						0x059A		0x0601			0x0b28		0x0bf12			@?S: UNKOWN ERROR					DURING ?S ?SRECOVERED
@ 70			0x0100		0x028f				0x0285						0x066C		0x0601			0x0ccc		0x0bf7			@?S: END OF FILE					DURING ?S ?SRECOVERED
@ 70			0x0080		0x028c				0x0285						0x064B		0x0601			0x0c8a		0x0bf11			@?S: LOAD POINT DETECTED			DURING ?S ?SRECOVERED
@ 70			0x0020		0x0290				0x0285						0x0675		0x0601			0x0cde		0x0bf8			@?S: BAD TYPE						DURING ?S ?SRECOVERED
@ 70			0x0002		0x0289				0x0285						0x0627		0x0601			0x0c42		0x0bf9			@?S: WORD COUNT ERROR				DURING ?S ?SRECOVERED
@ all						0x0291											0x05A4		0x0000			0x0B3C						@?S: ERROR RECOVERY LIMIT REACHED	
@ 64						0x0202				0x023f						0x057D		0x0000			0x0AEE						@?S: NOT READY	
@ 65						0x0285				0x0285						0x058F		0x0601			0x0B12		0x0bf12			@?S: CHANNEL TIMEOUT				DURING ?S ?SRECOVERED
@ 66						0x0284				0x0285						0x0585		0x0601			0x0AFE		0x0bf12			@?S: DEVICE TIMEOUT					DURING ?S ?SRECOVERED
@ 67						0x0287				0x023f						0x0615		0x0000			0x0C1E						@?S: NO WRITE RING			
@ 68						0x028a				0x0137 or 0x0285			0x0632		0x0601			0x0C58		0x0bf12			@?S: RECORD LENGTH ERROR			DURING ?S ?SRECOVERED
@ 69						0x028d				0x0285						0x0658		0x0601			0x0CA4		0x0bf12			@?S: POSITION LOST					DURING ?S ?SRECOVERED
@				0x2000		<-- if this is set on entering XTAPEC, than it waits until it clears
@               0x0001      <-- I think it is the 'done' bit. If it's not set on completion of a command it results in a 'NOT READY' error
@               0x5422      <-- if MSB is set, with any of these lower bits set, we call XTAPEB with command code 70 to figure out what to print
@				0x0004		<-- if this bit is set, XTAPEA will call XTAPEB with function code 67

@ OR[282] - return value
@ OR[283] - 0
@ OR[284] - device address
@ OR[285] - high-byte of desc. entry 20
@ OR[286] - some sort of error code???
@
@ Return value
@	0 - OK
@	2 - Recovery limit reached
@	1 - ??? <-- special cases from siblings, but can also be the return value
@	4 - ???
@   8 - ERROR - as in, this is the default

0x0000 (0x000000) 0x1008-       f:00010 d:   8 | A = 8 (0x0008)                
0x0001 (0x000002) 0x291A-       f:00024 d: 282 | OR[282] = A                   
0x0002 (0x000004) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0003 (0x000006) 0x291B-       f:00024 d: 283 | OR[283] = A                   
0x0004 (0x000008) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0005 (0x00000A) 0x1402-       f:00012 d:   2 | A = A + 2 (0x0002)            
0x0006 (0x00000C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0007 (0x00000E) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0008 (0x000010) 0x291C-       f:00024 d: 284 | OR[284] = A                   
@ Load high-byte of desc. entry 20 into OR[285], load operation string appropriately
@ 40: write
@ 0: read
@ any other: position
0x0009 (0x000012) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x000A (0x000014) 0x1414-       f:00012 d:  20 | A = A + 20 (0x0014)           
0x000B (0x000016) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x000C (0x000018) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x000D (0x00001A) 0x0808-       f:00004 d:   8 | A = A > 8 (0x0008)            
0x000E (0x00001C) 0x291D-       f:00024 d: 285 | OR[285] = A                   
0x000F (0x00001E) 0x211D-       f:00020 d: 285 | A = OR[285]                   
0x0010 (0x000020) 0x1628-       f:00013 d:  40 | A = A - 40 (0x0028)           
0x0011 (0x000022) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0013), A = 0     
0x0012 (0x000024) 0x7005-       f:00070 d:   5 | P = P + 5 (0x0017)            
0x0013 (0x000026) 0x1800-0x0314 f:00014 d:   0 | A = 788 (0x0314)              
0x0015 (0x00002A) 0x291F-       f:00024 d: 287 | OR[287] = A                   @ String address: WRITE
0x0016 (0x00002C) 0x700B-       f:00070 d:  11 | P = P + 11 (0x0021)           
0x0017 (0x00002E) 0x211D-       f:00020 d: 285 | A = OR[285]                   
0x0018 (0x000030) 0x8402-       f:00102 d:   2 | P = P + 2 (0x001A), A = 0     
0x0019 (0x000032) 0x7005-       f:00070 d:   5 | P = P + 5 (0x001E)            
0x001A (0x000034) 0x1800-0x0318 f:00014 d:   0 | A = 792 (0x0318)              
0x001C (0x000038) 0x291F-       f:00024 d: 287 | OR[287] = A                   @ String address: READ
0x001D (0x00003A) 0x7004-       f:00070 d:   4 | P = P + 4 (0x0021)            
0x001E (0x00003C) 0x1800-0x031C f:00014 d:   0 | A = 796 (0x031C)              
0x0020 (0x000040) 0x291F-       f:00024 d: 287 | OR[287] = A                   @ String address: POSITION
@ Increment low-order byte of desc. entry 20
0x0021 (0x000042) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0022 (0x000044) 0x1414-       f:00012 d:  20 | A = A + 20 (0x0014)           
0x0023 (0x000046) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x0024 (0x000048) 0x310D-       f:00030 d: 269 | A = (OR[269])                 
0x0025 (0x00004A) 0x290E-       f:00024 d: 270 | OR[270] = A                   
0x0026 (0x00004C) 0x210E-       f:00020 d: 270 | A = OR[270]                   
0x0027 (0x00004E) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x0028 (0x000050) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x0029 (0x000052) 0x290F-       f:00024 d: 271 | OR[271] = A                   
0x002A (0x000054) 0x210F-       f:00020 d: 271 | A = OR[271]                   
0x002B (0x000056) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x002C (0x000058) 0x290F-       f:00024 d: 271 | OR[271] = A                   
0x002D (0x00005A) 0x210E-       f:00020 d: 270 | A = OR[270]                   
0x002E (0x00005C) 0x1A00-0xFF00 f:00015 d:   0 | A = A & 65280 (0xFF00)        
0x0030 (0x000060) 0x250F-       f:00022 d: 271 | A = A + OR[271]               
0x0031 (0x000062) 0x290E-       f:00024 d: 270 | OR[270] = A                   
0x0032 (0x000064) 0x390D-       f:00034 d: 269 | (OR[269]) = A                 
@ Check if the low-order byte in entry 20 (we have just incremented) is 3, print error message:  '@?S: ERROR RECOVERY LIMIT REACHED' and return with error code 2
@ If not, parse function code in OR[281]
0x0033 (0x000066) 0x210F-       f:00020 d: 271 | A = OR[271]                   
0x0034 (0x000068) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0035 (0x00006A) 0x1414-       f:00012 d:  20 | A = A + 20 (0x0014)           
0x0036 (0x00006C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0037 (0x00006E) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0038 (0x000070) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x0039 (0x000072) 0x1603-       f:00013 d:   3 | A = A - 3 (0x0003)            
0x003A (0x000074) 0x8402-       f:00102 d:   2 | P = P + 2 (0x003C), A = 0     
0x003B (0x000076) 0x7009-       f:00070 d:   9 | P = P + 9 (0x0044)            
0x003C (0x000078) 0x1800-0x0291 f:00014 d:   0 | A = 657 (0x0291)              
0x003E (0x00007C) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x003F (0x00007E) 0x7E03-0x028F f:00077 d:   3 | R = OR[3]+655 (0x028F)        
0x0041 (0x000082) 0x1002-       f:00010 d:   2 | A = 2 (0x0002)                
0x0042 (0x000084) 0x291A-       f:00024 d: 282 | OR[282] = A                   
0x0043 (0x000086) 0x70C4-       f:00070 d: 196 | P = P + 196 (0x0107)          

@ Command 64
0x0044 (0x000088) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x0045 (0x00008A) 0x1640-       f:00013 d:  64 | A = A - 64 (0x0040)           
0x0046 (0x00008C) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0048), A = 0     
0x0047 (0x00008E) 0x7009-       f:00070 d:   9 | P = P + 9 (0x0050)            
0x0048 (0x000090) 0x1800-0x0202 f:00014 d:   0 | A = 514 (0x0202)              
0x004A (0x000094) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x004B (0x000096) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x004C (0x000098) 0x2923-       f:00024 d: 291 | OR[291] = A                   
0x004D (0x00009A) 0x7E03-0x023F f:00077 d:   3 | R = OR[3]+575 (0x023F)        
0x004F (0x00009E) 0x70B8-       f:00070 d: 184 | P = P + 184 (0x0107)          
@ Command 65
0x0050 (0x0000A0) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x0051 (0x0000A2) 0x1641-       f:00013 d:  65 | A = A - 65 (0x0041)           
0x0052 (0x0000A4) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0054), A = 0     
0x0053 (0x0000A6) 0x7007-       f:00070 d:   7 | P = P + 7 (0x005A)            
0x0054 (0x0000A8) 0x1800-0x0285 f:00014 d:   0 | A = 645 (0x0285)              
0x0056 (0x0000AC) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x0057 (0x0000AE) 0x7A03-0x0285 f:00075 d:   3 | P = OR[3]+645 (0x0285)        
0x0059 (0x0000B2) 0x70AE-       f:00070 d: 174 | P = P + 174 (0x0107)          
@ Command 66
0x005A (0x0000B4) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x005B (0x0000B6) 0x1642-       f:00013 d:  66 | A = A - 66 (0x0042)           
0x005C (0x0000B8) 0x8402-       f:00102 d:   2 | P = P + 2 (0x005E), A = 0     
0x005D (0x0000BA) 0x7007-       f:00070 d:   7 | P = P + 7 (0x0064)            
0x005E (0x0000BC) 0x1800-0x0284 f:00014 d:   0 | A = 644 (0x0284)              
0x0060 (0x0000C0) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x0061 (0x0000C2) 0x7A03-0x0285 f:00075 d:   3 | P = OR[3]+645 (0x0285)        
0x0063 (0x0000C6) 0x70A4-       f:00070 d: 164 | P = P + 164 (0x0107)          
@ Command 67
0x0064 (0x0000C8) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x0065 (0x0000CA) 0x1643-       f:00013 d:  67 | A = A - 67 (0x0043)           
0x0066 (0x0000CC) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0068), A = 0     
0x0067 (0x0000CE) 0x7009-       f:00070 d:   9 | P = P + 9 (0x0070)            
0x0068 (0x0000D0) 0x1800-0x0287 f:00014 d:   0 | A = 647 (0x0287)              
0x006A (0x0000D4) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x006B (0x0000D6) 0x1004-       f:00010 d:   4 | A = 4 (0x0004)                
0x006C (0x0000D8) 0x2923-       f:00024 d: 291 | OR[291] = A                   
0x006D (0x0000DA) 0x7E03-0x023F f:00077 d:   3 | R = OR[3]+575 (0x023F)        
0x006F (0x0000DE) 0x7098-       f:00070 d: 152 | P = P + 152 (0x0107)          
@ Command 68
0x0070 (0x0000E0) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x0071 (0x0000E2) 0x1644-       f:00013 d:  68 | A = A - 68 (0x0044)           
0x0072 (0x0000E4) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0074), A = 0     
0x0073 (0x0000E6) 0x700D-       f:00070 d:  13 | P = P + 13 (0x0080)           
0x0074 (0x0000E8) 0x1800-0x028A f:00014 d:   0 | A = 650 (0x028A)              
0x0076 (0x0000EC) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x0077 (0x0000EE) 0x211D-       f:00020 d: 285 | A = OR[285]                   
0x0078 (0x0000F0) 0x1628-       f:00013 d:  40 | A = A - 40 (0x0028)           
0x0079 (0x0000F2) 0x8402-       f:00102 d:   2 | P = P + 2 (0x007B), A = 0     
0x007A (0x0000F4) 0x7003-       f:00070 d:   3 | P = P + 3 (0x007D)            
0x007B (0x0000F6) 0x70BC-       f:00070 d: 188 | P = P + 188 (0x0137)          
0x007C (0x0000F8) 0x7003-       f:00070 d:   3 | P = P + 3 (0x007F)            
0x007D (0x0000FA) 0x7A03-0x0285 f:00075 d:   3 | P = OR[3]+645 (0x0285)        
0x007F (0x0000FE) 0x7088-       f:00070 d: 136 | P = P + 136 (0x0107)          
@ Command 69
0x0080 (0x000100) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x0081 (0x000102) 0x1645-       f:00013 d:  69 | A = A - 69 (0x0045)           
0x0082 (0x000104) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0084), A = 0     
0x0083 (0x000106) 0x7007-       f:00070 d:   7 | P = P + 7 (0x008A)            
0x0084 (0x000108) 0x1800-0x028D f:00014 d:   0 | A = 653 (0x028D)              
0x0086 (0x00010C) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x0087 (0x00010E) 0x7A03-0x0285 f:00075 d:   3 | P = OR[3]+645 (0x0285)        
0x0089 (0x000112) 0x707E-       f:00070 d: 126 | P = P + 126 (0x0107)          
@ Command 70
0x008A (0x000114) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x008B (0x000116) 0x1646-       f:00013 d:  70 | A = A - 70 (0x0046)           
0x008C (0x000118) 0x8402-       f:00102 d:   2 | P = P + 2 (0x008E), A = 0     
0x008D (0x00011A) 0x7078-       f:00070 d: 120 | P = P + 120 (0x0105)          
@@ Parse status code in RegA (from device desc.)
0x008E (0x00011C) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x008F (0x00011E) 0x140E-       f:00012 d:  14 | A = A + 14 (0x000E)           
0x0090 (0x000120) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0091 (0x000122) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0092 (0x000124) 0x2926-       f:00024 d: 294 | OR[294] = A 
@ Test bit 14, if set -> call 0x0137 with error code 0x0288 (DATA LATE)
0x0093 (0x000126) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x0094 (0x000128) 0x1A00-0x4000 f:00015 d:   0 | A = A & 16384 (0x4000)        
0x0096 (0x00012C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0097 (0x00012E) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0098 (0x000130) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x0099 (0x000132) 0x8602-       f:00103 d:   2 | P = P + 2 (0x009B), A # 0     
0x009A (0x000134) 0x7005-       f:00070 d:   5 | P = P + 5 (0x009F)            
0x009B (0x000136) 0x1800-0x0288 f:00014 d:   0 | A = 648 (0x0288)              
0x009D (0x00013A) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x009E (0x00013C) 0x7099-       f:00070 d: 153 | P = P + 153 (0x0137)          
@ Test bit 8
0x009F (0x00013E) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x00A0 (0x000140) 0x1300-       f:00011 d: 256 | A = A & 256 (0x0100)          
0x00A1 (0x000142) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00A2 (0x000144) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00A3 (0x000146) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x00A4 (0x000148) 0x8602-       f:00103 d:   2 | P = P + 2 (0x00A6), A # 0     
0x00A5 (0x00014A) 0x7013-       f:00070 d:  19 | P = P + 19 (0x00B8)           
0x00A6 (0x00014C) 0x211D-       f:00020 d: 285 | A = OR[285]                   @ Test if a write operation was in progress
0x00A7 (0x00014E) 0x1628-       f:00013 d:  40 | A = A - 40 (0x0028)           
0x00A8 (0x000150) 0x8402-       f:00102 d:   2 | P = P + 2 (0x00AA), A = 0     
0x00A9 (0x000152) 0x7007-       f:00070 d:   7 | P = P + 7 (0x00B0)            
0x00AA (0x000154) 0x1800-0x028F f:00014 d:   0 | A = 655 (0x028F)              @ Write operation: load error code 0x028f (End of File)
0x00AC (0x000158) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x00AD (0x00015A) 0x7A03-0x0285 f:00075 d:   3 | P = OR[3]+645 (0x0285)        @ Print error message
0x00AF (0x00015E) 0x7009-       f:00070 d:   9 | P = P + 9 (0x00B8)            
0x00B0 (0x000160) 0x1800-0x1000 f:00014 d:   0 | A = 4096 (0x1000)             @ Read operation:
0x00B2 (0x000164) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x00B3 (0x000166) 0x7E03-0x02FC f:00077 d:   3 | R = OR[3]+764 (0x02FC)        @ Call XTAPED function 4096 and update return code in OR[282], and return from overlay
0x00B5 (0x00016A) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x00B6 (0x00016C) 0x291A-       f:00024 d: 282 | OR[282] = A                   
0x00B7 (0x00016E) 0x7050-       f:00070 d:  80 | P = P + 80 (0x0107)           
0x00B8 (0x000170) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x00B9 (0x000172) 0x1220-       f:00011 d:  32 | A = A & 32 (0x0020)           
0x00BA (0x000174) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00BB (0x000176) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00BC (0x000178) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x00BD (0x00017A) 0x8602-       f:00103 d:   2 | P = P + 2 (0x00BF), A # 0     
0x00BE (0x00017C) 0x7006-       f:00070 d:   6 | P = P + 6 (0x00C4)            
0x00BF (0x00017E) 0x1800-0x0290 f:00014 d:   0 | A = 656 (0x0290)              
0x00C1 (0x000182) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x00C2 (0x000184) 0x7A03-0x0285 f:00075 d:   3 | P = OR[3]+645 (0x0285)        
0x00C4 (0x000188) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x00C5 (0x00018A) 0x1202-       f:00011 d:   2 | A = A & 2 (0x0002)            
0x00C6 (0x00018C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00C7 (0x00018E) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00C8 (0x000190) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x00C9 (0x000192) 0x8602-       f:00103 d:   2 | P = P + 2 (0x00CB), A # 0     
0x00CA (0x000194) 0x7006-       f:00070 d:   6 | P = P + 6 (0x00D0)            
0x00CB (0x000196) 0x1800-0x0289 f:00014 d:   0 | A = 649 (0x0289)              
0x00CD (0x00019A) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x00CE (0x00019C) 0x7A03-0x0285 f:00075 d:   3 | P = OR[3]+645 (0x0285)        
0x00D0 (0x0001A0) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x00D1 (0x0001A2) 0x1A00-0x0400 f:00015 d:   0 | A = A & 1024 (0x0400)         
0x00D3 (0x0001A6) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00D4 (0x0001A8) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00D5 (0x0001AA) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x00D6 (0x0001AC) 0x8602-       f:00103 d:   2 | P = P + 2 (0x00D8), A # 0     
0x00D7 (0x0001AE) 0x7005-       f:00070 d:   5 | P = P + 5 (0x00DC)            
0x00D8 (0x0001B0) 0x1800-0x028B f:00014 d:   0 | A = 651 (0x028B)              
0x00DA (0x0001B4) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x00DB (0x0001B6) 0x705C-       f:00070 d:  92 | P = P + 92 (0x0137)           
0x00DC (0x0001B8) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x00DD (0x0001BA) 0x1280-       f:00011 d: 128 | A = A & 128 (0x0080)          
0x00DE (0x0001BC) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00DF (0x0001BE) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00E0 (0x0001C0) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x00E1 (0x0001C2) 0x8602-       f:00103 d:   2 | P = P + 2 (0x00E3), A # 0     
0x00E2 (0x0001C4) 0x7006-       f:00070 d:   6 | P = P + 6 (0x00E8)            
0x00E3 (0x0001C6) 0x1800-0x028C f:00014 d:   0 | A = 652 (0x028C)              
0x00E5 (0x0001CA) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x00E6 (0x0001CC) 0x7A03-0x0285 f:00075 d:   3 | P = OR[3]+645 (0x0285)        
0x00E8 (0x0001D0) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x00E9 (0x0001D2) 0x1A00-0x0200 f:00015 d:   0 | A = A & 512 (0x0200)          
0x00EB (0x0001D6) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00EC (0x0001D8) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00ED (0x0001DA) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x00EE (0x0001DC) 0x8602-       f:00103 d:   2 | P = P + 2 (0x00F0), A # 0     
0x00EF (0x0001DE) 0x7010-       f:00070 d:  16 | P = P + 16 (0x00FF)           
0x00F0 (0x0001E0) 0x211D-       f:00020 d: 285 | A = OR[285]                   
0x00F1 (0x0001E2) 0x1628-       f:00013 d:  40 | A = A - 40 (0x0028)           
0x00F2 (0x0001E4) 0x8402-       f:00102 d:   2 | P = P + 2 (0x00F4), A = 0     
0x00F3 (0x0001E6) 0x7009-       f:00070 d:   9 | P = P + 9 (0x00FC)            
0x00F4 (0x0001E8) 0x1800-0x1001 f:00014 d:   0 | A = 4097 (0x1001)             
0x00F6 (0x0001EC) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x00F7 (0x0001EE) 0x7E03-0x02FC f:00077 d:   3 | R = OR[3]+764 (0x02FC)        @ Call XTAPED function 4097 and update return code in OR[282]
0x00F9 (0x0001F2) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x00FA (0x0001F4) 0x291A-       f:00024 d: 282 | OR[282] = A                   
0x00FB (0x0001F6) 0x7003-       f:00070 d:   3 | P = P + 3 (0x00FE)            
0x00FC (0x0001F8) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00FD (0x0001FA) 0x291A-       f:00024 d: 282 | OR[282] = A                   
0x00FE (0x0001FC) 0x7009-       f:00070 d:   9 | P = P + 9 (0x0107)            
0x00FF (0x0001FE) 0x1800-0x0286 f:00014 d:   0 | A = 646 (0x0286)              
0x0101 (0x000202) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x0102 (0x000204) 0x7A03-0x0285 f:00075 d:   3 | P = OR[3]+645 (0x0285)        
0x0104 (0x000208) 0x7003-       f:00070 d:   3 | P = P + 3 (0x0107)            
@ Bad command - fatal
0x0105 (0x00020A) 0x7C34-       f:00076 d:  52 | R = OR[52]                    
0x0106 (0x00020C) 0x0000-       f:00000 d:   0 | PASS                          
@ Common part of command handling
0x0107 (0x00020E) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0108 (0x000210) 0x1414-       f:00012 d:  20 | A = A + 20 (0x0014)           
0x0109 (0x000212) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x010A (0x000214) 0x310D-       f:00030 d: 269 | A = (OR[269])                 
0x010B (0x000216) 0x290E-       f:00024 d: 270 | OR[270] = A                   
0x010C (0x000218) 0x210E-       f:00020 d: 270 | A = OR[270]                   
0x010D (0x00021A) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x010E (0x00021C) 0x1601-       f:00013 d:   1 | A = A - 1 (0x0001)            
0x010F (0x00021E) 0x290F-       f:00024 d: 271 | OR[271] = A                   
0x0110 (0x000220) 0x210F-       f:00020 d: 271 | A = OR[271]                   
0x0111 (0x000222) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x0112 (0x000224) 0x290F-       f:00024 d: 271 | OR[271] = A                   
0x0113 (0x000226) 0x210E-       f:00020 d: 270 | A = OR[270]                   
0x0114 (0x000228) 0x1A00-0xFF00 f:00015 d:   0 | A = A & 65280 (0xFF00)        
0x0116 (0x00022C) 0x250F-       f:00022 d: 271 | A = A + OR[271]               
0x0117 (0x00022E) 0x290E-       f:00024 d: 270 | OR[270] = A                   
0x0118 (0x000230) 0x390D-       f:00034 d: 269 | (OR[269]) = A                 
0x0119 (0x000232) 0x210F-       f:00020 d: 271 | A = OR[271]                   
0x011A (0x000234) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x011B (0x000236) 0x1411-       f:00012 d:  17 | A = A + 17 (0x0011)           
0x011C (0x000238) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x011D (0x00023A) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x011E (0x00023C) 0x0E01-       f:00007 d:   1 | A = A << 1 (0x0001)           
0x011F (0x00023E) 0x0A08-       f:00005 d:   8 | A = A < 8 (0x0008)            
0x0120 (0x000240) 0x1400-       f:00012 d:   0 | A = A + 0 (0x0000)            
0x0121 (0x000242) 0x0C09-       f:00006 d:   9 | A = A >> 9 (0x0009)           
0x0122 (0x000244) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0123 (0x000246) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0124 (0x000248) 0x1411-       f:00012 d:  17 | A = A + 17 (0x0011)           
0x0125 (0x00024A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0126 (0x00024C) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0127 (0x00024E) 0x1A00-0xFF00 f:00015 d:   0 | A = A & 65280 (0xFF00)        
0x0129 (0x000252) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x012A (0x000254) 0x1002-       f:00010 d:   2 | A = 2 (0x0002)                
0x012B (0x000256) 0xDE0F-       f:00157 d:  15 | io 0017 (EXB), fn017           | Send control
@ Set return value (from OR[282])
0x012C (0x000258) 0x2005-       f:00020 d:   5 | A = OR[5]                     
0x012D (0x00025A) 0x1406-       f:00012 d:   6 | A = A + 6 (0x0006)            
0x012E (0x00025C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x012F (0x00025E) 0x211A-       f:00020 d: 282 | A = OR[282]                   
0x0130 (0x000260) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
@ Return from overlay
0x0131 (0x000262) 0x102A-       f:00010 d:  42 | A = 42 (0x002A)               
0x0132 (0x000264) 0x2929-       f:00024 d: 297 | OR[297] = A                   
0x0133 (0x000266) 0x1129-       f:00010 d: 297 | A = 297 (0x0129)              
0x0134 (0x000268) 0x5800-       f:00054 d:   0 | B = A                         
0x0135 (0x00026A) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0136 (0x00026C) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ This could be an optional error handler?
@@ OR[286]: Error code??
0x0137 (0x00026E) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0138 (0x000270) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x0139 (0x000272) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x013A (0x000274) 0x2922-       f:00024 d: 290 | OR[290] = A                   
@ Test last command issued. Was it a write (40) or wirte EOF (48)?
0x013B (0x000276) 0x211D-       f:00020 d: 285 | A = OR[285]                   
0x013C (0x000278) 0x1628-       f:00013 d:  40 | A = A - 40 (0x0028)           
0x013D (0x00027A) 0x8405-       f:00102 d:   5 | P = P + 5 (0x0142), A = 0     
0x013E (0x00027C) 0x211D-       f:00020 d: 285 | A = OR[285]                   
0x013F (0x00027E) 0x1630-       f:00013 d:  48 | A = A - 48 (0x0030)           
0x0140 (0x000280) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0142), A = 0     
0x0141 (0x000282) 0x706F-       f:00070 d: 111 | P = P + 111 (0x01B0)          
@ Yes, command 40 (write) or 48 (write EOF)
@ Check loop termination: retry cound reached or OR[282] (return code) is 0
0x0142 (0x000284) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x0143 (0x000286) 0x160F-       f:00013 d:  15 | A = A - 15 (0x000F)           
0x0144 (0x000288) 0x846A-       f:00102 d: 106 | P = P + 106 (0x01AE), A = 0   
0x0145 (0x00028A) 0x211A-       f:00020 d: 282 | A = OR[282]                   
0x0146 (0x00028C) 0x8468-       f:00102 d: 104 | P = P + 104 (0x01AE), A = 0   

0x0147 (0x00028E) 0x1800-0x0200 f:00014 d:   0 | A = 512 (0x0200)              
0x0149 (0x000292) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x014A (0x000294) 0x7E03-0x02CE f:00077 d:   3 | R = OR[3]+718 (0x02CE)        @ Check error status and set OR[294] for 0x5422 mask, OR[290] for MSB only
@ If there was an error bit set in 0x5422 mask, report the original error passed in, and return from overlay
0x014C (0x000298) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x014D (0x00029A) 0x8602-       f:00103 d:   2 | P = P + 2 (0x014F), A # 0     
0x014E (0x00029C) 0x7003-       f:00070 d:   3 | P = P + 3 (0x0151)            
0x014F (0x00029E) 0x7A03-0x0285 f:00075 d:   3 | P = OR[3]+645 (0x0285)        
@ No, that wasn't the case. So is the error reported a 0x028B (DATA PARITY) error?
0x0151 (0x0002A2) 0x211E-       f:00020 d: 286 | A = OR[286]                   
0x0152 (0x0002A4) 0x1E00-0x028B f:00017 d:   0 | A = A - 651 (0x028B)          
0x0154 (0x0002A8) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0156), A = 0     
0x0155 (0x0002AA) 0x700B-       f:00070 d:  11 | P = P + 11 (0x0160)           
@ Data parity error was reported...
0x0156 (0x0002AC) 0x1800-0x0206 f:00014 d:   0 | A = 518 (0x0206)              
0x0158 (0x0002B0) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x0159 (0x0002B2) 0x7E03-0x02CE f:00077 d:   3 | R = OR[3]+718 (0x02CE)        @ Check error status and set OR[294] for 0x5422 mask, OR[290] for MSB only
@ If there was an error bit set in 0x5422 mask, report the original error passed in, and return from overlay
0x015B (0x0002B6) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x015C (0x0002B8) 0x8602-       f:00103 d:   2 | P = P + 2 (0x015E), A # 0     
0x015D (0x0002BA) 0x7003-       f:00070 d:   3 | P = P + 3 (0x0160)            
0x015E (0x0002BC) 0x7A03-0x0285 f:00075 d:   3 | P = OR[3]+645 (0x0285)        
@ Continue: was the last command a write (40)?
0x0160 (0x0002C0) 0x211D-       f:00020 d: 285 | A = OR[285]                   
0x0161 (0x0002C2) 0x1628-       f:00013 d:  40 | A = A - 40 (0x0028)           
0x0162 (0x0002C4) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0164), A = 0     
0x0163 (0x0002C6) 0x7005-       f:00070 d:   5 | P = P + 5 (0x0168)            
@ Yes it was, update OR[295] with 515
0x0164 (0x0002C8) 0x1800-0x0203 f:00014 d:   0 | A = 515 (0x0203)              
0x0166 (0x0002CC) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x0167 (0x0002CE) 0x7004-       f:00070 d:   4 | P = P + 4 (0x016B)            
@ No it wasnt't (so it was a write EOF command... update to 519
0x0168 (0x0002D0) 0x1800-0x0207 f:00014 d:   0 | A = 519 (0x0207)              
0x016A (0x0002D4) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x016B (0x0002D6) 0x7E03-0x02CE f:00077 d:   3 | R = OR[3]+718 (0x02CE)        @ Check error status and set OR[294] for 0x5422 mask, OR[290] for MSB only
0x016D (0x0002DA) 0x2126-       f:00020 d: 294 | A = OR[294]                   
@ If thee was a mask 0x5422 error, just loop immediately
0x016E (0x0002DC) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0170), A = 0     
0x016F (0x0002DE) 0x703D-       f:00070 d:  61 | P = P + 61 (0x01AC)           
@ There was no mask 0x5422 error
0x0170 (0x0002E0) 0x211D-       f:00020 d: 285 | A = OR[285]                   
0x0171 (0x0002E2) 0x1628-       f:00013 d:  40 | A = A - 40 (0x0028)           
0x0172 (0x0002E4) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0174), A = 0     
0x0173 (0x0002E6) 0x7036-       f:00070 d:  54 | P = P + 54 (0x01A9)           
@ This part will noly happen for write commands
@ Calculate buffer offset and put it in OR[275]
0x0174 (0x0002E8) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0175 (0x0002EA) 0x140F-       f:00012 d:  15 | A = A + 15 (0x000F)           
0x0176 (0x0002EC) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0177 (0x0002EE) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0178 (0x0002F0) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x0179 (0x0002F2) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x017A (0x0002F4) 0x1E00-0x26B0 f:00017 d:   0 | A = A - 9904 (0x26B0)         
0x017C (0x0002F8) 0x2913-       f:00024 d: 275 | OR[275] = A                   
@ Read write buffer size into OR[276]
0x017D (0x0002FA) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x017E (0x0002FC) 0x1416-       f:00012 d:  22 | A = A + 22 (0x0016)           
0x017F (0x0002FE) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0180 (0x000300) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0181 (0x000302) 0x2914-       f:00024 d: 276 | OR[276] = A                   
0x0182 (0x000304) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x0183 (0x000306) 0x2714-       f:00023 d: 276 | A = A - OR[276]               
0x0184 (0x000308) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0186), A # 0     
0x0185 (0x00030A) 0x700A-       f:00070 d:  10 | P = P + 10 (0x018F)           
@ We have not written the whole buffer, overwrite error code with 68 - RECORD LENGTH ERROR
0x0186 (0x00030C) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x0187 (0x00030E) 0x1644-       f:00013 d:  68 | A = A - 68 (0x0044)           
0x0188 (0x000310) 0x8602-       f:00103 d:   2 | P = P + 2 (0x018A), A # 0     
0x0189 (0x000312) 0x7005-       f:00070 d:   5 | P = P + 5 (0x018E)            
0x018A (0x000314) 0x1044-       f:00010 d:  68 | A = 68 (0x0044)               
0x018B (0x000316) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x018C (0x000318) 0x7E03-0x02C3 f:00077 d:   3 | R = OR[3]+707 (0x02C3)        @ This call will just return
0x018E (0x00031C) 0x701A-       f:00070 d:  26 | P = P + 26 (0x01A8)           @ Continue looping
@ We have written the whole buffer
@ Check satus bit 0x0200 - UKNOWN ERROR
0x018F (0x00031E) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0190 (0x000320) 0x140E-       f:00012 d:  14 | A = A + 14 (0x000E)           
0x0191 (0x000322) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0192 (0x000324) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0193 (0x000326) 0x1A00-0x0200 f:00015 d:   0 | A = A & 512 (0x0200)          
0x0195 (0x00032A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0196 (0x00032C) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0197 (0x00032E) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x0198 (0x000330) 0x8602-       f:00103 d:   2 | P = P + 2 (0x019A), A # 0     
0x0199 (0x000332) 0x700D-       f:00070 d:  13 | P = P + 13 (0x01A6)           
@ Bit is set - unload tape (function 4097 in XTAPED)
0x019A (0x000334) 0x1800-0x1001 f:00014 d:   0 | A = 4097 (0x1001)             
0x019C (0x000338) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x019D (0x00033A) 0x7E03-0x02FC f:00077 d:   3 | R = OR[3]+764 (0x02FC)        @ Call XTAPED function 4097 and update return code in OR[282] if success
@ Set error-code from OR[283] if that's non-0, and return from overlay
0x019F (0x00033E) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x01A0 (0x000340) 0x8602-       f:00103 d:   2 | P = P + 2 (0x01A2), A # 0     
0x01A1 (0x000342) 0x7004-       f:00070 d:   4 | P = P + 4 (0x01A5)            
0x01A2 (0x000344) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x01A3 (0x000346) 0x291A-       f:00024 d: 282 | OR[282] = A                   @ OR[283] is 0 at this point, so zero-out return value
0x01A4 (0x000348) 0x729D-       f:00071 d: 157 | P = P - 157 (0x0107)          
0x01A5 (0x00034A) 0x7003-       f:00070 d:   3 | P = P + 3 (0x01A8)            
@ UNKOWN ERROR is not set, set return value to 0 and continue looping
0x01A6 (0x00034C) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x01A7 (0x00034E) 0x291A-       f:00024 d: 282 | OR[282] = A                   
0x01A8 (0x000350) 0x7003-       f:00070 d:   3 | P = P + 3 (0x01AB)            

0x01A9 (0x000352) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x01AA (0x000354) 0x291A-       f:00024 d: 282 | OR[282] = A                   
0x01AB (0x000356) 0x7002-       f:00070 d:   2 | P = P + 2 (0x01AD)            
0x01AC (0x000358) 0x2D21-       f:00026 d: 289 | OR[289] = OR[289] + 1         
0x01AD (0x00035A) 0x726B-       f:00071 d: 107 | P = P - 107 (0x0142)          
@ Exiting the loop above: either because retry limit reached or return code is non-0
0x01AE (0x00035C) 0x7A03-0x0224 f:00075 d:   3 | P = OR[3]+548 (0x0224)        
0x01B0 (0x000360) 0x211D-       f:00020 d: 285 | A = OR[285]                   
0x01B1 (0x000362) 0x8403-       f:00102 d:   3 | P = P + 3 (0x01B4), A = 0     
0x01B2 (0x000364) 0x7A03-0x0224 f:00075 d:   3 | P = OR[3]+548 (0x0224)        
0x01B4 (0x000368) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x01B5 (0x00036A) 0x1629-       f:00013 d:  41 | A = A - 41 (0x0029)           
0x01B6 (0x00036C) 0xAC03-0x0224 f:00126 d:   3 | P = OR[3]+548 (0x0224), A = 0 
0x01B8 (0x000370) 0x211A-       f:00020 d: 282 | A = OR[282]                   
0x01B9 (0x000372) 0xAC03-0x0224 f:00126 d:   3 | P = OR[3]+548 (0x0224), A = 0 
0x01BB (0x000376) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x01BC (0x000378) 0x1204-       f:00011 d:   4 | A = A & 4 (0x0004)            
0x01BD (0x00037A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x01BE (0x00037C) 0x1004-       f:00010 d:   4 | A = 4 (0x0004)                
0x01BF (0x00037E) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x01C0 (0x000380) 0x8402-       f:00102 d:   2 | P = P + 2 (0x01C2), A = 0     
0x01C1 (0x000382) 0x702D-       f:00070 d:  45 | P = P + 45 (0x01EE)           
0x01C2 (0x000384) 0x1005-       f:00010 d:   5 | A = 5 (0x0005)                
0x01C3 (0x000386) 0x2923-       f:00024 d: 291 | OR[291] = A                   
0x01C4 (0x000388) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x01C5 (0x00038A) 0x2924-       f:00024 d: 292 | OR[292] = A                   
0x01C6 (0x00038C) 0x2123-       f:00020 d: 291 | A = OR[291]                   
0x01C7 (0x00038E) 0x8417-       f:00102 d:  23 | P = P + 23 (0x01DE), A = 0    
0x01C8 (0x000390) 0x2124-       f:00020 d: 292 | A = OR[292]                   
0x01C9 (0x000392) 0x8615-       f:00103 d:  21 | P = P + 21 (0x01DE), A # 0    
0x01CA (0x000394) 0x1800-0x0200 f:00014 d:   0 | A = 512 (0x0200)              
0x01CC (0x000398) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x01CD (0x00039A) 0x7E03-0x02CE f:00077 d:   3 | R = OR[3]+718 (0x02CE)        @ Check error status and set OR[294] for 0x5422 mask, OR[290] for MSB only
0x01CF (0x00039E) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x01D0 (0x0003A0) 0x140E-       f:00012 d:  14 | A = A + 14 (0x000E)           
0x01D1 (0x0003A2) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x01D2 (0x0003A4) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x01D3 (0x0003A6) 0x1280-       f:00011 d: 128 | A = A & 128 (0x0080)          
0x01D4 (0x0003A8) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x01D5 (0x0003AA) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x01D6 (0x0003AC) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x01D7 (0x0003AE) 0x8602-       f:00103 d:   2 | P = P + 2 (0x01D9), A # 0     
0x01D8 (0x0003B0) 0x7004-       f:00070 d:   4 | P = P + 4 (0x01DC)            
0x01D9 (0x0003B2) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x01DA (0x0003B4) 0x2924-       f:00024 d: 292 | OR[292] = A                   
0x01DB (0x0003B6) 0x7002-       f:00070 d:   2 | P = P + 2 (0x01DD)            
0x01DC (0x0003B8) 0x2F23-       f:00027 d: 291 | OR[291] = OR[291] - 1         
0x01DD (0x0003BA) 0x7217-       f:00071 d:  23 | P = P - 23 (0x01C6)           
0x01DE (0x0003BC) 0x2123-       f:00020 d: 291 | A = OR[291]                   
0x01DF (0x0003BE) 0x1604-       f:00013 d:   4 | A = A - 4 (0x0004)            
0x01E0 (0x0003C0) 0x840D-       f:00102 d:  13 | P = P + 13 (0x01ED), A = 0    
0x01E1 (0x0003C2) 0x1800-0x0201 f:00014 d:   0 | A = 513 (0x0201)              
0x01E3 (0x0003C6) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x01E4 (0x0003C8) 0x7E03-0x02CE f:00077 d:   3 | R = OR[3]+718 (0x02CE)        @ Check error status and set OR[294] for 0x5422 mask, OR[290] for MSB only
0x01E6 (0x0003CC) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x01E7 (0x0003CE) 0x8602-       f:00103 d:   2 | P = P + 2 (0x01E9), A # 0     
0x01E8 (0x0003D0) 0x7003-       f:00070 d:   3 | P = P + 3 (0x01EB)            
0x01E9 (0x0003D2) 0x7A03-0x0285 f:00075 d:   3 | P = OR[3]+645 (0x0285)        
0x01EB (0x0003D6) 0x2D23-       f:00026 d: 291 | OR[291] = OR[291] + 1         
0x01EC (0x0003D8) 0x720E-       f:00071 d:  14 | P = P - 14 (0x01DE)           
0x01ED (0x0003DA) 0x700B-       f:00070 d:  11 | P = P + 11 (0x01F8)           
0x01EE (0x0003DC) 0x1800-0x0200 f:00014 d:   0 | A = 512 (0x0200)              
0x01F0 (0x0003E0) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x01F1 (0x0003E2) 0x7E03-0x02CE f:00077 d:   3 | R = OR[3]+718 (0x02CE)        @ Check error status and set OR[294] for 0x5422 mask, OR[290] for MSB only
0x01F3 (0x0003E6) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x01F4 (0x0003E8) 0x8602-       f:00103 d:   2 | P = P + 2 (0x01F6), A # 0     
0x01F5 (0x0003EA) 0x7003-       f:00070 d:   3 | P = P + 3 (0x01F8)            
0x01F6 (0x0003EC) 0x7A03-0x0285 f:00075 d:   3 | P = OR[3]+645 (0x0285)        
0x01F8 (0x0003F0) 0x1800-0x0202 f:00014 d:   0 | A = 514 (0x0202)              
0x01FA (0x0003F4) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x01FB (0x0003F6) 0x7E03-0x02CE f:00077 d:   3 | R = OR[3]+718 (0x02CE)        @ Check error status and set OR[294] for 0x5422 mask, OR[290] for MSB only
0x01FD (0x0003FA) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x01FE (0x0003FC) 0xAC03-0x0207 f:00126 d:   3 | P = OR[3]+519 (0x0207), A = 0 
0x0200 (0x000400) 0x701C-       f:00070 d:  28 | P = P + 28 (0x021C)           
0x0201 (0x000402) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0202 (0x000404) 0x140F-       f:00012 d:  15 | A = A + 15 (0x000F)           
0x0203 (0x000406) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0204 (0x000408) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0205 (0x00040A) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x0206 (0x00040C) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x0207 (0x00040E) 0x1E00-0x26B0 f:00017 d:   0 | A = A - 9904 (0x26B0)         
0x0209 (0x000412) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x020A (0x000414) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x020B (0x000416) 0x1415-       f:00012 d:  21 | A = A + 21 (0x0015)           
0x020C (0x000418) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x020D (0x00041A) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x020E (0x00041C) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x020F (0x00041E) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x0210 (0x000420) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0212), A = 0     
0x0211 (0x000422) 0x7009-       f:00070 d:   9 | P = P + 9 (0x021A)            
0x0212 (0x000424) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x0213 (0x000426) 0x1644-       f:00013 d:  68 | A = A - 68 (0x0044)           
0x0214 (0x000428) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0216), A # 0     
0x0215 (0x00042A) 0x7004-       f:00070 d:   4 | P = P + 4 (0x0219)            
0x0216 (0x00042C) 0x1044-       f:00010 d:  68 | A = 68 (0x0044)               
0x0217 (0x00042E) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x0218 (0x000430) 0x74A5-       f:00072 d: 165 | R = P + 165 (0x02BD)          @ Call XTAPEB function 68 - RECORD LENGTH ERROR
0x0219 (0x000432) 0x7003-       f:00070 d:   3 | P = P + 3 (0x021C)            
0x021A (0x000434) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x021B (0x000436) 0x291A-       f:00024 d: 282 | OR[282] = A                   
0x021C (0x000438) 0x2D21-       f:00026 d: 289 | OR[289] = OR[289] + 1         
0x021D (0x00043A) 0x7269-       f:00071 d: 105 | P = P - 105 (0x01B4)          
0x021E (0x00043C) 0x211A-       f:00020 d: 282 | A = OR[282]                   
0x021F (0x00043E) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0221), A = 0     
0x0220 (0x000440) 0x7007-       f:00070 d:   7 | P = P + 7 (0x0227)            
0x0221 (0x000442) 0x2122-       f:00020 d: 290 | A = OR[290]                   
0x0222 (0x000444) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0224), A # 0     
0x0223 (0x000446) 0x7004-       f:00070 d:   4 | P = P + 4 (0x0227)            

0x0224 (0x000448) 0x1046-       f:00010 d:  70 | A = 70 (0x0046)               
0x0225 (0x00044A) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x0226 (0x00044C) 0x7497-       f:00072 d: 151 | R = P + 151 (0x02BD)          @ Call XTAPEB function 70 - print error determined from status bits???
0x0227 (0x00044E) 0x211A-       f:00020 d: 282 | A = OR[282]                   
0x0228 (0x000450) 0x8402-       f:00102 d:   2 | P = P + 2 (0x022A), A = 0     
0x0229 (0x000452) 0x7003-       f:00070 d:   3 | P = P + 3 (0x022C)            
0x022A (0x000454) 0x705B-       f:00070 d:  91 | P = P + 91 (0x0285)           
0x022B (0x000456) 0x7002-       f:00070 d:   2 | P = P + 2 (0x022D)            
0x022C (0x000458) 0x7053-       f:00070 d:  83 | P = P + 83 (0x027F)           
0x022D (0x00045A) 0x1190-       f:00010 d: 400 | A = 400 (0x0190)              
0x022E (0x00045C) 0x8405-       f:00102 d:   5 | P = P + 5 (0x0233), A = 0     
0x022F (0x00045E) 0x420F-       f:00041 d:  15 | C = 1, io 0017 (EXB) = BZ       
0x0230 (0x000460) 0x8003-       f:00100 d:   3 | P = P + 3 (0x0233), C = 0     
0x0231 (0x000462) 0x1601-       f:00013 d:   1 | A = A - 1 (0x0001)            
0x0232 (0x000464) 0x7204-       f:00071 d:   4 | P = P - 4 (0x022E)            
0x0233 (0x000466) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0235), A = 0     
0x0234 (0x000468) 0x7004-       f:00070 d:   4 | P = P + 4 (0x0238)            
0x0235 (0x00046A) 0x1041-       f:00010 d:  65 | A = 65 (0x0041)               
0x0236 (0x00046C) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x0237 (0x00046E) 0x7486-       f:00072 d: 134 | R = P + 134 (0x02BD)          @ Call XTAPEB function 65 - CHANNEL TIMEOUT
0x0238 (0x000470) 0x0200-       f:00001 d:   0 | EXIT                          
0x0239 (0x000472) 0x7450-       f:00072 d:  80 | R = P + 80 (0x0289)           
@ Clear up upper byte of entry 11 - the stored error-code
0x023A (0x000474) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x023B (0x000476) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x023C (0x000478) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x023D (0x00047A) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x023E (0x00047C) 0x0A09-       f:00005 d:   9 | A = A < 9 (0x0009)            
0x023F (0x00047E) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x0240 (0x000480) 0x0C09-       f:00006 d:   9 | A = A >> 9 (0x0009)           
0x0241 (0x000482) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 

@@ This loop waits for bit 2 to become 0, at which point it checks for bit 0 and bit 8. It terminates if both are set.
@@ It also terminates if the high-byte of desc. entry 11 is not 1 or 4
@ Set up a while loop in OR[293]
0x0242 (0x000484) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0243 (0x000486) 0x2925-       f:00024 d: 293 | OR[293] = A                   
0x0244 (0x000488) 0x2125-       f:00020 d: 293 | A = OR[293]                   
0x0245 (0x00048A) 0x8630-       f:00103 d:  48 | P = P + 48 (0x0275), A # 0    

@ Read the value of RegA into OR[294]
0x0246 (0x00048C) 0x7462-       f:00072 d:  98 | R = P + 98 (0x02A8)           
0x0247 (0x00048E) 0x2123-       f:00020 d: 291 | A = OR[291]                   
0x0248 (0x000490) 0x1604-       f:00013 d:   4 | A = A - 4 (0x0004)            
0x0249 (0x000492) 0x8402-       f:00102 d:   2 | P = P + 2 (0x024B), A = 0     
0x024A (0x000494) 0x700A-       f:00070 d:  10 | P = P + 10 (0x0254)           
@ Test RegA bit 2, if not set, set OR[291] to 1
0x024B (0x000496) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x024C (0x000498) 0x1204-       f:00011 d:   4 | A = A & 4 (0x0004)            
0x024D (0x00049A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x024E (0x00049C) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x024F (0x00049E) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x0250 (0x0004A0) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0252), A = 0     
0x0251 (0x0004A2) 0x7003-       f:00070 d:   3 | P = P + 3 (0x0254)            
0x0252 (0x0004A4) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x0253 (0x0004A6) 0x2923-       f:00024 d: 291 | OR[291] = A                   
@ See if OR[291] is 1
0x0254 (0x0004A8) 0x2123-       f:00020 d: 291 | A = OR[291]                   
0x0255 (0x0004AA) 0x1601-       f:00013 d:   1 | A = A - 1 (0x0001)            
0x0256 (0x0004AC) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0258), A = 0     
0x0257 (0x0004AE) 0x700A-       f:00070 d:  10 | P = P + 10 (0x0261)           
@ Test RegA bit 8 and bit 1, if both are set, set OR[293] to 1, which will terminate the loop
0x0258 (0x0004B0) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x0259 (0x0004B2) 0x1281-       f:00011 d: 129 | A = A & 129 (0x0081)          
0x025A (0x0004B4) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x025B (0x0004B6) 0x1081-       f:00010 d: 129 | A = 129 (0x0081)              
0x025C (0x0004B8) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x025D (0x0004BA) 0x8402-       f:00102 d:   2 | P = P + 2 (0x025F), A = 0     
0x025E (0x0004BC) 0x7003-       f:00070 d:   3 | P = P + 3 (0x0261)            
0x025F (0x0004BE) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x0260 (0x0004C0) 0x2925-       f:00024 d: 293 | OR[293] = A                   
@ Load previously stored error code from desc. offset 11.
0x0261 (0x0004C2) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0262 (0x0004C4) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x0263 (0x0004C6) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0264 (0x0004C8) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0265 (0x0004CA) 0x0808-       f:00004 d:   8 | A = A > 8 (0x0008)            
0x0266 (0x0004CC) 0x291A-       f:00024 d: 282 | OR[282] = A                   
0x0267 (0x0004CE) 0x211A-       f:00020 d: 282 | A = OR[282]                   
0x0268 (0x0004D0) 0x1601-       f:00013 d:   1 | A = A - 1 (0x0001)            @ If error code is 1 or 4, ignore it and continue looping.
0x0269 (0x0004D2) 0x8405-       f:00102 d:   5 | P = P + 5 (0x026E), A = 0     
0x026A (0x0004D4) 0x211A-       f:00020 d: 282 | A = OR[282]                   
0x026B (0x0004D6) 0x1604-       f:00013 d:   4 | A = A - 4 (0x0004)            
0x026C (0x0004D8) 0x8402-       f:00102 d:   2 | P = P + 2 (0x026E), A = 0     
0x026D (0x0004DA) 0x7005-       f:00070 d:   5 | P = P + 5 (0x0272)            
@ zero out OR[282] - the return value
0x026E (0x0004DC) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x026F (0x0004DE) 0x291A-       f:00024 d: 282 | OR[282] = A                   
@ call 0x02AD - which will issue an overlay manager command 7 and continue looping
0x0270 (0x0004E0) 0x743D-       f:00072 d:  61 | R = P + 61 (0x02AD)           
0x0271 (0x0004E2) 0x7003-       f:00070 d:   3 | P = P + 3 (0x0274)            
@ Set default return value to 1 and continue looping
0x0272 (0x0004E4) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                @ Error code is neither 1 or 4: exit from loop
0x0273 (0x0004E6) 0x2925-       f:00024 d: 293 | OR[293] = A                   
0x0274 (0x0004E8) 0x7230-       f:00071 d:  48 | P = P - 48 (0x0244)           
@ outside of loop: zero out high-byte of entry 11 in desc. and return - at this point we might have the stored error-code in OR[282]
0x0275 (0x0004EA) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0276 (0x0004EC) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x0277 (0x0004EE) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0278 (0x0004F0) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0279 (0x0004F2) 0x0E01-       f:00007 d:   1 | A = A << 1 (0x0001)           
0x027A (0x0004F4) 0x0A08-       f:00005 d:   8 | A = A < 8 (0x0008)            
0x027B (0x0004F6) 0x1400-       f:00012 d:   0 | A = A + 0 (0x0000)            
0x027C (0x0004F8) 0x0C09-       f:00006 d:   9 | A = A >> 9 (0x0009)           
0x027D (0x0004FA) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x027E (0x0004FC) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Error handler
@@ OR[286]: Error code
0x027F (0x0004FE) 0x2003-       f:00020 d:   3 | A = OR[3]                     
0x0280 (0x000500) 0x1C00-0x0322 f:00016 d:   0 | A = A + 802 (0x0322)          
0x0282 (0x000504) 0x2920-       f:00024 d: 288 | OR[288] = A                   
0x0283 (0x000506) 0x7406-       f:00072 d:   6 | R = P + 6 (0x0289)            
0x0284 (0x000508) 0x737D-       f:00071 d: 381 | P = P - 381 (0x0107)          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Error handler
@@ OR[286]: Error code
0x0285 (0x00050A) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0286 (0x00050C) 0x2920-       f:00024 d: 288 | OR[288] = A                   
0x0287 (0x00050E) 0x7402-       f:00072 d:   2 | R = P + 2 (0x0289)            
0x0288 (0x000510) 0x7381-       f:00071 d: 385 | P = P - 385 (0x0107)          
0x0289 (0x000512) 0x1002-       f:00010 d:   2 | A = 2 (0x0002)                
0x028A (0x000514) 0xCE0F-       f:00147 d:  15 | io 0017 (EXB), fn007           | Set interrupt mode
0x028B (0x000516) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x028C (0x000518) 0x1400-       f:00012 d:   0 | A = A + 0 (0x0000)            
0x028D (0x00051A) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x028E (0x00051C) 0x2003-       f:00020 d:   3 | A = OR[3]                     
0x028F (0x00051E) 0x251F-       f:00022 d: 287 | A = A + OR[287]               
0x0290 (0x000520) 0x291F-       f:00024 d: 287 | OR[287] = A                   
@ Call MSGHND. Probably to print an error message
@ 1st param: Overlay manager command 40 - execute overlay
@ 2nd param: Overlay number 0x2f - MSGHND
@ 3rd param: Error code (from OR[286])
@ 4th param: Device name (points to the string MT0 in device desc)
@ 5th param: Local string pointer to operation string: 'WRITE' 'READ' or 'POSITION'
@ 6th param: 0
0x0291 (0x000522) 0x1028-       f:00010 d:  40 | A = 40 (0x0028)               
0x0292 (0x000524) 0x2929-       f:00024 d: 297 | OR[297] = A                   
0x0293 (0x000526) 0x1800-0x002F f:00014 d:   0 | A = 47 (0x002F)               
0x0295 (0x00052A) 0x292A-       f:00024 d: 298 | OR[298] = A                   
0x0296 (0x00052C) 0x211E-       f:00020 d: 286 | A = OR[286]                   
0x0297 (0x00052E) 0x292B-       f:00024 d: 299 | OR[299] = A                   
0x0298 (0x000530) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x0299 (0x000532) 0x292C-       f:00024 d: 300 | OR[300] = A                   
0x029A (0x000534) 0x211F-       f:00020 d: 287 | A = OR[287]                   
0x029B (0x000536) 0x292D-       f:00024 d: 301 | OR[301] = A                   
0x029C (0x000538) 0x2120-       f:00020 d: 288 | A = OR[288]                   
0x029D (0x00053A) 0x292E-       f:00024 d: 302 | OR[302] = A                   
0x029E (0x00053C) 0x1129-       f:00010 d: 297 | A = 297 (0x0129)              
0x029F (0x00053E) 0x5800-       f:00054 d:   0 | B = A                         
0x02A0 (0x000540) 0x1800-0x2318 f:00014 d:   0 | A = 8984 (0x2318)             
0x02A2 (0x000544) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

0x02A3 (0x000546) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x02A4 (0x000548) 0xCE0F-       f:00147 d:  15 | io 0017 (EXB), fn007           | Set interrupt mode
0x02A5 (0x00054A) 0x211C-       f:00020 d: 284 | A = OR[284]                   
0x02A6 (0x00054C) 0xCA0F-       f:00145 d:  15 | io 0017 (EXB), fn005           | Load device address
0x02A7 (0x00054E) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Read RegA and return it in OR[294]
0x02A8 (0x000550) 0xC20F-       f:00141 d:  15 | io 0017 (EXB), fn001           | Data input from A register (DIA)
0x02A9 (0x000552) 0x767C-       f:00073 d: 124 | R = P - 124 (0x022D)          
0x02AA (0x000554) 0xD00F-       f:00150 d:  15 | io 0017 (EXB), fn010           | Read data bus status
0x02AB (0x000556) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x02AC (0x000558) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Call kernel function PAUS - wait for 1ms (?)
0x02AD (0x00055A) 0x1002-       f:00010 d:   2 | A = 2 (0x0002)                
0x02AE (0x00055C) 0xCE0F-       f:00147 d:  15 | io 0017 (EXB), fn007           | Set interrupt mode
@ Kernel function PAUS - wait for 1ms (?)
0x02AF (0x00055E) 0x1007-       f:00010 d:   7 | A = 7 (0x0007)                
0x02B0 (0x000560) 0x2929-       f:00024 d: 297 | OR[297] = A                   
0x02B1 (0x000562) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x02B2 (0x000564) 0x292A-       f:00024 d: 298 | OR[298] = A                   
0x02B3 (0x000566) 0x1129-       f:00010 d: 297 | A = 297 (0x0129)              
0x02B4 (0x000568) 0x5800-       f:00054 d:   0 | B = A                         
0x02B5 (0x00056A) 0x1800-0x2318 f:00014 d:   0 | A = 8984 (0x2318)             
0x02B7 (0x00056E) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

0x02B8 (0x000570) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x02B9 (0x000572) 0xCE0F-       f:00147 d:  15 | io 0017 (EXB), fn007           | Set interrupt mode
0x02BA (0x000574) 0x211C-       f:00020 d: 284 | A = OR[284]                   
0x02BB (0x000576) 0xCA0F-       f:00145 d:  15 | io 0017 (EXB), fn005           | Load device address
0x02BC (0x000578) 0x0200-       f:00001 d:   0 | EXIT                          


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Call XTAPEB
0x02BD (0x00057A) 0x1800-0x00AF f:00014 d:   0 | A = 175 (0x00AF)              @ Call XTAPEB
0x02BF (0x00057E) 0x2928-       f:00024 d: 296 | OR[296] = A                   
0x02C0 (0x000580) 0x743B-       f:00072 d:  59 | R = P + 59 (0x02FB)           
0x02C1 (0x000582) 0x211B-       f:00020 d: 283 | A = OR[283]                   @ Check return value: if non-0, report error (?) and return from overlay
0x02C2 (0x000584) 0x8602-       f:00103 d:   2 | P = P + 2 (0x02C4), A # 0     
0x02C3 (0x000586) 0x7004-       f:00070 d:   4 | P = P + 4 (0x02C7)            
0x02C4 (0x000588) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x02C5 (0x00058A) 0x291A-       f:00024 d: 282 | OR[282] = A                   
0x02C6 (0x00058C) 0x7247-       f:00071 d:  71 | P = P - 71 (0x027F)           
0x02C7 (0x00058E) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Call XTAPEC
0x02C8 (0x000590) 0x1800-0x00B0 f:00014 d:   0 | A = 176 (0x00B0)              @ Call XTAPEC
0x02CA (0x000594) 0x2928-       f:00024 d: 296 | OR[296] = A                   
0x02CB (0x000596) 0x7430-       f:00072 d:  48 | R = P + 48 (0x02FB)           
0x02CC (0x000598) 0x211B-       f:00020 d: 283 | A = OR[283]                   @ Check return value: if non-0, report error (?) and return from overlay
0x02CD (0x00059A) 0x8602-       f:00103 d:   2 | P = P + 2 (0x02CF), A # 0     
0x02CE (0x00059C) 0x7004-       f:00070 d:   4 | P = P + 4 (0x02D2)            
0x02CF (0x00059E) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x02D0 (0x0005A0) 0x291A-       f:00024 d: 282 | OR[282] = A                   
0x02D1 (0x0005A2) 0x7252-       f:00071 d:  82 | P = P - 82 (0x027F)           

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Check error status and return flags appropriately:
@@ Output: OR[294] - set to 1 if there were bit set in status mask 0x5422
@@         OR[290] - set to 1 if only MSB is set, but no other bits in the above mask
@ Check entry 14, RegA
0x02D2 (0x0005A4) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x02D3 (0x0005A6) 0x140E-       f:00012 d:  14 | A = A + 14 (0x000E)           
0x02D4 (0x0005A8) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x02D5 (0x0005AA) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x02D6 (0x0005AC) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x02D7 (0x0005AE) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x02D8 (0x0005B0) 0x1A00-0x8000 f:00015 d:   0 | A = A & 32768 (0x8000)        
0x02DA (0x0005B4) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x02DB (0x0005B6) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x02DC (0x0005B8) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x02DD (0x0005BA) 0x8402-       f:00102 d:   2 | P = P + 2 (0x02DF), A = 0     
0x02DE (0x0005BC) 0x7006-       f:00070 d:   6 | P = P + 6 (0x02E4)            
@ MSB is not set, clear OR[294] and OR[290] and return
0x02DF (0x0005BE) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x02E0 (0x0005C0) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x02E1 (0x0005C2) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x02E2 (0x0005C4) 0x2922-       f:00024 d: 290 | OR[290] = A                   
0x02E3 (0x0005C6) 0x7012-       f:00070 d:  18 | P = P + 18 (0x02F5)           
@ MSB set, are there any important error-bits set to be reported?
0x02E4 (0x0005C8) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x02E5 (0x0005CA) 0x1A00-0x5422 f:00015 d:   0 | A = A & 21538 (0x5422)        
0x02E7 (0x0005CE) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x02E8 (0x0005D0) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x02E9 (0x0005D2) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x02EA (0x0005D4) 0x8402-       f:00102 d:   2 | P = P + 2 (0x02EC), A = 0     
0x02EB (0x0005D6) 0x7006-       f:00070 d:   6 | P = P + 6 (0x02F1)            
@ No error to report, clear OR[294] set OR[290] to 1 and return
0x02EC (0x0005D8) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x02ED (0x0005DA) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x02EE (0x0005DC) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x02EF (0x0005DE) 0x2922-       f:00024 d: 290 | OR[290] = A                   
0x02F0 (0x0005E0) 0x7005-       f:00070 d:   5 | P = P + 5 (0x02F5)            
@ There were some errors, so set OR[294] to 1, clear OR[290] and return
0x02F1 (0x0005E2) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x02F2 (0x0005E4) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x02F3 (0x0005E6) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x02F4 (0x0005E8) 0x2922-       f:00024 d: 290 | OR[290] = A                   
0x02F5 (0x0005EA) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Call XTAPED
0x02F6 (0x0005EC) 0x1800-0x00B1 f:00014 d:   0 | A = 177 (0x00B1)              
0x02F8 (0x0005F0) 0x2928-       f:00024 d: 296 | OR[296] = A                   
0x02F9 (0x0005F2) 0x7402-       f:00072 d:   2 | R = P + 2 (0x02FB)            
0x02FA (0x0005F4) 0x0200-       f:00001 d:   0 | EXIT                          

@ Here we call XTAPEB, C or D, depending on the value of OR[296]
0x02FB (0x0005F6) 0x1028-       f:00010 d:  40 | A = 40 (0x0028)               
0x02FC (0x0005F8) 0x2929-       f:00024 d: 297 | OR[297] = A                   
0x02FD (0x0005FA) 0x2128-       f:00020 d: 296 | A = OR[296]                   
0x02FE (0x0005FC) 0x292A-       f:00024 d: 298 | OR[298] = A                   
0x02FF (0x0005FE) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0300 (0x000600) 0x292B-       f:00024 d: 299 | OR[299] = A                   
0x0301 (0x000602) 0x2127-       f:00020 d: 295 | A = OR[295]                   
0x0302 (0x000604) 0x292C-       f:00024 d: 300 | OR[300] = A                   
0x0303 (0x000606) 0x1129-       f:00010 d: 297 | A = 297 (0x0129)              
0x0304 (0x000608) 0x5800-       f:00054 d:   0 | B = A                         
0x0305 (0x00060A) 0x1800-0x2318 f:00014 d:   0 | A = 8984 (0x2318)             
0x0307 (0x00060E) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

0x0308 (0x000610) 0x291B-       f:00024 d: 283 | OR[283] = A                   
0x0309 (0x000612) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x030A (0x000614) 0xCE0F-       f:00147 d:  15 | io 0017 (EXB), fn007           | Set interrupt mode
0x030B (0x000616) 0x211C-       f:00020 d: 284 | A = OR[284]                   
0x030C (0x000618) 0xCA0F-       f:00145 d:  15 | io 0017 (EXB), fn005           | Load device address
0x030D (0x00061A) 0x0200-       f:00001 d:   0 | EXIT                          

@ String 'WRITE'
0x030E (0x00061C) 0x5752
0x030F (0x00061E) 0x4954
0x0310 (0x000620) 0x4500
0x0311 (0x000622) 0x0000
@ String 'READ'
0x0312 (0x000624) 0x5245
0x0313 (0x000626) 0x4144
0x0314 (0x000628) 0x0000
0x0315 (0x00062A) 0x0000
@ String 'POSITION'
0x0316 (0x00062C) 0x504F
0x0317 (0x00062E) 0x5349
0x0318 (0x000630) 0x5449
0x0319 (0x000632) 0x4F4E
0x031A (0x000634) 0x0000
0x031B (0x000636) 0x0000
@ String 'UK'
0x031C (0x000638) 0x554E
0x031D (0x00063A) 0x0000
0x031E (0x00063C) 0x0000
0x031F (0x00063E) 0x0000
0x0320 (0x000640) 0x0000
0x0321 (0x000642) 0x0000
0x0322 (0x000644) 0x0000
0x0323 (0x000646) 0x0000
