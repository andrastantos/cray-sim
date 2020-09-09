@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Input:
@@    OR[280]: pointer to device desc. structure. layout:
@@               0, 1: string of the device name (MT0)
@@               2: device address (022)
@@               3: device driver offset number
@@               9: ??? address passed on to call to 0x1d57
@@               14: RegA
@@               15: RegB
@@               16: RegC
@@               17: Lower byte: interrupt counter, higher byte???
@@               20: upper byte: last RegA command issued , lower byte???
@@               22: number of bytes to transfer in a read/write operation
@@    OR[281]: some sort of a command code
@@               4096 - load tape
@@               4097 - unload tape?
@@ Return value:
@@	0 - OK
@@	7 - load failed? EOF file markers were not found
@@	8 - ???
@@ Loaded at 0x559E (0x5598)
@ OR[282] contains the return value?
0x0000 (0x000000) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0001 (0x000002) 0x291A-       f:00024 d: 282 | OR[282] = A                   
@ if OR[283] is set 1, error returns are called '7' otherwise '8'
0x0002 (0x000004) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0003 (0x000006) 0x291B-       f:00024 d: 283 | OR[283] = A                   
@ OR[284]: device address
0x0004 (0x000008) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0005 (0x00000A) 0x1402-       f:00012 d:   2 | A = A + 2 (0x0002)            
0x0006 (0x00000C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0007 (0x00000E) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0008 (0x000010) 0x291C-       f:00024 d: 284 | OR[284] = A                   
@ OR[288]: temp storage for desc. offset 22 value: number of bytes transferred
0x0009 (0x000012) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x000A (0x000014) 0x1416-       f:00012 d:  22 | A = A + 22 (0x0016)           
0x000B (0x000016) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x000C (0x000018) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x000D (0x00001A) 0x2920-       f:00024 d: 288 | OR[288] = A                   
@ Function 4096 handler
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
0x000E (0x00001C) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x000F (0x00001E) 0x1E00-0x1000 f:00017 d:   0 | A = A - 4096 (0x1000)         
0x0011 (0x000022) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0013), A = 0     
0x0012 (0x000024) 0x707E-       f:00070 d: 126 | P = P + 126 (0x0090)          
@ Store 2048 in desc. offset 22 (number of bytes transferred)
0x0013 (0x000026) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0014 (0x000028) 0x1416-       f:00012 d:  22 | A = A + 22 (0x0016)           
0x0015 (0x00002A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0016 (0x00002C) 0x1800-0x0800 f:00014 d:   0 | A = 2048 (0x0800)             
0x0018 (0x000030) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
@ Change error-code handling
0x0019 (0x000032) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x001A (0x000034) 0x291B-       f:00024 d: 283 | OR[283] = A                   
@ Call XTAPEC function 514 - read buffer from tape
0x001B (0x000036) 0x1800-0x0202 f:00014 d:   0 | A = 514 (0x0202)              
0x001D (0x00003A) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x001E (0x00003C) 0x754D-       f:00072 d: 333 | R = P + 333 (0x016B)          
@ Change error-code handling back
0x001F (0x00003E) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0020 (0x000040) 0x291B-       f:00024 d: 283 | OR[283] = A                   
@ Check returned RegB value (Buffer pointer), calculate the number of bytes transferred and store it in OR[275]
0x0021 (0x000042) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0022 (0x000044) 0x140F-       f:00012 d:  15 | A = A + 15 (0x000F)           
0x0023 (0x000046) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0024 (0x000048) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0025 (0x00004A) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x0026 (0x00004C) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x0027 (0x00004E) 0x1E00-0x26B0 f:00017 d:   0 | A = A - 9904 (0x26B0)         
0x0029 (0x000052) 0x2913-       f:00024 d: 275 | OR[275] = A                   
@ Read first three words of buffer and store it in OR[285] OR[286] and OR[287]
0x002A (0x000054) 0x1800-0x26B0 f:00014 d:   0 | A = 9904 (0x26B0)             
0x002C (0x000058) 0x2914-       f:00024 d: 276 | OR[276] = A                   
0x002D (0x00005A) 0x3114-       f:00030 d: 276 | A = (OR[276])                 
0x002E (0x00005C) 0x291D-       f:00024 d: 285 | OR[285] = A                   
0x002F (0x00005E) 0x2114-       f:00020 d: 276 | A = OR[276]                   
0x0030 (0x000060) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x0031 (0x000062) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0032 (0x000064) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0033 (0x000066) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x0034 (0x000068) 0x2114-       f:00020 d: 276 | A = OR[276]                   
0x0035 (0x00006A) 0x1402-       f:00012 d:   2 | A = A + 2 (0x0002)            
0x0036 (0x00006C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0037 (0x00006E) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0038 (0x000070) 0x291F-       f:00024 d: 287 | OR[287] = A                   

0x0039 (0x000072) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x003A (0x000074) 0x1603-       f:00013 d:   3 | A = A - 3 (0x0003)            
0x003B (0x000076) 0x8606-       f:00103 d:   6 | P = P + 6 (0x0041), A # 0     
0x003C (0x000078) 0x211D-       f:00020 d: 285 | A = OR[285]                   
0x003D (0x00007A) 0x1E00-0x404C f:00017 d:   0 | A = A - 16460 (0x404C)        
0x003F (0x00007E) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0041), A # 0     
0x0040 (0x000080) 0x7011-       f:00070 d:  17 | P = P + 17 (0x0051)           
@ Something wrong: either we didn't get exactly three WORDS from the buffer, or the first value wasn't 0x404C (@L)
@ Issue XTAPEC command 512 - space backwards one word
0x0041 (0x000082) 0x1800-0x0200 f:00014 d:   0 | A = 512 (0x0200)              
0x0043 (0x000086) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x0044 (0x000088) 0x7527-       f:00072 d: 295 | R = P + 295 (0x016B)          
@ Set return-code to 7
0x0045 (0x00008A) 0x1007-       f:00010 d:   7 | A = 7 (0x0007)                
0x0046 (0x00008C) 0x291A-       f:00024 d: 282 | OR[282] = A                   
@ Increment desc. entry 18 - the current file #
0x0047 (0x00008E) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0048 (0x000090) 0x1412-       f:00012 d:  18 | A = A + 18 (0x0012)           
0x0049 (0x000092) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x004A (0x000094) 0x3D08-       f:00036 d: 264 | (OR[264]) = (OR[264]) + 1     
@ Write 1 to entry 19 - no pending reads
0x004B (0x000096) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x004C (0x000098) 0x1413-       f:00012 d:  19 | A = A + 19 (0x0013)           
0x004D (0x00009A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x004E (0x00009C) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x004F (0x00009E) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
@ And get out of the overlay...
0x0050 (0x0000A0) 0x707D-       f:00070 d: 125 | P = P + 125 (0x00CD)          
@ End of error handling, which means we get here, with us having read three words and the buffer containing 0x404C at the beginning
@ This loop keeps reading until END_OF_FILE is set
0x0051 (0x0000A2) 0x748C-       f:00072 d: 140 | R = P + 140 (0x00DD)          @ Load next volume
@ Call read again
0x0052 (0x0000A4) 0x1800-0x0202 f:00014 d:   0 | A = 514 (0x0202)              
0x0054 (0x0000A8) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x0055 (0x0000AA) 0x7516-       f:00072 d: 278 | R = P + 278 (0x016B)          
@ Check RegA returned from desc. for END_OF_FILE bit
0x0056 (0x0000AC) 0x2122-       f:00020 d: 290 | A = OR[290]                   
0x0057 (0x0000AE) 0x1300-       f:00011 d: 256 | A = A & 256 (0x0100)          
0x0058 (0x0000B0) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0059 (0x0000B2) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x005A (0x0000B4) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x005B (0x0000B6) 0x8402-       f:00102 d:   2 | P = P + 2 (0x005D), A = 0     
0x005C (0x0000B8) 0x7003-       f:00070 d:   3 | P = P + 3 (0x005F)            
0x005D (0x0000BA) 0x720C-       f:00071 d:  12 | P = P - 12 (0x0051)           
@ This never seems to execute???
0x005E (0x0000BC) 0x7031-       f:00070 d:  49 | P = P + 49 (0x008F)           
@ So END_OF_FILE is reached
@ Call another read
0x005F (0x0000BE) 0x1800-0x0202 f:00014 d:   0 | A = 514 (0x0202)              
0x0061 (0x0000C2) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x0062 (0x0000C4) 0x7509-       f:00072 d: 265 | R = P + 265 (0x016B)          
@ Read first three entries of buffer into OR[275], OR[276] and OR[277]
0x0063 (0x0000C6) 0x1800-0x26B0 f:00014 d:   0 | A = 9904 (0x26B0)             
0x0065 (0x0000CA) 0x2916-       f:00024 d: 278 | OR[278] = A                   
0x0066 (0x0000CC) 0x3116-       f:00030 d: 278 | A = (OR[278])                 
0x0067 (0x0000CE) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x0068 (0x0000D0) 0x2116-       f:00020 d: 278 | A = OR[278]                   
0x0069 (0x0000D2) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x006A (0x0000D4) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x006B (0x0000D6) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x006C (0x0000D8) 0x2914-       f:00024 d: 276 | OR[276] = A                   
0x006D (0x0000DA) 0x2116-       f:00020 d: 278 | A = OR[278]                   
0x006E (0x0000DC) 0x1402-       f:00012 d:   2 | A = A + 2 (0x0002)            
0x006F (0x0000DE) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0070 (0x0000E0) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0071 (0x0000E2) 0x2915-       f:00024 d: 277 | OR[277] = A                   
@ Is the first WORD still the same as it was in the first test? (that is 0x404C)
0x0072 (0x0000E4) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x0073 (0x0000E6) 0x271D-       f:00023 d: 285 | A = A - OR[285]               
0x0074 (0x0000E8) 0x8605-       f:00103 d:   5 | P = P + 5 (0x0079), A # 0     
@ Does the second word match too?
0x0075 (0x0000EA) 0x2114-       f:00020 d: 276 | A = OR[276]                   
0x0076 (0x0000EC) 0x271E-       f:00023 d: 286 | A = A - OR[286]               
0x0077 (0x0000EE) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0079), A # 0     
0x0078 (0x0000F0) 0x7003-       f:00070 d:   3 | P = P + 3 (0x007B)            
@ No match, keep going in the loop
0x0079 (0x0000F2) 0x7228-       f:00071 d:  40 | P = P - 40 (0x0051)           
@ This never seems to execute???
0x007A (0x0000F4) 0x7006-       f:00070 d:   6 | P = P + 6 (0x0080)            
@ Both the first and the second WORD matches, check the third one
0x007B (0x0000F6) 0x2115-       f:00020 d: 277 | A = OR[277]                   
0x007C (0x0000F8) 0x271F-       f:00023 d: 287 | A = A - OR[287]               
0x007D (0x0000FA) 0x8602-       f:00103 d:   2 | P = P + 2 (0x007F), A # 0     
0x007E (0x0000FC) 0x7002-       f:00070 d:   2 | P = P + 2 (0x0080)            
@ No, keep looping
0x007F (0x0000FE) 0x722E-       f:00071 d:  46 | P = P - 46 (0x0051)           
@ Yes, all three words match, that means we're almost done somehow...
@ Issue yet another read
0x0080 (0x000100) 0x74EB-       f:00072 d: 235 | R = P + 235 (0x016B)          
@ Read entry 19 from the desc.
0x0081 (0x000102) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0082 (0x000104) 0x1413-       f:00012 d:  19 | A = A + 19 (0x0013)           
0x0083 (0x000106) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0084 (0x000108) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0085 (0x00010A) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0087), A = 0     
0x0086 (0x00010C) 0x7005-       f:00070 d:   5 | P = P + 5 (0x008B)            
@ If entry 19 is 0, issue another read
0x0087 (0x00010E) 0x1800-0x0202 f:00014 d:   0 | A = 514 (0x0202)              
0x0089 (0x000112) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x008A (0x000114) 0x7004-       f:00070 d:   4 | P = P + 4 (0x008E)            
@ If entry 19 is non-0, issue a command 512
0x008B (0x000116) 0x1800-0x0209 f:00014 d:   0 | A = 521 (0x0209)              
0x008D (0x00011A) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x008E (0x00011C) 0x74DD-       f:00072 d: 221 | R = P + 221 (0x016B)          
@ and we're done
0x008F (0x00011E) 0x703E-       f:00070 d:  62 | P = P + 62 (0x00CD)           

