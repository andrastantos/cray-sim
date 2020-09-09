@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@ Block multiplexer daemon
@@@ Load address 0x26d0 (0x26d6) among others
@@@              0x2BBC
@@@              0x26F8 (0x26fe)
@@@
@ First workitem:
@ Ofs value
@  1   0x5B4C --> ofs 3: 0x5B58 --> 0x0300
@  2   0x0001
@  3   0x4010
@  9   0x0000
@ 10   0x0000
@ 12   0x0000
@ Second workitem:
@  1   0x5B4C --> ofs 3: 0x5B58 --> 0x0300
@  2   0x0003
@  3   0x4010
@  9   0x0000
@ 10   0x0030
@ 12   0x0000

@ Get next entry from XCIQ list. If there's nothing left in the queue, return to idle loop.
0x0000 (0x000000) 0x0400-       f:00002 d:   0 | I = 0                         
0x0001 (0x000002) 0x0000-       f:00000 d:   0 | PASS                          
0x0002 (0x000004) 0x1800-0x0E6C f:00014 d:   0 | A = 3692 (0x0E6C)             @ Address that is used by the routine at 0x1ffd
0x0004 (0x000008) 0x289D-       f:00024 d: 157 | OR[157] = A                   
0x0005 (0x00000A) 0x7E00-0x1FFD f:00077 d:   0 | R = OR[0]+8189 (0x1FFD)       @ Remove curent item from linked list. Returns OR[156]. This routine is called HGET
0x0007 (0x00000E) 0x209C-       f:00020 d: 156 | A = OR[156]                   
0x0008 (0x000010) 0x292B-       f:00024 d: 299 | OR[299] = A                   
0x0009 (0x000012) 0x8402-       f:00102 d:   2 | P = P + 2 (0x000B), A = 0     @ No next item --> jump
0x000A (0x000014) 0x7003-       f:00070 d:   3 | P = P + 3 (0x000D)            
0x000B (0x000016) 0x7A00-0x1A40 f:00075 d:   0 | P = OR[0]+6720 (0x1A40)       @ We get here if there's no more work to do. This routine is called ESWP

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ XCIQ (channel queue request) pointer at 0x0E6C
@ OR[299] pointer to CHT
@   1: Pointer to CUT (in OR[304])
@          3: pointer to BDV (in OR[305])
@                 7: pointer to CPB (in OR[308])
@                       12/14: pointer to first CPW (in OR[312])
@          5: activity queue pointer
@   2: in OR[300]                -- some sort of action or state code
@   3: lower byte in OR[280]     -- contains channel address
@   9: in OR[301]                -- contains input tags
@  10: in OR[302]                -- contains the status response
@  12: upper byte in OR[303]
@
@ We have quite a few tables used here.
@ Since the HW configuration is: channel - controller - device, we have a table for each:
@    CHT - channel table
@    CUT - control-unit table
@    BDV - device table
@ Normally, multiple devices are attached to a single controller, and it's possible that a single device is attached to multiple controllers.
@ This is acheieved through 'banks', and is described in another two tables:
@    CBT - control-unit bank table
@    DBT - device bank table
@ There are also a set of global pointers to point to lists (arrays) of these tables
@   XCHT - 0x0E67 CHT table array
@   XDEV - 0x0E68 BDV table array
@   XCBT - CBT table array
@   XDBT - DBT table array
@   XCIQ - 0x0E6C CHT table pointer list for BMXDEM to pick up request from
@
@ Tables in use:
@ CPW: a 4-parcel struct describing one command to a device. in OR[312]
@      these are in an array called CPW list.
@     Offset 0, bit 0-7 - CPW@CM - channel command
@     Offset 0, bit 8   - CPW@DN - CPW done (must be cleared on call, set when processing is done for this CPW) (?)
@     Offset 0, bit 9   -
@     Offset 0, bit 10  -
@     Offset 0, bit 11  -
@     Offset 0, bit 12  - in OR[316] CPW@MS - 1 to indicate data transfer request
@     Offset 0, bit 13  - in OR[315] CPW@IN - 1 for sense (read) 0 for command (write)
@     Offset 0, bit 14  - in OR[314] CPW@DT - 1 if MOS transfer is requested (0 for command/sense type commands, 1 for data read/writes)
@     Offset 0, bit 15  - in OR[313] CPW@CC - 1 for command chaining  (that is, not the last one in the list)
@     Offset 1          - CPW@DA - data address for the buffer
@     Offset 2          - CPW@BU - upper word for byte-count (not used for IO memory transfers)
@     Offset 3          - CPW@BL - lower word for byte-count to transfer
@ Data transfers (MS = 1) also have special requirements in the CPB table, namely the buffer memory address is comming from it.
@ CPB: Command Parameter Block in OR[308]
@     CPB@@LE - size of the CPB (the CPB is part of the 'Tape Control Block'
@     Offset  8, lower  - CPB@OS - operation status
@     Offset  9         - CPB@B0 - first IO memory buffer address
@     Offset 10         - CPB@B1 - second IO memory buffer address
@     Offset 11, bit 15 - CPB@BP?? - determines which of the two buffers is active (first?) in OR[309]
@     Offset 12         - CPB@CB - points to the first element of the CPW list
@     Offset 13         - CPB@CE - points to the last element of the CPW list
@     Offset 14         - CPB@CC - points to the currently active CPW (? maybe offset 30, but not likely)
@     Offset 15         - ?????? - number of processed CPWs
@     Offset 20         - Set to 0 at the start of a command
@     Offset 27         - ?????? - some error counter, maybe?
@     Offset 28, upper  - Previous command
@     Offset 28, lower  - Current command
@     Offset 29         - pointer to something?
@     Offset 30         - same as CPB@CC in some cases - currently active CPW.
@     CPB@DE - points to (?) the DSC buffer descriptor for the first sector of the first block (for MOS data transfers only)
@     CPB@DU \ DSCs buffer memory address
@     CPB@DL /
@     CPB@LI - DSC buffer memory size
@     CPB@PT - DSC buffer descriptor entry offset
@     CPB@BP - pointer to the first active write IO memory buffer (either CPB@B0 or CPB@B1)
@     CPB@0R - first IO memory buffer ready (that is it contains valid data for writes at least)
@     CPB@1R - second IO memory buffer ready (that is it contains valid data for writes at least)
@     CPB@MU \ maximum block size (for reads)
@     CPB@ML /
@     CPB@PR - must address an initialized PRW (?)
@     CPB@EC - some kind of error code. Could contain the value ES$DTO for device timeouts
@ BDV: in OR[305]
@     Offset 0, bit 14,15- BDV@CM - Channel mode (bits 14-15 for BMX function 16)
@     Offset 1           - BDV@UN - physical path, that is channel number and device address
@     Offset 3           - BDV@CB - pointer to CBT table
@     Offset 7           - BDV@CP - pointer to CPB
@     Offset 4, lower    - BDV@DS - device status (this seems to be the HW status response code from the device)
@     Offset 5, bit 15-12- Status code
@     Offset 8           - BDV@TQ - task queue - the activity that is waiting for the particular device activity (I/O) to complete (awaiting request-in interrupt form this device)
@     Offset 10???       - BDV@LC - pointer to a pointer to the last used CUT table (that is the control unit last used to talk to this device)
@     BDV@NR             - 1 device is not ready, 0 if device ready (set if waiting for an interrupt after manual intervention, like tape mount)
@     BDV@RS             - 1 - restart needed (set when device busy status is returned)
@     BDV@WE             - 1 - waiting device end (set when device busy status is returned)
@     BDV@RI             - 1 - waiting request-in interupt
@ CUT: in OR[304]
@     Offset 3           - CUT@CO - BDV address for the assigned device, or 0 if free
@     CUT@FL             - 1 if control unit is not available, 0 if available
@     offset 2           - CUT@CN - channel number of the control unit
@     CUT@BZ             - control unit busy (set if status response indicates control unit busy, cleared through async (request-in) interrupt from control unit)
@     CUT@CA
@     CUT@DB             - pointer to first DBT in assigned DBT array (that is the list of devices in the bank handled by the control unit)
@ CBT
@     Offset 0           - CBT@QU - activity queue waiting for the control unit bank (set of control units accessing the same set of devices) to free up
@ CHT: in OR[299]
@     Offset 1           - CHT@CO - pointer to assigned CUT, or 0 if available
@     Offset 2           - CHT@NP - sequence code in OR[300]
@                                       0 - inactive????
@                                       1 - KIC$SC - start command sequence,    - this is the first sequence code (it seems), used by BMXDEM
@                                       2 - KIC$AC - advance command sequence,  - used by BMXDEM
@                                       3 - KIC$AD - advance data sequence,     - used by BMXDEM and interrupt handler
@                                       4 - KIC$ER - request-in sequence        - used by BMXDEM and interrupt handler
@                                       5 - KIC$IR - immediate return           - used by interrupt handler
@                                       6 - KIC$SR - start request-in           - used by interrupt handler
@                                       7 - KIC$CR - continue request-in        - used by interrupt handler
@     Offset 3, lower             - channel address, in OR[280]
@     Offset 4           - CHT@QU - activity queue waiting for the channel
@     Offset 8????????   - CHT@SC - pointer to a pointer to CUT table (how is that different from CHT@CO??) - this is used for request-in interrupts apparently
@     Offset 9                    - input tags in OR[301]
@     Offset 10, lower   - CHT@DS - HW device status (filled in by the interrupt handler) in OR[302]
@     Offset 10, upper   - CHT@RA - interrupting device address for request-in interrupts (filled in by the interrupt handler) in OR[302]
@ OR[312] points to the currently processed CPW.
@ I think OR[312] points to the currently processd CPW, a 4-parcel struct describing one BMX command

0x000D (0x00001A) 0x0600-       f:00003 d:   0 | I = 1                         
0x000E (0x00001C) 0x212B-       f:00020 d: 299 | A = OR[299]                   @ Load CHT@NP (sequence code) into OR[300]
0x000F (0x00001E) 0x1402-       f:00012 d:   2 | A = A + 2 (0x0002)            
0x0010 (0x000020) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0011 (0x000022) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0012 (0x000024) 0x292C-       f:00024 d: 300 | OR[300] = A                   
0x0013 (0x000026) 0x212B-       f:00020 d: 299 | A = OR[299]                   @ Load CHT@3 (channel address) into OR[280]
0x0014 (0x000028) 0x1403-       f:00012 d:   3 | A = A + 3 (0x0003)            
0x0015 (0x00002A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0016 (0x00002C) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0017 (0x00002E) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x0018 (0x000030) 0x2918-       f:00024 d: 280 | OR[280] = A                   
0x0019 (0x000032) 0x212B-       f:00020 d: 299 | A = OR[299]                   @ Load CHT@9 (input tags) into OR[301]
0x001A (0x000034) 0x1409-       f:00012 d:   9 | A = A + 9 (0x0009)            
0x001B (0x000036) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x001C (0x000038) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x001D (0x00003A) 0x292D-       f:00024 d: 301 | OR[301] = A                   
0x001E (0x00003C) 0x212B-       f:00020 d: 299 | A = OR[299]                   @ Load CHT@12, upper into OR[303]
0x001F (0x00003E) 0x140C-       f:00012 d:  12 | A = A + 12 (0x000C)           
0x0020 (0x000040) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0021 (0x000042) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0022 (0x000044) 0x0808-       f:00004 d:   8 | A = A > 8 (0x0008)            
0x0023 (0x000046) 0x292F-       f:00024 d: 303 | OR[303] = A                   
0x0024 (0x000048) 0x212B-       f:00020 d: 299 | A = OR[299]                   @ Load CHT@DS & CHT&RA into OR[302]
0x0025 (0x00004A) 0x140A-       f:00012 d:  10 | A = A + 10 (0x000A)           
0x0026 (0x00004C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0027 (0x00004E) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0028 (0x000050) 0x292E-       f:00024 d: 302 | OR[302] = A                   
0x0029 (0x000052) 0x212B-       f:00020 d: 299 | A = OR[299]                   @ Load CHT@CO (CUT pointer) into OR[304]
0x002A (0x000054) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x002B (0x000056) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x002C (0x000058) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x002D (0x00005A) 0x2930-       f:00024 d: 304 | OR[304] = A                   
0x002E (0x00005C) 0x2130-       f:00020 d: 304 | A = OR[304]                   @ Load CUT@CO (BDV pointer) into OR[305]
0x002F (0x00005E) 0x1403-       f:00012 d:   3 | A = A + 3 (0x0003)            
0x0030 (0x000060) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0031 (0x000062) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0032 (0x000064) 0x2931-       f:00024 d: 305 | OR[305] = A                   
0x0033 (0x000066) 0x3131-       f:00030 d: 305 | A = (OR[305])                 @ Load BDV@0 into OR[275] apparently only for logging, probably device address
0x0034 (0x000068) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x0035 (0x00006A) 0x2913-       f:00024 d: 275 | OR[275] = A                   

0x0036 (0x00006C) 0x0400-       f:00002 d:   0 | I = 0                         @ We do some logging here
0x0037 (0x00006E) 0x0000-       f:00000 d:   0 | PASS                          
0x0038 (0x000070) 0x1010-       f:00010 d:  16 | A = 16 (0x0010)               
0x0039 (0x000072) 0x29C3-       f:00024 d: 451 | OR[451] = A                   
0x003A (0x000074) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x003B (0x000076) 0x29C4-       f:00024 d: 452 | OR[452] = A                   
0x003C (0x000078) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x003D (0x00007A) 0x29C5-       f:00024 d: 453 | OR[453] = A                   
0x003E (0x00007C) 0x212D-       f:00020 d: 301 | A = OR[301]                   
0x003F (0x00007E) 0x29C6-       f:00024 d: 454 | OR[454] = A                   
0x0040 (0x000080) 0x212E-       f:00020 d: 302 | A = OR[302]                   
0x0041 (0x000082) 0x29C7-       f:00024 d: 455 | OR[455] = A                   
0x0042 (0x000084) 0x212C-       f:00020 d: 300 | A = OR[300]                   
0x0043 (0x000086) 0x29C8-       f:00024 d: 456 | OR[456] = A                   
0x0044 (0x000088) 0x7DC2-       f:00076 d: 450 | R = OR[450]                   @ Points to 0x206A
0x0045 (0x00008A) 0x0600-       f:00003 d:   0 | I = 1                         

0x0046 (0x00008C) 0x212C-       f:00020 d: 300 | A = OR[300]                   @ Test if CHT@NP (sequence code) is 1 or 2
0x0047 (0x00008E) 0x1601-       f:00013 d:   1 | A = A - 1 (0x0001)            
0x0048 (0x000090) 0x8406-       f:00102 d:   6 | P = P + 6 (0x004E), A = 0     
0x0049 (0x000092) 0x212C-       f:00020 d: 300 | A = OR[300]                   
0x004A (0x000094) 0x1602-       f:00013 d:   2 | A = A - 2 (0x0002)            
0x004B (0x000096) 0x8403-       f:00102 d:   3 | P = P + 3 (0x004E), A = 0     
0x004C (0x000098) 0x7A03-0x02BD f:00075 d:   3 | P = OR[3]+701 (0x02BD)        @ CHT@NP (sequence code) is neither 1 or 2
@ CHT@NP (sequence code) is 1 or 2
0x004E (0x00009C) 0x7E03-0x0728 f:00077 d:   3 | R = OR[3]+1832 (0x0728)       @ --> call 0x0722 Parse and update BDV
0x0050 (0x0000A0) 0x7E03-0x0762 f:00077 d:   3 | R = OR[3]+1890 (0x0762)       @ --> call 0x075C Parse CPW
0x0052 (0x0000A4) 0x212C-       f:00020 d: 300 | A = OR[300]                   @ Test if CHT@NP (sequence code) is 2
0x0053 (0x0000A6) 0x1602-       f:00013 d:   2 | A = A - 2 (0x0002)            
0x0054 (0x0000A8) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0056), A = 0     
0x0055 (0x0000AA) 0x705F-       f:00070 d:  95 | P = P + 95 (0x00B4)           
@ CHT@NP (sequence code) is 2
0x0056 (0x0000AC) 0x7E03-0x0516 f:00077 d:   3 | R = OR[3]+1302 (0x0516)       @ --> call 0x0510
0x0058 (0x0000B0) 0x2123-       f:00020 d: 291 | A = OR[291]                   
0x0059 (0x0000B2) 0x8E59-       f:00107 d:  89 | P = P - 89 (0x0000), A # 0    @ --> loop to the beginning of the overlay
0x005A (0x0000B4) 0x7E03-0x05B5 f:00077 d:   3 | R = OR[3]+1461 (0x05B5)       @ --> 0x05AF
0x005C (0x0000B8) 0x2123-       f:00020 d: 291 | A = OR[291]                   
0x005D (0x0000BA) 0x8E5D-       f:00107 d:  93 | P = P - 93 (0x0000), A # 0    @ --> loop to the beginning of the overlay
0x005E (0x0000BC) 0x2139-       f:00020 d: 313 | A = OR[313]                   @ Test for command chaining CPW@CC (is this the last command in this CPW list?)
0x005F (0x0000BE) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0061), A = 0     @ Yes --> call 0x06EA and we're done
0x0060 (0x0000C0) 0x7004-       f:00070 d:   4 | P = P + 4 (0x0064)            
0x0061 (0x0000C2) 0x7E03-0x06F0 f:00077 d:   3 | R = OR[3]+1776 (0x06F0)       @ --> 0x06EA Set BDV and CPB status to 3, increment CPB@15, zero CNT@NP and POP next activity if exists
0x0063 (0x0000C6) 0x7263-       f:00071 d:  99 | P = P - 99 (0x0000)           @ --> loop to the beginning of the overlay
0x0064 (0x0000C8) 0x1004-       f:00010 d:   4 | A = 4 (0x0004)                @ No --> Advance to the next CPW
0x0065 (0x0000CA) 0x2B38-       f:00025 d: 312 | OR[312] = A + OR[312]         
0x0066 (0x0000CC) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ Wrap around in the CPW list (test for CPB@CE, and load from CPB@CB if needed)
0x0067 (0x0000CE) 0x140D-       f:00012 d:  13 | A = A + 13 (0x000D)           
0x0068 (0x0000D0) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0069 (0x0000D2) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x006A (0x0000D4) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x006B (0x0000D6) 0x2138-       f:00020 d: 312 | A = OR[312]                   
0x006C (0x0000D8) 0x2713-       f:00023 d: 275 | A = A - OR[275]               
0x006D (0x0000DA) 0x8003-       f:00100 d:   3 | P = P + 3 (0x0070), C = 0     
0x006E (0x0000DC) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0070), A = 0     
0x006F (0x0000DE) 0x7002-       f:00070 d:   2 | P = P + 2 (0x0071)            
0x0070 (0x0000E0) 0x7006-       f:00070 d:   6 | P = P + 6 (0x0076)            
0x0071 (0x0000E2) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ Load CPW from CPB@CB
0x0072 (0x0000E4) 0x140C-       f:00012 d:  12 | A = A + 12 (0x000C)           
0x0073 (0x0000E6) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0074 (0x0000E8) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0075 (0x0000EA) 0x2938-       f:00024 d: 312 | OR[312] = A                   
0x0076 (0x0000EC) 0x7E03-0x0762 f:00077 d:   3 | R = OR[3]+1890 (0x0762)       @  --> call 0x075C Parse CPW
0x0078 (0x0000F0) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ Increment number of processed CPWs
0x0079 (0x0000F2) 0x140F-       f:00012 d:  15 | A = A + 15 (0x000F)           
0x007A (0x0000F4) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x007B (0x0000F6) 0x3D08-       f:00036 d: 264 | (OR[264]) = (OR[264]) + 1     
0x007C (0x0000F8) 0x3138-       f:00030 d: 312 | A = (OR[312])                 @ Test if current CPW is already done (CPW@DN)
0x007D (0x0000FA) 0x0808-       f:00004 d:   8 | A = A > 8 (0x0008)            
0x007E (0x0000FC) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x007F (0x0000FE) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0081), A # 0     @ Yes, CPW@DN is set --> call 0x0713 and we're done
0x0080 (0x000100) 0x7004-       f:00070 d:   4 | P = P + 4 (0x0084)            
0x0081 (0x000102) 0x7E03-0x0719 f:00077 d:   3 | R = OR[3]+1817 (0x0719)       @ --> 0x0713 Set BDV and CPB status to 5, increment CPB@27, zero CNT@NP and POP next activity if exists
0x0083 (0x000106) 0x7283-       f:00071 d: 131 | P = P - 131 (0x0000)          @ Jump to the front of the overlay
0x0084 (0x000108) 0x213A-       f:00020 d: 314 | A = OR[314]                   @ No, CPW@DN is not set --> Test if we do a data transfer (CPW@DT) from MOS memory (CPW@MS)
0x0085 (0x00010A) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0087), A # 0     
0x0086 (0x00010C) 0x7024-       f:00070 d:  36 | P = P + 36 (0x00AA)           
0x0087 (0x00010E) 0x213C-       f:00020 d: 316 | A = OR[316]                   @ CPW@MS
0x0088 (0x000110) 0x8602-       f:00103 d:   2 | P = P + 2 (0x008A), A # 0     
0x0089 (0x000112) 0x7021-       f:00070 d:  33 | P = P + 33 (0x00AA)           
0x008A (0x000114) 0x2135-       f:00020 d: 309 | A = OR[309]                   @ Yes, Data transfer from MOS memory - determine which buffer is active (OR[309] - CPB@BP??)
0x008B (0x000116) 0x8402-       f:00102 d:   2 | P = P + 2 (0x008D), A = 0     
0x008C (0x000118) 0x7009-       f:00070 d:   9 | P = P + 9 (0x0095)            
0x008D (0x00011A) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ Get CPB@11, bit 14 into OR[275]
0x008E (0x00011C) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x008F (0x00011E) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0090 (0x000120) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0091 (0x000122) 0x080E-       f:00004 d:  14 | A = A > 14 (0x000E)           
0x0092 (0x000124) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x0093 (0x000126) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x0094 (0x000128) 0x7008-       f:00070 d:   8 | P = P + 8 (0x009C)            
0x0095 (0x00012A) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ Get CPB@11, bit 13 into OR[275]
0x0096 (0x00012C) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x0097 (0x00012E) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0098 (0x000130) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0099 (0x000132) 0x080D-       f:00004 d:  13 | A = A > 13 (0x000D)           
0x009A (0x000134) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x009B (0x000136) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x009C (0x000138) 0x2113-       f:00020 d: 275 | A = OR[275]                   @ Test if OR[275] (bit 14 or 13 of CPB@11) is set: it must be to continue
0x009D (0x00013A) 0x8402-       f:00102 d:   2 | P = P + 2 (0x009F), A = 0     @ Bit not set: --> call 0x0713 and we're done
0x009E (0x00013C) 0x7004-       f:00070 d:   4 | P = P + 4 (0x00A2)            
0x009F (0x00013E) 0x7E03-0x0719 f:00077 d:   3 | R = OR[3]+1817 (0x0719)       @ --> 0x0713 Set BDV and CPB status to 5, increment CPB@27, zero CNT@NP and POP next activity if exists
0x00A1 (0x000142) 0x72A1-       f:00071 d: 161 | P = P - 161 (0x0000)          
0x00A2 (0x000144) 0x213B-       f:00020 d: 315 | A = OR[315]                   @ Bit is set, see if CPW@IN is set, and store CPW address in CPB@30 if it is
0x00A3 (0x000146) 0x8402-       f:00102 d:   2 | P = P + 2 (0x00A5), A = 0     
0x00A4 (0x000148) 0x7006-       f:00070 d:   6 | P = P + 6 (0x00AA)            
0x00A5 (0x00014A) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x00A6 (0x00014C) 0x141E-       f:00012 d:  30 | A = A + 30 (0x001E)           
0x00A7 (0x00014E) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00A8 (0x000150) 0x2138-       f:00020 d: 312 | A = OR[312]                   
0x00A9 (0x000152) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x00AA (0x000154) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ Register current CPW in CPB@CC
0x00AB (0x000156) 0x140E-       f:00012 d:  14 | A = A + 14 (0x000E)           
0x00AC (0x000158) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00AD (0x00015A) 0x2138-       f:00020 d: 312 | A = OR[312]                   
0x00AE (0x00015C) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x00AF (0x00015E) 0x1002-       f:00010 d:   2 | A = 2 (0x0002)                @ Set status code to 2
0x00B0 (0x000160) 0x2932-       f:00024 d: 306 | OR[306] = A                   
0x00B1 (0x000162) 0x7E03-0x07A9 f:00077 d:   3 | R = OR[3]+1961 (0x07A9)       @ --> 0x07A3 Set BDV, CPB status code and POP next activity if exists
0x00B3 (0x000166) 0x7021-       f:00070 d:  33 | P = P + 33 (0x00D4)           

