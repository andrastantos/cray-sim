@ This gets loaded into address 0x5DFA
@ OR[280] - device desc. addr.
@ OR[281] - function code
@           1 - search for file# passed in in OR[282]
@           2 - search for file# passed in in OR[282]
@           3 - write
@           4 - read
@           5 - close file
@           6 - close tape
@ OR[282] - buffer address
@ OR[283] - buffer size (up to 2048)
@
@ OR[284]
@ Return value:
@   0 - OK
@   2 - invalid driver - that is XTAPE is called for a non-MT device, or if not opened??

@ Test if we're an MT device
0x0000 (0x000000) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0001 (0x000002) 0x291C-       f:00024 d: 284 | OR[284] = A                   
0x0002 (0x000004) 0x3118-       f:00030 d: 280 | A = (OR[280])                 
0x0003 (0x000006) 0x1E00-0x4D54 f:00017 d:   0 | A = A - 19796 (0x4D54)        
0x0005 (0x00000A) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0007), A # 0     
0x0006 (0x00000C) 0x7004-       f:00070 d:   4 | P = P + 4 (0x000A)            
0x0007 (0x00000E) 0x1002-       f:00010 d:   2 | A = 2 (0x0002)                
0x0008 (0x000010) 0x291C-       f:00024 d: 284 | OR[284] = A                   
0x0009 (0x000012) 0x7050-       f:00070 d:  80 | P = P + 80 (0x0059)           
@ Yes, we are. Now check that the context (OR[6]) is the same where we got opened
0x000A (0x000014) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x000B (0x000016) 0x1408-       f:00012 d:   8 | A = A + 8 (0x0008)            
0x000C (0x000018) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x000D (0x00001A) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x000E (0x00001C) 0x2606-       f:00023 d:   6 | A = A - OR[6]                 
0x000F (0x00001E) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0011), A # 0     
0x0010 (0x000020) 0x7004-       f:00070 d:   4 | P = P + 4 (0x0014)            
0x0011 (0x000022) 0x1002-       f:00010 d:   2 | A = 2 (0x0002)                
0x0012 (0x000024) 0x291C-       f:00024 d: 284 | OR[284] = A                   
0x0013 (0x000026) 0x7046-       f:00070 d:  70 | P = P + 70 (0x0059)           
@ We're OK
0x0014 (0x000028) 0x7562-       f:00072 d: 354 | R = P + 354 (0x0176)          
@ Set device address
0x0015 (0x00002A) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0016 (0x00002C) 0x1402-       f:00012 d:   2 | A = A + 2 (0x0002)            
0x0017 (0x00002E) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0018 (0x000030) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0019 (0x000032) 0x291D-       f:00024 d: 285 | OR[285] = A                   
0x001A (0x000034) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x001B (0x000036) 0xCE0F-       f:00147 d:  15 | io 0017 (EXB), fn007           | Set interrupt mode
0x001C (0x000038) 0x211D-       f:00020 d: 285 | A = OR[285]                   
0x001D (0x00003A) 0xCA0F-       f:00145 d:  15 | io 0017 (EXB), fn005           | Load device address
@ Entry 17 contains, high byte: is there an async operation in progress? If yes, suspend until interrupted by tape drive
0x001E (0x00003C) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x001F (0x00003E) 0x1411-       f:00012 d:  17 | A = A + 17 (0x0011)           
0x0020 (0x000040) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0021 (0x000042) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0022 (0x000044) 0x0808-       f:00004 d:   8 | A = A > 8 (0x0008)            
0x0023 (0x000046) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0025), A # 0     
0x0024 (0x000048) 0x7002-       f:00070 d:   2 | P = P + 2 (0x0026)            
0x0025 (0x00004A) 0x7526-       f:00072 d: 294 | R = P + 294 (0x014B)          @ Wait for device interrupt or time out
@ Load entry 19 into OR[286], then clear it
0x0026 (0x00004C) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0027 (0x00004E) 0x1413-       f:00012 d:  19 | A = A + 19 (0x0013)           
0x0028 (0x000050) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0029 (0x000052) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x002A (0x000054) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x002B (0x000056) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x002C (0x000058) 0x1413-       f:00012 d:  19 | A = A + 19 (0x0013)           
0x002D (0x00005A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x002E (0x00005C) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x002F (0x00005E) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
@ Here we seem to parse the command code, comming in in OR[281]
@ Function code 1 and 2 - search file #
0x0030 (0x000060) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x0031 (0x000062) 0x1601-       f:00013 d:   1 | A = A - 1 (0x0001)            
0x0032 (0x000064) 0x8405-       f:00102 d:   5 | P = P + 5 (0x0037), A = 0     
0x0033 (0x000066) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x0034 (0x000068) 0x1602-       f:00013 d:   2 | A = A - 2 (0x0002)            
0x0035 (0x00006A) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0037), A = 0     
0x0036 (0x00006C) 0x7005-       f:00070 d:   5 | P = P + 5 (0x003B)            
0x0037 (0x00006E) 0x1008-       f:00010 d:   8 | A = 8 (0x0008)                
0x0038 (0x000070) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x0039 (0x000072) 0x7558-       f:00072 d: 344 | R = P + 344 (0x0191)          
0x003A (0x000074) 0x701F-       f:00070 d:  31 | P = P + 31 (0x0059)           
@ Function code 5 - close file
0x003B (0x000076) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x003C (0x000078) 0x1605-       f:00013 d:   5 | A = A - 5 (0x0005)            
0x003D (0x00007A) 0x8402-       f:00102 d:   2 | P = P + 2 (0x003F), A = 0     
0x003E (0x00007C) 0x7005-       f:00070 d:   5 | P = P + 5 (0x0043)            
0x003F (0x00007E) 0x1009-       f:00010 d:   9 | A = 9 (0x0009)                
0x0040 (0x000080) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x0041 (0x000082) 0x7550-       f:00072 d: 336 | R = P + 336 (0x0191)          
0x0042 (0x000084) 0x7017-       f:00070 d:  23 | P = P + 23 (0x0059)           
@ Function code 6 - close tape
0x0043 (0x000086) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x0044 (0x000088) 0x1606-       f:00013 d:   6 | A = A - 6 (0x0006)            
0x0045 (0x00008A) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0047), A = 0     
0x0046 (0x00008C) 0x7005-       f:00070 d:   5 | P = P + 5 (0x004B)            
0x0047 (0x00008E) 0x100A-       f:00010 d:  10 | A = 10 (0x000A)               
0x0048 (0x000090) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x0049 (0x000092) 0x7548-       f:00072 d: 328 | R = P + 328 (0x0191)          
0x004A (0x000094) 0x700F-       f:00070 d:  15 | P = P + 15 (0x0059)           
@ Function code 4 - read data from tape
0x004B (0x000096) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x004C (0x000098) 0x1604-       f:00013 d:   4 | A = A - 4 (0x0004)            
0x004D (0x00009A) 0x8402-       f:00102 d:   2 | P = P + 2 (0x004F), A = 0     
0x004E (0x00009C) 0x7003-       f:00070 d:   3 | P = P + 3 (0x0051)            
0x004F (0x00009E) 0x7417-       f:00072 d:  23 | R = P + 23 (0x0066)           
0x0050 (0x0000A0) 0x7009-       f:00070 d:   9 | P = P + 9 (0x0059)            
@ Function code 3 - write data to tape
0x0051 (0x0000A2) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x0052 (0x0000A4) 0x1603-       f:00013 d:   3 | A = A - 3 (0x0003)            
0x0053 (0x0000A6) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0055), A = 0     
0x0054 (0x0000A8) 0x7003-       f:00070 d:   3 | P = P + 3 (0x0057)            
0x0055 (0x0000AA) 0x745B-       f:00072 d:  91 | R = P + 91 (0x00B0)           
0x0056 (0x0000AC) 0x7003-       f:00070 d:   3 | P = P + 3 (0x0059)            
0x0057 (0x0000AE) 0x7C34-       f:00076 d:  52 | R = OR[52]                    
0x0058 (0x0000B0) 0x0000-       f:00000 d:   0 | PASS                          
@ This is what happens after the command is executed
0x0059 (0x0000B2) 0x1002-       f:00010 d:   2 | A = 2 (0x0002)                
0x005A (0x0000B4) 0xCE0F-       f:00147 d:  15 | io 0017 (EXB), fn007           | Set interrupt mode
@ Set return value: it's comming from OR[284]
0x005B (0x0000B6) 0x2005-       f:00020 d:   5 | A = OR[5]                     
0x005C (0x0000B8) 0x1406-       f:00012 d:   6 | A = A + 6 (0x0006)            
0x005D (0x0000BA) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x005E (0x0000BC) 0x211C-       f:00020 d: 284 | A = OR[284]                   
0x005F (0x0000BE) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
@ Return from overlay
0x0060 (0x0000C0) 0x102A-       f:00010 d:  42 | A = 42 (0x002A)               
0x0061 (0x0000C2) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x0062 (0x0000C4) 0x1127-       f:00010 d: 295 | A = 295 (0x0127)              
0x0063 (0x0000C6) 0x5800-       f:00054 d:   0 | B = A                         
0x0064 (0x0000C8) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0065 (0x0000CA) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Command code 4 handler - read data
@ Read lower byte of desc. entry 17. - interrupt count
0x0066 (0x0000CC) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0067 (0x0000CE) 0x1411-       f:00012 d:  17 | A = A + 17 (0x0011)           
0x0068 (0x0000D0) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0069 (0x0000D2) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x006A (0x0000D4) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x006B (0x0000D6) 0x8602-       f:00103 d:   2 | P = P + 2 (0x006D), A # 0     
0x006C (0x0000D8) 0x7038-       f:00070 d:  56 | P = P + 56 (0x00A4)           
@ Some interrupts received, since we've last checked
@ clear the interrupt counter
0x006D (0x0000DA) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x006E (0x0000DC) 0x1411-       f:00012 d:  17 | A = A + 17 (0x0011)           
0x006F (0x0000DE) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0070 (0x0000E0) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0071 (0x0000E2) 0x1A00-0xFF00 f:00015 d:   0 | A = A & 65280 (0xFF00)        
0x0073 (0x0000E6) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
@ Test MSB of entry 14 (RegA status), to see if there were any errors
0x0074 (0x0000E8) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0075 (0x0000EA) 0x140E-       f:00012 d:  14 | A = A + 14 (0x000E)           
0x0076 (0x0000EC) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0077 (0x0000EE) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0078 (0x0000F0) 0x1A00-0x8000 f:00015 d:   0 | A = A & 32768 (0x8000)        
0x007A (0x0000F4) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x007B (0x0000F6) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x007C (0x0000F8) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x007D (0x0000FA) 0x8602-       f:00103 d:   2 | P = P + 2 (0x007F), A # 0     
0x007E (0x0000FC) 0x7004-       f:00070 d:   4 | P = P + 4 (0x0082)            
@ Call XTAPEB and report the error
0x007F (0x0000FE) 0x1046-       f:00010 d:  70 | A = 70 (0x0046)               
0x0080 (0x000100) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x0081 (0x000102) 0x7514-       f:00072 d: 276 | R = P + 276 (0x0195)          
@ Were there any pending operations?
0x0082 (0x000104) 0x211E-       f:00020 d: 286 | A = OR[286]                   
0x0083 (0x000106) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0085), A = 0     
0x0084 (0x000108) 0x7020-       f:00070 d:  32 | P = P + 32 (0x00A4)           
@ Yes: get the results from the previous operation
@ Read RegB from desc, calculate buffer offset and store it in OR[290]
0x0085 (0x00010A) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0086 (0x00010C) 0x140F-       f:00012 d:  15 | A = A + 15 (0x000F)           
0x0087 (0x00010E) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0088 (0x000110) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0089 (0x000112) 0x2922-       f:00024 d: 290 | OR[290] = A                   
0x008A (0x000114) 0x2122-       f:00020 d: 290 | A = OR[290]                   
0x008B (0x000116) 0x1E00-0x26B0 f:00017 d:   0 | A = A - 9904 (0x26B0)         
0x008D (0x00011A) 0x2922-       f:00024 d: 290 | OR[290] = A                   
@ Store offset in entry 21
0x008E (0x00011C) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x008F (0x00011E) 0x1415-       f:00012 d:  21 | A = A + 21 (0x0015)           
0x0090 (0x000120) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0091 (0x000122) 0x2122-       f:00020 d: 290 | A = OR[290]                   
0x0092 (0x000124) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
@ Was there anything in the buffer?
0x0093 (0x000126) 0x2122-       f:00020 d: 290 | A = OR[290]                   
0x0094 (0x000128) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0096), A = 0     
0x0095 (0x00012A) 0x7009-       f:00070 d:   9 | P = P + 9 (0x009E)            
@ No, there was not: call XTAPEB with error code 68 - RECORD LENGTH ERROR
0x0096 (0x00012C) 0x1044-       f:00010 d:  68 | A = 68 (0x0044)               
0x0097 (0x00012E) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x0098 (0x000130) 0x74FD-       f:00072 d: 253 | R = P + 253 (0x0195)          
@ Yes, there was: load offset from desc. 21 - number of bytes in the buffer, then copy the (potentially partial) buffer out from the tape buffer into the caller buffer
0x0099 (0x000132) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x009A (0x000134) 0x1415-       f:00012 d:  21 | A = A + 21 (0x0015)           
0x009B (0x000136) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x009C (0x000138) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x009D (0x00013A) 0x2922-       f:00024 d: 290 | OR[290] = A                   
0x009E (0x00013C) 0x1800-0x26B0 f:00014 d:   0 | A = 9904 (0x26B0)             @ SRC buffer
0x00A0 (0x000140) 0x2920-       f:00024 d: 288 | OR[288] = A                   
0x00A1 (0x000142) 0x211A-       f:00020 d: 282 | A = OR[282]                   @ DST buffer
0x00A2 (0x000144) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x00A3 (0x000146) 0x7462-       f:00072 d:  98 | R = P + 98 (0x0105)           @ Buffer copy
@ Issue tape read command
@    OR[283]: buffer size
@    OR[280]: device desc pointer
@    OR[291]: tape command
0x00A4 (0x000148) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00A5 (0x00014A) 0x2923-       f:00024 d: 291 | OR[291] = A                   
0x00A6 (0x00014C) 0x746F-       f:00072 d: 111 | R = P + 111 (0x0115)          
@ If OR[286] was 1 set (no command pending) it to 0, call wait for device interrupt and redo the whole command again
0x00A7 (0x00014E) 0x211E-       f:00020 d: 286 | A = OR[286]                   
0x00A8 (0x000150) 0x1601-       f:00013 d:   1 | A = A - 1 (0x0001)            
0x00A9 (0x000152) 0x8402-       f:00102 d:   2 | P = P + 2 (0x00AB), A = 0     
0x00AA (0x000154) 0x7005-       f:00070 d:   5 | P = P + 5 (0x00AF)            
0x00AB (0x000156) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00AC (0x000158) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x00AD (0x00015A) 0x749E-       f:00072 d: 158 | R = P + 158 (0x014B)          @ Wait for device interrupt or time out
0x00AE (0x00015C) 0x7248-       f:00071 d:  72 | P = P - 72 (0x0066)           
0x00AF (0x00015E) 0x0200-       f:00001 d:   0 | EXIT                          


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Command code 3 handler - write data ????
@ Make sure buffer-size is within limits, that is less than or equal to 2048
0x00B0 (0x000160) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x00B1 (0x000162) 0x1E00-0x0800 f:00017 d:   0 | A = A - 2048 (0x0800)         
0x00B3 (0x000166) 0x8003-       f:00100 d:   3 | P = P + 3 (0x00B6), C = 0     
0x00B4 (0x000168) 0x8402-       f:00102 d:   2 | P = P + 2 (0x00B6), A = 0     
0x00B5 (0x00016A) 0x7C34-       f:00076 d:  52 | R = OR[52]                    
0x00B6 (0x00016C) 0x0000-       f:00000 d:   0 | PASS                          
@ Test retry counter
0x00B7 (0x00016E) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x00B8 (0x000170) 0x1411-       f:00012 d:  17 | A = A + 17 (0x0011)           
0x00B9 (0x000172) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00BA (0x000174) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x00BB (0x000176) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x00BC (0x000178) 0x8602-       f:00103 d:   2 | P = P + 2 (0x00BE), A # 0     
0x00BD (0x00017A) 0x702F-       f:00070 d:  47 | P = P + 47 (0x00EC)           
@ Non-zero retry counter, zero it
0x00BE (0x00017C) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x00BF (0x00017E) 0x1411-       f:00012 d:  17 | A = A + 17 (0x0011)           
0x00C0 (0x000180) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00C1 (0x000182) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x00C2 (0x000184) 0x1A00-0xFF00 f:00015 d:   0 | A = A & 65280 (0xFF00)        
0x00C4 (0x000188) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 

