test: 	recordtype.cpp tester.cpp recordtypetests.cpp
		g++ -c recordtype.cpp tester.cpp recordtypetests.cpp
		g++ recordtype.o tester.o recordtypetests.o -o test
		./test
