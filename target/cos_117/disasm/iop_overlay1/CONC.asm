@@@@@@@@@@@@@@@@@@ Gets loaded into 0x62c0 (0x62c6)
@@ Input:
@@ OR[280]: could be 0 - upper byte: concentrator ordinal? number in character

@@ OR[289]:
@@ OR[256]:
@@ OR[257]: pointer to some kind of a 'concentrator ordinal' desc.
@@      offset 0: some bit-field
@@                bit 15 - 1: concentrator active
@@                bit 14 - 1: initialization in progress
@@                bit 13 - 1: termination in progress
@@                bit 12 - 1: channel disabled
@@                bit 11 - 1:
@@                bit 10 - 1:
@@                bit 9  - 1:
@@                bit 8  - 1:
@@                bit 7...0: ???????? should be 30 (0x1e) ??????
@@      offset 1: lower byte: CI channel number
@@      offset 2: local buffer address (68 WORDs long) - see CONCIO for details of how this is layed out
@@      offset 3:
@@      offset 4:
@@      offset 5:
@@      offset 6:
@@ OR[259]: 'concentrator ordinal' ???
@ Call overlay CRAY
0x0000 (0x000000) 0x1028-       f:00010 d:  40 | A = 40 (0x0028)               
0x0001 (0x000002) 0x2929-       f:00024 d: 297 | OR[297] = A                   
0x0002 (0x000004) 0x1800-0x0017 f:00014 d:   0 | A = 23 (0x0017)               
0x0004 (0x000008) 0x292A-       f:00024 d: 298 | OR[298] = A                   
0x0005 (0x00000A) 0x1800-0xFFFF f:00014 d:   0 | A = 65535 (0xFFFF)            
0x0007 (0x00000E) 0x292B-       f:00024 d: 299 | OR[299] = A                   
0x0008 (0x000010) 0x1129-       f:00010 d: 297 | A = 297 (0x0129)              
0x0009 (0x000012) 0x5800-       f:00054 d:   0 | B = A                         
0x000A (0x000014) 0x1800-0x2318 f:00014 d:   0 | A = 8984 (0x2318)             
0x000C (0x000018) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

0x000D (0x00001A) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x000E (0x00001C) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x000F (0x00001E) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0010 (0x000020) 0x2900-       f:00024 d: 256 | OR[256] = A                   
0x0011 (0x000022) 0x2118-       f:00020 d: 280 | A = OR[280]                   @ Test upper byte of OR[280]
0x0012 (0x000024) 0x0808-       f:00004 d:   8 | A = A > 8 (0x0008)            
0x0013 (0x000026) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0014 (0x000028) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0015 (0x00002A) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x0016 (0x00002C) 0x8405-       f:00102 d:   5 | P = P + 5 (0x001B), A = 0     
0x0017 (0x00002E) 0x2118-       f:00020 d: 280 | A = OR[280]                   @ If non-0, shift by 8, and convert from numerical digit to byte
0x0018 (0x000030) 0x0808-       f:00004 d:   8 | A = A > 8 (0x0008)            
0x0019 (0x000032) 0x1630-       f:00013 d:  48 | A = A - 48 (0x0030)           
0x001A (0x000034) 0x2918-       f:00024 d: 280 | OR[280] = A                   
0x001B (0x000036) 0x2118-       f:00020 d: 280 | A = OR[280]                   @ Load concentrator ordinal? 
0x001C (0x000038) 0x1403-       f:00012 d:   3 | A = A + 3 (0x0003)            
0x001D (0x00003A) 0x2903-       f:00024 d: 259 | OR[259] = A                   @ OR[259]: concentrator ordinal?  + 3
0x001E (0x00003C) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x001F (0x00003E) 0x2901-       f:00024 d: 257 | OR[257] = A                   
0x0020 (0x000040) 0x2103-       f:00020 d: 259 | A = OR[259]                   
0x0021 (0x000042) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x0022 (0x000044) 0x210D-       f:00020 d: 269 | A = OR[269]                   
0x0023 (0x000046) 0x1607-       f:00013 d:   7 | A = A - 7 (0x0007)            
0x0024 (0x000048) 0x8002-       f:00100 d:   2 | P = P + 2 (0x0026), C = 0     
0x0025 (0x00004A) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0027), A # 0     
0x0026 (0x00004C) 0x7002-       f:00070 d:   2 | P = P + 2 (0x0028)            
0x0027 (0x00004E) 0x700F-       f:00070 d:  15 | P = P + 15 (0x0036)           
0x0028 (0x000050) 0x210D-       f:00020 d: 269 | A = OR[269]                   
0x0029 (0x000052) 0x8602-       f:00103 d:   2 | P = P + 2 (0x002B), A # 0     
0x002A (0x000054) 0x700C-       f:00070 d:  12 | P = P + 12 (0x0036)           
@ We get here if OR[269] < 7, but non-0 - it is the concentrator ordinal? + 3
0x002B (0x000056) 0x2048-       f:00020 d:  72 | A = OR[72]                    @ OR[72] points to 'FE' table
0x002C (0x000058) 0x1402-       f:00012 d:   2 | A = A + 2 (0x0002)            
0x002D (0x00005A) 0x290E-       f:00024 d: 270 | OR[270] = A                   @ OR[270] starts at 'FE' table offset 2
0x002E (0x00005C) 0x210D-       f:00020 d: 269 | A = OR[269]                   @ Terminate loop if OR[269] == 1
0x002F (0x00005E) 0x1601-       f:00013 d:   1 | A = A - 1 (0x0001)            
0x0030 (0x000060) 0x8405-       f:00102 d:   5 | P = P + 5 (0x0035), A = 0     
0x0031 (0x000062) 0x1007-       f:00010 d:   7 | A = 7 (0x0007)                @ Increment offset by 7
0x0032 (0x000064) 0x2B0E-       f:00025 d: 270 | OR[270] = A + OR[270]         
0x0033 (0x000066) 0x2F0D-       f:00027 d: 269 | OR[269] = OR[269] - 1         @ Decrement loop index
0x0034 (0x000068) 0x7206-       f:00071 d:   6 | P = P - 6 (0x002E)            @ And keep looping

0x0035 (0x00006A) 0x7003-       f:00070 d:   3 | P = P + 3 (0x0038)            
0x0036 (0x00006C) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0037 (0x00006E) 0x290E-       f:00024 d: 270 | OR[270] = A                   
@ We have OR[270] set up to point to the concentrator ordinal? desc: it's at 'FE' table + 2 + 7*concentrator ordinal (or 0 in case of an error???)
0x0038 (0x000070) 0x210E-       f:00020 d: 270 | A = OR[270]                   
0x0039 (0x000072) 0x2901-       f:00024 d: 257 | OR[257] = A                   @ Point OR[257] to the 'concentrator ordinal' desc, which is 7 entries long
0x003A (0x000074) 0x2101-       f:00020 d: 257 | A = OR[257]                   
0x003B (0x000076) 0x8402-       f:00102 d:   2 | P = P + 2 (0x003D), A = 0     
0x003C (0x000078) 0x7007-       f:00070 d:   7 | P = P + 7 (0x0043)            
0x003D (0x00007A) 0x1800-0x0259 f:00014 d:   0 | A = 601 (0x0259)              @ Print error message 'Concentrator ordinal' 'not configured. Command aborted.'
0x003F (0x00007E) 0x2919-       f:00024 d: 281 | OR[281] = A                   
0x0040 (0x000080) 0x1010-       f:00010 d:  16 | A = 16 (0x0010)               
0x0041 (0x000082) 0x291A-       f:00024 d: 282 | OR[282] = A                   
0x0042 (0x000084) 0x715B-       f:00070 d: 347 | P = P + 347 (0x019D)          
@ Concentrator ordinal is fine
0x0043 (0x000086) 0x3101-       f:00030 d: 257 | A = (OR[257])                 @ Read entry 0, bit 13 into OR[290]
0x0044 (0x000088) 0x080D-       f:00004 d:  13 | A = A > 13 (0x000D)           
0x0045 (0x00008A) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x0046 (0x00008C) 0x2922-       f:00024 d: 290 | OR[290] = A                   
0x0047 (0x00008E) 0x3101-       f:00030 d: 257 | A = (OR[257])                 @ Read entry 0, bit 14 into OR[291]
0x0048 (0x000090) 0x080E-       f:00004 d:  14 | A = A > 14 (0x000E)           
0x0049 (0x000092) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x004A (0x000094) 0x2923-       f:00024 d: 291 | OR[291] = A                   
0x004B (0x000096) 0x3101-       f:00030 d: 257 | A = (OR[257])                 @ Read entry 0, bit 15 into OR[292]
0x004C (0x000098) 0x080F-       f:00004 d:  15 | A = A > 15 (0x000F)           
0x004D (0x00009A) 0x2924-       f:00024 d: 292 | OR[292] = A                   
0x004E (0x00009C) 0x3101-       f:00030 d: 257 | A = (OR[257])                 @ Read entry 0, bit 12 into OR[293]
0x004F (0x00009E) 0x080C-       f:00004 d:  12 | A = A > 12 (0x000C)           
0x0050 (0x0000A0) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x0051 (0x0000A2) 0x2925-       f:00024 d: 293 | OR[293] = A                   
0x0052 (0x0000A4) 0x3101-       f:00030 d: 257 | A = (OR[257])                 @ Read entry, lower byte into OR[294]
0x0053 (0x0000A6) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x0054 (0x0000A8) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x0055 (0x0000AA) 0x2122-       f:00020 d: 290 | A = OR[290]                   @ Check if any of bits 12...15 is set
0x0056 (0x0000AC) 0x2523-       f:00022 d: 291 | A = A + OR[291]               
0x0057 (0x0000AE) 0x2524-       f:00022 d: 292 | A = A + OR[292]               
0x0058 (0x0000B0) 0x2525-       f:00022 d: 293 | A = A + OR[293]               
0x0059 (0x0000B2) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x005A (0x0000B4) 0x2127-       f:00020 d: 295 | A = OR[295]                   
0x005B (0x0000B6) 0x8605-       f:00103 d:   5 | P = P + 5 (0x0060), A # 0    
0x005C (0x0000B8) 0x2126-       f:00020 d: 294 | A = OR[294]                   @ Check if lower byte is 30
0x005D (0x0000BA) 0x161E-       f:00013 d:  30 | A = A - 30 (0x001E)           
0x005E (0x0000BC) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0060), A # 0     

