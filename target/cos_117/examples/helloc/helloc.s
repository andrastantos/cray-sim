entry @main
text: section code
@main: bss 0
lc a1,8
isub a1,a7,a1
isub a0,a5,a1
jap @estack
push a6
tr a6,b00
push a6
tr a6,a7
lba s1,D1
push s1
r @puts
inra a7
lc s7,0
tr a7,a6
pop a6
tr b00,a6
pop a6
j b00
rom:  section data
D1: = w.*
con X'48656C6C6F20434F
con X'5320776F726C6421
con X'0A00000000000000
text: section code
