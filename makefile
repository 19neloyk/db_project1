test: 	recordtype.cpp tester.cpp tests.cpp
		g++ -c recordtype.cpp tester.cpp tests.cpp
		g++ recordtype.o tester.o tests.o -o tests
		./tests
