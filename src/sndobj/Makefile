#####################################################
# SndObj v2.5 top-level Makefile
# makes the library, examples and installs                
#
######################################################

# choose either win, oss, mosx, alsa or blank (no RT)
#PLATFORM =.win
#PLATFORM =.oss
#PLATFORM =.mosx
#PLATFORM = .alsa
PLATFORM =.win

LIBDEST = /usr/local/lib
INCDEST = /usr/local/include
MAKE = make

all: lib/libsndobj.a examples 

lib/libsndobj.a: src/*.cpp src/*.h 
	cd src; $(MAKE) -f Makefile$(PLATFORM)

examples: src/examples/*.cpp 
	cd src/examples; $(MAKE) -f Makefile$(PLATFORM) install

install: lib/libsndobj.a include/SndObj/*.h
	cp -rf include/*.h $(INCDEST)
	cp lib/libsndobj.a $(LIBDEST)

clean: 	
	rm src/*.o

cleanall: 	
	rm src/*.o
	rm lib/*
	rm bin/*
