@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Input:
@@    OR[280]: pointer to device desc. structure. layout:
@@               0, 1: string of the device name (MT0)
@@               2: device address (022)
@@               3: device driver offset number
@@               4:
@@               5:
@@               6:
@@               7:
@@               8: cleared on device close or file close - context? of sorts (compared to OR[6] in XOPN)
@@               9: Linked list of contexts waiting on interrupts on this device
@@               10:
@@               11: upper byte cleared on device close or file close - probably contains return value from sub-drivers or something
@@               12: cleared on device close or file close - open mode: 1 - read, 2 - write
@@               13: cleared on device close - context in which the device was opened??? (input parameter to XOPN, which might come from OR[6])
@@               14: RegA
@@               15: RegB
@@               16: RegC
@@               17: Lower byte: interrupt counter, higher byte: non-0 if async operation is in progress (cleared by the EXB interrupt handler)
@@               18: Current file index
@@               19: Some sort of a bool flag: could be 1 or 0 - maybe: 0- command pending 1- no command pending
@@               20: upper byte: last RegA command issued , lower byte: recovery couter
@@               21: number of bytes returned in buffer
@@               22: number of bytes to transfer in a read/write operation
@@    OR[281]: some sort of a command code
@@               512 - issue command (RegA) 32, with RegC set to -1 - space backward by one byte
@@               513 - issue command (RegA) 24, with RegC set to -1 - space forward by one byte?
@@               514 - read bytes from tape to buffer at 0x26B0
@@               515 - write bytes to tape from buffer at 0x26B0
@@               516 - async rewind (repetitive command 8 until 'LOAD POINT DETECTED' is returned) - while waiting, CPU is yielded for other tasks
@@               517 - blocking rewind (repetitive command 8 until 'LOAD POINT DETECTED' is returned)
@@               518 - issue command 56 (no RegB or RegC update)
@@               519 - issue command 48 (no RegB or RegC update) - write EOF??
@@               520 - repetitively issue command 32 with RegC = -200, until END OF FILE is returned, then issue command (RegA) 24, with RegC set to -1 -- find file backward?
@@               521 - repetitively issue command 24 with RegC = -200 until END OF FILE is returned --  find next file forward?
@@ The load-address for this overlay is 0x5C8E
@@ Return
@@    OR[282]: return code
@@	0 - OK


@@ 32 - 10000 - space backward
@@  8 - 00100 - rewind?
@@ 24 - 01100 - space forward
@@ 48 - 11000 - write EOF forward
@@  0 - 00000 - read forward
@@ 40 - 10100 - write forward

@@ Though it doesn't quite match up the bit-patterns, but here's some description on tape command formats:
@@ http://www.sydex.com/pertec.html - this is a quite wide-spread standard. Kennedy seems to have been using it before changing to SCSI
@@ Some more docs on the pertec interface:
@@    http://bitsavers.trailing-edge.com/pdf/dilog/2120-0090-1_DU142_Jul87.pdf
@@    http://bitsavers.informatik.uni-stuttgart.de/pdf/m4data/125456D_9914V_Service_Jul93.pdf
@@ http://ibm-1401.info/729-Info.html - this is a different format

@@ Test if tape drive is rewinding (0x2000). If it does, wait until that condition clears
@@ and check for the success of it. That is: rewind should stop with READY going high and LOAD POINT DETECT being high as well
0x0000 (0x000000) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0001 (0x000002) 0x291A-       f:00024 d: 282 | OR[282] = A                   
0x0002 (0x000004) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0003 (0x000006) 0x1402-       f:00012 d:   2 | A = A + 2 (0x0002)            
0x0004 (0x000008) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0005 (0x00000A) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0006 (0x00000C) 0x291B-       f:00024 d: 283 | OR[283] = A                   @ Store device address in OR[283]
0x0007 (0x00000E) 0x7529-       f:00072 d: 297 | R = P + 297 (0x0130)          @ Read RegA, returns in OR[286]
0x0008 (0x000010) 0x211E-       f:00020 d: 286 | A = OR[286]                   
0x0009 (0x000012) 0x1A00-0x2000 f:00015 d:   0 | A = A & 8192 (0x2000)         
0x000B (0x000016) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x000C (0x000018) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x000D (0x00001A) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x000E (0x00001C) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0010), A # 0     
0x000F (0x00001E) 0x701F-       f:00070 d:  31 | P = P + 31 (0x002E)           
@ We get here if bit 0x2000 set in RegA. This is a loop that waits until that bit is cleared, that is, wait for the tape to finish rewiding
0x0010 (0x000020) 0x211E-       f:00020 d: 286 | A = OR[286]                   
0x0011 (0x000022) 0x1A00-0x2000 f:00015 d:   0 | A = A & 8192 (0x2000)         
0x0013 (0x000026) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0014 (0x000028) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0015 (0x00002A) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x0016 (0x00002C) 0x8404-       f:00102 d:   4 | P = P + 4 (0x001A), A = 0     
0x0017 (0x00002E) 0x751E-       f:00072 d: 286 | R = P + 286 (0x0135)          @ YIELD
0x0018 (0x000030) 0x7518-       f:00072 d: 280 | R = P + 280 (0x0130)          @ Read RegA from device
0x0019 (0x000032) 0x7209-       f:00071 d:   9 | P = P - 9 (0x0010)            

