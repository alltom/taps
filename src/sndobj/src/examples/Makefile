##############################################################
#    SndObj g++ makefile
#    makes:    all
#              install
##############################################################

CC	=	g++
      
libDir  =   ../../lib
iDir    =    ../../include
binDir = ../../bin

LIBNAME = $(libDir)/libsndobj.a
LIB    = -L../../lib -lsndobj 

CFLAGS =  -O2 -I$(iDir) $(DEFS)
LFLAGS =  -lm -lpthread


progrs =  cvoc sinus

pdlib = morph~.pd_linux

all: $(progrs)

morph~: $(pdlib)

install: $(progrs)
	 mv $(progrs) $(binDir)

$(progrs): % : %.cpp 
	$(CC) -o $@ $(CFLAGS) $< $(LIB) $(LFLAGS)

$(pdlib): % : morph_tilde.cpp 
	$(CC) -shared -o../../lib/$@ $(CFLAGS) $< $(LIB)