@ CHT@NP (sequence code) is 1 case
0x00B4 (0x000168) 0x213A-       f:00020 d: 314 | A = OR[314]                   @ Test if operation involves data transfer (CPW@DT)
0x00B5 (0x00016A) 0x8602-       f:00103 d:   2 | P = P + 2 (0x00B7), A # 0     
0x00B6 (0x00016C) 0x701E-       f:00070 d:  30 | P = P + 30 (0x00D4)           
0x00B7 (0x00016E) 0x213B-       f:00020 d: 315 | A = OR[315]                   @ Yes, data is involved, is it an input? (CPW@IN)
0x00B8 (0x000170) 0x8602-       f:00103 d:   2 | P = P + 2 (0x00BA), A # 0     
0x00B9 (0x000172) 0x701B-       f:00070 d:  27 | P = P + 27 (0x00D4)           
0x00BA (0x000174) 0x213C-       f:00020 d: 316 | A = OR[316]                   @ Yes, data in. Does it go to MOS memory? (CPW_MS)
0x00BB (0x000176) 0x8602-       f:00103 d:   2 | P = P + 2 (0x00BD), A # 0     
0x00BC (0x000178) 0x7018-       f:00070 d:  24 | P = P + 24 (0x00D4)           
@ All this gets only executed for DATA IN GOING TO MOS transfers
0x00BD (0x00017A) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x00BE (0x00017C) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x00BF (0x00017E) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00C0 (0x000180) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x00C1 (0x000182) 0x0E02-       f:00007 d:   2 | A = A << 2 (0x0002)           
0x00C2 (0x000184) 0x0A01-       f:00005 d:   1 | A = A < 1 (0x0001)            
0x00C3 (0x000186) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x00C4 (0x000188) 0x0C03-       f:00006 d:   3 | A = A >> 3 (0x0003)           
0x00C5 (0x00018A) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x00C6 (0x00018C) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x00C7 (0x00018E) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x00C8 (0x000190) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00C9 (0x000192) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x00CA (0x000194) 0x0E03-       f:00007 d:   3 | A = A << 3 (0x0003)           
0x00CB (0x000196) 0x0A01-       f:00005 d:   1 | A = A < 1 (0x0001)            
0x00CC (0x000198) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x00CD (0x00019A) 0x0C04-       f:00006 d:   4 | A = A >> 4 (0x0004)           
0x00CE (0x00019C) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x00CF (0x00019E) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x00D0 (0x0001A0) 0x141E-       f:00012 d:  30 | A = A + 30 (0x001E)           
0x00D1 (0x0001A2) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00D2 (0x0001A4) 0x2138-       f:00020 d: 312 | A = OR[312]                   
0x00D3 (0x0001A6) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 

@ Common code part: CHT@NP (sequence code) is either 1 or 2
0x00D4 (0x0001A8) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x00D5 (0x0001AA) 0x5800-       f:00054 d:   0 | B = A                         
0x00D6 (0x0001AC) 0xE000-       f:00160 d:   0 | IOB , fn000                   @ Reset BMX channel
0x00D7 (0x0001AE) 0xEC00-       f:00166 d:   0 | IOB , fn006                   @ Disable interrupts
0x00D8 (0x0001B0) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                @ Assemble channel mode (interrupt and such) in OR[284] - that is, set CC bit if CPW@CC is set in a very convoluted way
0x00D9 (0x0001B2) 0x291C-       f:00024 d: 284 | OR[284] = A                   
0x00DA (0x0001B4) 0x2139-       f:00020 d: 313 | A = OR[313]                   @ Do we do command-chaining? (CPW@CC)
0x00DB (0x0001B6) 0x8402-       f:00102 d:   2 | P = P + 2 (0x00DD), A = 0     
0x00DC (0x0001B8) 0x7004-       f:00070 d:   4 | P = P + 4 (0x00E0)            
0x00DD (0x0001BA) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                @ No command chaining
0x00DE (0x0001BC) 0x291D-       f:00024 d: 285 | OR[285] = A                   
0x00DF (0x0001BE) 0x7003-       f:00070 d:   3 | P = P + 3 (0x00E2)            
0x00E0 (0x0001C0) 0x1002-       f:00010 d:   2 | A = 2 (0x0002)                @ Command chaining
0x00E1 (0x0001C2) 0x291D-       f:00024 d: 285 | OR[285] = A                   
0x00E2 (0x0001C4) 0x211D-       f:00020 d: 285 | A = OR[285]                   
0x00E3 (0x0001C6) 0x1203-       f:00011 d:   3 | A = A & 3 (0x0003)            
0x00E4 (0x0001C8) 0x291D-       f:00024 d: 285 | OR[285] = A                   
0x00E5 (0x0001CA) 0x211C-       f:00020 d: 284 | A = OR[284]                   
0x00E6 (0x0001CC) 0x0E05-       f:00007 d:   5 | A = A << 5 (0x0005)           
0x00E7 (0x0001CE) 0x0A02-       f:00005 d:   2 | A = A < 2 (0x0002)            
0x00E8 (0x0001D0) 0x251D-       f:00022 d: 285 | A = A + OR[285]               
0x00E9 (0x0001D2) 0x0C07-       f:00006 d:   7 | A = A >> 7 (0x0007)           
0x00EA (0x0001D4) 0x291C-       f:00024 d: 284 | OR[284] = A                   
0x00EB (0x0001D6) 0x2131-       f:00020 d: 305 | A = OR[305]                   @ Load device address from BDV@UN
0x00EC (0x0001D8) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x00ED (0x0001DA) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x00EE (0x0001DC) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x00EF (0x0001DE) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x00F0 (0x0001E0) 0x2919-       f:00024 d: 281 | OR[281] = A                   
0x00F1 (0x0001E2) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x00F2 (0x0001E4) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x00F3 (0x0001E6) 0x2919-       f:00024 d: 281 | OR[281] = A                   
0x00F4 (0x0001E8) 0x211C-       f:00020 d: 284 | A = OR[284]                   @ Combine mode bits with device address
0x00F5 (0x0001EA) 0x1A00-0xFF00 f:00015 d:   0 | A = A & 65280 (0xFF00)        
0x00F7 (0x0001EE) 0x2519-       f:00022 d: 281 | A = A + OR[281]               
0x00F8 (0x0001F0) 0x291C-       f:00024 d: 284 | OR[284] = A                   
0x00F9 (0x0001F2) 0x3131-       f:00030 d: 305 | A = (OR[305])                 @ Get BDV@CM - this contains the communication mode (DC interlock, 3.0 or 4.5MBps speed) for the channel
0x00FA (0x0001F4) 0x080E-       f:00004 d:  14 | A = A > 14 (0x000E)           
0x00FB (0x0001F6) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x00FC (0x0001F8) 0x211E-       f:00020 d: 286 | A = OR[286]                   
0x00FD (0x0001FA) 0x1203-       f:00011 d:   3 | A = A & 3 (0x0003)            
0x00FE (0x0001FC) 0x291E-       f:00024 d: 286 | OR[286] = A                   
0x00FF (0x0001FE) 0x211C-       f:00020 d: 284 | A = OR[284]                   
0x0100 (0x000200) 0x0A03-       f:00005 d:   3 | A = A < 3 (0x0003)            
0x0101 (0x000202) 0x251E-       f:00022 d: 286 | A = A + OR[286]               
0x0102 (0x000204) 0x0C03-       f:00006 d:   3 | A = A >> 3 (0x0003)           
0x0103 (0x000206) 0x291C-       f:00024 d: 284 | OR[284] = A                   
0x0104 (0x000208) 0x211C-       f:00020 d: 284 | A = OR[284]                   
0x0105 (0x00020A) 0xFC00-       f:00176 d:   0 | IOB , fn016                   @ Enter device address, mode
0x0106 (0x00020C) 0x213A-       f:00020 d: 314 | A = OR[314]                   @ Test if we do a data transfer (CPW@DT)
0x0107 (0x00020E) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0109), A # 0     
0x0108 (0x000210) 0x70D5-       f:00070 d: 213 | P = P + 213 (0x01DD)          
0x0109 (0x000212) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x010A (0x000214) 0x2922-       f:00024 d: 290 | OR[290] = A                   
0x010B (0x000216) 0x1003-       f:00010 d:   3 | A = 3 (0x0003)                
0x010C (0x000218) 0x292C-       f:00024 d: 300 | OR[300] = A                   
0x010D (0x00021A) 0x213C-       f:00020 d: 316 | A = OR[316]                   @ CPW@MS
0x010E (0x00021C) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0110), A # 0     
0x010F (0x00021E) 0x7083-       f:00070 d: 131 | P = P + 131 (0x0192)          
0x0110 (0x000220) 0x2135-       f:00020 d: 309 | A = OR[309]                   @  OR[309] - CPB@BP??
0x0111 (0x000222) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0113), A = 0     
0x0112 (0x000224) 0x700C-       f:00070 d:  12 | P = P + 12 (0x011E)           
0x0113 (0x000226) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x0114 (0x000228) 0x1409-       f:00012 d:   9 | A = A + 9 (0x0009)            
0x0115 (0x00022A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0116 (0x00022C) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0117 (0x00022E) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x0118 (0x000230) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x0119 (0x000232) 0x140A-       f:00012 d:  10 | A = A + 10 (0x000A)           
0x011A (0x000234) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x011B (0x000236) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x011C (0x000238) 0x2929-       f:00024 d: 297 | OR[297] = A                   
0x011D (0x00023A) 0x700B-       f:00070 d:  11 | P = P + 11 (0x0128)           
0x011E (0x00023C) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x011F (0x00023E) 0x140A-       f:00012 d:  10 | A = A + 10 (0x000A)           
0x0120 (0x000240) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0121 (0x000242) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0122 (0x000244) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x0123 (0x000246) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x0124 (0x000248) 0x1409-       f:00012 d:   9 | A = A + 9 (0x0009)            
0x0125 (0x00024A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0126 (0x00024C) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0127 (0x00024E) 0x2929-       f:00024 d: 297 | OR[297] = A                   
0x0128 (0x000250) 0x213B-       f:00020 d: 315 | A = OR[315]                   @ CPW@IN
0x0129 (0x000252) 0x8602-       f:00103 d:   2 | P = P + 2 (0x012B), A # 0     
0x012A (0x000254) 0x700C-       f:00070 d:  12 | P = P + 12 (0x0136)           
0x012B (0x000256) 0x1800-0x1000 f:00014 d:   0 | A = 4096 (0x1000)             
0x012D (0x00025A) 0x2928-       f:00024 d: 296 | OR[296] = A                   
0x012E (0x00025C) 0x1800-0x1000 f:00014 d:   0 | A = 4096 (0x1000)             
0x0130 (0x000260) 0x292A-       f:00024 d: 298 | OR[298] = A                   
0x0131 (0x000262) 0x1002-       f:00010 d:   2 | A = 2 (0x0002)                
0x0132 (0x000264) 0x2B27-       f:00025 d: 295 | OR[295] = A + OR[295]         
0x0133 (0x000266) 0x1003-       f:00010 d:   3 | A = 3 (0x0003)                
0x0134 (0x000268) 0x2B29-       f:00025 d: 297 | OR[297] = A + OR[297]         
0x0135 (0x00026A) 0x705C-       f:00070 d:  92 | P = P + 92 (0x0191)           
0x0136 (0x00026C) 0x2135-       f:00020 d: 309 | A = OR[309]                   @  OR[309] - CPB@BP??
0x0137 (0x00026E) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0139), A = 0     
0x0138 (0x000270) 0x7009-       f:00070 d:   9 | P = P + 9 (0x0141)            
0x0139 (0x000272) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x013A (0x000274) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x013B (0x000276) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x013C (0x000278) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x013D (0x00027A) 0x080D-       f:00004 d:  13 | A = A > 13 (0x000D)           
0x013E (0x00027C) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x013F (0x00027E) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x0140 (0x000280) 0x7008-       f:00070 d:   8 | P = P + 8 (0x0148)            
0x0141 (0x000282) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x0142 (0x000284) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x0143 (0x000286) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0144 (0x000288) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0145 (0x00028A) 0x080E-       f:00004 d:  14 | A = A > 14 (0x000E)           
0x0146 (0x00028C) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x0147 (0x00028E) 0x2921-       f:00024 d: 289 | OR[289] = A                   
@@@@@@@@@@@@
@ We fill-in the buffer addresses for a large, MOS based trasfer here:
@ For large (MOS-based) data transfers, we use two 4k buffers to get the data in and out of in IO memory
@ If we need to transfer more than 4k, we have to swap the two buffers, decrement the byte count and loop
@ The two buffer addresses are in OR[295] and OR[297]. The byte counts are in OR[296] and OR[298]
0x0148 (0x000290) 0x2138-       f:00020 d: 312 | A = OR[312]                   @ Load upper byte count from CPW into OR[275]
0x0149 (0x000292) 0x1402-       f:00012 d:   2 | A = A + 2 (0x0002)            
0x014A (0x000294) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x014B (0x000296) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x014C (0x000298) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x014D (0x00029A) 0x2138-       f:00020 d: 312 | A = OR[312]                   @ Load lower byte count from CPW into OR[276]
0x014E (0x00029C) 0x1403-       f:00012 d:   3 | A = A + 3 (0x0003)            
0x014F (0x00029E) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0150 (0x0002A0) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0151 (0x0002A2) 0x2914-       f:00024 d: 276 | OR[276] = A                   
0x0152 (0x0002A4) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x0153 (0x0002A6) 0x8608-       f:00103 d:   8 | P = P + 8 (0x015B), A # 0     
0x0154 (0x0002A8) 0x2114-       f:00020 d: 276 | A = OR[276]                   
0x0155 (0x0002AA) 0x1E00-0x1000 f:00017 d:   0 | A = A - 4096 (0x1000)         
0x0157 (0x0002AE) 0x8003-       f:00100 d:   3 | P = P + 3 (0x015A), C = 0     
0x0158 (0x0002B0) 0x8402-       f:00102 d:   2 | P = P + 2 (0x015A), A = 0     
0x0159 (0x0002B2) 0x7002-       f:00070 d:   2 | P = P + 2 (0x015B)            
0x015A (0x0002B4) 0x7027-       f:00070 d:  39 | P = P + 39 (0x0181)           
0x015B (0x0002B6) 0x2121-       f:00020 d: 289 | A = OR[289]                   @ We have more than 4k to transfer - swap some buffers (I guess)
0x015C (0x0002B8) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x015D (0x0002BA) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x015E (0x0002BC) 0x2922-       f:00024 d: 290 | OR[290] = A                   
0x015F (0x0002BE) 0x1800-0x1000 f:00014 d:   0 | A = 4096 (0x1000)             
0x0161 (0x0002C2) 0x2928-       f:00024 d: 296 | OR[296] = A                   
0x0162 (0x0002C4) 0x1002-       f:00010 d:   2 | A = 2 (0x0002)                
0x0163 (0x0002C6) 0x2B27-       f:00025 d: 295 | OR[295] = A + OR[295]         
0x0164 (0x0002C8) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x0165 (0x0002CA) 0x290F-       f:00024 d: 271 | OR[271] = A                   
0x0166 (0x0002CC) 0x2114-       f:00020 d: 276 | A = OR[276]                   @ Decrement byte count by 4k
0x0167 (0x0002CE) 0x1E00-0x1000 f:00017 d:   0 | A = A - 4096 (0x1000)         
0x0169 (0x0002D2) 0x2914-       f:00024 d: 276 | OR[276] = A                   
0x016A (0x0002D4) 0x8202-       f:00101 d:   2 | P = P + 2 (0x016C), C = 1     
0x016B (0x0002D6) 0x2F13-       f:00027 d: 275 | OR[275] = OR[275] - 1         
0x016C (0x0002D8) 0x0810-       f:00004 d:  16 | A = A > 16 (0x0010)           
0x016D (0x0002DA) 0x230F-       f:00021 d: 271 | A = A & OR[271]               
0x016E (0x0002DC) 0x2113-       f:00020 d: 275 | A = OR[275]                   @ Test if we still have more than 4k to transfer
0x016F (0x0002DE) 0x8608-       f:00103 d:   8 | P = P + 8 (0x0177), A # 0     
0x0170 (0x0002E0) 0x2114-       f:00020 d: 276 | A = OR[276]                   
0x0171 (0x0002E2) 0x1E00-0x1000 f:00017 d:   0 | A = A - 4096 (0x1000)         
0x0173 (0x0002E6) 0x8003-       f:00100 d:   3 | P = P + 3 (0x0176), C = 0     
0x0174 (0x0002E8) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0176), A = 0     
0x0175 (0x0002EA) 0x7002-       f:00070 d:   2 | P = P + 2 (0x0177)            
0x0176 (0x0002EC) 0x7007-       f:00070 d:   7 | P = P + 7 (0x017D)            
0x0177 (0x0002EE) 0x1800-0x1000 f:00014 d:   0 | A = 4096 (0x1000)             @ Yes we still have more than 4k
0x0179 (0x0002F2) 0x292A-       f:00024 d: 298 | OR[298] = A                   @ OR[298] will eventually be the byte count for the current transfer, load it with the maximum
0x017A (0x0002F4) 0x1003-       f:00010 d:   3 | A = 3 (0x0003)                
0x017B (0x0002F6) 0x2B29-       f:00025 d: 297 | OR[297] = A + OR[297]         
0x017C (0x0002F8) 0x7004-       f:00070 d:   4 | P = P + 4 (0x0180)            
0x017D (0x0002FA) 0x2114-       f:00020 d: 276 | A = OR[276]                   @ No, we're transferring between 4k and 8k
0x017E (0x0002FC) 0x292A-       f:00024 d: 298 | OR[298] = A                   @ OR[298] will eventually be the byte count for the current transfer, load it with the remainder
0x017F (0x0002FE) 0x2D29-       f:00026 d: 297 | OR[297] = OR[297] + 1         
0x0180 (0x000300) 0x7011-       f:00070 d:  17 | P = P + 17 (0x0191)           
0x0181 (0x000302) 0x2139-       f:00020 d: 313 | A = OR[313]                   @ Transfer no more than 4k bytes - load CPW@CC
0x0182 (0x000304) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0184), A # 0     
0x0183 (0x000306) 0x7005-       f:00070 d:   5 | P = P + 5 (0x0188)            
0x0184 (0x000308) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x0185 (0x00030A) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x0186 (0x00030C) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x0187 (0x00030E) 0x2922-       f:00024 d: 290 | OR[290] = A                   
0x0188 (0x000310) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0189 (0x000312) 0x2921-       f:00024 d: 289 | OR[289] = A                   
0x018A (0x000314) 0x2114-       f:00020 d: 276 | A = OR[276]                   
0x018B (0x000316) 0x2928-       f:00024 d: 296 | OR[296] = A                   
0x018C (0x000318) 0x2127-       f:00020 d: 295 | A = OR[295]                   
0x018D (0x00031A) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x018E (0x00031C) 0x2929-       f:00024 d: 297 | OR[297] = A                   
0x018F (0x00031E) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x0190 (0x000320) 0x292A-       f:00024 d: 298 | OR[298] = A                   
0x0191 (0x000322) 0x7012-       f:00070 d:  18 | P = P + 18 (0x01A3)           

