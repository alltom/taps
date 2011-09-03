CHUCK_DIR=../../../chuck_dev/v2

current:
	@echo "[tapestrea build]: please use one of the following:";
	@echo "   make linux-alsa, make linux-jack, make linux-oss"
	@echo "   make osx, make osx-ub, or make win32"

install:
	@echo "(reminder: must be root to install)"
	cp $(wildcard taps taps.exe) /usr/bin/; chmod 755 /usr/bin/$(wildcard taps taps.exe)

osx:
	-make -f makefile.osx

osx-ppc: 
	-make -f makefile.osx

osx-intel:
	-make -f makefile.osx-intel

osx-ub:
	-make -f makefile.osx-ub

linux-oss:
	-make -f makefile.oss 

linux-jack:
	-make -f makefile.jack

linux-alsa:
	-make -f makefile.alsa

win32: 
	@echo "No."
	-make -f makefile.win32

clean:
	rm -f *.o $(CHUCK_DIR)/*.o *~ $(wildcard taps taps.exe)
