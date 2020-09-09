@@@@@@@@@@@@@@@@@@ Loaded to address 0x49c0 (0x49c6)
@ OR[280] - device desc. addr.
@ OR[281] - function code
@            8 - search for file# passed in in OR[282]
@            9 - close file
@           10 - close tape
@ OR[282] - buffer address
@
@ OR[283] - return value
@
@ Return value
@   0 - OK
@   6 - search failed
@
@ Load address: 0x5b30 (0x5b36)

0x0000 (0x000000) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0001 (0x000002) 0x291B-       f:00024 d: 283 | OR[283] = A                   
@ Load device address into OR[284]
0x0002 (0x000004) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0003 (0x000006) 0x1402-       f:00012 d:   2 | A = A + 2 (0x0002)            
0x0004 (0x000008) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0005 (0x00000A) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0006 (0x00000C) 0x291C-       f:00024 d: 284 | OR[284] = A                   
@ Function code 8
0x0007 (0x00000E) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x0008 (0x000010) 0x1608-       f:00013 d:   8 | A = A - 8 (0x0008)            
0x0009 (0x000012) 0x8402-       f:00102 d:   2 | P = P + 2 (0x000B), A = 0     
0x000A (0x000014) 0x7003-       f:00070 d:   3 | P = P + 3 (0x000D)            
0x000B (0x000016) 0x7418-       f:00072 d:  24 | R = P + 24 (0x0023)           
0x000C (0x000018) 0x700C-       f:00070 d:  12 | P = P + 12 (0x0018)           
@ Function code 9 and 10
0x000D (0x00001A) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x000E (0x00001C) 0x1609-       f:00013 d:   9 | A = A - 9 (0x0009)            
0x000F (0x00001E) 0x8405-       f:00102 d:   5 | P = P + 5 (0x0014), A = 0     
0x0010 (0x000020) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x0011 (0x000022) 0x160A-       f:00013 d:  10 | A = A - 10 (0x000A)           
0x0012 (0x000024) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0014), A = 0     
0x0013 (0x000026) 0x7003-       f:00070 d:   3 | P = P + 3 (0x0016)            
0x0014 (0x000028) 0x747F-       f:00072 d: 127 | R = P + 127 (0x0093)          
0x0015 (0x00002A) 0x7003-       f:00070 d:   3 | P = P + 3 (0x0018)            
0x0016 (0x00002C) 0x7C34-       f:00076 d:  52 | R = OR[52]                    
0x0017 (0x00002E) 0x0000-       f:00000 d:   0 | PASS                          
@ Set return value (from OR[283])
0x0018 (0x000030) 0x2005-       f:00020 d:   5 | A = OR[5]                     
0x0019 (0x000032) 0x1406-       f:00012 d:   6 | A = A + 6 (0x0006)            
0x001A (0x000034) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x001B (0x000036) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x001C (0x000038) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
@ Exit from overlay
0x001D (0x00003A) 0x102A-       f:00010 d:  42 | A = 42 (0x002A)               
0x001E (0x00003C) 0x2923-       f:00024 d: 291 | OR[291] = A                   
0x001F (0x00003E) 0x1123-       f:00010 d: 291 | A = 291 (0x0123)              @ Wait for interrupt??
0x0020 (0x000040) 0x5800-       f:00054 d:   0 | B = A                         
0x0021 (0x000042) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0022 (0x000044) 0x7C09-       f:00076 d:   9 | R = OR[9]

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Function code 8 handler - search for file # passed in in OR[282] ???
0x0023 (0x000046) 0x74C3-       f:00072 d: 195 | R = P + 195 (0x00E6)          @ Make sure device is ready
0x0024 (0x000048) 0x74D9-       f:00072 d: 217 | R = P + 217 (0x00FD)          @ Test for presence of write-wring (i.e. write-enable)
@ Set entry 19 to 1 - no pending reads
0x0025 (0x00004A) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0026 (0x00004C) 0x1413-       f:00012 d:  19 | A = A + 19 (0x0013)           
0x0027 (0x00004E) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0028 (0x000050) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x0029 (0x000052) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
@ Check entry 18 (current file number), issue XTAPEC 516 if it is greater than the passed-in file number. Also issue command if file 0 is selected (OR[282] is 0)
0x002A (0x000054) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x002B (0x000056) 0x1412-       f:00012 d:  18 | A = A + 18 (0x0012)           
0x002C (0x000058) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x002D (0x00005A) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x002E (0x00005C) 0x2920-       f:00024 d: 288 | OR[288] = A                   
0x002F (0x00005E) 0x211A-       f:00020 d: 282 | A = OR[282]                   
0x0030 (0x000060) 0x2720-       f:00023 d: 288 | A = A - OR[288]               
0x0031 (0x000062) 0x8004-       f:00100 d:   4 | P = P + 4 (0x0035), C = 0     
0x0032 (0x000064) 0x211A-       f:00020 d: 282 | A = OR[282]                   
0x0033 (0x000066) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0035), A = 0     
0x0034 (0x000068) 0x7008-       f:00070 d:   8 | P = P + 8 (0x003C)            
@ Calling XTAPEC function 516 - async rewind ?
0x0035 (0x00006A) 0x1800-0x0204 f:00014 d:   0 | A = 516 (0x0204)              
0x0037 (0x00006E) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x0038 (0x000070) 0x74FF-       f:00072 d: 255 | R = P + 255 (0x0137)          @ Call XTAPEC to perform operation

