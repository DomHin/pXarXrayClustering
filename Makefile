ROOTCFLAGS	= $(shell root-config --cflags)
ROOTLIBS	= $(shell root-config --libs)
ROOTGLIBS	= $(shell root-config --glibs)

#USERLIBS	= -L lib/ 

CC 		= g++
CCFLAGS 	= -g -O -Wall

LD		= g++
LDFLAGS 	= -O 

########################################

CCFLAGS 	+= $(ROOTCFLAGS)
LIBS		= $(ROOTLIBS)
GLIBS		= $(ROOTGLIBS)

########################################

all: directories bin/clustering 

directories: 
	mkdir bin lib

lib/clustering.so: src/clustering.cc
	$(CC) $(CCFLAGS) -c src/clustering.cc -o lib/clustering.so

bin/clustering: lib/clustering.so
	$(LD) $(LDFLAGS) lib/clustering.so $(LIBS) -o bin/clustering

clean:
	rm *.o *~
