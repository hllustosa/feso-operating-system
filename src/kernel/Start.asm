;==============================================================================================
[BITS 32]      

Section .setup	
ALIGN 4
mboot:

    ; Código obtido no OSDEVER para compatibildade GRUB
    MULTIBOOT_PAGE_ALIGN	equ 1<<0
    MULTIBOOT_MEMORYiNFO	equ 1<<1
    MULTIBOOT_AOUT_KLUDGE	equ 1<<16
    MULTIBOOT_HEADER_MAGIC	equ 0x1BADB002
    MULTIBOOT_HEADER_FLAGS	equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORYiNFO | MULTIBOOT_AOUT_KLUDGE
    MULTIBOOT_CHECKSUM	equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
    EXTERN code, bss, end

    ; Cabeçalho Multiboot do GRUB
    dd MULTIBOOT_HEADER_MAGIC
    dd MULTIBOOT_HEADER_FLAGS
    dd MULTIBOOT_CHECKSUM
    
    dd mboot
    dd code
    dd bss
    dd end
    dd start
   
global start 	

;Ponto de entrada do executável do Kernel
start:
    lgdt [gdt_falsa]    ;Configurando a GDT falsa
	mov ax, 0x10		;Alerando registradores de segmento
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	jmp 0x08:higherhalf ;Jump para o higherhalf para alterar o valor de CS	
	
;GDT temporaria para carregar o kernel a partir de 3 GB de memória	
gdt_falsa:
	dw gdt_end - gdt - 1 ; tam da GDT
	dd gdt               ; endereço linear da GDR
 
gdt:
	dd 0, 0							                    ;entrada nula
	db 0xFF, 0xFF, 0, 0, 0, 10011010b, 11001111b, 0x40	; code selector 0x08: base 0x40000000, limite 0xFFFFFFFF, tipo 0x9A(anel 0, código), granularidade 0xCF
	db 0xFF, 0xFF, 0, 0, 0, 10010010b, 11001111b, 0x40	; data selector 0x10: base 0x40000000, limite 0xFFFFFFFF, tipo 0x92(anel 0, data), granularidade 0xCF
	
gdt_end: ; label para dermacar fim da GDT	

;Kernel rodando em 3GB de memória	
Section .text  
higherhalf:
    mov esp, _sys_stack     ;Aponta para nova área da Pilha
    push _sys_stack;		;passando a posição da pilha
	push end;               ;passando para função main a varíavel main definida no script do linker, ela possuí o endereço do final do kernel
 	push ebx                ;passandopara a função main a estrutura multiboot com o mapa da memória
    extern main             
    call main               ;Chamada da função main em main.cpp
    jmp $
	
;=======================================================================================	
;Carrega o novo endereço da Global Descriptor Table	
global carregar_gdt
extern gp            ; ponteiro para gdt, definidio em Gdt.h  
carregar_gdt:
    lgdt [gp]        ; configurando a nova GDT
    mov ax, 0x10     ; Alterando os registradores de segmento para a segunda entrada na GDT
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:prox ; Jump para alterar o valor do registrador CS
prox:
    ret

;=======================================================================================	
; Carrega a Interrupt Descriptor Table.
global carregar_idt
extern idtp
carregar_idt:
    lidt [idtp]
    ret

	
	
;=======================================================================================	
; Função para garantir acesso atômico ao mutex
ALIGN 4
_lock:
dd 0

global _down
_down:
	push  ebp       
	mov   ebp, esp;

	mov   eax, 	[ebp + 8]
    	lock bts dword[eax],0       
    	jnc .acquired
 
.retest:
    test dword [eax],1      
    je .retest               
 
    lock bts dword [eax],0       
    jc .retest
 
.acquired:
	mov esp, ebp    
	pop ebp ;	
    ret

global _up
_up:
    mov dword [_lock],0
    ret
	
;=======================================================================================
;Funções para o tratamento das Exceptions

global isr0
global isr1
global isr2
global isr3
global isr4
global isr5
global isr6
global isr7
global isr8
global isr9
global isr10
global isr11
global isr12
global isr13
global isr14
global isr15
global isr16
global isr17
global isr18
global isr19
global isr20
global isr21
global isr22
global isr23
global isr24
global isr25
global isr26
global isr27
global isr28
global isr29
global isr30
global isr31

;  0: Divide By Zero 
isr0:
    cli
    push byte 0
    push byte 0
    jmp tratamento_comum_exceptions

