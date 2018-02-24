#Paths
########################
PATH_ROOT   := $(shell pwd)
PATH_INC    := $(PATH_ROOT)/include
PATH_BIN    := $(PATH_ROOT)/bin
PATH_LIB    := ../../lib
PROJDIRS    := $(PATH_ROOT) $(PATH_INC) $(PATH_BIN) $(PATH_LIB)
CSRCFILES   := $(shell find $(PROJDIRS) -type f -name "*.cpp") 
ASMSRCFILES := $(shell find $(PROJDIRS) -type f -name "*.asm") 
HDRFILES    := $(shell find $(PROJDIRS) -type f -name "*.h")
COBJFILES   := $(patsubst %.cpp, %.o, $(CSRCFILES)) 
ASMOBJFILES := $(patsubst %.asm, %.o, $(ASMSRCFILES)) 

#alterar nome do executável aqui
EXECUTABLE  := $(PATH_BIN)/ide.o

# FERRAMENTAS
########################
NASM    =nasm -f elf
C++     =/usr/local/cross/bin/i386-elf-c++
LD      =/usr/local/cross/bin/i386-elf-ld
RM      =rm

# FLAG DE COMPILAÇÃO
########################
CFLAGS 	=  -Wall -w -O0 -fpermissive -fstrength-reduce -fomit-frame-pointer -finline-functions -fno-exceptions -fno-builtin -I./include

.PHONY: all clean
	
all: $(EXECUTABLE) clean

#linkando binários
$(EXECUTABLE): $(COBJFILES) $(ASMOBJFILES)
	@$(LD) -T link.ld -o $(EXECUTABLE) $(COBJFILES) $(ASMOBJFILES)	

#compilando arquivos cpp	
%.o: %.cpp
	@$(C++) $(CFLAGS)  -c -o $@ $<	

#compilando arquivos asm	
%.o: %.asm
	@$(NASM) -o $@ $<	

clean:	
	-@$(RM) $(COBJFILES) $(ASMOBJFILES)
