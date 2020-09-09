@ OR[280] : Pointer to raw integer
@ OR[281] : Start bit-position
@ OR[282] : Number of bits in the input
@ OR[283] : Return buffer location
@ OR[284] : Starting buffer offset
@ OR[285] : 0: don't print trailing 0-s, 1: print trailing 0-s
@ OR[286] : Number of digits to display (?)
@
@ Return: Number of characters printed

@ OR[285] determines if OR[289] ends up being 48 (0) or 32 (non-0)
0x0000 (0x000000) 0x1030-       f:00010 d:  48 | A = 48 (0x0030)               
0x0001 (0x000002) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x0002 (0x000004) 0x211D-       f:00020 d: 285 | A = OR[285]                   
0x0003 (0x000006) 0x1601-       f:00013 d:   1 | A = A - 1 (0x0001)            
0x0004 (0x000008) 0x8603-       f:00103 d:   3 | P = P + 3 (0x0007), A # 0     
0x0005 (0x00000A) 0x1020-       f:00010 d:  32 | A = 32 (0x0020)               
0x0006 (0x00000C) 0x2921-       f:00024 d: 289 | OR[289] = A                   
@ OR[285] also determines if OR[286] gets initialized to 10 (0) or left as it came in (non-0)
0x0007 (0x00000E) 0x211D-       f:00020 d: 285 | A = OR[285]                   
0x0008 (0x000010) 0x8603-       f:00103 d:   3 | P = P + 3 (0x000B), A # 0     
0x0009 (0x000012) 0x100A-       f:00010 d:  10 | A = 10 (0x000A)               
0x000A (0x000014) 0x291E-       f:00024 d: 286 | OR[286] = A                   
@
@ Load the requested number of bits from the input number into OR[293] (high word) and OR[294] (low word) in the following loop
@ (very wierd, but we essentially bit-by-bit shift in the input into the two registers above)
@
@ Initialize loop variables OR[293] and OR[294] to both 0.
0x000B (0x000016) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x000C (0x000018) 0x2925-       f:00024 d: 293 | OR[293] = A                   
0x000D (0x00001A) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x000E (0x00001C) 0x2926-       f:00024 d: 294 | OR[294] = A                   
@ Shift result (OR[293]:OR[294]) left by one
0x000F (0x00001E) 0x1010-       f:00010 d:  16 | A = 16 (0x0010)               @ Extract MSB of OR[294] into OR[269]
0x0010 (0x000020) 0x1601-       f:00013 d:   1 | A = A - 1 (0x0001)            
0x0011 (0x000022) 0x5800-       f:00054 d:   0 | B = A                         
0x0012 (0x000024) 0x2126-       f:00020 d: 294 | A = OR[294]                   @@@@@@@@@@@@@@@@@@@@ THIS IS AT 3bb8
0x0013 (0x000026) 0x4800-       f:00044 d:   0 | A = A > B                     
0x0014 (0x000028) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x0015 (0x00002A) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                @ Shift OR[294] up by 1
0x0016 (0x00002C) 0x5800-       f:00054 d:   0 | B = A                         
0x0017 (0x00002E) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x0018 (0x000030) 0x4A00-       f:00045 d:   0 | A = A < B                     
0x0019 (0x000032) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x001A (0x000034) 0x2125-       f:00020 d: 293 | A = OR[293]                   @ Shift OR[293] up by 1, add-in carry from OR[294] (now in OR[269]) @@@@@@@@@@@@@ THIS IS AT 3bc0
0x001B (0x000036) 0x4A00-       f:00045 d:   0 | A = A < B                     
0x001C (0x000038) 0x250D-       f:00022 d: 269 | A = A + OR[269]               
0x001D (0x00003A) 0x2925-       f:00024 d: 293 | OR[293] = A                   

