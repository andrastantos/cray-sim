@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ CRAY STARTUP FROM DISK
@@
@@ Load address: 
@@ INPUT
@@    OR[280] : 'DK' 0x0000 - load device description
@@    OR[281] : Start address in central memory, high word
@@    OR[282] : Start address in central memory, low word

@@    OR[283] : return value
@@    OR[284] : device desc.
@@    OR[285] : file handle
@@    OR[286] : 2k buffer

0x0000 (0x000000) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0001 (0x000002) 0x291B-       f:00024 d: 283 | OR[283] = A                   
0x0002 (0x000004) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0003 (0x000006) 0x291C-       f:00024 d: 284 | OR[284] = A                   
0x0004 (0x000008) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0005 (0x00000A) 0x291D-       f:00024 d: 285 | OR[285] = A                   
0x0006 (0x00000C) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0007 (0x00000E) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x0008 (0x000010) 0x7436-       f:00072 d:  54 | R = P + 54 (0x003E)           @ Search file and setup device desc. entry 18 and 19

@ Allocate a 2k buffer in OR[286], wait until successful
0x0009 (0x000012) 0x101A-       f:00010 d:  26 | A = 26 (0x001A)               
0x000A (0x000014) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x000B (0x000016) 0x111E-       f:00010 d: 286 | A = 286 (0x011E)              
0x000C (0x000018) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x000D (0x00001A) 0x1126-       f:00010 d: 294 | A = 294 (0x0126)              
0x000E (0x00001C) 0x5800-       f:00054 d:   0 | B = A                         
0x000F (0x00001E) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0010 (0x000020) 0x7C09-       f:00076 d:   9 | R = OR[9]                     
0x0011 (0x000022) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0013), A # 0     
0x0012 (0x000024) 0x700B-       f:00070 d:  11 | P = P + 11 (0x001D)           
0x0013 (0x000026) 0x1007-       f:00010 d:   7 | A = 7 (0x0007)                
0x0014 (0x000028) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x0015 (0x00002A) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x0016 (0x00002C) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x0017 (0x00002E) 0x1126-       f:00010 d: 294 | A = 294 (0x0126)              
0x0018 (0x000030) 0x5800-       f:00054 d:   0 | B = A                         
0x0019 (0x000032) 0x1800-0x1D18 f:00014 d:   0 | A = 7448 (0x1D18)             
0x001B (0x000036) 0x7C09-       f:00076 d:   9 | R = OR[9]                     
0x001C (0x000038) 0x7213-       f:00071 d:  19 | P = P - 19 (0x0009)           

@ Read file from disk, one sector at a time and send over to BIOP to transfer to main memory
0x001D (0x00003A) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x001E (0x00003C) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x001F (0x00003E) 0x211F-       f:00020 d: 287 | A = OR[287]                   
0x0020 (0x000040) 0x8604-       f:00103 d:   4 | P = P + 4 (0x0024), A # 0     
0x0021 (0x000042) 0x7472-       f:00072 d: 114 | R = P + 114 (0x0093)          @ Read sector from disk
0x0022 (0x000044) 0x7504-       f:00072 d: 260 | R = P + 260 (0x0126)          @ Pass on sector to BIOP?
0x0023 (0x000046) 0x7204-       f:00071 d:   4 | P = P - 4 (0x001F)            

@ Free buffer, if not NULL
0x0024 (0x000048) 0x211E-       f:00020 d: 286 | A = OR[286]                   
0x0025 (0x00004A) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0027), A # 0     
0x0026 (0x00004C) 0x7009-       f:00070 d:   9 | P = P + 9 (0x002F)            
0x0027 (0x00004E) 0x101B-       f:00010 d:  27 | A = 27 (0x001B)               
0x0028 (0x000050) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x0029 (0x000052) 0x211E-       f:00020 d: 286 | A = OR[286]                   
0x002A (0x000054) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x002B (0x000056) 0x1126-       f:00010 d: 294 | A = 294 (0x0126)              
0x002C (0x000058) 0x5800-       f:00054 d:   0 | B = A                         
0x002D (0x00005A) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x002E (0x00005C) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

