/*   This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "include/Irq.h"
#include "include/Idt.h"
#include "include/Util.h"
#include "include/Syscall.h"
#include "include/Processo.h"
#include "include/Multiprocessamento.h"
#include "include/Mensagem.h"
#include "include/Vfs.h"

//Fun��o para a Syscall externas, definidas em start.asm
extern "C" void systemcall();

//Inst�ncia da classe syscall
Syscall syscall;

//Inicializa a classe Syscall      
void Syscall::inicializar(Idt idt)
{
	num = 0;
	
    //adicionando o endere�o da fun��o systemcall como o 80� entrada na IDT
    idt.adicionar_registro(80, (unsigned)systemcall,  0x08, 0xEE);
}
 
//Fun��o que executa a chamanda ao sistema referente ao c�digo no registrador EAX
void Syscall::chamar_systemcall(struct REGS *r)
{
	unsigned int syscallid = r->eax;
	
	//switch para decidir systemcall
	switch(syscallid)
	{
		case 0  : syscall.sbrk(r);              break;
		case 1  : syscall.obter_info_kernel(r); break;
		case 2  : syscall.enviar_msg(r);        break;
		case 3  : syscall.enviar_msg_pid(r);    break;
		case 4  : syscall.receber_msg(r);       break;
		case 5  : syscall.escutar_porta(r);     break;
		case 6  : syscall.abrir_arquivo(r);     break;
		case 7  : syscall.ler_arquivo(r);       break;
		case 8  : syscall.escrever_arquivo(r);  break;
		case 9  : syscall.buscar_arquivo(r);    break;
		case 10 : syscall.excluir_arquivo(r);   break;
		case 11 : syscall.fechar_arquivo(r);    break;
		case 12 : syscall.executar(r);   	    break;
		case 13 : syscall.sair(r);   	   	    break;
		case 14 : syscall.obter_pid(r);   	    break;
		case 15 : syscall.obter_info_arquivo(r);break;
		case 16 : syscall.adicionar_no(r);   	break;
		case 17 : syscall.montar_no(r);		   	break;
		case 18 : syscall.remover_no(r);		break;
		case 19 : syscall.finalizar_proceso(r);	break;
		case 20 : syscall.criar_thread(r);		break;
	}
	
}

//Aumentar o HEAP do processo
void Syscall::sbrk(struct REGS *r)
{
    volatile unsigned int end;
	
	//obtendo thread em execu��o
	Thread& t = escalonador.obter_thread(escalonador.obter_pid_em_execucao());
	
	//obtendo processo atual
	Processo& p = escalonador.obter_processo(t.id_processo);
	
	//aumentando a quantidade de mem�ria solicitada	 
    p.aumentar_heap(&end, r->ebx);

	//colocando novo endere�o em eax
	r->eax =  end;

}

//Obter informa��es sobre o Kernel
void Syscall::obter_info_kernel(struct REGS *r)
{	
    char * ptr_arquivo = r->ebx;
	int cod = r->ecx;
	
	char arquivo[100];
	memcpy(arquivo, ptr_arquivo, strlen(ptr_arquivo)+1);
	
	switch(cod)
	{
	 case INFO_PROCESSOS:escalonador.obter_info(arquivo); break;
     case INFO_MENSAGENS: entregador.obter_info(arquivo) ;break; 	
     case INFO_MEMORIA  : mem_fisica.obter_info(arquivo) ;break; 
     case INFO_VFS      : /*TO-DO*/ ;break;
	}
}


//--------------------------------TROCA DE MENSAGENS-------------------------------------------
void Syscall::enviar_msg(struct REGS *r)
{
    unsigned char *msg = r->ebx;
	unsigned int porta = r->ecx;
	unsigned int sinalizar_recebimento = r->edx;
	unsigned int pid = escalonador.obter_pid_em_execucao();
	
	//verificando se o o ponteiro est� dentro do espa�o de endere�os do programa
	if(msg < POS_INICIAL_KERNEL || escalonador.driver_em_execucao())
	{
		 entregador.tratar_envio(r, pid,  porta, sinalizar_recebimento, msg);
	}
	else
	{
		r->eax = 0;
	}
}

void Syscall::enviar_msg_pid(struct REGS *r)
{
	unsigned char *msg = r->ebx;
	unsigned int pid_destino = r->ecx;
	unsigned int pid = escalonador.obter_pid_em_execucao();

	//verificando se o o ponteiro est� dentro do espa�o de endere�os do programa
	if(msg < POS_INICIAL_KERNEL || escalonador.driver_em_execucao())
	{
		 r->eax= entregador.tratar_envio_pid(pid, pid_destino, MSG_COMUM, msg);
	}
	else
	{
		r->eax = 0;
	}
}