0x005F (0x0000BE) 0x702B-       f:00070 d:  43 | P = P + 43 (0x008A)           @ Lower byte is 30 and none of the highest nibble bits are set
@@ Error handling:
@ Either the lower byte is not 30 or the some of the highest nibble bits are set
0x0060 (0x0000C0) 0x2122-       f:00020 d: 290 | A = OR[290]                   @ This is bit 13
0x0061 (0x0000C2) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0063), A # 0    
0x0062 (0x0000C4) 0x7007-       f:00070 d:   7 | P = P + 7 (0x0069)            
0x0063 (0x0000C6) 0x1800-0x026A f:00014 d:   0 | A = 618 (0x026A)              @ Bit 13 is set - print 'Concentrator ordinal' 'termination in progress. Command aborted.'
0x0065 (0x0000CA) 0x2919-       f:00024 d: 281 | OR[281] = A                   
0x0066 (0x0000CC) 0x1015-       f:00010 d:  21 | A = 21 (0x0015)               
0x0067 (0x0000CE) 0x291A-       f:00024 d: 282 | OR[282] = A                   
0x0068 (0x0000D0) 0x7021-       f:00070 d:  33 | P = P + 33 (0x0089)           
0x0069 (0x0000D2) 0x2123-       f:00020 d: 291 | A = OR[291]                   @ This is bit 14
0x006A (0x0000D4) 0x8602-       f:00103 d:   2 | P = P + 2 (0x006C), A # 0    
0x006B (0x0000D6) 0x7007-       f:00070 d:   7 | P = P + 7 (0x0072)            
0x006C (0x0000D8) 0x1800-0x027F f:00014 d:   0 | A = 639 (0x027F)              @ Bit 14 is set - print 'Concentrator ordinal' 'initialization in progress. Command aborted.'
0x006E (0x0000DC) 0x2919-       f:00024 d: 281 | OR[281] = A                   
0x006F (0x0000DE) 0x1016-       f:00010 d:  22 | A = 22 (0x0016)               
0x0070 (0x0000E0) 0x291A-       f:00024 d: 282 | OR[282] = A                   
0x0071 (0x0000E2) 0x7018-       f:00070 d:  24 | P = P + 24 (0x0089)           
0x0072 (0x0000E4) 0x2124-       f:00020 d: 292 | A = OR[292]                   @ This is bit 15
0x0073 (0x0000E6) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0075), A # 0    
0x0074 (0x0000E8) 0x7007-       f:00070 d:   7 | P = P + 7 (0x007B)            
0x0075 (0x0000EA) 0x1800-0x0296 f:00014 d:   0 | A = 662 (0x0296)              @ Bit 15 is set - print 'Concentrator ordinal' 'already active. Command aborted.'
0x0077 (0x0000EE) 0x2919-       f:00024 d: 281 | OR[281] = A                   
0x0078 (0x0000F0) 0x1010-       f:00010 d:  16 | A = 16 (0x0010)               
0x0079 (0x0000F2) 0x291A-       f:00024 d: 282 | OR[282] = A                   
0x007A (0x0000F4) 0x700F-       f:00070 d:  15 | P = P + 15 (0x0089)           
0x007B (0x0000F6) 0x2125-       f:00020 d: 293 | A = OR[293]                   @ This is bit 12
0x007C (0x0000F8) 0x8602-       f:00103 d:   2 | P = P + 2 (0x007E), A # 0    
0x007D (0x0000FA) 0x7007-       f:00070 d:   7 | P = P + 7 (0x0084)            
0x007E (0x0000FC) 0x1800-0x02BD f:00014 d:   0 | A = 701 (0x02BD)              @ Bit 12 is set - print 'Concentrator ordinal' 'channel disabled. Command aborted.'
0x0080 (0x000100) 0x2919-       f:00024 d: 281 | OR[281] = A                   
0x0081 (0x000102) 0x1011-       f:00010 d:  17 | A = 17 (0x0011)               
0x0082 (0x000104) 0x291A-       f:00024 d: 282 | OR[282] = A                   
0x0083 (0x000106) 0x7006-       f:00070 d:   6 | P = P + 6 (0x0089)            
@ None of the bits are set, that is lower byte is not 30
0x0084 (0x000108) 0x1800-0x02A7 f:00014 d:   0 | A = 679 (0x02A7)              @ Print error message: 'Concentrator ordinal' 'not configured correctly. Command aborted.'
0x0086 (0x00010C) 0x2919-       f:00024 d: 281 | OR[281] = A                   
0x0087 (0x00010E) 0x1015-       f:00010 d:  21 | A = 21 (0x0015)               
0x0088 (0x000110) 0x291A-       f:00024 d: 282 | OR[282] = A                   
0x0089 (0x000112) 0x7114-       f:00070 d: 276 | P = P + 276 (0x019D)          

@ Concentrator state is OK, continue
0x008A (0x000114) 0x3101-       f:00030 d: 257 | A = (OR[257])                 @ Read bit 8 of offset 0 into OR[290]
0x008B (0x000116) 0x0808-       f:00004 d:   8 | A = A > 8 (0x0008)            
0x008C (0x000118) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x008D (0x00011A) 0x2922-       f:00024 d: 290 | OR[290] = A                   
0x008E (0x00011C) 0x2122-       f:00020 d: 290 | A = OR[290]                   
0x008F (0x00011E) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0091), A # 0     
0x0090 (0x000120) 0x700F-       f:00070 d:  15 | P = P + 15 (0x009F)           
@ If bit-8 is set, set bit-9 and call runtime function 4???
0x0091 (0x000122) 0x3101-       f:00030 d: 257 | A = (OR[257])                 @ Set bit 9 of entry 0
0x0092 (0x000124) 0x0E07-       f:00007 d:   7 | A = A << 7 (0x0007)           
0x0093 (0x000126) 0x0A01-       f:00005 d:   1 | A = A < 1 (0x0001)            
0x0094 (0x000128) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x0095 (0x00012A) 0x0C08-       f:00006 d:   8 | A = A >> 8 (0x0008)           
0x0096 (0x00012C) 0x3901-       f:00034 d: 257 | (OR[257]) = A                 
0x0097 (0x00012E) 0x1004-       f:00010 d:   4 | A = 4 (0x0004)                @ Call runtime function 4 - this must occasionally clear bit 8 (but how!! and WHY????) otherwise we never exit the loop here...
0x0098 (0x000130) 0x2929-       f:00024 d: 297 | OR[297] = A                   
0x0099 (0x000132) 0x1129-       f:00010 d: 297 | A = 297 (0x0129)              
0x009A (0x000134) 0x5800-       f:00054 d:   0 | B = A                         
0x009B (0x000136) 0x1800-0x2318 f:00014 d:   0 | A = 8984 (0x2318)             
0x009D (0x00013A) 0x7C09-       f:00076 d:   9 | R = OR[9]                     
0x009E (0x00013C) 0x7214-       f:00071 d:  20 | P = P - 20 (0x008A)           
@ We're done calling runtime function 4 for whatever reason...
0x009F (0x00013E) 0x3101-       f:00030 d: 257 | A = (OR[257])                 @ Set bit 14 of entry 1: initialization in progress
0x00A0 (0x000140) 0x0E02-       f:00007 d:   2 | A = A << 2 (0x0002)           
0x00A1 (0x000142) 0x0A01-       f:00005 d:   1 | A = A < 1 (0x0001)            
0x00A2 (0x000144) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x00A3 (0x000146) 0x0C03-       f:00006 d:   3 | A = A >> 3 (0x0003)           
0x00A4 (0x000148) 0x3901-       f:00034 d: 257 | (OR[257]) = A                 
0x00A5 (0x00014A) 0x2101-       f:00020 d: 257 | A = OR[257]                   @ Read channel number (offset 1) into OR[258]
0x00A6 (0x00014C) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x00A7 (0x00014E) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00A8 (0x000150) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x00A9 (0x000152) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x00AA (0x000154) 0x2902-       f:00024 d: 258 | OR[258] = A                   @ OR[258] contains the CI channel number
0x00AB (0x000156) 0x2102-       f:00020 d: 258 | A = OR[258]                   
0x00AC (0x000158) 0x5800-       f:00054 d:   0 | B = A                         
0x00AD (0x00015A) 0xE000-       f:00160 d:   0 | IOB , fn000                   @ Clear channel
0x00AE (0x00015C) 0xE600-       f:00163 d:   0 | IOB , fn003                   @ Clear channel parity error flags
0x00AF (0x00015E) 0xEC00-       f:00166 d:   0 | IOB , fn006                   @ Clear channel interrupt enable
0x00B0 (0x000160) 0x2102-       f:00020 d: 258 | A = OR[258]                   
0x00B1 (0x000162) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            @ Advance to the CO channel
0x00B2 (0x000164) 0x5800-       f:00054 d:   0 | B = A                         
0x00B3 (0x000166) 0xE000-       f:00160 d:   0 | IOB , fn000                   @ Clear channel
0x00B4 (0x000168) 0xE600-       f:00163 d:   0 | IOB , fn003                   @ Clear channel error flags
0x00B5 (0x00016A) 0x1800-0x4000 f:00014 d:   0 | A = 16384 (0x4000)            @ This would be the I/O reset normally, but what is it here???
0x00B7 (0x00016E) 0xE800-       f:00164 d:   0 | IOB , fn004                   
@ Call YIELD
0x00B8 (0x000170) 0x1007-       f:00010 d:   7 | A = 7 (0x0007)                
0x00B9 (0x000172) 0x2929-       f:00024 d: 297 | OR[297] = A                   
0x00BA (0x000174) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x00BB (0x000176) 0x292A-       f:00024 d: 298 | OR[298] = A                   
0x00BC (0x000178) 0x1129-       f:00010 d: 297 | A = 297 (0x0129)              
0x00BD (0x00017A) 0x5800-       f:00054 d:   0 | B = A                         
0x00BE (0x00017C) 0x1800-0x2318 f:00014 d:   0 | A = 8984 (0x2318)             
0x00C0 (0x000180) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