@@@@@@@@@@@@@
@ We fill in the buffer addresses for a small (I/O memory based) trasfer here
@ These are either command or sense operations
@ OR[312]: pointer to CPW
0x0192 (0x000324) 0x2138-       f:00020 d: 312 | A = OR[312]                   @ Load local memory address from CPW offset 1 into OR[295]
0x0193 (0x000326) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x0194 (0x000328) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0195 (0x00032A) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0196 (0x00032C) 0x2927-       f:00024 d: 295 | OR[295] = A                   
0x0197 (0x00032E) 0x2138-       f:00020 d: 312 | A = OR[312]                   @ Load byte count from CPW offset 3 into OR[296]
0x0198 (0x000330) 0x1403-       f:00012 d:   3 | A = A + 3 (0x0003)            
0x0199 (0x000332) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x019A (0x000334) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x019B (0x000336) 0x2928-       f:00024 d: 296 | OR[296] = A                   
0x019C (0x000338) 0x2127-       f:00020 d: 295 | A = OR[295]                   
0x019D (0x00033A) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            @ Put byte count + 1 into OR[297] (this is what we set the secondary byte counter to if no chaining is specified)
0x019E (0x00033C) 0x2929-       f:00024 d: 297 | OR[297] = A                   
0x019F (0x00033E) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x01A0 (0x000340) 0x292A-       f:00024 d: 298 | OR[298] = A                   
0x01A1 (0x000342) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x01A2 (0x000344) 0x2921-       f:00024 d: 289 | OR[289] = A                   
@ Common section for both IO and MOS based transfers
0x01A3 (0x000346) 0x1006-       f:00010 d:   6 | A = 6 (0x0006)                
0x01A4 (0x000348) 0x292F-       f:00024 d: 303 | OR[303] = A                   
0x01A5 (0x00034A) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x01A6 (0x00034C) 0x5800-       f:00054 d:   0 | B = A                         
0x01A7 (0x00034E) 0x2127-       f:00020 d: 295 | A = OR[295]                   
0x01A8 (0x000350) 0xF800-       f:00174 d:   0 | IOB , fn014                   @ Enter local memory address
0x01A9 (0x000352) 0x1A00-0xFFFF f:00015 d:   0 | A = A & 65535 (0xFFFF)        
0x01AB (0x000356) 0xF000-       f:00170 d:   0 | IOB , fn010                   @ Read local memory address
0x01AC (0x000358) 0x2917-       f:00024 d: 279 | OR[279] = A                   
0x01AD (0x00035A) 0x2727-       f:00023 d: 295 | A = A - OR[295]               
0x01AE (0x00035C) 0x8602-       f:00103 d:   2 | P = P + 2 (0x01B0), A # 0     
0x01AF (0x00035E) 0x7004-       f:00070 d:   4 | P = P + 4 (0x01B3)            
0x01B0 (0x000360) 0x7E03-0x06FF f:00077 d:   3 | R = OR[3]+1791 (0x06FF)       @ --> 0x06F9 Set BDV and CPB status to 4, increment CPB@15, DO SOME STUFF!!!, zero CNT@NP and POP next activity if exists
0x01B2 (0x000364) 0x73B2-       f:00071 d: 434 | P = P - 434 (0x0000)          
0x01B3 (0x000366) 0x2129-       f:00020 d: 297 | A = OR[297]                   
0x01B4 (0x000368) 0xF800-       f:00174 d:   0 | IOB , fn014                   @ Enter local memory address
0x01B5 (0x00036A) 0x1A00-0xFFFF f:00015 d:   0 | A = A & 65535 (0xFFFF)        
0x01B7 (0x00036E) 0xF000-       f:00170 d:   0 | IOB , fn010                   @ Read local memory address
0x01B8 (0x000370) 0x2917-       f:00024 d: 279 | OR[279] = A                   
0x01B9 (0x000372) 0x2729-       f:00023 d: 297 | A = A - OR[297]               
0x01BA (0x000374) 0x8602-       f:00103 d:   2 | P = P + 2 (0x01BC), A # 0     
0x01BB (0x000376) 0x7004-       f:00070 d:   4 | P = P + 4 (0x01BF)            
0x01BC (0x000378) 0x7E03-0x06FF f:00077 d:   3 | R = OR[3]+1791 (0x06FF)       @ --> 0x06F9 Set BDV and CPB status to 4, increment CPB@15, DO SOME STUFF!!!, zero CNT@NP and POP next activity if exists
0x01BE (0x00037C) 0x73BE-       f:00071 d: 446 | P = P - 446 (0x0000)          
0x01BF (0x00037E) 0x1005-       f:00010 d:   5 | A = 5 (0x0005)                
0x01C0 (0x000380) 0x292F-       f:00024 d: 303 | OR[303] = A                   
0x01C1 (0x000382) 0x2128-       f:00020 d: 296 | A = OR[296]                   
0x01C2 (0x000384) 0xFA00-       f:00175 d:   0 | IOB , fn015                   @ Enter byte count
0x01C3 (0x000386) 0x213B-       f:00020 d: 315 | A = OR[315]                   @ CPW@IN
0x01C4 (0x000388) 0x8604-       f:00103 d:   4 | P = P + 4 (0x01C8), A # 0     
0x01C5 (0x00038A) 0x2121-       f:00020 d: 289 | A = OR[289]                   
0x01C6 (0x00038C) 0x8602-       f:00103 d:   2 | P = P + 2 (0x01C8), A # 0     
0x01C7 (0x00038E) 0x700C-       f:00070 d:  12 | P = P + 12 (0x01D3)           
0x01C8 (0x000390) 0x212A-       f:00020 d: 298 | A = OR[298]                   
0x01C9 (0x000392) 0xFA00-       f:00175 d:   0 | IOB , fn015                   @ Enter byte count
0x01CA (0x000394) 0x1A00-0xFFFF f:00015 d:   0 | A = A & 65535 (0xFFFF)        
0x01CC (0x000398) 0xF200-       f:00171 d:   0 | IOB , fn011                   @ Read byte counter
0x01CD (0x00039A) 0x272A-       f:00023 d: 298 | A = A - OR[298]               
0x01CE (0x00039C) 0x8602-       f:00103 d:   2 | P = P + 2 (0x01D0), A # 0     
0x01CF (0x00039E) 0x7004-       f:00070 d:   4 | P = P + 4 (0x01D3)            
0x01D0 (0x0003A0) 0x7E03-0x06FF f:00077 d:   3 | R = OR[3]+1791 (0x06FF)       @ --> 0x06F9 Set BDV and CPB status to 4, increment CPB@15, DO SOME STUFF!!!, zero CNT@NP and POP next activity if exists
0x01D2 (0x0003A4) 0x73D2-       f:00071 d: 466 | P = P - 466 (0x0000)          
0x01D3 (0x0003A6) 0x1A00-0xFFFF f:00015 d:   0 | A = A & 65535 (0xFFFF)        
0x01D5 (0x0003AA) 0xF200-       f:00171 d:   0 | IOB , fn011                   @ Read byte counter
0x01D6 (0x0003AC) 0x2728-       f:00023 d: 296 | A = A - OR[296]               
0x01D7 (0x0003AE) 0x8602-       f:00103 d:   2 | P = P + 2 (0x01D9), A # 0     
0x01D8 (0x0003B0) 0x7004-       f:00070 d:   4 | P = P + 4 (0x01DC)            
0x01D9 (0x0003B2) 0x7E03-0x06FF f:00077 d:   3 | R = OR[3]+1791 (0x06FF)       @ --> 0x06F9 Set BDV and CPB status to 4, increment CPB@15, DO SOME STUFF!!!, zero CNT@NP and POP next activity if exists
0x01DB (0x0003B6) 0x73DB-       f:00071 d: 475 | P = P - 475 (0x0000)          
0x01DC (0x0003B8) 0x7003-       f:00070 d:   3 | P = P + 3 (0x01DF)            
@ This is some strange special-case code for something, but it gets jumped to
0x01DD (0x0003BA) 0x1002-       f:00010 d:   2 | A = 2 (0x0002)                
0x01DE (0x0003BC) 0x292C-       f:00024 d: 300 | OR[300] = A                   

0x01DF (0x0003BE) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                @ Set command code from OR[283] in a very convoluted way
0x01E0 (0x0003C0) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x01E1 (0x0003C2) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x01E2 (0x0003C4) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x01E3 (0x0003C6) 0x291B-       f:00024 d: 283 | OR[283] = A                   
0x01E4 (0x0003C8) 0x211F-       f:00020 d: 287 | A = OR[287]                   
0x01E5 (0x0003CA) 0x1A00-0xFF00 f:00015 d:   0 | A = A & 65280 (0xFF00)        
0x01E7 (0x0003CE) 0x251B-       f:00022 d: 283 | A = A + OR[283]               
0x01E8 (0x0003D0) 0x291F-       f:00024 d: 287 | OR[287] = A                   
0x01E9 (0x0003D2) 0x211F-       f:00020 d: 287 | A = OR[287]                   
0x01EA (0x0003D4) 0xE400-       f:00162 d:   0 | IOB , fn002                   @ Start operation
0x01EB (0x0003D6) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ Clear CPB@20
0x01EC (0x0003D8) 0x1414-       f:00012 d:  20 | A = A + 20 (0x0014)           
0x01ED (0x0003DA) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x01EE (0x0003DC) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x01EF (0x0003DE) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x01F0 (0x0003E0) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ Copy lower byte into upper byte of CPB@28
0x01F1 (0x0003E2) 0x141C-       f:00012 d:  28 | A = A + 28 (0x001C)           
0x01F2 (0x0003E4) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x01F3 (0x0003E6) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x01F4 (0x0003E8) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x01F5 (0x0003EA) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x01F6 (0x0003EC) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x01F7 (0x0003EE) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x01F8 (0x0003F0) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x01F9 (0x0003F2) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x01FA (0x0003F4) 0x141C-       f:00012 d:  28 | A = A + 28 (0x001C)           
0x01FB (0x0003F6) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x01FC (0x0003F8) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x01FD (0x0003FA) 0x0A09-       f:00005 d:   9 | A = A < 9 (0x0009)            
0x01FE (0x0003FC) 0x2513-       f:00022 d: 275 | A = A + OR[275]               
0x01FF (0x0003FE) 0x0C09-       f:00006 d:   9 | A = A >> 9 (0x0009)           
0x0200 (0x000400) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0201 (0x000402) 0x211B-       f:00020 d: 283 | A = OR[283]                   @ Put command code into lower buyte CPB@28
0x0202 (0x000404) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x0203 (0x000406) 0x291B-       f:00024 d: 283 | OR[283] = A                   
0x0204 (0x000408) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x0205 (0x00040A) 0x141C-       f:00012 d:  28 | A = A + 28 (0x001C)           
0x0206 (0x00040C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0207 (0x00040E) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0208 (0x000410) 0x1A00-0xFF00 f:00015 d:   0 | A = A & 65280 (0xFF00)        
0x020A (0x000414) 0x251B-       f:00022 d: 283 | A = A + OR[283]               
0x020B (0x000416) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x020C (0x000418) 0x212B-       f:00020 d: 299 | A = OR[299]                   @ OR[299] pointer to CHT - clear CHT@3, bit 14
0x020D (0x00041A) 0x1403-       f:00012 d:   3 | A = A + 3 (0x0003)            
0x020E (0x00041C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x020F (0x00041E) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0210 (0x000420) 0x1A00-0xBFFF f:00015 d:   0 | A = A & 49151 (0xBFFF)        
0x0212 (0x000424) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0213 (0x000426) 0x212B-       f:00020 d: 299 | A = OR[299]                   @ Set CHT@3, bit 13
0x0214 (0x000428) 0x1403-       f:00012 d:   3 | A = A + 3 (0x0003)            
0x0215 (0x00042A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0216 (0x00042C) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0217 (0x00042E) 0x0E03-       f:00007 d:   3 | A = A << 3 (0x0003)           
0x0218 (0x000430) 0x0A01-       f:00005 d:   1 | A = A < 1 (0x0001)            
0x0219 (0x000432) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x021A (0x000434) 0x0C04-       f:00006 d:   4 | A = A >> 4 (0x0004)           
0x021B (0x000436) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x021C (0x000438) 0x2131-       f:00020 d: 305 | A = OR[305]                   @ OR[305] pointer to BDV - clear BDV@5, bits 0-2
0x021D (0x00043A) 0x1405-       f:00012 d:   5 | A = A + 5 (0x0005)            
0x021E (0x00043C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x021F (0x00043E) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0220 (0x000440) 0x1A00-0xFFF7 f:00015 d:   0 | A = A & 65527 (0xFFF7)        
0x0222 (0x000444) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0223 (0x000446) 0x213A-       f:00020 d: 314 | A = OR[314]                   @ OR[314] CPW@DT
0x0224 (0x000448) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0226), A # 0     
0x0225 (0x00044A) 0x7068-       f:00070 d: 104 | P = P + 104 (0x028D)          
0x0226 (0x00044C) 0x213C-       f:00020 d: 316 | A = OR[316]                   @ OR[316] CPW@MS
0x0227 (0x00044E) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0229), A # 0     
0x0228 (0x000450) 0x7065-       f:00070 d: 101 | P = P + 101 (0x028D)          
@ We get here for MOS-based data transfers
0x0229 (0x000452) 0x212B-       f:00020 d: 299 | A = OR[299]                   @ Clear CHT@3, bit 15
0x022A (0x000454) 0x1403-       f:00012 d:   3 | A = A + 3 (0x0003)            
0x022B (0x000456) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x022C (0x000458) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x022D (0x00045A) 0x1A00-0xEFFF f:00015 d:   0 | A = A & 61439 (0xEFFF)        
0x022F (0x00045E) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0230 (0x000460) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ Clear CPB@11, bit 15
0x0231 (0x000462) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x0232 (0x000464) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0233 (0x000466) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0234 (0x000468) 0x1A00-0xEFFF f:00015 d:   0 | A = A & 61439 (0xEFFF)        
0x0236 (0x00046C) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0237 (0x00046E) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ Clear CPB@11, bit 6
0x0238 (0x000470) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x0239 (0x000472) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x023A (0x000474) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x023B (0x000476) 0x1A00-0xFFDF f:00015 d:   0 | A = A & 65503 (0xFFDF)        
0x023D (0x00047A) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x023E (0x00047C) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ Clear CPB@11, bit 7
0x023F (0x00047E) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x0240 (0x000480) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0241 (0x000482) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0242 (0x000484) 0x1A00-0xFFEF f:00015 d:   0 | A = A & 65519 (0xFFEF)        
0x0244 (0x000488) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0245 (0x00048A) 0x2135-       f:00020 d: 309 | A = OR[309]                   @  OR[309] - original CPB@BP??
0x0246 (0x00048C) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0248), A # 0     
0x0247 (0x00048E) 0x7046-       f:00070 d:  70 | P = P + 70 (0x028D)           
@ We have a MOS-based data transfer and CPB@11, bit 15 was set originally (that is we've just cleared it)
0x0248 (0x000490) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ swap CPB@B0 and CPB@B1
0x0249 (0x000492) 0x1409-       f:00012 d:   9 | A = A + 9 (0x0009)            
0x024A (0x000494) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x024B (0x000496) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x024C (0x000498) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x024D (0x00049A) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x024E (0x00049C) 0x140A-       f:00012 d:  10 | A = A + 10 (0x000A)           
0x024F (0x00049E) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0250 (0x0004A0) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0251 (0x0004A2) 0x2914-       f:00024 d: 276 | OR[276] = A                   
0x0252 (0x0004A4) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x0253 (0x0004A6) 0x1409-       f:00012 d:   9 | A = A + 9 (0x0009)            
0x0254 (0x0004A8) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0255 (0x0004AA) 0x2114-       f:00020 d: 276 | A = OR[276]                   
0x0256 (0x0004AC) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0257 (0x0004AE) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x0258 (0x0004B0) 0x140A-       f:00012 d:  10 | A = A + 10 (0x000A)           
0x0259 (0x0004B2) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x025A (0x0004B4) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x025B (0x0004B6) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x025C (0x0004B8) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ swap CPB@11, bit 14 and 13
0x025D (0x0004BA) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x025E (0x0004BC) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x025F (0x0004BE) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0260 (0x0004C0) 0x080E-       f:00004 d:  14 | A = A > 14 (0x000E)           
0x0261 (0x0004C2) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x0262 (0x0004C4) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x0263 (0x0004C6) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x0264 (0x0004C8) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x0265 (0x0004CA) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0266 (0x0004CC) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0267 (0x0004CE) 0x080D-       f:00004 d:  13 | A = A > 13 (0x000D)           
0x0268 (0x0004D0) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x0269 (0x0004D2) 0x2914-       f:00024 d: 276 | OR[276] = A                   
0x026A (0x0004D4) 0x2114-       f:00020 d: 276 | A = OR[276]                   
0x026B (0x0004D6) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x026C (0x0004D8) 0x2914-       f:00024 d: 276 | OR[276] = A                   
0x026D (0x0004DA) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x026E (0x0004DC) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x026F (0x0004DE) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0270 (0x0004E0) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0271 (0x0004E2) 0x0E02-       f:00007 d:   2 | A = A << 2 (0x0002)           
0x0272 (0x0004E4) 0x0A01-       f:00005 d:   1 | A = A < 1 (0x0001)            
0x0273 (0x0004E6) 0x2514-       f:00022 d: 276 | A = A + OR[276]               
0x0274 (0x0004E8) 0x0C03-       f:00006 d:   3 | A = A >> 3 (0x0003)           
0x0275 (0x0004EA) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0276 (0x0004EC) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x0277 (0x0004EE) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x0278 (0x0004F0) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x0279 (0x0004F2) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x027A (0x0004F4) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x027B (0x0004F6) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x027C (0x0004F8) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x027D (0x0004FA) 0x0E03-       f:00007 d:   3 | A = A << 3 (0x0003)           
0x027E (0x0004FC) 0x0A01-       f:00005 d:   1 | A = A < 1 (0x0001)            
0x027F (0x0004FE) 0x2513-       f:00022 d: 275 | A = A + OR[275]               
0x0280 (0x000500) 0x0C04-       f:00006 d:   4 | A = A >> 4 (0x0004)           
0x0281 (0x000502) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0282 (0x000504) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ Clear CPB@BP??
0x0283 (0x000506) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x0284 (0x000508) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0285 (0x00050A) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0286 (0x00050C) 0x0E01-       f:00007 d:   1 | A = A << 1 (0x0001)           
0x0287 (0x00050E) 0x0A01-       f:00005 d:   1 | A = A < 1 (0x0001)            
0x0288 (0x000510) 0x1400-       f:00012 d:   0 | A = A + 0 (0x0000)            
0x0289 (0x000512) 0x0C02-       f:00006 d:   2 | A = A >> 2 (0x0002)           
0x028A (0x000514) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x028B (0x000516) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                @ Clear OR[309] - original CPB@BP??
0x028C (0x000518) 0x2935-       f:00024 d: 309 | OR[309] = A                   
@ Common code for both MOS-based and non-MOS-based (or non-data) transfers
0x028D (0x00051A) 0x213A-       f:00020 d: 314 | A = OR[314]                   @ CPW@DT
0x028E (0x00051C) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0290), A # 0     
0x028F (0x00051E) 0x700C-       f:00070 d:  12 | P = P + 12 (0x029B)           
0x0290 (0x000520) 0x213C-       f:00020 d: 316 | A = OR[316]                   @ CPW@MS
0x0291 (0x000522) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0293), A # 0     
0x0292 (0x000524) 0x7009-       f:00070 d:   9 | P = P + 9 (0x029B)            
0x0293 (0x000526) 0x213B-       f:00020 d: 315 | A = OR[315]                   @ CPW@IN
0x0294 (0x000528) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0296), A = 0     
0x0295 (0x00052A) 0x7006-       f:00070 d:   6 | P = P + 6 (0x029B)            
0x0296 (0x00052C) 0x2122-       f:00020 d: 290 | A = OR[290]                   @ I think this is set if we have two buffers to transfer
0x0297 (0x00052E) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0299), A # 0     
0x0298 (0x000530) 0x7003-       f:00070 d:   3 | P = P + 3 (0x029B)            
0x0299 (0x000532) 0x7E03-0x07C8 f:00077 d:   3 | R = OR[3]+1992 (0x07C8)       @ --> 0x07C2 - we get here for a MOS data transfer with more more than one buffer (?) and if CPW@0, bit 15 is clear
0x029B (0x000536) 0x3138-       f:00030 d: 312 | A = (OR[312])                 @ Load CPW@0, upper byte (flags) into OR[275]
0x029C (0x000538) 0x0808-       f:00004 d:   8 | A = A > 8 (0x0008)            
0x029D (0x00053A) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x029E (0x00053C) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ Load CPB@11 (flags) into OR[276]
0x029F (0x00053E) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x02A0 (0x000540) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x02A1 (0x000542) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x02A2 (0x000544) 0x2914-       f:00024 d: 276 | OR[276] = A                   
0x02A3 (0x000546) 0x0400-       f:00002 d:   0 | I = 0                         @ Do some logging, using CPW and CPB flags
0x02A4 (0x000548) 0x0000-       f:00000 d:   0 | PASS                          
0x02A5 (0x00054A) 0x1011-       f:00010 d:  17 | A = 17 (0x0011)               
0x02A6 (0x00054C) 0x29C3-       f:00024 d: 451 | OR[451] = A                   
0x02A7 (0x00054E) 0x2138-       f:00020 d: 312 | A = OR[312]                   
0x02A8 (0x000550) 0x29C4-       f:00024 d: 452 | OR[452] = A                   
0x02A9 (0x000552) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x02AA (0x000554) 0x29C5-       f:00024 d: 453 | OR[453] = A                   
0x02AB (0x000556) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x02AC (0x000558) 0x29C6-       f:00024 d: 454 | OR[454] = A                   
0x02AD (0x00055A) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x02AE (0x00055C) 0x29C7-       f:00024 d: 455 | OR[455] = A                   
0x02AF (0x00055E) 0x2114-       f:00020 d: 276 | A = OR[276]                   
0x02B0 (0x000560) 0x29C8-       f:00024 d: 456 | OR[456] = A                   
0x02B1 (0x000562) 0x7DC2-       f:00076 d: 450 | R = OR[450]                   
0x02B2 (0x000564) 0x0600-       f:00003 d:   0 | I = 1                         
0x02B3 (0x000566) 0x7E03-0x0806 f:00077 d:   3 | R = OR[3]+2054 (0x0806)       @ --> 0x0800 - TPUSH current activity into CHT@QU (waiting for channel), than enable (BMX) interrupts
0x02B5 (0x00056A) 0x7A03-0x0514 f:00075 d:   3 | P = OR[3]+1300 (0x0514)       @ --> 0x050e - jump to the beginning, pick next CHT. THIS IS WHERE WE LOOP FOR THE FIRST CHT ENTRY IN SIMULATION!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

