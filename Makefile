
prebuild:
	cd group && bldmake bldfiles gcce urel

build:
	cd group && abld build -v gcce urel

clean: 
	cd group && abld reallyclean gcce urel

sis:
	cd sis && makesis -v -d$(EPOCROOT) *.pkg


RENV_ADDR="10.42.0.231"
APP_NAME=gngeo
SIS_SRC=`ls sis/*_gcce.sis`
SIS_DST=`basename sis/*_gcce.sis`

EXE=$(APP_NAME).exe
EXE_FP=$(EPOCROOT)/epoc32/release/gcce/urel/$(EXE)

depoly:
	renv -m c -s $(RENV_ADDR)  -c "sendfile $(SIS_SRC) /data/$(SIS_DST)"
run:
	renv -m c -s $(RENV_ADDR) -c "sendfile $(EXE_FP) D:\\$(EXE)"
	renv -m c -s $(RENV_ADDR) -c "copyfile D:\\$(EXE) C:\\sys\\bin\\$(EXE)"
	renv -m c -s $(RENV_ADDR) -c "runexe $(EXE) 0 1"
