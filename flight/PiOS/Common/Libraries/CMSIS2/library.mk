#
# Rules to add CMSIS2 to a PiOS target
#

CMSIS2_DIR	:= $(dir $(lastword $(MAKEFILE_LIST)))
EXTRAINCDIRS	+= $(CMSIS2_DIR)Include

# Rules to build the ARM DSP library
ifeq ($(USE_DSP_LIB), YES)
    DSPLIB_NAME		:= dsp
    CMSIS_DSPLIB	:= $(CMSIS2_DIR)DSP_Lib/Source

    # Compile all files into output directory
    DSPLIB_SRC		:= $(sort $(wildcard $(CMSIS_DSPLIB)/*/*.c))
    DSPLIB_SRCBASE	:= $(notdir $(basename $(DSPLIB_SRC)))
    $(foreach src, $(DSPLIB_SRC), $(eval $(call COMPILE_C_TEMPLATE, $(src))))

    # Define the object files directory and a list of object files for the library
    DSPLIB_OBJDIR	= $(OUTDIR)
    DSPLIB_OBJ		= $(addprefix $(DSPLIB_OBJDIR)/, $(addsuffix .o, $(DSPLIB_SRCBASE)))

    # Create a library file
    $(eval $(call ARCHIVE_TEMPLATE, $(OUTDIR)/lib$(DSPLIB_NAME).a, $(DSPLIB_OBJ), $(DSPLIB_OBJDIR)))

    # Add library to the list of linked objects
    ALLLIB		+= $(OUTDIR)/lib$(DSPLIB_NAME).a
endif
