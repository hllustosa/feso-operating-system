# lib Paths
########################
PATH_ROOT   := $(shell pwd)
PATH_INC    := $(PATH_ROOT)/lib/include
PATH_BIN    := $(PATH_ROOT)/lib/bin
LPATH	    := $(PATH_ROOT)/lib
PROJDIRS    := $(LPATH)  $(PATH_INC) $(PATH_BIN)
CSRCFILES   := $(shell find $(KPROJDIRS) -type f -name "*.cpp") 
ASMSRCFILES := $(shell find $(KPROJDIRS) -type f -name "*.asm") 
HDRFILES    := $(shell find $(KPROJDIRS) -type f -name "*.h")
COBJFILES   := $(patsubst %.cpp, %.o, $(CSRCFILES)) 
ASMOBJFILES := $(patsubst %.asm, %.o, $(ASMSRCFILES)) 
EXECUTABLE  := $(PATH_BIN)/libfeso.o
TRAMPOLIM   := tramp.o

# FERRAMENTAS
########################
NASM    =nasm -f elf
C++     =/usr/local/cross/bin/i586-elf-c++
LD      =/usr/local/cross/bin/i586-elf-ld
RM      =rm

# FLAG DE COMPILAÇÃO
########################
CFLAGS 	=  -Wall -w -O0 -fpermissive -fstrength-reduce -fomit-frame-pointer -finline-functions -fno-exceptions -fno-builtin -I./include

.PHONY: all clean
	
all: $(EXECUTABLE) clean

#linkando binários
$(EXECUTABLE): $(COBJFILES) $(ASMOBJFILES)
	@$(LD) -o $(EXECUTABLE) $(COBJFILES) $(ASMOBJFILES) 	

#compilando arquivos cpp	
%.o: %.cpp
	@$(C++) $(CFLAGS)  -c -o $@ $<	

#compilando arquivos asm	
%.o: %.asm
	@$(NASM) -o $@ $<	

clean:	
	-@$(RM) $(COBJFILES) $(ASMOBJFILES)