void Syscall::receber_msg(struct REGS *r)
{
	unsigned char *msg = r->ebx;
	unsigned int pid = escalonador.obter_pid_em_execucao();
	int porta = r->ecx;
	
	//verificando se o o ponteiro est� dentro do espa�o de endere�os do programa
	if(msg < POS_INICIAL_KERNEL || escalonador.driver_em_execucao())
	{
	     entregador.tratar_recebimento(r, pid, msg, porta);	
	}
}

void Syscall::escutar_porta(struct REGS *r)
{
	//definindo qual porta deve ser escutada
	unsigned int porta = r->ebx;
	
   //verificando se o processo � um DRIVER, ou se a porta � maior que 1000
   //as 1000 primeiras portas s�o reservadas para as IRQS e drivers
   if(escalonador.driver_em_execucao() || porta > 1000)
   {
	 unsigned int pid   = escalonador.obter_pid_em_execucao();
	 r->eax = entregador.adicionar_listerner(pid, porta);
   }
   else
   {
	 r->eax = 0;
   }
}

//--------------------------------SISTEMA DE ARQUIVOS-------------------------------------------
void Syscall::abrir_arquivo(struct REGS *r)
{
	//unsigned char * caminho = r->ebx;
	unsigned char caminho[100];// = r->ebx;
    memcpy(caminho, (unsigned char *)r->ebx, strlen((unsigned char *)r->ebx) +1);
 
	unsigned char modo = r->ecx;
	unsigned int * serv = r->edx;
	unsigned int descritor;
	
	//abrindo arquivo
	int ret = vfs.abrir(caminho, modo, &descritor, serv);
	
	//colocando o descritor do arquivo em ebx, e o retorno da fun��o em eax
	r->eax = ret;
	
	if(descritor == 0)
	{
	   r->ebx = ret;
	}
	else
	{
	   r->ebx = descritor;
	}
}

void Syscall::ler_arquivo(struct REGS *r)
{
	unsigned int descritor = r->ebx;
	unsigned char * buf = r->ecx;
	unsigned int tam = r->edx;
		
	if(buf < POS_INICIAL_KERNEL || escalonador.driver_em_execucao())
	{
		//chamando fun��o ler do sistema de arquivos virtual
		//unsigned char teste[100];
		
		int ret = vfs.ler(descritor, buf, tam);
	
		r->eax = ret;
	}
	else
	{
		r->eax = 0;
	}
}

void Syscall::escrever_arquivo(struct REGS *r)
{
	unsigned int descritor = r->ebx;
	unsigned char * buf = r->ecx;
	unsigned int tam = r->edx;
    
	if(buf < POS_INICIAL_KERNEL || escalonador.driver_em_execucao())
	{	
		
		int ret = vfs.escrever(descritor, buf, tam);
		r->eax =ret;
		
	}
	else
	{
		r->eax = 0;
	}
}

void Syscall::buscar_arquivo(struct REGS *r)
{
	unsigned int descritor = r->ebx;
	unsigned int pos 	   = r->ecx;
	int ret = vfs.buscar(descritor, pos);
	r->eax = ret;
}

void Syscall::excluir_arquivo(struct REGS *r)
{
	unsigned int descritor = r->ebx;
	int ret = vfs.excluir(descritor);
	r->eax = ret;
	
}

void Syscall::fechar_arquivo(struct REGS *r)
{
	unsigned int descritor = r->ebx;
	int ret = vfs.fechar(descritor);
	r->eax = ret;
	
}

void Syscall::obter_info_arquivo(struct REGS *r)
{
	unsigned int descritor = r->ebx;
	unsigned char * dados   = r->ecx;
	
	r->eax = vfs.obter_info_arquivo(descritor, dados);
}

void Syscall::adicionar_no(struct REGS *r)
{
	//apenas drivers podem executar essa fun��o
	if(escalonador.driver_em_execucao())
	{
		int tamanho        			= r->ecx;
		unsigned char * nome        = r->ebx;
		unsigned int * descritor    = r->edx;
		unsigned int porta_servidor = r->esi;
		
		unsigned char caminho[100];// = r->ebx;
        memcpy(caminho, (unsigned char *)nome, strlen((unsigned char *)nome) +1);
		
		r->eax = vfs.adicionar_no(caminho, tamanho, porta_servidor, descritor);
	}
	else
	{
		r->eax = 0;
	}
}

