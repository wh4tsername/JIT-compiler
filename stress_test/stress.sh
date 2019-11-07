g++ gen.cpp -std=c++17 -o gen

cd ../cmake-build-debug

cmake .. -DCMAKE_C_COMPILER=/opt/arm-gcc/bin/arm-linux-gnueabi-gcc -DCMAKE_CXX_COMPILER=/opt/arm-gcc/bin/arm-linux-gnueabi-g++

make

cd ../stress_test

for ((i = 0; i < 500; i++))
do
    ./gen $i > input.txt
    python3 checker.py < input.txt > out1.txt
    cd ../cmake-build-debug
    qemu-arm -L /opt/arm-sysroot ./JIT_COMPILER < ../stress_test/input.txt > ../stress_test/out2.txt
    cd ../stress_test
    diff out1.txt out2.txt || break
    echo test $i OK
done