0x00C1 (0x000182) 0x2102-       f:00020 d: 258 | A = OR[258]                   @ Load CO channel number
0x00C2 (0x000184) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x00C3 (0x000186) 0x5800-       f:00054 d:   0 | B = A                         
0x00C4 (0x000188) 0xE600-       f:00163 d:   0 | IOB , fn003                   @ Clear channel error flags
0x00C5 (0x00018A) 0x1800-0x0200 f:00014 d:   0 | A = 512 (0x0200)              @ Set hold-disconnect and get concentrator out of reset
0x00C7 (0x00018E) 0xE800-       f:00164 d:   0 | IOB , fn004                   
0x00C8 (0x000190) 0xE000-       f:00160 d:   0 | IOB , fn000                   @ Clear channel
0x00C9 (0x000192) 0xEC00-       f:00166 d:   0 | IOB , fn006                   @ Clear channel interrupt enable
@ Allocate 68 words of buffer, return in OR[256]
0x00CA (0x000194) 0x1018-       f:00010 d:  24 | A = 24 (0x0018)               
0x00CB (0x000196) 0x2929-       f:00024 d: 297 | OR[297] = A                   
0x00CC (0x000198) 0x1044-       f:00010 d:  68 | A = 68 (0x0044)               
0x00CD (0x00019A) 0x292A-       f:00024 d: 298 | OR[298] = A                   
0x00CE (0x00019C) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00CF (0x00019E) 0x292B-       f:00024 d: 299 | OR[299] = A                   
0x00D0 (0x0001A0) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00D1 (0x0001A2) 0x292C-       f:00024 d: 300 | OR[300] = A                   
0x00D2 (0x0001A4) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00D3 (0x0001A6) 0x292D-       f:00024 d: 301 | OR[301] = A                   
0x00D4 (0x0001A8) 0x1100-       f:00010 d: 256 | A = 256 (0x0100)              
0x00D5 (0x0001AA) 0x292E-       f:00024 d: 302 | OR[302] = A                   
0x00D6 (0x0001AC) 0x1129-       f:00010 d: 297 | A = 297 (0x0129)              
0x00D7 (0x0001AE) 0x5800-       f:00054 d:   0 | B = A                         
0x00D8 (0x0001B0) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00D9 (0x0001B2) 0x7C09-       f:00076 d:   9 | R = OR[9]                     
0x00DA (0x0001B4) 0x86C5-       f:00103 d: 197 | P = P + 197 (0x019F), A # 0  
@ Zero out buffer 
0x00DB (0x0001B6) 0x2100-       f:00020 d: 256 | A = OR[256]                   
0x00DC (0x0001B8) 0x290E-       f:00024 d: 270 | OR[270] = A                   
0x00DD (0x0001BA) 0x1044-       f:00010 d:  68 | A = 68 (0x0044)               
0x00DE (0x0001BC) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x00DF (0x0001BE) 0x210D-       f:00020 d: 269 | A = OR[269]                   
0x00E0 (0x0001C0) 0x8406-       f:00102 d:   6 | P = P + 6 (0x00E6), A = 0     
0x00E1 (0x0001C2) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00E2 (0x0001C4) 0x390E-       f:00034 d: 270 | (OR[270]) = A                 
0x00E3 (0x0001C6) 0x2F0D-       f:00027 d: 269 | OR[269] = OR[269] - 1         
0x00E4 (0x0001C8) 0x2D0E-       f:00026 d: 270 | OR[270] = OR[270] + 1         
0x00E5 (0x0001CA) 0x7206-       f:00071 d:   6 | P = P - 6 (0x00DF)            

0x00E6 (0x0001CC) 0x2101-       f:00020 d: 257 | A = OR[257]                   @ Put buffer address into Concentrator desc. offset 2
0x00E7 (0x0001CE) 0x1402-       f:00012 d:   2 | A = A + 2 (0x0002)            
0x00E8 (0x0001D0) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00E9 (0x0001D2) 0x2100-       f:00020 d: 256 | A = OR[256]                   
0x00EA (0x0001D4) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 

0x00EB (0x0001D6) 0x2100-       f:00020 d: 256 | A = OR[256]                   @ Set lower byte of buffer entry 62 to 1
0x00EC (0x0001D8) 0x143E-       f:00012 d:  62 | A = A + 62 (0x003E)           
0x00ED (0x0001DA) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00EE (0x0001DC) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x00EF (0x0001DE) 0x1A00-0xFF00 f:00015 d:   0 | A = A & 65280 (0xFF00)        
0x00F1 (0x0001E2) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x00F2 (0x0001E4) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x00F3 (0x0001E6) 0x2100-       f:00020 d: 256 | A = OR[256]                   @ OR[287] points to buffer entry 63
0x00F4 (0x0001E8) 0x143F-       f:00012 d:  63 | A = A + 63 (0x003F)           
0x00F5 (0x0001EA) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x00F6 (0x0001EC) 0x2100-       f:00020 d: 256 | A = OR[256]                   @ OR[286] points to buffer entry 0
0x00F7 (0x0001EE) 0x1400-       f:00012 d:   0 | A = A + 0 (0x0000)            
0x00F8 (0x0001F0) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x00F9 (0x0001F2) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x00FA (0x0001F4) 0x2922-       f:00024 d: 290 | OR[290] = A                   
@ We have a loop here seems to be waiting for the 'ready waiting' flag to clear: keep popping messages from the conecntrator until it's empty...
0x00FB (0x0001F6) 0x2122-       f:00020 d: 290 | A = OR[290]                   @ Exit loop if OR[290] == 0, that is ready waiting is cleared
0x00FC (0x0001F8) 0x8426-       f:00102 d:  38 | P = P + 38 (0x0122), A = 0    
0x00FD (0x0001FA) 0x2102-       f:00020 d: 258 | A = OR[258]                   
0x00FE (0x0001FC) 0x5800-       f:00054 d:   0 | B = A                         
0x00FF (0x0001FE) 0xF200-       f:00171 d:   0 | IOB , fn011                   @ Read CI channel status (ready waiting, parity errors)
0x0100 (0x000200) 0x080F-       f:00004 d:  15 | A = A > 15 (0x000F)           @ Get ready-waiting into OR[290]
0x0101 (0x000202) 0x2922-       f:00024 d: 290 | OR[290] = A                   
0x0102 (0x000204) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0104), A # 0     @ If ready-waiting, receive message and continue looping
0x0103 (0x000206) 0x701E-       f:00070 d:  30 | P = P + 30 (0x0121)           
0x0104 (0x000208) 0xE000-       f:00160 d:   0 | IOB , fn000                   @ Clear channel
0x0105 (0x00020A) 0xEE00-       f:00167 d:   0 | IOB , fn007                   @ Enable interrupts
0x0106 (0x00020C) 0x1018-       f:00010 d:  24 | A = 24 (0x0018)               @ Message length: 24 QWORDS, or 0x60 WORDs
0x0107 (0x00020E) 0xE400-       f:00162 d:   0 | IOB , fn002                   
0x0108 (0x000210) 0x211E-       f:00020 d: 286 | A = OR[286]                   
0x0109 (0x000212) 0xE200-       f:00161 d:   0 | IOB , fn001                   @ We receive the message into our buffer, start transfer
0x010A (0x000214) 0x0400-       f:00002 d:   0 | I = 0                         
0x010B (0x000216) 0x0000-       f:00000 d:   0 | PASS                          
0x010C (0x000218) 0x7E03-0x0230 f:00077 d:   3 | R = OR[3]+560 (0x0230)        @ calls offset 0x022A. Wait for interrupt.
0x010E (0x00021C) 0x0600-       f:00003 d:   0 | I = 1                         
0x010F (0x00021E) 0x8606-       f:00103 d:   6 | P = P + 6 (0x0115), A # 0     @ Print error and bail if we timed out
0x0110 (0x000220) 0x2123-       f:00020 d: 291 | A = OR[291]                   
0x0111 (0x000222) 0x1E00-0xFFF8 f:00017 d:   0 | A = A - 65528 (0xFFF8)        
0x0113 (0x000226) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0115), A # 0     @ Print error and bail if we're tired of trying ????
0x0114 (0x000228) 0x700D-       f:00070 d:  13 | P = P + 13 (0x0121)           @ Continue looping
0x0115 (0x00022A) 0x2102-       f:00020 d: 258 | A = OR[258]                   
0x0116 (0x00022C) 0x5800-       f:00054 d:   0 | B = A                         
0x0117 (0x00022E) 0xE800-       f:00164 d:   0 | IOB , fn004                   @ Clear ready-waiting
0x0118 (0x000230) 0xE000-       f:00160 d:   0 | IOB , fn000                   @ Clear channel
0x0119 (0x000232) 0xE600-       f:00163 d:   0 | IOB , fn003                   @ Clear error flags
0x011A (0x000234) 0xEC00-       f:00166 d:   0 | IOB , fn006                   @ Clear interrupt enable flag
0x011B (0x000236) 0x1800-0x02CF f:00014 d:   0 | A = 719 (0x02CF)              @ Print error message: 'Concentrator ordinal' 'channel initialization error. Command aborted.'
0x011D (0x00023A) 0x2919-       f:00024 d: 281 | OR[281] = A                   
0x011E (0x00023C) 0x1017-       f:00010 d:  23 | A = 23 (0x0017)               
0x011F (0x00023E) 0x291A-       f:00024 d: 282 | OR[282] = A                   
0x0120 (0x000240) 0x707D-       f:00070 d: 125 | P = P + 125 (0x019D)          
0x0121 (0x000242) 0x7226-       f:00071 d:  38 | P = P - 38 (0x00FB)           
@ Out of the loop: we have ready-waiting cleared: the concentrator's input queue must be empty
0x0122 (0x000244) 0x2102-       f:00020 d: 258 | A = OR[258]                   
0x0123 (0x000246) 0x5800-       f:00054 d:   0 | B = A                         
0x0124 (0x000248) 0xE000-       f:00160 d:   0 | IOB , fn000                   @ CI Clear channel
0x0125 (0x00024A) 0xE600-       f:00163 d:   0 | IOB , fn003                   @ CI Clear error flags
0x0126 (0x00024C) 0xEC00-       f:00166 d:   0 | IOB , fn006                   @ CI Clear interrupt enable flag
0x0127 (0x00024E) 0x3101-       f:00030 d: 257 | A = (OR[257])                 @ Get desc. entry 0, bit 11 into OR[290]
0x0128 (0x000250) 0x080B-       f:00004 d:  11 | A = A > 11 (0x000B)           
0x0129 (0x000252) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x012A (0x000254) 0x2922-       f:00024 d: 290 | OR[290] = A                   
0x012B (0x000256) 0x3101-       f:00030 d: 257 | A = (OR[257])                 @ Get desc. entry 0, bit 10 into OR[291]
0x012C (0x000258) 0x080A-       f:00004 d:  10 | A = A > 10 (0x000A)           
0x012D (0x00025A) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x012E (0x00025C) 0x2923-       f:00024 d: 291 | OR[291] = A                   
0x012F (0x00025E) 0x2122-       f:00020 d: 290 | A = OR[290]                   
0x0130 (0x000260) 0x2523-       f:00022 d: 291 | A = A + OR[291]               
0x0131 (0x000262) 0x2908-       f:00024 d: 264 | OR[264] = A                   @ The sum of bit 10 and bit 11 into OR[264]
0x0132 (0x000264) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0133 (0x000266) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x0134 (0x000268) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0136), A # 0     @ Jump if either bit 10 or 11 is set
0x0135 (0x00026A) 0x7037-       f:00070 d:  55 | P = P + 55 (0x016C)           @ Neither bit 10 or 11 is set: no need to flush output queue
0x0136 (0x00026C) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0137 (0x00026E) 0x2924-       f:00024 d: 292 | OR[292] = A                   
0x0138 (0x000270) 0x2123-       f:00020 d: 291 | A = OR[291]                   
0x0139 (0x000272) 0x8403-       f:00102 d:   3 | P = P + 3 (0x013C), A = 0     
0x013A (0x000274) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x013B (0x000276) 0x2924-       f:00024 d: 292 | OR[292] = A                   
0x013C (0x000278) 0x211E-       f:00020 d: 286 | A = OR[286]                   
0x013D (0x00027A) 0x1403-       f:00012 d:   3 | A = A + 3 (0x0003)            
0x013E (0x00027C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x013F (0x00027E) 0x2124-       f:00020 d: 292 | A = OR[292]                   
0x0140 (0x000280) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0141 (0x000282) 0x2100-       f:00020 d: 256 | A = OR[256]                   
0x0142 (0x000284) 0x143E-       f:00012 d:  62 | A = A + 62 (0x003E)           
0x0143 (0x000286) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0144 (0x000288) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0145 (0x00028A) 0x0E01-       f:00007 d:   1 | A = A << 1 (0x0001)           
0x0146 (0x00028C) 0x0A01-       f:00005 d:   1 | A = A < 1 (0x0001)            
0x0147 (0x00028E) 0x1400-       f:00012 d:   0 | A = A + 0 (0x0000)            
0x0148 (0x000290) 0x0C02-       f:00006 d:   2 | A = A >> 2 (0x0002)           
0x0149 (0x000292) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x014A (0x000294) 0x2102-       f:00020 d: 258 | A = OR[258]                   
0x014B (0x000296) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x014C (0x000298) 0x5800-       f:00054 d:   0 | B = A                         
0x014D (0x00029A) 0xE000-       f:00160 d:   0 | IOB , fn000                   @ CO Clear channel
0x014E (0x00029C) 0xEE00-       f:00167 d:   0 | IOB , fn007                   @ CO Set interrupt enable
0x014F (0x00029E) 0x1004-       f:00010 d:   4 | A = 4 (0x0004)                
0x0150 (0x0002A0) 0xE400-       f:00162 d:   0 | IOB , fn002                   @ Set parcel count: 4
0x0151 (0x0002A2) 0x211E-       f:00020 d: 286 | A = OR[286]                   @ This is our local buffer
0x0152 (0x0002A4) 0xE200-       f:00161 d:   0 | IOB , fn001                   @ CO Set I/O memory address, start transfer
0x0153 (0x0002A6) 0x0400-       f:00002 d:   0 | I = 0                         
0x0154 (0x0002A8) 0x0000-       f:00000 d:   0 | PASS                          
0x0155 (0x0002AA) 0x7E03-0x0230 f:00077 d:   3 | R = OR[3]+560 (0x0230)        @ Calls offset 0x022A - wait for interrupt
0x0157 (0x0002AE) 0x0600-       f:00003 d:   0 | I = 1                         
0x0158 (0x0002B0) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x0159 (0x0002B2) 0x2102-       f:00020 d: 258 | A = OR[258]                   
0x015A (0x0002B4) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x015B (0x0002B6) 0x5800-       f:00054 d:   0 | B = A                         
0x015C (0x0002B8) 0xE000-       f:00160 d:   0 | IOB , fn000                   @ CO Clear channel
0x015D (0x0002BA) 0xE600-       f:00163 d:   0 | IOB , fn003                   @ CO Clear error flag
0x015E (0x0002BC) 0xEC00-       f:00166 d:   0 | IOB , fn006                   @ CO Clear interrupt enable
0x015F (0x0002BE) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x0160 (0x0002C0) 0x8606-       f:00103 d:   6 | P = P + 6 (0x0166), A # 0     @ If we've timed out, print error message and return
0x0161 (0x0002C2) 0x2123-       f:00020 d: 291 | A = OR[291]                   
0x0162 (0x0002C4) 0x1E00-0xFFF9 f:00017 d:   0 | A = A - 65529 (0xFFF9)        @ Same if we've timed out
0x0164 (0x0002C8) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0166), A # 0     
0x0165 (0x0002CA) 0x7007-       f:00070 d:   7 | P = P + 7 (0x016C)            @ Done flushing the output queue I guess
0x0166 (0x0002CC) 0x1800-0x02E7 f:00014 d:   0 | A = 743 (0x02E7)              
0x0168 (0x0002D0) 0x2919-       f:00024 d: 281 | OR[281] = A                   
0x0169 (0x0002D2) 0x1016-       f:00010 d:  22 | A = 22 (0x0016)               
0x016A (0x0002D4) 0x291A-       f:00024 d: 282 | OR[282] = A                   
0x016B (0x0002D6) 0x7032-       f:00070 d:  50 | P = P + 50 (0x019D)           
@ Done flushing the output queue I guess
0x016C (0x0002D8) 0x3101-       f:00030 d: 257 | A = (OR[257])                 @ Set bit 15 in descriptor offset 0: concentrator active
0x016D (0x0002DA) 0x0A02-       f:00005 d:   2 | A = A < 2 (0x0002)            
0x016E (0x0002DC) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x016F (0x0002DE) 0x0C02-       f:00006 d:   2 | A = A >> 2 (0x0002)           
0x0170 (0x0002E0) 0x3901-       f:00034 d: 257 | (OR[257]) = A                 
0x0171 (0x0002E2) 0x3101-       f:00030 d: 257 | A = (OR[257])                 
0x0172 (0x0002E4) 0x1A00-0xBFFF f:00015 d:   0 | A = A & 49151 (0xBFFF)        @ Clear bit 14 in descriptor offset 0: initialization in progress
0x0174 (0x0002E8) 0x3901-       f:00034 d: 257 | (OR[257]) = A                 
@ Call overlay function 45
0x0175 (0x0002EA) 0x102D-       f:00010 d:  45 | A = 45 (0x002D)               
0x0176 (0x0002EC) 0x2929-       f:00024 d: 297 | OR[297] = A                   
0x0177 (0x0002EE) 0x1800-0x00F9 f:00014 d:   0 | A = 249 (0x00F9)              
0x0179 (0x0002F2) 0x292A-       f:00024 d: 298 | OR[298] = A                   
0x017A (0x0002F4) 0x1008-       f:00010 d:   8 | A = 8 (0x0008)                
0x017B (0x0002F6) 0x292B-       f:00024 d: 299 | OR[299] = A                   
0x017C (0x0002F8) 0x1800-0xFFFF f:00014 d:   0 | A = 65535 (0xFFFF)            
0x017E (0x0002FC) 0x292C-       f:00024 d: 300 | OR[300] = A                   
0x017F (0x0002FE) 0x1129-       f:00010 d: 297 | A = 297 (0x0129)              
0x0180 (0x000300) 0x5800-       f:00054 d:   0 | B = A                         
0x0181 (0x000302) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0182 (0x000304) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