0x002F (0x00005E) 0x211C-       f:00020 d: 284 | A = OR[284]                   
0x0030 (0x000060) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0032), A # 0     
0x0031 (0x000062) 0x7002-       f:00070 d:   2 | P = P + 2 (0x0033)            
0x0032 (0x000064) 0x74A3-       f:00072 d: 163 | R = P + 163 (0x00D5)          
@ Set return code from OR[283] and return from overlay
0x0033 (0x000066) 0x2005-       f:00020 d:   5 | A = OR[5]                     
0x0034 (0x000068) 0x1406-       f:00012 d:   6 | A = A + 6 (0x0006)            
0x0035 (0x00006A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0036 (0x00006C) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x0037 (0x00006E) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0038 (0x000070) 0x102A-       f:00010 d:  42 | A = 42 (0x002A)               
0x0039 (0x000072) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x003A (0x000074) 0x1126-       f:00010 d: 294 | A = 294 (0x0126)              
0x003B (0x000076) 0x5800-       f:00054 d:   0 | B = A                         
0x003C (0x000078) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x003D (0x00007A) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Setup routine

@ Load device address from first parameter
0x003E (0x00007C) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x003F (0x00007E) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x0040 (0x000080) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0041 (0x000082) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0042 (0x000084) 0x2920-       f:00024 d: 288 | OR[288] = A                   
0x0043 (0x000086) 0x2120-       f:00020 d: 288 | A = OR[288]                   
0x0044 (0x000088) 0x290D-       f:00024 d: 269 | OR[269] = A                   

0x0045 (0x00008A) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                @ Volume ID
0x0046 (0x00008C) 0x290E-       f:00024 d: 270 | OR[270] = A                   
0x0047 (0x00008E) 0x2006-       f:00020 d:   6 | A = OR[6]                     @ Context
0x0048 (0x000090) 0x2910-       f:00024 d: 272 | OR[272] = A                   

@ Call XOPN
0x0049 (0x000092) 0x1028-       f:00010 d:  40 | A = 40 (0x0028)               
0x004A (0x000094) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x004B (0x000096) 0x1800-0x00A9 f:00014 d:   0 | A = 169 (0x00A9)              
0x004D (0x00009A) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x004E (0x00009C) 0x1800-0x444B f:00014 d:   0 | A = 17483 (0x444B)            @ Device type 'DK'
0x0050 (0x0000A0) 0x2928-       f:00024 d: 296 | OR[296] = A                   
0x0051 (0x0000A2) 0x210D-       f:00020 d: 269 | A = OR[269]                   @ Device index
0x0052 (0x0000A4) 0x2929-       f:00024 d: 297 | OR[297] = A                   
0x0053 (0x0000A6) 0x1003-       f:00010 d:   3 | A = 3 (0x0003)                @ Open mode (could be 3 or 4 for disks)
0x0054 (0x0000A8) 0x292A-       f:00024 d: 298 | OR[298] = A                   
0x0055 (0x0000AA) 0x210E-       f:00020 d: 270 | A = OR[270]                   @ Volume ID: 0 so no volume ID
0x0056 (0x0000AC) 0x292B-       f:00024 d: 299 | OR[299] = A                   
0x0057 (0x0000AE) 0x210F-       f:00020 d: 271 | A = OR[271]                   @ File number to open
0x0058 (0x0000B0) 0x292C-       f:00024 d: 300 | OR[300] = A                   
0x0059 (0x0000B2) 0x2110-       f:00020 d: 272 | A = OR[272]                   @ Context (comes from OR[6])
0x005A (0x0000B4) 0x292D-       f:00024 d: 301 | OR[301] = A                   
0x005B (0x0000B6) 0x1010-       f:00010 d:  16 | A = 16 (0x0010)               @ Context offset to store device descriptor address at
0x005C (0x0000B8) 0x292E-       f:00024 d: 302 | OR[302] = A                   
0x005D (0x0000BA) 0x1126-       f:00010 d: 294 | A = 294 (0x0126)              
0x005E (0x0000BC) 0x5800-       f:00054 d:   0 | B = A                         
0x005F (0x0000BE) 0x1800-0x1D18 f:00014 d:   0 | A = 7448 (0x1D18)             
0x0061 (0x0000C2) 0x7C09-       f:00076 d:   9 | R = OR[9]                     
@ Check return code, call return code converter if there was a failure and return from overlay
0x0062 (0x0000C4) 0x291B-       f:00024 d: 283 | OR[283] = A                   
0x0063 (0x0000C6) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x0064 (0x0000C8) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0066), A # 0     
0x0065 (0x0000CA) 0x7002-       f:00070 d:   2 | P = P + 2 (0x0067)            
0x0066 (0x0000CC) 0x7095-       f:00070 d: 149 | P = P + 149 (0x00FB)          

