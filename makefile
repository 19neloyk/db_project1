test: 	recordtype.cpp tester.cpp tests.cpp database.cpp
		g++ -c -std=c++11 recordtype.cpp tester.cpp tests.cpp database.cpp
		g++ recordtype.o tester.o tests.o database.o -o tests
		./tests