;  1: Debug Exception
isr1:
    cli
    push byte 0
    push byte 1
    jmp tratamento_comum_exceptions

;  2: Non Maskable Interrupt Exception
isr2:
    cli
    push byte 0
    push byte 2
    jmp tratamento_comum_exceptions

;  3: Int 3 Exception
isr3:
    cli
    push byte 0
    push byte 3
    jmp tratamento_comum_exceptions

;  4: INTO Exception
isr4:
    cli
    push byte 0
    push byte 4
    jmp tratamento_comum_exceptions

;  5: Out of Bounds Exception
isr5:
    cli
    push byte 0
    push byte 5
    jmp tratamento_comum_exceptions

;  6: Invalid Opcode Exception
isr6:
    cli
    push byte 0
    push byte 6
    jmp tratamento_comum_exceptions

;  7: Coprocessor Not Available Exception
isr7:
    cli
    push byte 0
    push byte 7
    jmp tratamento_comum_exceptions

;  8: Double Fault Exception 
isr8:
    cli
    push byte 8
    jmp tratamento_comum_exceptions

;  9: Coprocessor Segment Overrun Exception
isr9:
    cli
    push byte 0
    push byte 9
    jmp tratamento_comum_exceptions

; 10: Bad TSS Exception 
isr10:
    cli
    push byte 10
    jmp tratamento_comum_exceptions

; 11: Segment Not Present Exception 
isr11:
    cli
    push byte 11
    jmp tratamento_comum_exceptions

; 12: Stack Fault Exception 
isr12:
    cli
    push byte 12
    jmp tratamento_comum_exceptions

; 13: General Protection Fault Exception 
isr13:
    cli
    push byte 13
    jmp tratamento_comum_exceptions

; 14: Page Fault Exception 
isr14:
    cli
    push byte 14
    jmp tratamento_comum_exceptions

; 15: Reservado Exception
isr15:
    cli
    push byte 0
    push byte 15
    jmp tratamento_comum_exceptions

; 16: Floating Point Exception
isr16:
    cli
    push byte 0
    push byte 16
    jmp tratamento_comum_exceptions

; 17: Alignment Check Exception
isr17:
    cli
    push byte 0
    push byte 17
    jmp tratamento_comum_exceptions

; 18: Machine Check Exception
isr18:
    cli
    push byte 0
    push byte 18
    jmp tratamento_comum_exceptions

; 19: Reservado
isr19:
    cli
    push byte 0
    push byte 19
    jmp tratamento_comum_exceptions

; 20: Reservado
isr20:
    cli
    push byte 0
    push byte 20
    jmp tratamento_comum_exceptions

; 21: Reservado
isr21:
    cli
    push byte 0
    push byte 21
    jmp tratamento_comum_exceptions

; 22: Reservado
isr22:
    cli
    push byte 0
    push byte 22
    jmp tratamento_comum_exceptions

; 23: Reservado
isr23:
    cli
    push byte 0
    push byte 23
    jmp tratamento_comum_exceptions

; 24: Reservado
isr24:
    cli
    push byte 0
    push byte 24
    jmp tratamento_comum_exceptions

; 25: Reservado
isr25:
    cli
    push byte 0
    push byte 25
    jmp tratamento_comum_exceptions

; 26: Reservado
isr26:
    cli
    push byte 0
    push byte 26
    jmp tratamento_comum_exceptions

; 27: Reservado
isr27:
    cli
    push byte 0
    push byte 27
    jmp tratamento_comum_exceptions

; 28: Reservado
isr28:
    cli
    push byte 0
    push byte 28
    jmp tratamento_comum_exceptions

; 29: Reservado
isr29:
    cli
    push byte 0
    push byte 29
    jmp tratamento_comum_exceptions

; 30: Reservado
isr30:
    cli
    push byte 0
    push byte 30
    jmp tratamento_comum_exceptions

; 31: Reservado
isr31:
    cli
    push byte 0
    push byte 31
    jmp tratamento_comum_exceptions


; Declaração da função em C++ para tratar todas as interrupções
extern tratar_exception

;Rotina que salva o contexto, chama a função tratar_excecao definida em C++
;E restaura o contexto. Usada também no tratamento de interrupções
tratamento_comum_exceptions:
    pusha
    push ds
    push es
    push fs
    push gs
	
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
	
    mov eax, esp
    push eax
    mov eax, tratar_exception
	
    call eax
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret
	
