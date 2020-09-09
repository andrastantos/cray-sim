c -aa test/jinstall_0.dat -aa test/jinstall_1.dat -aa test/jinstall_2.dat -aa test/jinstall_3.dat -o test/test.out
c -a test/acctdef.bin -o test/test.out
e -i test/acctdef.ds -x 0 test/test0.bin
e -i test/jinstall.job -xa 0 test/test0.txt  -xa 1 test/test1.txt  -xa 3 test/test3.txt  -xa 2 test/test2.txt
e -i test/jinstall.job -xalla test/test_all_ascii -xall test/test_all_bin -x 3 test/test3.txt
e -i test/jinstall.job -xalla test/test_all_ascii_%d.txt -xall test/test_all_bin_%d.bin -x 3 test/test3.txt