0x001A (0x000034) 0x100A-       f:00010 d:  10 | A = 10 (0x000A)               
0x001B (0x000036) 0x2920-       f:00024 d: 288 | OR[288] = A                   
@ Test is we're at the beginning of tape? (LOAD POINT DETECT and READY)
0x001C (0x000038) 0x211E-       f:00020 d: 286 | A = OR[286]                   
0x001D (0x00003A) 0x1281-       f:00011 d: 129 | A = A & 129 (0x0081)          
0x001E (0x00003C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x001F (0x00003E) 0x1081-       f:00010 d: 129 | A = 129 (0x0081)              
0x0020 (0x000040) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x0021 (0x000042) 0x8407-       f:00102 d:   7 | P = P + 7 (0x0028), A = 0     
0x0022 (0x000044) 0x2120-       f:00020 d: 288 | A = OR[288]                   
0x0023 (0x000046) 0x8405-       f:00102 d:   5 | P = P + 5 (0x0028), A = 0     
0x0024 (0x000048) 0x7511-       f:00072 d: 273 | R = P + 273 (0x0135)          @ YIELD
0x0025 (0x00004A) 0x750B-       f:00072 d: 267 | R = P + 267 (0x0130)          @ Read RegA from device
@ Loop
0x0026 (0x00004C) 0x2F20-       f:00027 d: 288 | OR[288] = OR[288] - 1         
0x0027 (0x00004E) 0x720B-       f:00071 d:  11 | P = P - 11 (0x001C)           
@ We're not on a load-point or timed out waiting on it to clear
0x0028 (0x000050) 0x2120-       f:00020 d: 288 | A = OR[288]                   
0x0029 (0x000052) 0x8402-       f:00102 d:   2 | P = P + 2 (0x002B), A = 0     
@ If timed out, return error
0x002A (0x000054) 0x7004-       f:00070 d:   4 | P = P + 4 (0x002E)            
0x002B (0x000056) 0x1042-       f:00010 d:  66 | A = 66 (0x0042)               
0x002C (0x000058) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x002D (0x00005A) 0x7518-       f:00072 d: 280 | R = P + 280 (0x0145)          @ Error 66 - DEVICE TIMEOUT

@ Parse the operation request, passed in in OR[281]
0x002E (0x00005C) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x002F (0x00005E) 0x1E00-0x0207 f:00017 d:   0 | A = A - 519 (0x0207)          
0x0031 (0x000062) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0033), A = 0     
0x0032 (0x000064) 0x7005-       f:00070 d:   5 | P = P + 5 (0x0037)            

@ Code 519 handler
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Issue command (RegA) 48
0x0033 (0x000066) 0x1030-       f:00010 d:  48 | A = 48 (0x0030)               
0x0034 (0x000068) 0x291C-       f:00024 d: 284 | OR[284] = A                   
0x0035 (0x00006A) 0x7499-       f:00072 d: 153 | R = P + 153 (0x00CE)          @ Start operation without updateing RegB or RegC
0x0036 (0x00006C) 0x7085-       f:00070 d: 133 | P = P + 133 (0x00BB)          
@ -- switch
0x0037 (0x00006E) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x0038 (0x000070) 0x1E00-0x0204 f:00017 d:   0 | A = A - 516 (0x0204)          
0x003A (0x000074) 0x8406-       f:00102 d:   6 | P = P + 6 (0x0040), A = 0     
0x003B (0x000076) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x003C (0x000078) 0x1E00-0x0205 f:00017 d:   0 | A = A - 517 (0x0205)          
0x003E (0x00007C) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0040), A = 0     
0x003F (0x00007E) 0x7014-       f:00070 d:  20 | P = P + 20 (0x0053)           

