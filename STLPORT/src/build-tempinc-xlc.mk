tempIncConst = tempinc
buildDir = .
tempIncDir = $(buildDir)/$(tempIncConst)

TempInc_CFiles := $(wildcard $(tempIncDir)/*.C)
TempInc_OFiles := $(patsubst %.C,%.o, $(TempInc_CFiles))

all.PHONY : $(TempInc_OFiles)

%.o : %.C
    @echo "Compiling File $@"
    @xlC_r  \
    -I/tools/STLport-4.0/stlport -qmaxerr=10 -qtempinc=$(tempIncDir)
-qrtti=all -qsrcmsg \
    -o $(@) \
    -c $<