0x0039 (0x000072) 0x74AD-       f:00072 d: 173 | R = P + 173 (0x00E6)          @ Make sure device is ready
0x003A (0x000074) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x003B (0x000076) 0x2920-       f:00024 d: 288 | OR[288] = A                   

0x003C (0x000078) 0x211A-       f:00020 d: 282 | A = OR[282]                   
0x003D (0x00007A) 0x2720-       f:00023 d: 288 | A = A - OR[288]               
0x003E (0x00007C) 0x8003-       f:00100 d:   3 | P = P + 3 (0x0041), C = 0     
0x003F (0x00007E) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0041), A = 0     
0x0040 (0x000080) 0x7002-       f:00070 d:   2 | P = P + 2 (0x0042)            
0x0041 (0x000082) 0x7002-       f:00070 d:   2 | P = P + 2 (0x0043)            Intended <= Actual
0x0042 (0x000084) 0x7407-       f:00072 d:   7 | R = P + 7 (0x0049)            Intended > Actual
@ Intended file# <= actual file# -> Write back actual to entry 18
0x0043 (0x000086) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0044 (0x000088) 0x1412-       f:00012 d:  18 | A = A + 18 (0x0012)           
0x0045 (0x00008A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0046 (0x00008C) 0x2120-       f:00020 d: 288 | A = OR[288]                   
0x0047 (0x00008E) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0048 (0x000090) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Input:
@@    OR[288]: Actual file number
@@    OR[282]: Intended file number
@ We have to search for the intended file# forward, unless we already there
0x0049 (0x000092) 0x2120-       f:00020 d: 288 | A = OR[288]                   
0x004A (0x000094) 0x271A-       f:00023 d: 282 | A = A - OR[282]               
0x004B (0x000096) 0x8447-       f:00102 d:  71 | P = P + 71 (0x0092), A = 0    
@ Calling XTAPEC function 521 - jump over next file?
0x004C (0x000098) 0x1800-0x0209 f:00014 d:   0 | A = 521 (0x0209)              
0x004E (0x00009C) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x004F (0x00009E) 0x74E8-       f:00072 d: 232 | R = P + 232 (0x0137)          @ Call XTAPEC to perform operation
@ Increment actual file#
0x0050 (0x0000A0) 0x2D20-       f:00026 d: 288 | OR[288] = OR[288] + 1         @@@@@ 0x5b86
@ Calling XTAPEC function 513 - seek one word forward
0x0051 (0x0000A2) 0x1800-0x0201 f:00014 d:   0 | A = 513 (0x0201)              
0x0053 (0x0000A6) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x0054 (0x0000A8) 0x74E3-       f:00072 d: 227 | R = P + 227 (0x0137)          @ Call XTAPEC to perform operation
@ Read RegA status, test for END_OF_FILE
0x0055 (0x0000AA) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0056 (0x0000AC) 0x140E-       f:00012 d:  14 | A = A + 14 (0x000E)           
0x0057 (0x0000AE) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0058 (0x0000B0) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0059 (0x0000B2) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x005A (0x0000B4) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x005B (0x0000B6) 0x1300-       f:00011 d: 256 | A = A & 256 (0x0100)          
0x005C (0x0000B8) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x005D (0x0000BA) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x005E (0x0000BC) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x005F (0x0000BE) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0061), A # 0     @@@@@ 0x5B95
0x0060 (0x0000C0) 0x7029-       f:00070 d:  41 | P = P + 41 (0x0089)           
@ We've seen a double-EOF - this is the mark for an EOT I think
@ Test offset 12 being 2 - opened for write
0x0061 (0x0000C2) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0062 (0x0000C4) 0x140C-       f:00012 d:  12 | A = A + 12 (0x000C)           
0x0063 (0x0000C6) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0064 (0x0000C8) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0065 (0x0000CA) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x0066 (0x0000CC) 0x211F-       f:00020 d: 287 | A = OR[287]                   
0x0067 (0x0000CE) 0x1602-       f:00013 d:   2 | A = A - 2 (0x0002)            
0x0068 (0x0000D0) 0x8402-       f:00102 d:   2 | P = P + 2 (0x006A), A = 0     
0x0069 (0x0000D2) 0x7009-       f:00070 d:   9 | P = P + 9 (0x0072)            

