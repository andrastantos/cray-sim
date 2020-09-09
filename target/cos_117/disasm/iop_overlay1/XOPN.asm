@@@@@@@@@@@@@@@@@@@
@@ Load address: 0x52DC (0x52E2)
@@ Input:
@@     OR[280]: Device name 'MT' for example
@@     OR[281]: Device index 0x0000 for example for MT0
@@     OR[282]: for MT devices - 1: read, 2: write
@@     OR[283]: Volume ID string pointer or 0 if no ID
@@     OR[284]: File number to open
@@     OR[285]: 0x7500 - value of OR[6]???
@@     OR[286]: 0x0010 - OR[6] offset to store device descriptor address at

@@     OR[287]: return code
@@     OR[288]: 2k device buffer
@@     OR[289]: device descriptor address

0x0000 (0x000000) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                @@@@@@ 0x52e2
0x0001 (0x000002) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x0002 (0x000004) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0003 (0x000006) 0x2920-       f:00024 d: 288 | OR[288] = A                   
0x0004 (0x000008) 0x2118-       f:00020 d: 280 | A = OR[280]                   @ Compare to 'PR'
0x0005 (0x00000A) 0x1E00-0x5052 f:00017 d:   0 | A = A - 20562 (0x5052)        
0x0007 (0x00000E) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0009), A = 0     
0x0008 (0x000010) 0x700C-       f:00070 d:  12 | P = P + 12 (0x0014)           
0x0009 (0x000012) 0x211A-       f:00020 d: 282 | A = OR[282]                   @ OR[282] could be 0 or 'PT' (0x5054) ????
0x000A (0x000014) 0x8602-       f:00103 d:   2 | P = P + 2 (0x000C), A # 0     
0x000B (0x000016) 0x7008-       f:00070 d:   8 | P = P + 8 (0x0013)            
0x000C (0x000018) 0x211A-       f:00020 d: 282 | A = OR[282]                   
0x000D (0x00001A) 0x1E00-0x5054 f:00017 d:   0 | A = A - 20564 (0x5054)        
0x000F (0x00001E) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0011), A # 0     
0x0010 (0x000020) 0x7003-       f:00070 d:   3 | P = P + 3 (0x0013)            
0x0011 (0x000022) 0x7C34-       f:00076 d:  52 | R = OR[52]                    
0x0012 (0x000024) 0x0000-       f:00000 d:   0 | PASS                          
0x0013 (0x000026) 0x7033-       f:00070 d:  51 | P = P + 51 (0x0046)           
@ Continue comparing
0x0014 (0x000028) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0015 (0x00002A) 0x1E00-0x4D54 f:00017 d:   0 | A = A - 19796 (0x4D54)        @ Compare to 'MT'
0x0017 (0x00002E) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0019), A = 0     
0x0018 (0x000030) 0x700C-       f:00070 d:  12 | P = P + 12 (0x0024)           
0x0019 (0x000032) 0x211A-       f:00020 d: 282 | A = OR[282]                   @ OR[282] could be 1 or 2
0x001A (0x000034) 0x1601-       f:00013 d:   1 | A = A - 1 (0x0001)            
0x001B (0x000036) 0x8602-       f:00103 d:   2 | P = P + 2 (0x001D), A # 0     
0x001C (0x000038) 0x7007-       f:00070 d:   7 | P = P + 7 (0x0023)            
0x001D (0x00003A) 0x211A-       f:00020 d: 282 | A = OR[282]                   
0x001E (0x00003C) 0x1602-       f:00013 d:   2 | A = A - 2 (0x0002)            
0x001F (0x00003E) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0021), A # 0     
0x0020 (0x000040) 0x7003-       f:00070 d:   3 | P = P + 3 (0x0023)            
0x0021 (0x000042) 0x7C34-       f:00076 d:  52 | R = OR[52]                    
0x0022 (0x000044) 0x0000-       f:00000 d:   0 | PASS                          
0x0023 (0x000046) 0x7023-       f:00070 d:  35 | P = P + 35 (0x0046)           
@ Continue comparing
0x0024 (0x000048) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0025 (0x00004A) 0x1E00-0x444B f:00017 d:   0 | A = A - 17483 (0x444B)        @ Compare to 'DK'
0x0027 (0x00004E) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0029), A = 0     
0x0028 (0x000050) 0x700C-       f:00070 d:  12 | P = P + 12 (0x0034)           
0x0029 (0x000052) 0x211A-       f:00020 d: 282 | A = OR[282]                   @ OR[282] could be 3 or 4
0x002A (0x000054) 0x1603-       f:00013 d:   3 | A = A - 3 (0x0003)            
0x002B (0x000056) 0x8602-       f:00103 d:   2 | P = P + 2 (0x002D), A # 0     
0x002C (0x000058) 0x7007-       f:00070 d:   7 | P = P + 7 (0x0033)            
0x002D (0x00005A) 0x211A-       f:00020 d: 282 | A = OR[282]                   
0x002E (0x00005C) 0x1604-       f:00013 d:   4 | A = A - 4 (0x0004)            
0x002F (0x00005E) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0031), A # 0     
0x0030 (0x000060) 0x7003-       f:00070 d:   3 | P = P + 3 (0x0033)            
0x0031 (0x000062) 0x7C34-       f:00076 d:  52 | R = OR[52]                    
0x0032 (0x000064) 0x0000-       f:00000 d:   0 | PASS                          
0x0033 (0x000066) 0x7013-       f:00070 d:  19 | P = P + 19 (0x0046)           
@ Continue comparing
0x0034 (0x000068) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0035 (0x00006A) 0x1E00-0x434B f:00017 d:   0 | A = A - 17227 (0x434B)        @ Compare to 'CK'
0x0037 (0x00006E) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0039), A = 0     
0x0038 (0x000070) 0x700C-       f:00070 d:  12 | P = P + 12 (0x0044)           
0x0039 (0x000072) 0x211A-       f:00020 d: 282 | A = OR[282]                   @ OR[282] could be 1 or 2
0x003A (0x000074) 0x1601-       f:00013 d:   1 | A = A - 1 (0x0001)            
0x003B (0x000076) 0x8602-       f:00103 d:   2 | P = P + 2 (0x003D), A # 0     
0x003C (0x000078) 0x7007-       f:00070 d:   7 | P = P + 7 (0x0043)            
0x003D (0x00007A) 0x211A-       f:00020 d: 282 | A = OR[282]                   
0x003E (0x00007C) 0x1602-       f:00013 d:   2 | A = A - 2 (0x0002)            
0x003F (0x00007E) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0041), A # 0     
0x0040 (0x000080) 0x7003-       f:00070 d:   3 | P = P + 3 (0x0043)            
0x0041 (0x000082) 0x7C34-       f:00076 d:  52 | R = OR[52]                    
0x0042 (0x000084) 0x0000-       f:00000 d:   0 | PASS                          
0x0043 (0x000086) 0x7003-       f:00070 d:   3 | P = P + 3 (0x0046)            
@ Invalid device name: FATAL
0x0044 (0x000088) 0x7C34-       f:00076 d:  52 | R = OR[52]                    
0x0045 (0x00008A) 0x0000-       f:00000 d:   0 | PASS                          