;=======================================================================================	
;Funções para o tratamento das IRQs

global irq0
global irq1
global irq2
global irq3
global irq4
global irq5
global irq6
global irq7
global irq8
global irq9
global irq10
global irq11
global irq12
global irq13
global irq14
global irq15
global irq_spu

; 32: IRQ0
irq0:
    cli
    push byte 0
    push byte 32
    jmp tratamento_comum_irq
	
; 33: IRQ1
irq1:
    cli
    push byte 0
    push byte 33
    jmp tratamento_comum_irq

; 34: IRQ2
irq2:
    cli
    push byte 0
    push byte 34
    jmp tratamento_comum_irq

; 35: IRQ3
irq3:
    cli
    push byte 0
    push byte 35
    jmp tratamento_comum_irq

; 36: IRQ4
irq4:
    cli
    push byte 0
    push byte 36
    jmp tratamento_comum_irq

; 37: IRQ5
irq5:
    cli
    push byte 0
    push byte 37
    jmp tratamento_comum_irq

; 38: IRQ6
irq6:
    cli
    push byte 0
    push byte 38
    jmp tratamento_comum_irq

; 39: IRQ7
irq7:
    cli
    push byte 0
    push byte 39
    jmp tratamento_comum_irq

; 40: IRQ8
irq8:
    cli
    push byte 0
    push byte 40
    jmp tratamento_comum_irq

; 41: IRQ9
irq9:
    cli
    push byte 0
    push byte 41
    jmp tratamento_comum_irq

; 42: IRQ10
irq10:
    cli
    push byte 0
    push byte 42
    jmp tratamento_comum_irq

; 43: IRQ11
irq11:
    cli
    push byte 0
    push byte 43
    jmp tratamento_comum_irq

; 44: IRQ12
irq12:
    cli
    push byte 0
    push byte 44
    jmp tratamento_comum_irq

; 45: IRQ13
irq13:
    cli
    push byte 0
    push byte 45
    jmp tratamento_comum_irq

; 46: IRQ14
irq14:
    cli
    push byte 0
    push byte 46
    jmp tratamento_comum_irq

; 47: IRQ15
irq15:
    cli
    push byte 0
    push byte 47
    jmp tratamento_comum_irq
	
;irq_spu	
irq_spu:
    cli
    push byte 0
    push byte 127
    jmp tratamento_comum_irq
	
global irq80	
; 47: IRQ15
irq80:
    push byte 0
    push byte 80
    jmp tratamento_comum_irq	

extern tratar_irq

;=============================================================================
;Função que salva o contexto atual, executa a ISR e restaura o contexto
tratamento_comum_irq:
    pusha       ; empilhando registradores da CPU
    push ds     ; empilhando registradores de segmento
    push es
    push fs
    push gs

    mov ax, 0x10; mudando registradores de segmento para apontar para a enrada de dados na GDT
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp  ;salvando esp em eax

    push eax  ;empilhando eax com valor de esp
    mov eax, tratar_irq ; chamando função em C para tratamento da IRQ
    call eax
    pop eax

    pop gs  ; restaurando registradores de segmento
    pop fs  
    pop es
    pop ds
    popa    ; restaurando registradores da CPU
    add esp, 8 ;removendo da pilha os dois valores inseridos pela ISR
    iret	
  	
	
;===========================================================
;Funções para o tratamento das Chamadas ao sistema
	
global systemcall
extern tratar_systemcall

systemcall :	
    cli			;desabilitando interrupções
    push byte 0
    push byte 80
	pusha       ; empilhando registradores da CPU
    push ds     ; empilhando registradores de segmento
    push es
    push fs
    push gs

    mov ax, 0x10; mudando registradores de segmento para apontar para a enrada de dados na GDT
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp  ;salvando esp em eax

    push eax  ;empilhando eax com valor de esp
    mov eax, tratar_systemcall ; chamando função em C para tratamento da systemcall
    call eax
    pop eax

    pop gs  ; restaurando registradores de segmento
    pop fs  
    pop es
    pop ds
    popa    ; restaurando registradores da CPU
    add esp, 8 ;removendo da pilha os dois valores inseridos pela ISR
    iret	
  	
	
	
; Definição da seção BSS section. 
SECTION .bss
    resb 16384               ; Reservar 8kb de memória para a piha
_sys_stack:					;label que demarca o começo da pilha, após 8kb.