@ We get here if CHT@NP (sequence code) is neither 1 or 2
0x02B7 (0x00056E) 0x212C-       f:00020 d: 300 | A = OR[300]                   
0x02B8 (0x000570) 0x1603-       f:00013 d:   3 | A = A - 3 (0x0003)            
0x02B9 (0x000572) 0x8402-       f:00102 d:   2 | P = P + 2 (0x02BB), A = 0     
0x02BA (0x000574) 0x7111-       f:00070 d: 273 | P = P + 273 (0x03CB)          
@ CHT@NP (sequence code) is 3 case
0x02BB (0x000576) 0x7E03-0x0728 f:00077 d:   3 | R = OR[3]+1832 (0x0728)       @ --> call 0x0722 Parse and update BDV
0x02BD (0x00057A) 0x7E03-0x0762 f:00077 d:   3 | R = OR[3]+1890 (0x0762)       @ --> call 0x075C Parse CPW
0x02BF (0x00057E) 0x7E03-0x0516 f:00077 d:   3 | R = OR[3]+1302 (0x0516)       @ --> call 0x0510
0x02C1 (0x000582) 0x2123-       f:00020 d: 291 | A = OR[291]                   
0x02C2 (0x000584) 0xAE03-0x0006 f:00127 d:   3 | P = OR[3]+6 (0x0006), A # 0   @ --> jump to the beginning, pick next CHT. THIS IS WHERE WE LOOP FOR THE SECOND CHT ENTRY IN SIMULATION!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
0x02C4 (0x000588) 0x212B-       f:00020 d: 299 | A = OR[299]                   
0x02C5 (0x00058A) 0x1403-       f:00012 d:   3 | A = A + 3 (0x0003)            
0x02C6 (0x00058C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x02C7 (0x00058E) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x02C8 (0x000590) 0x080E-       f:00004 d:  14 | A = A > 14 (0x000E)           
0x02C9 (0x000592) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x02CA (0x000594) 0x2920-       f:00024 d: 288 | OR[288] = A                   
0x02CB (0x000596) 0x212B-       f:00020 d: 299 | A = OR[299]                   
0x02CC (0x000598) 0x1403-       f:00012 d:   3 | A = A + 3 (0x0003)            
0x02CD (0x00059A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x02CE (0x00059C) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x02CF (0x00059E) 0x080C-       f:00004 d:  12 | A = A > 12 (0x000C)           
0x02D0 (0x0005A0) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x02D1 (0x0005A2) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x02D2 (0x0005A4) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x02D3 (0x0005A6) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x02D4 (0x0005A8) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x02D5 (0x0005AA) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x02D6 (0x0005AC) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x02D7 (0x0005AE) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x02D8 (0x0005B0) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x02D9 (0x0005B2) 0x0A02-       f:00005 d:   2 | A = A < 2 (0x0002)            
0x02DA (0x0005B4) 0x2513-       f:00022 d: 275 | A = A + OR[275]               
0x02DB (0x0005B6) 0x0C02-       f:00006 d:   2 | A = A >> 2 (0x0002)           
0x02DC (0x0005B8) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x02DD (0x0005BA) 0x2120-       f:00020 d: 288 | A = OR[288]                   
0x02DE (0x0005BC) 0x8602-       f:00103 d:   2 | P = P + 2 (0x02E0), A # 0     
0x02DF (0x0005BE) 0x7097-       f:00070 d: 151 | P = P + 151 (0x0376)          
0x02E0 (0x0005C0) 0x3138-       f:00030 d: 312 | A = (OR[312])                 
0x02E1 (0x0005C2) 0x0E08-       f:00007 d:   8 | A = A << 8 (0x0008)           
0x02E2 (0x0005C4) 0x0A01-       f:00005 d:   1 | A = A < 1 (0x0001)            
0x02E3 (0x0005C6) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x02E4 (0x0005C8) 0x0C09-       f:00006 d:   9 | A = A >> 9 (0x0009)           
0x02E5 (0x0005CA) 0x3938-       f:00034 d: 312 | (OR[312]) = A                 
0x02E6 (0x0005CC) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x02E7 (0x0005CE) 0x5800-       f:00054 d:   0 | B = A                         
0x02E8 (0x0005D0) 0xF200-       f:00171 d:   0 | IOB , fn011                   @ Read byte counter
0x02E9 (0x0005D2) 0xF200-       f:00171 d:   0 | IOB , fn011                   @ Read byte counter
0x02EA (0x0005D4) 0x2925-       f:00024 d: 293 | OR[293] = A                   
0x02EB (0x0005D6) 0x213B-       f:00020 d: 315 | A = OR[315]                   @ CPW@IN
0x02EC (0x0005D8) 0x8402-       f:00102 d:   2 | P = P + 2 (0x02EE), A = 0     
0x02ED (0x0005DA) 0x7011-       f:00070 d:  17 | P = P + 17 (0x02FE)           
0x02EE (0x0005DC) 0x2125-       f:00020 d: 293 | A = OR[293]                   
0x02EF (0x0005DE) 0x8602-       f:00103 d:   2 | P = P + 2 (0x02F1), A # 0     
0x02F0 (0x0005E0) 0x700E-       f:00070 d:  14 | P = P + 14 (0x02FE)           
0x02F1 (0x0005E2) 0x212E-       f:00020 d: 302 | A = OR[302]                   
0x02F2 (0x0005E4) 0x1202-       f:00011 d:   2 | A = A & 2 (0x0002)            
0x02F3 (0x0005E6) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x02F4 (0x0005E8) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x02F5 (0x0005EA) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x02F6 (0x0005EC) 0x8402-       f:00102 d:   2 | P = P + 2 (0x02F8), A = 0     
0x02F7 (0x0005EE) 0x7007-       f:00070 d:   7 | P = P + 7 (0x02FE)            
0x02F8 (0x0005F0) 0x1005-       f:00010 d:   5 | A = 5 (0x0005)                
0x02F9 (0x0005F2) 0x292F-       f:00024 d: 303 | OR[303] = A                   
0x02FA (0x0005F4) 0x7E03-0x06FF f:00077 d:   3 | R = OR[3]+1791 (0x06FF)       @ --> 0x06F9 Set BDV and CPB status to 4, increment CPB@15, DO SOME STUFF!!!, zero CNT@NP and POP next activity if exists
0x02FC (0x0005F8) 0x7A03-0x0006 f:00075 d:   3 | P = OR[3]+6 (0x0006)          @ --> jump to the beginning, we seem to be in an endless loop here!!!
0x02FE (0x0005FC) 0x213C-       f:00020 d: 316 | A = OR[316]                   @ CPW@MS
0x02FF (0x0005FE) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0301), A = 0     
0x0300 (0x000600) 0x700A-       f:00070 d:  10 | P = P + 10 (0x030A)           
0x0301 (0x000602) 0x2138-       f:00020 d: 312 | A = OR[312]                   
0x0302 (0x000604) 0x1403-       f:00012 d:   3 | A = A + 3 (0x0003)            
0x0303 (0x000606) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0304 (0x000608) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0305 (0x00060A) 0x2924-       f:00024 d: 292 | OR[292] = A                   
0x0306 (0x00060C) 0x2124-       f:00020 d: 292 | A = OR[292]                   
0x0307 (0x00060E) 0x2725-       f:00023 d: 293 | A = A - OR[293]               
0x0308 (0x000610) 0x2924-       f:00024 d: 292 | OR[292] = A                   
0x0309 (0x000612) 0x706C-       f:00070 d: 108 | P = P + 108 (0x0375)          
0x030A (0x000614) 0x213B-       f:00020 d: 315 | A = OR[315]                   @ CPW@IN
0x030B (0x000616) 0x8602-       f:00103 d:   2 | P = P + 2 (0x030D), A # 0     
0x030C (0x000618) 0x7052-       f:00070 d:  82 | P = P + 82 (0x035E)           
0x030D (0x00061A) 0x1800-0x1000 f:00014 d:   0 | A = 4096 (0x1000)             
0x030F (0x00061E) 0x2924-       f:00024 d: 292 | OR[292] = A                   
0x0310 (0x000620) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x0311 (0x000622) 0x2735-       f:00023 d: 309 | A = A - OR[309]               @ OR[309] - CPB@BP??
0x0312 (0x000624) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0314), A # 0     
0x0313 (0x000626) 0x7024-       f:00070 d:  36 | P = P + 36 (0x0337)           
0x0314 (0x000628) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x0315 (0x00062A) 0x2935-       f:00024 d: 309 | OR[309] = A                   @ OR[309] - CPB@BP??
0x0316 (0x00062C) 0x2125-       f:00020 d: 293 | A = OR[293]                   
0x0317 (0x00062E) 0x1E00-0x1000 f:00017 d:   0 | A = A - 4096 (0x1000)         
0x0319 (0x000632) 0x8602-       f:00103 d:   2 | P = P + 2 (0x031B), A # 0     
0x031A (0x000634) 0x701C-       f:00070 d:  28 | P = P + 28 (0x0336)           
0x031B (0x000636) 0x7E03-0x0775 f:00077 d:   3 | R = OR[3]+1909 (0x0775)       
0x031D (0x00063A) 0x7E03-0x078F f:00077 d:   3 | R = OR[3]+1935 (0x078F)       
0x031F (0x00063E) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x0320 (0x000640) 0x141E-       f:00012 d:  30 | A = A + 30 (0x001E)           
0x0321 (0x000642) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0322 (0x000644) 0x2138-       f:00020 d: 312 | A = OR[312]                   
0x0323 (0x000646) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0324 (0x000648) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x0325 (0x00064A) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x0326 (0x00064C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0327 (0x00064E) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0328 (0x000650) 0x0E0B-       f:00007 d:  11 | A = A << 11 (0x000B)          
0x0329 (0x000652) 0x0A01-       f:00005 d:   1 | A = A < 1 (0x0001)            
0x032A (0x000654) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x032B (0x000656) 0x0C0C-       f:00006 d:  12 | A = A >> 12 (0x000C)          
0x032C (0x000658) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x032D (0x00065A) 0x7E03-0x07C8 f:00077 d:   3 | R = OR[3]+1992 (0x07C8)       @ --> 0x07C2
0x032F (0x00065E) 0x7E03-0x081A f:00077 d:   3 | R = OR[3]+2074 (0x081A)       
0x0331 (0x000662) 0x2124-       f:00020 d: 292 | A = OR[292]                   
0x0332 (0x000664) 0x2725-       f:00023 d: 293 | A = A - OR[293]               
0x0333 (0x000666) 0x2924-       f:00024 d: 292 | OR[292] = A                   
0x0334 (0x000668) 0x7E03-0x07F6 f:00077 d:   3 | R = OR[3]+2038 (0x07F6)       @ --> 0x07F0 Toggle CPB@BP??
0x0336 (0x00066C) 0x7027-       f:00070 d:  39 | P = P + 39 (0x035D)           
0x0337 (0x00066E) 0x2124-       f:00020 d: 292 | A = OR[292]                   
0x0338 (0x000670) 0x2725-       f:00023 d: 293 | A = A - OR[293]               
0x0339 (0x000672) 0x2924-       f:00024 d: 292 | OR[292] = A                   
0x033A (0x000674) 0x2124-       f:00020 d: 292 | A = OR[292]                   
0x033B (0x000676) 0x8602-       f:00103 d:   2 | P = P + 2 (0x033D), A # 0     
0x033C (0x000678) 0x7021-       f:00070 d:  33 | P = P + 33 (0x035D)           
0x033D (0x00067A) 0x7E03-0x07F6 f:00077 d:   3 | R = OR[3]+2038 (0x07F6)       @ --> 0x07F0 Toggle CPB@BP??
0x033F (0x00067E) 0x2135-       f:00020 d: 309 | A = OR[309]                   @ OR[309] - CPB@BP??
0x0340 (0x000680) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0342), A = 0     
0x0341 (0x000682) 0x7009-       f:00070 d:   9 | P = P + 9 (0x034A)            
0x0342 (0x000684) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x0343 (0x000686) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x0344 (0x000688) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0345 (0x00068A) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0346 (0x00068C) 0x080E-       f:00004 d:  14 | A = A > 14 (0x000E)           
0x0347 (0x00068E) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x0348 (0x000690) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x0349 (0x000692) 0x7008-       f:00070 d:   8 | P = P + 8 (0x0351)            
0x034A (0x000694) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x034B (0x000696) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x034C (0x000698) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x034D (0x00069A) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x034E (0x00069C) 0x080D-       f:00004 d:  13 | A = A > 13 (0x000D)           
0x034F (0x00069E) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x0350 (0x0006A0) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x0351 (0x0006A2) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x0352 (0x0006A4) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0354), A = 0     
0x0353 (0x0006A6) 0x700A-       f:00070 d:  10 | P = P + 10 (0x035D)           
0x0354 (0x0006A8) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x0355 (0x0006AA) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x0356 (0x0006AC) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0357 (0x0006AE) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0358 (0x0006B0) 0x0E0B-       f:00007 d:  11 | A = A << 11 (0x000B)          
0x0359 (0x0006B2) 0x0A01-       f:00005 d:   1 | A = A < 1 (0x0001)            
0x035A (0x0006B4) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x035B (0x0006B6) 0x0C0C-       f:00006 d:  12 | A = A >> 12 (0x000C)          
0x035C (0x0006B8) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x035D (0x0006BA) 0x7018-       f:00070 d:  24 | P = P + 24 (0x0375)           
0x035E (0x0006BC) 0x1800-0x1000 f:00014 d:   0 | A = 4096 (0x1000)             
0x0360 (0x0006C0) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x0361 (0x0006C2) 0x2F13-       f:00027 d: 275 | OR[275] = OR[275] - 1         
0x0362 (0x0006C4) 0x2138-       f:00020 d: 312 | A = OR[312]                   
0x0363 (0x0006C6) 0x1403-       f:00012 d:   3 | A = A + 3 (0x0003)            
0x0364 (0x0006C8) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0365 (0x0006CA) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0366 (0x0006CC) 0x2924-       f:00024 d: 292 | OR[292] = A                   
0x0367 (0x0006CE) 0x2124-       f:00020 d: 292 | A = OR[292]                   
0x0368 (0x0006D0) 0x2313-       f:00021 d: 275 | A = A & OR[275]               
0x0369 (0x0006D2) 0x2924-       f:00024 d: 292 | OR[292] = A                   
0x036A (0x0006D4) 0x2124-       f:00020 d: 292 | A = OR[292]                   
0x036B (0x0006D6) 0x8402-       f:00102 d:   2 | P = P + 2 (0x036D), A = 0     
0x036C (0x0006D8) 0x7004-       f:00070 d:   4 | P = P + 4 (0x0370)            
0x036D (0x0006DA) 0x1800-0x1000 f:00014 d:   0 | A = 4096 (0x1000)             
0x036F (0x0006DE) 0x2924-       f:00024 d: 292 | OR[292] = A                   
0x0370 (0x0006E0) 0x2124-       f:00020 d: 292 | A = OR[292]                   
0x0371 (0x0006E2) 0x2725-       f:00023 d: 293 | A = A - OR[293]               
0x0372 (0x0006E4) 0x2924-       f:00024 d: 292 | OR[292] = A                   
0x0373 (0x0006E6) 0x7E03-0x07F6 f:00077 d:   3 | R = OR[3]+2038 (0x07F6)       @ --> 0x07F0 Toggle CPB@BP??
0x0375 (0x0006EA) 0x7006-       f:00070 d:   6 | P = P + 6 (0x037B)            
0x0376 (0x0006EC) 0x1800-0x1000 f:00014 d:   0 | A = 4096 (0x1000)             
0x0378 (0x0006F0) 0x2924-       f:00024 d: 292 | OR[292] = A                   
0x0379 (0x0006F2) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x037A (0x0006F4) 0x2935-       f:00024 d: 309 | OR[309] = A                   @ OR[309] - CPB@BP??
0x037B (0x0006F6) 0x7E03-0x081A f:00077 d:   3 | R = OR[3]+2074 (0x081A)       
0x037D (0x0006FA) 0x7E03-0x0775 f:00077 d:   3 | R = OR[3]+1909 (0x0775)       
0x037F (0x0006FE) 0x213C-       f:00020 d: 316 | A = OR[316]                   @ CPW@MS
0x0380 (0x000700) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0382), A # 0     
0x0381 (0x000702) 0x7038-       f:00070 d:  56 | P = P + 56 (0x03B9)           
0x0382 (0x000704) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x0383 (0x000706) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x0384 (0x000708) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0385 (0x00070A) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0386 (0x00070C) 0x0E04-       f:00007 d:   4 | A = A << 4 (0x0004)           
0x0387 (0x00070E) 0x0A01-       f:00005 d:   1 | A = A < 1 (0x0001)            
0x0388 (0x000710) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x0389 (0x000712) 0x0C05-       f:00006 d:   5 | A = A >> 5 (0x0005)           
0x038A (0x000714) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x038B (0x000716) 0x213B-       f:00020 d: 315 | A = OR[315]                   @ CPW@IN
0x038C (0x000718) 0x8602-       f:00103 d:   2 | P = P + 2 (0x038E), A # 0     
0x038D (0x00071A) 0x700E-       f:00070 d:  14 | P = P + 14 (0x039B)           
0x038E (0x00071C) 0x2124-       f:00020 d: 292 | A = OR[292]                   
0x038F (0x00071E) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0391), A # 0     
0x0390 (0x000720) 0x700A-       f:00070 d:  10 | P = P + 10 (0x039A)           
0x0391 (0x000722) 0x7E03-0x078F f:00077 d:   3 | R = OR[3]+1935 (0x078F)       
0x0393 (0x000726) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x0394 (0x000728) 0x141E-       f:00012 d:  30 | A = A + 30 (0x001E)           
0x0395 (0x00072A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0396 (0x00072C) 0x2138-       f:00020 d: 312 | A = OR[312]                   
0x0397 (0x00072E) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0398 (0x000730) 0x7E03-0x07C8 f:00077 d:   3 | R = OR[3]+1992 (0x07C8)       @ --> 0x07C2
0x039A (0x000734) 0x701E-       f:00070 d:  30 | P = P + 30 (0x03B8)           
0x039B (0x000736) 0x2120-       f:00020 d: 288 | A = OR[288]                   
0x039C (0x000738) 0x8402-       f:00102 d:   2 | P = P + 2 (0x039E), A = 0     
0x039D (0x00073A) 0x7019-       f:00070 d:  25 | P = P + 25 (0x03B6)           
0x039E (0x00073C) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x039F (0x00073E) 0x1412-       f:00012 d:  18 | A = A + 18 (0x0012)           
0x03A0 (0x000740) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x03A1 (0x000742) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x03A2 (0x000744) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x03A3 (0x000746) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x03A4 (0x000748) 0x1413-       f:00012 d:  19 | A = A + 19 (0x0013)           
0x03A5 (0x00074A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x03A6 (0x00074C) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x03A7 (0x00074E) 0x2914-       f:00024 d: 276 | OR[276] = A                   
0x03A8 (0x000750) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x03A9 (0x000752) 0x8604-       f:00103 d:   4 | P = P + 4 (0x03AD), A # 0     
0x03AA (0x000754) 0x2114-       f:00020 d: 276 | A = OR[276]                   
0x03AB (0x000756) 0x8602-       f:00103 d:   2 | P = P + 2 (0x03AD), A # 0     
0x03AC (0x000758) 0x7004-       f:00070 d:   4 | P = P + 4 (0x03B0)            
0x03AD (0x00075A) 0x7E03-0x07C8 f:00077 d:   3 | R = OR[3]+1992 (0x07C8)       @ --> 0x07C2
0x03AF (0x00075E) 0x7006-       f:00070 d:   6 | P = P + 6 (0x03B5)            
0x03B0 (0x000760) 0x2139-       f:00020 d: 313 | A = OR[313]                   @ CPW@CC
0x03B1 (0x000762) 0x8602-       f:00103 d:   2 | P = P + 2 (0x03B3), A # 0     
0x03B2 (0x000764) 0x7003-       f:00070 d:   3 | P = P + 3 (0x03B5)            
0x03B3 (0x000766) 0x7E03-0x07C8 f:00077 d:   3 | R = OR[3]+1992 (0x07C8)       @ --> 0x07C2
0x03B5 (0x00076A) 0x7003-       f:00070 d:   3 | P = P + 3 (0x03B8)            
0x03B6 (0x00076C) 0x7E03-0x07E3 f:00077 d:   3 | R = OR[3]+2019 (0x07E3)       @ --> 0x07DD Clear OR[308] offset 11, bit 14 or 13 depending on @ OR[309] (CPB@BP??) being 0
0x03B8 (0x000770) 0x7009-       f:00070 d:   9 | P = P + 9 (0x03C1)            
0x03B9 (0x000772) 0x213B-       f:00020 d: 315 | A = OR[315]                   @ CPW@IN
0x03BA (0x000774) 0x8602-       f:00103 d:   2 | P = P + 2 (0x03BC), A # 0     
0x03BB (0x000776) 0x7006-       f:00070 d:   6 | P = P + 6 (0x03C1)            
0x03BC (0x000778) 0x2138-       f:00020 d: 312 | A = OR[312]                   
0x03BD (0x00077A) 0x1403-       f:00012 d:   3 | A = A + 3 (0x0003)            
0x03BE (0x00077C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x03BF (0x00077E) 0x2124-       f:00020 d: 292 | A = OR[292]                   
0x03C0 (0x000780) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x03C1 (0x000782) 0x2120-       f:00020 d: 288 | A = OR[288]                   
0x03C2 (0x000784) 0x8602-       f:00103 d:   2 | P = P + 2 (0x03C4), A # 0     
0x03C3 (0x000786) 0x7004-       f:00070 d:   4 | P = P + 4 (0x03C7)            
0x03C4 (0x000788) 0x7A03-0x0060 f:00075 d:   3 | P = OR[3]+96 (0x0060)         
0x03C6 (0x00078C) 0x7003-       f:00070 d:   3 | P = P + 3 (0x03C9)            
0x03C7 (0x00078E) 0x7E03-0x0806 f:00077 d:   3 | R = OR[3]+2054 (0x0806)       @ --> 0x0800 - TPUSH current activity into CHT@QU (waiting for channel), than enable (BMX) interrupts
0x03C9 (0x000792) 0x7A03-0x0514 f:00075 d:   3 | P = OR[3]+1300 (0x0514)       @ --> 0x050e - jump to the beginning, we seem to be in an endless loop here!!!

@ Test if CHT@NP (sequence code) is 6
0x03CB (0x000796) 0x212C-       f:00020 d: 300 | A = OR[300]                   
0x03CC (0x000798) 0x1606-       f:00013 d:   6 | A = A - 6 (0x0006)            
0x03CD (0x00079A) 0x8403-       f:00102 d:   3 | P = P + 3 (0x03D0), A = 0     
0x03CE (0x00079C) 0x7A03-0x0512 f:00075 d:   3 | P = OR[3]+1298 (0x0512)       
0x03D0 (0x0007A0) 0x212B-       f:00020 d: 299 | A = OR[299]                   
0x03D1 (0x0007A2) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x03D2 (0x0007A4) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x03D3 (0x0007A6) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x03D4 (0x0007A8) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x03D5 (0x0007AA) 0x212B-       f:00020 d: 299 | A = OR[299]                   
0x03D6 (0x0007AC) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x03D7 (0x0007AE) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x03D8 (0x0007B0) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x03D9 (0x0007B2) 0x2919-       f:00024 d: 281 | OR[281] = A                   
0x03DA (0x0007B4) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x03DB (0x0007B6) 0x0808-       f:00004 d:   8 | A = A > 8 (0x0008)            
0x03DC (0x0007B8) 0x2919-       f:00024 d: 281 | OR[281] = A                   
0x03DD (0x0007BA) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x03DE (0x0007BC) 0x12F0-       f:00011 d: 240 | A = A & 240 (0x00F0)          
0x03DF (0x0007BE) 0x291A-       f:00024 d: 282 | OR[282] = A                   
0x03E0 (0x0007C0) 0x212B-       f:00020 d: 299 | A = OR[299]                   @ Load CHT@8 into OR[304]
0x03E1 (0x0007C2) 0x1408-       f:00012 d:   8 | A = A + 8 (0x0008)            
0x03E2 (0x0007C4) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x03E3 (0x0007C6) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x03E4 (0x0007C8) 0x2930-       f:00024 d: 304 | OR[304] = A                   
0x03E5 (0x0007CA) 0x2130-       f:00020 d: 304 | A = OR[304]                   
0x03E6 (0x0007CC) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x03E7 (0x0007CE) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x03E8 (0x0007D0) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x03E9 (0x0007D2) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x03EA (0x0007D4) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x03EB (0x0007D6) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x03EC (0x0007D8) 0x271A-       f:00023 d: 282 | A = A - OR[282]               
0x03ED (0x0007DA) 0x840C-       f:00102 d:  12 | P = P + 12 (0x03F9), A = 0    
0x03EE (0x0007DC) 0x2130-       f:00020 d: 304 | A = OR[304]                   
0x03EF (0x0007DE) 0x840A-       f:00102 d:  10 | P = P + 10 (0x03F9), A = 0    
0x03F0 (0x0007E0) 0x3130-       f:00030 d: 304 | A = (OR[304])                 
0x03F1 (0x0007E2) 0x2930-       f:00024 d: 304 | OR[304] = A                   
0x03F2 (0x0007E4) 0x2130-       f:00020 d: 304 | A = OR[304]                   
0x03F3 (0x0007E6) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x03F4 (0x0007E8) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x03F5 (0x0007EA) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x03F6 (0x0007EC) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x03F7 (0x0007EE) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x03F8 (0x0007F0) 0x720D-       f:00071 d:  13 | P = P - 13 (0x03EB)           
0x03F9 (0x0007F2) 0x2130-       f:00020 d: 304 | A = OR[304]                   
0x03FA (0x0007F4) 0x8403-       f:00102 d:   3 | P = P + 3 (0x03FD), A = 0     
0x03FB (0x0007F6) 0x7A03-0x0407 f:00075 d:   3 | P = OR[3]+1031 (0x0407)       
0x03FD (0x0007FA) 0x7E03-0x069F f:00077 d:   3 | R = OR[3]+1695 (0x069F)       @ --> 0x0699
0x03FF (0x0007FE) 0x7A03-0x0006 f:00075 d:   3 | P = OR[3]+6 (0x0006)          @ --> jump to the beginning, we seem to be in an endless loop here!!!
0x0401 (0x000802) 0x2130-       f:00020 d: 304 | A = OR[304]                   
0x0402 (0x000804) 0x1402-       f:00012 d:   2 | A = A + 2 (0x0002)            
0x0403 (0x000806) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0404 (0x000808) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0405 (0x00080A) 0x1A00-0xDFFF f:00015 d:   0 | A = A & 57343 (0xDFFF)        
0x0407 (0x00080E) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0408 (0x000810) 0x212E-       f:00020 d: 302 | A = OR[302]                   
0x0409 (0x000812) 0x121F-       f:00011 d:  31 | A = A & 31 (0x001F)           
0x040A (0x000814) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x040B (0x000816) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x040C (0x000818) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x040D (0x00081A) 0x8402-       f:00102 d:   2 | P = P + 2 (0x040F), A = 0     
0x040E (0x00081C) 0x7004-       f:00070 d:   4 | P = P + 4 (0x0412)            
0x040F (0x00081E) 0x7E03-0x0691 f:00077 d:   3 | R = OR[3]+1681 (0x0691)       
0x0411 (0x000822) 0x70FA-       f:00070 d: 250 | P = P + 250 (0x050B)          
0x0412 (0x000824) 0x2119-       f:00020 d: 281 | A = OR[281]                   
0x0413 (0x000826) 0x120F-       f:00011 d:  15 | A = A & 15 (0x000F)           
0x0414 (0x000828) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x0415 (0x00082A) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x0416 (0x00082C) 0x1610-       f:00013 d:  16 | A = A - 16 (0x0010)           
0x0417 (0x00082E) 0x8002-       f:00100 d:   2 | P = P + 2 (0x0419), C = 0     
0x0418 (0x000830) 0x700D-       f:00070 d:  13 | P = P + 13 (0x0425)           
0x0419 (0x000832) 0x2130-       f:00020 d: 304 | A = OR[304]                   
0x041A (0x000834) 0x1404-       f:00012 d:   4 | A = A + 4 (0x0004)            
0x041B (0x000836) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x041C (0x000838) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x041D (0x00083A) 0x2914-       f:00024 d: 276 | OR[276] = A                   
0x041E (0x00083C) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x041F (0x00083E) 0x2514-       f:00022 d: 276 | A = A + OR[276]               
0x0420 (0x000840) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x0421 (0x000842) 0x2931-       f:00024 d: 305 | OR[305] = A                   
0x0422 (0x000844) 0x3131-       f:00030 d: 305 | A = (OR[305])                 
0x0423 (0x000846) 0x2931-       f:00024 d: 305 | OR[305] = A                   
0x0424 (0x000848) 0x7003-       f:00070 d:   3 | P = P + 3 (0x0427)            
0x0425 (0x00084A) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0426 (0x00084C) 0x2931-       f:00024 d: 305 | OR[305] = A                   
0x0427 (0x00084E) 0x2131-       f:00020 d: 305 | A = OR[305]                   
0x0428 (0x000850) 0x8402-       f:00102 d:   2 | P = P + 2 (0x042A), A = 0     
0x0429 (0x000852) 0x7005-       f:00070 d:   5 | P = P + 5 (0x042E)            
0x042A (0x000854) 0x7E03-0x0691 f:00077 d:   3 | R = OR[3]+1681 (0x0691)       
0x042C (0x000858) 0x7A03-0x0006 f:00075 d:   3 | P = OR[3]+6 (0x0006)          @ --> jump to the beginning, we seem to be in an endless loop here!!!
0x042E (0x00085C) 0x2131-       f:00020 d: 305 | A = OR[305]                   
0x042F (0x00085E) 0x1405-       f:00012 d:   5 | A = A + 5 (0x0005)            
0x0430 (0x000860) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0431 (0x000862) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0432 (0x000864) 0x0806-       f:00004 d:   6 | A = A > 6 (0x0006)            
0x0433 (0x000866) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x0434 (0x000868) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x0435 (0x00086A) 0x2131-       f:00020 d: 305 | A = OR[305]                   
0x0436 (0x00086C) 0x1406-       f:00012 d:   6 | A = A + 6 (0x0006)            
0x0437 (0x00086E) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0438 (0x000870) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0439 (0x000872) 0x2914-       f:00024 d: 276 | OR[276] = A                   
0x043A (0x000874) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x043B (0x000876) 0x8404-       f:00102 d:   4 | P = P + 4 (0x043F), A = 0     
0x043C (0x000878) 0x2114-       f:00020 d: 276 | A = OR[276]                   
0x043D (0x00087A) 0x8402-       f:00102 d:   2 | P = P + 2 (0x043F), A = 0     
0x043E (0x00087C) 0x7004-       f:00070 d:   4 | P = P + 4 (0x0442)            
0x043F (0x00087E) 0x7E03-0x0691 f:00077 d:   3 | R = OR[3]+1681 (0x0691)       
0x0441 (0x000882) 0x70CA-       f:00070 d: 202 | P = P + 202 (0x050B)          
0x0442 (0x000884) 0x2131-       f:00020 d: 305 | A = OR[305]                   
0x0443 (0x000886) 0x1405-       f:00012 d:   5 | A = A + 5 (0x0005)            
0x0444 (0x000888) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0445 (0x00088A) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0446 (0x00088C) 0x1A00-0xFFEF f:00015 d:   0 | A = A & 65519 (0xFFEF)        
0x0448 (0x000890) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0449 (0x000892) 0x2131-       f:00020 d: 305 | A = OR[305]                   
0x044A (0x000894) 0x1405-       f:00012 d:   5 | A = A + 5 (0x0005)            
0x044B (0x000896) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x044C (0x000898) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x044D (0x00089A) 0x1A00-0xFFBF f:00015 d:   0 | A = A & 65471 (0xFFBF)        
0x044F (0x00089E) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0450 (0x0008A0) 0x2131-       f:00020 d: 305 | A = OR[305]                   
0x0451 (0x0008A2) 0x140A-       f:00012 d:  10 | A = A + 10 (0x000A)           
0x0452 (0x0008A4) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0453 (0x0008A6) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0454 (0x0008A8) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x0455 (0x0008AA) 0x3113-       f:00030 d: 275 | A = (OR[275])                 
0x0456 (0x0008AC) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x0457 (0x0008AE) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x0458 (0x0008B0) 0x1402-       f:00012 d:   2 | A = A + 2 (0x0002)            
0x0459 (0x0008B2) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x045A (0x0008B4) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x045B (0x0008B6) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x045C (0x0008B8) 0x2914-       f:00024 d: 276 | OR[276] = A                   
0x045D (0x0008BA) 0x1800-0x0E67 f:00014 d:   0 | A = 3687 (0x0E67)             
0x045F (0x0008BE) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0460 (0x0008C0) 0x2114-       f:00020 d: 276 | A = OR[276]                   
0x0461 (0x0008C2) 0x1610-       f:00013 d:  16 | A = A - 16 (0x0010)           
0x0462 (0x0008C4) 0x3508-       f:00032 d: 264 | A = A + (OR[264])             
0x0463 (0x0008C6) 0x2914-       f:00024 d: 276 | OR[276] = A                   
0x0464 (0x0008C8) 0x3114-       f:00030 d: 276 | A = (OR[276])                 
0x0465 (0x0008CA) 0x2914-       f:00024 d: 276 | OR[276] = A                   
0x0466 (0x0008CC) 0x2114-       f:00020 d: 276 | A = OR[276]                   
0x0467 (0x0008CE) 0x140C-       f:00012 d:  12 | A = A + 12 (0x000C)           
0x0468 (0x0008D0) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x0469 (0x0008D2) 0x310D-       f:00030 d: 269 | A = (OR[269])                 
0x046A (0x0008D4) 0x290E-       f:00024 d: 270 | OR[270] = A                   
0x046B (0x0008D6) 0x210E-       f:00020 d: 270 | A = OR[270]                   
0x046C (0x0008D8) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x046D (0x0008DA) 0x1601-       f:00013 d:   1 | A = A - 1 (0x0001)            
0x046E (0x0008DC) 0x290F-       f:00024 d: 271 | OR[271] = A                   
0x046F (0x0008DE) 0x210F-       f:00020 d: 271 | A = OR[271]                   
0x0470 (0x0008E0) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x0471 (0x0008E2) 0x290F-       f:00024 d: 271 | OR[271] = A                   
0x0472 (0x0008E4) 0x210E-       f:00020 d: 270 | A = OR[270]                   
0x0473 (0x0008E6) 0x1A00-0xFF00 f:00015 d:   0 | A = A & 65280 (0xFF00)        
0x0475 (0x0008EA) 0x250F-       f:00022 d: 271 | A = A + OR[271]               
0x0476 (0x0008EC) 0x290E-       f:00024 d: 270 | OR[270] = A                   
0x0477 (0x0008EE) 0x390D-       f:00034 d: 269 | (OR[269]) = A                 
0x0478 (0x0008F0) 0x210F-       f:00020 d: 271 | A = OR[271]                   
0x0479 (0x0008F2) 0x2131-       f:00020 d: 305 | A = OR[305]                   
0x047A (0x0008F4) 0x1403-       f:00012 d:   3 | A = A + 3 (0x0003)            
0x047B (0x0008F6) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x047C (0x0008F8) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x047D (0x0008FA) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x047E (0x0008FC) 0x1003-       f:00010 d:   3 | A = 3 (0x0003)                
0x047F (0x0008FE) 0x2B13-       f:00025 d: 275 | OR[275] = A + OR[275]         
0x0480 (0x000900) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x0481 (0x000902) 0x1404-       f:00012 d:   4 | A = A + 4 (0x0004)            
0x0482 (0x000904) 0x2914-       f:00024 d: 276 | OR[276] = A                   
0x0483 (0x000906) 0x3113-       f:00030 d: 275 | A = (OR[275])                 
0x0484 (0x000908) 0x2915-       f:00024 d: 277 | OR[277] = A                   
0x0485 (0x00090A) 0x2115-       f:00020 d: 277 | A = OR[277]                   
0x0486 (0x00090C) 0x2730-       f:00023 d: 304 | A = A - OR[304]               
0x0487 (0x00090E) 0x8408-       f:00102 d:   8 | P = P + 8 (0x048F), A = 0     
0x0488 (0x000910) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x0489 (0x000912) 0x2714-       f:00023 d: 276 | A = A - OR[276]               
0x048A (0x000914) 0x8405-       f:00102 d:   5 | P = P + 5 (0x048F), A = 0     
0x048B (0x000916) 0x2D13-       f:00026 d: 275 | OR[275] = OR[275] + 1         
0x048C (0x000918) 0x3113-       f:00030 d: 275 | A = (OR[275])                 
0x048D (0x00091A) 0x2915-       f:00024 d: 277 | OR[277] = A                   
0x048E (0x00091C) 0x7209-       f:00071 d:   9 | P = P - 9 (0x0485)            
0x048F (0x00091E) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x0490 (0x000920) 0x2714-       f:00023 d: 276 | A = A - OR[276]               
0x0491 (0x000922) 0xB434-       f:00132 d:  52 | R = OR[52], A = 0             
0x0492 (0x000924) 0x0000-       f:00000 d:   0 | PASS                          
0x0493 (0x000926) 0x2131-       f:00020 d: 305 | A = OR[305]                   
0x0494 (0x000928) 0x140A-       f:00012 d:  10 | A = A + 10 (0x000A)           
0x0495 (0x00092A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0496 (0x00092C) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x0497 (0x00092E) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0498 (0x000930) 0x2130-       f:00020 d: 304 | A = OR[304]                   
0x0499 (0x000932) 0x1403-       f:00012 d:   3 | A = A + 3 (0x0003)            
0x049A (0x000934) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x049B (0x000936) 0x2131-       f:00020 d: 305 | A = OR[305]                   
0x049C (0x000938) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x049D (0x00093A) 0x212B-       f:00020 d: 299 | A = OR[299]                   
0x049E (0x00093C) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x049F (0x00093E) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x04A0 (0x000940) 0x2130-       f:00020 d: 304 | A = OR[304]                   
0x04A1 (0x000942) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x04A2 (0x000944) 0x2131-       f:00020 d: 305 | A = OR[305]                   
0x04A3 (0x000946) 0x1407-       f:00012 d:   7 | A = A + 7 (0x0007)            
0x04A4 (0x000948) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x04A5 (0x00094A) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x04A6 (0x00094C) 0x2934-       f:00024 d: 308 | OR[308] = A                   
0x04A7 (0x00094E) 0x212B-       f:00020 d: 299 | A = OR[299]                   
0x04A8 (0x000950) 0x140D-       f:00012 d:  13 | A = A + 13 (0x000D)           
0x04A9 (0x000952) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x04AA (0x000954) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x04AB (0x000956) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x04AC (0x000958) 0x2131-       f:00020 d: 305 | A = OR[305]                   
0x04AD (0x00095A) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x04AE (0x00095C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x04AF (0x00095E) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x04B0 (0x000960) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x04B1 (0x000962) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x04B2 (0x000964) 0x2130-       f:00020 d: 304 | A = OR[304]                   
0x04B3 (0x000966) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x04B4 (0x000968) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x04B5 (0x00096A) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x04B6 (0x00096C) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x04B7 (0x00096E) 0x2914-       f:00024 d: 276 | OR[276] = A                   
0x04B8 (0x000970) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x04B9 (0x000972) 0x120F-       f:00011 d:  15 | A = A & 15 (0x000F)           
0x04BA (0x000974) 0x2514-       f:00022 d: 276 | A = A + OR[276]               
0x04BB (0x000976) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x04BC (0x000978) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x04BD (0x00097A) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x04BE (0x00097C) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x04BF (0x00097E) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x04C0 (0x000980) 0x1416-       f:00012 d:  22 | A = A + 22 (0x0016)           
0x04C1 (0x000982) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x04C2 (0x000984) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x04C3 (0x000986) 0x0A09-       f:00005 d:   9 | A = A < 9 (0x0009)            
0x04C4 (0x000988) 0x2513-       f:00022 d: 275 | A = A + OR[275]               
0x04C5 (0x00098A) 0x0C09-       f:00006 d:   9 | A = A >> 9 (0x0009)           
0x04C6 (0x00098C) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x04C7 (0x00098E) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x04C8 (0x000990) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x04C9 (0x000992) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x04CA (0x000994) 0x2131-       f:00020 d: 305 | A = OR[305]                   
0x04CB (0x000996) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x04CC (0x000998) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x04CD (0x00099A) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x04CE (0x00099C) 0x1A00-0xFF00 f:00015 d:   0 | A = A & 65280 (0xFF00)        
0x04D0 (0x0009A0) 0x2513-       f:00022 d: 275 | A = A + OR[275]               
0x04D1 (0x0009A2) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x04D2 (0x0009A4) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x04D3 (0x0009A6) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x04D4 (0x0009A8) 0x2918-       f:00024 d: 280 | OR[280] = A                   
0x04D5 (0x0009AA) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x04D6 (0x0009AC) 0x1415-       f:00012 d:  21 | A = A + 21 (0x0015)           
0x04D7 (0x0009AE) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x04D8 (0x0009B0) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x04D9 (0x0009B2) 0x1A00-0xFF00 f:00015 d:   0 | A = A & 65280 (0xFF00)        
0x04DB (0x0009B6) 0x2518-       f:00022 d: 280 | A = A + OR[280]               
0x04DC (0x0009B8) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x04DD (0x0009BA) 0x7E03-0x0728 f:00077 d:   3 | R = OR[3]+1832 (0x0728)       @ --> call 0x0722 Parse and update BDV
0x04DF (0x0009BE) 0x7E03-0x0762 f:00077 d:   3 | R = OR[3]+1890 (0x0762)       @ --> call 0x075C Parse CPW
0x04E1 (0x0009C2) 0x742F-       f:00072 d:  47 | R = P + 47 (0x0510)           @ --> call 0x0510
0x04E2 (0x0009C4) 0x2123-       f:00020 d: 291 | A = OR[291]                   
0x04E3 (0x0009C6) 0xAE03-0x0006 f:00127 d:   3 | P = OR[3]+6 (0x0006), A # 0   @ --> jump to the beginning, we seem to be in an endless loop here!!!
0x04E5 (0x0009CA) 0x2136-       f:00020 d: 310 | A = OR[310]                   
0x04E6 (0x0009CC) 0x8602-       f:00103 d:   2 | P = P + 2 (0x04E8), A # 0     
0x04E7 (0x0009CE) 0x701E-       f:00070 d:  30 | P = P + 30 (0x0505)           
0x04E8 (0x0009D0) 0x212E-       f:00020 d: 302 | A = OR[302]                   
0x04E9 (0x0009D2) 0x1202-       f:00011 d:   2 | A = A & 2 (0x0002)            
0x04EA (0x0009D4) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x04EB (0x0009D6) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x04EC (0x0009D8) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x04ED (0x0009DA) 0x8402-       f:00102 d:   2 | P = P + 2 (0x04EF), A = 0     
0x04EE (0x0009DC) 0x7017-       f:00070 d:  23 | P = P + 23 (0x0505)           
0x04EF (0x0009DE) 0x2133-       f:00020 d: 307 | A = OR[307]                   
0x04F0 (0x0009E0) 0x8402-       f:00102 d:   2 | P = P + 2 (0x04F2), A = 0     
0x04F1 (0x0009E2) 0x7011-       f:00070 d:  17 | P = P + 17 (0x0502)           
0x04F2 (0x0009E4) 0x2131-       f:00020 d: 305 | A = OR[305]                   
0x04F3 (0x0009E6) 0x1404-       f:00012 d:   4 | A = A + 4 (0x0004)            
0x04F4 (0x0009E8) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x04F5 (0x0009EA) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x04F6 (0x0009EC) 0x1A00-0xFF00 f:00015 d:   0 | A = A & 65280 (0xFF00)        
0x04F8 (0x0009F0) 0x144E-       f:00012 d:  78 | A = A + 78 (0x004E)           
0x04F9 (0x0009F2) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x04FA (0x0009F4) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x04FB (0x0009F6) 0x1415-       f:00012 d:  21 | A = A + 21 (0x0015)           
0x04FC (0x0009F8) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x04FD (0x0009FA) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x04FE (0x0009FC) 0x0A09-       f:00005 d:   9 | A = A < 9 (0x0009)            
0x04FF (0x0009FE) 0x144E-       f:00012 d:  78 | A = A + 78 (0x004E)           
0x0500 (0x000A00) 0x0C09-       f:00006 d:   9 | A = A >> 9 (0x0009)           
0x0501 (0x000A02) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0502 (0x000A04) 0x7E03-0x06BB f:00077 d:   3 | R = OR[3]+1723 (0x06BB)       @ --> 0x06B5 Set BDV and CPB status to 6 or 4 depending on various transfer conditions, POP next activity if exists
0x0504 (0x000A08) 0x7007-       f:00070 d:   7 | P = P + 7 (0x050B)            
0x0505 (0x000A0A) 0x74AA-       f:00072 d: 170 | R = P + 170 (0x05AF)          
0x0506 (0x000A0C) 0x2123-       f:00020 d: 291 | A = OR[291]                   
0x0507 (0x000A0E) 0xAE03-0x0006 f:00127 d:   3 | P = OR[3]+6 (0x0006), A # 0   @ --> jump to the beginning, we seem to be in an endless loop here!!!
0x0509 (0x000A12) 0x7E03-0x06F0 f:00077 d:   3 | R = OR[3]+1776 (0x06F0)       @ --> 0x06EA Set BDV and CPB status to 3, increment CPB@15, zero CNT@NP and POP next activity if exists
0x050B (0x000A16) 0x7003-       f:00070 d:   3 | P = P + 3 (0x050E)            
@ offset 2 is not 1,2,3 or 6
0x050C (0x000A18) 0x7E03-0x069F f:00077 d:   3 | R = OR[3]+1695 (0x069F)       @ --> 0x0699
0x050E (0x000A1C) 0x7A03-0x0006 f:00075 d:   3 | P = OR[3]+6 (0x0006)          @ --> jump to the beginning, we seem to be in an endless loop here!!!

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ This routine is entered at the beginning, after parsing the BDV and CPB structures
@@ INPUT:
@@    OR[303] - Load CHT@12, upper
@@    OR[302] - channel status response
@@              bit 0 - Attention
@@              bit 1 - Status modifier
@@              bit 2 - Control unit end
@@              bit 3 - Busy
@@              bit 4 - Channel end
@@              bit 5 - Device end
@@              bit 6 - Unit check
@@              bit 7 - Unit exception
@@    OR[314] - CPW@DT - 1 to indicate data transfer request
@@    OR[315] - CPW@IN - 1 for sense (read) 0 for command (write)
@@    OR[316] - CPW@MS - 1 if MOS transfer is requested (0 for command/sense type commands, 1 for data read/writes)
@@ OUTPUT:
@@    OR[291] - 0 if continue processing, 1 if no further processing is needed
0x0510 (0x000A20) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                @ Default return value - no further processing needed
0x0511 (0x000A22) 0x2923-       f:00024 d: 291 | OR[291] = A                   
0x0512 (0x000A24) 0x212D-       f:00020 d: 301 | A = OR[301]                   @ This probably contains the input tags
0x0513 (0x000A26) 0x1A00-0xC00C f:00015 d:   0 | A = A & 49164 (0xC00C)        @ Test for DISCONECT_IN, SELECT_IN, P-ROM_PARITY or BUS-IN_PARITY
0x0515 (0x000A2A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0516 (0x000A2C) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0517 (0x000A2E) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x0518 (0x000A30) 0x8602-       f:00103 d:   2 | P = P + 2 (0x051A), A # 0     @ Jump if any of those bits set
0x0519 (0x000A32) 0x701D-       f:00070 d:  29 | P = P + 29 (0x0536)           
@ DISCONECT_IN, SELECT_IN, P-ROM_PARITY or BUS-IN_PARITY is set
0x051A (0x000A34) 0x212D-       f:00020 d: 301 | A = OR[301]                   @ Test for DISCONNECT_IN
0x051B (0x000A36) 0x1208-       f:00011 d:   8 | A = A & 8 (0x0008)            
0x051C (0x000A38) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x051D (0x000A3A) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x051E (0x000A3C) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x051F (0x000A3E) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0521), A # 0     
0x0520 (0x000A40) 0x7011-       f:00070 d:  17 | P = P + 17 (0x0531)           
0x0521 (0x000A42) 0x2131-       f:00020 d: 305 | A = OR[305]                   
0x0522 (0x000A44) 0x1404-       f:00012 d:   4 | A = A + 4 (0x0004)            
0x0523 (0x000A46) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0524 (0x000A48) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0525 (0x000A4A) 0x1A00-0xFF00 f:00015 d:   0 | A = A & 65280 (0xFF00)        
0x0527 (0x000A4E) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0528 (0x000A50) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x0529 (0x000A52) 0x1415-       f:00012 d:  21 | A = A + 21 (0x0015)           
0x052A (0x000A54) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x052B (0x000A56) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x052C (0x000A58) 0x0E01-       f:00007 d:   1 | A = A << 1 (0x0001)           
0x052D (0x000A5A) 0x0A08-       f:00005 d:   8 | A = A < 8 (0x0008)            
0x052E (0x000A5C) 0x1400-       f:00012 d:   0 | A = A + 0 (0x0000)            
0x052F (0x000A5E) 0x0C09-       f:00006 d:   9 | A = A >> 9 (0x0009)           
0x0530 (0x000A60) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0531 (0x000A62) 0x1002-       f:00010 d:   2 | A = 2 (0x0002)                
0x0532 (0x000A64) 0x292F-       f:00024 d: 303 | OR[303] = A                   
0x0533 (0x000A66) 0x7E03-0x06FF f:00077 d:   3 | R = OR[3]+1791 (0x06FF)       @ --> 0x06F9 Set BDV and CPB status to 4, increment CPB@15, DO SOME STUFF!!!, zero CNT@NP and POP next activity if exists
0x0535 (0x000A6A) 0x7079-       f:00070 d: 121 | P = P + 121 (0x05AE)          @ Exit here
@ None of DISCONECT_IN, SELECT_IN, P-ROM_PARITY or BUS-IN_PARITY is set
0x0536 (0x000A6C) 0x212F-       f:00020 d: 303 | A = OR[303]                   @ See if CHT@12 is non-0. If it is non-0, call 0x06F9 below (set error code?) and exit
0x0537 (0x000A6E) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0539), A # 0     
0x0538 (0x000A70) 0x7004-       f:00070 d:   4 | P = P + 4 (0x053C)            
0x0539 (0x000A72) 0x7E03-0x06FF f:00077 d:   3 | R = OR[3]+1791 (0x06FF)       @ ---> 0x06F9 Set BDV and CPB status to 4, increment CPB@15, DO SOME STUFF!!!, zero CNT@NP and POP next activity if exists
0x053B (0x000A76) 0x7073-       f:00070 d: 115 | P = P + 115 (0x05AE)          @ Exit here
@ CHT@12 is 0
0x053C (0x000A78) 0x212E-       f:00020 d: 302 | A = OR[302]                   @ This contains the channel status response
0x053D (0x000A7A) 0x1210-       f:00011 d:  16 | A = A & 16 (0x0010)           @ Check CHANNEL_END (set if operation is compelted)
0x053E (0x000A7C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x053F (0x000A7E) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0540 (0x000A80) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x0541 (0x000A82) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0543), A # 0     
0x0542 (0x000A84) 0x7033-       f:00070 d:  51 | P = P + 51 (0x0575)           @ jump if CHANNEL_END is not set
0x0543 (0x000A86) 0x213A-       f:00020 d: 314 | A = OR[314]                   @ Test for MOS, DATA and WRITE
0x0544 (0x000A88) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0546), A # 0     
0x0545 (0x000A8A) 0x701D-       f:00070 d:  29 | P = P + 29 (0x0562)           
0x0546 (0x000A8C) 0x213C-       f:00020 d: 316 | A = OR[316]                   @ CPW@MS
0x0547 (0x000A8E) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0549), A # 0     
0x0548 (0x000A90) 0x701A-       f:00070 d:  26 | P = P + 26 (0x0562)           
0x0549 (0x000A92) 0x213B-       f:00020 d: 315 | A = OR[315]                   @ CPW@IN
0x054A (0x000A94) 0x8402-       f:00102 d:   2 | P = P + 2 (0x054C), A = 0     
0x054B (0x000A96) 0x7017-       f:00070 d:  23 | P = P + 23 (0x0562)           
@ We get here for MOS-based data output transfers (I think) - we set CPB@11 bit 13 or 14 to 1 depending on CPB@BP???
@ That is: we mark the proper buffer complete
0x054C (0x000A98) 0x2135-       f:00020 d: 309 | A = OR[309]                   @ OR[309] - CPB@BP??
0x054D (0x000A9A) 0x8402-       f:00102 d:   2 | P = P + 2 (0x054F), A = 0     
0x054E (0x000A9C) 0x700B-       f:00070 d:  11 | P = P + 11 (0x0559)           

0x054F (0x000A9E) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ Set CPB@11 bit 14 to 1
0x0550 (0x000AA0) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x0551 (0x000AA2) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0552 (0x000AA4) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0553 (0x000AA6) 0x0E02-       f:00007 d:   2 | A = A << 2 (0x0002)           
0x0554 (0x000AA8) 0x0A01-       f:00005 d:   1 | A = A < 1 (0x0001)            
0x0555 (0x000AAA) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x0556 (0x000AAC) 0x0C03-       f:00006 d:   3 | A = A >> 3 (0x0003)           
0x0557 (0x000AAE) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0558 (0x000AB0) 0x700A-       f:00070 d:  10 | P = P + 10 (0x0562)           

0x0559 (0x000AB2) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ Set CPB@11 bit 13 to 1
0x055A (0x000AB4) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x055B (0x000AB6) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x055C (0x000AB8) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x055D (0x000ABA) 0x0E03-       f:00007 d:   3 | A = A << 3 (0x0003)           
0x055E (0x000ABC) 0x0A01-       f:00005 d:   1 | A = A < 1 (0x0001)            
0x055F (0x000ABE) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x0560 (0x000AC0) 0x0C04-       f:00006 d:   4 | A = A >> 4 (0x0004)           
0x0561 (0x000AC2) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
@ We get here for all operations
0x0562 (0x000AC4) 0x212E-       f:00020 d: 302 | A = OR[302]                   @ Test the UNIT_CHECK bit in the status response
0x0563 (0x000AC6) 0x1240-       f:00011 d:  64 | A = A & 64 (0x0040)           
0x0564 (0x000AC8) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0565 (0x000ACA) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0566 (0x000ACC) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x0567 (0x000ACE) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0569), A # 0     
0x0568 (0x000AD0) 0x7006-       f:00070 d:   6 | P = P + 6 (0x056E)            
@ UNIT_CHECK is set
0x0569 (0x000AD2) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ Increment CPB@26
0x056A (0x000AD4) 0x141A-       f:00012 d:  26 | A = A + 26 (0x001A)           
0x056B (0x000AD6) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x056C (0x000AD8) 0x3D08-       f:00036 d: 264 | (OR[264]) = (OR[264]) + 1     
0x056D (0x000ADA) 0x7005-       f:00070 d:   5 | P = P + 5 (0x0572)            
@ UNIT_CHECK is not set
0x056E (0x000ADC) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ Increment CPB@25
0x056F (0x000ADE) 0x1419-       f:00012 d:  25 | A = A + 25 (0x0019)           
0x0570 (0x000AE0) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0571 (0x000AE2) 0x3D08-       f:00036 d: 264 | (OR[264]) = (OR[264]) + 1     

0x0572 (0x000AE4) 0x7E03-0x06E5 f:00077 d:   3 | R = OR[3]+1765 (0x06E5)       @ --> 0x06DF Set BDV and CPB status to 4, zero CNT@NP and POP next activity if exists --- WE EXIT PRCOESSING AFTER THE DATA TRANSFER HERE
0x0574 (0x000AE8) 0x703A-       f:00070 d:  58 | P = P + 58 (0x05AE)           @ and exit
@ CHANNEL_END is not set
0x0575 (0x000AEA) 0x212E-       f:00020 d: 302 | A = OR[302]                   @ Test the UNIT_CHECK bit in the status response
0x0576 (0x000AEC) 0x1240-       f:00011 d:  64 | A = A & 64 (0x0040)           
0x0577 (0x000AEE) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0578 (0x000AF0) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0579 (0x000AF2) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x057A (0x000AF4) 0x8602-       f:00103 d:   2 | P = P + 2 (0x057C), A # 0     
0x057B (0x000AF6) 0x7020-       f:00070 d:  32 | P = P + 32 (0x059B)           
@ UNIT_CHECK is set
0x057C (0x000AF8) 0x212E-       f:00020 d: 302 | A = OR[302]                   @ Test for STATUS_MODIFIER
0x057D (0x000AFA) 0x1202-       f:00011 d:   2 | A = A & 2 (0x0002)            
0x057E (0x000AFC) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x057F (0x000AFE) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0580 (0x000B00) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x0581 (0x000B02) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0583), A # 0     
0x0582 (0x000B04) 0x7019-       f:00070 d:  25 | P = P + 25 (0x059B)           
@ STATUS_MODIFIER is set
0x0583 (0x000B06) 0x212E-       f:00020 d: 302 | A = OR[302]                   @ Test for BUSY
0x0584 (0x000B08) 0x1208-       f:00011 d:   8 | A = A & 8 (0x0008)            
0x0585 (0x000B0A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0586 (0x000B0C) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0587 (0x000B0E) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x0588 (0x000B10) 0x8602-       f:00103 d:   2 | P = P + 2 (0x058A), A # 0     
0x0589 (0x000B12) 0x7011-       f:00070 d:  17 | P = P + 17 (0x059A)           @ BUSY is not set --> exit
@ BUSY is set (so far UNIT_CHECK, STATUS_MODIFIER and BUSY is set)
0x058A (0x000B14) 0x212E-       f:00020 d: 302 | A = OR[302]                   @ Test for CONTROL_UNIT_END
0x058B (0x000B16) 0x1204-       f:00011 d:   4 | A = A & 4 (0x0004)            
0x058C (0x000B18) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x058D (0x000B1A) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x058E (0x000B1C) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x058F (0x000B1E) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0591), A # 0     
0x0590 (0x000B20) 0x7004-       f:00070 d:   4 | P = P + 4 (0x0594)            
@ CONTROL_UNIT_END is set
0x0591 (0x000B22) 0x7E03-0x06BB f:00077 d:   3 | R = OR[3]+1723 (0x06BB)       @ --> 0x06B5 Set BDV and CPB status to 6 or 4 depending on various transfer conditions, POP next activity if exists
0x0593 (0x000B26) 0x7003-       f:00070 d:   3 | P = P + 3 (0x0596)            
@ CONTROL_UNIT_END is not set
0x0594 (0x000B28) 0x7E03-0x06DA f:00077 d:   3 | R = OR[3]+1754 (0x06DA)       @ --> 0x06D4 Set BDV and CPB status to 1, zero CNT@NP and POP next activity if exists