@ Device name and OR[282] is validated
0x0046 (0x00008C) 0x211D-       f:00020 d: 285 | A = OR[285]                   
0x0047 (0x00008E) 0xB434-       f:00132 d:  52 | R = OR[52], A = 0             
0x0048 (0x000090) 0x0000-       f:00000 d:   0 | PASS                          
@ Reconstruct device numeral in ASCII, after checking it's not 0xffff
0x0049 (0x000092) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x004A (0x000094) 0x1E00-0xFFFF f:00017 d:   0 | A = A - 65535 (0xFFFF)        
0x004C (0x000098) 0x8602-       f:00103 d:   2 | P = P + 2 (0x004E), A # 0     
0x004D (0x00009A) 0x7005-       f:00070 d:   5 | P = P + 5 (0x0052)            
0x004E (0x00009C) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x004F (0x00009E) 0x1430-       f:00012 d:  48 | A = A + 48 (0x0030)           
0x0050 (0x0000A0) 0x0A08-       f:00005 d:   8 | A = A < 8 (0x0008)            
0x0051 (0x0000A2) 0x2919-       f:00024 d: 281 | OR[281] = A                   
0x0052 (0x0000A4) 0x7488-       f:00072 d: 136 | R = P + 136 (0x00DA)          @ Find descriptor for device, return it in OR[289], reason for match in OR[293]
0x0053 (0x0000A6) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x0054 (0x0000A8) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0056), A = 0     
0x0055 (0x0000AA) 0x7005-       f:00070 d:   5 | P = P + 5 (0x005A)            
0x0056 (0x0000AC) 0x1005-       f:00010 d:   5 | A = 5 (0x0005)                @ No descriptor found, get out of overlay with error code 5
0x0057 (0x0000AE) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x0058 (0x0000B0) 0x704C-       f:00070 d:  76 | P = P + 76 (0x00A4)           
0x0059 (0x0000B2) 0x701C-       f:00070 d:  28 | P = P + 28 (0x0075)           
0x005A (0x0000B4) 0x2125-       f:00020 d: 293 | A = OR[293]                   @ Desc. is non-NULL, let's see for the reason it's a match
0x005B (0x0000B6) 0x1602-       f:00013 d:   2 | A = A - 2 (0x0002)            
0x005C (0x0000B8) 0x8402-       f:00102 d:   2 | P = P + 2 (0x005E), A = 0     
0x005D (0x0000BA) 0x7018-       f:00070 d:  24 | P = P + 24 (0x0075)           @ Jump if we already own it
0x005E (0x0000BC) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x005F (0x0000BE) 0x291B-       f:00024 d: 283 | OR[283] = A                   @ Clear OR[283] (one of the inputs)
0x0060 (0x0000C0) 0x2118-       f:00020 d: 280 | A = OR[280]                   @ We don't own it, so is it an 'MT' device we want?
0x0061 (0x0000C2) 0x1E00-0x4D54 f:00017 d:   0 | A = A - 19796 (0x4D54)        
0x0063 (0x0000C6) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0065), A = 0     
0x0064 (0x0000C8) 0x7011-       f:00070 d:  17 | P = P + 17 (0x0075)           @ Nope
0x0065 (0x0000CA) 0x211A-       f:00020 d: 282 | A = OR[282]                   @ It is an MT device, do we intend to open for write (OR[282] == 2)?
0x0066 (0x0000CC) 0x1602-       f:00013 d:   2 | A = A - 2 (0x0002)            
0x0067 (0x0000CE) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0069), A = 0     
0x0068 (0x0000D0) 0x700D-       f:00070 d:  13 | P = P + 13 (0x0075)           @ Nope
0x0069 (0x0000D2) 0x2121-       f:00020 d: 289 | A = OR[289]                   @ OR[282] is 2, is the current file-index is what we want it to be?
0x006A (0x0000D4) 0x1412-       f:00012 d:  18 | A = A + 18 (0x0012)           
0x006B (0x0000D6) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x006C (0x0000D8) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x006D (0x0000DA) 0x271C-       f:00023 d: 284 | A = A - OR[284]               
0x006E (0x0000DC) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0070), A # 0     
0x006F (0x0000DE) 0x7006-       f:00070 d:   6 | P = P + 6 (0x0075)            @ Yes
@ So far we know this, if we're here: We're trying to open an MT device for write, which we don't own, but somebody does and the tape is not positioned where it should be
@ This means we would need to position the tape, but we're not allowed to touch it as it's aleady owned by someone else: so return with error-code 6
0x0070 (0x0000E0) 0x1006-       f:00010 d:   6 | A = 6 (0x0006)                
0x0071 (0x0000E2) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x0072 (0x0000E4) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0073 (0x0000E6) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x0074 (0x0000E8) 0x7030-       f:00070 d:  48 | P = P + 48 (0x00A4)           