0x00C5 (0x00018A) 0x211E-       f:00020 d: 286 | A = OR[286]                   
0x00C6 (0x00018C) 0x8402-       f:00102 d:   2 | P = P + 2 (0x00C8), A = 0     
0x00C7 (0x00018E) 0x7025-       f:00070 d:  37 | P = P + 37 (0x00EC)           
@ Read RegA from desc, see if there's an error to report (MSB set), if yes, call XTAPEB, function 70 to parse and report the error
0x00C8 (0x000190) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x00C9 (0x000192) 0x140E-       f:00012 d:  14 | A = A + 14 (0x000E)           
0x00CA (0x000194) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00CB (0x000196) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x00CC (0x000198) 0x1A00-0x8000 f:00015 d:   0 | A = A & 32768 (0x8000)        
0x00CE (0x00019C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00CF (0x00019E) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00D0 (0x0001A0) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x00D1 (0x0001A2) 0x8602-       f:00103 d:   2 | P = P + 2 (0x00D3), A # 0     
0x00D2 (0x0001A4) 0x7005-       f:00070 d:   5 | P = P + 5 (0x00D7)            
0x00D3 (0x0001A6) 0x1046-       f:00010 d:  70 | A = 70 (0x0046)               
0x00D4 (0x0001A8) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x00D5 (0x0001AA) 0x74C0-       f:00072 d: 192 | R = P + 192 (0x0195)          
0x00D6 (0x0001AC) 0x7016-       f:00070 d:  22 | P = P + 22 (0x00EC)           
@ Device error to report, let's calculate the buffer offset and store it in OR[292]
0x00D7 (0x0001AE) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x00D8 (0x0001B0) 0x140F-       f:00012 d:  15 | A = A + 15 (0x000F)           
0x00D9 (0x0001B2) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00DA (0x0001B4) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x00DB (0x0001B6) 0x2924-       f:00024 d: 292 | OR[292] = A                   
0x00DC (0x0001B8) 0x2124-       f:00020 d: 292 | A = OR[292]                   
0x00DD (0x0001BA) 0x1E00-0x26B0 f:00017 d:   0 | A = A - 9904 (0x26B0)         
0x00DF (0x0001BE) 0x2924-       f:00024 d: 292 | OR[292] = A                   
@ See if all the data have been written (that is the offset reported by the HW in RegB is equal to the buffer-length in desc. offset 22)
0x00E0 (0x0001C0) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x00E1 (0x0001C2) 0x1416-       f:00012 d:  22 | A = A + 22 (0x0016)           
0x00E2 (0x0001C4) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00E3 (0x0001C6) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x00E4 (0x0001C8) 0x2925-       f:00024 d: 293 | OR[293] = A                   
0x00E5 (0x0001CA) 0x2124-       f:00020 d: 292 | A = OR[292]                   
0x00E6 (0x0001CC) 0x2725-       f:00023 d: 293 | A = A - OR[293]               
0x00E7 (0x0001CE) 0x8602-       f:00103 d:   2 | P = P + 2 (0x00E9), A # 0     
0x00E8 (0x0001D0) 0x7004-       f:00070 d:   4 | P = P + 4 (0x00EC)            
@ Partial buffer was written, report it through XTAPEB
0x00E9 (0x0001D2) 0x1044-       f:00010 d:  68 | A = 68 (0x0044)               
0x00EA (0x0001D4) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x00EB (0x0001D6) 0x74AA-       f:00072 d: 170 | R = P + 170 (0x0195)          
@ Copy the data into the internal buffer and issue the write operation on the tape
0x00EC (0x0001D8) 0x211A-       f:00020 d: 282 | A = OR[282]                   
0x00ED (0x0001DA) 0x2920-       f:00024 d: 288 | OR[288] = A                   
0x00EE (0x0001DC) 0x1800-0x26B0 f:00014 d:   0 | A = 9904 (0x26B0)             
0x00F0 (0x0001E0) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x00F1 (0x0001E2) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x00F2 (0x0001E4) 0x2922-       f:00024 d: 290 | OR[290] = A                   
0x00F3 (0x0001E6) 0x7412-       f:00072 d:  18 | R = P + 18 (0x0105)           
0x00F4 (0x0001E8) 0x1028-       f:00010 d:  40 | A = 40 (0x0028)               
0x00F5 (0x0001EA) 0x2923-       f:00024 d: 291 | OR[291] = A                   
0x00F6 (0x0001EC) 0x741F-       f:00072 d:  31 | R = P + 31 (0x0115)           
0x00F7 (0x0001EE) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Make sure tape device is ready (that is LSB of RegA is set)
0x00F8 (0x0001F0) 0xC20F-       f:00141 d:  15 | io 0017 (EXB), fn001           | Data input from A register (DIA)
0x00F9 (0x0001F2) 0x7446-       f:00072 d:  70 | R = P + 70 (0x013F)           
0x00FA (0x0001F4) 0xD00F-       f:00150 d:  15 | io 0017 (EXB), fn010           | Read data bus status
0x00FB (0x0001F6) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x00FC (0x0001F8) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00FD (0x0001FA) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00FE (0x0001FC) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x00FF (0x0001FE) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0101), A = 0     
0x0100 (0x000200) 0x7004-       f:00070 d:   4 | P = P + 4 (0x0104)            
0x0101 (0x000202) 0x1045-       f:00010 d:  69 | A = 69 (0x0045)               
0x0102 (0x000204) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x0103 (0x000206) 0x7492-       f:00072 d: 146 | R = P + 146 (0x0195)          
0x0104 (0x000208) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Buffer copy
@@ Input:
@@   OR[288]: source buffer address
@@   OR[289]: destination buffer address
@@   OR[290]: number of words to copy
0x0105 (0x00020A) 0x2120-       f:00020 d: 288 | A = OR[288]                   
0x0106 (0x00020C) 0x2924-       f:00024 d: 292 | OR[292] = A                   
0x0107 (0x00020E) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x0108 (0x000210) 0x2925-       f:00024 d: 293 | OR[293] = A                   
0x0109 (0x000212) 0x2120-       f:00020 d: 288 | A = OR[288]                   
0x010A (0x000214) 0x2522-       f:00022 d: 290 | A = A + OR[290]               
0x010B (0x000216) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x010C (0x000218) 0x2124-       f:00020 d: 292 | A = OR[292]                   
0x010D (0x00021A) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x010E (0x00021C) 0x8406-       f:00102 d:   6 | P = P + 6 (0x0114), A = 0     
0x010F (0x00021E) 0x3124-       f:00030 d: 292 | A = (OR[292])                 
0x0110 (0x000220) 0x3925-       f:00034 d: 293 | (OR[293]) = A                 
0x0111 (0x000222) 0x2D24-       f:00026 d: 292 | OR[292] = OR[292] + 1         
0x0112 (0x000224) 0x2D25-       f:00026 d: 293 | OR[293] = OR[293] + 1         
0x0113 (0x000226) 0x720A-       f:00071 d:  10 | P = P - 10 (0x0109)           
0x0114 (0x000228) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Issue tape operation
@@ Input
@@    OR[283]: buffer size
@@    OR[280]: device desc pointer
@@    OR[291]: tape command
0x0115 (0x00022A) 0x761D-       f:00073 d:  29 | R = P - 29 (0x00F8)           @ Make sure tape device idles
0x0116 (0x00022C) 0x1800-0x26B0 f:00014 d:   0 | A = 9904 (0x26B0)             @ Set buffer address to 0x26b0
0x0118 (0x000230) 0xDA0F-       f:00155 d:  15 | io 15 (EXB), fn015             | Data output to B register (DOB)
0x0119 (0x000232) 0x7426-       f:00072 d:  38 | R = P + 38 (0x013F)           
@ Load the transfer size from OR[283] and store it in desc. entry 22
0x011A (0x000234) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x011B (0x000236) 0x271B-       f:00023 d: 283 | A = A - OR[283]               @ OR[283] contains buffer size (0x800) in our case
0x011C (0x000238) 0xDC0F-       f:00156 d:  15 | io 0017 (EXB), fn016           | Data output to C register (DOC)
0x011D (0x00023A) 0x2118-       f:00020 d: 280 | A = OR[280]                   @ Store buffer size in entry 22 from device desc.
0x011E (0x00023C) 0x1416-       f:00012 d:  22 | A = A + 22 (0x0016)           
0x011F (0x00023E) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0120 (0x000240) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x0121 (0x000242) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0122 (0x000244) 0x741D-       f:00072 d:  29 | R = P + 29 (0x013F)           
@ Load command from OR[291], set it on the device and store it in desc. entry 20, upper byte
0x0123 (0x000246) 0x2123-       f:00020 d: 291 | A = OR[291]                   
0x0124 (0x000248) 0xD80F-       f:00154 d:  15 | io 0017 (EXB), fn014           | Data output to A register (DOA)
0x0125 (0x00024A) 0x2123-       f:00020 d: 291 | A = OR[291]                   
0x0126 (0x00024C) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x0127 (0x00024E) 0x2923-       f:00024 d: 291 | OR[291] = A                   
0x0128 (0x000250) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0129 (0x000252) 0x1414-       f:00012 d:  20 | A = A + 20 (0x0014)           
0x012A (0x000254) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x012B (0x000256) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x012C (0x000258) 0x0A09-       f:00005 d:   9 | A = A < 9 (0x0009)            
0x012D (0x00025A) 0x2523-       f:00022 d: 291 | A = A + OR[291]               
0x012E (0x00025C) 0x0C09-       f:00006 d:   9 | A = A >> 9 (0x0009)           
0x012F (0x00025E) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0130 (0x000260) 0x740F-       f:00072 d:  15 | R = P + 15 (0x013F)           
@ Send START command
0x0131 (0x000262) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x0132 (0x000264) 0xDE0F-       f:00157 d:  15 | io 0017 (EXB), fn017           | Send control   @@@@@@ 0x5dd4
0x0133 (0x000266) 0x740C-       f:00072 d:  12 | R = P + 12 (0x013F)           
@ Enable device interrupts
0x0134 (0x000268) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0135 (0x00026A) 0xCC0F-       f:00146 d:  15 | io 0017 (EXB), fn006           | Send interface mask (MSKO)
0x0136 (0x00026C) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0137 (0x00026E) 0x1411-       f:00012 d:  17 | A = A + 17 (0x0011)           
0x0138 (0x000270) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0139 (0x000272) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x013A (0x000274) 0x0A09-       f:00005 d:   9 | A = A < 9 (0x0009)            
0x013B (0x000276) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x013C (0x000278) 0x0C09-       f:00006 d:   9 | A = A >> 9 (0x0009)           
0x013D (0x00027A) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x013E (0x00027C) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Wait for delayed operation to complete
0x013F (0x00027E) 0x1190-       f:00010 d: 400 | A = 400 (0x0190)              
0x0140 (0x000280) 0x8405-       f:00102 d:   5 | P = P + 5 (0x0145), A = 0     
0x0141 (0x000282) 0x420F-       f:00041 d:  15 | C = 1, io 0017 (EXB) = BZ     
0x0142 (0x000284) 0x8003-       f:00100 d:   3 | P = P + 3 (0x0145), C = 0     
0x0143 (0x000286) 0x1601-       f:00013 d:   1 | A = A - 1 (0x0001)            
0x0144 (0x000288) 0x7204-       f:00071 d:   4 | P = P - 4 (0x0140)            
0x0145 (0x00028A) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0147), A = 0     
0x0146 (0x00028C) 0x7004-       f:00070 d:   4 | P = P + 4 (0x014A)            
0x0147 (0x00028E) 0x1041-       f:00010 d:  65 | A = 65 (0x0041)               
0x0148 (0x000290) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x0149 (0x000292) 0x744C-       f:00072 d:  76 | R = P + 76 (0x0195)           
0x014A (0x000294) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Wait for device interrupt or time out
@ Disable interrupts
0x014B (0x000296) 0x0400-       f:00002 d:   0 | I = 0                         
0x014C (0x000298) 0x0000-       f:00000 d:   0 | PASS                          
@ Enable EXB controller interrupts
0x014D (0x00029A) 0x1002-       f:00010 d:   2 | A = 2 (0x0002)                
0x014E (0x00029C) 0xCE0F-       f:00147 d:  15 | io 0017 (EXB), fn007           | Set interrupt mode
@ Kernel call 9: Suspend current context until timeout (200ms ?) or interrupt on this device
0x014F (0x00029E) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0150 (0x0002A0) 0x1409-       f:00012 d:   9 | A = A + 9 (0x0009)            
0x0151 (0x0002A2) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x0152 (0x0002A4) 0x1009-       f:00010 d:   9 | A = 9 (0x0009)                
0x0153 (0x0002A6) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x0154 (0x0002A8) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x0155 (0x0002AA) 0x2928-       f:00024 d: 296 | OR[296] = A                   
0x0156 (0x0002AC) 0x10C8-       f:00010 d: 200 | A = 200 (0x00C8)              
0x0157 (0x0002AE) 0x2929-       f:00024 d: 297 | OR[297] = A                   
0x0158 (0x0002B0) 0x1127-       f:00010 d: 295 | A = 295 (0x0127)              
0x0159 (0x0002B2) 0x5800-       f:00054 d:   0 | B = A                         
0x015A (0x0002B4) 0x1800-0x1F18 f:00014 d:   0 | A = 7960 (0x1F18)             
0x015C (0x0002B8) 0x7C09-       f:00076 d:   9 | R = OR[9]                     
@ Load OR[275] from current context - the queue this context belongs to
0x015D (0x0002BA) 0x2006-       f:00020 d:   6 | A = OR[6]                     
0x015E (0x0002BC) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x015F (0x0002BE) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0160 (0x0002C0) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0161 (0x0002C2) 0x2913-       f:00024 d: 275 | OR[275] = A                   
@ Disable all EXB interrupts
0x0162 (0x0002C4) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0163 (0x0002C6) 0xCE0F-       f:00147 d:  15 | io 0017 (EXB), fn007           | Set interrupt mode
@ Reload device address
0x0164 (0x0002C8) 0x211D-       f:00020 d: 285 | A = OR[285]                   
0x0165 (0x0002CA) 0xCA0F-       f:00145 d:  15 | io 0017 (EXB), fn005           | Load device address
0x0166 (0x0002CC) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x0167 (0x0002CE) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0169), A # 0     
0x0168 (0x0002D0) 0x700D-       f:00070 d:  13 | P = P + 13 (0x0175)           
@ Since the interrupt handler clears entry 11, if it's non-0, it must be a timeout
@ In that case clear the upper byte of entry 17
0x0169 (0x0002D2) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x016A (0x0002D4) 0x1411-       f:00012 d:  17 | A = A + 17 (0x0011)           
0x016B (0x0002D6) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x016C (0x0002D8) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x016D (0x0002DA) 0x0E01-       f:00007 d:   1 | A = A << 1 (0x0001)           
0x016E (0x0002DC) 0x0A08-       f:00005 d:   8 | A = A < 8 (0x0008)            
0x016F (0x0002DE) 0x1400-       f:00012 d:   0 | A = A + 0 (0x0000)            
0x0170 (0x0002E0) 0x0C09-       f:00006 d:   9 | A = A >> 9 (0x0009)           
0x0171 (0x0002E2) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
@ And call XTABE function 66: DEVICE TIMEOUT
0x0172 (0x0002E4) 0x1042-       f:00010 d:  66 | A = 66 (0x0042)               
0x0173 (0x0002E6) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x0174 (0x0002E8) 0x7421-       f:00072 d:  33 | R = P + 33 (0x0195)           
0x0175 (0x0002EA) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ This routine reads the high-byte of entry 11 into OR[284], clears it in memory and does some checks on it.
@ Get lower high-byte of entry 11 into OR[284]
0x0176 (0x0002EC) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0177 (0x0002EE) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x0178 (0x0002F0) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0179 (0x0002F2) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x017A (0x0002F4) 0x0808-       f:00004 d:   8 | A = A > 8 (0x0008)            
0x017B (0x0002F6) 0x291C-       f:00024 d: 284 | OR[284] = A                   
@ Clear the high-byte of entry 11
0x017C (0x0002F8) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x017D (0x0002FA) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x017E (0x0002FC) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x017F (0x0002FE) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0180 (0x000300) 0x0E01-       f:00007 d:   1 | A = A << 1 (0x0001)           
0x0181 (0x000302) 0x0A08-       f:00005 d:   8 | A = A < 8 (0x0008)            
0x0182 (0x000304) 0x1400-       f:00012 d:   0 | A = A + 0 (0x0000)            
0x0183 (0x000306) 0x0C09-       f:00006 d:   9 | A = A >> 9 (0x0009)           
0x0184 (0x000308) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
@ If high-byte is 4, let's 0 it in OR[284] and return.
@ If it non-0, freak out and return from overlay
@ If it's 0, simply return
0x0185 (0x00030A) 0x211C-       f:00020 d: 284 | A = OR[284]                   
0x0186 (0x00030C) 0x1604-       f:00013 d:   4 | A = A - 4 (0x0004)            
0x0187 (0x00030E) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0189), A = 0     
0x0188 (0x000310) 0x7004-       f:00070 d:   4 | P = P + 4 (0x018C)            
0x0189 (0x000312) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x018A (0x000314) 0x291C-       f:00024 d: 284 | OR[284] = A                   
0x018B (0x000316) 0x7005-       f:00070 d:   5 | P = P + 5 (0x0190)            
0x018C (0x000318) 0x211C-       f:00020 d: 284 | A = OR[284]                   
0x018D (0x00031A) 0x8602-       f:00103 d:   2 | P = P + 2 (0x018F), A # 0     
0x018E (0x00031C) 0x7002-       f:00070 d:   2 | P = P + 2 (0x0190)            
0x018F (0x00031E) 0x7336-       f:00071 d: 310 | P = P - 310 (0x0059)          
0x0190 (0x000320) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Actual handler for command 0, 1, 5, 6 (will call XTAPEA)
@@ OR[287] 8 for command 0 and 1
@@         9 for command 5
@@        10 for command 6
0x0191 (0x000322) 0x1800-0x00AE f:00014 d:   0 | A = 174 (0x00AE)              
0x0193 (0x000326) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x0194 (0x000328) 0x7005-       f:00070 d:   5 | P = P + 5 (0x0199)            

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Here we'll call XTAPEB
@@ OR[287] - error code
0x0195 (0x00032A) 0x1800-0x00AF f:00014 d:   0 | A = 175 (0x00AF)              
0x0197 (0x00032E) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x0198 (0x000330) 0x7001-       f:00070 d:   1 | P = P + 1 (0x0199)            

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Common handler: calls XTAPEA or XTAPEB
0x0199 (0x000332) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x019A (0x000334) 0x1414-       f:00012 d:  20 | A = A + 20 (0x0014)           
0x019B (0x000336) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x019C (0x000338) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x019D (0x00033A) 0x1A00-0xFF00 f:00015 d:   0 | A = A & 65280 (0xFF00)        
0x019F (0x00033E) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x01A0 (0x000340) 0x1028-       f:00010 d:  40 | A = 40 (0x0028)               
0x01A1 (0x000342) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x01A2 (0x000344) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x01A3 (0x000346) 0x2928-       f:00024 d: 296 | OR[296] = A                   
0x01A4 (0x000348) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x01A5 (0x00034A) 0x2929-       f:00024 d: 297 | OR[297] = A                   
0x01A6 (0x00034C) 0x211F-       f:00020 d: 287 | A = OR[287]                   
0x01A7 (0x00034E) 0x292A-       f:00024 d: 298 | OR[298] = A                   
0x01A8 (0x000350) 0x211A-       f:00020 d: 282 | A = OR[282]                   
0x01A9 (0x000352) 0x292B-       f:00024 d: 299 | OR[299] = A                   
0x01AA (0x000354) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x01AB (0x000356) 0x292C-       f:00024 d: 300 | OR[300] = A                   
0x01AC (0x000358) 0x1127-       f:00010 d: 295 | A = 295 (0x0127)              
0x01AD (0x00035A) 0x5800-       f:00054 d:   0 | B = A                         
0x01AE (0x00035C) 0x1800-0x1F18 f:00014 d:   0 | A = 7960 (0x1F18)             
0x01B0 (0x000360) 0x7C09-       f:00076 d:   9 | R = OR[9]                     
@ Test return value: if non-0, return from overlay - with that return value, otherwise enable interrupts, and return
0x01B1 (0x000362) 0x291C-       f:00024 d: 284 | OR[284] = A                   
0x01B2 (0x000364) 0x211C-       f:00020 d: 284 | A = OR[284]                   
0x01B3 (0x000366) 0x8602-       f:00103 d:   2 | P = P + 2 (0x01B5), A # 0     
0x01B4 (0x000368) 0x7002-       f:00070 d:   2 | P = P + 2 (0x01B6)            
0x01B5 (0x00036A) 0x735C-       f:00071 d: 348 | P = P - 348 (0x0059)          
0x01B6 (0x00036C) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x01B7 (0x00036E) 0xCE0F-       f:00147 d:  15 | io 0017 (EXB), fn007           | Set interrupt mode
0x01B8 (0x000370) 0x211D-       f:00020 d: 285 | A = OR[285]                   
0x01B9 (0x000372) 0xCA0F-       f:00145 d:  15 | io 0017 (EXB), fn005           | Load device address
0x01BA (0x000374) 0x0200-       f:00001 d:   0 | EXIT                          
0x01BB (0x000376) 0x0000-       f:00000 d:   0 | PASS                          
0x01BC (0x000378) 0x0000-       f:00000 d:   0 | PASS                          
0x01BD (0x00037A) 0x0000-       f:00000 d:   0 | PASS                          
0x01BE (0x00037C) 0x0000-       f:00000 d:   0 | PASS                          
0x01BF (0x00037E) 0x0000-       f:00000 d:   0 | PASS                          