void Syscall::montar_no(struct REGS *r)
{
	if(escalonador.driver_em_execucao())
	{
		unsigned int desc  = r->ebx;
		r->eax = vfs.montar_no(desc);
	}
	else
	{
		r->eax = 0;
	}
}

void Syscall::remover_no(struct REGS *r)
{
	if(escalonador.driver_em_execucao())
	{
		unsigned int desc  = r->ebx;
		r->eax = vfs.remover_no(desc);
	}
	else
	{
		r->eax = 0;
	}
}

//--------------------------------GER�NCIA DE PROCESSOS-------------------------------------------
void Syscall::executar(struct REGS *r)
{
	unsigned char * nome   = r->ebx;
	unsigned int descritor = r->esi;	
	unsigned char * args   = r->edx;
	unsigned int argc      = 0;//r->ecx;
	unsigned int serv;
	
	int ret =0;

	//alocando mem�ria para o array de par�metros
	unsigned char ** cargs = (unsigned char **)kmalloc( sizeof(unsigned char *)*10);	
	
	//separando os parametros divididos por espa�os em branco
	char * tok = strtok(args," ");
   
    //quando tok � nulo, final da string alcan�ado
	while (tok != NULL)
    {
	   //tamanho do token	
	   int tam_param = strlen(tok)+1;
	   
	   //alocando mem�ria para mais um par�metros
	   cargs[argc] = (char *)kmalloc(tam_param);

	   //copiando da string para o array de par�metros	
       memcpy(cargs[argc], tok, tam_param);	
	   
	   //obtendo pr�ximo token
	   tok = strtok(NULL," ");
		
	   //incrementando contagem de par�metros	
	   argc++;
    }
	
	//obtendo imagem do arquivo
	unsigned char * imagem = vfs.obter_imagem(descritor);
	
	//adicionando um processo
	int res = escalonador.adicionar_processo(nome, imagem, Processo::USUARIO, argc, (char **)cargs);
	
	 //liberando memoria ocupada pelos par�metros
	for(int i =0; i < argc; i++)
	{
		free(cargs[i]);
	}
	
	//liberando memoria do conjunto de parametros
	free(cargs);
	
	//atribuindo valor de retorno a EAX
	r->eax = res;
}

void Syscall::sair(struct REGS *r)
{
	//obtendo thread em execu��o
	Thread& t = escalonador.obter_thread(escalonador.obter_pid_em_execucao());
	
	if(&t != NULL)
	{
		//obtendo processo referente ao PID
		Processo& p = escalonador.obter_processo(t.pid);
		
		if(&p != NULL)
		{	
			escalonador.eliminar_processo(p);
		}
		else
		{
			t.estado = Processo::FINALIZADO;
			escalonador.eliminar_thread(t);
		}
		
		escalonador.executar_prox_thread(r);
		
	}
	
}

void Syscall::obter_pid(struct REGS *r)
{
	if(r->ebx == 0)
	{
		r->eax = escalonador.obter_pid_em_execucao();
	}
	else
	{
		Thread& t = escalonador.obter_thread(r->ebx);
		
		if(&t != NULL)
			r->eax = t.id_processo;
		else
			r->eax = 0;
	}
}

void Syscall::finalizar_proceso(struct REGS *r)
{
   unsigned int pid = r->ebx;
   
   if(pid != escalonador.obter_pid_em_execucao())
   {
		Processo &proc = escalonador.obter_processo(pid);
		
		if(&proc != NULL)
		{
			if(proc.pid_pai == escalonador.obter_pid_em_execucao())
			{
				//proc.estado = Processo::FINALIZADO;
				escalonador.eliminar_processo(proc);
			}
		}
   }
}
 
void Syscall::criar_thread(struct REGS *r) 
{
	//obtendo ponto de entrada
	int entrada = r->ebx;
	
	//obtendo thread em execu��o
	Thread t = escalonador.obter_thread(escalonador.obter_pid_em_execucao());
	
	//obtendo processo referente ao PID
	Processo& p = escalonador.obter_processo(t.id_processo);
	
	r->eax = escalonador.adicionar_thread(p, entrada);
}
 
// Fun��o chamada a partir de start.asm em resposta a uma systemcall
extern "C" void tratar_systemcall(struct REGS *r)
{
	down();
	syscall.chamar_systemcall(r);
	up();
}