@ OK, so for one reason or another we are allowed to open the device here...
@ Store 'context'? (OR[6]) at offset 8
0x0075 (0x0000EA) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x0076 (0x0000EC) 0x1408-       f:00012 d:   8 | A = A + 8 (0x0008)            
0x0077 (0x0000EE) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0078 (0x0000F0) 0x2006-       f:00020 d:   6 | A = OR[6]                     
0x0079 (0x0000F2) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
@ Clear MSB of offset 11
0x007A (0x0000F4) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x007B (0x0000F6) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x007C (0x0000F8) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x007D (0x0000FA) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x007E (0x0000FC) 0x0E01-       f:00007 d:   1 | A = A << 1 (0x0001)           
0x007F (0x0000FE) 0x0A08-       f:00005 d:   8 | A = A < 8 (0x0008)            
0x0080 (0x000100) 0x1400-       f:00012 d:   0 | A = A + 0 (0x0000)            
0x0081 (0x000102) 0x0C09-       f:00006 d:   9 | A = A >> 9 (0x0009)           
0x0082 (0x000104) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
@ Store open mode at offset 12 (could be 1 - read, or 2 - write for MT and CK, 3 or 4 for DK and 0 or PT for PR)
0x0083 (0x000106) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x0084 (0x000108) 0x140C-       f:00012 d:  12 | A = A + 12 (0x000C)           
0x0085 (0x00010A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0086 (0x00010C) 0x211A-       f:00020 d: 282 | A = OR[282]                   
0x0087 (0x00010E) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
@ Store OR[285] (conext???) at offset 13
0x0088 (0x000110) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x0089 (0x000112) 0x140D-       f:00012 d:  13 | A = A + 13 (0x000D)           
0x008A (0x000114) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x008B (0x000116) 0x211D-       f:00020 d: 285 | A = OR[285]                   
0x008C (0x000118) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 


@ Special-case for 'MT' devices
0x008D (0x00011A) 0x2118-       f:00020 d: 280 | A = OR[280]                   @ Another special-case for 'MT' devices
0x008E (0x00011C) 0x1E00-0x4D54 f:00017 d:   0 | A = A - 19796 (0x4D54)        
0x0090 (0x000120) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0092), A = 0     
0x0091 (0x000122) 0x7008-       f:00070 d:   8 | P = P + 8 (0x0099)            
0x0092 (0x000124) 0x211B-       f:00020 d: 283 | A = OR[283]                   @ Check if we need to warn the user about mouting the right volume, if yes, print out message
0x0093 (0x000126) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0095), A # 0     
0x0094 (0x000128) 0x7002-       f:00070 d:   2 | P = P + 2 (0x0096)            
0x0095 (0x00012A) 0x7496-       f:00072 d: 150 | R = P + 150 (0x012B)          @ Volume ID is specified - Print volume mounting message
0x0096 (0x00012C) 0x7E03-0x0242 f:00077 d:   3 | R = OR[3]+578 (0x0242)        @ Call device driver function 1: search for file
0x0098 (0x000130) 0x700C-       f:00070 d:  12 | P = P + 12 (0x00A4)           
@ Special-case for 'DK' devices
0x0099 (0x000132) 0x2118-       f:00020 d: 280 | A = OR[280]                   @ Is it a 'DK' device?
0x009A (0x000134) 0x1E00-0x444B f:00017 d:   0 | A = A - 17483 (0x444B)        
0x009C (0x000138) 0x8402-       f:00102 d:   2 | P = P + 2 (0x009E), A = 0     
0x009D (0x00013A) 0x7005-       f:00070 d:   5 | P = P + 5 (0x00A2)            
0x009E (0x00013C) 0x7E03-0x0242 f:00077 d:   3 | R = OR[3]+578 (0x0242)        @ Call device driver function 1: clear interrupts
0x00A0 (0x000140) 0x749F-       f:00072 d: 159 | R = P + 159 (0x013F)          @ Check if disk is initialized and if it's the requested volume
0x00A1 (0x000142) 0x7003-       f:00070 d:   3 | P = P + 3 (0x00A4)            
@ Generic case: not an 'MT' or a 'DK' device
0x00A2 (0x000144) 0x7E03-0x0242 f:00077 d:   3 | R = OR[3]+578 (0x0242)        @ Call device driver function 1: ???????
@ Common case for all devices
0x00A4 (0x000148) 0x2120-       f:00020 d: 288 | A = OR[288]                   
0x00A5 (0x00014A) 0x8602-       f:00103 d:   2 | P = P + 2 (0x00A7), A # 0     
0x00A6 (0x00014C) 0x7009-       f:00070 d:   9 | P = P + 9 (0x00AF)            
@ Call overlay manager function 27 - release buffer memory
0x00A7 (0x00014E) 0x101B-       f:00010 d:  27 | A = 27 (0x001B)               
0x00A8 (0x000150) 0x292C-       f:00024 d: 300 | OR[300] = A                   
0x00A9 (0x000152) 0x2120-       f:00020 d: 288 | A = OR[288]                   
0x00AA (0x000154) 0x292D-       f:00024 d: 301 | OR[301] = A                   
0x00AB (0x000156) 0x112C-       f:00010 d: 300 | A = 300 (0x012C)              
0x00AC (0x000158) 0x5800-       f:00054 d:   0 | B = A                         
0x00AD (0x00015A) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00AE (0x00015C) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

