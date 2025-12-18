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



APP_NAME=gngeo
SIS_FP="sis/$(APP_NAME)_gcce.sisx"
EXE=$(APP_NAME).exe
EXE_FP=$(EPOCROOT)/epoc32/release/gcce/urel/$(EXE)

mksis:
	cd sis && makesis -v -d$(EPOCROOT) $(APP_NAME)_gcce.pkg

mksisx:
	cd sis && signsis $(APP_NAME)_gcce.sis $(APP_NAME)_gcce.sisx mycert.cer mykey.key

depoly:
	renv_send "$(SIS_FP)"
run:
	renv_install "$(EXE_FP)" -r

#HEAP_SIZE = 27000000 # 25MB
#HEAP_SIZE = 20240000 # 10MB
HEAP_SIZE = 32115244 # 30MB

run_linux:
	cd  $(GNGEO_SRC_PATH)/src && ./gngeo --datafile=../gngeo.dat/gngeo_data.zip --blitter=soft

lrun_linux:
	cd  $(GNGEO_SRC_PATH)/src && prlimit --data=$(HEAP_SIZE) \
	./gngeo --datafile=../gngeo.dat/gngeo_data.zip --blitter=soft

drun_linux:
	cd  $(GNGEO_SRC_PATH)/src && valgrind -s --leak-check=full --show-leak-kinds=all \
	    ./gngeo --datafile=../gngeo.dat/gngeo_data.zip --blitter=soft