@ Tape is opened for writing
@ If intended file# > actual file# -> set exit code to 6 and return
0x006A (0x0000D4) 0x2120-       f:00020 d: 288 | A = OR[288]                   
0x006B (0x0000D6) 0x271A-       f:00023 d: 282 | A = A - OR[282]               
0x006C (0x0000D8) 0x8002-       f:00100 d:   2 | P = P + 2 (0x006E), C = 0     
0x006D (0x0000DA) 0x7005-       f:00070 d:   5 | P = P + 5 (0x0072)            
@ Set return value to 6 and return
0x006E (0x0000DC) 0x1006-       f:00010 d:   6 | A = 6 (0x0006)                
0x006F (0x0000DE) 0x291B-       f:00024 d: 283 | OR[283] = A                   
0x0070 (0x0000E0) 0x7258-       f:00071 d:  88 | P = P - 88 (0x0018)           
0x0071 (0x0000E2) 0x7017-       f:00070 d:  23 | P = P + 23 (0x0088)           

@ Tape is not opened for writing. Is it opened for reading?
0x0072 (0x0000E4) 0x211F-       f:00020 d: 287 | A = OR[287]                   
0x0073 (0x0000E6) 0x1601-       f:00013 d:   1 | A = A - 1 (0x0001)            
0x0074 (0x0000E8) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0076), A = 0     
0x0075 (0x0000EA) 0x700F-       f:00070 d:  15 | P = P + 15 (0x0084)           
@ Tape os opened for reading
@ Calling XTAPEC function 516 - async rewind
0x0076 (0x0000EC) 0x1800-0x0204 f:00014 d:   0 | A = 516 (0x0204)              
0x0078 (0x0000F0) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x0079 (0x0000F2) 0x74BE-       f:00072 d: 190 | R = P + 190 (0x0137)          @ Call XTAPEC to perform operation
0x007A (0x0000F4) 0x746C-       f:00072 d: 108 | R = P + 108 (0x00E6)          @ Make sure device is ready
@ Zero out actual file # at offset 18
0x007B (0x0000F6) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x007C (0x0000F8) 0x1412-       f:00012 d:  18 | A = A + 18 (0x0012)           
0x007D (0x0000FA) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x007E (0x0000FC) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x007F (0x0000FE) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
@ Set return value to 6 and return
0x0080 (0x000100) 0x1006-       f:00010 d:   6 | A = 6 (0x0006)                
0x0081 (0x000102) 0x291B-       f:00024 d: 283 | OR[283] = A                   
0x0082 (0x000104) 0x726A-       f:00071 d: 106 | P = P - 106 (0x0018)          
0x0083 (0x000106) 0x7005-       f:00070 d:   5 | P = P + 5 (0x0088)            

