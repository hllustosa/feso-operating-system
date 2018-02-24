# Kernel Paths
########################
PATH_ROOT   := $(shell pwd)
KPATH_INC   := $(PATH_ROOT)/include
KPATH_BIN   := $(PATH_ROOT)/bin
KPROJDIRS   := $(PATH_ROOT) $(KPATH_INC) $(KPATH_BIN)
CSRCFILES   := $(shell find $(KPROJDIRS) -type f -name "*.cpp") 
ASMSRCFILES := $(shell find $(KPROJDIRS) -type f -name "*.asm") 
HDRFILES    := $(shell find $(KPROJDIRS) -type f -name "*.h")
COBJFILES   := $(patsubst %.cpp, %.o, $(CSRCFILES)) 
ASMOBJFILES := $(patsubst %.asm, %.o, $(ASMSRCFILES)) 
EXECUTABLE  := $(KPATH_BIN)/kernel.bin
#TRAMPOLIM   := $(KPATH_BIN)/trampolim.o

# FERRAMENTAS
########################
NASM    =nasm -f elf
C++     =/usr/local/cross/bin/i386-elf-c++
LD      =/usr/local/cross/bin/i386-elf-ld
RM      =rm

# FLAG DE COMPILAÇÃO
########################
CFLAGS 	=  -Wall -w -O -fpermissive -fstrength-reduce -fomit-frame-pointer -finline-functions -fno-exceptions -fno-builtin -I./include

.PHONY: all clean
	
all: $(EXECUTABLE) clean

#linkando binários
$(EXECUTABLE): $(COBJFILES) $(ASMOBJFILES) $(TRAMPOLIM)
	@$(LD) -T link.ld -o $(EXECUTABLE)  $(ASMOBJFILES) $(COBJFILES) 	
	

#compilando arquivos cpp	
%.o: %.cpp
	@$(C++) $(CFLAGS)  -c -o $@ $<	

#compilando arquivos asm	
%.o: %.asm
	@$(NASM) -o $@ $<	

#compilando código trampolim a parte	
$(TRAMPOLIM):	
	-@$(NASM) -o $(PATH_ROOT)/trampolim.o $(PATH_ROOT)/trampolim.asm2
	-@$(LD) -T linktramp.ld -o $(KPATH_BIN)/trampolim.o $(PATH_ROOT)/trampolim.o
	-@$(RM) $(PATH_ROOT)/trampolim.o
	
clean:	
	-@$(RM) $(COBJFILES) $(ASMOBJFILES)
