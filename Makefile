.SUFFIXES: .c .u
CC= g++
CXXFLAGS= -O3 -Wall -std=c++11

LDFLAGS= -lm

LOBJECTS= taobaolda.o
LSOURCE= taobaolda.cpp

taobaolda:	$(LOBJECTS)
			$(CC) $(CXXFLAGS) $(LOBJECTS) -o taobaolda

clean:
	-rm -f *.o
