LIB_FLAG = -lgmpxx -lgmp
all: rsa test

test:	test.cpp
	g++ test.cpp -o test $(LIB_FLAG)

rsa:	rsa.cpp
	g++ rsa.cpp -o rsa $(LIB_FLAG)