@ Code 516 and 517 handler
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
0x0040 (0x000080) 0x74F0-       f:00072 d: 240 | R = P + 240 (0x0130)          @ Read RegA again, returned in OR[286]
0x0041 (0x000082) 0x211E-       f:00020 d: 286 | A = OR[286]                   
0x0042 (0x000084) 0x1280-       f:00011 d: 128 | A = A & 128 (0x0080)          @ Test if bit 7 is set 
0x0043 (0x000086) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0044 (0x000088) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0045 (0x00008A) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x0046 (0x00008C) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0048), A = 0     
0x0047 (0x00008E) 0x700B-       f:00070 d:  11 | P = P + 11 (0x0052)           @ If bit-7 is set, we're done
@ Issue command (RegA) 8 - rewind
0x0048 (0x000090) 0x1008-       f:00010 d:   8 | A = 8 (0x0008)                
0x0049 (0x000092) 0x291C-       f:00024 d: 284 | OR[284] = A                   
0x004A (0x000094) 0x7484-       f:00072 d: 132 | R = P + 132 (0x00CE)          @ Start operation without updating RegB or RegC
0x004B (0x000096) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x004C (0x000098) 0x1E00-0x0204 f:00017 d:   0 | A = A - 516 (0x0204)          
0x004E (0x00009C) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0050), A = 0     
0x004F (0x00009E) 0x7003-       f:00070 d:   3 | P = P + 3 (0x0052)            
0x0050 (0x0000A0) 0x74E5-       f:00072 d: 229 | R = P + 229 (0x0135)          @ This call only happens for command 516 - wait on interrupt ?
0x0051 (0x0000A2) 0x724A-       f:00071 d:  74 | P = P - 74 (0x0007)           @ Re-do the whole command again - so essentially we try to rewind or load until bit-7 is set...
0x0052 (0x0000A4) 0x7069-       f:00070 d: 105 | P = P + 105 (0x00BB)          
@ -- switch
0x0053 (0x0000A6) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x0054 (0x0000A8) 0x1E00-0x0206 f:00017 d:   0 | A = A - 518 (0x0206)          
0x0056 (0x0000AC) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0058), A = 0     
0x0057 (0x0000AE) 0x7005-       f:00070 d:   5 | P = P + 5 (0x005C)            

@ Code 518 handler
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Issue command (RegA) 56
0x0058 (0x0000B0) 0x1038-       f:00010 d:  56 | A = 56 (0x0038)               
0x0059 (0x0000B2) 0x291C-       f:00024 d: 284 | OR[284] = A                   
0x005A (0x0000B4) 0x7474-       f:00072 d: 116 | R = P + 116 (0x00CE)          @ Start operation without updateing RegB or RegC
0x005B (0x0000B6) 0x7060-       f:00070 d:  96 | P = P + 96 (0x00BB)           
@ -- switch
0x005C (0x0000B8) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x005D (0x0000BA) 0x1E00-0x0200 f:00017 d:   0 | A = A - 512 (0x0200)          
0x005F (0x0000BE) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0061), A = 0     
0x0060 (0x0000C0) 0x7007-       f:00070 d:   7 | P = P + 7 (0x0067)            

@ Code 512 handler
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Issue command (RegA) 32, with RegC set to -1
0x0061 (0x0000C2) 0x1020-       f:00010 d:  32 | A = 32 (0x0020)               
0x0062 (0x0000C4) 0x291C-       f:00024 d: 284 | OR[284] = A                   
0x0063 (0x0000C6) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x0064 (0x0000C8) 0x291D-       f:00024 d: 285 | OR[285] = A                   
0x0065 (0x0000CA) 0x7465-       f:00072 d: 101 | R = P + 101 (0x00CA)          @ Start operation without updateing RegB
0x0066 (0x0000CC) 0x7055-       f:00070 d:  85 | P = P + 85 (0x00BB)           
@ -- switch
0x0067 (0x0000CE) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x0068 (0x0000D0) 0x1E00-0x0201 f:00017 d:   0 | A = A - 513 (0x0201)          
0x006A (0x0000D4) 0x8402-       f:00102 d:   2 | P = P + 2 (0x006C), A = 0     
0x006B (0x0000D6) 0x7007-       f:00070 d:   7 | P = P + 7 (0x0072)            