0x0183 (0x000306) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0185), A # 0    
0x0184 (0x000308) 0x700D-       f:00070 d:  13 | P = P + 13 (0x0191)           @ Success from function 45: print message and return (I guess)
0x0185 (0x00030A) 0x3101-       f:00030 d: 257 | A = (OR[257])                 @ Failure from function 45: clear bit 15 in descriptor offset 0: concentrator is not active now
0x0186 (0x00030C) 0x0E01-       f:00007 d:   1 | A = A << 1 (0x0001)           
0x0187 (0x00030E) 0x0A01-       f:00005 d:   1 | A = A < 1 (0x0001)            
0x0188 (0x000310) 0x1400-       f:00012 d:   0 | A = A + 0 (0x0000)            
0x0189 (0x000312) 0x0C02-       f:00006 d:   2 | A = A >> 2 (0x0002)           
0x018A (0x000314) 0x3901-       f:00034 d: 257 | (OR[257]) = A                 
0x018B (0x000316) 0x1800-0x02FE f:00014 d:   0 | A = 766 (0x02FE)              @ Print error message: 'Concentrator ordinal' 'Creation of conc. activity failed. Command aborted.'
0x018D (0x00031A) 0x2919-       f:00024 d: 281 | OR[281] = A                   
0x018E (0x00031C) 0x1019-       f:00010 d:  25 | A = 25 (0x0019)               
0x018F (0x00031E) 0x291A-       f:00024 d: 282 | OR[282] = A                   
0x0190 (0x000320) 0x700D-       f:00070 d:  13 | P = P + 13 (0x019D)           
0x0191 (0x000322) 0x1800-0x0318 f:00014 d:   0 | A = 792 (0x0318)              @ Print message: 'Concentrator ordinal' 'initialized'
0x0193 (0x000326) 0x2919-       f:00024 d: 281 | OR[281] = A                   
0x0194 (0x000328) 0x1006-       f:00010 d:   6 | A = 6 (0x0006)                
0x0195 (0x00032A) 0x291A-       f:00024 d: 282 | OR[282] = A                   
0x0196 (0x00032C) 0x7422-       f:00072 d:  34 | R = P + 34 (0x01B8)           
@ Call overlay function 3
0x0197 (0x00032E) 0x1003-       f:00010 d:   3 | A = 3 (0x0003)                
0x0198 (0x000330) 0x2929-       f:00024 d: 297 | OR[297] = A                   
0x0199 (0x000332) 0x1129-       f:00010 d: 297 | A = 297 (0x0129)              
0x019A (0x000334) 0x5800-       f:00054 d:   0 | B = A                         
0x019B (0x000336) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x019C (0x000338) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Error handler?
@@ Input:
@@ OR[281]: Offset into this overlay (message string??)
@@ OR[282]: Length of the message
@@
@@ Local:
@@ OR[285]: message buffer
0x019D (0x00033A) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x019E (0x00033C) 0x961A-       f:00113 d:  26 | R = P + 26 (0x01B8), A # 0    

0x019F (0x00033E) 0x3101-       f:00030 d: 257 | A = (OR[257])                 
0x01A0 (0x000340) 0x1A00-0xBFFF f:00015 d:   0 | A = A & 49151 (0xBFFF)        
0x01A2 (0x000344) 0x3901-       f:00034 d: 257 | (OR[257]) = A                 
0x01A3 (0x000346) 0x2100-       f:00020 d: 256 | A = OR[256]                   
0x01A4 (0x000348) 0x8602-       f:00103 d:   2 | P = P + 2 (0x01A6), A # 0     
0x01A5 (0x00034A) 0x700E-       f:00070 d:  14 | P = P + 14 (0x01B3)           
@ Call overlay function 25 - release memory in OR[256]
0x01A6 (0x00034C) 0x1019-       f:00010 d:  25 | A = 25 (0x0019)               
0x01A7 (0x00034E) 0x2929-       f:00024 d: 297 | OR[297] = A                   
0x01A8 (0x000350) 0x2100-       f:00020 d: 256 | A = OR[256]                   
0x01A9 (0x000352) 0x292A-       f:00024 d: 298 | OR[298] = A                   
0x01AA (0x000354) 0x1129-       f:00010 d: 297 | A = 297 (0x0129)              
0x01AB (0x000356) 0x5800-       f:00054 d:   0 | B = A                         
0x01AC (0x000358) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x01AD (0x00035A) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