0x001E (0x00003C) 0x100F-       f:00010 d:  15 | A = 15 (0x000F)               @ Calculate bit-position (in B) as the lower-order 4 bits of OR[281] (actually 15-(OR[281] & 0xf)
0x001F (0x00003E) 0x2719-       f:00023 d: 281 | A = A - OR[281]               
0x0020 (0x000040) 0x120F-       f:00011 d:  15 | A = A & 15 (0x000F)           
0x0021 (0x000042) 0x5800-       f:00054 d:   0 | B = A                         
0x0022 (0x000044) 0x2119-       f:00020 d: 281 | A = OR[281]                   @ Calculate the word-offset from the higer-order bits of OR[281], and load the approparite word
0x0023 (0x000046) 0x0804-       f:00004 d:   4 | A = A > 4 (0x0004)            
0x0024 (0x000048) 0x2518-       f:00022 d: 280 | A = A + OR[280]               
0x0025 (0x00004A) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x0026 (0x00004C) 0x310D-       f:00030 d: 269 | A = (OR[269])                 
0x0027 (0x00004E) 0x4800-       f:00044 d:   0 | A = A > B                     @ Extract the appropriate bit from the loaded word and store it in OR[287]
0x0028 (0x000050) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x0029 (0x000052) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x002A (0x000054) 0x2126-       f:00020 d: 294 | A = OR[294]                   @ Add in the extracted bit as the LSB of OR[294]
0x002B (0x000056) 0x251F-       f:00022 d: 287 | A = A + OR[287]               
0x002C (0x000058) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x002D (0x00005A) 0x2F1A-       f:00027 d: 282 | OR[282] = OR[282] - 1         @ Decrement bit-count, increment bit-position and loop if necessary
0x002E (0x00005C) 0x2D19-       f:00026 d: 281 | OR[281] = OR[281] + 1         
0x002F (0x00005E) 0x211A-       f:00020 d: 282 | A = OR[282]                   
0x0030 (0x000060) 0x8E21-       f:00107 d:  33 | P = P - 33 (0x000F), A # 0    

@ Now we have the extracted number in OR[293]:OR[294]. Still in binary format, but only the relevant bits

@ Load current power of 10 from multiplication table below into OR[291] (lower) and OR[290] (higher)
0x0031 (0x000062) 0x211E-       f:00020 d: 286 | A = OR[286]                   @ Load the lower word into OR[291]
0x0032 (0x000064) 0x0A01-       f:00005 d:   1 | A = A < 1 (0x0001)            
0x0033 (0x000066) 0x1C00-0x0097 f:00016 d:   0 | A = A + 151 (0x0097)          
0x0035 (0x00006A) 0x2403-       f:00022 d:   3 | A = A + OR[3]                 
0x0036 (0x00006C) 0x2924-       f:00024 d: 292 | OR[292] = A                   
0x0037 (0x00006E) 0x3124-       f:00030 d: 292 | A = (OR[292])                 
0x0038 (0x000070) 0x2923-       f:00024 d: 291 | OR[291] = A                   
0x0039 (0x000072) 0x2123-       f:00020 d: 291 | A = OR[291]                   
0x003A (0x000074) 0x844C-       f:00102 d:  76 | P = P + 76 (0x0086), A = 0    @ Bail if we fall off at the beginning of the table (no power of 10 should have a lower byte of 0)
0x003B (0x000076) 0x2F24-       f:00027 d: 292 | OR[292] = OR[292] - 1         @ Load the upper word into OR[290]
0x003C (0x000078) 0x3124-       f:00030 d: 292 | A = (OR[292])                 
0x003D (0x00007A) 0x2922-       f:00024 d: 290 | OR[290] = A                   
0x003E (0x00007C) 0x2F24-       f:00027 d: 292 | OR[292] = OR[292] - 1         @ Update pointer to the next power of 10 (well, previous as we start from the top)
@ This seems to be a simple divide loop by continously subtracting OR[291]:OR[290] from OR[293]:OR[294] until we get a negative result (?). We also increment OR[288] in each iteration to get the result
0x003F (0x00007E) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0040 (0x000080) 0x2920-       f:00024 d: 288 | OR[288] = A                   
0x0041 (0x000082) 0x2D20-       f:00026 d: 288 | OR[288] = OR[288] + 1         
0x0042 (0x000084) 0x2125-       f:00020 d: 293 | A = OR[293]                   
0x0043 (0x000086) 0x2722-       f:00023 d: 290 | A = A - OR[290]               
0x0044 (0x000088) 0x2925-       f:00024 d: 293 | OR[293] = A                   
0x0045 (0x00008A) 0x0810-       f:00004 d:  16 | A = A > 16 (0x0010)           
0x0046 (0x00008C) 0x290F-       f:00024 d: 271 | OR[271] = A                   
0x0047 (0x00008E) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x0048 (0x000090) 0x2723-       f:00023 d: 291 | A = A - OR[291]               
0x0049 (0x000092) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x004A (0x000094) 0x8202-       f:00101 d:   2 | P = P + 2 (0x004C), C = 1     
0x004B (0x000096) 0x2F25-       f:00027 d: 293 | OR[293] = OR[293] - 1         
0x004C (0x000098) 0x0810-       f:00004 d:  16 | A = A > 16 (0x0010)           
0x004D (0x00009A) 0x230F-       f:00021 d: 271 | A = A & OR[271]               
0x004E (0x00009C) 0x8E0D-       f:00107 d:  13 | P = P - 13 (0x0041), A # 0    
@ Since we've decremented until the remainder is negative, add once back the divisor (OR[291]:OR[290]) and decement OR[288] to get the true dividend (OR[288]) and remainder (OR[293]:OR[294])
0x004F (0x00009E) 0x2125-       f:00020 d: 293 | A = OR[293]                   
0x0050 (0x0000A0) 0x2522-       f:00022 d: 290 | A = A + OR[290]               
0x0051 (0x0000A2) 0x2925-       f:00024 d: 293 | OR[293] = A                   
0x0052 (0x0000A4) 0x2126-       f:00020 d: 294 | A = OR[294]                   
0x0053 (0x0000A6) 0x2523-       f:00022 d: 291 | A = A + OR[291]               
0x0054 (0x0000A8) 0x2926-       f:00024 d: 294 | OR[294] = A                   
0x0055 (0x0000AA) 0x8002-       f:00100 d:   2 | P = P + 2 (0x0057), C = 0     
0x0056 (0x0000AC) 0x2D25-       f:00026 d: 293 | OR[293] = OR[293] + 1         
0x0057 (0x0000AE) 0x2F20-       f:00027 d: 288 | OR[288] = OR[288] - 1         

0x0058 (0x0000B0) 0x2F1E-       f:00027 d: 286 | OR[286] = OR[286] - 1         @ Decrease the number of digits left to process
0x0059 (0x0000B2) 0x1009-       f:00010 d:   9 | A = 9 (0x0009)                @ Test if digit is biffer than 9 (I guess bail if it is)
0x005A (0x0000B4) 0x2720-       f:00023 d: 288 | A = A - OR[288]               
0x005B (0x0000B6) 0x8029-       f:00100 d:  41 | P = P + 41 (0x0084), C = 0    
@ If the digit is non-0, print it and record the fact that we've printed something in OR[285], so subsequent 0-s are also printed
@ If the digit is 0, but it's the last digit, print it as well
@ If the digit is 0, and not the last one, but OR[285] is 0 (left-aligned, maybe), than simply eat it and continue to the next character
@ If the digit is 0, and not the last one, but OR[285] is non-0 (0-filled or character already printed), print it
0x005C (0x0000B8) 0x2120-       f:00020 d: 288 | A = OR[288]                   @ If digit is 0, and it's not the last digit, do something special, otherwise continue 
0x005D (0x0000BA) 0x8604-       f:00103 d:   4 | P = P + 4 (0x0061), A # 0     
0x005E (0x0000BC) 0x211E-       f:00020 d: 286 | A = OR[286]                   
0x005F (0x0000BE) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0061), A = 0     
0x0060 (0x0000C0) 0x7005-       f:00070 d:   5 | P = P + 5 (0x0065)            
0x0061 (0x0000C2) 0x1030-       f:00010 d:  48 | A = 48 (0x0030)               
0x0062 (0x0000C4) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x0063 (0x0000C6) 0x1002-       f:00010 d:   2 | A = 2 (0x0002)                
0x0064 (0x0000C8) 0x291D-       f:00024 d: 285 | OR[285] = A                   
0x0065 (0x0000CA) 0x211D-       f:00020 d: 285 | A = OR[285]                   
0x0066 (0x0000CC) 0x8C2F-       f:00106 d:  47 | P = P - 47 (0x0037), A = 0    @ Continue on with the next digit
@ Print the character into the buffer at OR[283], offset OR[284]. This is a bit tricky as we're running on a 16-bit machine, but essentially write the byte into the buffer
0x0067 (0x0000CE) 0x2120-       f:00020 d: 288 | A = OR[288]                   @ OR[289] contains the offset of '0', so we calculate the ASCII code for the digit
0x0068 (0x0000D0) 0x2521-       f:00022 d: 289 | A = A + OR[289]               
0x0069 (0x0000D2) 0x2920-       f:00024 d: 288 | OR[288] = A                   
0x006A (0x0000D4) 0x2120-       f:00020 d: 288 | A = OR[288]                   
0x006B (0x0000D6) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x006C (0x0000D8) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x006D (0x0000DA) 0x211C-       f:00020 d: 284 | A = OR[284]                   
0x006E (0x0000DC) 0x0801-       f:00004 d:   1 | A = A > 1 (0x0001)            
0x006F (0x0000DE) 0x251B-       f:00022 d: 283 | A = A + OR[283]               
0x0070 (0x0000E0) 0x290E-       f:00024 d: 270 | OR[270] = A                   
0x0071 (0x0000E2) 0x211C-       f:00020 d: 284 | A = OR[284]                   
0x0072 (0x0000E4) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x0073 (0x0000E6) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0074 (0x0000E8) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0075 (0x0000EA) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x0076 (0x0000EC) 0x8607-       f:00103 d:   7 | P = P + 7 (0x007D), A # 0     
0x0077 (0x0000EE) 0x310E-       f:00030 d: 270 | A = (OR[270])                 
0x0078 (0x0000F0) 0x0A09-       f:00005 d:   9 | A = A < 9 (0x0009)            
0x0079 (0x0000F2) 0x250D-       f:00022 d: 269 | A = A + OR[269]               
0x007A (0x0000F4) 0x0C09-       f:00006 d:   9 | A = A >> 9 (0x0009)           
0x007B (0x0000F6) 0x390E-       f:00034 d: 270 | (OR[270]) = A                 
0x007C (0x0000F8) 0x7006-       f:00070 d:   6 | P = P + 6 (0x0082)            
0x007D (0x0000FA) 0x310E-       f:00030 d: 270 | A = (OR[270])                 
0x007E (0x0000FC) 0x1A00-0xFF00 f:00015 d:   0 | A = A & 65280 (0xFF00)        
0x0080 (0x000100) 0x250D-       f:00022 d: 269 | A = A + OR[269]               
0x0081 (0x000102) 0x390E-       f:00034 d: 270 | (OR[270]) = A                 

