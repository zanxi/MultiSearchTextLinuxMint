# This Makefile will:
# Compile function14.c and function14.h into an object file function14.o
# Compile hw14.c into hw14.o
# Link and generate the executable hw14

cs: hw14.o function14.o
	gcc -o cs hw14.o function14.o

hw14.o: hw14.c function14.h
	gcc -c hw14.c

function14.o: function14.c function14.h
	gcc -c function14.c