@ Code 513 handler
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Issue command (RegA) 24, with RegC set to -1
0x006C (0x0000D8) 0x1018-       f:00010 d:  24 | A = 24 (0x0018)               
0x006D (0x0000DA) 0x291C-       f:00024 d: 284 | OR[284] = A                   
0x006E (0x0000DC) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x006F (0x0000DE) 0x291D-       f:00024 d: 285 | OR[285] = A                   
0x0070 (0x0000E0) 0x745A-       f:00072 d:  90 | R = P + 90 (0x00CA)           @ Start operation without updateing RegB
0x0071 (0x0000E2) 0x704A-       f:00070 d:  74 | P = P + 74 (0x00BB)           
@ -- switch
0x0072 (0x0000E4) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x0073 (0x0000E6) 0x1E00-0x0208 f:00017 d:   0 | A = A - 520 (0x0208)          
0x0075 (0x0000EA) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0077), A = 0     
0x0076 (0x0000EC) 0x7015-       f:00070 d:  21 | P = P + 21 (0x008B)           

@ Code 520 handler
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
0x0077 (0x0000EE) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0078 (0x0000F0) 0x291E-       f:00024 d: 286 | OR[286] = A                   
@ Repetitively issue command 32 with RegC = -200, until END OF FILE is returned
0x0079 (0x0000F2) 0x211E-       f:00020 d: 286 | A = OR[286]                   
0x007A (0x0000F4) 0x1300-       f:00011 d: 256 | A = A & 256 (0x0100)          
0x007B (0x0000F6) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x007C (0x0000F8) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x007D (0x0000FA) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x007E (0x0000FC) 0x8607-       f:00103 d:   7 | P = P + 7 (0x0085), A # 0     
@ Issue command (RegA) 32, with RegC set to -200
0x007F (0x0000FE) 0x1020-       f:00010 d:  32 | A = 32 (0x0020)               
0x0080 (0x000100) 0x291C-       f:00024 d: 284 | OR[284] = A                   
0x0081 (0x000102) 0x10C8-       f:00010 d: 200 | A = 200 (0x00C8)              
0x0082 (0x000104) 0x291D-       f:00024 d: 285 | OR[285] = A                   
0x0083 (0x000106) 0x7447-       f:00072 d:  71 | R = P + 71 (0x00CA)           @ Start operation without updateing RegB
0x0084 (0x000108) 0x720B-       f:00071 d:  11 | P = P - 11 (0x0079)           
@ Issue command (RegA) 24, with RegC set to -1
0x0085 (0x00010A) 0x1018-       f:00010 d:  24 | A = 24 (0x0018)               
0x0086 (0x00010C) 0x291C-       f:00024 d: 284 | OR[284] = A                   
0x0087 (0x00010E) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x0088 (0x000110) 0x291D-       f:00024 d: 285 | OR[285] = A                   
0x0089 (0x000112) 0x7441-       f:00072 d:  65 | R = P + 65 (0x00CA)           @ Start operation without updateing RegB
0x008A (0x000114) 0x7031-       f:00070 d:  49 | P = P + 49 (0x00BB)           
@ -- switch
0x008B (0x000116) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x008C (0x000118) 0x1E00-0x0209 f:00017 d:   0 | A = A - 521 (0x0209)          
0x008E (0x00011C) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0090), A = 0     
0x008F (0x00011E) 0x7010-       f:00070 d:  16 | P = P + 16 (0x009F)           

@ Code 521 handler
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
0x0090 (0x000120) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0091 (0x000122) 0x291E-       f:00024 d: 286 | OR[286] = A                   
@ Repetitively issue command 24 with RegC = -200 until END OF FILE is returned
0x0092 (0x000124) 0x211E-       f:00020 d: 286 | A = OR[286]                   
0x0093 (0x000126) 0x1300-       f:00011 d: 256 | A = A & 256 (0x0100)          
0x0094 (0x000128) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0095 (0x00012A) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0096 (0x00012C) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x0097 (0x00012E) 0x8607-       f:00103 d:   7 | P = P + 7 (0x009E), A = 0     
@ Issue command (RegA) 24, with RegC set to -200
0x0098 (0x000130) 0x1018-       f:00010 d:  24 | A = 24 (0x0018)               
0x0099 (0x000132) 0x291C-       f:00024 d: 284 | OR[284] = A                   
0x009A (0x000134) 0x10C8-       f:00010 d: 200 | A = 200 (0x00C8)              
0x009B (0x000136) 0x291D-       f:00024 d: 285 | OR[285] = A                   
0x009C (0x000138) 0x742E-       f:00072 d:  46 | R = P + 46 (0x00CA)           @ Start operation without updateing RegB
0x009D (0x00013A) 0x720B-       f:00071 d:  11 | P = P - 11 (0x0092)           
0x009E (0x00013C) 0x701D-       f:00070 d:  29 | P = P + 29 (0x00BB)           
@ -- switch
0x009F (0x00013E) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x00A0 (0x000140) 0x1E00-0x0202 f:00017 d:   0 | A = A - 514 (0x0202)          
0x00A2 (0x000144) 0x8406-       f:00102 d:   6 | P = P + 6 (0x00A8), A = 0     
0x00A3 (0x000146) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x00A4 (0x000148) 0x1E00-0x0203 f:00017 d:   0 | A = A - 515 (0x0203)          
0x00A6 (0x00014C) 0x8402-       f:00102 d:   2 | P = P + 2 (0x00A8), A = 0     
0x00A7 (0x00014E) 0x7012-       f:00070 d:  18 | P = P + 18 (0x00B9)           

