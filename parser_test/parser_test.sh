#!/bin/bash
rm -f log.txt

cd ../cmake-build-debug

# cmake .. -DCMAKE_C_COMPILER=/opt/arm-gcc/bin/arm-linux-gnueabi-gcc -DCMAKE_CXX_COMPILER=/opt/arm-gcc/bin/arm-linux-gnueabi-g++

make

cd ../parser_test

number_of_tests=50
for ((i = 0; i < $number_of_tests; i++))
do
	let j=$i*2+2

	head -n $j tests.txt | tail -n 1 > input.txt
	head -n $j tests.txt | tail -n 1 > output.txt

	cd ../cmake-build-debug
    qemu-arm -L /opt/arm-sysroot ./parser_test < ../parser_test/input.txt > ../parser_test/program_output.txt
    cd ../parser_test

	if cmp -s output.txt program_output.txt ; then
 		echo "#test $i OK"
	else
		echo -e "test:\n" >> log.txt
    	cat input.txt >> log.txt
    	echo -e "answer:\n" >> log.txt
    	cat output.txt >> log.txt
    	echo -e "program output:\n" >> log.txt
    	cat program_output.txt >> log.txt
    	rm input.txt
    	echo "#test $i WA" && break
	fi
done

rm input.txt
rm output.txt
rm program_output.txt