0x00AF (0x00015E) 0x2005-       f:00020 d:   5 | A = OR[5]                     
0x00B0 (0x000160) 0x251E-       f:00022 d: 286 | A = A + OR[286]               
0x00B1 (0x000162) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x00B2 (0x000164) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x00B3 (0x000166) 0x390D-       f:00034 d: 269 | (OR[269]) = A                 
@ Set return code (from OR[287]) and return from overlay
0x00B4 (0x000168) 0x2005-       f:00020 d:   5 | A = OR[5]                     
0x00B5 (0x00016A) 0x1406-       f:00012 d:   6 | A = A + 6 (0x0006)            
0x00B6 (0x00016C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00B7 (0x00016E) 0x211F-       f:00020 d: 287 | A = OR[287]                   
0x00B8 (0x000170) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x00B9 (0x000172) 0x102A-       f:00010 d:  42 | A = 42 (0x002A)               
0x00BA (0x000174) 0x292C-       f:00024 d: 300 | OR[300] = A                   
0x00BB (0x000176) 0x112C-       f:00010 d: 300 | A = 300 (0x012C)              
0x00BC (0x000178) 0x5800-       f:00054 d:   0 | B = A                         
0x00BD (0x00017A) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00BE (0x00017C) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

0x00BF (0x00017E) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x00C0 (0x000180) 0x1408-       f:00012 d:   8 | A = A + 8 (0x0008)            
0x00C1 (0x000182) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00C2 (0x000184) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00C3 (0x000186) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x00C4 (0x000188) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x00C5 (0x00018A) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x00C6 (0x00018C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00C7 (0x00018E) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x00C8 (0x000190) 0x0E01-       f:00007 d:   1 | A = A << 1 (0x0001)           
0x00C9 (0x000192) 0x0A08-       f:00005 d:   8 | A = A < 8 (0x0008)            
0x00CA (0x000194) 0x1400-       f:00012 d:   0 | A = A + 0 (0x0000)            
0x00CB (0x000196) 0x0C09-       f:00006 d:   9 | A = A >> 9 (0x0009)           
0x00CC (0x000198) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x00CD (0x00019A) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x00CE (0x00019C) 0x140C-       f:00012 d:  12 | A = A + 12 (0x000C)           
0x00CF (0x00019E) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00D0 (0x0001A0) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00D1 (0x0001A2) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x00D2 (0x0001A4) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x00D3 (0x0001A6) 0x140D-       f:00012 d:  13 | A = A + 13 (0x000D)           
0x00D4 (0x0001A8) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00D5 (0x0001AA) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00D6 (0x0001AC) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x00D7 (0x0001AE) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00D8 (0x0001B0) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x00D9 (0x0001B2) 0x7235-       f:00071 d:  53 | P = P - 53 (0x00A4)           

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Input: none
@@    OR[280]: device name
@@    OR[281]: device number
@@ Return:
@@    OR[289]: desc. address for the first matching device
@@    OR[293]: 0 - not valid
@@             1 - valid, not 'owned'??
@@             2 - valid, owned by the requestor
@@             3 - ???
@ 0x3949 seems to be some sort of a device descriptor table
0x00DA (0x0001B4) 0x1800-0x3949 f:00014 d:   0 | A = 14665 (0x3949)            @@@@@@@@@@ 0x53bc
0x00DC (0x0001B8) 0x2924-       f:00024 d: 292 | OR[292] = A                   
0x00DD (0x0001BA) 0x3124-       f:00030 d: 292 | A = (OR[292])                 
0x00DE (0x0001BC) 0x840A-       f:00102 d:  10 | P = P + 10 (0x00E8), A = 0    
0x00DF (0x0001BE) 0x3124-       f:00030 d: 292 | A = (OR[292])                 
0x00E0 (0x0001C0) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x00E1 (0x0001C2) 0x740A-       f:00072 d:  10 | R = P + 10 (0x00EB)           
0x00E2 (0x0001C4) 0x2125-       f:00020 d: 293 | A = OR[293]                   
0x00E3 (0x0001C6) 0x8602-       f:00103 d:   2 | P = P + 2 (0x00E5), A # 0     
0x00E4 (0x0001C8) 0x7002-       f:00070 d:   2 | P = P + 2 (0x00E6)            
0x00E5 (0x0001CA) 0x0200-       f:00001 d:   0 | EXIT                          
0x00E6 (0x0001CC) 0x2D24-       f:00026 d: 292 | OR[292] = OR[292] + 1         
0x00E7 (0x0001CE) 0x720A-       f:00071 d:  10 | P = P - 10 (0x00DD)           
0x00E8 (0x0001D0) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00E9 (0x0001D2) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x00EA (0x0001D4) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Test device descriptor
@@ Input:
@@    OR[289]: desc. address
@@    OR[280]: device name
@@    OR[281]: device number
@@ Return:
@@    OR[293]: 0 - not valid
@@             1 - valid, not 'owned'??
@@             2 - valid, owned by the requestor
@@             3 - ???
0x00EB (0x0001D6) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00EC (0x0001D8) 0x2925-       f:00024 d: 293 | OR[293] = A                   
0x00ED (0x0001DA) 0x3121-       f:00030 d: 289 | A = (OR[289])                 
0x00EE (0x0001DC) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x00EF (0x0001DE) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x00F0 (0x0001E0) 0x2726-       f:00023 d: 294 | A = A - OR[294]               
0x00F1 (0x0001E2) 0x8402-       f:00102 d:   2 | P = P + 2 (0x00F3), A = 0     
0x00F2 (0x0001E4) 0x7038-       f:00070 d:  56 | P = P + 56 (0x012A)           
@ Device name matches
0x00F3 (0x0001E6) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x00F4 (0x0001E8) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x00F5 (0x0001EA) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00F6 (0x0001EC) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x00F7 (0x0001EE) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x00F8 (0x0001F0) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x00F9 (0x0001F2) 0x1E00-0xFFFF f:00017 d:   0 | A = A - 65535 (0xFFFF)        
0x00FB (0x0001F6) 0x8405-       f:00102 d:   5 | P = P + 5 (0x0100), A = 0     
0x00FC (0x0001F8) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x00FD (0x0001FA) 0x2726-       f:00023 d: 294 | A = A - OR[294]               
0x00FE (0x0001FC) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0100), A = 0     
0x00FF (0x0001FE) 0x702B-       f:00070 d:  43 | P = P + 43 (0x012A)           
@ Device number matches
0x0100 (0x000200) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x0101 (0x000202) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x0102 (0x000204) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0103 (0x000206) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0104 (0x000208) 0x0804-       f:00004 d:   4 | A = A > 4 (0x0004)            
0x0105 (0x00020A) 0x120F-       f:00011 d:  15 | A = A & 15 (0x000F)           
0x0106 (0x00020C) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x0107 (0x00020E) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x0108 (0x000210) 0x8402-       f:00102 d:   2 | P = P + 2 (0x010A), A = 0     
0x0109 (0x000212) 0x7021-       f:00070 d:  33 | P = P + 33 (0x012A)           
@ Entry 11, bits 7...4 are all 0-s
0x010A (0x000214) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x010B (0x000216) 0x140D-       f:00012 d:  13 | A = A + 13 (0x000D)           
0x010C (0x000218) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x010D (0x00021A) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x010E (0x00021C) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x010F (0x00021E) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x0110 (0x000220) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0112), A = 0     
0x0111 (0x000222) 0x7004-       f:00070 d:   4 | P = P + 4 (0x0115)            
@ Entry 13 is 0 - return 1 (success)
0x0112 (0x000224) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x0113 (0x000226) 0x2925-       f:00024 d: 293 | OR[293] = A                   
0x0114 (0x000228) 0x7016-       f:00070 d:  22 | P = P + 22 (0x012A)           
@ Entry 13 is non-0
0x0115 (0x00022A) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x0116 (0x00022C) 0x271D-       f:00023 d: 285 | A = A - OR[285]               
0x0117 (0x00022E) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0119), A = 0     
0x0118 (0x000230) 0x7012-       f:00070 d:  18 | P = P + 18 (0x012A)           
@ Entry 13 is the same as OR[285], which is an input (conext????)
0x0119 (0x000232) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x011A (0x000234) 0x1408-       f:00012 d:   8 | A = A + 8 (0x0008)            
0x011B (0x000236) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x011C (0x000238) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x011D (0x00023A) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x011E (0x00023C) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x011F (0x00023E) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0121), A = 0     
0x0120 (0x000240) 0x7004-       f:00070 d:   4 | P = P + 4 (0x0124)            
@ Entry 8 is 0 - return 2 (success)
0x0121 (0x000242) 0x1002-       f:00010 d:   2 | A = 2 (0x0002)                
0x0122 (0x000244) 0x2925-       f:00024 d: 293 | OR[293] = A                   
0x0123 (0x000246) 0x7007-       f:00070 d:   7 | P = P + 7 (0x012A)            
0x0124 (0x000248) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x0125 (0x00024A) 0x2606-       f:00023 d:   6 | A = A - OR[6]                 
0x0126 (0x00024C) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0128), A = 0     
0x0127 (0x00024E) 0x7003-       f:00070 d:   3 | P = P + 3 (0x012A)            @ Entry 8 is non-0 and non OR[6] - return 0
@ Entry 8 is OR[6] - return 3 (success)
0x0128 (0x000250) 0x1003-       f:00010 d:   3 | A = 3 (0x0003)                
0x0129 (0x000252) 0x2925-       f:00024 d: 293 | OR[293] = A                   
0x012A (0x000254) 0x0200-       f:00001 d:   0 | EXIT                          


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Print volume mounting message
0x012B (0x000256) 0x211A-       f:00020 d: 282 | A = OR[282]                   @ OR[282] could be 1 - read or 2 - write for MT devices
0x012C (0x000258) 0x1601-       f:00013 d:   1 | A = A - 1 (0x0001)            
0x012D (0x00025A) 0x8402-       f:00102 d:   2 | P = P + 2 (0x012F), A = 0     
0x012E (0x00025C) 0x7005-       f:00070 d:   5 | P = P + 5 (0x0133)            @ It's not 1
0x012F (0x00025E) 0x1800-0x0281 f:00014 d:   0 | A = 641 (0x0281)              @ It's 1: Print message 0x0281: '@?S:?D PLEASE MOUNT TAPE FOR READING, ID=?S'
0x0131 (0x000262) 0x2928-       f:00024 d: 296 | OR[296] = A                   
0x0132 (0x000264) 0x700B-       f:00070 d:  11 | P = P + 11 (0x013D)           
0x0133 (0x000266) 0x211A-       f:00020 d: 282 | A = OR[282]                   
0x0134 (0x000268) 0x1602-       f:00013 d:   2 | A = A - 2 (0x0002)            
0x0135 (0x00026A) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0137), A = 0     
0x0136 (0x00026C) 0x7005-       f:00070 d:   5 | P = P + 5 (0x013B)            
0x0137 (0x00026E) 0x1800-0x0282 f:00014 d:   0 | A = 642 (0x0282)              @ It's 2: Print message 0x0282: '@?S:?D PLEASE MOUNT TAPE FOR WRITING, ID=?S.'
0x0139 (0x000272) 0x2928-       f:00024 d: 296 | OR[296] = A                   
0x013A (0x000274) 0x7003-       f:00070 d:   3 | P = P + 3 (0x013D)            
0x013B (0x000276) 0x7C34-       f:00076 d:  52 | R = OR[52]                    @ Unknown open mode: FATAL
0x013C (0x000278) 0x0000-       f:00000 d:   0 | PASS                          
0x013D (0x00027A) 0x7457-       f:00072 d:  87 | R = P + 87 (0x0194)           @ Print the message we've decided on above
0x013E (0x00027C) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Read sector 3 from DK device
@ Call overlay manager function 26 - allocate 2k memory and return in OR[288]
0x013F (0x00027E) 0x101A-       f:00010 d:  26 | A = 26 (0x001A)               
0x0140 (0x000280) 0x292C-       f:00024 d: 300 | OR[300] = A                   
0x0141 (0x000282) 0x1120-       f:00010 d: 288 | A = 288 (0x0120)              
0x0142 (0x000284) 0x292D-       f:00024 d: 301 | OR[301] = A                   
0x0143 (0x000286) 0x112C-       f:00010 d: 300 | A = 300 (0x012C)              
0x0144 (0x000288) 0x5800-       f:00054 d:   0 | B = A                         
0x0145 (0x00028A) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0146 (0x00028C) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