0x0067 (0x0000CE) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0068 (0x0000D0) 0x1402-       f:00012 d:   2 | A = A + 2 (0x0002)            
0x0069 (0x0000D2) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x006A (0x0000D4) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x006B (0x0000D6) 0x140A-       f:00012 d:  10 | A = A + 10 (0x000A)           
0x006C (0x0000D8) 0x2922-       f:00024 d: 290 | OR[290] = A                   

@ Call XFMACC - access expander disk file
0x006D (0x0000DA) 0x1028-       f:00010 d:  40 | A = 40 (0x0028)               
0x006E (0x0000DC) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x006F (0x0000DE) 0x1800-0x00E7 f:00014 d:   0 | A = 231 (0x00E7)              
0x0071 (0x0000E2) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x0072 (0x0000E4) 0x211C-       f:00020 d: 284 | A = OR[284]                   @ Device descriptor
0x0073 (0x0000E6) 0x2928-       f:00024 d: 296 | OR[296] = A                   
0x0074 (0x0000E8) 0x2121-       f:00020 d: 289 | A = OR[289]                   @ Directory name
0x0075 (0x0000EA) 0x2929-       f:00024 d: 297 | OR[297] = A                   
0x0076 (0x0000EC) 0x2122-       f:00020 d: 290 | A = OR[290]                   @ File name - directory name + 10
0x0077 (0x0000EE) 0x292A-       f:00024 d: 298 | OR[298] = A                   
0x0078 (0x0000F0) 0x1011-       f:00010 d:  17 | A = 17 (0x0011)               @ First output variable - somehow it's OR[285], I think - this is the file-handle
0x0079 (0x0000F2) 0x292B-       f:00024 d: 299 | OR[299] = A                   
0x007A (0x0000F4) 0x1017-       f:00010 d:  23 | A = 23 (0x0017)               @ Second output variable - somehow it's OR[291], I think - this is directory entry offset 25
0x007B (0x0000F6) 0x292C-       f:00024 d: 300 | OR[300] = A                   
0x007C (0x0000F8) 0x1018-       f:00010 d:  24 | A = 24 (0x0018)               @ Third output variable - somehow it's OR[292], I think - this is directory entry offset 26
0x007D (0x0000FA) 0x292D-       f:00024 d: 301 | OR[301] = A                   
0x007E (0x0000FC) 0x1126-       f:00010 d: 294 | A = 294 (0x0126)              
0x007F (0x0000FE) 0x5800-       f:00054 d:   0 | B = A                         
0x0080 (0x000100) 0x1800-0x1D18 f:00014 d:   0 | A = 7448 (0x1D18)             
0x0082 (0x000104) 0x7C09-       f:00076 d:   9 | R = OR[9]                     
@ Check return code, call return code converter if there was a failure and return from overlay
0x0083 (0x000106) 0x291B-       f:00024 d: 283 | OR[283] = A                   
0x0084 (0x000108) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x0085 (0x00010A) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0087), A # 0    
0x0086 (0x00010C) 0x7002-       f:00070 d:   2 | P = P + 2 (0x0088)            
0x0087 (0x00010E) 0x7074-       f:00070 d: 116 | P = P + 116 (0x00FB)          
@ XFMACC found the directory entry - copy returned values into desc. passed in (this is NOT the device desc. that's in OR[284])
0x0088 (0x000110) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0089 (0x000112) 0x1412-       f:00012 d:  18 | A = A + 18 (0x0012)           
0x008A (0x000114) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x008B (0x000116) 0x2123-       f:00020 d: 291 | A = OR[291]                   @ This is hopefully returned by XFMACC and contains OR[291] there: directory entry offset 25
0x008C (0x000118) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x008D (0x00011A) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x008E (0x00011C) 0x1413-       f:00012 d:  19 | A = A + 19 (0x0013)           
0x008F (0x00011E) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0090 (0x000120) 0x2124-       f:00020 d: 292 | A = OR[292]                   @ This is hopefully returned by XFMACC and contains OR[292] there: directory entry offset 26
0x0091 (0x000122) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0092 (0x000124) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Read sector from disk
@@ Input:
@@       OR[286]: 2k buffer
@@       OR[291]: directory entry offset 25 - file size in QWORDs - high word
@@       OR[292]: directory entry offset 26 - file size in QWORDs - low word
@@ Output:
@@       OR[287]: set to 1 if last sector
@ Clear out buffer
0x0093 (0x000126) 0x211E-       f:00020 d: 286 | A = OR[286]                   
0x0094 (0x000128) 0x290E-       f:00024 d: 270 | OR[270] = A                   
0x0095 (0x00012A) 0x1800-0x0800 f:00014 d:   0 | A = 2048 (0x0800)             
0x0097 (0x00012E) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x0098 (0x000130) 0x210D-       f:00020 d: 269 | A = OR[269]                   
0x0099 (0x000132) 0x8406-       f:00102 d:   6 | P = P + 6 (0x009F), A = 0     
0x009A (0x000134) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x009B (0x000136) 0x390E-       f:00034 d: 270 | (OR[270]) = A                 
0x009C (0x000138) 0x2F0D-       f:00027 d: 269 | OR[269] = OR[269] - 1         
0x009D (0x00013A) 0x2D0E-       f:00026 d: 270 | OR[270] = OR[270] + 1         
0x009E (0x00013C) 0x7206-       f:00071 d:   6 | P = P - 6 (0x0098)            
@ Check parameters
0x009F (0x00013E) 0x2123-       f:00020 d: 291 | A = OR[291]                   
0x00A0 (0x000140) 0x8402-       f:00102 d:   2 | P = P + 2 (0x00A2), A = 0     
0x00A1 (0x000142) 0x700D-       f:00070 d:  13 | P = P + 13 (0x00AE)           
0x00A2 (0x000144) 0x2124-       f:00020 d: 292 | A = OR[292]                   
0x00A3 (0x000146) 0x1E00-0x0200 f:00017 d:   0 | A = A - 512 (0x0200)          
0x00A5 (0x00014A) 0x8002-       f:00100 d:   2 | P = P + 2 (0x00A7), C = 0     
0x00A6 (0x00014C) 0x8602-       f:00103 d:   2 | P = P + 2 (0x00A8), A # 0    
0x00A7 (0x00014E) 0x7002-       f:00070 d:   2 | P = P + 2 (0x00A9)            
0x00A8 (0x000150) 0x7006-       f:00070 d:   6 | P = P + 6 (0x00AE)            
@ OR[291] == 0 and OR[292] <= 512 -> set remainder in OR[293], and 1 in OR[287], which will terminates the loop, read the last sector and return
0x00A9 (0x000152) 0x2124-       f:00020 d: 292 | A = OR[292]                   
0x00AA (0x000154) 0x2925-       f:00024 d: 293 | OR[293] = A                   
0x00AB (0x000156) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x00AC (0x000158) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x00AD (0x00015A) 0x700D-       f:00070 d:  13 | P = P + 13 (0x00BA)           
@ We have at least 512 QWORDs left to read: decrement number of QWORDs left and issue another read
0x00AE (0x00015C) 0x1800-0x0200 f:00014 d:   0 | A = 512 (0x0200)              
0x00B0 (0x000160) 0x2925-       f:00024 d: 293 | OR[293] = A                   
0x00B1 (0x000162) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x00B2 (0x000164) 0x290F-       f:00024 d: 271 | OR[271] = A                   
0x00B3 (0x000166) 0x2124-       f:00020 d: 292 | A = OR[292]                   
0x00B4 (0x000168) 0x2725-       f:00023 d: 293 | A = A - OR[293]               
0x00B5 (0x00016A) 0x2924-       f:00024 d: 292 | OR[292] = A                   
0x00B6 (0x00016C) 0x8202-       f:00101 d:   2 | P = P + 2 (0x00B8), C = 1     
0x00B7 (0x00016E) 0x2F23-       f:00027 d: 291 | OR[291] = OR[291] - 1         
0x00B8 (0x000170) 0x0810-       f:00004 d:  16 | A = A > 16 (0x0010)           
0x00B9 (0x000172) 0x230F-       f:00021 d: 271 | A = A & OR[271]               
@ Call XFMIO
0x00BA (0x000174) 0x1028-       f:00010 d:  40 | A = 40 (0x0028)               
0x00BB (0x000176) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x00BC (0x000178) 0x1800-0x00F2 f:00014 d:   0 | A = 242 (0x00F2)              
0x00BE (0x00017C) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x00BF (0x00017E) 0x211C-       f:00020 d: 284 | A = OR[284]                   @ Device desc.
0x00C0 (0x000180) 0x2928-       f:00024 d: 296 | OR[296] = A                   
0x00C1 (0x000182) 0x211D-       f:00020 d: 285 | A = OR[285]                   @ File-handle
0x00C2 (0x000184) 0x2929-       f:00024 d: 297 | OR[297] = A                   
0x00C3 (0x000186) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                @ Intended operation: 1 - read
0x00C4 (0x000188) 0x292A-       f:00024 d: 298 | OR[298] = A                   
0x00C5 (0x00018A) 0x1800-0xFFFF f:00014 d:   0 | A = 65535 (0xFFFF)            @ Target sector not specified: read next sector
0x00C7 (0x00018E) 0x292B-       f:00024 d: 299 | OR[299] = A                   
0x00C8 (0x000190) 0x211E-       f:00020 d: 286 | A = OR[286]                   @ Buffer
0x00C9 (0x000192) 0x292C-       f:00024 d: 300 | OR[300] = A                   
0x00CA (0x000194) 0x1126-       f:00010 d: 294 | A = 294 (0x0126)              
0x00CB (0x000196) 0x5800-       f:00054 d:   0 | B = A                         
0x00CC (0x000198) 0x1800-0x1D18 f:00014 d:   0 | A = 7448 (0x1D18)             
0x00CE (0x00019C) 0x7C09-       f:00076 d:   9 | R = OR[9]                     
@ Check return code, call return code converter if there was a failure and return from overlay
0x00CF (0x00019E) 0x291B-       f:00024 d: 283 | OR[283] = A                   
0x00D0 (0x0001A0) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x00D1 (0x0001A2) 0x8602-       f:00103 d:   2 | P = P + 2 (0x00D3), A # 0     
0x00D2 (0x0001A4) 0x7002-       f:00070 d:   2 | P = P + 2 (0x00D4)            
0x00D3 (0x0001A6) 0x7028-       f:00070 d:  40 | P = P + 40 (0x00FB)           
0x00D4 (0x0001A8) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Overlay epilog: call XFMCLS and XCLS
0x00D5 (0x0001AA) 0x211D-       f:00020 d: 285 | A = OR[285]                   
0x00D6 (0x0001AC) 0x8602-       f:00103 d:   2 | P = P + 2 (0x00D8), A # 0     
0x00D7 (0x0001AE) 0x7013-       f:00070 d:  19 | P = P + 19 (0x00EA)           
@ Call XFMCLS
0x00D8 (0x0001B0) 0x1028-       f:00010 d:  40 | A = 40 (0x0028)               
0x00D9 (0x0001B2) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x00DA (0x0001B4) 0x1800-0x00E8 f:00014 d:   0 | A = 232 (0x00E8)              
0x00DC (0x0001B8) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x00DD (0x0001BA) 0x211C-       f:00020 d: 284 | A = OR[284]                   
0x00DE (0x0001BC) 0x2928-       f:00024 d: 296 | OR[296] = A                   
0x00DF (0x0001BE) 0x211D-       f:00020 d: 285 | A = OR[285]                   
0x00E0 (0x0001C0) 0x2929-       f:00024 d: 297 | OR[297] = A                   
0x00E1 (0x0001C2) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00E2 (0x0001C4) 0x292A-       f:00024 d: 298 | OR[298] = A                   
0x00E3 (0x0001C6) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00E4 (0x0001C8) 0x292B-       f:00024 d: 299 | OR[299] = A                   
0x00E5 (0x0001CA) 0x1126-       f:00010 d: 294 | A = 294 (0x0126)              
0x00E6 (0x0001CC) 0x5800-       f:00054 d:   0 | B = A                         
0x00E7 (0x0001CE) 0x1800-0x1D18 f:00014 d:   0 | A = 7448 (0x1D18)             
0x00E9 (0x0001D2) 0x7C09-       f:00076 d:   9 | R = OR[9]                     
@ Call XCLS
0x00EA (0x0001D4) 0x1006-       f:00010 d:   6 | A = 6 (0x0006)                
0x00EB (0x0001D6) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x00EC (0x0001D8) 0x1028-       f:00010 d:  40 | A = 40 (0x0028)               
0x00ED (0x0001DA) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x00EE (0x0001DC) 0x1800-0x00A5 f:00014 d:   0 | A = 165 (0x00A5)              
0x00F0 (0x0001E0) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x00F1 (0x0001E2) 0x211C-       f:00020 d: 284 | A = OR[284]                   
0x00F2 (0x0001E4) 0x2928-       f:00024 d: 296 | OR[296] = A                   
0x00F3 (0x0001E6) 0x210D-       f:00020 d: 269 | A = OR[269]                   
0x00F4 (0x0001E8) 0x2929-       f:00024 d: 297 | OR[297] = A                   
0x00F5 (0x0001EA) 0x1126-       f:00010 d: 294 | A = 294 (0x0126)              
0x00F6 (0x0001EC) 0x5800-       f:00054 d:   0 | B = A                         
0x00F7 (0x0001EE) 0x1800-0x1D18 f:00014 d:   0 | A = 7448 (0x1D18)             
0x00F9 (0x0001F2) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

0x00FA (0x0001F4) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Converter return code from the likes of XOPN and return from overlay
@@ Input:
@@      OR[283]: error return code from overlay, like XOPN
@@
0x00FB (0x0001F6) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x00FC (0x0001F8) 0x1605-       f:00013 d:   5 | A = A - 5 (0x0005)            
0x00FD (0x0001FA) 0x8402-       f:00102 d:   2 | P = P + 2 (0x00FF), A = 0     
0x00FE (0x0001FC) 0x7005-       f:00070 d:   5 | P = P + 5 (0x0103)            
0x00FF (0x0001FE) 0x1800-0x0646 f:00014 d:   0 | A = 1606 (0x0646)             
0x0101 (0x000202) 0x291B-       f:00024 d: 283 | OR[283] = A                   
0x0102 (0x000204) 0x7023-       f:00070 d:  35 | P = P + 35 (0x0125)           
0x0103 (0x000206) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x0104 (0x000208) 0x160A-       f:00013 d:  10 | A = A - 10 (0x000A)           
0x0105 (0x00020A) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0107), A = 0     
0x0106 (0x00020C) 0x7005-       f:00070 d:   5 | P = P + 5 (0x010B)            
0x0107 (0x00020E) 0x1800-0x064E f:00014 d:   0 | A = 1614 (0x064E)             
0x0109 (0x000212) 0x291B-       f:00024 d: 283 | OR[283] = A                   
0x010A (0x000214) 0x701B-       f:00070 d:  27 | P = P + 27 (0x0125)           
0x010B (0x000216) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x010C (0x000218) 0x1608-       f:00013 d:   8 | A = A - 8 (0x0008)            
0x010D (0x00021A) 0x8402-       f:00102 d:   2 | P = P + 2 (0x010F), A = 0     
0x010E (0x00021C) 0x7005-       f:00070 d:   5 | P = P + 5 (0x0113)            
0x010F (0x00021E) 0x1800-0x0649 f:00014 d:   0 | A = 1609 (0x0649)             
0x0111 (0x000222) 0x291B-       f:00024 d: 283 | OR[283] = A                   
0x0112 (0x000224) 0x7013-       f:00070 d:  19 | P = P + 19 (0x0125)           
0x0113 (0x000226) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x0114 (0x000228) 0x1609-       f:00013 d:   9 | A = A - 9 (0x0009)            
0x0115 (0x00022A) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0117), A = 0     
0x0116 (0x00022C) 0x7005-       f:00070 d:   5 | P = P + 5 (0x011B)            
0x0117 (0x00022E) 0x1800-0x064D f:00014 d:   0 | A = 1613 (0x064D)             
0x0119 (0x000232) 0x291B-       f:00024 d: 283 | OR[283] = A                   
0x011A (0x000234) 0x700B-       f:00070 d:  11 | P = P + 11 (0x0125)           
0x011B (0x000236) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x011C (0x000238) 0x1602-       f:00013 d:   2 | A = A - 2 (0x0002)            
0x011D (0x00023A) 0x8405-       f:00102 d:   5 | P = P + 5 (0x0122), A = 0     
0x011E (0x00023C) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x011F (0x00023E) 0x1603-       f:00013 d:   3 | A = A - 3 (0x0003)            
0x0120 (0x000240) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0122), A = 0     
0x0121 (0x000242) 0x7004-       f:00070 d:   4 | P = P + 4 (0x0125)            
0x0122 (0x000244) 0x1800-0x064C f:00014 d:   0 | A = 1612 (0x064C)             
0x0124 (0x000248) 0x291B-       f:00024 d: 283 | OR[283] = A                   
0x0125 (0x00024A) 0x7301-       f:00071 d: 257 | P = P - 257 (0x0024)          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Start transfer of block of memory to the mainframe, update target address
@@ Input:
@@   OR[281]-OR[282]: Target address
@@   OR[286]: Local memory address
@@   OR[293]: Buffer length
@@ Output:
@@   OR[281]-OR[282]: Updated target address
0x0126 (0x00024C) 0x2119-       f:00020 d: 281 | A = OR[281]                   @ Test if target address is zero, if not, call HPSW immediately
0x0127 (0x00024E) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0129), A = 0     
0x0128 (0x000250) 0x7009-       f:00070 d:   9 | P = P + 9 (0x0131)            
0x0129 (0x000252) 0x211A-       f:00020 d: 282 | A = OR[282]                   
0x012A (0x000254) 0x8402-       f:00102 d:   2 | P = P + 2 (0x012C), A = 0     
0x012B (0x000256) 0x7006-       f:00070 d:   6 | P = P + 6 (0x0131)            
0x012C (0x000258) 0x1020-       f:00010 d:  32 | A = 32 (0x0020)               @ Target address is 0: chop off the first 8 QWORDS of the buffer before transfer. This is to get rid of the various headers in the image file
0x012D (0x00025A) 0x2B1E-       f:00025 d: 286 | OR[286] = A + OR[286]         
0x012E (0x00025C) 0x2125-       f:00020 d: 293 | A = OR[293]                   
0x012F (0x00025E) 0x1608-       f:00013 d:   8 | A = A - 8 (0x0008)            
0x0130 (0x000260) 0x2925-       f:00024 d: 293 | OR[293] = A                   
@ Call HPSW (function 35) - hand over buffer to BIOP and transfer to mainframe memory
0x0131 (0x000262) 0x1023-       f:00010 d:  35 | A = 35 (0x0023)               @ Kernel function code (HPSW)
0x0132 (0x000264) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x0133 (0x000266) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                @ Target memory type: Central memory
0x0134 (0x000268) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x0135 (0x00026A) 0x2119-       f:00020 d: 281 | A = OR[281]                   @ Target memory address high
0x0136 (0x00026C) 0x2928-       f:00024 d: 296 | OR[296] = A                   
0x0137 (0x00026E) 0x211A-       f:00020 d: 282 | A = OR[282]                   @ Target memory address low
0x0138 (0x000270) 0x2929-       f:00024 d: 297 | OR[297] = A                   
0x0139 (0x000272) 0x211E-       f:00020 d: 286 | A = OR[286]                   @ Local memory address
0x013A (0x000274) 0x292A-       f:00024 d: 298 | OR[298] = A                   
0x013B (0x000276) 0x2125-       f:00020 d: 293 | A = OR[293]                   @ Buffer length
0x013C (0x000278) 0x292B-       f:00024 d: 299 | OR[299] = A                   
0x013D (0x00027A) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                @ Nowait: return immediately
0x013E (0x00027C) 0x292C-       f:00024 d: 300 | OR[300] = A                   
0x013F (0x00027E) 0x1126-       f:00010 d: 294 | A = 294 (0x0126)              
0x0140 (0x000280) 0x5800-       f:00054 d:   0 | B = A                         
0x0141 (0x000282) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0142 (0x000284) 0x7C09-       f:00076 d:   9 | R = OR[9]                     
@ Back from HPSW
0x0143 (0x000286) 0x2119-       f:00020 d: 281 | A = OR[281]                   @ Test if target address is zero. If it is, patch back the size and local address fields we've fixed up just before the call
0x0144 (0x000288) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0146), A = 0     
0x0145 (0x00028A) 0x7007-       f:00070 d:   7 | P = P + 7 (0x014C)            
0x0146 (0x00028C) 0x211A-       f:00020 d: 282 | A = OR[282]                   
0x0147 (0x00028E) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0149), A = 0     
0x0148 (0x000290) 0x7004-       f:00070 d:   4 | P = P + 4 (0x014C)            
0x0149 (0x000292) 0x211E-       f:00020 d: 286 | A = OR[286]                   
0x014A (0x000294) 0x1620-       f:00013 d:  32 | A = A - 32 (0x0020)           
0x014B (0x000296) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x014C (0x000298) 0x211A-       f:00020 d: 282 | A = OR[282]                   @ Move target address by the transfer length
0x014D (0x00029A) 0x2525-       f:00022 d: 293 | A = A + OR[293]               
0x014E (0x00029C) 0x291A-       f:00024 d: 282 | OR[282] = A                   
0x014F (0x00029E) 0x8002-       f:00100 d:   2 | P = P + 2 (0x0151), C = 0     
0x0150 (0x0002A0) 0x2D19-       f:00026 d: 281 | OR[281] = OR[281] + 1         
0x0151 (0x0002A2) 0x0200-       f:00001 d:   0 | EXIT                          
0x0152 (0x0002A4) 0x0000-       f:00000 d:   0 | PASS                          
0x0153 (0x0002A6) 0x0000-       f:00000 d:   0 | PASS                          





0x0141 (0x000282) 0x1800-0x1D18 f:00014 d:   0 | A = 7448 (0x1D18)             
0x0143 (0x000284) 0x7C09-       f:00076 d:   9 | R = OR[9]                     
@ Back from HPSW
0x0144 (0x000286) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x0145 (0x000288) 0x8607-       f:00103 d:   7 | P = P + 7 (0x014C), A # 0     