0x01AE (0x00035C) 0x2101-       f:00020 d: 257 | A = OR[257]                   
0x01AF (0x00035E) 0x1402-       f:00012 d:   2 | A = A + 2 (0x0002)            
0x01B0 (0x000360) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x01B1 (0x000362) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x01B2 (0x000364) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x01B3 (0x000366) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x01B4 (0x000368) 0x8E1D-       f:00107 d:  29 | P = P - 29 (0x0197), A # 0    
0x01B5 (0x00036A) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x01B6 (0x00036C) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x01B7 (0x00036E) 0x73A8-       f:00071 d: 424 | P = P - 424 (0x000F)          
@ Allocate 42 words of memory, store it in OR[283]
0x01B8 (0x000370) 0x1018-       f:00010 d:  24 | A = 24 (0x0018)               
0x01B9 (0x000372) 0x2929-       f:00024 d: 297 | OR[297] = A                   
0x01BA (0x000374) 0x1028-       f:00010 d:  40 | A = 40 (0x0028)               
0x01BB (0x000376) 0x1402-       f:00012 d:   2 | A = A + 2 (0x0002)            
0x01BC (0x000378) 0x292A-       f:00024 d: 298 | OR[298] = A                   
0x01BD (0x00037A) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x01BE (0x00037C) 0x292B-       f:00024 d: 299 | OR[299] = A                   
0x01BF (0x00037E) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x01C0 (0x000380) 0x292C-       f:00024 d: 300 | OR[300] = A                   
0x01C1 (0x000382) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x01C2 (0x000384) 0x292D-       f:00024 d: 301 | OR[301] = A                   
0x01C3 (0x000386) 0x111B-       f:00010 d: 283 | A = 283 (0x011B)              
0x01C4 (0x000388) 0x292E-       f:00024 d: 302 | OR[302] = A                   
0x01C5 (0x00038A) 0x1129-       f:00010 d: 297 | A = 297 (0x0129)              
0x01C6 (0x00038C) 0x5800-       f:00054 d:   0 | B = A                         
0x01C7 (0x00038E) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x01C8 (0x000390) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

0x01C9 (0x000392) 0x8E2A-       f:00107 d:  42 | P = P - 42 (0x019F), A # 0    
0x01CA (0x000394) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x01CB (0x000396) 0x1428-       f:00012 d:  40 | A = A + 40 (0x0028)           
0x01CC (0x000398) 0x2920-       f:00024 d: 288 | OR[288] = A                   
0x01CD (0x00039A) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x01CE (0x00039C) 0x290E-       f:00024 d: 270 | OR[270] = A                   
0x01CF (0x00039E) 0x1028-       f:00010 d:  40 | A = 40 (0x0028)               
0x01D0 (0x0003A0) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x01D1 (0x0003A2) 0x210D-       f:00020 d: 269 | A = OR[269]                   
0x01D2 (0x0003A4) 0x8407-       f:00102 d:   7 | P = P + 7 (0x01D9), A = 0     
0x01D3 (0x0003A6) 0x1800-0x2020 f:00014 d:   0 | A = 8224 (0x2020)             
0x01D5 (0x0003AA) 0x390E-       f:00034 d: 270 | (OR[270]) = A                 
0x01D6 (0x0003AC) 0x2F0D-       f:00027 d: 269 | OR[269] = OR[269] - 1         
0x01D7 (0x0003AE) 0x2D0E-       f:00026 d: 270 | OR[270] = OR[270] + 1         
0x01D8 (0x0003B0) 0x7207-       f:00071 d:   7 | P = P - 7 (0x01D1)            
0x01D9 (0x0003B2) 0x1800-0x024E f:00014 d:   0 | A = 590 (0x024E)              
0x01DB (0x0003B6) 0x2403-       f:00022 d:   3 | A = A + OR[3]                 
0x01DC (0x0003B8) 0x291C-       f:00024 d: 284 | OR[284] = A                   
0x01DD (0x0003BA) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x01DE (0x0003BC) 0x140A-       f:00012 d:  10 | A = A + 10 (0x000A)           
0x01DF (0x0003BE) 0x1402-       f:00012 d:   2 | A = A + 2 (0x0002)            
0x01E0 (0x0003C0) 0x291D-       f:00024 d: 285 | OR[285] = A                   
0x01E1 (0x0003C2) 0x211C-       f:00020 d: 284 | A = OR[284]                   
0x01E2 (0x0003C4) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x01E3 (0x0003C6) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x01E4 (0x0003C8) 0x290E-       f:00024 d: 270 | OR[270] = A                   
0x01E5 (0x0003CA) 0x100A-       f:00010 d:  10 | A = 10 (0x000A)               
0x01E6 (0x0003CC) 0x290F-       f:00024 d: 271 | OR[271] = A                   
0x01E7 (0x0003CE) 0x7006-       f:00070 d:   6 | P = P + 6 (0x01ED)            
0x01E8 (0x0003D0) 0x310D-       f:00030 d: 269 | A = (OR[269])                 
0x01E9 (0x0003D2) 0x390E-       f:00034 d: 270 | (OR[270]) = A                 
0x01EA (0x0003D4) 0x2D0D-       f:00026 d: 269 | OR[269] = OR[269] + 1         
0x01EB (0x0003D6) 0x2D0E-       f:00026 d: 270 | OR[270] = OR[270] + 1         
0x01EC (0x0003D8) 0x2F0F-       f:00027 d: 271 | OR[271] = OR[271] - 1         
0x01ED (0x0003DA) 0x210F-       f:00020 d: 271 | A = OR[271]                   
0x01EE (0x0003DC) 0x8E06-       f:00107 d:   6 | P = P - 6 (0x01E8), C = 0     
0x01EF (0x0003DE) 0x2103-       f:00020 d: 259 | A = OR[259]                   
0x01F0 (0x0003E0) 0x3920-       f:00034 d: 288 | (OR[288]) = A                 
@ Call overlay BTD - Binary to decimal ASCII conversion
0x01F1 (0x0003E2) 0x1028-       f:00010 d:  40 | A = 40 (0x0028)               
0x01F2 (0x0003E4) 0x2929-       f:00024 d: 297 | OR[297] = A                   
0x01F3 (0x0003E6) 0x1800-0x000F f:00014 d:   0 | A = 15 (0x000F)               
0x01F5 (0x0003EA) 0x292A-       f:00024 d: 298 | OR[298] = A                   
0x01F6 (0x0003EC) 0x2120-       f:00020 d: 288 | A = OR[288]                   
0x01F7 (0x0003EE) 0x292B-       f:00024 d: 299 | OR[299] = A                   
0x01F8 (0x0003F0) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x01F9 (0x0003F2) 0x292C-       f:00024 d: 300 | OR[300] = A                   
0x01FA (0x0003F4) 0x1010-       f:00010 d:  16 | A = 16 (0x0010)               
0x01FB (0x0003F6) 0x292D-       f:00024 d: 301 | OR[301] = A                   
0x01FC (0x0003F8) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x01FD (0x0003FA) 0x292E-       f:00024 d: 302 | OR[302] = A                   
0x01FE (0x0003FC) 0x1015-       f:00010 d:  21 | A = 21 (0x0015)               
0x01FF (0x0003FE) 0x292F-       f:00024 d: 303 | OR[303] = A                   
0x0200 (0x000400) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0201 (0x000402) 0x2930-       f:00024 d: 304 | OR[304] = A                   
0x0202 (0x000404) 0x1129-       f:00010 d: 297 | A = 297 (0x0129)              
0x0203 (0x000406) 0x5800-       f:00054 d:   0 | B = A                         
0x0204 (0x000408) 0x1800-0x2318 f:00014 d:   0 | A = 8984 (0x2318)             
0x0206 (0x00040C) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

0x0207 (0x00040E) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x0208 (0x000410) 0x2403-       f:00022 d:   3 | A = A + OR[3]                 
0x0209 (0x000412) 0x2919-       f:00024 d: 281 | OR[281] = A                   
0x020A (0x000414) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x020B (0x000416) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x020C (0x000418) 0x211D-       f:00020 d: 285 | A = OR[285]                   
0x020D (0x00041A) 0x290E-       f:00024 d: 270 | OR[270] = A                   
0x020E (0x00041C) 0x211A-       f:00020 d: 282 | A = OR[282]                   
0x020F (0x00041E) 0x290F-       f:00024 d: 271 | OR[271] = A                   
0x0210 (0x000420) 0x7006-       f:00070 d:   6 | P = P + 6 (0x0216)            
0x0211 (0x000422) 0x310D-       f:00030 d: 269 | A = (OR[269])                 
0x0212 (0x000424) 0x390E-       f:00034 d: 270 | (OR[270]) = A                 
0x0213 (0x000426) 0x2D0D-       f:00026 d: 269 | OR[269] = OR[269] + 1         
0x0214 (0x000428) 0x2D0E-       f:00026 d: 270 | OR[270] = OR[270] + 1         
0x0215 (0x00042A) 0x2F0F-       f:00027 d: 271 | OR[271] = OR[271] - 1         
0x0216 (0x00042C) 0x210F-       f:00020 d: 271 | A = OR[271]                   
0x0217 (0x00042E) 0x8E06-       f:00107 d:   6 | P = P - 6 (0x0211), A # 0     
@ Call overlay function 16
0x0218 (0x000430) 0x1010-       f:00010 d:  16 | A = 16 (0x0010)               
0x0219 (0x000432) 0x2929-       f:00024 d: 297 | OR[297] = A                   
0x021A (0x000434) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x021B (0x000436) 0x292A-       f:00024 d: 298 | OR[298] = A                   
0x021C (0x000438) 0x1129-       f:00010 d: 297 | A = 297 (0x0129)              
0x021D (0x00043A) 0x5800-       f:00054 d:   0 | B = A                         
0x021E (0x00043C) 0x1800-0x2318 f:00014 d:   0 | A = 8984 (0x2318)             
0x0220 (0x000440) 0x7C09-       f:00076 d:   9 | R = OR[9]                     
@ Call overlay function 25: release memory from OR[283]
0x0221 (0x000442) 0x1019-       f:00010 d:  25 | A = 25 (0x0019)               
0x0222 (0x000444) 0x2929-       f:00024 d: 297 | OR[297] = A                   
0x0223 (0x000446) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x0224 (0x000448) 0x292A-       f:00024 d: 298 | OR[298] = A                   
0x0225 (0x00044A) 0x1129-       f:00010 d: 297 | A = 297 (0x0129)              
0x0226 (0x00044C) 0x5800-       f:00054 d:   0 | B = A                         
0x0227 (0x00044E) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0228 (0x000450) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