0x0147 (0x00028E) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0149), A # 0     
0x0148 (0x000290) 0x700B-       f:00070 d:  11 | P = P + 11 (0x0153)           
@ Call overlay manager function 7: YIELD
0x0149 (0x000292) 0x1007-       f:00010 d:   7 | A = 7 (0x0007)                
0x014A (0x000294) 0x292C-       f:00024 d: 300 | OR[300] = A                   
0x014B (0x000296) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x014C (0x000298) 0x292D-       f:00024 d: 301 | OR[301] = A                   
0x014D (0x00029A) 0x112C-       f:00010 d: 300 | A = 300 (0x012C)              
0x014E (0x00029C) 0x5800-       f:00054 d:   0 | B = A                         
0x014F (0x00029E) 0x1800-0x2918 f:00014 d:   0 | A = 10520 (0x2918)            
0x0151 (0x0002A2) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

0x0152 (0x0002A4) 0x7213-       f:00071 d:  19 | P = P - 19 (0x013F)           
@ Allocation was successful
0x0153 (0x0002A6) 0x211A-       f:00020 d: 282 | A = OR[282]                   
0x0154 (0x0002A8) 0x1604-       f:00013 d:   4 | A = A - 4 (0x0004)            
0x0155 (0x0002AA) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0157), A # 0     
0x0156 (0x0002AC) 0x703D-       f:00070 d:  61 | P = P + 61 (0x0193)           @ If open mode OR[282] == 4 - return

@ Call device driver, function code 4 (read sector 3 from device)
0x0157 (0x0002AE) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x0158 (0x0002B0) 0x1403-       f:00012 d:   3 | A = A + 3 (0x0003)            
0x0159 (0x0002B2) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x015A (0x0002B4) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x015B (0x0002B6) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x015C (0x0002B8) 0x1028-       f:00010 d:  40 | A = 40 (0x0028)               
0x015D (0x0002BA) 0x292C-       f:00024 d: 300 | OR[300] = A                   
0x015E (0x0002BC) 0x210D-       f:00020 d: 269 | A = OR[269]                   @ Device driver overlay number
0x015F (0x0002BE) 0x292D-       f:00024 d: 301 | OR[301] = A                   
0x0160 (0x0002C0) 0x2121-       f:00020 d: 289 | A = OR[289]                   @ Device desc. address
0x0161 (0x0002C2) 0x292E-       f:00024 d: 302 | OR[302] = A                   
0x0162 (0x0002C4) 0x1004-       f:00010 d:   4 | A = 4 (0x0004)                @ Function code: 4 - read
0x0163 (0x0002C6) 0x292F-       f:00024 d: 303 | OR[303] = A                   
0x0164 (0x0002C8) 0x2120-       f:00020 d: 288 | A = OR[288]                   @ Buffer address
0x0165 (0x0002CA) 0x2930-       f:00024 d: 304 | OR[304] = A                   
0x0166 (0x0002CC) 0x1003-       f:00010 d:   3 | A = 3 (0x0003)                @ Sector address: 3
0x0167 (0x0002CE) 0x2932-       f:00024 d: 306 | OR[306] = A                   
0x0168 (0x0002D0) 0x112C-       f:00010 d: 300 | A = 300 (0x012C)              
0x0169 (0x0002D2) 0x5800-       f:00054 d:   0 | B = A                         
0x016A (0x0002D4) 0x1800-0x2918 f:00014 d:   0 | A = 10520 (0x2918)            
0x016C (0x0002D8) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

