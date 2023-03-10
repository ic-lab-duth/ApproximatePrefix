# Synthesis of Approximate Parallel Prefix Adders
This github repository contains the implementation of our method for generating approximate parallel prefix adders. The user can specify design constraints, and the method will return the verilog implementation of a select number of the smallest adders it could generate meeting those constraints.
 ## Compilation
 ```
 g++ -O3 PrefixAdder.hpp adder_gen.cpp -o adder_gen
 ```
 ## Usage
 To use the executable after compiling, pass the following 6 command line arguments in that order: 
 - Number of bits of the adder
 - Minimum carry chain length
 - Maximum logic levels
 - Maximum fanout
 - Number of solutions to report and generate verilog for
 - Name of the files generated

After execution, the information for the smallest adders found will be reported in a .csv file, and the verilog for the same adders will be written in the .sv files.

For example, if we want to generate 16 bit adders, with 11 minimum carry chain, 4 logic levels, maximum fanout of 5, report the 10 smallest solutions and store them in files named approx_adder, we would execute the following command:
```
./adder_gen 16 11 4 5 10 approx_adder 
```
The output of the console would be the following:
```
Generating solutions for 4 bits: 5 solutions generated.
Generating solutions for 5 bits: 16 solutions generated.
Generating solutions for 6 bits: 57 solutions generated.
Generating solutions for 7 bits: 232 solutions generated.
Generating solutions for 8 bits: 947 solutions generated.
Generating solutions for 9 bits: 3923 solutions generated.
Generating solutions for 10 bits: 14150 solutions generated.
Generating solutions for 11 bits: 31184 solutions generated.
Generating solutions for 12 bits: 39029 solutions generated.
Generating solutions for 13 bits: 39726 solutions generated.
Generating solutions for 14 bits: 42748 solutions generated.
Generating solutions for 15 bits: 39655 solutions generated.
Generating solutions for 16 bits: 34138 solutions generated.
Generating csv file...
Wrote results in approx_adder_report.csv
Generating verilog files...
Wrote verilog files in approx_adder_x.sv
Final solution number after pruning: 28439
adder size -> # solutions with this size
29 -> 1
30 -> 22
31 -> 194
32 -> 561
33 -> 1306
34 -> 2543
35 -> 3257
36 -> 3814
37 -> 3635
38 -> 3934
39 -> 4488
40 -> 4684
elapsed time: 0.254714m
elapsed time: 15.2828s
```
 In the end, the number of adders found for each operator number is also reported to give a better sense of the quality of solutions generated. 
 The verilog of the 10 smallest adders will be generated in the files ``approx_adder_0.sv``, ``approx_adder_1.sv`` ... ``approx_adder_9.sv``. 
 The characteristics of those 10 adders can be found in ``approx_adder_report.csv`` which will have contents similar to this:
 ```
 adder,level,min-carry-chain,max-carry-chain,max-fanout,#operators,EF-uniform,MRE-uniform,EF-mixed,MRE-mixed
approx_adder_0,4,11,14,5,29,0.00018,0.32404,0.00468,42.7644
approx_adder_1,4,11,14,5,30,0.00018,0.16386,0.005,47.7997
approx_adder_2,4,11,14,5,30,0.00014,1.11416,0.0042,40.3782
approx_adder_3,4,11,14,5,30,0.00024,0.81924,0.00436,43.1678
approx_adder_4,4,11,14,5,30,0.00014,1.07222,0.00408,38.2656
approx_adder_5,4,11,14,5,30,0.00024,1.07176,0.004,40.607
approx_adder_6,4,11,14,4,30,0.00012,0.13832,0.00414,40.0159
approx_adder_7,4,11,14,5,30,0.00022,1.69798,0.00406,34.2251
approx_adder_8,4,11,14,4,30,0.0003,2.19078,0.00444,41.9154
approx_adder_9,4,11,14,4,30,0.00024,0.37226,0.00436,42.3559

 ```