@ Function 4097 handler
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
0x0090 (0x000120) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x0091 (0x000122) 0x1E00-0x1001 f:00017 d:   0 | A = A - 4097 (0x1001)         
0x0093 (0x000126) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0095), A = 0     
0x0094 (0x000128) 0x7037-       f:00070 d:  55 | P = P + 55 (0x00CB)           
@ Issue function 519 on XTAPEC
0x0095 (0x00012A) 0x1800-0x0207 f:00014 d:   0 | A = 519 (0x0207)              
0x0097 (0x00012E) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x0098 (0x000130) 0x74D3-       f:00072 d: 211 | R = P + 211 (0x016B)          
@ Store the following in the tape buffer: 0x404C 0xFFFF, value of OR[67]
0x0099 (0x000132) 0x1800-0xFFFF f:00014 d:   0 | A = 65535 (0xFFFF)            
0x009B (0x000136) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x009C (0x000138) 0x2043-       f:00020 d:  67 | A = OR[67]                    
0x009D (0x00013A) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x009E (0x00013C) 0x1800-0x26B0 f:00014 d:   0 | A = 9904 (0x26B0)             
0x00A0 (0x000140) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x00A1 (0x000142) 0x1800-0x404C f:00014 d:   0 | A = 16460 (0x404C)            
0x00A3 (0x000146) 0x3913-       f:00034 d: 275 | (OR[275]) = A                 
0x00A4 (0x000148) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x00A5 (0x00014A) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x00A6 (0x00014C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00A7 (0x00014E) 0x211E-       f:00020 d: 286 | A = OR[286]                   
0x00A8 (0x000150) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x00A9 (0x000152) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x00AA (0x000154) 0x1402-       f:00012 d:   2 | A = A + 2 (0x0002)            
0x00AB (0x000156) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00AC (0x000158) 0x211F-       f:00020 d: 287 | A = OR[287]                   
0x00AD (0x00015A) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
@ Store 3 in desc. offset 22 (buffer length)
0x00AE (0x00015C) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x00AF (0x00015E) 0x1416-       f:00012 d:  22 | A = A + 22 (0x0016)           
0x00B0 (0x000160) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00B1 (0x000162) 0x1003-       f:00010 d:   3 | A = 3 (0x0003)                
0x00B2 (0x000164) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
@ Issue a write commands and a 519 command
0x00B3 (0x000166) 0x1800-0x0203 f:00014 d:   0 | A = 515 (0x0203)              
0x00B5 (0x00016A) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x00B6 (0x00016C) 0x74B5-       f:00072 d: 181 | R = P + 181 (0x016B)          
0x00B7 (0x00016E) 0x1800-0x0207 f:00014 d:   0 | A = 519 (0x0207)              
0x00B9 (0x000172) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x00BA (0x000174) 0x74B1-       f:00072 d: 177 | R = P + 177 (0x016B)          
0x00BB (0x000176) 0x74B0-       f:00072 d: 176 | R = P + 176 (0x016B)          
@ Load nex volume, do some user I/O and test for the presence of the write-ring
0x00BC (0x000178) 0x7421-       f:00072 d:  33 | R = P + 33 (0x00DD)           @ Load next volume
0x00BD (0x00017A) 0x747E-       f:00072 d: 126 | R = P + 126 (0x013B)          
@ Issue an 519 command to XTAPEC
0x00BE (0x00017C) 0x1800-0x0207 f:00014 d:   0 | A = 519 (0x0207)              
0x00C0 (0x000180) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x00C1 (0x000182) 0x74AA-       f:00072 d: 170 | R = P + 170 (0x016B)          
@ Followed by another write
0x00C2 (0x000184) 0x1800-0x0203 f:00014 d:   0 | A = 515 (0x0203)              
0x00C4 (0x000188) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x00C5 (0x00018A) 0x74A6-       f:00072 d: 166 | R = P + 166 (0x016B)          
@ And finally another 519 command
0x00C6 (0x00018C) 0x1800-0x0207 f:00014 d:   0 | A = 519 (0x0207)              
0x00C8 (0x000190) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x00C9 (0x000192) 0x74A2-       f:00072 d: 162 | R = P + 162 (0x016B)          
0x00CA (0x000194) 0x7003-       f:00070 d:   3 | P = P + 3 (0x00CD)            
@ Invalid function code -> fatal
0x00CB (0x000196) 0x7C34-       f:00076 d:  52 | R = OR[52]                    
0x00CC (0x000198) 0x0000-       f:00000 d:   0 | PASS                          
@ Common part of function handlers
@ Restore desc. offset 22 (number of bytes transferred) from OR[288]
0x00CD (0x00019A) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x00CE (0x00019C) 0x1416-       f:00012 d:  22 | A = A + 22 (0x0016)           
0x00CF (0x00019E) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00D0 (0x0001A0) 0x2120-       f:00020 d: 288 | A = OR[288]                   
0x00D1 (0x0001A2) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
@ Set return value (from OR[282])
0x00D2 (0x0001A4) 0x2005-       f:00020 d:   5 | A = OR[5]                     
0x00D3 (0x0001A6) 0x1406-       f:00012 d:   6 | A = A + 6 (0x0006)            
0x00D4 (0x0001A8) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00D5 (0x0001AA) 0x211A-       f:00020 d: 282 | A = OR[282]                   
0x00D6 (0x0001AC) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
@ Return from overlay
0x00D7 (0x0001AE) 0x102A-       f:00010 d:  42 | A = 42 (0x002A)               
0x00D8 (0x0001B0) 0x2924-       f:00024 d: 292 | OR[292] = A                   
0x00D9 (0x0001B2) 0x1124-       f:00010 d: 292 | A = 292 (0x0124)              
0x00DA (0x0001B4) 0x5800-       f:00054 d:   0 | B = A                         
0x00DB (0x0001B6) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00DC (0x0001B8) 0x7C09-       f:00076 d:   9 | R = OR[9]                     


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Load next volume
@ Call XTAPEC, function 516 (async rewind??)
0x00DD (0x0001BA) 0x1800-0x0204 f:00014 d:   0 | A = 516 (0x0204)              
0x00DF (0x0001BE) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x00E0 (0x0001C0) 0x748B-       f:00072 d: 139 | R = P + 139 (0x016B)          

0x00E1 (0x0001C2) 0x1002-       f:00010 d:   2 | A = 2 (0x0002)                
0x00E2 (0x0001C4) 0xCE0F-       f:00147 d:  15 | io 0017 (EXB), fn007           | Set interrupt mode
0x00E3 (0x0001C6) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x00E4 (0x0001C8) 0x1400-       f:00012 d:   0 | A = A + 0 (0x0000)            
0x00E5 (0x0001CA) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x00E6 (0x0001CC) 0x1028-       f:00010 d:  40 | A = 40 (0x0028)               @ Print: '@?S: PLEASE MOUNT NEXT VOLUME'
0x00E7 (0x0001CE) 0x2924-       f:00024 d: 292 | OR[292] = A                   
0x00E8 (0x0001D0) 0x1800-0x002F f:00014 d:   0 | A = 47 (0x002F)               
0x00EA (0x0001D4) 0x2925-       f:00024 d: 293 | OR[293] = A                   
0x00EB (0x0001D6) 0x1800-0x0283 f:00014 d:   0 | A = 643 (0x0283)              
0x00ED (0x0001DA) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x00EE (0x0001DC) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x00EF (0x0001DE) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x00F0 (0x0001E0) 0x1124-       f:00010 d: 292 | A = 292 (0x0124)              
0x00F1 (0x0001E2) 0x5800-       f:00054 d:   0 | B = A                         
0x00F2 (0x0001E4) 0x1800-0x1918 f:00014 d:   0 | A = 6424 (0x1918)             
0x00F4 (0x0001E8) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

0x00F5 (0x0001EA) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00F6 (0x0001EC) 0xCE0F-       f:00147 d:  15 | io 0017 (EXB), fn007           | Set interrupt mode
0x00F7 (0x0001EE) 0x211C-       f:00020 d: 284 | A = OR[284]                   
0x00F8 (0x0001F0) 0xCA0F-       f:00145 d:  15 | io 0017 (EXB), fn005           | Load device address
0x00F9 (0x0001F2) 0x7402-       f:00072 d:   2 | R = P + 2 (0x00FB)            
0x00FA (0x0001F4) 0x0200-       f:00001 d:   0 | EXIT                          

@ Ask the user: *** RESPOND *** [RESUME,ABORT,RESTART] ***
0x00FB (0x0001F6) 0x1002-       f:00010 d:   2 | A = 2 (0x0002)                
0x00FC (0x0001F8) 0xCE0F-       f:00147 d:  15 | io 0017 (EXB), fn007           | Set interrupt mode
@ Call MSGHND handler with message code 0x0201
0x00FD (0x0001FA) 0x1028-       f:00010 d:  40 | A = 40 (0x0028)               
0x00FE (0x0001FC) 0x2924-       f:00024 d: 292 | OR[292] = A                   
0x00FF (0x0001FE) 0x1800-0x002F f:00014 d:   0 | A = 47 (0x002F)               
0x0101 (0x000202) 0x2925-       f:00024 d: 293 | OR[293] = A                   
0x0102 (0x000204) 0x1800-0x0201 f:00014 d:   0 | A = 513 (0x0201)              
0x0104 (0x000208) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x0105 (0x00020A) 0x1124-       f:00010 d: 292 | A = 292 (0x0124)              
0x0106 (0x00020C) 0x5800-       f:00054 d:   0 | B = A                         
0x0107 (0x00020E) 0x1800-0x1918 f:00014 d:   0 | A = 6424 (0x1918)             
0x0109 (0x000212) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

0x010A (0x000214) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x010B (0x000216) 0xCE0F-       f:00147 d:  15 | io 0017 (EXB), fn007           | Set interrupt mode
0x010C (0x000218) 0x211C-       f:00020 d: 284 | A = OR[284]                   
0x010D (0x00021A) 0xCA0F-       f:00145 d:  15 | io 0017 (EXB), fn005           | Load device address
0x010E (0x00021C) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x010F (0x00021E) 0x291A-       f:00024 d: 282 | OR[282] = A                   
0x0110 (0x000220) 0x211A-       f:00020 d: 282 | A = OR[282]                   
0x0111 (0x000222) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x0112 (0x000224) 0x291A-       f:00024 d: 282 | OR[282] = A                   
@ Update stored return code in device desc. 11.
0x0113 (0x000226) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0114 (0x000228) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x0115 (0x00022A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0116 (0x00022C) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0117 (0x00022E) 0x0A09-       f:00005 d:   9 | A = A < 9 (0x0009)            
0x0118 (0x000230) 0x251A-       f:00022 d: 282 | A = A + OR[282]               
0x0119 (0x000232) 0x0C09-       f:00006 d:   9 | A = A >> 9 (0x0009)           
0x011A (0x000234) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 

0x011B (0x000236) 0x211A-       f:00020 d: 282 | A = OR[282]                   
0x011C (0x000238) 0x1601-       f:00013 d:   1 | A = A - 1 (0x0001)            
0x011D (0x00023A) 0x8609-       f:00103 d:   9 | P = P + 9 (0x0126), A # 0     
@ Read the stored return code from entry 11...
0x011E (0x00023C) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x011F (0x00023E) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x0120 (0x000240) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0121 (0x000242) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0122 (0x000244) 0x0808-       f:00004 d:   8 | A = A > 8 (0x0008)            
0x0123 (0x000246) 0x291A-       f:00024 d: 282 | OR[282] = A                   
0x0124 (0x000248) 0x7432-       f:00072 d:  50 | R = P + 50 (0x0156)           @ Wait on interrupt?
0x0125 (0x00024A) 0x720A-       f:00071 d:  10 | P = P - 10 (0x011B)           
@ Clear the stored error code in entry 11
0x0126 (0x00024C) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0127 (0x00024E) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x0128 (0x000250) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0129 (0x000252) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x012A (0x000254) 0x0E01-       f:00007 d:   1 | A = A << 1 (0x0001)           
0x012B (0x000256) 0x0A08-       f:00005 d:   8 | A = A < 8 (0x0008)            
0x012C (0x000258) 0x1400-       f:00012 d:   0 | A = A + 0 (0x0000)            
0x012D (0x00025A) 0x0C09-       f:00006 d:   9 | A = A >> 9 (0x0009)           
0x012E (0x00025C) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x012F (0x00025E) 0x211A-       f:00020 d: 282 | A = OR[282]                   
0x0130 (0x000260) 0x1604-       f:00013 d:   4 | A = A - 4 (0x0004)            
0x0131 (0x000262) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0133), A = 0     
0x0132 (0x000264) 0x7004-       f:00070 d:   4 | P = P + 4 (0x0136)            
0x0133 (0x000266) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0134 (0x000268) 0x291A-       f:00024 d: 282 | OR[282] = A                   
0x0135 (0x00026A) 0x7005-       f:00070 d:   5 | P = P + 5 (0x013A)            
0x0136 (0x00026C) 0x211A-       f:00020 d: 282 | A = OR[282]                   
0x0137 (0x00026E) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0139), A # 0     
0x0138 (0x000270) 0x7002-       f:00070 d:   2 | P = P + 2 (0x013A)            
0x0139 (0x000272) 0x726C-       f:00071 d: 108 | P = P - 108 (0x00CD)          
0x013A (0x000274) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Test presence of WRITE_WRING (write protect feature on the tape)
@ Returns, if write-enabled, otherwise report error: NO WRITE RING
0x013B (0x000276) 0xC20F-       f:00141 d:  15 | io 0017 (EXB), fn001           | Data input from A register (DIA)
0x013C (0x000278) 0x740E-       f:00072 d:  14 | R = P + 14 (0x014A)           
0x013D (0x00027A) 0xD00F-       f:00150 d:  15 | io 0017 (EXB), fn010           | Read data bus status
0x013E (0x00027C) 0x2922-       f:00024 d: 290 | OR[290] = A                   
0x013F (0x00027E) 0x2122-       f:00020 d: 290 | A = OR[290]                   
0x0140 (0x000280) 0x1204-       f:00011 d:   4 | A = A & 4 (0x0004)            
0x0141 (0x000282) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0142 (0x000284) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0143 (0x000286) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x0144 (0x000288) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0146), A # 0     
0x0145 (0x00028A) 0x7004-       f:00070 d:   4 | P = P + 4 (0x0149)            
0x0146 (0x00028C) 0x1043-       f:00010 d:  67 | A = 67 (0x0043)               
0x0147 (0x00028E) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x0148 (0x000290) 0x741E-       f:00072 d:  30 | R = P + 30 (0x0166)           
0x0149 (0x000292) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Wait for delayed operations
0x014A (0x000294) 0x1190-       f:00010 d: 400 | A = 400 (0x0190)              
0x014B (0x000296) 0x8405-       f:00102 d:   5 | P = P + 5 (0x0150), A = 0     
0x014C (0x000298) 0x420F-       f:00041 d:  15 | C = 1, io 0017 (EXB) = BZ       
0x014D (0x00029A) 0x8003-       f:00100 d:   3 | P = P + 3 (0x0150), C = 0     
0x014E (0x00029C) 0x1601-       f:00013 d:   1 | A = A - 1 (0x0001)            
0x014F (0x00029E) 0x7204-       f:00071 d:   4 | P = P - 4 (0x014B)            
0x0150 (0x0002A0) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0152), A = 0     
0x0151 (0x0002A2) 0x7004-       f:00070 d:   4 | P = P + 4 (0x0155)            
@ Error report through XTAPEB: CHANNEL_TIMEOUT
0x0152 (0x0002A4) 0x1041-       f:00010 d:  65 | A = 65 (0x0041)               
0x0153 (0x0002A6) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x0154 (0x0002A8) 0x7412-       f:00072 d:  18 | R = P + 18 (0x0166)           
0x0155 (0x0002AA) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ YIELD
0x0156 (0x0002AC) 0x1002-       f:00010 d:   2 | A = 2 (0x0002)                
0x0157 (0x0002AE) 0xCE0F-       f:00147 d:  15 | io 0017 (EXB), fn007           | Set interrupt mode
@ Kernel function PAUS - wait for 1ms (?)
0x0158 (0x0002B0) 0x1007-       f:00010 d:   7 | A = 7 (0x0007)                
0x0159 (0x0002B2) 0x2924-       f:00024 d: 292 | OR[292] = A                   
0x015A (0x0002B4) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x015B (0x0002B6) 0x2925-       f:00024 d: 293 | OR[293] = A                   
0x015C (0x0002B8) 0x1124-       f:00010 d: 292 | A = 292 (0x0124)              
0x015D (0x0002BA) 0x5800-       f:00054 d:   0 | B = A                         
0x015E (0x0002BC) 0x1800-0x1918 f:00014 d:   0 | A = 6424 (0x1918)             
0x0160 (0x0002C0) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