0x016D (0x0002DA) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x016E (0x0002DC) 0x211F-       f:00020 d: 287 | A = OR[287]                   
0x016F (0x0002DE) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0171), A # 0     
0x0170 (0x0002E0) 0x7002-       f:00070 d:   2 | P = P + 2 (0x0172)            
0x0171 (0x0002E2) 0x72B2-       f:00071 d: 178 | P = P - 178 (0x00BF)          @ Unsuccessful read - return from overlay with error-code
@ Test returned data: is disk initialized?
0x0172 (0x0002E4) 0x3120-       f:00030 d: 288 | A = (OR[288])                 
0x0173 (0x0002E6) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x0174 (0x0002E8) 0x2127-       f:00020 d: 295 | A = OR[295]                   
0x0175 (0x0002EA) 0x1E00-0xFFFF f:00017 d:   0 | A = A - 65535 (0xFFFF)        
0x0177 (0x0002EE) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0179), A # 0     
0x0178 (0x0002F0) 0x700E-       f:00070 d:  14 | P = P + 14 (0x0186)           
0x0179 (0x0002F2) 0x211B-       f:00020 d: 283 | A = OR[283]                   @ TapeID for MT devices, but what is it for DK ones?
0x017A (0x0002F4) 0x8602-       f:00103 d:   2 | P = P + 2 (0x017C), A # 0     
0x017B (0x0002F6) 0x7007-       f:00070 d:   7 | P = P + 7 (0x0182)            
@ So first WORD on disk, sector 3 is not 0xFFFF and OR[283] (volume ID) is non-0
0x017C (0x0002F8) 0x1800-0x02C1 f:00014 d:   0 | A = 705 (0x02C1)              @ Print string: '@?S: PLEASE MOUNT DISK FOR IO, ID=?S'
0x017E (0x0002FC) 0x2928-       f:00024 d: 296 | OR[296] = A                   
0x017F (0x0002FE) 0x7415-       f:00072 d:  21 | R = P + 21 (0x0194)           
0x0180 (0x000300) 0x722D-       f:00071 d:  45 | P = P - 45 (0x0153)           
0x0181 (0x000302) 0x7004-       f:00070 d:   4 | P = P + 4 (0x0185)            
@ So first WORD on disk, sector 3 is not 0xFFFF, but OR[283] (volume ID) is 0
0x0182 (0x000304) 0x100A-       f:00010 d:  10 | A = 10 (0x000A)               @ Bail from overlay with error-code 10
0x0183 (0x000306) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x0184 (0x000308) 0x72C5-       f:00071 d: 197 | P = P - 197 (0x00BF)          
0x0185 (0x00030A) 0x700E-       f:00070 d:  14 | P = P + 14 (0x0193)           

@ So first WORD on disk, sector 3 is 0xFFFF - which means disk is initialzed
0x0186 (0x00030C) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x0187 (0x00030E) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0189), A # 0     
0x0188 (0x000310) 0x700B-       f:00070 d:  11 | P = P + 11 (0x0193)           @ We don't have a volume ID passed it, so we're done
0x0189 (0x000312) 0x7E03-0x020A f:00077 d:   3 | R = OR[3]+522 (0x020A)        @ Compare volume ID on disk with requested one  OR[297]: 0 - doesn't match  1 - matches
0x018B (0x000316) 0x2129-       f:00020 d: 297 | A = OR[297]                   
0x018C (0x000318) 0x8402-       f:00102 d:   2 | P = P + 2 (0x018E), A = 0     
0x018D (0x00031A) 0x7006-       f:00070 d:   6 | P = P + 6 (0x0193)            @ Volume ID matches: exit
0x018E (0x00031C) 0x1800-0x02C1 f:00014 d:   0 | A = 705 (0x02C1)              @ Volume ID doesn't match, print message: '@?S: PLEASE MOUNT DISK FOR IO, ID=?S'
0x0190 (0x000320) 0x2928-       f:00024 d: 296 | OR[296] = A                   
0x0191 (0x000322) 0x7403-       f:00072 d:   3 | R = P + 3 (0x0194)            
0x0192 (0x000324) 0x723F-       f:00071 d:  63 | P = P - 63 (0x0153)           
0x0193 (0x000326) 0x0200-       f:00001 d:   0 | EXIT                          



@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Print message
@@ Input:
@@     OR[296]: message code
@@     OR[289]: device desc.
0x0194 (0x000328) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x0195 (0x00032A) 0x1400-       f:00012 d:   0 | A = A + 0 (0x0000)            
0x0196 (0x00032C) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x0197 (0x00032E) 0x2128-       f:00020 d: 296 | A = OR[296]                   
0x0198 (0x000330) 0x1E00-0x0281 f:00017 d:   0 | A = A - 641 (0x0281)          
0x019A (0x000334) 0x8406-       f:00102 d:   6 | P = P + 6 (0x01A0), A = 0     
0x019B (0x000336) 0x2128-       f:00020 d: 296 | A = OR[296]                   
0x019C (0x000338) 0x1E00-0x0282 f:00017 d:   0 | A = A - 642 (0x0282)          
0x019E (0x00033C) 0x8402-       f:00102 d:   2 | P = P + 2 (0x01A0), A = 0     
0x019F (0x00033E) 0x7014-       f:00070 d:  20 | P = P + 20 (0x01B3)           

@ Call MSGHND
0x01A0 (0x000340) 0x1028-       f:00010 d:  40 | A = 40 (0x0028)               
0x01A1 (0x000342) 0x292C-       f:00024 d: 300 | OR[300] = A                   
0x01A2 (0x000344) 0x1800-0x002F f:00014 d:   0 | A = 47 (0x002F)               
0x01A4 (0x000348) 0x292D-       f:00024 d: 301 | OR[301] = A                   
0x01A5 (0x00034A) 0x2128-       f:00020 d: 296 | A = OR[296]                   
0x01A6 (0x00034C) 0x292E-       f:00024 d: 302 | OR[302] = A                   
0x01A7 (0x00034E) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x01A8 (0x000350) 0x292F-       f:00024 d: 303 | OR[303] = A                   
0x01A9 (0x000352) 0x211C-       f:00020 d: 284 | A = OR[284]                   
0x01AA (0x000354) 0x2930-       f:00024 d: 304 | OR[304] = A                   
0x01AB (0x000356) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x01AC (0x000358) 0x2931-       f:00024 d: 305 | OR[305] = A                   
0x01AD (0x00035A) 0x112C-       f:00010 d: 300 | A = 300 (0x012C)              
0x01AE (0x00035C) 0x5800-       f:00054 d:   0 | B = A                         
0x01AF (0x00035E) 0x1800-0x2918 f:00014 d:   0 | A = 10520 (0x2918)            
0x01B1 (0x000362) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

