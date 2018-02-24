[BITS 32]

global sbk
sbk:
	push  ebp       ; salvando ebp 
    mov   ebp, esp; ;
	
	mov  eax, 0x00      ; colocando 0 em eax (system call 0 solicita mais memoria)
	mov  ebx, [ebp + 8] ;obtendo primeiro parâmetro
	int  80             ;chamando interrupção
	
	
	mov esp, ebp    ;restaurando ebp; 
	pop ebp ;
	ret 			;retorno
	
	
global obter_info_kernel
obter_info_kernel:

	push  ebp       ; 
    mov   ebp, esp; 
	
	mov  eax, 0x01
	mov  ebx, [ebp + 8]
	mov  ecx, [ebp + 12]
	int  80
	
	mov esp, ebp  
	pop ebp ;
	ret 	
	
	
global enviar_msg
enviar_msg:
	push  ebp       ; 
    mov   ebp, esp; 
	
	mov  eax, 0x02
	mov  ecx, [ebp + 8]
	mov  ebx, [ebp + 12]
	mov  edx, 1
	int  80
	
	mov esp, ebp  
	pop ebp ;
	ret

global enviar_receber_msg:
enviar_receber_msg:
	push  ebp       ; 
    mov   ebp, esp; 
	
	mov  eax, 0x02
	mov  ecx, [ebp + 8]
	mov  ebx, [ebp + 12]
	mov  edx, 0
	int  80
	
obter_msg :	
	mov  eax, 0x04
	mov  ecx, [ebp + 8]
	mov  ebx, [ebp + 12]
	int  80
	
	cmp eax, 0x00  ;se o retorno for 0
	je obter_msg ; verificar msg novamente
	
	mov esp, ebp  
	pop ebp ;
	ret 	
	
global enviar_msg_pid	
enviar_msg_pid:
	push  ebp       ; 
    mov   ebp, esp; 
	
	mov  eax, 0x03
	mov  ecx, [ebp + 8]
	mov  ebx, [ebp + 12]
	int  80
	
	mov esp, ebp  
	pop ebp ;
	ret 

global receber_msg	
receber_msg:
	push  ebp       ; 
    mov   ebp, esp; 

verificar_msg :	
	mov  eax, 0x04
	mov  ebx, [ebp + 8]
	mov  ecx, [ebp + 12]
	int  80
	
	cmp eax, 0x00  ;se o retorno for 0
	je verificar_msg ; verificar msg novamente
	
	mov esp, ebp  
	pop ebp ;
	ret 

	
global escutar_porta
escutar_porta:
	push  ebp; 
    mov   ebp, esp; 
	
	mov  eax, 0x05
	mov  ebx, [ebp + 8]
	int  80
	
	mov esp, ebp  
	pop ebp ;
	ret 


global _abrir
_abrir :
	
	push  ebp; 
    mov   ebp, esp; 
;l1:	
	mov  eax, 0x06
	mov  ebx, [ebp + 8]
	mov  ecx, [ebp + 12]
	mov  edx, [ebp + 16]
	int  80
	
;	cmp eax, 0x02
;	je l1
	
	mov eax, ebx
	mov esp, ebp  
	pop ebp ;
	ret 
	
global _ler
_ler :
	
	push  ebp; 
    mov   ebp, esp; 
	
	mov  eax, 0x07
	mov  ebx, [ebp + 8]
	mov  ecx, [ebp + 12]
	mov  edx, [ebp + 16]
	int  80
	
	mov esp, ebp  
	pop ebp ;
	ret 	
	
global _escrever
_escrever :
	
	push  ebp; 
    mov   ebp, esp; 
	
	mov  eax, 0x08
	mov  ebx, [ebp + 8]
	mov  ecx, [ebp + 12]
	mov  edx, [ebp + 16]
	int  80
	
	mov esp, ebp  
	pop ebp ;
	ret 		
	
global _buscar
_buscar :
	
	push  ebp; 
    mov   ebp, esp; 
	
	mov  eax, 0x09
	mov  ebx, [ebp + 8]
	mov  ecx, [ebp + 12]
	
	int  80
	
	mov esp, ebp  
	pop ebp ;
	ret 		
	
global _excluir
_excluir :
	
	push  ebp; 
    mov   ebp, esp; 
	
	mov  eax, 0x0A
	mov  ebx, [ebp + 8]
	
	int  80
	
	mov esp, ebp  
	pop ebp ;
	ret 	
	
global _fechar
_fechar :
	
	push  ebp; 
    mov   ebp, esp; 
	
	mov  eax, 0x0B
	mov  ebx, [ebp + 8]
	
	int  80
	
	mov esp, ebp  
	pop ebp ;
	ret 	

global _executar
_executar :
	
	push  ebp; 
    mov   ebp, esp; 
	
	mov  eax, 0x0C
	mov  ebx, [ebp + 8]
	mov  ecx, [ebp + 12]
	mov  edx, [ebp + 16]
	mov  esi, [ebp + 20]
	
	int  80
	
	mov esp, ebp  
	pop ebp ;
	ret 		

global sair
sair :
	
	push  ebp; 
    mov   ebp, esp; 
	
	mov  eax, 0x0D
	int  80
	
lsair:	
	jmp lsair
	
	mov esp, ebp  
	pop ebp ;
	ret 	

global _obter_pid
_obter_pid:
	
	push  ebp; 
    mov   ebp, esp; 
	
	mov  eax, 0x0E
	mov  ebx, [ebp+8]
	int  80
	
	mov esp, ebp  
	pop ebp ;
	ret 
	
global finalizar_proceso
finalizar_proceso:
	
	push  ebp; 
    mov   ebp, esp; 
	
	mov  eax, 0x13
	mov  ebx, [ebp+8]
	int  80
	
	mov esp, ebp  
	pop ebp ;
	ret 	
	
global criar_thread
criar_thread:
	
	push  ebp; 
    mov   ebp, esp; 
	
	mov  eax, 0x14
	mov  ebx, [ebp+8]
	int  80
	
	mov esp, ebp  
	pop ebp ;
	ret 	
	
global _obter_info_arq
_obter_info_arq:
	
	push  ebp; 
    mov   ebp, esp; 
	
	mov  eax, 0x0F
	mov  ebx, [ebp + 8]
	mov  ecx, [ebp + 12]
	
	int  80
	
	mov esp, ebp  
	pop ebp ;
	ret 
	
global _adicionar_no
_adicionar_no:
	
	push  ebp; 
    mov   ebp, esp; 
	
	mov  eax, 0x10
	mov  ebx, [ebp + 8]
	mov  ecx, [ebp + 12]
	mov  edx, [ebp + 16]
	mov  esi, [ebp + 20]
	
	int  80
	
	mov esp, ebp  
	pop ebp ;
	ret 
	
global _montar_no
_montar_no:
	
	push  ebp; 
    mov   ebp, esp; 
	
	mov  eax, 0x11
	mov  ebx, [ebp + 8]
	
	int  80
	
	mov esp, ebp  
	pop ebp ;
	ret 	

global _remover_no
_remover_no:
	
	push  ebp; 
    mov   ebp, esp; 
	
	mov  eax, 0x12
	mov  ebx, [ebp + 8]
	
	int  80
	
	mov esp, ebp  
	pop ebp ;
	ret 	
	