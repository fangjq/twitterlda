CC= g++
CXXFLAGS= -O3 -Wall -std=c++11 -I./include

LDFLAGS= -lm

SRC=$(wildcard src/*.cpp)
ALL_OBJS=$(patsubst src/%.cpp, build/%.o, $(SRC))
BIN_TARGET= bin/twitterlda
DYLIB_TARGET= lib/libtwitterlda.so

.PHONY: clean all

all: $(ALL_OBJS) $(BIN_TARGET) $(DYLIB_TARGET)

build/%.o: src/%.cpp
	@mkdir -p $(@D)
	$(CC) $(CXXFLAGS) -MM -MT build/$*.o $< >build/$*.d
	$(CC) -c $(CXXFLAGS) $< -o $@

$(BIN_TARGET):	$(ALL_OBJS)
			@mkdir -p $(@D)
			$(CC) $(CXXFLAGS) $(ALL_OBJS) -o $@ $(LDFLAGS) 

$(DYLIB_TARGET): $(ALL_OBJS)
			@mkdir -p $(@D)
			$(CC) $(CXXFLAGS) $(ALL_OBJS) -o $@ $(LDFLAGS) -shared -fPIC 

clean:
	-rm -rf *.o *.so lib build bin
