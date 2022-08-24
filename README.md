# ufv-operational-research
Contributors
###### - Rodrigo Oliveira - 2746
###### - João Vitor Azevedo - 6010
###### - Thiago Souza - 6011

#### Project developed in undergraduate course of Operational Research at the Federal University of Viçosa.
#### Prof. Pablo Luiz Araújo Munhoz - SIN 480

## ABOUT

In this project we implement a linear programming model from the article Relocation of passengers from flights canceled due to the closure of a transit airport (galoa-proceedings--sbpo-2021--139657.pdf).

## HOW TO RUN

After installing cplex on your machine, perform the following steps:

1- Open the Makefile file and change lines 25 and 26 to the address of cplex installation and concert on your machine.

2- With the terminal open in the project folder, compile the program with the following instruction:

`$ make`

3- To run the program we pass a parameter with the name of the input file:

`$ ./main input1.csv`

`$ ./main input2.csv`