0x0229 (0x000452) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Wait for interrupt
@@ Input:
@@   OR[287]: pointer to some flag. If flag is non-0, zero it out and return immediately
@@ Output:
@@   A : 0 if timeout, non-0 if interrupt received
@@   OR[291]: old value from (OR[287])
@@   (OR[287]): set to 0
0x022A (0x000454) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x022B (0x000456) 0x2928-       f:00024 d: 296 | OR[296] = A                   
0x022C (0x000458) 0x311F-       f:00030 d: 287 | A = (OR[287])                 
0x022D (0x00045A) 0x8402-       f:00102 d:   2 | P = P + 2 (0x022F), A = 0     
0x022E (0x00045C) 0x7014-       f:00070 d:  20 | P = P + 20 (0x0242)           
@ Call overlay function 9: wait for interrupt
0x022F (0x00045E) 0x1009-       f:00010 d:   9 | A = 9 (0x0009)                
0x0230 (0x000460) 0x2929-       f:00024 d: 297 | OR[297] = A                   
0x0231 (0x000462) 0x211F-       f:00020 d: 287 | A = OR[287]                   
0x0232 (0x000464) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x0233 (0x000466) 0x292A-       f:00024 d: 298 | OR[298] = A                   
0x0234 (0x000468) 0x1064-       f:00010 d: 100 | A = 100 (0x0064)              
0x0235 (0x00046A) 0x292B-       f:00024 d: 299 | OR[299] = A                   
0x0236 (0x00046C) 0x1129-       f:00010 d: 297 | A = 297 (0x0129)              
0x0237 (0x00046E) 0x5800-       f:00054 d:   0 | B = A                         
0x0238 (0x000470) 0x1800-0x2318 f:00014 d:   0 | A = 8984 (0x2318)             
0x023A (0x000474) 0x7C09-       f:00076 d:   9 | R = OR[9]                     
@ Read timeout value: did we time out or got an interrupt?
0x023B (0x000476) 0x2006-       f:00020 d:   6 | A = OR[6]                     
0x023C (0x000478) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x023D (0x00047A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x023E (0x00047C) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x023F (0x00047E) 0x0400-       f:00002 d:   0 | I = 0                         
0x0240 (0x000480) 0x0000-       f:00000 d:   0 | PASS                          
0x0241 (0x000482) 0x2928-       f:00024 d: 296 | OR[296] = A                   
0x0242 (0x000484) 0x311F-       f:00030 d: 287 | A = (OR[287])                 
0x0243 (0x000486) 0x2923-       f:00024 d: 291 | OR[291] = A                   
0x0244 (0x000488) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0245 (0x00048A) 0x391F-       f:00034 d: 287 | (OR[287]) = A                 
0x0246 (0x00048C) 0x2128-       f:00020 d: 296 | A = OR[296]                   
0x0247 (0x00048E) 0x0200-       f:00001 d:   0 | EXIT                          

@ Various strings here:
@   Concentrator ordinal  not configured. Command aborted.  termination in progress. Command aborted. initialization in progress. Command aborted.already active.Command aborted.  not configured correctly. Command aborted.  
@   channel disabled.Command aborted.  channel initialization error.Command aborted.  VAX interfaceselect error. Command aborted.Creation of conc. activity failed. Command aborted. initialized
0x0248 (0x000490) 0x436F-       f:00041 d: 367 | C = 1, io 0557 = BZ            
0x0249 (0x000492) 0x6E63-       f:00067 d:  99 | OR[B] = OR[B] - 1              | **** non-standard encoding with D:0x0063 ****
0x024A (0x000494) 0x656E-       f:00062 d: 366 | A = A + OR[B]                  | **** non-standard encoding with D:0x016E ****
0x024B (0x000496) 0x7472-       f:00072 d: 114 | R = P + 114 (0x02BD)          
0x024C (0x000498) 0x6174-       f:00060 d: 372 | A = OR[B]                      | **** non-standard encoding with D:0x0174 ****
0x024D (0x00049A) 0x6F72-       f:00067 d: 370 | OR[B] = OR[B] - 1              | **** non-standard encoding with D:0x0172 ****
0x024E (0x00049C) 0x206F-       f:00020 d: 111 | A = OR[111]                   
0x024F (0x00049E) 0x7264-       f:00071 d: 100 | P = P - 100 (0x01EB)          
0x0250 (0x0004A0) 0x696E-       f:00064 d: 366 | OR[B] = A                      | **** non-standard encoding with D:0x016E ****
0x0251 (0x0004A2) 0x616C-       f:00060 d: 364 | A = OR[B]                      | **** non-standard encoding with D:0x016C ****
0x0252 (0x0004A4) 0x0000-       f:00000 d:   0 | PASS                          
0x0253 (0x0004A6) 0x6E6F-       f:00067 d: 111 | OR[B] = OR[B] - 1              | **** non-standard encoding with D:0x006F ****
0x0254 (0x0004A8) 0x7420-       f:00072 d:  32 | R = P + 32 (0x0274)           
0x0255 (0x0004AA) 0x636F-       f:00061 d: 367 | A = A & OR[B]                  | **** non-standard encoding with D:0x016F ****
0x0256 (0x0004AC) 0x6E66-       f:00067 d: 102 | OR[B] = OR[B] - 1              | **** non-standard encoding with D:0x0066 ****
0x0257 (0x0004AE) 0x6967-       f:00064 d: 359 | OR[B] = A                      | **** non-standard encoding with D:0x0167 ****
0x0258 (0x0004B0) 0x7572-       f:00072 d: 370 | R = P + 370 (0x03CA)          
0x0259 (0x0004B2) 0x6564-       f:00062 d: 356 | A = A + OR[B]                  | **** non-standard encoding with D:0x0164 ****
0x025A (0x0004B4) 0x2E20-       f:00027 d:  32 | OR[32] = OR[32] - 1           
0x025B (0x0004B6) 0x436F-       f:00041 d: 367 | C = 1, io 0557 = BZ           
0x025C (0x0004B8) 0x6D6D-       f:00066 d: 365 | OR[B] = OR[B] + 1              | **** non-standard encoding with D:0x016D ****
0x025D (0x0004BA) 0x616E-       f:00060 d: 366 | A = OR[B]                      | **** non-standard encoding with D:0x016E ****
0x025E (0x0004BC) 0x6420-       f:00062 d:  32 | A = A + OR[B]                  | **** non-standard encoding with D:0x0020 ****
0x025F (0x0004BE) 0x6162-       f:00060 d: 354 | A = OR[B]                      | **** non-standard encoding with D:0x0162 ****
0x0260 (0x0004C0) 0x6F72-       f:00067 d: 370 | OR[B] = OR[B] - 1              | **** non-standard encoding with D:0x0172 ****
0x0261 (0x0004C2) 0x7465-       f:00072 d: 101 | R = P + 101 (0x02C6)          
0x0262 (0x0004C4) 0x642E-       f:00062 d:  46 | A = A + OR[B]                  | **** non-standard encoding with D:0x002E ****
0x0263 (0x0004C6) 0x0000-       f:00000 d:   0 | PASS                          
0x0264 (0x0004C8) 0x7465-       f:00072 d: 101 | R = P + 101 (0x02C9)          
0x0265 (0x0004CA) 0x726D-       f:00071 d: 109 | P = P - 109 (0x01F8)          
0x0266 (0x0004CC) 0x696E-       f:00064 d: 366 | OR[B] = A                      | **** non-standard encoding with D:0x016E ****
0x0267 (0x0004CE) 0x6174-       f:00060 d: 372 | A = OR[B]                      | **** non-standard encoding with D:0x0174 ****
0x0268 (0x0004D0) 0x696F-       f:00064 d: 367 | OR[B] = A                      | **** non-standard encoding with D:0x016F ****
0x0269 (0x0004D2) 0x6E20-       f:00067 d:  32 | OR[B] = OR[B] - 1              | **** non-standard encoding with D:0x0020 ****
0x026A (0x0004D4) 0x696E-       f:00064 d: 366 | OR[B] = A                      | **** non-standard encoding with D:0x016E ****
0x026B (0x0004D6) 0x2070-       f:00020 d: 112 | A = OR[112]                   
0x026C (0x0004D8) 0x726F-       f:00071 d: 111 | P = P - 111 (0x01FD)          
0x026D (0x0004DA) 0x6772-       f:00063 d: 370 | A = A - OR[B]                  | **** non-standard encoding with D:0x0172 ****
0x026E (0x0004DC) 0x6573-       f:00062 d: 371 | A = A + OR[B]                  | **** non-standard encoding with D:0x0173 ****
0x026F (0x0004DE) 0x732E-       f:00071 d: 302 | P = P - 302 (0x0141)          
0x0270 (0x0004E0) 0x2043-       f:00020 d:  67 | A = OR[67]                    
0x0271 (0x0004E2) 0x6F6D-       f:00067 d: 365 | OR[B] = OR[B] - 1              | **** non-standard encoding with D:0x016D ****
0x0272 (0x0004E4) 0x6D61-       f:00066 d: 353 | OR[B] = OR[B] + 1              | **** non-standard encoding with D:0x0161 ****
0x0273 (0x0004E6) 0x6E64-       f:00067 d: 100 | OR[B] = OR[B] - 1              | **** non-standard encoding with D:0x0064 ****
0x0274 (0x0004E8) 0x2061-       f:00020 d:  97 | A = OR[97]                    
0x0275 (0x0004EA) 0x626F-       f:00061 d: 111 | A = A & OR[B]                  | **** non-standard encoding with D:0x006F ****
0x0276 (0x0004EC) 0x7274-       f:00071 d: 116 | P = P - 116 (0x0202)          
0x0277 (0x0004EE) 0x6564-       f:00062 d: 356 | A = A + OR[B]                  | **** non-standard encoding with D:0x0164 ****
0x0278 (0x0004F0) 0x2E00-       f:00027 d:   0 | OR[0] = OR[0] - 1             
0x0279 (0x0004F2) 0x696E-       f:00064 d: 366 | OR[B] = A                      | **** non-standard encoding with D:0x016E ****
0x027A (0x0004F4) 0x6974-       f:00064 d: 372 | OR[B] = A                      | **** non-standard encoding with D:0x0174 ****
0x027B (0x0004F6) 0x6961-       f:00064 d: 353 | OR[B] = A                      | **** non-standard encoding with D:0x0161 ****
0x027C (0x0004F8) 0x6C69-       f:00066 d: 105 | OR[B] = OR[B] + 1              | **** non-standard encoding with D:0x0069 ****
0x027D (0x0004FA) 0x7A61-0x7469 f:00075 d:  97 | P = OR[97]+29801 (0x7469)     
0x027F (0x0004FE) 0x6F6E-       f:00067 d: 366 | OR[B] = OR[B] - 1              | **** non-standard encoding with D:0x016E ****
0x0280 (0x000500) 0x2069-       f:00020 d: 105 | A = OR[105]                   
0x0281 (0x000502) 0x6E20-       f:00067 d:  32 | OR[B] = OR[B] - 1              | **** non-standard encoding with D:0x0020 ****
0x0282 (0x000504) 0x7072-       f:00070 d: 114 | P = P + 114 (0x02F4)          
0x0283 (0x000506) 0x6F67-       f:00067 d: 359 | OR[B] = OR[B] - 1              | **** non-standard encoding with D:0x0167 ****
0x0284 (0x000508) 0x7265-       f:00071 d: 101 | P = P - 101 (0x021F)          
0x0285 (0x00050A) 0x7373-       f:00071 d: 371 | P = P - 371 (0x0112)          
0x0286 (0x00050C) 0x2E20-       f:00027 d:  32 | OR[32] = OR[32] - 1           
0x0287 (0x00050E) 0x436F-       f:00041 d: 367 | C = 1, io 0557 = BZ           
0x0288 (0x000510) 0x6D6D-       f:00066 d: 365 | OR[B] = OR[B] + 1              | **** non-standard encoding with D:0x016D ****
0x0289 (0x000512) 0x616E-       f:00060 d: 366 | A = OR[B]                      | **** non-standard encoding with D:0x016E ****
0x028A (0x000514) 0x6420-       f:00062 d:  32 | A = A + OR[B]                  | **** non-standard encoding with D:0x0020 ****
0x028B (0x000516) 0x6162-       f:00060 d: 354 | A = OR[B]                      | **** non-standard encoding with D:0x0162 ****
0x028C (0x000518) 0x6F72-       f:00067 d: 370 | OR[B] = OR[B] - 1              | **** non-standard encoding with D:0x0172 ****
0x028D (0x00051A) 0x7465-       f:00072 d: 101 | R = P + 101 (0x02F2)          
0x028E (0x00051C) 0x642E-       f:00062 d:  46 | A = A + OR[B]                  | **** non-standard encoding with D:0x002E ****
0x028F (0x00051E) 0x0000-       f:00000 d:   0 | PASS                          
0x0290 (0x000520) 0x616C-       f:00060 d: 364 | A = OR[B]                      | **** non-standard encoding with D:0x016C ****
0x0291 (0x000522) 0x7265-       f:00071 d: 101 | P = P - 101 (0x022C)          
0x0292 (0x000524) 0x6164-       f:00060 d: 356 | A = OR[B]                      | **** non-standard encoding with D:0x0164 ****
0x0293 (0x000526) 0x7920-       f:00074 d: 288 | P = OR[288]                   
0x0294 (0x000528) 0x6163-       f:00060 d: 355 | A = OR[B]                      | **** non-standard encoding with D:0x0163 ****
0x0295 (0x00052A) 0x7469-       f:00072 d: 105 | R = P + 105 (0x02FE)          
0x0296 (0x00052C) 0x7665-       f:00073 d: 101 | R = P - 101 (0x0231)          
0x0297 (0x00052E) 0x2E20-       f:00027 d:  32 | OR[32] = OR[32] - 1           
0x0298 (0x000530) 0x436F-       f:00041 d: 367 | C = 1, io 0557 = BZ           
0x0299 (0x000532) 0x6D6D-       f:00066 d: 365 | OR[B] = OR[B] + 1              | **** non-standard encoding with D:0x016D ****
0x029A (0x000534) 0x616E-       f:00060 d: 366 | A = OR[B]                      | **** non-standard encoding with D:0x016E ****
0x029B (0x000536) 0x6420-       f:00062 d:  32 | A = A + OR[B]                  | **** non-standard encoding with D:0x0020 ****
0x029C (0x000538) 0x6162-       f:00060 d: 354 | A = OR[B]                      | **** non-standard encoding with D:0x0162 ****
0x029D (0x00053A) 0x6F72-       f:00067 d: 370 | OR[B] = OR[B] - 1              | **** non-standard encoding with D:0x0172 ****
0x029E (0x00053C) 0x7465-       f:00072 d: 101 | R = P + 101 (0x0303)          
0x029F (0x00053E) 0x642E-       f:00062 d:  46 | A = A + OR[B]                  | **** non-standard encoding with D:0x002E ****
0x02A0 (0x000540) 0x0000-       f:00000 d:   0 | PASS                          
0x02A1 (0x000542) 0x6E6F-       f:00067 d: 111 | OR[B] = OR[B] - 1              | **** non-standard encoding with D:0x006F ****
0x02A2 (0x000544) 0x7420-       f:00072 d:  32 | R = P + 32 (0x02C2)           
0x02A3 (0x000546) 0x636F-       f:00061 d: 367 | A = A & OR[B]                  | **** non-standard encoding with D:0x016F ****
0x02A4 (0x000548) 0x6E66-       f:00067 d: 102 | OR[B] = OR[B] - 1              | **** non-standard encoding with D:0x0066 ****
0x02A5 (0x00054A) 0x6967-       f:00064 d: 359 | OR[B] = A                      | **** non-standard encoding with D:0x0167 ****
0x02A6 (0x00054C) 0x7572-       f:00072 d: 370 | R = P + 370 (0x0418)          
0x02A7 (0x00054E) 0x6564-       f:00062 d: 356 | A = A + OR[B]                  | **** non-standard encoding with D:0x0164 ****
0x02A8 (0x000550) 0x2063-       f:00020 d:  99 | A = OR[99]                    
0x02A9 (0x000552) 0x6F72-       f:00067 d: 370 | OR[B] = OR[B] - 1              | **** non-standard encoding with D:0x0172 ****
0x02AA (0x000554) 0x7265-       f:00071 d: 101 | P = P - 101 (0x0245)          
0x02AB (0x000556) 0x6374-       f:00061 d: 372 | A = A & OR[B]                  | **** non-standard encoding with D:0x0174 ****
0x02AC (0x000558) 0x6C79-       f:00066 d: 121 | OR[B] = OR[B] + 1              | **** non-standard encoding with D:0x0079 ****
0x02AD (0x00055A) 0x2E20-       f:00027 d:  32 | OR[32] = OR[32] - 1           
0x02AE (0x00055C) 0x436F-       f:00041 d: 367 | C = 1, io 0557 = BZ           
0x02AF (0x00055E) 0x6D6D-       f:00066 d: 365 | OR[B] = OR[B] + 1              | **** non-standard encoding with D:0x016D ****
0x02B0 (0x000560) 0x616E-       f:00060 d: 366 | A = OR[B]                      | **** non-standard encoding with D:0x016E ****
0x02B1 (0x000562) 0x6420-       f:00062 d:  32 | A = A + OR[B]                  | **** non-standard encoding with D:0x0020 ****
0x02B2 (0x000564) 0x6162-       f:00060 d: 354 | A = OR[B]                      | **** non-standard encoding with D:0x0162 ****
0x02B3 (0x000566) 0x6F72-       f:00067 d: 370 | OR[B] = OR[B] - 1              | **** non-standard encoding with D:0x0172 ****
0x02B4 (0x000568) 0x7465-       f:00072 d: 101 | R = P + 101 (0x0319)          
0x02B5 (0x00056A) 0x642E-       f:00062 d:  46 | A = A + OR[B]                  | **** non-standard encoding with D:0x002E ****
0x02B6 (0x00056C) 0x0000-       f:00000 d:   0 | PASS                          
0x02B7 (0x00056E) 0x6368-       f:00061 d: 360 | A = A & OR[B]                  | **** non-standard encoding with D:0x0168 ****
0x02B8 (0x000570) 0x616E-       f:00060 d: 366 | A = OR[B]                      | **** non-standard encoding with D:0x016E ****
0x02B9 (0x000572) 0x6E65-       f:00067 d: 101 | OR[B] = OR[B] - 1              | **** non-standard encoding with D:0x0065 ****
0x02BA (0x000574) 0x6C20-       f:00066 d:  32 | OR[B] = OR[B] + 1              | **** non-standard encoding with D:0x0020 ****
0x02BB (0x000576) 0x6469-       f:00062 d: 105 | A = A + OR[B]                  | **** non-standard encoding with D:0x0069 ****
0x02BC (0x000578) 0x7361-       f:00071 d: 353 | P = P - 353 (0x015B)          
0x02BD (0x00057A) 0x626C-       f:00061 d: 108 | A = A & OR[B]                  | **** non-standard encoding with D:0x006C ****
0x02BE (0x00057C) 0x6564-       f:00062 d: 356 | A = A + OR[B]                  | **** non-standard encoding with D:0x0164 ****
0x02BF (0x00057E) 0x2E20-       f:00027 d:  32 | OR[32] = OR[32] - 1           
0x02C0 (0x000580) 0x436F-       f:00041 d: 367 | C = 1, io 0557 = BZ           
0x02C1 (0x000582) 0x6D6D-       f:00066 d: 365 | OR[B] = OR[B] + 1              | **** non-standard encoding with D:0x016D ****
0x02C2 (0x000584) 0x616E-       f:00060 d: 366 | A = OR[B]                      | **** non-standard encoding with D:0x016E ****
0x02C3 (0x000586) 0x6420-       f:00062 d:  32 | A = A + OR[B]                  | **** non-standard encoding with D:0x0020 ****
0x02C4 (0x000588) 0x6162-       f:00060 d: 354 | A = OR[B]                      | **** non-standard encoding with D:0x0162 ****
0x02C5 (0x00058A) 0x6F72-       f:00067 d: 370 | OR[B] = OR[B] - 1              | **** non-standard encoding with D:0x0172 ****
0x02C6 (0x00058C) 0x7465-       f:00072 d: 101 | R = P + 101 (0x032B)          
0x02C7 (0x00058E) 0x642E-       f:00062 d:  46 | A = A + OR[B]                  | **** non-standard encoding with D:0x002E ****
0x02C8 (0x000590) 0x0000-       f:00000 d:   0 | PASS                          
0x02C9 (0x000592) 0x6368-       f:00061 d: 360 | A = A & OR[B]                  | **** non-standard encoding with D:0x0168 ****
0x02CA (0x000594) 0x616E-       f:00060 d: 366 | A = OR[B]                      | **** non-standard encoding with D:0x016E ****
0x02CB (0x000596) 0x6E65-       f:00067 d: 101 | OR[B] = OR[B] - 1              | **** non-standard encoding with D:0x0065 ****
0x02CC (0x000598) 0x6C20-       f:00066 d:  32 | OR[B] = OR[B] + 1              | **** non-standard encoding with D:0x0020 ****
0x02CD (0x00059A) 0x696E-       f:00064 d: 366 | OR[B] = A                      | **** non-standard encoding with D:0x016E ****
0x02CE (0x00059C) 0x6974-       f:00064 d: 372 | OR[B] = A                      | **** non-standard encoding with D:0x0174 ****
0x02CF (0x00059E) 0x6961-       f:00064 d: 353 | OR[B] = A                      | **** non-standard encoding with D:0x0161 ****
0x02D0 (0x0005A0) 0x6C69-       f:00066 d: 105 | OR[B] = OR[B] + 1              | **** non-standard encoding with D:0x0069 ****
0x02D1 (0x0005A2) 0x7A61-0x7469 f:00075 d:  97 | P = OR[97]+29801 (0x7469)     
0x02D3 (0x0005A6) 0x6F6E-       f:00067 d: 366 | OR[B] = OR[B] - 1              | **** non-standard encoding with D:0x016E ****
0x02D4 (0x0005A8) 0x2065-       f:00020 d: 101 | A = OR[101]                   
0x02D5 (0x0005AA) 0x7272-       f:00071 d: 114 | P = P - 114 (0x0263)          
0x02D6 (0x0005AC) 0x6F72-       f:00067 d: 370 | OR[B] = OR[B] - 1              | **** non-standard encoding with D:0x0172 ****
0x02D7 (0x0005AE) 0x2E20-       f:00027 d:  32 | OR[32] = OR[32] - 1           
0x02D8 (0x0005B0) 0x436F-       f:00041 d: 367 | C = 1, io 0557 = BZ           
0x02D9 (0x0005B2) 0x6D6D-       f:00066 d: 365 | OR[B] = OR[B] + 1              | **** non-standard encoding with D:0x016D ****
0x02DA (0x0005B4) 0x616E-       f:00060 d: 366 | A = OR[B]                      | **** non-standard encoding with D:0x016E ****
0x02DB (0x0005B6) 0x6420-       f:00062 d:  32 | A = A + OR[B]                  | **** non-standard encoding with D:0x0020 ****
0x02DC (0x0005B8) 0x6162-       f:00060 d: 354 | A = OR[B]                      | **** non-standard encoding with D:0x0162 ****
0x02DD (0x0005BA) 0x6F72-       f:00067 d: 370 | OR[B] = OR[B] - 1              | **** non-standard encoding with D:0x0172 ****
0x02DE (0x0005BC) 0x7465-       f:00072 d: 101 | R = P + 101 (0x0343)          
0x02DF (0x0005BE) 0x642E-       f:00062 d:  46 | A = A + OR[B]                  | **** non-standard encoding with D:0x002E ****
0x02E0 (0x0005C0) 0x0000-       f:00000 d:   0 | PASS                          
0x02E1 (0x0005C2) 0x5641-       f:00053 d:  65 | A = A - B                      | **** non-standard encoding with D:0x0041 ****
0x02E2 (0x0005C4) 0x5820-       f:00054 d:  32 | B = A                          | **** non-standard encoding with D:0x0020 ****
0x02E3 (0x0005C6) 0x696E-       f:00064 d: 366 | OR[B] = A                      | **** non-standard encoding with D:0x016E ****
0x02E4 (0x0005C8) 0x7465-       f:00072 d: 101 | R = P + 101 (0x0349)          
0x02E5 (0x0005CA) 0x7266-       f:00071 d: 102 | P = P - 102 (0x027F)          
0x02E6 (0x0005CC) 0x6163-       f:00060 d: 355 | A = OR[B]                      | **** non-standard encoding with D:0x0163 ****
0x02E7 (0x0005CE) 0x6520-       f:00062 d: 288 | A = A + OR[B]                  | **** non-standard encoding with D:0x0120 ****
0x02E8 (0x0005D0) 0x7365-       f:00071 d: 357 | P = P - 357 (0x0183)          
0x02E9 (0x0005D2) 0x6C65-       f:00066 d: 101 | OR[B] = OR[B] + 1              | **** non-standard encoding with D:0x0065 ****
0x02EA (0x0005D4) 0x6374-       f:00061 d: 372 | A = A & OR[B]                  | **** non-standard encoding with D:0x0174 ****
0x02EB (0x0005D6) 0x2065-       f:00020 d: 101 | A = OR[101]                   
0x02EC (0x0005D8) 0x7272-       f:00071 d: 114 | P = P - 114 (0x027A)          
0x02ED (0x0005DA) 0x6F72-       f:00067 d: 370 | OR[B] = OR[B] - 1              | **** non-standard encoding with D:0x0172 ****
0x02EE (0x0005DC) 0x2E20-       f:00027 d:  32 | OR[32] = OR[32] - 1           
0x02EF (0x0005DE) 0x436F-       f:00041 d: 367 | C = 1, io 0557 = BZ           
0x02F0 (0x0005E0) 0x6D6D-       f:00066 d: 365 | OR[B] = OR[B] + 1              | **** non-standard encoding with D:0x016D ****
0x02F1 (0x0005E2) 0x616E-       f:00060 d: 366 | A = OR[B]                      | **** non-standard encoding with D:0x016E ****
0x02F2 (0x0005E4) 0x6420-       f:00062 d:  32 | A = A + OR[B]                  | **** non-standard encoding with D:0x0020 ****
0x02F3 (0x0005E6) 0x6162-       f:00060 d: 354 | A = OR[B]                      | **** non-standard encoding with D:0x0162 ****
0x02F4 (0x0005E8) 0x6F72-       f:00067 d: 370 | OR[B] = OR[B] - 1              | **** non-standard encoding with D:0x0172 ****
0x02F5 (0x0005EA) 0x7465-       f:00072 d: 101 | R = P + 101 (0x035A)          
0x02F6 (0x0005EC) 0x642E-       f:00062 d:  46 | A = A + OR[B]                  | **** non-standard encoding with D:0x002E ****
0x02F7 (0x0005EE) 0x0000-       f:00000 d:   0 | PASS                          
0x02F8 (0x0005F0) 0x4372-       f:00041 d: 370 | C = 1, io 0562 = BZ           
0x02F9 (0x0005F2) 0x6561-       f:00062 d: 353 | A = A + OR[B]                  | **** non-standard encoding with D:0x0161 ****
0x02FA (0x0005F4) 0x7469-       f:00072 d: 105 | R = P + 105 (0x0363)          
0x02FB (0x0005F6) 0x6F6E-       f:00067 d: 366 | OR[B] = OR[B] - 1              | **** non-standard encoding with D:0x016E ****
0x02FC (0x0005F8) 0x206F-       f:00020 d: 111 | A = OR[111]                   
0x02FD (0x0005FA) 0x6620-       f:00063 d:  32 | A = A - OR[B]                  | **** non-standard encoding with D:0x0020 ****
0x02FE (0x0005FC) 0x636F-       f:00061 d: 367 | A = A & OR[B]                  | **** non-standard encoding with D:0x016F ****
0x02FF (0x0005FE) 0x6E63-       f:00067 d:  99 | OR[B] = OR[B] - 1              | **** non-standard encoding with D:0x0063 ****
0x0300 (0x000600) 0x2E20-       f:00027 d:  32 | OR[32] = OR[32] - 1           
0x0301 (0x000602) 0x6163-       f:00060 d: 355 | A = OR[B]                      | **** non-standard encoding with D:0x0163 ****
0x0302 (0x000604) 0x7469-       f:00072 d: 105 | R = P + 105 (0x036B)          
0x0303 (0x000606) 0x7669-       f:00073 d: 105 | R = P - 105 (0x029A)          
0x0304 (0x000608) 0x7479-       f:00072 d: 121 | R = P + 121 (0x037D)          
0x0305 (0x00060A) 0x2066-       f:00020 d: 102 | A = OR[102]                   
0x0306 (0x00060C) 0x6169-       f:00060 d: 361 | A = OR[B]                      | **** non-standard encoding with D:0x0169 ****
0x0307 (0x00060E) 0x6C65-       f:00066 d: 101 | OR[B] = OR[B] + 1              | **** non-standard encoding with D:0x0065 ****
0x0308 (0x000610) 0x642E-       f:00062 d:  46 | A = A + OR[B]                  | **** non-standard encoding with D:0x002E ****
0x0309 (0x000612) 0x2043-       f:00020 d:  67 | A = OR[67]                    
0x030A (0x000614) 0x6F6D-       f:00067 d: 365 | OR[B] = OR[B] - 1              | **** non-standard encoding with D:0x016D ****
0x030B (0x000616) 0x6D61-       f:00066 d: 353 | OR[B] = OR[B] + 1              | **** non-standard encoding with D:0x0161 ****
0x030C (0x000618) 0x6E64-       f:00067 d: 100 | OR[B] = OR[B] - 1              | **** non-standard encoding with D:0x0064 ****
0x030D (0x00061A) 0x2061-       f:00020 d:  97 | A = OR[97]                    
0x030E (0x00061C) 0x626F-       f:00061 d: 111 | A = A & OR[B]                  | **** non-standard encoding with D:0x006F ****
0x030F (0x00061E) 0x7274-       f:00071 d: 116 | P = P - 116 (0x029B)          
0x0310 (0x000620) 0x6564-       f:00062 d: 356 | A = A + OR[B]                  | **** non-standard encoding with D:0x0164 ****
0x0311 (0x000622) 0x2E00-       f:00027 d:   0 | OR[0] = OR[0] - 1             
0x0312 (0x000624) 0x696E-       f:00064 d: 366 | OR[B] = A                      | **** non-standard encoding with D:0x016E ****
0x0313 (0x000626) 0x6974-       f:00064 d: 372 | OR[B] = A                      | **** non-standard encoding with D:0x0174 ****
0x0314 (0x000628) 0x6961-       f:00064 d: 353 | OR[B] = A                      | **** non-standard encoding with D:0x0161 ****
0x0315 (0x00062A) 0x6C69-       f:00066 d: 105 | OR[B] = OR[B] + 1              | **** non-standard encoding with D:0x0069 ****
0x0316 (0x00062C) 0x7A65-0x6400 f:00075 d: 101 | P = OR[101]+25600 (0x6400)    
0x0318 (0x000630) 0x0000-       f:00000 d:   0 | PASS                          
0x0319 (0x000632) 0x0000-       f:00000 d:   0 | PASS                          
0x031A (0x000634) 0x0000-       f:00000 d:   0 | PASS                          
0x031B (0x000636) 0x0000-       f:00000 d:   0 | PASS                          
