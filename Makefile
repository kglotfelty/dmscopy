
#MK_TOP = ../../../..
MK_TOP = /export/ciao_from_source/ciao-4.8/src
KJG = /export/ciao

include $(MK_TOP)/Makefile.master
include $(MK_TOP)/include/Makefile.scidev

EXEC              = dmscopy
PURE_EXEC	  = 
LIB_FILES         =
PAR_FILES         = dmscopy.par
INC_FILES         =
XML_FILES         = 

SRCS	= dmscopy.c t_dmscopy.c

OBJS	= $(SRCS:.c=.o)


MAKETEST_SCRIPT = 


include $(MK_TOP)/Makefile.all

#-----------------------------------------------------------------------
# 			MAKEFILE DEPENDENCIES	
#-----------------------------------------------------------------------

$(EXEC): $(OBJS)
	$(LINK)
	@echo

kjg: $(EXEC)
	/bin/cp -f $(EXEC) $(KJG)/binexe/
	/bin/cp -f $(KJG)/bin/dmlist $(KJG)/bin/$(EXEC)
	/bin/cp -f $(PAR_FILES) $(KJG)/param/$(PAR_FILES)



announce1:
	@echo "   /----------------------------------------------------------\ "
	@echo "   |             Building paddy DM host tool         | "
	@echo "   \----------------------------------------------------------/ "
