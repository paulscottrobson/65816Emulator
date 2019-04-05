python process.py
cp getset.h.new includes/getset.h
gcc -I includes -c cpuops.cpp -o cpu65816.o 
gcc cpu65816.o test.cpp