0x0082 (0x000104) 0x2D1C-       f:00026 d: 284 | OR[284] = OR[284] + 1         @ Increment byte position
0x0083 (0x000106) 0x724C-       f:00071 d:  76 | P = P - 76 (0x0037)           @ Continue on with the next digit

0x0084 (0x000108) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                @ We get here if there's an error, that is a digit got greater than 9
0x0085 (0x00010A) 0x291C-       f:00024 d: 284 | OR[284] = A                   


0x0086 (0x00010C) 0x2005-       f:00020 d:   5 | A = OR[5]                     @ We get here after the conversion is done: Load the return value (Number of characters printed) into the return value location and return from overlay
0x0087 (0x00010E) 0x1406-       f:00012 d:   6 | A = A + 6 (0x0006)            
0x0088 (0x000110) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0089 (0x000112) 0x211C-       f:00020 d: 284 | A = OR[284]                   
0x008A (0x000114) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x008B (0x000116) 0x102A-       f:00010 d:  42 | A = 42 (0x002A)               
0x008C (0x000118) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x008D (0x00011A) 0x1127-       f:00010 d: 295 | A = 295 (0x0127)              
0x008E (0x00011C) 0x5800-       f:00054 d:   0 | B = A                         
0x008F (0x00011E) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0090 (0x000120) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