@ Code 514 and 515 handlers
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Read the number of bytes to be transfred to/from the tape from entry 22 in the device desc.
0x00A8 (0x000150) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x00A9 (0x000152) 0x1416-       f:00012 d:  22 | A = A + 22 (0x0016)           
0x00AA (0x000154) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00AB (0x000156) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x00AC (0x000158) 0x291D-       f:00024 d: 285 | OR[285] = A                   
0x00AD (0x00015A) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x00AE (0x00015C) 0x1E00-0x0203 f:00017 d:   0 | A = A - 515 (0x0203)          
0x00B0 (0x000160) 0x8402-       f:00102 d:   2 | P = P + 2 (0x00B2), A = 0     
0x00B1 (0x000162) 0x7004-       f:00070 d:   4 | P = P + 4 (0x00B5)            
@ Code 515: issue a write (40) operation
0x00B2 (0x000164) 0x1028-       f:00010 d:  40 | A = 40 (0x0028)               
0x00B3 (0x000166) 0x291C-       f:00024 d: 284 | OR[284] = A                   
0x00B4 (0x000168) 0x7003-       f:00070 d:   3 | P = P + 3 (0x00B7)            
@ Code 514: issue a read (0) operation
0x00B5 (0x00016A) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00B6 (0x00016C) 0x291C-       f:00024 d: 284 | OR[284] = A                   
0x00B7 (0x00016E) 0x740F-       f:00072 d:  15 | R = P + 15 (0x00C6)           @ Start operation with RegB set to 0x26B0 (buffer)
0x00B8 (0x000170) 0x7003-       f:00070 d:   3 | P = P + 3 (0x00BB)            
@ Switch default: fatal
0x00B9 (0x000172) 0x7C34-       f:00076 d:  52 | R = OR[52]                    
0x00BA (0x000174) 0x0000-       f:00000 d:   0 | PASS                          

