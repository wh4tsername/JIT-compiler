#!/bin/bash
rm -f log.txt

cd ../cmake-build-debug

make

cd ../tests

number_of_tests=50
for ((i = 0; i < $number_of_tests; i++))
do
	let j=$i*7+2
	for ((k = 0; k < 4; k++))
	do
		let l=j+k
		head -n $l tests.txt | tail -n 1 >> input.txt
	done

	let j=j+4
	head -n $j tests.txt | tail -n 1 > output.txt

	cd ../cmake-build-debug
    qemu-arm -L /opt/arm-sysroot ./JIT_COMPILER < ../tests/input.txt > ../tests/program_output.txt
    cd ../tests

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

	rm input.txt
done

rm output.txt
rm program_output.txt
