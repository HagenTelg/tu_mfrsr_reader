CC	= /usr/bin/gcc
INC	= -IRsrlib -ISolarinfo
CFLAGS	= $(INC) -O
#CFLAGS	= $(INC) -g 
OBJS	= tu.o setup_pp.o getopt.o unpack.o sunae.o calib.o split.o 
LIBS	= -LRsrlib -LSolarinfo -lm -lrsr -lsi -ll

TAR	= tu calib.c calib.h cos.c getopt.c setup_pp.c split.c sunae.c sunae.h \
	  tu.c tu.h unpack.c Rsrlib Solarinfo Docs Makefile Test

TARBIN  = /bin/tar
ZIPBIN  = /usr/bin/zip -r
TARNAME = tu.tar.Z
ZIPNAME = tu.zip
FTPUPLOAD = /home/ftp/pub/software/tu

tu:	$(OBJS) Rsrlib/librsr.a Solarinfo/libsi.a
	$(CC) -o tu $(CFLAGS) $(OBJS) $(LIBS)

Rsrlib/librsr.a:
	cd Rsrlib; make CC=$(CC);

Solarinfo/libsi.a:
	cd Solarinfo; make CC=$(CC);

install:
	strip ./tu
	cp ./tu /usr/local/bin

clean: 
	cd Rsrlib ;\
	make clean ;
	cd Solarinfo ;\
	make clean ;
	/bin/rm -f $(OBJS)

tar :
	@VERS=`egrep '^#define.*VERSION' tu.c | head -1 | /usr/local/bin/perl -ane '$$v=pop(@F); $$v=~s/\"//g; print "$$v\n";'`; \
	TARFILE=tu\_$$VERS\.tar.Z; \
	ZIPFILE=tu\_$$VERS\.zip; \
	echo Creating tar file $$TARFILE ;\
	$(TARBIN) -Zcvf $$TARFILE $(TAR) ;\
	echo Creating zip file $$ZIPFILE ;\
	$(ZIPBIN) $$ZIPFILE $(TAR) ;\
	echo Copying $$TARFILE and $$ZIPFILE to $(FTPUPLOAD) ;\
	cp $$TARFILE $$ZIPFILE $(FTPUPLOAD) ;\
	cd $(FTPUPLOAD) ;\
	/bin/rm -f $(TARNAME) ;\
	 ln -s $$TARFILE $(TARNAME) ;\
	/bin/rm -f $(ZIPNAME) ;\
	 ln -s $$ZIPFILE $(ZIPNAME) ;