0x0596 (0x000B2C) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ Increment CPB@24
0x0597 (0x000B2E) 0x1418-       f:00012 d:  24 | A = A + 24 (0x0018)           
0x0598 (0x000B30) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0599 (0x000B32) 0x3D08-       f:00036 d: 264 | (OR[264]) = (OR[264]) + 1     
0x059A (0x000B34) 0x7014-       f:00070 d:  20 | P = P + 20 (0x05AE)           @ Exit
@ UNIT_CHECK is not set, or STATUS_MODIFIER is not set
0x059B (0x000B36) 0x212D-       f:00020 d: 301 | A = OR[301]                   @ Test for ADDRESS_MISCOMPARE
0x059C (0x000B38) 0x1A00-0x1000 f:00015 d:   0 | A = A & 4096 (0x1000)         
0x059E (0x000B3C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x059F (0x000B3E) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x05A0 (0x000B40) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x05A1 (0x000B42) 0x8602-       f:00103 d:   2 | P = P + 2 (0x05A3), A # 0     
0x05A2 (0x000B44) 0x700A-       f:00070 d:  10 | P = P + 10 (0x05AC)           
@ ADDRESS_MISCOMPARE is set
0x05A3 (0x000B46) 0x212C-       f:00020 d: 300 | A = OR[300]                   @ Test if CHT@NP (sequence code) is 6 - KIC$SR - start request-in           - used by interrupt handler
0x05A4 (0x000B48) 0x1606-       f:00013 d:   6 | A = A - 6 (0x0006)            
0x05A5 (0x000B4A) 0x8602-       f:00103 d:   2 | P = P + 2 (0x05A7), A # 0     
0x05A6 (0x000B4C) 0x7006-       f:00070 d:   6 | P = P + 6 (0x05AC)            
0x05A7 (0x000B4E) 0x1002-       f:00010 d:   2 | A = 2 (0x0002)                @ CHT@NP (sequence code) is 6 - KIC$SR - start request-in           - used by interrupt handler
0x05A8 (0x000B50) 0x292F-       f:00024 d: 303 | OR[303] = A                   @ Update OR[303] to 2 (it will get stored back to where it came from, CHT@12, in the call below)
0x05A9 (0x000B52) 0x7E03-0x06FF f:00077 d:   3 | R = OR[3]+1791 (0x06FF)       @ --> 0x06F9 Set BDV and CPB status to 4, increment CPB@15, DO SOME STUFF!!!, zero CNT@NP and POP next activity if exists
0x05AB (0x000B56) 0x7003-       f:00070 d:   3 | P = P + 3 (0x05AE)            @ Exit
@ ADDRESS_MISCOMPARE is not set or CHT@NP is already 6
0x05AC (0x000B58) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                @ Clear return value (need further processing) and return
0x05AD (0x000B5A) 0x2923-       f:00024 d: 291 | OR[291] = A                   
0x05AE (0x000B5C) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
0x05AF (0x000B5E) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x05B0 (0x000B60) 0x2923-       f:00024 d: 291 | OR[291] = A                   
0x05B1 (0x000B62) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x05B2 (0x000B64) 0x2937-       f:00024 d: 311 | OR[311] = A                   
0x05B3 (0x000B66) 0x213A-       f:00020 d: 314 | A = OR[314]                   @ CPW@DT
0x05B4 (0x000B68) 0x8602-       f:00103 d:   2 | P = P + 2 (0x05B6), A # 0     
0x05B5 (0x000B6A) 0x703C-       f:00070 d:  60 | P = P + 60 (0x05F1)           
0x05B6 (0x000B6C) 0x213B-       f:00020 d: 315 | A = OR[315]                   @ CPW@IN
0x05B7 (0x000B6E) 0x8602-       f:00103 d:   2 | P = P + 2 (0x05B9), A # 0     
0x05B8 (0x000B70) 0x7039-       f:00070 d:  57 | P = P + 57 (0x05F1)           
0x05B9 (0x000B72) 0x213C-       f:00020 d: 316 | A = OR[316]                   @ CPW@MS
0x05BA (0x000B74) 0x8602-       f:00103 d:   2 | P = P + 2 (0x05BC), A # 0     
0x05BB (0x000B76) 0x7036-       f:00070 d:  54 | P = P + 54 (0x05F1)           
0x05BC (0x000B78) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x05BD (0x000B7A) 0x1410-       f:00012 d:  16 | A = A + 16 (0x0010)           
0x05BE (0x000B7C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x05BF (0x000B7E) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x05C0 (0x000B80) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x05C1 (0x000B82) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x05C2 (0x000B84) 0x1411-       f:00012 d:  17 | A = A + 17 (0x0011)           
0x05C3 (0x000B86) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x05C4 (0x000B88) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x05C5 (0x000B8A) 0x2914-       f:00024 d: 276 | OR[276] = A                   
0x05C6 (0x000B8C) 0x2138-       f:00020 d: 312 | A = OR[312]                   
0x05C7 (0x000B8E) 0x1402-       f:00012 d:   2 | A = A + 2 (0x0002)            
0x05C8 (0x000B90) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x05C9 (0x000B92) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x05CA (0x000B94) 0x2915-       f:00024 d: 277 | OR[277] = A                   
0x05CB (0x000B96) 0x2138-       f:00020 d: 312 | A = OR[312]                   
0x05CC (0x000B98) 0x1403-       f:00012 d:   3 | A = A + 3 (0x0003)            
0x05CD (0x000B9A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x05CE (0x000B9C) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x05CF (0x000B9E) 0x2916-       f:00024 d: 278 | OR[278] = A                   
0x05D0 (0x000BA0) 0x2115-       f:00020 d: 277 | A = OR[277]                   
0x05D1 (0x000BA2) 0x2713-       f:00023 d: 275 | A = A - OR[275]               
0x05D2 (0x000BA4) 0x8003-       f:00100 d:   3 | P = P + 3 (0x05D5), C = 0     
0x05D3 (0x000BA6) 0x8402-       f:00102 d:   2 | P = P + 2 (0x05D5), A = 0     
0x05D4 (0x000BA8) 0x7002-       f:00070 d:   2 | P = P + 2 (0x05D6)            
0x05D5 (0x000BAA) 0x7004-       f:00070 d:   4 | P = P + 4 (0x05D9)            
0x05D6 (0x000BAC) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x05D7 (0x000BAE) 0x2937-       f:00024 d: 311 | OR[311] = A                   
0x05D8 (0x000BB0) 0x700D-       f:00070 d:  13 | P = P + 13 (0x05E5)           
0x05D9 (0x000BB2) 0x2115-       f:00020 d: 277 | A = OR[277]                   
0x05DA (0x000BB4) 0x2713-       f:00023 d: 275 | A = A - OR[275]               
0x05DB (0x000BB6) 0x8402-       f:00102 d:   2 | P = P + 2 (0x05DD), A = 0     
0x05DC (0x000BB8) 0x7009-       f:00070 d:   9 | P = P + 9 (0x05E5)            
0x05DD (0x000BBA) 0x2116-       f:00020 d: 278 | A = OR[278]                   
0x05DE (0x000BBC) 0x2714-       f:00023 d: 276 | A = A - OR[276]               
0x05DF (0x000BBE) 0x8003-       f:00100 d:   3 | P = P + 3 (0x05E2), C = 0     
0x05E0 (0x000BC0) 0x8402-       f:00102 d:   2 | P = P + 2 (0x05E2), A = 0     
0x05E1 (0x000BC2) 0x7002-       f:00070 d:   2 | P = P + 2 (0x05E3)            
0x05E2 (0x000BC4) 0x7003-       f:00070 d:   3 | P = P + 3 (0x05E5)            
0x05E3 (0x000BC6) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                
0x05E4 (0x000BC8) 0x2937-       f:00024 d: 311 | OR[311] = A                   
0x05E5 (0x000BCA) 0x2137-       f:00020 d: 311 | A = OR[311]                   
0x05E6 (0x000BCC) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x05E7 (0x000BCE) 0x2937-       f:00024 d: 311 | OR[311] = A                   
0x05E8 (0x000BD0) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x05E9 (0x000BD2) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x05EA (0x000BD4) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x05EB (0x000BD6) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x05EC (0x000BD8) 0x0E0C-       f:00007 d:  12 | A = A << 12 (0x000C)          
0x05ED (0x000BDA) 0x0A01-       f:00005 d:   1 | A = A < 1 (0x0001)            
0x05EE (0x000BDC) 0x2537-       f:00022 d: 311 | A = A + OR[311]               
0x05EF (0x000BDE) 0x0C0D-       f:00006 d:  13 | A = A >> 13 (0x000D)          
0x05F0 (0x000BE0) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x05F1 (0x000BE2) 0x212E-       f:00020 d: 302 | A = OR[302]                   
0x05F2 (0x000BE4) 0x1202-       f:00011 d:   2 | A = A & 2 (0x0002)            
0x05F3 (0x000BE6) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x05F4 (0x000BE8) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x05F5 (0x000BEA) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x05F6 (0x000BEC) 0x8603-       f:00103 d:   3 | P = P + 3 (0x05F9), A # 0     
0x05F7 (0x000BEE) 0x7A03-0x0632 f:00075 d:   3 | P = OR[3]+1586 (0x0632)       
0x05F9 (0x000BF2) 0x211B-       f:00020 d: 283 | A = OR[283]                   
0x05FA (0x000BF4) 0x1603-       f:00013 d:   3 | A = A - 3 (0x0003)            
0x05FB (0x000BF6) 0x8403-       f:00102 d:   3 | P = P + 3 (0x05FE), A = 0     
0x05FC (0x000BF8) 0x7A03-0x0628 f:00075 d:   3 | P = OR[3]+1576 (0x0628)       
0x05FE (0x000BFC) 0x212E-       f:00020 d: 302 | A = OR[302]                   
0x05FF (0x000BFE) 0x1204-       f:00011 d:   4 | A = A & 4 (0x0004)            
0x0600 (0x000C00) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0601 (0x000C02) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0602 (0x000C04) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x0603 (0x000C06) 0xAC03-0x060C f:00126 d:   3 | P = OR[3]+1548 (0x060C), A = 0
0x0605 (0x000C0A) 0x701D-       f:00070 d:  29 | P = P + 29 (0x0622)           
0x0606 (0x000C0C) 0x212B-       f:00020 d: 299 | A = OR[299]                   
0x0607 (0x000C0E) 0x140C-       f:00012 d:  12 | A = A + 12 (0x000C)           
0x0608 (0x000C10) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x0609 (0x000C12) 0x310D-       f:00030 d: 269 | A = (OR[269])                 
0x060A (0x000C14) 0x290E-       f:00024 d: 270 | OR[270] = A                   
0x060B (0x000C16) 0x210E-       f:00020 d: 270 | A = OR[270]                   
0x060C (0x000C18) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x060D (0x000C1A) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x060E (0x000C1C) 0x290F-       f:00024 d: 271 | OR[271] = A                   
0x060F (0x000C1E) 0x210F-       f:00020 d: 271 | A = OR[271]                   
0x0610 (0x000C20) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x0611 (0x000C22) 0x290F-       f:00024 d: 271 | OR[271] = A                   
0x0612 (0x000C24) 0x210E-       f:00020 d: 270 | A = OR[270]                   
0x0613 (0x000C26) 0x1A00-0xFF00 f:00015 d:   0 | A = A & 65280 (0xFF00)        
0x0615 (0x000C2A) 0x250F-       f:00022 d: 271 | A = A + OR[271]               
0x0616 (0x000C2C) 0x290E-       f:00024 d: 270 | OR[270] = A                   
0x0617 (0x000C2E) 0x390D-       f:00034 d: 269 | (OR[269]) = A                 
0x0618 (0x000C30) 0x210F-       f:00020 d: 271 | A = OR[271]                   
0x0619 (0x000C32) 0x2131-       f:00020 d: 305 | A = OR[305]                   
0x061A (0x000C34) 0x1405-       f:00012 d:   5 | A = A + 5 (0x0005)            
0x061B (0x000C36) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x061C (0x000C38) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x061D (0x000C3A) 0x0E0A-       f:00007 d:  10 | A = A << 10 (0x000A)          
0x061E (0x000C3C) 0x0A01-       f:00005 d:   1 | A = A < 1 (0x0001)            
0x061F (0x000C3E) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x0620 (0x000C40) 0x0C0B-       f:00006 d:  11 | A = A >> 11 (0x000B)          
0x0621 (0x000C42) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0622 (0x000C44) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x0623 (0x000C46) 0x1414-       f:00012 d:  20 | A = A + 20 (0x0014)           
0x0624 (0x000C48) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0625 (0x000C4A) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0626 (0x000C4C) 0x0A02-       f:00005 d:   2 | A = A < 2 (0x0002)            
0x0627 (0x000C4E) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x0628 (0x000C50) 0x0C02-       f:00006 d:   2 | A = A >> 2 (0x0002)           
0x0629 (0x000C52) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x062A (0x000C54) 0x74CF-       f:00072 d: 207 | R = P + 207 (0x06F9)          @ Set BDV and CPB status to 4, increment CPB@15, DO SOME STUFF!!!, zero CNT@NP and POP next activity if exists
0x062B (0x000C56) 0x705F-       f:00070 d:  95 | P = P + 95 (0x068A)           
0x062C (0x000C58) 0x212E-       f:00020 d: 302 | A = OR[302]                   
0x062D (0x000C5A) 0x1204-       f:00011 d:   4 | A = A & 4 (0x0004)            
0x062E (0x000C5C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x062F (0x000C5E) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0630 (0x000C60) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x0631 (0x000C62) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0633), A = 0     
0x0632 (0x000C64) 0x701F-       f:00070 d:  31 | P = P + 31 (0x0651)           
0x0633 (0x000C66) 0x212B-       f:00020 d: 299 | A = OR[299]                   
0x0634 (0x000C68) 0x140C-       f:00012 d:  12 | A = A + 12 (0x000C)           
0x0635 (0x000C6A) 0x290D-       f:00024 d: 269 | OR[269] = A                   
0x0636 (0x000C6C) 0x310D-       f:00030 d: 269 | A = (OR[269])                 
0x0637 (0x000C6E) 0x290E-       f:00024 d: 270 | OR[270] = A                   
0x0638 (0x000C70) 0x210E-       f:00020 d: 270 | A = OR[270]                   
0x0639 (0x000C72) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x063A (0x000C74) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x063B (0x000C76) 0x290F-       f:00024 d: 271 | OR[271] = A                   
0x063C (0x000C78) 0x210F-       f:00020 d: 271 | A = OR[271]                   
0x063D (0x000C7A) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x063E (0x000C7C) 0x290F-       f:00024 d: 271 | OR[271] = A                   
0x063F (0x000C7E) 0x210E-       f:00020 d: 270 | A = OR[270]                   
0x0640 (0x000C80) 0x1A00-0xFF00 f:00015 d:   0 | A = A & 65280 (0xFF00)        
0x0642 (0x000C84) 0x250F-       f:00022 d: 271 | A = A + OR[271]               
0x0643 (0x000C86) 0x290E-       f:00024 d: 270 | OR[270] = A                   
0x0644 (0x000C88) 0x390D-       f:00034 d: 269 | (OR[269]) = A                 
0x0645 (0x000C8A) 0x210F-       f:00020 d: 271 | A = OR[271]                   
0x0646 (0x000C8C) 0x2131-       f:00020 d: 305 | A = OR[305]                   
0x0647 (0x000C8E) 0x1405-       f:00012 d:   5 | A = A + 5 (0x0005)            
0x0648 (0x000C90) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0649 (0x000C92) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x064A (0x000C94) 0x0E0A-       f:00007 d:  10 | A = A << 10 (0x000A)          
0x064B (0x000C96) 0x0A01-       f:00005 d:   1 | A = A < 1 (0x0001)            
0x064C (0x000C98) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x064D (0x000C9A) 0x0C0B-       f:00006 d:  11 | A = A >> 11 (0x000B)          
0x064E (0x000C9C) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x064F (0x000C9E) 0x7485-       f:00072 d: 133 | R = P + 133 (0x06D4)          @ Set BDV and CPB status to 1, zero CNT@NP and POP next activity if exists
0x0650 (0x000CA0) 0x703A-       f:00070 d:  58 | P = P + 58 (0x068A)           
0x0651 (0x000CA2) 0x212E-       f:00020 d: 302 | A = OR[302]                   
0x0652 (0x000CA4) 0x1280-       f:00011 d: 128 | A = A & 128 (0x0080)          
0x0653 (0x000CA6) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0654 (0x000CA8) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0655 (0x000CAA) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x0656 (0x000CAC) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0658), A = 0     
0x0657 (0x000CAE) 0x7013-       f:00070 d:  19 | P = P + 19 (0x066A)           
0x0658 (0x000CB0) 0x212E-       f:00020 d: 302 | A = OR[302]                   
0x0659 (0x000CB2) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x065A (0x000CB4) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x065B (0x000CB6) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x065C (0x000CB8) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x065D (0x000CBA) 0x8602-       f:00103 d:   2 | P = P + 2 (0x065F), A # 0     
0x065E (0x000CBC) 0x700C-       f:00070 d:  12 | P = P + 12 (0x066A)           
0x065F (0x000CBE) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x0660 (0x000CC0) 0x1414-       f:00012 d:  20 | A = A + 20 (0x0014)           
0x0661 (0x000CC2) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0662 (0x000CC4) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0663 (0x000CC6) 0x0E02-       f:00007 d:   2 | A = A << 2 (0x0002)           
0x0664 (0x000CC8) 0x0A01-       f:00005 d:   1 | A = A < 1 (0x0001)            
0x0665 (0x000CCA) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x0666 (0x000CCC) 0x0C03-       f:00006 d:   3 | A = A >> 3 (0x0003)           
0x0667 (0x000CCE) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0668 (0x000CD0) 0x7491-       f:00072 d: 145 | R = P + 145 (0x06F9)          @ Set BDV and CPB status to 4, increment CPB@15, DO SOME STUFF!!!, zero CNT@NP and POP next activity if exists
0x0669 (0x000CD2) 0x7021-       f:00070 d:  33 | P = P + 33 (0x068A)           
0x066A (0x000CD4) 0x212E-       f:00020 d: 302 | A = OR[302]                   
0x066B (0x000CD6) 0x1280-       f:00011 d: 128 | A = A & 128 (0x0080)          
0x066C (0x000CD8) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x066D (0x000CDA) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x066E (0x000CDC) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x066F (0x000CDE) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0671), A # 0     
0x0670 (0x000CE0) 0x700A-       f:00070 d:  10 | P = P + 10 (0x067A)           
0x0671 (0x000CE2) 0x212E-       f:00020 d: 302 | A = OR[302]                   
0x0672 (0x000CE4) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x0673 (0x000CE6) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0674 (0x000CE8) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0675 (0x000CEA) 0x2708-       f:00023 d: 264 | A = A - OR[264]               
0x0676 (0x000CEC) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0678), A # 0     
0x0677 (0x000CEE) 0x7003-       f:00070 d:   3 | P = P + 3 (0x067A)            
0x0678 (0x000CF0) 0x7472-       f:00072 d: 114 | R = P + 114 (0x06EA)          @ Set BDV and CPB status to 3, increment CPB@15, zero CNT@NP and POP next activity if exists
0x0679 (0x000CF2) 0x7011-       f:00070 d:  17 | P = P + 17 (0x068A)           
0x067A (0x000CF4) 0x2137-       f:00020 d: 311 | A = OR[311]                   
0x067B (0x000CF6) 0x8602-       f:00103 d:   2 | P = P + 2 (0x067D), A # 0     
0x067C (0x000CF8) 0x700C-       f:00070 d:  12 | P = P + 12 (0x0688)           
0x067D (0x000CFA) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x067E (0x000CFC) 0x1414-       f:00012 d:  20 | A = A + 20 (0x0014)           
0x067F (0x000CFE) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0680 (0x000D00) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0681 (0x000D02) 0x0E03-       f:00007 d:   3 | A = A << 3 (0x0003)           
0x0682 (0x000D04) 0x0A01-       f:00005 d:   1 | A = A < 1 (0x0001)            
0x0683 (0x000D06) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x0684 (0x000D08) 0x0C04-       f:00006 d:   4 | A = A >> 4 (0x0004)           
0x0685 (0x000D0A) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0686 (0x000D0C) 0x7473-       f:00072 d: 115 | R = P + 115 (0x06F9)          @ Set BDV and CPB status to 4, increment CPB@15, DO SOME STUFF!!!, zero CNT@NP and POP next activity if exists
0x0687 (0x000D0E) 0x7003-       f:00070 d:   3 | P = P + 3 (0x068A)            
0x0688 (0x000D10) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0689 (0x000D12) 0x2923-       f:00024 d: 291 | OR[291] = A                   
0x068A (0x000D14) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ POP next activity from CUT@5 OR[304] offset 5, if exists
0x068B (0x000D16) 0x2130-       f:00020 d: 304 | A = OR[304]                   
0x068C (0x000D18) 0x1405-       f:00012 d:   5 | A = A + 5 (0x0005)            
0x068D (0x000D1A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x068E (0x000D1C) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x068F (0x000D1E) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x0690 (0x000D20) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x0691 (0x000D22) 0x2B13-       f:00025 d: 275 | OR[275] = A + OR[275]         
0x0692 (0x000D24) 0x3113-       f:00030 d: 275 | A = (OR[275])                 
0x0693 (0x000D26) 0x8602-       f:00103 d:   2 | P = P + 2 (0x0695), A # 0     
0x0694 (0x000D28) 0x7003-       f:00070 d:   3 | P = P + 3 (0x0697)            
0x0695 (0x000D2A) 0x7E03-0x084A f:00077 d:   3 | R = OR[3]+2122 (0x084A)       @ --> 0x0844 POP from queue in OR[275]
0x0697 (0x000D2E) 0x7402-       f:00072 d:   2 | R = P + 2 (0x0699)            
0x0698 (0x000D30) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ POP next activity from CHT@QU OR[299], if exists
@ Otherwise, enable interrupts on REQUEST_IN
@ That is:
@   activate an activity if there's one waiting for the channel,
@   otherwise enable async interrupts
0x0699 (0x000D32) 0x212B-       f:00020 d: 299 | A = OR[299]                   @ POP next activity from queue OR[299] offset 4, if exists
0x069A (0x000D34) 0x1404-       f:00012 d:   4 | A = A + 4 (0x0004)            
0x069B (0x000D36) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x069C (0x000D38) 0x3113-       f:00030 d: 275 | A = (OR[275])                 
0x069D (0x000D3A) 0x8602-       f:00103 d:   2 | P = P + 2 (0x069F), A # 0     
0x069E (0x000D3C) 0x7004-       f:00070 d:   4 | P = P + 4 (0x06A2)            
0x069F (0x000D3E) 0x7E03-0x084A f:00077 d:   3 | R = OR[3]+2122 (0x084A)       @ --> 0x0844 POP from queue in OR[275]
0x06A1 (0x000D42) 0x7013-       f:00070 d:  19 | P = P + 19 (0x06B4)           

0x06A2 (0x000D44) 0x212B-       f:00020 d: 299 | A = OR[299]                   @ No activity to pop
0x06A3 (0x000D46) 0x1402-       f:00012 d:   2 | A = A + 2 (0x0002)            @ Put '4' into OR[299] offset 4
0x06A4 (0x000D48) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x06A5 (0x000D4A) 0x1004-       f:00010 d:   4 | A = 4 (0x0004)                
0x06A6 (0x000D4C) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x06A7 (0x000D4E) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                @ Set bit 12 of OR[284] - set interrupt on 'REQUEST_IN', set device address to 0
0x06A8 (0x000D50) 0x291C-       f:00024 d: 284 | OR[284] = A                   
0x06A9 (0x000D52) 0x211C-       f:00020 d: 284 | A = OR[284]                   
0x06AA (0x000D54) 0x0E03-       f:00007 d:   3 | A = A << 3 (0x0003)           
0x06AB (0x000D56) 0x0A02-       f:00005 d:   2 | A = A < 2 (0x0002)            
0x06AC (0x000D58) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x06AD (0x000D5A) 0x0C05-       f:00006 d:   5 | A = A >> 5 (0x0005)           
0x06AE (0x000D5C) 0x291C-       f:00024 d: 284 | OR[284] = A                   
0x06AF (0x000D5E) 0x2118-       f:00020 d: 280 | A = OR[280]                   
0x06B0 (0x000D60) 0x5800-       f:00054 d:   0 | B = A                         
0x06B1 (0x000D62) 0x211C-       f:00020 d: 284 | A = OR[284]                   
0x06B2 (0x000D64) 0xFC00-       f:00176 d:   0 | IOB , fn016                   @ Enter device address/mode
0x06B3 (0x000D66) 0xEE00-       f:00167 d:   0 | IOB , fn007                   @ Enable interrupts
0x06B4 (0x000D68) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Set BDV and CPB status to 6 or 4 depending on various transfer conditions, POP next activity if exists
0x06B5 (0x000D6A) 0x213A-       f:00020 d: 314 | A = OR[314]                   @ Test for CPW@DT being set, if not --> Set BDV and CPB status to 6, zero CNT@NP and POP next activity if exists
0x06B6 (0x000D6C) 0x8602-       f:00103 d:   2 | P = P + 2 (0x06B8), A # 0     
0x06B7 (0x000D6E) 0x7012-       f:00070 d:  18 | P = P + 18 (0x06C9)           @ Set BDV and CPB status to 6, zero CNT@NP and POP next activity if exists
0x06B8 (0x000D70) 0x213C-       f:00020 d: 316 | A = OR[316]                   @ Test for CPW@MS being set, if not --> Set BDV and CPB status to 6, zero CNT@NP and POP next activity if exists
0x06B9 (0x000D72) 0x8602-       f:00103 d:   2 | P = P + 2 (0x06BB), A # 0     
0x06BA (0x000D74) 0x700F-       f:00070 d:  15 | P = P + 15 (0x06C9)           @ Set BDV and CPB status to 6, zero CNT@NP and POP next activity if exists
@ We have a MOS-based data transfer
0x06BB (0x000D76) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ Test CPB@11, bit 12, if 0 --> Set BDV and CPB status to 6, zero CNT@NP and POP next activity if exists
0x06BC (0x000D78) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x06BD (0x000D7A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x06BE (0x000D7C) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x06BF (0x000D7E) 0x080C-       f:00004 d:  12 | A = A > 12 (0x000C)           
0x06C0 (0x000D80) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x06C1 (0x000D82) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x06C2 (0x000D84) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x06C3 (0x000D86) 0x8602-       f:00103 d:   2 | P = P + 2 (0x06C5), A # 0     
0x06C4 (0x000D88) 0x7005-       f:00070 d:   5 | P = P + 5 (0x06C9)            @ Set BDV and CPB status to 6, zero CNT@NP and POP next activity if exists
0x06C5 (0x000D8A) 0x1007-       f:00010 d:   7 | A = 7 (0x0007)                
0x06C6 (0x000D8C) 0x292F-       f:00024 d: 303 | OR[303] = A                   
0x06C7 (0x000D8E) 0x7432-       f:00072 d:  50 | R = P + 50 (0x06F9)           @ Set BDV and CPB status to 4, increment CPB@15, DO SOME STUFF!!!, zero CNT@NP and POP next activity if exists
0x06C8 (0x000D90) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Set BDV and CPB status to 6, zero CNT@NP and POP next activity if exists
0x06C9 (0x000D92) 0x1006-       f:00010 d:   6 | A = 6 (0x0006)                @ Status code
0x06CA (0x000D94) 0x2932-       f:00024 d: 306 | OR[306] = A                   
0x06CB (0x000D96) 0x74D8-       f:00072 d: 216 | R = P + 216 (0x07A3)          @ Set BDV, CPB status code and POP next activity if exists
0x06CC (0x000D98) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x06CD (0x000D9A) 0x292C-       f:00024 d: 300 | OR[300] = A                   
0x06CE (0x000D9C) 0x212B-       f:00020 d: 299 | A = OR[299]                   
0x06CF (0x000D9E) 0x1402-       f:00012 d:   2 | A = A + 2 (0x0002)            
0x06D0 (0x000DA0) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x06D1 (0x000DA2) 0x212C-       f:00020 d: 300 | A = OR[300]                   
0x06D2 (0x000DA4) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x06D3 (0x000DA6) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Set BDV and CPB status to 1, zero CNT@NP and POP next activity if exists
0x06D4 (0x000DA8) 0x1001-       f:00010 d:   1 | A = 1 (0x0001)                @ Status code
0x06D5 (0x000DAA) 0x2932-       f:00024 d: 306 | OR[306] = A                   
0x06D6 (0x000DAC) 0x74CD-       f:00072 d: 205 | R = P + 205 (0x07A3)          @ Set BDV, CPB status code and POP next activity if exists
0x06D7 (0x000DAE) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x06D8 (0x000DB0) 0x292C-       f:00024 d: 300 | OR[300] = A                   
0x06D9 (0x000DB2) 0x212B-       f:00020 d: 299 | A = OR[299]                   
0x06DA (0x000DB4) 0x1402-       f:00012 d:   2 | A = A + 2 (0x0002)            
0x06DB (0x000DB6) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x06DC (0x000DB8) 0x212C-       f:00020 d: 300 | A = OR[300]                   
0x06DD (0x000DBA) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x06DE (0x000DBC) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Set BDV and CPB status to 4, zero CNT@NP and POP next activity if exists
0x06DF (0x000DBE) 0x1004-       f:00010 d:   4 | A = 4 (0x0004)                @ Status code
0x06E0 (0x000DC0) 0x2932-       f:00024 d: 306 | OR[306] = A                   
0x06E1 (0x000DC2) 0x74C2-       f:00072 d: 194 | R = P + 194 (0x07A3)          @ Set BDV, CPB status code and POP next activity if exists
0x06E2 (0x000DC4) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x06E3 (0x000DC6) 0x292C-       f:00024 d: 300 | OR[300] = A                   
0x06E4 (0x000DC8) 0x212B-       f:00020 d: 299 | A = OR[299]                   
0x06E5 (0x000DCA) 0x1402-       f:00012 d:   2 | A = A + 2 (0x0002)            
0x06E6 (0x000DCC) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x06E7 (0x000DCE) 0x212C-       f:00020 d: 300 | A = OR[300]                   
0x06E8 (0x000DD0) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x06E9 (0x000DD2) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Set BDV and CPB status to 3, increment CPB@15, zero CNT@NP and POP next activity if exists
0x06EA (0x000DD4) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x06EB (0x000DD6) 0x140F-       f:00012 d:  15 | A = A + 15 (0x000F)           
0x06EC (0x000DD8) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x06ED (0x000DDA) 0x3D08-       f:00036 d: 264 | (OR[264]) = (OR[264]) + 1     
0x06EE (0x000DDC) 0x1003-       f:00010 d:   3 | A = 3 (0x0003)                @ Status code
0x06EF (0x000DDE) 0x2932-       f:00024 d: 306 | OR[306] = A                   
0x06F0 (0x000DE0) 0x74B3-       f:00072 d: 179 | R = P + 179 (0x07A3)          @ Set BDV, CPB status code and POP next activity if exists
0x06F1 (0x000DE2) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x06F2 (0x000DE4) 0x292C-       f:00024 d: 300 | OR[300] = A                   
0x06F3 (0x000DE6) 0x212B-       f:00020 d: 299 | A = OR[299]                   
0x06F4 (0x000DE8) 0x1402-       f:00012 d:   2 | A = A + 2 (0x0002)            
0x06F5 (0x000DEA) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x06F6 (0x000DEC) 0x212C-       f:00020 d: 300 | A = OR[300]                   
0x06F7 (0x000DEE) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x06F8 (0x000DF0) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Set BDV and CPB status to 4, increment CPB@15, DO SOME STUFF!!!, zero CNT@NP and POP next activity if exists
0x06F9 (0x000DF2) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ increment CPB@15 - number of processed CPWs
0x06FA (0x000DF4) 0x140F-       f:00012 d:  15 | A = A + 15 (0x000F)           
0x06FB (0x000DF6) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x06FC (0x000DF8) 0x3D08-       f:00036 d: 264 | (OR[264]) = (OR[264]) + 1     
0x06FD (0x000DFA) 0x212F-       f:00020 d: 303 | A = OR[303]                   @ Clear upper byte of CHT@12 (???), which is in OR[303]
0x06FE (0x000DFC) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x06FF (0x000DFE) 0x292F-       f:00024 d: 303 | OR[303] = A                   
0x0700 (0x000E00) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ Store CHT@12 (???), which is in OR[303] in CPB@OS
0x0701 (0x000E02) 0x1408-       f:00012 d:   8 | A = A + 8 (0x0008)            
0x0702 (0x000E04) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0703 (0x000E06) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0704 (0x000E08) 0x0A09-       f:00005 d:   9 | A = A < 9 (0x0009)            
0x0705 (0x000E0A) 0x252F-       f:00022 d: 303 | A = A + OR[303]               
0x0706 (0x000E0C) 0x0C09-       f:00006 d:   9 | A = A >> 9 (0x0009)           
0x0707 (0x000E0E) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0708 (0x000E10) 0x1004-       f:00010 d:   4 | A = 4 (0x0004)                @ Status code
0x0709 (0x000E12) 0x2932-       f:00024 d: 306 | OR[306] = A                   
0x070A (0x000E14) 0x7499-       f:00072 d: 153 | R = P + 153 (0x07A3)          @ Set BDV, CPB status code and POP next activity if exists
0x070B (0x000E16) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                @ Zero out CHT@NP (sequence code)
0x070C (0x000E18) 0x292C-       f:00024 d: 300 | OR[300] = A                   
0x070D (0x000E1A) 0x212B-       f:00020 d: 299 | A = OR[299]                   
0x070E (0x000E1C) 0x1402-       f:00012 d:   2 | A = A + 2 (0x0002)            
0x070F (0x000E1E) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0710 (0x000E20) 0x212C-       f:00020 d: 300 | A = OR[300]                   
0x0711 (0x000E22) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0712 (0x000E24) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Set BDV and CPB status to 5, increment CPB@27, zero CNT@NP and POP next activity if exists
0x0713 (0x000E26) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ Increment CPB@27 (some error counter?)
0x0714 (0x000E28) 0x141B-       f:00012 d:  27 | A = A + 27 (0x001B)           
0x0715 (0x000E2A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0716 (0x000E2C) 0x3D08-       f:00036 d: 264 | (OR[264]) = (OR[264]) + 1     
0x0717 (0x000E2E) 0x1005-       f:00010 d:   5 | A = 5 (0x0005)                @ Status code
0x0718 (0x000E30) 0x2932-       f:00024 d: 306 | OR[306] = A                   
0x0719 (0x000E32) 0x748A-       f:00072 d: 138 | R = P + 138 (0x07A3)          @ Set BDV, CPB status code and POP next activity if exists
0x071A (0x000E34) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                @ Set CHT@NP (sequence code) to 0
0x071B (0x000E36) 0x292C-       f:00024 d: 300 | OR[300] = A                   
0x071C (0x000E38) 0x212B-       f:00020 d: 299 | A = OR[299]                   
0x071D (0x000E3A) 0x1402-       f:00012 d:   2 | A = A + 2 (0x0002)            
0x071E (0x000E3C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x071F (0x000E3E) 0x212C-       f:00020 d: 300 | A = OR[300]                   
0x0720 (0x000E40) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0721 (0x000E42) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Parse and update BDV
@ Input:
@    OR[301] - input tags
@    OR[302] - status response
@    OR[305] - pointer to BDV table
@ Output:
@    OR[307] - BDV@5, bit 3
@    OR[308] - pointer to CPB
@    OR[309] - CPB@BP??
@    OR[310] - CPB@11, bit 7
@    OR[312] - pointer to CPW
0x0722 (0x000E44) 0x212E-       f:00020 d: 302 | A = OR[302]                   @ Mask status response?
0x0723 (0x000E46) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x0724 (0x000E48) 0x292E-       f:00024 d: 302 | OR[302] = A                   
0x0725 (0x000E4A) 0x2131-       f:00020 d: 305 | A = OR[305]                   @ Put the status response into the low byte of BDV@4 table
0x0726 (0x000E4C) 0x1404-       f:00012 d:   4 | A = A + 4 (0x0004)            
0x0727 (0x000E4E) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0728 (0x000E50) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0729 (0x000E52) 0x1A00-0xFF00 f:00015 d:   0 | A = A & 65280 (0xFF00)        
0x072B (0x000E56) 0x252E-       f:00022 d: 302 | A = A + OR[302]               
0x072C (0x000E58) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x072D (0x000E5A) 0x2131-       f:00020 d: 305 | A = OR[305]                   @ Put BDV@5, bit 3 into OR[307]
0x072E (0x000E5C) 0x1405-       f:00012 d:   5 | A = A + 5 (0x0005)            
0x072F (0x000E5E) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0730 (0x000E60) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0731 (0x000E62) 0x0803-       f:00004 d:   3 | A = A > 3 (0x0003)            
0x0732 (0x000E64) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x0733 (0x000E66) 0x2933-       f:00024 d: 307 | OR[307] = A                   
0x0734 (0x000E68) 0x2131-       f:00020 d: 305 | A = OR[305]                   @ Load CPB address from BDV@7 into OR[308]
0x0735 (0x000E6A) 0x1407-       f:00012 d:   7 | A = A + 7 (0x0007)            
0x0736 (0x000E6C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0737 (0x000E6E) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0738 (0x000E70) 0x2934-       f:00024 d: 308 | OR[308] = A                   
0x0739 (0x000E72) 0x212E-       f:00020 d: 302 | A = OR[302]                   @ Mask status response again (we've dirtied the upper byte above)
0x073A (0x000E74) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x073B (0x000E76) 0x292E-       f:00024 d: 302 | OR[302] = A                   
0x073C (0x000E78) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ Put the status response into the upper byte of CPB@21
0x073D (0x000E7A) 0x1415-       f:00012 d:  21 | A = A + 21 (0x0015)           
0x073E (0x000E7C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x073F (0x000E7E) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0740 (0x000E80) 0x0A09-       f:00005 d:   9 | A = A < 9 (0x0009)            
0x0741 (0x000E82) 0x252E-       f:00022 d: 302 | A = A + OR[302]               
0x0742 (0x000E84) 0x0C09-       f:00006 d:   9 | A = A >> 9 (0x0009)           
0x0743 (0x000E86) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0744 (0x000E88) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ Put OR[301] (input tags) into CPB@23
0x0745 (0x000E8A) 0x1417-       f:00012 d:  23 | A = A + 23 (0x0017)           
0x0746 (0x000E8C) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0747 (0x000E8E) 0x212D-       f:00020 d: 301 | A = OR[301]                   
0x0748 (0x000E90) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0749 (0x000E92) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ Put CPB@BP?? into OR[309]
0x074A (0x000E94) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x074B (0x000E96) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x074C (0x000E98) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x074D (0x000E9A) 0x080F-       f:00004 d:  15 | A = A > 15 (0x000F)           
0x074E (0x000E9C) 0x2935-       f:00024 d: 309 | OR[309] = A                   
0x074F (0x000E9E) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ Put bit 7 of CPB@11 into OR[310]
0x0750 (0x000EA0) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x0751 (0x000EA2) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0752 (0x000EA4) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0753 (0x000EA6) 0x0807-       f:00004 d:   7 | A = A > 7 (0x0007)            
0x0754 (0x000EA8) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x0755 (0x000EAA) 0x2936-       f:00024 d: 310 | OR[310] = A                   
0x0756 (0x000EAC) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ Load CPW address from CPB@CC into OR[312]     THIS IS THE ONLY POINT WE LOAD CPW IN THE REPRO
0x0757 (0x000EAE) 0x140E-       f:00012 d:  14 | A = A + 14 (0x000E)           
0x0758 (0x000EB0) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0759 (0x000EB2) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x075A (0x000EB4) 0x2938-       f:00024 d: 312 | OR[312] = A                   
0x075B (0x000EB6) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Parse CPW
@ Input: OR[312] - pointer to CPW
@ Output:
@    OR[313]: CPW@CC
@    OR[314]: CPW@DT
@    OR[315]: CPW@IN
@    OR[316]: CPW@MS
@    OR[283]: command code
0x075C (0x000EB8) 0x3138-       f:00030 d: 312 | A = (OR[312])                 
0x075D (0x000EBA) 0x080F-       f:00004 d:  15 | A = A > 15 (0x000F)           
0x075E (0x000EBC) 0x2939-       f:00024 d: 313 | OR[313] = A                   
0x075F (0x000EBE) 0x3138-       f:00030 d: 312 | A = (OR[312])                 
0x0760 (0x000EC0) 0x080E-       f:00004 d:  14 | A = A > 14 (0x000E)           
0x0761 (0x000EC2) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x0762 (0x000EC4) 0x293A-       f:00024 d: 314 | OR[314] = A                   
0x0763 (0x000EC6) 0x3138-       f:00030 d: 312 | A = (OR[312])                 
0x0764 (0x000EC8) 0x080D-       f:00004 d:  13 | A = A > 13 (0x000D)           
0x0765 (0x000ECA) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x0766 (0x000ECC) 0x293B-       f:00024 d: 315 | OR[315] = A                   
0x0767 (0x000ECE) 0x3138-       f:00030 d: 312 | A = (OR[312])                 
0x0768 (0x000ED0) 0x080C-       f:00004 d:  12 | A = A > 12 (0x000C)           
0x0769 (0x000ED2) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x076A (0x000ED4) 0x293C-       f:00024 d: 316 | OR[316] = A                   
0x076B (0x000ED6) 0x3138-       f:00030 d: 312 | A = (OR[312])                 
0x076C (0x000ED8) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x076D (0x000EDA) 0x291B-       f:00024 d: 283 | OR[283] = A                   
0x076E (0x000EDC) 0x0200-       f:00001 d:   0 | EXIT                          

0x076F (0x000EDE) 0x212B-       f:00020 d: 299 | A = OR[299]                   
0x0770 (0x000EE0) 0x140E-       f:00012 d:  14 | A = A + 14 (0x000E)           
0x0771 (0x000EE2) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0772 (0x000EE4) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0773 (0x000EE6) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x0774 (0x000EE8) 0x212B-       f:00020 d: 299 | A = OR[299]                   
0x0775 (0x000EEA) 0x140F-       f:00012 d:  15 | A = A + 15 (0x000F)           
0x0776 (0x000EEC) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0777 (0x000EEE) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0778 (0x000EF0) 0x2914-       f:00024 d: 276 | OR[276] = A                   
0x0779 (0x000EF2) 0x2114-       f:00020 d: 276 | A = OR[276]                   
0x077A (0x000EF4) 0x2524-       f:00022 d: 292 | A = A + OR[292]               
0x077B (0x000EF6) 0x2914-       f:00024 d: 276 | OR[276] = A                   
0x077C (0x000EF8) 0x8002-       f:00100 d:   2 | P = P + 2 (0x077E), C = 0     
0x077D (0x000EFA) 0x2D13-       f:00026 d: 275 | OR[275] = OR[275] + 1         
0x077E (0x000EFC) 0x212B-       f:00020 d: 299 | A = OR[299]                   
0x077F (0x000EFE) 0x140E-       f:00012 d:  14 | A = A + 14 (0x000E)           
0x0780 (0x000F00) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0781 (0x000F02) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x0782 (0x000F04) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0783 (0x000F06) 0x212B-       f:00020 d: 299 | A = OR[299]                   
0x0784 (0x000F08) 0x140F-       f:00012 d:  15 | A = A + 15 (0x000F)           
0x0785 (0x000F0A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0786 (0x000F0C) 0x2114-       f:00020 d: 276 | A = OR[276]                   
0x0787 (0x000F0E) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0788 (0x000F10) 0x0200-       f:00001 d:   0 | EXIT                          
0x0789 (0x000F12) 0x2138-       f:00020 d: 312 | A = OR[312]                   
0x078A (0x000F14) 0x1402-       f:00012 d:   2 | A = A + 2 (0x0002)            
0x078B (0x000F16) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x078C (0x000F18) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x078D (0x000F1A) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x078E (0x000F1C) 0x2138-       f:00020 d: 312 | A = OR[312]                   
0x078F (0x000F1E) 0x1403-       f:00012 d:   3 | A = A + 3 (0x0003)            
0x0790 (0x000F20) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0791 (0x000F22) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0792 (0x000F24) 0x2914-       f:00024 d: 276 | OR[276] = A                   
0x0793 (0x000F26) 0x2114-       f:00020 d: 276 | A = OR[276]                   
0x0794 (0x000F28) 0x2524-       f:00022 d: 292 | A = A + OR[292]               
0x0795 (0x000F2A) 0x2914-       f:00024 d: 276 | OR[276] = A                   
0x0796 (0x000F2C) 0x8002-       f:00100 d:   2 | P = P + 2 (0x0798), C = 0     
0x0797 (0x000F2E) 0x2D13-       f:00026 d: 275 | OR[275] = OR[275] + 1         
0x0798 (0x000F30) 0x2138-       f:00020 d: 312 | A = OR[312]                   
0x0799 (0x000F32) 0x1402-       f:00012 d:   2 | A = A + 2 (0x0002)            
0x079A (0x000F34) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x079B (0x000F36) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x079C (0x000F38) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x079D (0x000F3A) 0x2138-       f:00020 d: 312 | A = OR[312]                   
0x079E (0x000F3C) 0x1403-       f:00012 d:   3 | A = A + 3 (0x0003)            
0x079F (0x000F3E) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x07A0 (0x000F40) 0x2114-       f:00020 d: 276 | A = OR[276]                   
0x07A1 (0x000F42) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x07A2 (0x000F44) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Set BDV, CPB status code and POP next activity if exists
@ Input:
@     OR[306]: Status code
@     OR[305]: pointer to BDV table
@     OR[308]: pointer to CPB table
0x07A3 (0x000F46) 0x2132-       f:00020 d: 306 | A = OR[306]                   @ Clear upper 12 bits
0x07A4 (0x000F48) 0x120F-       f:00011 d:  15 | A = A & 15 (0x000F)           
0x07A5 (0x000F4A) 0x2932-       f:00024 d: 306 | OR[306] = A                   
0x07A6 (0x000F4C) 0x2131-       f:00020 d: 305 | A = OR[305]                   @ Put status code into BDV@5, upmost 4 bits
0x07A7 (0x000F4E) 0x1405-       f:00012 d:   5 | A = A + 5 (0x0005)            
0x07A8 (0x000F50) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x07A9 (0x000F52) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x07AA (0x000F54) 0x0A05-       f:00005 d:   5 | A = A < 5 (0x0005)            
0x07AB (0x000F56) 0x2532-       f:00022 d: 306 | A = A + OR[306]               
0x07AC (0x000F58) 0x0C05-       f:00006 d:   5 | A = A >> 5 (0x0005)           
0x07AD (0x000F5A) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x07AE (0x000F5C) 0x2132-       f:00020 d: 306 | A = OR[306]                   
0x07AF (0x000F5E) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x07B0 (0x000F60) 0x2932-       f:00024 d: 306 | OR[306] = A                   
0x07B1 (0x000F62) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ Put status code into CPB@OS
0x07B2 (0x000F64) 0x1408-       f:00012 d:   8 | A = A + 8 (0x0008)            
0x07B3 (0x000F66) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x07B4 (0x000F68) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x07B5 (0x000F6A) 0x1A00-0xFF00 f:00015 d:   0 | A = A & 65280 (0xFF00)        
0x07B7 (0x000F6E) 0x2532-       f:00022 d: 306 | A = A + OR[306]               
0x07B8 (0x000F70) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x07B9 (0x000F72) 0x2131-       f:00020 d: 305 | A = OR[305]                   @ Test if BDV@TQ is non-0, that is, if there's an activity awaiting device interrupts for this device, POP it
0x07BA (0x000F74) 0x1408-       f:00012 d:   8 | A = A + 8 (0x0008)            
0x07BB (0x000F76) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x07BC (0x000F78) 0x3113-       f:00030 d: 275 | A = (OR[275])                 
0x07BD (0x000F7A) 0x8602-       f:00103 d:   2 | P = P + 2 (0x07BF), A # 0     
0x07BE (0x000F7C) 0x7003-       f:00070 d:   3 | P = P + 3 (0x07C1)            
0x07BF (0x000F7E) 0x7E03-0x084A f:00077 d:   3 | R = OR[3]+2122 (0x084A)       @ --> 0x0844 POP from queue in OR[275]
0x07C1 (0x000F82) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ This routine probably adds an activity to a queue, but I'm not sure at the moment
@ Input:
@     OR[309] - original CPB@BP??
@     OR[308] - CPB
0x07C2 (0x000F84) 0x741B-       f:00072 d:  27 | R = P + 27 (0x07DD)           @ Clear CPB@11, bit bit 14 or 13 depending on OR[309] (CPB@BP??) being 0
0x07C3 (0x000F86) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ Load CPB@29 into OR[275]
0x07C4 (0x000F88) 0x141D-       f:00012 d:  29 | A = A + 29 (0x001D)           
0x07C5 (0x000F8A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x07C6 (0x000F8C) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x07C7 (0x000F8E) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x07C8 (0x000F90) 0x3113-       f:00030 d: 275 | A = (OR[275])                 @ Load OR[275] offset 0 into OR[276]
0x07C9 (0x000F92) 0x2914-       f:00024 d: 276 | OR[276] = A                   
0x07CA (0x000F94) 0x2113-       f:00020 d: 275 | A = OR[275]                   @ Load OR[275] offset 1 into OR[277]
0x07CB (0x000F96) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x07CC (0x000F98) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x07CD (0x000F9A) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x07CE (0x000F9C) 0x2915-       f:00024 d: 277 | OR[277] = A                   
0x07CF (0x000F9E) 0x0400-       f:00002 d:   0 | I = 0                         
0x07D0 (0x000FA0) 0x0000-       f:00000 d:   0 | PASS                          
0x07D1 (0x000FA2) 0x2134-       f:00020 d: 308 | A = OR[308]                   @ Load CPB into OR[29], OR[277] into OR[25] and call 0x1DDF
0x07D2 (0x000FA4) 0x281D-       f:00024 d:  29 | OR[29] = A                    
0x07D3 (0x000FA6) 0x2115-       f:00020 d: 277 | A = OR[277]                   
0x07D4 (0x000FA8) 0x2819-       f:00024 d:  25 | OR[25] = A                    
0x07D5 (0x000FAA) 0x7E00-0x1DDF f:00077 d:   0 | R = OR[0]+7647 (0x1DDF)       
0x07D7 (0x000FAE) 0x2114-       f:00020 d: 276 | A = OR[276]                   
0x07D8 (0x000FB0) 0x2896-       f:00024 d: 150 | OR[150] = A                   
0x07D9 (0x000FB2) 0x7E00-0x196C f:00077 d:   0 | R = OR[0]+6508 (0x196C)       
0x07DB (0x000FB6) 0x0600-       f:00003 d:   0 | I = 1                         
0x07DC (0x000FB8) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Clear OR[308] offset 11, bit 14 or 13 depending on @ OR[309] - CPB@BP??
0x07DD (0x000FBA) 0x2135-       f:00020 d: 309 | A = OR[309]                   @ OR[309] - CPB@BP??
0x07DE (0x000FBC) 0x8402-       f:00102 d:   2 | P = P + 2 (0x07E0), A = 0     
0x07DF (0x000FBE) 0x7009-       f:00070 d:   9 | P = P + 9 (0x07E8)            
0x07E0 (0x000FC0) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x07E1 (0x000FC2) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x07E2 (0x000FC4) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x07E3 (0x000FC6) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x07E4 (0x000FC8) 0x1A00-0xDFFF f:00015 d:   0 | A = A & 57343 (0xDFFF)        
0x07E6 (0x000FCC) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x07E7 (0x000FCE) 0x7008-       f:00070 d:   8 | P = P + 8 (0x07EF)            
0x07E8 (0x000FD0) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x07E9 (0x000FD2) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x07EA (0x000FD4) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x07EB (0x000FD6) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x07EC (0x000FD8) 0x1A00-0xBFFF f:00015 d:   0 | A = A & 49151 (0xBFFF)        
0x07EE (0x000FDC) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x07EF (0x000FDE) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Toggle CPB@BP??
0x07F0 (0x000FE0) 0x2135-       f:00020 d: 309 | A = OR[309]                   @ OR[309] - CPB@BP??
0x07F1 (0x000FE2) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x07F2 (0x000FE4) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x07F3 (0x000FE6) 0x2935-       f:00024 d: 309 | OR[309] = A                   @ OR[309] - CPB@BP??
0x07F4 (0x000FE8) 0x2135-       f:00020 d: 309 | A = OR[309]                   @ OR[309] - CPB@BP??
0x07F5 (0x000FEA) 0x1201-       f:00011 d:   1 | A = A & 1 (0x0001)            
0x07F6 (0x000FEC) 0x2935-       f:00024 d: 309 | OR[309] = A                   @ OR[309] - CPB@BP??
0x07F7 (0x000FEE) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x07F8 (0x000FF0) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x07F9 (0x000FF2) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x07FA (0x000FF4) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x07FB (0x000FF6) 0x0A02-       f:00005 d:   2 | A = A < 2 (0x0002)            
0x07FC (0x000FF8) 0x2535-       f:00022 d: 309 | A = A + OR[309]               @ OR[309] - CPB@BP??
0x07FD (0x000FFA) 0x0C02-       f:00006 d:   2 | A = A >> 2 (0x0002)           
0x07FE (0x000FFC) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x07FF (0x000FFE) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ TPUSH current activity into CHT@QU (waiting for channel), than enable (BMX) interrupts
0x0800 (0x001000) 0x0400-       f:00002 d:   0 | I = 0                         
0x0801 (0x001002) 0x0000-       f:00000 d:   0 | PASS                          
0x0802 (0x001004) 0x212B-       f:00020 d: 299 | A = OR[299]                   @ CHT table (16-12 = 4), so we effectively TPUSH an activity into CHT@QU
0x0803 (0x001006) 0x1410-       f:00012 d:  16 | A = A + 16 (0x0010)           
0x0804 (0x001008) 0x281D-       f:00024 d:  29 | OR[29] = A                    
0x0805 (0x00100A) 0x1800-0x0960 f:00014 d:   0 | A = 2400 (0x0960)             @ Timeout: 240 seconds, that is 4 minutes
0x0807 (0x00100E) 0x281C-       f:00024 d:  28 | OR[28] = A                    
0x0808 (0x001010) 0x7E00-0x1E02 f:00077 d:   0 | R = OR[0]+7682 (0x1E02)       @ Add activity to the waiting list OR[28] - timeout     OR[29] - context offset 12 (not value, but address)
0x080A (0x001014) 0x0600-       f:00003 d:   0 | I = 1                         
0x080B (0x001016) 0x212B-       f:00020 d: 299 | A = OR[299]                   @ Store back OR[300] (CHT@NP (sequence code)) to CHT@NP
0x080C (0x001018) 0x1402-       f:00012 d:   2 | A = A + 2 (0x0002)            
0x080D (0x00101A) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x080E (0x00101C) 0x212C-       f:00020 d: 300 | A = OR[300]                   
0x080F (0x00101E) 0x3908-       f:00034 d: 264 | (OR[264]) = A                 
0x0810 (0x001020) 0x2118-       f:00020 d: 280 | A = OR[280]                   @ Enable BMX interrupts
0x0811 (0x001022) 0x5800-       f:00054 d:   0 | B = A                         
0x0812 (0x001024) 0xEE00-       f:00167 d:   0 | IOB , fn007                   @ Enable interrupts
0x0813 (0x001026) 0x0200-       f:00001 d:   0 | EXIT                          

0x0814 (0x001028) 0x3131-       f:00030 d: 305 | A = (OR[305])                 
0x0815 (0x00102A) 0x12FF-       f:00011 d: 255 | A = A & 255 (0x00FF)          
0x0816 (0x00102C) 0x2913-       f:00024 d: 275 | OR[275] = A                   
0x0817 (0x00102E) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x0818 (0x001030) 0x140B-       f:00012 d:  11 | A = A + 11 (0x000B)           
0x0819 (0x001032) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x081A (0x001034) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x081B (0x001036) 0x2914-       f:00024 d: 276 | OR[276] = A                   
0x081C (0x001038) 0x213C-       f:00020 d: 316 | A = OR[316]                   @ CPW@MS
0x081D (0x00103A) 0x8402-       f:00102 d:   2 | P = P + 2 (0x081F), A = 0     
0x081E (0x00103C) 0x7007-       f:00070 d:   7 | P = P + 7 (0x0825)            
0x081F (0x00103E) 0x2138-       f:00020 d: 312 | A = OR[312]                   
0x0820 (0x001040) 0x1401-       f:00012 d:   1 | A = A + 1 (0x0001)            
0x0821 (0x001042) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0822 (0x001044) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0823 (0x001046) 0x2915-       f:00024 d: 277 | OR[277] = A                   
0x0824 (0x001048) 0x700F-       f:00070 d:  15 | P = P + 15 (0x0833)           
0x0825 (0x00104A) 0x2135-       f:00020 d: 309 | A = OR[309]                   
0x0826 (0x00104C) 0x8402-       f:00102 d:   2 | P = P + 2 (0x0828), A = 0     
0x0827 (0x00104E) 0x7007-       f:00070 d:   7 | P = P + 7 (0x082E)            
0x0828 (0x001050) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x0829 (0x001052) 0x140A-       f:00012 d:  10 | A = A + 10 (0x000A)           
0x082A (0x001054) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x082B (0x001056) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x082C (0x001058) 0x2915-       f:00024 d: 277 | OR[277] = A                   
0x082D (0x00105A) 0x7006-       f:00070 d:   6 | P = P + 6 (0x0833)            
0x082E (0x00105C) 0x2134-       f:00020 d: 308 | A = OR[308]                   
0x082F (0x00105E) 0x1409-       f:00012 d:   9 | A = A + 9 (0x0009)            
0x0830 (0x001060) 0x2908-       f:00024 d: 264 | OR[264] = A                   
0x0831 (0x001062) 0x3108-       f:00030 d: 264 | A = (OR[264])                 
0x0832 (0x001064) 0x2915-       f:00024 d: 277 | OR[277] = A                   
0x0833 (0x001066) 0x0400-       f:00002 d:   0 | I = 0                         
0x0834 (0x001068) 0x0000-       f:00000 d:   0 | PASS                          
0x0835 (0x00106A) 0x1012-       f:00010 d:  18 | A = 18 (0x0012)               
0x0836 (0x00106C) 0x29C3-       f:00024 d: 451 | OR[451] = A                   
0x0837 (0x00106E) 0x2113-       f:00020 d: 275 | A = OR[275]                   
0x0838 (0x001070) 0x29C4-       f:00024 d: 452 | OR[452] = A                   
0x0839 (0x001072) 0x2138-       f:00020 d: 312 | A = OR[312]                   
0x083A (0x001074) 0x29C5-       f:00024 d: 453 | OR[453] = A                   
0x083B (0x001076) 0x2114-       f:00020 d: 276 | A = OR[276]                   
0x083C (0x001078) 0x29C6-       f:00024 d: 454 | OR[454] = A                   
0x083D (0x00107A) 0x2115-       f:00020 d: 277 | A = OR[277]                   
0x083E (0x00107C) 0x29C7-       f:00024 d: 455 | OR[455] = A                   
0x083F (0x00107E) 0x2124-       f:00020 d: 292 | A = OR[292]                   
0x0840 (0x001080) 0x29C8-       f:00024 d: 456 | OR[456] = A                   
0x0841 (0x001082) 0x7DC2-       f:00076 d: 450 | R = OR[450]                   
0x0842 (0x001084) 0x0600-       f:00003 d:   0 | I = 1                         
0x0843 (0x001086) 0x0200-       f:00001 d:   0 | EXIT                          

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@2
@ Call POP
@ Input:
@     OR[275]: queue
0x0844 (0x001088) 0x1002-       f:00010 d:   2 | A = 2 (0x0002)                @ Kernel call: POP
0x0845 (0x00108A) 0x293D-       f:00024 d: 317 | OR[317] = A                   
0x0846 (0x00108C) 0x2113-       f:00020 d: 275 | A = OR[275]                   @ Queue from OR[275]
0x0847 (0x00108E) 0x293E-       f:00024 d: 318 | OR[318] = A                   
0x0848 (0x001090) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                @ Message for popped activity
0x0849 (0x001092) 0x293F-       f:00024 d: 319 | OR[319] = A                   
0x084A (0x001094) 0x113D-       f:00010 d: 317 | A = 317 (0x013D)              
0x084B (0x001096) 0x5800-       f:00054 d:   0 | B = A                         
0x084C (0x001098) 0x1000-       f:00010 d:   0 | A = 0 (0x0000)                
0x084D (0x00109A) 0x7C09-       f:00076 d:   9 | R = OR[9]                     
0x084E (0x00109C) 0x0200-       f:00001 d:   0 | EXIT                          
0x084F (0x00109E) 0x0000-       f:00000 d:   0 | PASS                          
0x0850 (0x0010A0) 0x0000-       f:00000 d:   0 | PASS                          
0x0851 (0x0010A2) 0x0000-       f:00000 d:   0 | PASS                          
0x0852 (0x0010A4) 0x0000-       f:00000 d:   0 | PASS                          
0x0853 (0x0010A6) 0x0000-       f:00000 d:   0 | PASS                          
