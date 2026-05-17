file1 :
    g++ file1.cpp -std=c++23 -o out1
    ./out1 

file2 :
    g++ file2.cpp -std=c++23 -O3 -march=native -fopenmp -o out2
    ./out2

file3 :
    g++ file3.cpp -std=c++23 -o out3
    ./out3

file4 :
    g++ file4.cpp -std=c++23 -o out4
    ./out4