@ We get here if open-mode is 0, which it should never be!!!
@ Calling XTAPEC function 512 - seek one word backwards
0x0084 (0x000108) 0x1800-0x0200 f:00014 d:   0 | A = 512 (0x0200)              
0x0086 (0x00010C) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x0087 (0x00010E) 0x74B0-       f:00072 d: 176 | R = P + 176 (0x0137)          @ Call XTAPEC to perform operation
0x0088 (0x000110) 0x7009-       f:00070 d:   9 | P = P + 9 (0x0091)            

@ We get here if no EOF was found after operation 513
@ Calling XTAPEC function 512 - step backwards one location?
0x0089 (0x000112) 0x1800-0x0200 f:00014 d:   0 | A = 512 (0x0200)              
0x008B (0x000116) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x008C (0x000118) 0x74AB-       f:00072 d: 171 | R = P + 171 (0x0137)          @ Call XTAPEC to perform operation
@ Call XTAPED with function code 4096 - load tape
0x008D (0x00011A) 0x1800-0x1000 f:00014 d:   0 | A = 4096 (0x1000)             
0x008F (0x00011E) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x0090 (0x000120) 0x74AB-       f:00072 d: 171 | R = P + 171 (0x013B)          @ OR[286] - function code for XTAPED
@ Continue looping
0x0091 (0x000122) 0x7248-       f:00071 d:  72 | P = P - 72 (0x0049)           
0x0092 (0x000124) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Function code 9 and 10 handler                     
@@ For function 9: - close file
@@    issue XTAPEC command 519 - write EOF
@@    increment offset 18
@@ For function 10: - close tape??
@@    issue XTAPEC command 519 - write EOF
@@    issue XTAPEC command 519 - write EOF
@@    issue XTAPEC command 517 - blocking rewind??
@@    zero offset 18
@ Load entry 12, see if it is 2
0x0093 (0x000126) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0094 (0x000128) 0x140C-       f:00012 d:  12 | A = A + 12 (0x000C)           
0x0095 (0x00012A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0096 (0x00012C) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0097 (0x00012E) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x0098 (0x000130) 0x211F-       f:00020 d: 287 | A = OR[287]                   
0x0099 (0x000132) 0x1602-       f:00013 d:   2 | A = A - 2 (0x0002)            
0x009A (0x000134) 0x8402-       f:00102 d:   2 | P = P + 2 (0x009C), A = 0     
0x009B (0x000136) 0x702D-       f:00070 d:  45 | P = P + 45 (0x00C8)           
@ Entry 12 is 2 - load lower byte of entry 17 (interrupt count), see if it is 0
0x009C (0x000138) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x009D (0x00013A) 0x1411-       f:00012 d:  17 | A = A + 17 (0x0011)           
0x009E (0x00013C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x009F (0x00013E) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x00A0 (0x000140) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x00A1 (0x000142) 0x8602-       f:00103 d:   2 | P = P + 2 (0x00A3), A # 0     
0x00A2 (0x000144) 0x7019-       f:00070 d:  25 | P = P + 25 (0x00BB)           
@ We've had interrupts, read RegA from desc., see if there's an error to report (MSB set), if yes, report it through XTAPEB, function code 70
0x00A3 (0x000146) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x00A4 (0x000148) 0x140E-       f:00012 d:  14 | A = A + 14 (0x000E)           
0x00A5 (0x00014A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00A6 (0x00014C) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x00A7 (0x00014E) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x00A8 (0x000150) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x00A9 (0x000152) 0x1A00-0x8000 f:00015 d:   0 | A = A & 32768 (0x8000)        
0x00AB (0x000156) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00AC (0x000158) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00AD (0x00015A) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x00AE (0x00015C) 0x8602-       f:00103 d:   2 | P = P + 2 (0x00B0), A # 0     
0x00AF (0x00015E) 0x700C-       f:00070 d:  12 | P = P + 12 (0x00BB)           
0x00B0 (0x000160) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x00B1 (0x000162) 0x1A00-0x5422 f:00015 d:   0 | A = A & 21538 (0x5422)        
0x00B3 (0x000166) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00B4 (0x000168) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00B5 (0x00016A) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x00B6 (0x00016C) 0x8602-       f:00103 d:   2 | P = P + 2 (0x00B8), A # 0     
0x00B7 (0x00016E) 0x7004-       f:00070 d:   4 | P = P + 4 (0x00BB)            
0x00B8 (0x000170) 0x1046-       f:00010 d:  70 | A = 70 (0x0046)               
0x00B9 (0x000172) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x00BA (0x000174) 0x7479-       f:00072 d: 121 | R = P + 121 (0x0133)          @ Call XTAPEB to perform operation
@ Done with device error reporting, let's continue
@ Calling XTAPEC function 519
0x00BB (0x000176) 0x1800-0x0207 f:00014 d:   0 | A = 519 (0x0207)              
0x00BD (0x00017A) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x00BE (0x00017C) 0x7479-       f:00072 d: 121 | R = P + 121 (0x0137)          @ Call XTAPEC to perform operation
@ Test function code
0x00BF (0x00017E) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x00C0 (0x000180) 0x160A-       f:00013 d:  10 | A = A - 10 (0x000A)           
0x00C1 (0x000182) 0x8402-       f:00102 d:   2 | P = P + 2 (0x00C3), A = 0     
0x00C2 (0x000184) 0x7002-       f:00070 d:   2 | P = P + 2 (0x00C4)            
@ Calling XTAPEC function 519?? again - only for function code 10
0x00C3 (0x000186) 0x7474-       f:00072 d: 116 | R = P + 116 (0x0137)          @ Call XTAPEC to perform operation
@ Increment entry 18
0x00C4 (0x000188) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x00C5 (0x00018A) 0x1412-       f:00012 d:  18 | A = A + 18 (0x0012)           
0x00C6 (0x00018C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00C7 (0x00018E) 0x3D08-       f:00036 d: 264 | (OR[264]) = (OR[264]) + 1     
@ Test function code
0x00C8 (0x000190) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x00C9 (0x000192) 0x160A-       f:00013 d:  10 | A = A - 10 (0x000A)           
0x00CA (0x000194) 0x8402-       f:00102 d:   2 | P = P + 2 (0x00CC), A = 0     
0x00CB (0x000196) 0x700A-       f:00070 d:  10 | P = P + 10 (0x00D5)           
@ Calling XTAPEC function 517 - only for function code 10
0x00CC (0x000198) 0x1800-0x0205 f:00014 d:   0 | A = 517 (0x0205)              
0x00CE (0x00019C) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x00CF (0x00019E) 0x7468-       f:00072 d: 104 | R = P + 104 (0x0137)          @ Call XTAPEC to perform operation
@ Clear desc. offset 18 - only for function code 10
0x00D0 (0x0001A0) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x00D1 (0x0001A2) 0x1412-       f:00012 d:  18 | A = A + 18 (0x0012)           
0x00D2 (0x0001A4) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00D3 (0x0001A6) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00D4 (0x0001A8) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
@ Common code continues:
@ Clear the upper byte of entry 17 - no async operations pending
0x00D5 (0x0001AA) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x00D6 (0x0001AC) 0x1411-       f:00012 d:  17 | A = A + 17 (0x0011)           
0x00D7 (0x0001AE) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00D8 (0x0001B0) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x00D9 (0x0001B2) 0x0E01-       f:00007 d:   1 | A = A << 1 (0x0001)           
0x00DA (0x0001B4) 0x0A08-       f:00005 d:   8 | A = A < 8 (0x0008)            
0x00DB (0x0001B6) 0x1400-       f:00012 d:   0 | A = A + 0 (0x0000)            
0x00DC (0x0001B8) 0x0C09-       f:00006 d:   9 | A = A >> 9 (0x0009)           
0x00DD (0x0001BA) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
@ Clear he lower byte of entry 17
0x00DE (0x0001BC) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x00DF (0x0001BE) 0x1411-       f:00012 d:  17 | A = A + 17 (0x0011)           
0x00E0 (0x0001C0) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00E1 (0x0001C2) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x00E2 (0x0001C4) 0x1A00-0xFF00 f:00015 d:   0 | A = A & 65280 (0xFF00)        
0x00E4 (0x0001C8) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x00E5 (0x0001CA) 0x0200-       f:00001 d:   0 | EXIT                          


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Makes sure device is ready. If not, reports NOT READY
0x00E6 (0x0001CC) 0x7438-       f:00072 d:  56 | R = P + 56 (0x011E)            @ Read RegA from device into OR[289]
0x00E7 (0x0001CE) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x00E8 (0x0001D0) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x00E9 (0x0001D2) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00EA (0x0001D4) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00EB (0x0001D6) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x00EC (0x0001D8) 0x8402-       f:00102 d:   2 | P = P + 2 (0x00EE), A = 0     
@ Ready, exit
0x00ED (0x0001DA) 0x700F-       f:00070 d:  15 | P = P + 15 (0x00FC)           
@ Device ready, check the BUSY bit
0x00EE (0x0001DC) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x00EF (0x0001DE) 0x1A00-0x2000 f:00015 d:   0 | A = A & 8192 (0x2000)         
0x00F1 (0x0001E2) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00F2 (0x0001E4) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00F3 (0x0001E6) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x00F4 (0x0001E8) 0x8602-       f:00103 d:   2 | P = P + 2 (0x00F6), A # 0     
0x00F5 (0x0001EA) 0x7004-       f:00070 d:   4 | P = P + 4 (0x00F9)            
@ Device busy - wait for interrupt and retry
0x00F6 (0x0001EC) 0x742D-       f:00072 d:  45 | R = P + 45 (0x0123)           @ Wait for interrupt??
0x00F7 (0x0001EE) 0x7211-       f:00071 d:  17 | P = P - 17 (0x00E6)           
@ This never gets executed...
0x00F8 (0x0001F0) 0x7004-       f:00070 d:   4 | P = P + 4 (0x00FC)            
@ So device is not BUSY, report NOT READY through XTAPEB
0x00F9 (0x0001F2) 0x1040-       f:00010 d:  64 | A = 64 (0x0040)               
0x00FA (0x0001F4) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x00FB (0x0001F6) 0x7438-       f:00072 d:  56 | R = P + 56 (0x0133)           @ Call XTAPEB to perform operation
0x00FC (0x0001F8) 0x0200-       f:00001 d:   0 | EXIT                          


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Test for presence of write-wring
@ Read entry 12, test if it's 2
0x00FD (0x0001FA) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x00FE (0x0001FC) 0x140C-       f:00012 d:  12 | A = A + 12 (0x000C)           
0x00FF (0x0001FE) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0100 (0x000200) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0101 (0x000202) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x0102 (0x000204) 0x211F-       f:00020 d: 287 | A = OR[287]                   
0x0103 (0x000206) 0x1602-       f:00013 d:   2 | A = A - 2 (0x0002)            
0x0104 (0x000208) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0106), A = 0     
0x0105 (0x00020A) 0x700C-       f:00070 d:  12 | P = P + 12 (0x0111)           
@ Entry 12 is 2, test for write-ring. If not present, report error and return. If present, simply return
0x0106 (0x00020C) 0x7418-       f:00072 d:  24 | R = P + 24 (0x011E)           @ Read RegA from device into OR[289]
0x0107 (0x00020E) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x0108 (0x000210) 0x1204-       f:00011 d:   4 | A = A & 4 (0x0004)            
0x0109 (0x000212) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x010A (0x000214) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x010B (0x000216) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x010C (0x000218) 0x8602-       f:00103 d:   2 | P = P + 2 (0x010E), A # 0     
0x010D (0x00021A) 0x7004-       f:00070 d:   4 | P = P + 4 (0x0111)            
0x010E (0x00021C) 0x1043-       f:00010 d:  67 | A = 67 (0x0043)               
0x010F (0x00021E) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x0110 (0x000220) 0x7423-       f:00072 d:  35 | R = P + 35 (0x0133)           @ Call XTAPEB to perform operation
0x0111 (0x000222) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Wait for delayed operations
0x0112 (0x000224) 0x1190-       f:00010 d: 400 | A = 400 (0x0190)              
0x0113 (0x000226) 0x8405-       f:00102 d:   5 | P = P + 5 (0x0118), A = 0     
0x0114 (0x000228) 0x420F-       f:00041 d:  15 | C = 1, io 0017 (EXB) = BZ       
0x0115 (0x00022A) 0x8003-       f:00100 d:   3 | P = P + 3 (0x0118), C = 0     
0x0116 (0x00022C) 0x1601-       f:00013 d:   1 | A = A - 1 (0x0001)            
0x0117 (0x00022E) 0x7204-       f:00071 d:   4 | P = P - 4 (0x0113)            
0x0118 (0x000230) 0x8402-       f:00102 d:   2 | P = P + 2 (0x011A), A = 0     
0x0119 (0x000232) 0x7004-       f:00070 d:   4 | P = P + 4 (0x011D)            
0x011A (0x000234) 0x1041-       f:00010 d:  65 | A = 65 (0x0041)               
0x011B (0x000236) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x011C (0x000238) 0x7417-       f:00072 d:  23 | R = P + 23 (0x0133)           @ Call XTAPEB to perform operation
0x011D (0x00023A) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Read RegA from device into OR[289]
0x011E (0x00023C) 0xC20F-       f:00141 d:  15 | io 0017 (EXB), fn001           | Data input from A register (DIA)
0x011F (0x00023E) 0x760D-       f:00073 d:  13 | R = P - 13 (0x0112)           
0x0120 (0x000240) 0xD00F-       f:00150 d:  15 | io 0017 (EXB), fn010           | Read data bus status
0x0121 (0x000242) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x0122 (0x000244) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Wait for interrupt??
0x0123 (0x000246) 0x1002-       f:00010 d:   2 | A = 2 (0x0002)                
0x0124 (0x000248) 0xCE0F-       f:00147 d:  15 | io 0017 (EXB), fn007           | Set interrupt mode
@ Kernel function PAUS - wait for 1ms (?)
0x0125 (0x00024A) 0x1007-       f:00010 d:   7 | A = 7 (0x0007)                
0x0126 (0x00024C) 0x2923-       f:00024 d: 291 | OR[291] = A                   
0x0127 (0x00024E) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x0128 (0x000250) 0x2924-       f:00024 d: 292 | OR[292] = A                   
0x0129 (0x000252) 0x1123-       f:00010 d: 291 | A = 291 (0x0123)              
0x012A (0x000254) 0x5800-       f:00054 d:   0 | B = A                         
0x012B (0x000256) 0x1800-0x1718 f:00014 d:   0 | A = 5912 (0x1718)             
0x012D (0x00025A) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

0x012E (0x00025C) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x012F (0x00025E) 0xCE0F-       f:00147 d:  15 | io 0017 (EXB), fn007           | Set interrupt mode
0x0130 (0x000260) 0x211C-       f:00020 d: 284 | A = OR[284]                  
0x0131 (0x000262) 0xCA0F-       f:00145 d:  15 | io 0017 (EXB), fn005           | Load device address
0x0132 (0x000264) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Call XTAPEB to perform operation
@ OR[286] - function code for XTAPEB
0x0133 (0x000266) 0x1800-0x00AF f:00014 d:   0 | A = 175 (0x00AF)              
0x0135 (0x00026A) 0x2922-       f:00024 d: 290 | OR[290] = A                   
0x0136 (0x00026C) 0x7009-       f:00070 d:   9 | P = P + 9 (0x013F)            

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Call XTAPEC to perform operation
@ OR[286] - function code for XTAPEC
0x0137 (0x00026E) 0x1800-0x00B0 f:00014 d:   0 | A = 176 (0x00B0)              
0x0139 (0x000272) 0x2922-       f:00024 d: 290 | OR[290] = A                   
0x013A (0x000274) 0x7005-       f:00070 d:   5 | P = P + 5 (0x013F)            

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Call XTAPED to perform operation
@ OR[286] - function code for XTAPED
0x013B (0x000276) 0x1800-0x00B1 f:00014 d:   0 | A = 177 (0x00B1)              
0x013D (0x00027A) 0x2922-       f:00024 d: 290 | OR[290] = A                   
0x013E (0x00027C) 0x7001-       f:00070 d:   1 | P = P + 1 (0x013F)            
@ Call XTAPEB, XTAPEC or XTAPED
0x013F (0x00027E) 0x1028-       f:00010 d:  40 | A = 40 (0x0028)               
0x0140 (0x000280) 0x2923-       f:00024 d: 291 | OR[291] = A                   
0x0141 (0x000282) 0x2122-       f:00020 d: 290 | A = OR[290]                   
0x0142 (0x000284) 0x2924-       f:00024 d: 292 | OR[292] = A                   
0x0143 (0x000286) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0144 (0x000288) 0x2925-       f:00024 d: 293 | OR[293] = A                   
0x0145 (0x00028A) 0x211E-       f:00020 d: 286 | A = OR[286]                   
0x0146 (0x00028C) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x0147 (0x00028E) 0x1123-       f:00010 d: 291 | A = 291 (0x0123)              
0x0148 (0x000290) 0x5800-       f:00054 d:   0 | B = A                         
0x0149 (0x000292) 0x1800-0x1718 f:00014 d:   0 | A = 5912 (0x1718)             
0x014B (0x000296) 0x7C09-       f:00076 d:   9 | R = OR[9]                     
@ If return code is 7, eat it and replace it with 0 and continue as if it was 0
@ If return code is 0, re-enable interrupts and return to callee, if non-0, exit from overlay and propagate return code
0x014C (0x000298) 0x291B-       f:00024 d: 283 | OR[283] = A                   
0x014D (0x00029A) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x014E (0x00029C) 0x1607-       f:00013 d:   7 | A = A - 7 (0x0007)            
0x014F (0x00029E) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0151), A = 0     
0x0150 (0x0002A0) 0x7004-       f:00070 d:   4 | P = P + 4 (0x0154)            
0x0151 (0x0002A2) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0152 (0x0002A4) 0x291B-       f:00024 d: 283 | OR[283] = A                   
0x0153 (0x0002A6) 0x7005-       f:00070 d:   5 | P = P + 5 (0x0158)            
0x0154 (0x0002A8) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x0155 (0x0002AA) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0157), A # 0     
0x0156 (0x0002AC) 0x7002-       f:00070 d:   2 | P = P + 2 (0x0158)            
0x0157 (0x0002AE) 0x733F-       f:00071 d: 319 | P = P - 319 (0x0018)          
0x0158 (0x0002B0) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0159 (0x0002B2) 0xCE0F-       f:00147 d:  15 | io 0017 (EXB), fn007           | Set interrupt mode
0x015A (0x0002B4) 0x211C-       f:00020 d: 284 | A = OR[284]                   
0x015B (0x0002B6) 0xCA0F-       f:00145 d:  15 | io 0017 (EXB), fn005           | Load device address
0x015C (0x0002B8) 0x0200-       f:00001 d:   0 | EXIT                          
0x015D (0x0002BA) 0x0000-       f:00000 d:   0 | PASS                          
0x015E (0x0002BC) 0x0000-       f:00000 d:   0 | PASS                          
0x015F (0x0002BE) 0x0000-       f:00000 d:   0 | PASS                          
