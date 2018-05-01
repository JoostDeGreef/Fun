
AR=ar
CC=gcc
CPP=g++
CFLAGS=-W -Wall -O3 -DNDEBUG
CPPFLAGS=-W -Wall -O3 -DNDEBUG
#CFLAGS=
#CPPFLAGS=

OBJECTS:=$(patsubst %.c,%.c.o,$(patsubst %.cpp,%.cpp.o,$(patsubst %.cc,%.cc.o,$(patsubst %,tmp/%,${SOURCES}))))

DIRS:=$(sort $(dir ${OBJECTS})) bin

.PHONY: all
all: static

.PHONY: clean
clean:
	rm -f *~
	rm -rf tmp

.PHONY: static
static: tree bin/${NAME}

.PHONY: tree
tree:
	mkdir -p ${DIRS}

bin/${NAME}: ${OBJECTS} Makefile
	${CPP} -o bin/${NAME} ${OBJECTS} ${LIBS}

tmp/%.c.o: %.c Makefile
	${CC} ${CFLAGS} ${DEFINES} ${INCLUDE} -fPIC -o $@ -c $<
        
tmp/%.cc.o: %.cc Makefile
	${CPP} ${CPPFLAGS} ${DEFINES} ${INCLUDE} -fPIC -o $@ -c $<
        
tmp/%.cpp.o: %.cpp Makefile
	${CPP} ${CPPFLAGS} ${DEFINES} ${INCLUDE} -fPIC -o $@ -c $<
        

