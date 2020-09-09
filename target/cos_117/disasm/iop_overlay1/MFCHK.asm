@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Mainframe test code for testing functionality before booting
@@ ------------------------------------------------------------
@@ The code does the following
@@ - Setting all registers to all 1-s and all 0-s, and storing them in a buffer
@@   starting at 0x200
@@ - Sending a single-word packet to the IOP containing the CPUID
@@ - Receiving a single-word packet from the IOP (probably just a handshake,
@@   the value is never read
@@ - On a multi-processor system, kick off the same test on each CPU (most likely,
@@   that part of the code is disabled as code was compiled for a single CPU config)
@@ - Enter an infinite loop
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

@@@ Exchange packet
00000000  00 00 00 00 44 00 00 00  ....D... @ P = 11:p0                 A0 = 0
00000001  00 00 00 00 00 00 00 00  ........ @ IBA = 0x0                 A1 = 0
00000002  00 00 3F FF F1 00 00 00  ..?ÿñ... @ ILA = 0xfff0              A2 = 0
00000003  00 00 00 00 00 00 00 00  ........ @ XA = 0   VL = 0   F = 0   A3 = 0
00000004  00 00 00 00 00 00 00 00  ........ @ DBA = 0                   A4 = 0
00000005  00 00 3F FF F0 00 00 00  ..?ÿð... @ DLA = 0xfff0              A5 = 0
00000006  00 00 00 00 00 00 00 00  ........ @                           A6 = 0
00000007  00 00 00 00 00 00 00 00  ........ @                           A7 = 0
00000008  00 00 00 00 00 00 00 00  ........ @                           S0 = 0
00000009  00 00 00 00 00 00 00 00  ........ @                           S1 = 0
0000000a  00 00 00 00 00 00 00 00  ........ @                           S2 = 0
0000000b  00 00 00 00 00 00 00 00  ........ @                           S3 = 0
0000000c  00 00 00 00 00 00 00 00  ........ @                           S4 = 0
0000000d  00 00 00 00 00 00 00 00  ........ @                           S5 = 0
0000000e  00 00 00 00 00 00 00 00  ........ @                           S6 = 0
0000000f  00 00 00 00 00 00 00 00  ........ @                           S7 = 0

@ This is value 0x9
0x000010:p0 (0x000010:p0) 0000000         - ERR0                           | 
0x000010:p1 (0x000010:p1) 0000000         - ERR0                           | 
0x000010:p2 (0x000010:p2) 0000000         - ERR0                           | 
0x000010:p3 (0x000010:p3) 0000011         - ERR9                           | 

@@@ START OF CODE AFTER RESET VECTOR
@ We set up a loop of 10000 (0x2710) iterations. In the loop we keep clearing any pending channel interrupts
0x000011:p0 (0x000011:p0) 0020100:0023420 - A1 0x002710                    | 
0x000011:p2 (0x000011:p2) 0033200         - A2 CI                          | A2 = channel number with the highest priority active interrupt request
0x000011:p3 (0x000011:p3) 0030002         - A0 A2                          | 
0x000012:p0 (0x000012:p0) 0010000:0000113 - JAZ 0x000012:p3                | Jump if A0 == 0
0x000012:p2 (0x000012:p2) 0001220         - CI,A2                          | Clear interrupt and error flags for Channel A2. Clear device master-clear.
0x000012:p3 (0x000012:p3) 0031110         - A1 A1-1                        | 
0x000013:p0 (0x000013:p0) 0030001         - A0 A1                          | 
0x000013:p1 (0x000013:p1) 0011000:0000106 - JAN 0x000011:p2                | Jump if A0 != 0
@ Initialize all A and S registers to all 1-s, then store them in a memory location starting at 0x200
0x000013:p3 (0x000013:p3) 0001405         - CCI                            | Clear programmable clock interrupt request
0x000014:p0 (0x000014:p0) 0001407         - DCI                            | Disable programmable clock interrupt request
0x000014:p1 (0x000014:p1) 0120000:0000244 - S0 [0x0000A4,]                 | 
0x000014:p3 (0x000014:p3) 0120100:0000244 - S1 [0x0000A4,]                 | 
0x000015:p1 (0x000015:p1) 0120200:0000244 - S2 [0x0000A4,]                 | 
0x000015:p3 (0x000015:p3) 0120300:0000244 - S3 [0x0000A4,]                 | 
0x000016:p1 (0x000016:p1) 0120400:0000244 - S4 [0x0000A4,]                 | 
0x000016:p3 (0x000016:p3) 0120500:0000244 - S5 [0x0000A4,]                 | 
0x000017:p1 (0x000017:p1) 0120600:0000244 - S6 [0x0000A4,]                 | 
0x000017:p3 (0x000017:p3) 0120700:0000244 - S7 [0x0000A4,]                 | 
0x000018:p1 (0x000018:p1) 0100000:0000244 - A0 [0x0000A4,]                 | 
0x000018:p3 (0x000018:p3) 0100100:0000244 - A1 [0x0000A4,]                 | 
0x000019:p1 (0x000019:p1) 0100200:0000244 - A2 [0x0000A4,]                 | 
0x000019:p3 (0x000019:p3) 0100300:0000244 - A3 [0x0000A4,]                 | 
0x00001A:p1 (0x00001A:p1) 0100400:0000244 - A4 [0x0000A4,]                 | 
0x00001A:p3 (0x00001A:p3) 0100500:0000244 - A5 [0x0000A4,]                 | 
0x00001B:p1 (0x00001B:p1) 0100600:0000244 - A6 [0x0000A4,]                 | 
0x00001B:p3 (0x00001B:p3) 0100700:0000244 - A7 [0x0000A4,]                 | 
0x00001C:p1 (0x00001C:p1) 0130000:0001000 - [0x000200,] S0                 | 
0x00001C:p3 (0x00001C:p3) 0130100:0001001 - [0x000201,] S1                 | 
0x00001D:p1 (0x00001D:p1) 0130200:0001002 - [0x000202,] S2                 | 
0x00001D:p3 (0x00001D:p3) 0130300:0001003 - [0x000203,] S3                 | 
0x00001E:p1 (0x00001E:p1) 0130400:0001004 - [0x000204,] S4                 | 
0x00001E:p3 (0x00001E:p3) 0130500:0001005 - [0x000205,] S5                 | 
0x00001F:p1 (0x00001F:p1) 0130600:0001006 - [0x000206,] S6                 | 
0x00001F:p3 (0x00001F:p3) 0130700:0001007 - [0x000207,] S7                 | 
0x000020:p1 (0x000020:p1) 0110000:0001010 - [0x000208,] A0                 | 
0x000020:p3 (0x000020:p3) 0110100:0001011 - [0x000209,] A1                 | 
0x000021:p1 (0x000021:p1) 0110200:0001012 - [0x00020A,] A2                 | 
0x000021:p3 (0x000021:p3) 0110300:0001013 - [0x00020B,] A3                 | 
0x000022:p1 (0x000022:p1) 0110400:0001014 - [0x00020C,] A4                 | 
0x000022:p3 (0x000022:p3) 0110500:0001015 - [0x00020D,] A5                 | 
0x000023:p1 (0x000023:p1) 0110600:0001016 - [0x00020E,] A6                 | 
0x000023:p3 (0x000023:p3) 0110700:0001017 - [0x00020F,] A7                 | 
@ Zero all A and S registers, then store them in a memory location starting at 0x210
0x000024:p1 (0x000024:p1) 0043000         - S0 0                           | 
0x000024:p2 (0x000024:p2) 0043100         - S1 0                           | 
0x000024:p3 (0x000024:p3) 0043200         - S2 0                           | 
0x000025:p0 (0x000025:p0) 0043300         - S3 0                           | 
0x000025:p1 (0x000025:p1) 0043400         - S4 0                           | 
0x000025:p2 (0x000025:p2) 0043500         - S5 0                           | 
0x000025:p3 (0x000025:p3) 0043600         - S6 0                           | 
0x000026:p0 (0x000026:p0) 0043700         - S7 0                           | 
0x000026:p1 (0x000026:p1) 0022000         - A0 0x000000                    | 
0x000026:p2 (0x000026:p2) 0022100         - A1 0x000000                    | 
0x000026:p3 (0x000026:p3) 0022200         - A2 0x000000                    | 
0x000027:p0 (0x000027:p0) 0022300         - A3 0x000000                    | 
0x000027:p1 (0x000027:p1) 0022400         - A4 0x000000                    | 
0x000027:p2 (0x000027:p2) 0022500         - A5 0x000000                    | 
0x000027:p3 (0x000027:p3) 0022600         - A6 0x000000                    | 
0x000028:p0 (0x000028:p0) 0022700         - A7 0x000000                    | 
0x000028:p1 (0x000028:p1) 0130000:0001020 - [0x000210,] S0                 | 
0x000028:p3 (0x000028:p3) 0130100:0001021 - [0x000211,] S1                 | 
0x000029:p1 (0x000029:p1) 0130200:0001022 - [0x000212,] S2                 | 
0x000029:p3 (0x000029:p3) 0130300:0001023 - [0x000213,] S3                 | 
0x00002A:p1 (0x00002A:p1) 0130400:0001024 - [0x000214,] S4                 | 
0x00002A:p3 (0x00002A:p3) 0130500:0001025 - [0x000215,] S5                 | 
0x00002B:p1 (0x00002B:p1) 0130600:0001026 - [0x000216,] S6                 | 
0x00002B:p3 (0x00002B:p3) 0130700:0001027 - [0x000217,] S7                 | 
0x00002C:p1 (0x00002C:p1) 0110000:0001030 - [0x000218,] A0                 | 
0x00002C:p3 (0x00002C:p3) 0110100:0001031 - [0x000219,] A1                 | 
0x00002D:p1 (0x00002D:p1) 0110200:0001032 - [0x00021A,] A2                 | 
0x00002D:p3 (0x00002D:p3) 0110300:0001033 - [0x00021B,] A3                 | 
0x00002E:p1 (0x00002E:p1) 0110400:0001034 - [0x00021C,] A4                 | 
0x00002E:p3 (0x00002E:p3) 0110500:0001035 - [0x00021D,] A5                 | 
0x00002F:p1 (0x00002F:p1) 0110600:0001036 - [0x00021E,] A6                 | 
0x00002F:p3 (0x00002F:p3) 0110700:0001037 - [0x00021F,] A7                 | 
@ Init the B/T registers to all 1-s, and store them starting 0x220
0x000030:p1 (0x000030:p1) 0020600:0001040 - A6 0x000220                    | 
0x000030:p3 (0x000030:p3) 0007000:0000706 - R 0x000071:p2                  @ Set buffer to all 1's at 0xb5
0x000031:p1 (0x000031:p1) 0020000:0000265 - A0 0x0000B5                    | 
0x000031:p3 (0x000031:p3) 0020100:0000100 - A1 0x000040                    | 
0x000032:p1 (0x000032:p1) 0034100         - B0,A1 [,A0]                    | Transfer A1 words from memory at address A0 to B registers starting from B0
0x000032:p2 (0x000032:p2) 0036100         - T0,A1 [,A0]                    | Transfer A1 words from memory at address A0 to T registers starting from T0
0x000032:p3 (0x000032:p3) 0030006         - A0 A6                          | 
0x000033:p0 (0x000033:p0) 0035100         - [,A0] B0,A1                    | Transfer A1 words from B registers starting from B0 to memory at address A0
0x000033:p1 (0x000033:p1) 0030661         - A6 A6+A1                       | 
0x000033:p2 (0x000033:p2) 0030006         - A0 A6                          | 
0x000033:p3 (0x000033:p3) 0037100         - [,A0] T0,A1                    | Transfer A1 words from T registers starting from T0 to memory at address A0
@ Now, zero out T and B registers and store them after the previous values (0x2a0 if I'm not mistaken)
0x000034:p0 (0x000034:p0) 0030661         - A6 A6+A1                       | 
0x000034:p1 (0x000034:p1) 0007000:0000712 - R 0x000072:p2                  @ Zero buffer at 0xb5
0x000034:p3 (0x000034:p3) 0020000:0000265 - A0 0x0000B5                    | 
0x000035:p1 (0x000035:p1) 0020100:0000100 - A1 0x000040                    | 
0x000035:p3 (0x000035:p3) 0034100         - B0,A1 [,A0]                    | Transfer A1 words from memory at address A0 to B registers starting from B0
0x000036:p0 (0x000036:p0) 0036100         - T0,A1 [,A0]                    | Transfer A1 words from memory at address A0 to T registers starting from T0
0x000036:p1 (0x000036:p1) 0030006         - A0 A6                          | 
0x000036:p2 (0x000036:p2) 0035100         - [,A0] B0,A1                    | Transfer A1 words from B registers starting from B0 to memory at address A0
0x000036:p3 (0x000036:p3) 0030661         - A6 A6+A1                       | 
0x000037:p0 (0x000037:p0) 0030006         - A0 A6                          | 
0x000037:p1 (0x000037:p1) 0037100         - [,A0] T0,A1                    | Transfer A1 words from T registers starting from T0 to memory at address A0
0x000037:p2 (0x000037:p2) 0030661         - A6 A6+A1                       | 
@ Set and clear all bits of V registers and store them in the buffer of 0x2xx
0x000037:p3 (0x000037:p3) 0007000:0000706 - R 0x000071:p2                  @ Set buffer to all 1's at 0xb5
0x000038:p1 (0x000038:p1) 0007000:0000727 - R 0x000075:p3                  @ Load all V registers from buffer at 0xb5, and store them starting at A6. Returns updated A6
0x000038:p3 (0x000038:p3) 0007000:0000712 - R 0x000072:p2                  @ Zero buffer at 0xb5
0x000039:p1 (0x000039:p1) 0007000:0000727 - R 0x000075:p3                  @ Load all V registers from buffer at 0xb5, and store them starting at A6. Returns updated A6
@ Set and clear all bits of the vector mask register and store it in the buffer of 0x2xx
0x000039:p3 (0x000039:p3) 0120100:0000244 - S1 [0x0000A4,]                 | 
0x00003A:p1 (0x00003A:p1) 0003010         - VMS1                           | Set Vector Mask register to S1
0x00003A:p2 (0x00003A:p2) 0073100         - S1 VM                          | Read the vector-mask register
0x00003A:p3 (0x00003A:p3) 0136100:0000000 - [0x000000,A6] S1               | 
0x00003B:p1 (0x00003B:p1) 0030660         - A6 A6+1                        | 
0x00003B:p2 (0x00003B:p2) 0043100         - S1 0                           | 
0x00003B:p3 (0x00003B:p3) 0003010         - VMS1                           | Set Vector Mask register to S1
0x00003C:p0 (0x00003C:p0) 0073100         - S1 VM                          | Read the vector-mask register
0x00003C:p1 (0x00003C:p1) 0136100:0000000 - [0x000000,A6] S1               | 
0x00003C:p3 (0x00003C:p3) 0030660         - A6 A6+1                        | 
@ We test all clusters here in some wierd way (bunch of computed jumps, but no calls. Eventually we're ending up coming out at 0x43:p3
0x00003D:p0 (0x00003D:p0) 0020300:0000003 - A3 0x000003                    | 
0x00003D:p2 (0x00003D:p2) 0022400         - A4 0x000000                    | 
0x00003D:p3 (0x00003D:p3) 0030003         - A0 A3                          | 
0x00003E:p0 (0x00003E:p0) 0010000:0000417 - JAZ 0x000043:p3                | Jump if A0 == 0
0x00003E:p2 (0x00003E:p2) 0030104         - A1 A4                          | 
@ Test SM, SB, ST registers in all clusters
0x00003E:p3 (0x00003E:p3) 0101200:0000225 - A2 [0x000095,A1]               @ Address is 0x50:p0, a routine below, it cycles through all clusters and calls the test routing pointed to by A7
0x00003F:p1 (0x00003F:p1) 0020700:0000401 - A7 0x000101                    @ This is the address of the routine blow (0x40:p1) - It's called by the routine above to store all ST, SM, SB registers will all 1-s and all 0-s.
0x00003F:p3 (0x00003F:p3) 0025200         - B0 A2                          | 
0x000040:p0 (0x000040:p0) 0005000         - J B0                           | 

@ Set all ST, SM and SB registers to all 1-s and store them in buffer 0x2xx
0x000040:p1 (0x000040:p1) 0100100:0000244 - A1 [0x0000A4,]                 | 
0x000040:p3 (0x000040:p3) 0120100:0000244 - S1 [0x0000A4,]                 | 
0x000041:p1 (0x000041:p1) 0007000:0000775 - R 0x00007F:p1                  | 
@ Set all ST, SM and SB resiters to 0 and store them in buffer 0x2xx
0x000041:p3 (0x000041:p3) 0022100         - A1 0x000000                    | 
0x000042:p0 (0x000042:p0) 0043100         - S1 0                           | 
0x000042:p1 (0x000042:p1) 0007000:0000775 - R 0x00007F:p1                  | 
0x000042:p3 (0x000042:p3) 0031330         - A3 A3-1                        | 
0x000043:p0 (0x000043:p0) 0030440         - A4 A4+1                        | 
0x000043:p1 (0x000043:p1) 0006000:0000367 - J 0x00003D:p3                  | 

@ Done with all the clusters
0x000043:p3 (0x000043:p3) 0007000:0000647 - R 0x000069:p3                  @ send processor ID to IOP and wait for response
0x000044:p1 (0x000044:p1) 0073101         - S1 SR0                         @ Read processor ID from SR register
0x000044:p2 (0x000044:p2) 0051201         - S2 0!S1                        | S2 0 or S1
0x000044:p3 (0x000044:p3) 0054226         - S2 S2 < 22                     | 
0x000045:p0 (0x000045:p0) 0055202         - S2 S2 > 62                     | 
0x000045:p1 (0x000045:p1) 0023220         - A2 S2                          | 
0x000045:p2 (0x000045:p2) 0030502         - A5 A2                          | 
0x000045:p3 (0x000045:p3) 0030002         - A0 A2                          | 
0x000046:p0 (0x000046:p0) 0011000:0000471 - JAN 0x00004E:p1                @ If we're not CPU0, we're done: set the appropriate field in the buffer at 0x9e as well as set [0x9d] to one, and wait forever | Jump if A0 != 0
@ We only get here if we're CPU0. We probably test here for other (slave) CPUs to finish testing or something... But since this was compiled for a single-processor system, we're doing a complicated alway-jump below
0x000046:p2 (0x000046:p2) 0022500         - A5 0x000000                    | 
0x000046:p3 (0x000046:p3) 0020400:0000001 - A4 0x000001                    | 
0x000047:p1 (0x000047:p1) 0031440         - A4 A4-1                        | 
0x000047:p2 (0x000047:p2) 0031045         - A0 A4-A5                       | 
0x000047:p3 (0x000047:p3) 0010000:0000471 - JAZ 0x00004E:p1                @ We always jump here, that is we're done... | Jump if A0 == 0
0x000048:p1 (0x000048:p1) 0020100:0001000 - A1 0x000200                    | 
0x000048:p3 (0x000048:p3) 0020200:0005000 - A2 0x000A00                    | 
0x000049:p1 (0x000049:p1) 0040100:0000525 - S1 0x0000000000000155          | 
0x000049:p3 (0x000049:p3) 0131100:0000000 - [0x000000,A1] S1               | 
0x00004A:p1 (0x00004A:p1) 0030110         - A1 A1+1                        | 
0x00004A:p2 (0x00004A:p2) 0031220         - A2 A2-1                        | 
0x00004A:p3 (0x00004A:p3) 0030002         - A0 A2                          | 
0x00004B:p0 (0x00004B:p0) 0011000:0000447 - JAN 0x000049:p3                | Jump if A0 != 0
0x00004B:p2 (0x00004B:p2) 0105400:0000232 - A4 [0x00009A,A5]               | 
0x00004C:p0 (0x00004C:p0) 0020700:0000464 - A7 0x000134                    | 
0x00004C:p2 (0x00004C:p2) 0025400         - B0 A4                          | 
0x00004C:p3 (0x00004C:p3) 0005000         - J B0                           | 
0x00004D:p0 (0x00004D:p0) 0030550         - A5 A5+1                        | 
0x00004D:p1 (0x00004D:p1) 0007000:0000633 - R 0x000066:p3                  | 
0x00004D:p3 (0x00004D:p3) 0006000:0000433 - J 0x000046:p3                  | 
0x00004E:p1 (0x00004E:p1) 0042177         - S1 1                           | 
0x00004E:p2 (0x00004E:p2) 0130100:0000235 - [0x00009D,] S1                 | 
0x00004F:p0 (0x00004F:p0) 0135100:0000236 - [0x00009E,A5] S1               | 
0x00004F:p2 (0x00004F:p2) 0006000:0000476 - J 0x00004F:p2                  @ Infinite loop. We just wait here for another reset or something...


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ These small routines set the active cluster 1 through 5
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
0x000050:p0 (0x000050:p0) 0001413         - CLN 1                          | Set cluster number to 1
0x000050:p1 (0x000050:p1) 0025700         - B0 A7                          | 
0x000050:p2 (0x000050:p2) 0005000         - J B0                           | 
0x000050:p3 (0x000050:p3) 0001423         - CLN 2                          | Set cluster number to 2
0x000051:p0 (0x000051:p0) 0025700         - B0 A7                          | 
0x000051:p1 (0x000051:p1) 0005000         - J B0                           | 
0x000051:p2 (0x000051:p2) 0001433         - CLN 3                          | Set cluster number to 3
0x000051:p3 (0x000051:p3) 0025700         - B0 A7                          | 
0x000052:p0 (0x000052:p0) 0005000         - J B0                           | 
0x000052:p1 (0x000052:p1) 0001443         - CLN 4                          | Set cluster number to 4
0x000052:p2 (0x000052:p2) 0025700         - B0 A7                          | 
0x000052:p3 (0x000052:p3) 0005000         - J B0                           | 
0x000053:p0 (0x000053:p0) 0001453         - CLN 5                          | Set cluster number to 5
0x000053:p1 (0x000053:p1) 0025700         - B0 A7                          | 
0x000053:p2 (0x000053:p2) 0005000         - J B0                           | 


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Inter-processor handshare (not used)
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
0x000053:p3 (0x000053:p3) 0022612         - A6 0x00000A                    | 
0x000054:p0 (0x000054:p0) 0001402         - IP 0                           | Clear received interprocessor interrupt
0x000054:p1 (0x000054:p1) 0007000:0000574 - R 0x00005F:p0                  | 
0x000054:p3 (0x000054:p3) 0001411         - IP,1 1                         | Set interprocessor interrupt request of CPU1
0x000055:p0 (0x000055:p0) 0007000:0000614 - R 0x000063:p0                  | 
0x000055:p2 (0x000055:p2) 0031660         - A6 A6-1                        | 
0x000055:p3 (0x000055:p3) 0030006         - A0 A6                          | 
0x000056:p0 (0x000056:p0) 0010000:0000534 - JAZ 0x000057:p0                | Jump if A0 == 0
0x000056:p2 (0x000056:p2) 0015000:0000520 - JSN 0x000054:p0                | Jump if S0 != 0
0x000057:p0 (0x000057:p0) 0025700         - B0 A7                          | 
0x000057:p1 (0x000057:p1) 0005000         - J B0                           | 

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Inter-processor handshare (not used)
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
0x000057:p2 (0x000057:p2) 0022612         - A6 0x00000A                    | 
0x000057:p3 (0x000057:p3) 0001402         - IP 0                           | Clear received interprocessor interrupt
0x000058:p0 (0x000058:p0) 0007000:0000574 - R 0x00005F:p0                  | 
0x000058:p2 (0x000058:p2) 0001421         - IP,2 1                         | Set interprocessor interrupt request of CPU2
0x000058:p3 (0x000058:p3) 0007000:0000614 - R 0x000063:p0                  | 
0x000059:p1 (0x000059:p1) 0031660         - A6 A6-1                        | 
0x000059:p2 (0x000059:p2) 0030006         - A0 A6                          | 
0x000059:p3 (0x000059:p3) 0010000:0000553 - JAZ 0x00005A:p3                | Jump if A0 == 0
0x00005A:p1 (0x00005A:p1) 0015000:0000537 - JSN 0x000057:p3                | Jump if S0 != 0
0x00005A:p3 (0x00005A:p3) 0025700         - B0 A7                          | 
0x00005B:p0 (0x00005B:p0) 0005000         - J B0                           | 

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Inter-processor handshare (not used)
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
0x00005B:p1 (0x00005B:p1) 0022612         - A6 0x00000A                    | 
0x00005B:p2 (0x00005B:p2) 0001402         - IP 0                           | Clear received interprocessor interrupt
0x00005B:p3 (0x00005B:p3) 0007000:0000574 - R 0x00005F:p0                  | 
0x00005C:p1 (0x00005C:p1) 0001431         - IP,3 1                         | Set interprocessor interrupt request of CPU3
0x00005C:p2 (0x00005C:p2) 0007000:0000614 - R 0x000063:p0                  | 
0x00005D:p0 (0x00005D:p0) 0031660         - A6 A6-1                        | 
0x00005D:p1 (0x00005D:p1) 0030006         - A0 A6                          | 
0x00005D:p2 (0x00005D:p2) 0010000:0000572 - JAZ 0x00005E:p2                | Jump if A0 == 0
0x00005E:p0 (0x00005E:p0) 0015000:0000556 - JSN 0x00005B:p2                | Jump if S0 != 0
0x00005E:p2 (0x00005E:p2) 0025700         - B0 A7                          | 
0x00005E:p3 (0x00005E:p3) 0005000         - J B0                           | 

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
0x00005F:p0 (0x00005F:p0) 0022100         - A1 0x000000                    | 
0x00005F:p1 (0x00005F:p1) 0020200:0000245 - A2 0x0000A5                    | 
0x00005F:p3 (0x00005F:p3) 0020300:0000020 - A3 0x000010                    | 
0x000060:p1 (0x000060:p1) 0122100:0000000 - S1 [0x000000,A2]               | 
0x000060:p3 (0x000060:p3) 0131100:0000000 - [0x000000,A1] S1               | 
0x000061:p1 (0x000061:p1) 0030110         - A1 A1+1                        | 
0x000061:p2 (0x000061:p2) 0030220         - A2 A2+1                        | 
0x000061:p3 (0x000061:p3) 0031330         - A3 A3-1                        | 
0x000062:p0 (0x000062:p0) 0030003         - A0 A3                          | 
0x000062:p1 (0x000062:p1) 0011000:0000601 - JAN 0x000060:p1                | Jump if A0 != 0
0x000062:p3 (0x000062:p3) 0005000         - J B0                           | 

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
0x000063:p0 (0x000063:p0) 0020117:0041100 - A1 0x0F4240                    | 
0x000063:p2 (0x000063:p2) 0120100:0000235 - S1 [0x00009D,]                 | 
0x000064:p0 (0x000064:p0) 0051001         - S0 0!S1                        | S0 0 or S1
0x000064:p1 (0x000064:p1) 0015000:0000627 - JSN 0x000065:p3                | Jump if S0 != 0
0x000064:p3 (0x000064:p3) 0031110         - A1 A1-1                        | 
0x000065:p0 (0x000065:p0) 0030001         - A0 A1                          | 
0x000065:p1 (0x000065:p1) 0011000:0000616 - JAN 0x000063:p2                | Jump if A0 != 0
0x000065:p3 (0x000065:p3) 0043100         - S1 0                           | 
0x000066:p0 (0x000066:p0) 0130100:0000235 - [0x00009D,] S1                 | 
0x000066:p2 (0x000066:p2) 0005000         - J B0                           | 

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
0x000066:p3 (0x000066:p3) 0020117:0041100 - A1 0x0F4240                    | 
0x000067:p1 (0x000067:p1) 0125100:0000236 - S1 [0x00009E,A5]               | 
0x000067:p3 (0x000067:p3) 0051001         - S0 0!S1                        | S0 0 or S1
0x000068:p0 (0x000068:p0) 0015000:0000646 - JSN 0x000069:p2                | Jump if S0 != 0
0x000068:p2 (0x000068:p2) 0031110         - A1 A1-1                        | 
0x000068:p3 (0x000068:p3) 0030001         - A0 A1                          | 
0x000069:p0 (0x000069:p0) 0011000:0000635 - JAN 0x000067:p1                | Jump if A0 != 0
0x000069:p2 (0x000069:p2) 0005000         - J B0                           | 


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Send processor number to IOP and wait for response, which is put into 0xa3
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
0x000069:p3 (0x000069:p3) 0073101         - S1 SR0                         @ Read the processor number (PN field)
0x00006A:p0 (0x00006A:p0) 0043202         - S2 >2                          
0x00006A:p1 (0x00006A:p1) 0055252         - S2 S2 > 22                     
0x00006A:p2 (0x00006A:p2) 0044112         - S1 S1&S2                       
0x00006A:p3 (0x00006A:p3) 0055130         - S1 S1 > 40                     
0x00006B:p0 (0x00006B:p0) 0130100:0000242 - [0x0000A2,] S1                 @ Store processor number at 0xA2
0x00006B:p2 (0x00006B:p2) 0100100:0000020 - A1 [0x000010,]                 @ Read value from 0x10 (contains 9) - channel number (COA)
0x00006C:p0 (0x00006C:p0) 0020200:0000242 - A2 0x0000A2                    @ Send packet of one word (containing the processor number) over to the IOP
0x00006C:p2 (0x00006C:p2) 0030320         - A3 A2+1                        
0x00006C:p3 (0x00006C:p3) 0001113         - CL,A1 A3                       
0x00006D:p0 (0x00006D:p0) 0001012         - CA,A1 A2                       @ Set channel address to be the processor number
0x00006D:p1 (0x00006D:p1) 0033200         - A2 CI                          @ Wait until IOP takes all the data from the channel
0x00006D:p2 (0x00006D:p2) 0031021         - A0 A2-A1                       
0x00006D:p3 (0x00006D:p3) 0011000:0000665 - JAN 0x00006D:p1                | Jump if A0 != 0
0x00006E:p1 (0x00006E:p1) 0001210         - CI,A1                          | Clear interrupt and error flags for Channel A1. Clear device master-clear.
0x00006E:p2 (0x00006E:p2) 0031110         - A1 A1-1                        | 
0x00006E:p3 (0x00006E:p3) 0020200:0000243 - A2 0x0000A3                    @ Set up reception of a single word
0x00006F:p1 (0x00006F:p1) 0030320         - A3 A2+1                        | 
0x00006F:p2 (0x00006F:p2) 0001113         - CL,A1 A3                       | 
0x00006F:p3 (0x00006F:p3) 0001012         - CA,A1 A2                       | 
0x000070:p0 (0x000070:p0) 0033200         - A2 CI                          @ Wait until packet arrives from IOP
0x000070:p1 (0x000070:p1) 0031021         - A0 A2-A1                       | 
0x000070:p2 (0x000070:p2) 0011000:0000700 - JAN 0x000070:p0                | Jump if A0 != 0
0x000071:p0 (0x000071:p0) 0001210         - CI,A1                          | Clear interrupt and error flags for Channel A1. Clear device master-clear.
0x000071:p1 (0x000071:p1) 0005000         - J B0                           | 


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Set buffer to all 1's at 0xb5
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
0x000071:p2 (0x000071:p2) 0120100:0000244 - S1 [0x0000A4,]                 | 
0x000072:p0 (0x000072:p0) 0006000:0000713 - J 0x000072:p3                  | 
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Zero buffer at 0xb5
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
0x000072:p2 (0x000072:p2) 0043100         - S1 0                           | 
@ Common portion of above routines
0x000072:p3 (0x000072:p3) 0020100:0000265 - A1 0x0000B5                    | 
0x000073:p1 (0x000073:p1) 0020200:0000100 - A2 0x000040                    | 
0x000073:p3 (0x000073:p3) 0131100:0000000 - [0x000000,A1] S1               | 
0x000074:p1 (0x000074:p1) 0031220         - A2 A2-1                        | 
0x000074:p2 (0x000074:p2) 0030110         - A1 A1+1                        | 
0x000074:p3 (0x000074:p3) 0030002         - A0 A2                          | 
0x000075:p0 (0x000075:p0) 0011000:0000717 - JAN 0x000073:p3                | Jump if A0 != 0
0x000075:p2 (0x000075:p2) 0005000         - J B0                           | 


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Load all V registers from buffer at 0xb5, and store them starting at A6. Returns updated A6
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
0x000075:p3 (0x000075:p3) 0020000:0000265 - A0 0x0000B5                    | 
0x000076:p1 (0x000076:p1) 0020100:0000100 - A1 0x000040                    | 
0x000076:p3 (0x000076:p3) 0002001         - VL A1                          | Set vector Vector Length register
0x000077:p0 (0x000077:p0) 0176000         - V0 [,A0,1]                     | 
0x000077:p1 (0x000077:p1) 0176100         - V1 [,A0,1]                     | 
0x000077:p2 (0x000077:p2) 0176200         - V2 [,A0,1]                     | 
0x000077:p3 (0x000077:p3) 0176300         - V3 [,A0,1]                     | 
0x000078:p0 (0x000078:p0) 0176400         - V4 [,A0,1]                     | 
0x000078:p1 (0x000078:p1) 0176500         - V5 [,A0,1]                     | 
0x000078:p2 (0x000078:p2) 0176600         - V6 [,A0,1]                     | 
0x000078:p3 (0x000078:p3) 0176700         - V7 [,A0,1]                     | 
0x000079:p0 (0x000079:p0) 0030006         - A0 A6                          | 
0x000079:p1 (0x000079:p1) 0177000         - [,A0,1] V0                     | 
0x000079:p2 (0x000079:p2) 0030661         - A6 A6+A1                       | 
0x000079:p3 (0x000079:p3) 0030006         - A0 A6                          | 
0x00007A:p0 (0x00007A:p0) 0177010         - [,A0,1] V1                     | 
0x00007A:p1 (0x00007A:p1) 0030661         - A6 A6+A1                       | 
0x00007A:p2 (0x00007A:p2) 0030006         - A0 A6                          | 
0x00007A:p3 (0x00007A:p3) 0177020         - [,A0,1] V2                     | 
0x00007B:p0 (0x00007B:p0) 0030661         - A6 A6+A1                       | 
0x00007B:p1 (0x00007B:p1) 0030006         - A0 A6                          | 
0x00007B:p2 (0x00007B:p2) 0177030         - [,A0,1] V3                     | 
0x00007B:p3 (0x00007B:p3) 0030661         - A6 A6+A1                       | 
0x00007C:p0 (0x00007C:p0) 0030006         - A0 A6                          | 
0x00007C:p1 (0x00007C:p1) 0177040         - [,A0,1] V4                     | 
0x00007C:p2 (0x00007C:p2) 0030661         - A6 A6+A1                       | 
0x00007C:p3 (0x00007C:p3) 0030006         - A0 A6                          | 
0x00007D:p0 (0x00007D:p0) 0177050         - [,A0,1] V5                     | 
0x00007D:p1 (0x00007D:p1) 0030661         - A6 A6+A1                       | 
0x00007D:p2 (0x00007D:p2) 0030006         - A0 A6                          | 
0x00007D:p3 (0x00007D:p3) 0177060         - [,A0,1] V6                     | 
0x00007E:p0 (0x00007E:p0) 0030661         - A6 A6+A1                       | 
0x00007E:p1 (0x00007E:p1) 0030006         - A0 A6                          | 
0x00007E:p2 (0x00007E:p2) 0177070         - [,A0,1] V7                     | 
0x00007E:p3 (0x00007E:p3) 0030661         - A6 A6+A1                       | 
0x00007F:p0 (0x00007F:p0) 0005000         - J B0                           | 

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ Initialize all shared registers in the active cluster and store them
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
0x00007F:p1 (0x00007F:p1) 0027107         - SB0 A1                         | Load shared address register SB0 with A1
0x00007F:p2 (0x00007F:p2) 0027117         - SB1 A1                         | Load shared address register SB1 with A1
0x00007F:p3 (0x00007F:p3) 0027127         - SB2 A1                         | Load shared address register SB2 with A1
0x000080:p0 (0x000080:p0) 0027137         - SB3 A1                         | Load shared address register SB3 with A1
0x000080:p1 (0x000080:p1) 0027147         - SB4 A1                         | Load shared address register SB4 with A1
0x000080:p2 (0x000080:p2) 0027157         - SB5 A1                         | Load shared address register SB5 with A1
0x000080:p3 (0x000080:p3) 0027167         - SB6 A1                         | Load shared address register SB6 with A1
0x000081:p0 (0x000081:p0) 0027177         - SB7 A1                         | Load shared address register SB7 with A1
0x000081:p1 (0x000081:p1) 0073103         - ST0 S1                         | 
0x000081:p2 (0x000081:p2) 0073113         - ST1 S1                         | 
0x000081:p3 (0x000081:p3) 0073123         - ST2 S1                         | 
0x000082:p0 (0x000082:p0) 0073133         - ST3 S1                         | 
0x000082:p1 (0x000082:p1) 0073143         - ST4 S1                         | 
0x000082:p2 (0x000082:p2) 0073153         - ST5 S1                         | 
0x000082:p3 (0x000082:p3) 0073163         - ST6 S1                         | 
0x000083:p0 (0x000083:p0) 0073173         - ST7 S1                         | 
0x000083:p1 (0x000083:p1) 0073102         - SM S1                          | 
0x000083:p2 (0x000083:p2) 0026107         - A1 SB0                         | Load shared address register SB0 to A1
0x000083:p3 (0x000083:p3) 0116100:0000000 - [0x000000,A6] A1               | 
0x000084:p1 (0x000084:p1) 0030660         - A6 A6+1                        | 
0x000084:p2 (0x000084:p2) 0026117         - A1 SB1                         | Load shared address register SB1 to A1
0x000084:p3 (0x000084:p3) 0116100:0000000 - [0x000000,A6] A1               | 
0x000085:p1 (0x000085:p1) 0030660         - A6 A6+1                        | 
0x000085:p2 (0x000085:p2) 0026127         - A1 SB2                         | Load shared address register SB2 to A1
0x000085:p3 (0x000085:p3) 0116100:0000000 - [0x000000,A6] A1               | 
0x000086:p1 (0x000086:p1) 0030660         - A6 A6+1                        | 
0x000086:p2 (0x000086:p2) 0026137         - A1 SB3                         | Load shared address register SB3 to A1
0x000086:p3 (0x000086:p3) 0116100:0000000 - [0x000000,A6] A1               | 
0x000087:p1 (0x000087:p1) 0030660         - A6 A6+1                        | 
0x000087:p2 (0x000087:p2) 0026147         - A1 SB4                         | Load shared address register SB4 to A1
0x000087:p3 (0x000087:p3) 0116100:0000000 - [0x000000,A6] A1               | 
0x000088:p1 (0x000088:p1) 0030660         - A6 A6+1                        | 
0x000088:p2 (0x000088:p2) 0026157         - A1 SB5                         | Load shared address register SB5 to A1
0x000088:p3 (0x000088:p3) 0116100:0000000 - [0x000000,A6] A1               | 
0x000089:p1 (0x000089:p1) 0030660         - A6 A6+1                        | 
0x000089:p2 (0x000089:p2) 0026167         - A1 SB6                         | Load shared address register SB6 to A1
0x000089:p3 (0x000089:p3) 0116100:0000000 - [0x000000,A6] A1               | 
0x00008A:p1 (0x00008A:p1) 0030660         - A6 A6+1                        | 
0x00008A:p2 (0x00008A:p2) 0026177         - A1 SB7                         | Load shared address register SB7 to A1
0x00008A:p3 (0x00008A:p3) 0116100:0000000 - [0x000000,A6] A1               | 
0x00008B:p1 (0x00008B:p1) 0030660         - A6 A6+1                        | 
0x00008B:p2 (0x00008B:p2) 0072103         - S1 ST0                         | 
0x00008B:p3 (0x00008B:p3) 0136100:0000000 - [0x000000,A6] S1               | 
0x00008C:p1 (0x00008C:p1) 0030660         - A6 A6+1                        | 
0x00008C:p2 (0x00008C:p2) 0072113         - S1 ST1                         | 
0x00008C:p3 (0x00008C:p3) 0136100:0000000 - [0x000000,A6] S1               | 
0x00008D:p1 (0x00008D:p1) 0030660         - A6 A6+1                        | 
0x00008D:p2 (0x00008D:p2) 0072123         - S1 ST2                         | 
0x00008D:p3 (0x00008D:p3) 0136100:0000000 - [0x000000,A6] S1               | 
0x00008E:p1 (0x00008E:p1) 0030660         - A6 A6+1                        | 
0x00008E:p2 (0x00008E:p2) 0072133         - S1 ST3                         | 
0x00008E:p3 (0x00008E:p3) 0136100:0000000 - [0x000000,A6] S1               | 
0x00008F:p1 (0x00008F:p1) 0030660         - A6 A6+1                        | 
0x00008F:p2 (0x00008F:p2) 0072143         - S1 ST4                         | 
0x00008F:p3 (0x00008F:p3) 0136100:0000000 - [0x000000,A6] S1               | 
0x000090:p1 (0x000090:p1) 0030660         - A6 A6+1                        | 
0x000090:p2 (0x000090:p2) 0072153         - S1 ST5                         | 
0x000090:p3 (0x000090:p3) 0136100:0000000 - [0x000000,A6] S1               | 
0x000091:p1 (0x000091:p1) 0030660         - A6 A6+1                        | 
0x000091:p2 (0x000091:p2) 0072163         - S1 ST6                         | 
0x000091:p3 (0x000091:p3) 0136100:0000000 - [0x000000,A6] S1               | 
0x000092:p1 (0x000092:p1) 0030660         - A6 A6+1                        | 
0x000092:p2 (0x000092:p2) 0072173         - S1 ST7                         | 
0x000092:p3 (0x000092:p3) 0136100:0000000 - [0x000000,A6] S1               | 
0x000093:p1 (0x000093:p1) 0030660         - A6 A6+1                        | 
0x000093:p2 (0x000093:p2) 0072102         - S1 SM                          | Read the semaphores
0x000093:p3 (0x000093:p3) 0136100:0000000 - [0x000000,A6] S1               | 
0x000094:p1 (0x000094:p1) 0030660         - A6 A6+1                        | 
0x000094:p2 (0x000094:p2) 0005000         - J B0                           | 

0x000094:p3 (0x000094:p3) 0000000         - ERR0                           | 

@Cluster select able: one address for each of the clusters
0x000095:p0 (0x000095:p0) 0000000         - ERR0                           @ This is 0x0000000000000140, which is used as an address of a routine (so it's 0x50:p0)
0x000095:p1 (0x000095:p1) 0000000         - ERR0                           | 
0x000095:p2 (0x000095:p2) 0000000         - ERR0                           | 
0x000095:p3 (0x000095:p3) 0000500         - ERR320                         | 
0x000096:p0 (0x000096:p0) 0000000         - ERR0                           | 
0x000096:p1 (0x000096:p1) 0000000         - ERR0                           | 
0x000096:p2 (0x000096:p2) 0000000         - ERR0                           | 
0x000096:p3 (0x000096:p3) 0000503         - ERR323                         | 
0x000097:p0 (0x000097:p0) 0000000         - ERR0                           | 
0x000097:p1 (0x000097:p1) 0000000         - ERR0                           | 
0x000097:p2 (0x000097:p2) 0000000         - ERR0                           | 
0x000097:p3 (0x000097:p3) 0000506         - ERR326                         | 
0x000098:p0 (0x000098:p0) 0000000         - ERR0                           | 
0x000098:p1 (0x000098:p1) 0000000         - ERR0                           | 
0x000098:p2 (0x000098:p2) 0000000         - ERR0                           | 
0x000098:p3 (0x000098:p3) 0000511         - ERR329                         | 
0x000099:p0 (0x000099:p0) 0000000         - ERR0                           | 
0x000099:p1 (0x000099:p1) 0000000         - ERR0                           | 
0x000099:p2 (0x000099:p2) 0000000         - ERR0                           | 
0x000099:p3 (0x000099:p3) 0000514         - ERR332                         | 

0x00009A:p0 (0x00009A:p0) 0000000         - ERR0                           @ Points to 0x000053:p3
0x00009A:p1 (0x00009A:p1) 0000000         - ERR0                           | 
0x00009A:p2 (0x00009A:p2) 0000000         - ERR0                           | 
0x00009A:p3 (0x00009A:p3) 0000517         - ERR335                         | 
0x00009B:p0 (0x00009B:p0) 0000000         - ERR0                           | 
0x00009B:p1 (0x00009B:p1) 0000000         - ERR0                           | 
0x00009B:p2 (0x00009B:p2) 0000000         - ERR0                           | 
0x00009B:p3 (0x00009B:p3) 0000536         - ERR350                         | 
0x00009C:p0 (0x00009C:p0) 0000000         - ERR0                           | 
0x00009C:p1 (0x00009C:p1) 0000000         - ERR0                           | 
0x00009C:p2 (0x00009C:p2) 0000000         - ERR0                           | 
0x00009C:p3 (0x00009C:p3) 0000555         - ERR365                         | 

0x00009D:p0 (0x00009D:p0) 0000000         - ERR0                           | 
0x00009D:p1 (0x00009D:p1) 0000000         - ERR0                           | 
0x00009D:p2 (0x00009D:p2) 0000000         - ERR0                           | 
0x00009D:p3 (0x00009D:p3) 0000000         - ERR0                           | 
@ Probably an array of test-done indicators, one for each CPU
0x00009E:p0 (0x00009E:p0) 0000000         - ERR0                           | 
0x00009E:p1 (0x00009E:p1) 0000000         - ERR0                           | 
0x00009E:p2 (0x00009E:p2) 0000000         - ERR0                           | 
0x00009E:p3 (0x00009E:p3) 0000000         - ERR0                           | 
0x00009F:p0 (0x00009F:p0) 0000000         - ERR0                           | 
0x00009F:p1 (0x00009F:p1) 0000000         - ERR0                           | 
0x00009F:p2 (0x00009F:p2) 0000000         - ERR0                           | 
0x00009F:p3 (0x00009F:p3) 0000000         - ERR0                           | 
0x0000A0:p0 (0x0000A0:p0) 0000000         - ERR0                           | 
0x0000A0:p1 (0x0000A0:p1) 0000000         - ERR0                           | 
0x0000A0:p2 (0x0000A0:p2) 0000000         - ERR0                           | 
0x0000A0:p3 (0x0000A0:p3) 0000000         - ERR0                           | 
0x0000A1:p0 (0x0000A1:p0) 0000000         - ERR0                           | 
0x0000A1:p1 (0x0000A1:p1) 0000000         - ERR0                           | 
0x0000A1:p2 (0x0000A1:p2) 0000000         - ERR0                           | 
0x0000A1:p3 (0x0000A1:p3) 0000000         - ERR0                           | 

@ Outgoing packet: contains the processor number
0x0000A2:p0 (0x0000A2:p0) 0000000         - ERR0                           | 
0x0000A2:p1 (0x0000A2:p1) 0000000         - ERR0                           | 
0x0000A2:p2 (0x0000A2:p2) 0000000         - ERR0                           | 
0x0000A2:p3 (0x0000A2:p3) 0000000         - ERR0                           | 
@ Incomming packet:
0x0000A3:p0 (0x0000A3:p0) 0000000         - ERR0                           | 
0x0000A3:p1 (0x0000A3:p1) 0000000         - ERR0                           | 
0x0000A3:p2 (0x0000A3:p2) 0000000         - ERR0                           | 
0x0000A3:p3 (0x0000A3:p3) 0000000         - ERR0                           | 
@ Initial values for all S and A registers (0xffffffffffffffff)
0x0000A4:p0 (0x0000A4:p0) 0177777         - [,A0,A7] V7                    | 
0x0000A4:p1 (0x0000A4:p1) 0177777         - [,A0,A7] V7                    | 
0x0000A4:p2 (0x0000A4:p2) 0177777         - [,A0,A7] V7                    | 
0x0000A4:p3 (0x0000A4:p3) 0177777         - [,A0,A7] V7                    | 

0x0000A5:p0 (0x0000A5:p0) 0000000         - ERR0                           | 
0x0000A5:p1 (0x0000A5:p1) 0000000         - ERR0                           | 
0x0000A5:p2 (0x0000A5:p2) 0042000         - S0 -1                          | 
0x0000A5:p3 (0x0000A5:p3) 0000000         - ERR0                           | 
0x0000A6:p0 (0x0000A6:p0) 0000000         - ERR0                           | 
0x0000A6:p1 (0x0000A6:p1) 0000000         - ERR0                           | 
0x0000A6:p2 (0x0000A6:p2) 0000000         - ERR0                           | 
0x0000A6:p3 (0x0000A6:p3) 0000000         - ERR0                           | 
0x0000A7:p0 (0x0000A7:p0) 0000000         - ERR0                           | 
0x0000A7:p1 (0x0000A7:p1) 0037777         - [,A0] T63,A7                   | Transfer A7 words from T registers starting from T63 to memory at address A0
0x0000A7:p2 (0x0000A7:p2) 0170400         - V4 FV0                         | Vector floating-point add
0x0000A7:p3 (0x0000A7:p3) 0000000         - ERR0                           | 
0x0000A8:p0 (0x0000A8:p0) 0000000         - ERR0                           | 
0x0000A8:p1 (0x0000A8:p1) 0000000         - ERR0                           | 
0x0000A8:p2 (0x0000A8:p2) 0000000         - ERR0                           | 
0x0000A8:p3 (0x0000A8:p3) 0000000         - ERR0                           | 
0x0000A9:p0 (0x0000A9:p0) 0000000         - ERR0                           | 
0x0000A9:p1 (0x0000A9:p1) 0000000         - ERR0                           | 
0x0000A9:p2 (0x0000A9:p2) 0000000         - ERR0                           | 
0x0000A9:p3 (0x0000A9:p3) 0000000         - ERR0                           | 
0x0000AA:p0 (0x0000AA:p0) 0000000         - ERR0                           | 
0x0000AA:p1 (0x0000AA:p1) 0037777         - [,A0] T63,A7                   | Transfer A7 words from T registers starting from T63 to memory at address A0
0x0000AA:p2 (0x0000AA:p2) 0170000         - V0 FV0                         | Vector floating-point add
0x0000AA:p3 (0x0000AA:p3) 0000000         - ERR0                           | 
0x0000AB:p0 (0x0000AB:p0) 0000000         - ERR0                           | 
0x0000AB:p1 (0x0000AB:p1) 0000000         - ERR0                           | 
0x0000AB:p2 (0x0000AB:p2) 0000000         - ERR0                           | 
0x0000AB:p3 (0x0000AB:p3) 0000000         - ERR0                           | 
0x0000AC:p0 (0x0000AC:p0) 0000000         - ERR0                           | 
0x0000AC:p1 (0x0000AC:p1) 0000000         - ERR0                           | 
0x0000AC:p2 (0x0000AC:p2) 0000000         - ERR0                           | 
0x0000AC:p3 (0x0000AC:p3) 0000000         - ERR0                           | 
0x0000AD:p0 (0x0000AD:p0) 0000000         - ERR0                           | 
0x0000AD:p1 (0x0000AD:p1) 0000000         - ERR0                           | 
0x0000AD:p2 (0x0000AD:p2) 0000000         - ERR0                           | 
0x0000AD:p3 (0x0000AD:p3) 0000000         - ERR0                           | 
0x0000AE:p0 (0x0000AE:p0) 0000000         - ERR0                           | 
0x0000AE:p1 (0x0000AE:p1) 0000000         - ERR0                           | 
0x0000AE:p2 (0x0000AE:p2) 0000000         - ERR0                           | 
0x0000AE:p3 (0x0000AE:p3) 0000000         - ERR0                           | 
0x0000AF:p0 (0x0000AF:p0) 0000000         - ERR0                           | 
0x0000AF:p1 (0x0000AF:p1) 0000000         - ERR0                           | 
0x0000AF:p2 (0x0000AF:p2) 0000000         - ERR0                           | 
0x0000AF:p3 (0x0000AF:p3) 0000000         - ERR0                           | 
0x0000B0:p0 (0x0000B0:p0) 0000000         - ERR0                           | 
0x0000B0:p1 (0x0000B0:p1) 0000000         - ERR0                           | 
0x0000B0:p2 (0x0000B0:p2) 0000000         - ERR0                           | 
0x0000B0:p3 (0x0000B0:p3) 0000000         - ERR0                           | 
0x0000B1:p0 (0x0000B1:p0) 0000000         - ERR0                           | 
0x0000B1:p1 (0x0000B1:p1) 0000000         - ERR0                           | 
0x0000B1:p2 (0x0000B1:p2) 0000000         - ERR0                           | 
0x0000B1:p3 (0x0000B1:p3) 0000000         - ERR0                           | 
0x0000B2:p0 (0x0000B2:p0) 0000000         - ERR0                           | 
0x0000B2:p1 (0x0000B2:p1) 0000000         - ERR0                           | 
0x0000B2:p2 (0x0000B2:p2) 0000000         - ERR0                           | 
0x0000B2:p3 (0x0000B2:p3) 0000000         - ERR0                           | 
0x0000B3:p0 (0x0000B3:p0) 0000000         - ERR0                           | 
0x0000B3:p1 (0x0000B3:p1) 0000000         - ERR0                           | 
0x0000B3:p2 (0x0000B3:p2) 0000000         - ERR0                           | 
0x0000B3:p3 (0x0000B3:p3) 0000000         - ERR0                           | 
0x0000B4:p0 (0x0000B4:p0) 0000000         - ERR0                           | 
0x0000B4:p1 (0x0000B4:p1) 0000000         - ERR0                           | 
0x0000B4:p2 (0x0000B4:p2) 0000000         - ERR0                           | 
0x0000B4:p3 (0x0000B4:p3) 0000000         - ERR0                           | 
@ We have a buffer here of 0x40 QWORDS, used to set B,T,V registers
0x0000B5:p0 (0x0000B5:p0) 0000000         - ERR0                           | 
0x0000B5:p1 (0x0000B5:p1) 0000000         - ERR0                           | 
0x0000B5:p2 (0x0000B5:p2) 0000000         - ERR0                           | 
0x0000B5:p3 (0x0000B5:p3) 0000000         - ERR0                           | 
0x0000B6:p0 (0x0000B6:p0) 0000000         - ERR0                           | 
0x0000B6:p1 (0x0000B6:p1) 0000000         - ERR0                           | 
0x0000B6:p2 (0x0000B6:p2) 0000000         - ERR0                           | 
0x0000B6:p3 (0x0000B6:p3) 0000000         - ERR0                           | 
