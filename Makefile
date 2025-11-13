GNGEO_SRC_PATH=src/gngeo-0.8

prebuild:
	cd  $(GNGEO_SRC_PATH)/src && cp config_symbian.h config.h
	cd group && bldmake bldfiles gcce urel

prebuild_linux:
	cd  $(GNGEO_SRC_PATH) && rm -f config.h src/config.h && ./configure  --disable-i386asm --with-z80core=mamez80 CFLAGS="-fcommon"

build:
	cd group && abld build -v gcce urel


build_linux:
	cd  $(GNGEO_SRC_PATH) && make

clean: 
	cd group && abld reallyclean gcce urel

clean_linux:
	cd  $(GNGEO_SRC_PATH) && make clean



RENV_ADDR="10.42.0.231"
APP_NAME=gngeo
SIS_SRC="sis/$(APP_NAME)_gcce.sisx"
SIS_DST=`basename sis/$(APP_NAME)_gcce.sisx`
EXE=$(APP_NAME).exe
EXE_FP=$(EPOCROOT)/epoc32/release/gcce/urel/$(EXE)

mksis:
	cd sis && makesis -v -d$(EPOCROOT) $(APP_NAME)_gcce.pkg \
	&& signsis $(APP_NAME)_gcce.sis $(APP_NAME)_gcce.sisx mycert.cer mykey.key


depoly:
	renv -m c -s $(RENV_ADDR)  -c "sendfile $(SIS_SRC) /data/$(SIS_DST)"
run:
	renv -m c -s $(RENV_ADDR) -c "sendfile $(EXE_FP) D:\\$(EXE)"
	renv -m c -s $(RENV_ADDR) -c "copyfile D:\\$(EXE) C:\\sys\\bin\\$(EXE)"
	renv -m c -s $(RENV_ADDR) -c "runexe $(EXE) 0 1"

run_linux:
	cd  $(GNGEO_SRC_PATH)/src && ./gngeo --datafile=../gngeo.dat/gngeo_data.zip --blitter=soft


drun_linux:
	cd  $(GNGEO_SRC_PATH)/src && valgrind -s --leak-check=full --show-leak-kinds=all \
	    ./gngeo --datafile=../gngeo.dat/gngeo_data.zip --blitter=soft


