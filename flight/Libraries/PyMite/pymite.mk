#
# Rules to add PYMite to a PiOS target
#

PYMITE = $(FLIGHTLIB)/PyMite
PYMITELIB = $(PYMITE)/lib
PYMITEPLAT = $(PYMITE)/platform/openpilot
PYMITETOOLS = $(PYMITE)/tools
PYMITEVM = $(PYMITE)/vm
PYMITEINC = $(PYMITEVM)
PYMITEINC += $(PYMITEPLAT)
PYMITEINC += $(OUTDIR)

# List C source files here. (C dependencies are automatically generated.)
# use file-extension c for "c-only"-files

## PyMite files and modules
SRC += $(OUTDIR)/pmlib_img.c
SRC += $(OUTDIR)/pmlib_nat.c
SRC += $(OUTDIR)/pmlibusr_img.c
SRC += $(OUTDIR)/pmlibusr_nat.c
PYSRC += $(wildcard ${PYMITEVM}/*.c)
PYSRC += $(wildcard ${PYMITEPLAT}/*.c)
PYSRC += ${foreach MOD, ${PYMODULES}, ${wildcard ${OPMODULEDIR}/${MOD}/*.c}}
SRC += $(PYSRC)

EXTRAINCDIRS  += $(PYMITEINC)

# Generate intermediate code
gencode: ${OUTDIR}/pmlib_img.c ${OUTDIR}/pmlib_nat.c ${OUTDIR}/pmlibusr_img.c ${OUTDIR}/pmlibusr_nat.c ${OUTDIR}/pmfeatures.h 

$(PYSRC): gencode

PYTHON = python

# Generate code for PyMite
${OUTDIR}/pmlib_img.c ${OUTDIR}/pmlib_nat.c ${OUTDIR}/pmlibusr_img.c ${OUTDIR}/pmlibusr_nat.c ${OUTDIR}/pmfeatures.h: $(wildcard ${PYMITELIB}/*.py) $(wildcard ${PYMITEPLAT}/*.py) $(wildcard ${FLIGHTPLANLIB}/*.py) $(wildcard ${FLIGHTPLANS}/*.py) 
	@echo $(MSG_PYMITEINIT) $(call toprel, $@)
	@$(PYTHON) $(PYMITETOOLS)/pmImgCreator.py -f $(PYMITEPLAT)/pmfeatures.py -c -s --memspace=flash -o $(OUTDIR)/pmlib_img.c --native-file=$(OUTDIR)/pmlib_nat.c $(PYMITELIB)/list.py $(PYMITELIB)/dict.py $(PYMITELIB)/__bi.py $(PYMITELIB)/sys.py $(PYMITELIB)/string.py $(wildcard $(FLIGHTPLANLIB)/*.py)
	@$(PYTHON) $(PYMITETOOLS)/pmGenPmFeatures.py $(PYMITEPLAT)/pmfeatures.py > $(OUTDIR)/pmfeatures.h
	@$(PYTHON) $(PYMITETOOLS)/pmImgCreator.py -f $(PYMITEPLAT)/pmfeatures.py -c -u -o $(OUTDIR)/pmlibusr_img.c --native-file=$(OUTDIR)/pmlibusr_nat.c $(FLIGHTPLANS)/test.py