0x01B2 (0x000364) 0x7011-       f:00070 d:  17 | P = P + 17 (0x01C3)           
@ Call MSGHND
0x01B3 (0x000366) 0x1028-       f:00010 d:  40 | A = 40 (0x0028)               
0x01B4 (0x000368) 0x292C-       f:00024 d: 300 | OR[300] = A                   
0x01B5 (0x00036A) 0x1800-0x002F f:00014 d:   0 | A = 47 (0x002F)               
0x01B7 (0x00036E) 0x292D-       f:00024 d: 301 | OR[301] = A                   
0x01B8 (0x000370) 0x2128-       f:00020 d: 296 | A = OR[296]                   
0x01B9 (0x000372) 0x292E-       f:00024 d: 302 | OR[302] = A                   
0x01BA (0x000374) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x01BB (0x000376) 0x292F-       f:00024 d: 303 | OR[303] = A                   
0x01BC (0x000378) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x01BD (0x00037A) 0x2930-       f:00024 d: 304 | OR[304] = A                   
0x01BE (0x00037C) 0x112C-       f:00010 d: 300 | A = 300 (0x012C)              
0x01BF (0x00037E) 0x5800-       f:00054 d:   0 | B = A                         
0x01C0 (0x000380) 0x1800-0x2918 f:00014 d:   0 | A = 10520 (0x2918)            
0x01C2 (0x000384) 0x7C09-       f:00076 d:   9 | R = OR[9]                     
@ Call MSGHND
0x01C3 (0x000386) 0x1028-       f:00010 d:  40 | A = 40 (0x0028)               
0x01C4 (0x000388) 0x292C-       f:00024 d: 300 | OR[300] = A                   
0x01C5 (0x00038A) 0x1800-0x002F f:00014 d:   0 | A = 47 (0x002F)               
0x01C7 (0x00038E) 0x292D-       f:00024 d: 301 | OR[301] = A                   
0x01C8 (0x000390) 0x1800-0x0201 f:00014 d:   0 | A = 513 (0x0201)              
0x01CA (0x000394) 0x292E-       f:00024 d: 302 | OR[302] = A                   
0x01CB (0x000396) 0x112C-       f:00010 d: 300 | A = 300 (0x012C)              
0x01CC (0x000398) 0x5800-       f:00054 d:   0 | B = A                         
0x01CD (0x00039A) 0x1800-0x2918 f:00014 d:   0 | A = 10520 (0x2918)            
0x01CF (0x00039E) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

0x01D0 (0x0003A0) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x01D1 (0x0003A2) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x01D2 (0x0003A4) 0x211F-       f:00020 d: 287 | A = OR[287]                   
0x01D3 (0x0003A6) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x01D4 (0x0003A8) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x01D5 (0x0003AA) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x01D6 (0x0003AC) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x01D7 (0x0003AE) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x01D8 (0x0003B0) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x01D9 (0x0003B2) 0x0A09-       f:00005 d:   9 | A = A < 9 (0x0009)            
0x01DA (0x0003B4) 0x251F-       f:00022 d: 287 | A = A + OR[287]               
0x01DB (0x0003B6) 0x0C09-       f:00006 d:   9 | A = A >> 9 (0x0009)           
0x01DC (0x0003B8) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x01DD (0x0003BA) 0x211F-       f:00020 d: 287 | A = OR[287]                   
0x01DE (0x0003BC) 0x1601-       f:00013 d:   1 | A = A - 1 (0x0001)            
0x01DF (0x0003BE) 0x8611-       f:00103 d:  17 | P = P + 17 (0x01F0), A # 0    
@ YIELD
0x01E0 (0x0003C0) 0x1007-       f:00010 d:   7 | A = 7 (0x0007)                
0x01E1 (0x0003C2) 0x292C-       f:00024 d: 300 | OR[300] = A                   
0x01E2 (0x0003C4) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x01E3 (0x0003C6) 0x292D-       f:00024 d: 301 | OR[301] = A                   
0x01E4 (0x0003C8) 0x112C-       f:00010 d: 300 | A = 300 (0x012C)              
0x01E5 (0x0003CA) 0x5800-       f:00054 d:   0 | B = A                         
0x01E6 (0x0003CC) 0x1800-0x2918 f:00014 d:   0 | A = 10520 (0x2918)            
0x01E8 (0x0003D0) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

