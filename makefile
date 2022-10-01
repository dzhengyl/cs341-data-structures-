(CXX) = g++
(CXXFLAGS) = -Wall

all:
	make proj4
proj4: mytest.o file.o hash.o
	$(CXX) $(CXXFLAGS) mytest.o file.o hash.o -o proj4
mytest.o: mytest.cpp
	$(CXX) $(CXXFLAGS) -c mytest.cpp
file.o: file.cpp file.h
	$(CXX) $(CXXFLAGS) -c file.cpp
hash.o: hash.cpp hash.h
	$(CXX) $(CXXFLAGS) -c hash.cpp
clean:
	rm *.o proj4