@ common portion of command handler - set return value (from OR[282]) and return
0x00BB (0x000176) 0x2005-       f:00020 d:   5 | A = OR[5]                     
0x00BC (0x000178) 0x1406-       f:00012 d:   6 | A = A + 6 (0x0006)            
0x00BD (0x00017A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00BE (0x00017C) 0x211A-       f:00020 d: 282 | A = OR[282]                   
0x00BF (0x00017E) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
@ Return from overlay
0x00C0 (0x000180) 0x102A-       f:00010 d:  42 | A = 42 (0x002A)               
0x00C1 (0x000182) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x00C2 (0x000184) 0x1121-       f:00010 d: 289 | A = 289 (0x0121)              
0x00C3 (0x000186) 0x5800-       f:00054 d:   0 | B = A                         
0x00C4 (0x000188) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00C5 (0x00018A) 0x7C09-       f:00076 d:   9 | R = OR[9]                     

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Start operation.
@@ This routine can be called at different entry points, depending
@@ on which registers need to be set
@@ Input: OR[285]: negative value intended for RegC - RegC is the negative number of bytes to read from the tape it seems, so this makes sense
@@        OR[284]: intended RegA value
@@ Return:
@@        OR[286]: returned RegA value

@ Set the register values, send a START control and enable - device - interrupts
0x00C6 (0x00018C) 0x1800-0x26B0 f:00014 d:   0 | A = 9904 (0x26B0)             @ Location of the data buffer
0x00C8 (0x000190) 0xDA0F-       f:00155 d:  15 | io 15 (EXB), fn015             | Data output to B register (DOB)
0x00C9 (0x000192) 0x745B-       f:00072 d:  91 | R = P + 91 (0x0124)           
0x00CA (0x000194) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00CB (0x000196) 0x271D-       f:00023 d: 285 | A = A - OR[285]               
0x00CC (0x000198) 0xDC0F-       f:00156 d:  15 | io 0017 (EXB), fn016           | Data output to C register (DOC)
0x00CD (0x00019A) 0x7457-       f:00072 d:  87 | R = P + 87 (0x0124)           
0x00CE (0x00019C) 0x211C-       f:00020 d: 284 | A = OR[284]                   
0x00CF (0x00019E) 0xD80F-       f:00154 d:  15 | io 0017 (EXB), fn014           | Data output to A register (DOA)
0x00D0 (0x0001A0) 0x7454-       f:00072 d:  84 | R = P + 84 (0x0124)           
0x00D1 (0x0001A2) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x00D2 (0x0001A4) 0xDE0F-       f:00157 d:  15 | io 0017 (EXB), fn017           | Send control
0x00D3 (0x0001A6) 0x7451-       f:00072 d:  81 | R = P + 81 (0x0124)           
0x00D4 (0x0001A8) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x00D5 (0x0001AA) 0xCC0F-       f:00146 d:  15 | io 0017 (EXB), fn006           | Send interface mask (MSKO)
@ Store lower 8-bits of intended RegA in the upper byte of the device desc. entry 20
0x00D6 (0x0001AC) 0x211C-       f:00020 d: 284 | A = OR[284]                   
0x00D7 (0x0001AE) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x00D8 (0x0001B0) 0x291C-       f:00024 d: 284 | OR[284] = A                   
0x00D9 (0x0001B2) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x00DA (0x0001B4) 0x1414-       f:00012 d:  20 | A = A + 20 (0x0014)           
0x00DB (0x0001B6) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00DC (0x0001B8) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x00DD (0x0001BA) 0x0A09-       f:00005 d:   9 | A = A < 9 (0x0009)            
0x00DE (0x0001BC) 0x251C-       f:00022 d: 284 | A = A + OR[284]               
0x00DF (0x0001BE) 0x0C09-       f:00006 d:   9 | A = A >> 9 (0x0009)           
0x00E0 (0x0001C0) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
@ If we've issued a rewind (???) operation (code 8), we're done
0x00E1 (0x0001C2) 0x211C-       f:00020 d: 284 | A = OR[284]                   
0x00E2 (0x0001C4) 0x1608-       f:00013 d:   8 | A = A - 8 (0x0008)            
0x00E3 (0x0001C6) 0x8602-       f:00103 d:   2 | P = P + 2 (0x00E5), A # 0     
0x00E4 (0x0001C8) 0x0200-       f:00001 d:   0 | EXIT                          
@ So we haven't issued a rewind operation, let's continue...
0x00E5 (0x0001CA) 0x0400-       f:00002 d:   0 | I = 0                         
0x00E6 (0x0001CC) 0x0000-       f:00000 d:   0 | PASS                          
0x00E7 (0x0001CE) 0x1002-       f:00010 d:   2 | A = 2 (0x0002)                
0x00E8 (0x0001D0) 0xCE0F-       f:00147 d:  15 | io 0017 (EXB), fn007           | Set interrupt mode
@ Set bit 9 in entry 17: async operation pending
0x00E9 (0x0001D2) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x00EA (0x0001D4) 0x1411-       f:00012 d:  17 | A = A + 17 (0x0011)           
0x00EB (0x0001D6) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00EC (0x0001D8) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x00ED (0x0001DA) 0x0A09-       f:00005 d:   9 | A = A < 9 (0x0009)            
0x00EE (0x0001DC) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x00EF (0x0001DE) 0x0C09-       f:00006 d:   9 | A = A >> 9 (0x0009)           
0x00F0 (0x0001E0) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x00F1 (0x0001E2) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x00F2 (0x0001E4) 0x1409-       f:00012 d:   9 | A = A + 9 (0x0009)            
0x00F3 (0x0001E6) 0x2913-       f:00024 d: 275 | OR[275] = A                   
@ Kernel call 9: Wait for interrupt or timeout (200ms ?)
0x00F4 (0x0001E8) 0x1009-       f:00010 d:   9 | A = 9 (0x0009)                
0x00F5 (0x0001EA) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x00F6 (0x0001EC) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x00F7 (0x0001EE) 0x2922-       f:00024 d: 290 | OR[290] = A                   
0x00F8 (0x0001F0) 0x10C8-       f:00010 d: 200 | A = 200 (0x00C8)              
0x00F9 (0x0001F2) 0x2923-       f:00024 d: 291 | OR[291] = A                   
0x00FA (0x0001F4) 0x1121-       f:00010 d: 289 | A = 289 (0x0121)              
0x00FB (0x0001F6) 0x5800-       f:00054 d:   0 | B = A                         
0x00FC (0x0001F8) 0x1800-0x1318 f:00014 d:   0 | A = 4888 (0x1318)             
0x00FE (0x0001FC) 0x7C09-       f:00076 d:   9 | R = OR[9]                     
@ Read context entry 11: if it's 0, we've got an interrupt. If not, we've timed out waiting
0x00FF (0x0001FE) 0x2006-       f:00020 d:   6 | A = OR[6]                     
0x0100 (0x000200) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x0101 (0x000202) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0102 (0x000204) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0103 (0x000206) 0x291E-       f:00024 d: 286 | OR[286] = A                   
@ re-enable device interrupts
0x0104 (0x000208) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0105 (0x00020A) 0xCE0F-       f:00147 d:  15 | io 0017 (EXB), fn007           | Set interrupt mode
@ re-load device address from device desc.
0x0106 (0x00020C) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x0107 (0x00020E) 0xCA0F-       f:00145 d:  15 | io 0017 (EXB), fn005           | Load device address
@ Clear interrupt count
0x0108 (0x000210) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0109 (0x000212) 0x1411-       f:00012 d:  17 | A = A + 17 (0x0011)           
0x010A (0x000214) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x010B (0x000216) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x010C (0x000218) 0x1A00-0xFF00 f:00015 d:   0 | A = A & 65280 (0xFF00)        
0x010E (0x00021C) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
@ Did we time out waiting?
0x010F (0x00021E) 0x211E-       f:00020 d: 286 | A = OR[286]                   
0x0110 (0x000220) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0112), A # 0     
0x0111 (0x000222) 0x700D-       f:00070 d:  13 | P = P + 13 (0x011E)           
@ Timeout case: clear upper byte of offset 17 (no async operating pending) and report error
0x0112 (0x000224) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x0113 (0x000226) 0x1411-       f:00012 d:  17 | A = A + 17 (0x0011)           
0x0114 (0x000228) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0115 (0x00022A) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0116 (0x00022C) 0x0E01-       f:00007 d:   1 | A = A << 1 (0x0001)           
0x0117 (0x00022E) 0x0A08-       f:00005 d:   8 | A = A < 8 (0x0008)            
0x0118 (0x000230) 0x1400-       f:00012 d:   0 | A = A + 0 (0x0000)            
0x0119 (0x000232) 0x0C09-       f:00006 d:   9 | A = A >> 9 (0x0009)           
0x011A (0x000234) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x011B (0x000236) 0x1042-       f:00010 d:  66 | A = 66 (0x0042)               
0x011C (0x000238) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x011D (0x00023A) 0x7428-       f:00072 d:  40 | R = P + 40 (0x0145)           @ Error 66 - DEVICE TIMEOUT
@ No time-out: read RegA returned by operation into OR[286] and return
0x011E (0x00023C) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x011F (0x00023E) 0x140E-       f:00012 d:  14 | A = A + 14 (0x000E)           
0x0120 (0x000240) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0121 (0x000242) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0122 (0x000244) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x0123 (0x000246) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ This small routine waits on a delayed EXB opration to complete
0x0124 (0x000248) 0x1190-       f:00010 d: 400 | A = 400 (0x0190)              
0x0125 (0x00024A) 0x8405-       f:00102 d:   5 | P = P + 5 (0x012A), A = 0     
0x0126 (0x00024C) 0x420F-       f:00041 d:  15 | C = 1, io 0017 (EXB) = BZ       
0x0127 (0x00024E) 0x8003-       f:00100 d:   3 | P = P + 3 (0x012A), C = 0     
0x0128 (0x000250) 0x1601-       f:00013 d:   1 | A = A - 1 (0x0001)            
0x0129 (0x000252) 0x7204-       f:00071 d:   4 | P = P - 4 (0x0125)            
0x012A (0x000254) 0x8402-       f:00102 d:   2 | P = P + 2 (0x012C), A = 0     
0x012B (0x000256) 0x7004-       f:00070 d:   4 | P = P + 4 (0x012F)            
0x012C (0x000258) 0x1041-       f:00010 d:  65 | A = 65 (0x0041)               
0x012D (0x00025A) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x012E (0x00025C) 0x7417-       f:00072 d:  23 | R = P + 23 (0x0145)           @ Error 65 - CHANNEL TIMEOUT
0x012F (0x00025E) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Read RegA from the already selected device
0x0130 (0x000260) 0xC20F-       f:00141 d:  15 | io 0017 (EXB), fn001           | Data input from A register (DIA)
0x0131 (0x000262) 0x760D-       f:00073 d:  13 | R = P - 13 (0x0124)           
0x0132 (0x000264) 0xD00F-       f:00150 d:  15 | io 0017 (EXB), fn010           | Read data bus status
0x0133 (0x000266) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x0134 (0x000268) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ YIELD
0x0135 (0x00026A) 0x1002-       f:00010 d:   2 | A = 2 (0x0002)                
0x0136 (0x00026C) 0xCE0F-       f:00147 d:  15 | io 0017 (EXB), fn007           | Set interrupt mode
@ Kernel function PAUS - wait for 1ms (?)
0x0137 (0x00026E) 0x1007-       f:00010 d:   7 | A = 7 (0x0007)                
0x0138 (0x000270) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x0139 (0x000272) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x013A (0x000274) 0x2922-       f:00024 d: 290 | OR[290] = A                   
0x013B (0x000276) 0x1121-       f:00010 d: 289 | A = 289 (0x0121)              
0x013C (0x000278) 0x5800-       f:00054 d:   0 | B = A                         
0x013D (0x00027A) 0x1800-0x1318 f:00014 d:   0 | A = 4888 (0x1318)             
0x013F (0x00027E) 0x7C09-       f:00076 d:   9 | R = OR[9]                     
@ re-enable device interrupts
0x0140 (0x000280) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0141 (0x000282) 0xCE0F-       f:00147 d:  15 | io 0017 (EXB), fn007           | Set interrupt mode
@ re-set device address from device desc.
0x0142 (0x000284) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x0143 (0x000286) 0xCA0F-       f:00145 d:  15 | io 0017 (EXB), fn005           | Load device address
0x0144 (0x000288) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ This routine handles time-outs for waiting on delayed operations on the EXB
@ Calling XTAPEB
0x0145 (0x00028A) 0x1028-       f:00010 d:  40 | A = 40 (0x0028)               
0x0146 (0x00028C) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x0147 (0x00028E) 0x1800-0x00AF f:00014 d:   0 | A = 175 (0x00AF)              
0x0149 (0x000292) 0x2922-       f:00024 d: 290 | OR[290] = A                   
0x014A (0x000294) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x014B (0x000296) 0x2923-       f:00024 d: 291 | OR[291] = A                   
0x014C (0x000298) 0x211F-       f:00020 d: 287 | A = OR[287]                   
0x014D (0x00029A) 0x2924-       f:00024 d: 292 | OR[292] = A                   
0x014E (0x00029C) 0x1121-       f:00010 d: 289 | A = 289 (0x0121)              
0x014F (0x00029E) 0x5800-       f:00054 d:   0 | B = A                         
0x0150 (0x0002A0) 0x1800-0x1318 f:00014 d:   0 | A = 4888 (0x1318)             
0x0152 (0x0002A4) 0x7C09-       f:00076 d:   9 | R = OR[9]                     
0x0153 (0x0002A6) 0x291A-       f:00024 d: 282 | OR[282] = A                   
0x0154 (0x0002A8) 0x211A-       f:00020 d: 282 | A = OR[282]                   
0x0155 (0x0002AA) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0157), A # 0     
0x0156 (0x0002AC) 0x7002-       f:00070 d:   2 | P = P + 2 (0x0158)            
0x0157 (0x0002AE) 0x729C-       f:00071 d: 156 | P = P - 156 (0x00BB)          
@ re-enable device interrupts
0x0158 (0x0002B0) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0159 (0x0002B2) 0xCE0F-       f:00147 d:  15 | io 0017 (EXB), fn007           | Set interrupt mode
@ re-set device address from device desc.
0x015A (0x0002B4) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x015B (0x0002B6) 0xCA0F-       f:00145 d:  15 | io 0017 (EXB), fn005           | Load device address
0x015C (0x0002B8) 0x0200-       f:00001 d:   0 | EXIT                          

0x015D (0x0002BA) 0x0000-       f:00000 d:   0 | PASS                          
0x015E (0x0002BC) 0x0000-       f:00000 d:   0 | PASS                          
0x015F (0x0002BE) 0x0000-       f:00000 d:   0 | PASS                          