0x01E9 (0x0003D2) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x01EA (0x0003D4) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x01EB (0x0003D6) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x01EC (0x0003D8) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x01ED (0x0003DA) 0x0808-       f:00004 d:   8 | A = A > 8 (0x0008)            
0x01EE (0x0003DC) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x01EF (0x0003DE) 0x7212-       f:00071 d:  18 | P = P - 18 (0x01DD)           
0x01F0 (0x0003E0) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x01F1 (0x0003E2) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x01F2 (0x0003E4) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x01F3 (0x0003E6) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x01F4 (0x0003E8) 0x0E01-       f:00007 d:   1 | A = A << 1 (0x0001)           
0x01F5 (0x0003EA) 0x0A08-       f:00005 d:   8 | A = A < 8 (0x0008)            
0x01F6 (0x0003EC) 0x1400-       f:00012 d:   0 | A = A + 0 (0x0000)            
0x01F7 (0x0003EE) 0x0C09-       f:00006 d:   9 | A = A >> 9 (0x0009)           
0x01F8 (0x0003F0) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x01F9 (0x0003F2) 0x211F-       f:00020 d: 287 | A = OR[287]                   
0x01FA (0x0003F4) 0x1604-       f:00013 d:   4 | A = A - 4 (0x0004)            
0x01FB (0x0003F6) 0xAC03-0x0205 f:00126 d:   3 | P = OR[3]+517 (0x0205), A = 0 
0x01FD (0x0003FA) 0x7A03-0x0208 f:00075 d:   3 | P = OR[3]+520 (0x0208)        
0x01FF (0x0003FE) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0200 (0x000400) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x0201 (0x000402) 0x7002-       f:00070 d:   2 | P = P + 2 (0x0203)            
0x0202 (0x000404) 0x7343-       f:00071 d: 323 | P = P - 323 (0x00BF)          
0x0203 (0x000406) 0x0200-       f:00001 d:   0 | EXIT                          


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Compare volume ID on disk with requested one 
@@ Input:
@@     OR[288]: buffer containing sector 3
@@     OR[283]: pointer to volume ID string
@@ Output:
@@     OR[297]: 0 - doesn't match
@@              1 - matches
0x0204 (0x000408) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0205 (0x00040A) 0x2929-       f:00024 d: 297 | OR[297] = A                   
0x0206 (0x00040C) 0x2120-       f:00020 d: 288 | A = OR[288]                   
0x0207 (0x00040E) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x0208 (0x000410) 0x292A-       f:00024 d: 298 | OR[298] = A                   
@ We have a loop here from 0 through 8
0x0209 (0x000412) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x020A (0x000414) 0x292B-       f:00024 d: 299 | OR[299] = A                   
0x020B (0x000416) 0x212B-       f:00020 d: 299 | A = OR[299]                   
0x020C (0x000418) 0x1608-       f:00013 d:   8 | A = A - 8 (0x0008)            
0x020D (0x00041A) 0x842C-       f:00102 d:  44 | P = P + 44 (0x0239), A = 0    
@ Read WORD[idx/2] into OR[269]
0x020E (0x00041C) 0x212B-       f:00020 d: 299 | A = OR[299]                   
0x020F (0x00041E) 0x0801-       f:00004 d:   1 | A = A > 1 (0x0001)            
0x0210 (0x000420) 0x252A-       f:00022 d: 298 | A = A + OR[298]               
0x0211 (0x000422) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x0212 (0x000424) 0x310D-       f:00030 d: 269 | A = (OR[269])                 
0x0213 (0x000426) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x0214 (0x000428) 0x212B-       f:00020 d: 299 | A = OR[299]                   
0x0215 (0x00042A) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x0216 (0x00042C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0217 (0x00042E) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0218 (0x000430) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x0219 (0x000432) 0x8604-       f:00103 d:   4 | P = P + 4 (0x021D), A # 0     
0x021A (0x000434) 0x210D-       f:00020 d: 269 | A = OR[269]                   
0x021B (0x000436) 0x0808-       f:00004 d:   8 | A = A > 8 (0x0008)            
0x021C (0x000438) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x021D (0x00043A) 0x210D-       f:00020 d: 269 | A = OR[269]                   
0x021E (0x00043C) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x021F (0x00043E) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x0220 (0x000440) 0x212B-       f:00020 d: 299 | A = OR[299]                   
0x0221 (0x000442) 0x0801-       f:00004 d:   1 | A = A > 1 (0x0001)            
0x0222 (0x000444) 0x251B-       f:00022 d: 283 | A = A + OR[283]               
0x0223 (0x000446) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x0224 (0x000448) 0x310D-       f:00030 d: 269 | A = (OR[269])                 
0x0225 (0x00044A) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x0226 (0x00044C) 0x212B-       f:00020 d: 299 | A = OR[299]                   
0x0227 (0x00044E) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x0228 (0x000450) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0229 (0x000452) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x022A (0x000454) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x022B (0x000456) 0x8604-       f:00103 d:   4 | P = P + 4 (0x022F), A # 0     
0x022C (0x000458) 0x210D-       f:00020 d: 269 | A = OR[269]                   
0x022D (0x00045A) 0x0808-       f:00004 d:   8 | A = A > 8 (0x0008)            
0x022E (0x00045C) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x022F (0x00045E) 0x210D-       f:00020 d: 269 | A = OR[269]                   
0x0230 (0x000460) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x0231 (0x000462) 0x2914-       f:00024 d: 276 | OR[276] = A                   
0x0232 (0x000464) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x0233 (0x000466) 0x2714-       f:00023 d: 276 | A = A - OR[276]               
0x0234 (0x000468) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0236), A # 0     
0x0235 (0x00046A) 0x7002-       f:00070 d:   2 | P = P + 2 (0x0237)            
0x0236 (0x00046C) 0x0200-       f:00001 d:   0 | EXIT                          
0x0237 (0x00046E) 0x2D2B-       f:00026 d: 299 | OR[299] = OR[299] + 1         
0x0238 (0x000470) 0x722D-       f:00071 d:  45 | P = P - 45 (0x020B)           
0x0239 (0x000472) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x023A (0x000474) 0x2929-       f:00024 d: 297 | OR[297] = A                   
0x023B (0x000476) 0x0200-       f:00001 d:   0 | EXIT                          

@ Call device driver function 1: search for file
0x023C (0x000478) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x023D (0x00047A) 0x1403-       f:00012 d:   3 | A = A + 3 (0x0003)            
0x023E (0x00047C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x023F (0x00047E) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0240 (0x000480) 0x2923-       f:00024 d: 291 | OR[291] = A                   
0x0241 (0x000482) 0x1028-       f:00010 d:  40 | A = 40 (0x0028)               
0x0242 (0x000484) 0x292C-       f:00024 d: 300 | OR[300] = A                   
0x0243 (0x000486) 0x2123-       f:00020 d: 291 | A = OR[291]                   @ Device driver
0x0244 (0x000488) 0x292D-       f:00024 d: 301 | OR[301] = A                   
0x0245 (0x00048A) 0x2121-       f:00020 d: 289 | A = OR[289]                   @ Desc. address
0x0246 (0x00048C) 0x292E-       f:00024 d: 302 | OR[302] = A                   
0x0247 (0x00048E) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                @ Function code: 1 - search for file # passed in in next arg
0x0248 (0x000490) 0x292F-       f:00024 d: 303 | OR[303] = A                   
0x0249 (0x000492) 0x211C-       f:00020 d: 284 | A = OR[284]                   @ File number
0x024A (0x000494) 0x2930-       f:00024 d: 304 | OR[304] = A                   
0x024B (0x000496) 0x112C-       f:00010 d: 300 | A = 300 (0x012C)              
0x024C (0x000498) 0x5800-       f:00054 d:   0 | B = A                         
0x024D (0x00049A) 0x1800-0x2918 f:00014 d:   0 | A = 10520 (0x2918)            
0x024F (0x00049E) 0x7C09-       f:00076 d:   9 | R = OR[9]                     
@ Check return code and either exit on success or (probably) fail from overlay with error-code
0x0250 (0x0004A0) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x0251 (0x0004A2) 0x211F-       f:00020 d: 287 | A = OR[287]                   
0x0252 (0x0004A4) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0254), A # 0     
0x0253 (0x0004A6) 0x7002-       f:00070 d:   2 | P = P + 2 (0x0255)            
0x0254 (0x0004A8) 0x7395-       f:00071 d: 405 | P = P - 405 (0x00BF)          
0x0255 (0x0004AA) 0x0200-       f:00001 d:   0 | EXIT                          
0x0256 (0x0004AC) 0x0000-       f:00000 d:   0 | PASS                          
0x0257 (0x0004AE) 0x0000-       f:00000 d:   0 | PASS                          
