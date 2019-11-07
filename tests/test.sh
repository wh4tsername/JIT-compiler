#!/bin/bash
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

	# cat input.txt

	# echo

	let j=j+4
	head -n $j tests.txt | tail -n 1 > output.txt

	# cat output.txt

	# echo

	cd ../cmake-build-debug
    qemu-arm -L /opt/arm-sysroot ./JIT_COMPILER < ../tests/input.txt > ../tests/program_output.txt
    cd ../tests

    # cat program_output.txt

    rm input.txt

	if cmp -s output.txt program_output.txt ; then
 		echo "#test $i OK"
	else
		echo "#test $i WA" && break
	fi
done
