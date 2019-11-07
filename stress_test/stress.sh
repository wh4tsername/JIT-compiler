#!/bin/bash
rm -f log.txt

g++ gen.cpp -std=c++17 -o gen

cd ../cmake-build-debug

# cmake .. -DCMAKE_C_COMPILER=/opt/arm-gcc/bin/arm-linux-gnueabi-gcc -DCMAKE_CXX_COMPILER=/opt/arm-gcc/bin/arm-linux-gnueabi-g++

make

cd ../stress_test

for ((i = 0; i < 10; i++))
do
    ./gen $i > input.txt
    python3 checker.py < input.txt > out1.txt
    cd ../cmake-build-debug
    qemu-arm -L /opt/arm-sysroot ./JIT_COMPILER < ../stress_test/input.txt > ../stress_test/out2.txt
    cd ../stress_test
    if diff out1.txt out2.txt ; then
    	echo "test $i OK"
    else
    	echo -e "test:\n" >> log.txt
    	cat input.txt >> log.txt
    	echo -e "answer:\n" >> log.txt
    	cat out1.txt >> log.txt
    	echo -e "program output:\n" >> log.txt
    	cat out2.txt >> log.txt
    	echo "#test $i WA" && break
    fi
done

rm input.txt
rm out1.txt
rm out2.txt
rm gen