@ 0-termination of the multiplication table
0x0091 (0x000122) 0x0000-       f:00000 d:   0 | PASS                          

@ This is a multiplication table. Values are:
0x00000001 1
0x0000000A 10
0x00000064 100
0x000003E8 1000
0x00002710 10000
0x000186A0 100000
0x000F4240 1000000
0x00989680 10000000
0x05F5E100 100000000
0x3B9ACA00 1000000000

0x0092 (0x000124) 0x0000-       f:00000 d:   0 | PASS                          
0x0093 (0x000126) 0x0001-       f:00000 d:   1 | PASS                           | **** non-standard encoding with D:0x0001 ****
0x0094 (0x000128) 0x0000-       f:00000 d:   0 | PASS                          
0x0095 (0x00012A) 0x000A-       f:00000 d:  10 | PASS                           | **** non-standard encoding with D:0x000A ****
0x0096 (0x00012C) 0x0000-       f:00000 d:   0 | PASS                          
0x0097 (0x00012E) 0x0064-       f:00000 d: 100 | PASS                           | **** non-standard encoding with D:0x0064 ****
0x0098 (0x000130) 0x0000-       f:00000 d:   0 | PASS                          
0x0099 (0x000132) 0x03E8-       f:00001 d: 488 | EXIT                           | **** non-standard encoding with D:0x01E8 ****
0x009A (0x000134) 0x0000-       f:00000 d:   0 | PASS                          
0x009B (0x000136) 0x2710-       f:00023 d: 272 | A = A - OR[272]               
0x009C (0x000138) 0x0001-       f:00000 d:   1 | PASS                           | **** non-standard encoding with D:0x0001 ****
0x009D (0x00013A) 0x86A0-       f:00103 d: 160 | P = P + 160 (0x013D), A # 0   
0x009E (0x00013C) 0x000F-       f:00000 d:  15 | PASS                           | **** non-standard encoding with D:0x000F ****
0x009F (0x00013E) 0x4240-       f:00041 d:  64 | C = 1, io 0100 = BZ           
0x00A0 (0x000140) 0x0098-       f:00000 d: 152 | PASS                           | **** non-standard encoding with D:0x0098 ****
0x00A1 (0x000142) 0x9680-       f:00113 d: 128 | R = P + 128 (0x0121), A # 0   
0x00A2 (0x000144) 0x05F5-       f:00002 d: 501 | I = 0                          | **** non-standard encoding with D:0x01F5 ****
0x00A3 (0x000146) 0xE100-       f:00160 d: 256 | IOB , fn000                    | **** non-standard encoding with D:0x0100 ****
0x00A4 (0x000148) 0x3B9A-       f:00035 d: 410 | (OR[410]) = A + (OR[410])     
0x00A5 (0x00014A) 0xCA00-       f:00145 d:   0 | io 0000 (IOR), fn005           | **** Undefined function ****

0x00A6 (0x00014C) 0x0000-       f:00000 d:   0 | PASS                          
0x00A7 (0x00014E) 0x0000-       f:00000 d:   0 | PASS                          