0x0161 (0x0002C2) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0162 (0x0002C4) 0xCE0F-       f:00147 d:  15 | io 0017 (EXB), fn007           | Set interrupt mode
0x0163 (0x0002C6) 0x211C-       f:00020 d: 284 | A = OR[284]                   
0x0164 (0x0002C8) 0xCA0F-       f:00145 d:  15 | io 0017 (EXB), fn005           | Load device address
0x0165 (0x0002CA) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Cal XTAPEB with function code in OR[289]
0x0166 (0x0002CC) 0x1800-0x00AF f:00014 d:   0 | A = 175 (0x00AF)              
0x0168 (0x0002D0) 0x2923-       f:00024 d: 291 | OR[291] = A                   
0x0169 (0x0002D2) 0x741D-       f:00072 d:  29 | R = P + 29 (0x0186)           
0x016A (0x0002D4) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Call XTAPEC with function code in OR[289]

@ Call XTAPEC
0x016B (0x0002D6) 0x1800-0x00B0 f:00014 d:   0 | A = 176 (0x00B0)              
0x016D (0x0002DA) 0x2923-       f:00024 d: 291 | OR[291] = A                   
0x016E (0x0002DC) 0x7418-       f:00072 d:  24 | R = P + 24 (0x0186)           
@ Check RegA status in device desc.
0x016F (0x0002DE) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0170 (0x0002E0) 0x140E-       f:00012 d:  14 | A = A + 14 (0x000E)           
0x0171 (0x0002E2) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0172 (0x0002E4) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0173 (0x0002E6) 0x2922-       f:00024 d: 290 | OR[290] = A                   
0x0174 (0x0002E8) 0x2122-       f:00020 d: 290 | A = OR[290]                   
@ Check status bits: DATA_LATE, 0x1000, DATA_PARITY_ERROR, BAD_TYPE and WORD_COUNT_ERROR
0x0175 (0x0002EA) 0x1A00-0x5422 f:00015 d:   0 | A = A & 21538 (0x5422)        
0x0177 (0x0002EE) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0178 (0x0002F0) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0179 (0x0002F2) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x017A (0x0002F4) 0x8602-       f:00103 d:   2 | P = P + 2 (0x017C), A # 0     
@ If none of them set, exit
0x017B (0x0002F6) 0x700A-       f:00070 d:  10 | P = P + 10 (0x0185)           
@ Some error bits are set, depending on OR[283], set return value in OR[282] to 7 or 8
0x017C (0x0002F8) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x017D (0x0002FA) 0x8602-       f:00103 d:   2 | P = P + 2 (0x017F), A # 0     
0x017E (0x0002FC) 0x7004-       f:00070 d:   4 | P = P + 4 (0x0182)            
0x017F (0x0002FE) 0x1007-       f:00010 d:   7 | A = 7 (0x0007)                
0x0180 (0x000300) 0x291A-       f:00024 d: 282 | OR[282] = A                   
0x0181 (0x000302) 0x7003-       f:00070 d:   3 | P = P + 3 (0x0184)            
0x0182 (0x000304) 0x1008-       f:00010 d:   8 | A = 8 (0x0008)                
0x0183 (0x000306) 0x291A-       f:00024 d: 282 | OR[282] = A                   
@ Let's get the hell out of the overlay
0x0184 (0x000308) 0x72B7-       f:00071 d: 183 | P = P - 183 (0x00CD)          
0x0185 (0x00030A) 0x0200-       f:00001 d:   0 | EXIT                          

@ Call either XTAPEB or XTAPEC depending on OR[291]
0x0186 (0x00030C) 0x1028-       f:00010 d:  40 | A = 40 (0x0028)               
0x0187 (0x00030E) 0x2924-       f:00024 d: 292 | OR[292] = A                   
0x0188 (0x000310) 0x2123-       f:00020 d: 291 | A = OR[291]                   
0x0189 (0x000312) 0x2925-       f:00024 d: 293 | OR[293] = A                   
0x018A (0x000314) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x018B (0x000316) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x018C (0x000318) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x018D (0x00031A) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x018E (0x00031C) 0x1124-       f:00010 d: 292 | A = 292 (0x0124)              
0x018F (0x00031E) 0x5800-       f:00054 d:   0 | B = A                         
0x0190 (0x000320) 0x1800-0x1918 f:00014 d:   0 | A = 6424 (0x1918)             
0x0192 (0x000324) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

0x0193 (0x000326) 0x291A-       f:00024 d: 282 | OR[282] = A                   
0x0194 (0x000328) 0x211A-       f:00020 d: 282 | A = OR[282]                   
0x0195 (0x00032A) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0197), A # 0     
0x0196 (0x00032C) 0x7002-       f:00070 d:   2 | P = P + 2 (0x0198)            
0x0197 (0x00032E) 0x72CA-       f:00071 d: 202 | P = P - 202 (0x00CD)          
0x0198 (0x000330) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0199 (0x000332) 0xCE0F-       f:00147 d:  15 | io 0017 (EXB), fn007           | Set interrupt mode
0x019A (0x000334) 0x211C-       f:00020 d: 284 | A = OR[284]                   
0x019B (0x000336) 0xCA0F-       f:00145 d:  15 | io 0017 (EXB), fn005           | Load device address
0x019C (0x000338) 0x0200-       f:00001 d:   0 | EXIT                          
0x019D (0x00033A) 0x0000-       f:00000 d:   0 | PASS                          
0x019E (0x00033C) 0x0000-       f:00000 d:   0 | PASS                          
0x019F (0x00033E) 0x0000-       f:00000 d:   0 | PASS                          
