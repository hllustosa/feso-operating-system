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
#include "include/Mensagem.h"
#include "include/Processo.h"
#include "include/Memoria.h"
#include "include/Multiprocessamento.h"
#include "include/Vfs.h"
#include "include/Idt.h"
#include "include/Irq.h"

//declara��o da inst�ncia global da classe entregador
volatile Entregador entregador;

//fun��o para notificar o entregador das IRQS
extern "C" void notificar(struct REGS * r)
{
	entregador.notificar_irq(r, r->int_no - 32);
}
		
//Inicializa o entregador		
void Entregador::inicializar(Irq irq)
{
   mutex = 0;
   id_dono = -1;	
   rec_num = 6;	
   portas.rec_num = 7;
   portas_alocadas.rec_num = 8;
	
	
   //instalando fun��o para notificar IRQ
   for(int  i = 0; i <= 15; i++)
   {
	  irq.instalar_funcao(i, notificar);
   }
}

unsigned int cpu =0;

//M�todo que serve para notificar o entregador em caso de IRQ
void Entregador::notificar_irq(struct REGS * r, int irq_num)
{	
	//Obtendo porta referente a IRQ
	Porta& p = portas[irq_num];
	

	//Verificando a ser a porta foi alocada por algum processo
    if(&p != NULL)    
	{
		//Criando Mensagem com conte�do IRQ
		unsigned char msg[TAM_MAX_MSG] = "IRQ";	

		//Enviando uma mensagem para o processo
		tratar_envio_pid(KERNEL_MSG, p.pid_responsavel, MSG_IRQ, msg);
		
		//Executado nova thread
		escalonador.executar_prox_thread(r);
	}
	else if( (irq_num == 0 || irq_num == 2) && escalonador.escalonamento_ativo)
	{
		//IRQ 0 (Timer), executar o escalonador  
		escalonador.executar_prox_thread(r);
	}
	
}

//M�todo que serve para notificar o entregador em caso de evento do kernel
void Entregador::notificar_evento(char * dados, int evt_num)
{
	Lista<int> pids_remover;
	
	//Adicionando a Lista de listerners correspondente a porta	
    switch(evt_num)
    {
	  case KERNEL_PORTA_PROC_CRIADO : 
		 kernel_listerners = kernel_listerners_evt_proc_criado; break;
	  case KERNEL_PORTA_PROC_DESTRUIDO : 
		 kernel_listerners = kernel_listerners_evt_proc_destruido; break;
	  case KERNEL_PORTA_ALOCADA : 
		kernel_listerners = kernel_listerners_evt_porta_alocada; break;
	  case KERNEL_PORTA_DESALOCADA:
		kernel_listerners = kernel_listerners_evt_porta_desalocada; break;
	  case KERNEL_PORTA_VFS: 
		kernel_listerners = kernel_listerners_evt_vfs; break;
	  case KERNEL_PORTA_EXCEPTION:
		kernel_listerners = kernel_listerners_exception; break;
    }
	
	//Varrendo lista de listerners para o evento
	for(int i =0; i < kernel_listerners.tamanho(); i++)
	{
		tratar_envio_pid(KERNEL_MSG, kernel_listerners[i], MSG_EVT, dados);	
	}
	
	//Removendo Listerners, caso seja necess�rio
	for(int i = 0; i < pids_remover.tamanho(); i++)
	{
		kernel_listerners.remover(pids_remover[i]);
	}
	
	pids_remover.limpar();
	
}

//Met�do para acessar os dados de uma porta
Porta& Entregador::obter_porta(int num)
{
	return portas[num];
}

//M�todo para adicionar listerner de enveto do kernel
int Entregador::adicionar_listerner(unsigned pid, unsigned porta)
{
	//Verificando se n�o � uma porta utilizada pelo kernel
	if( (porta < KERNEL_PORTA_LIM_INF) || (porta > KERNEL_PORTA_LIM_SUP) )
	{
		Porta& _p = portas[porta];
		
		//caso j� exista algum processo escutando a porta
		if(&_p != NULL)
		{
			Thread& t = escalonador.obter_thread(_p.pid_responsavel);
			
			if(&t != NULL)
			{
				return MSG_ERR_PORTA_OCUPADA;
			}
			else	
			{
				_p.pid_responsavel = pid;
				return MSG_SUCESSO;
			}
				
		}
		else
		{
			//declarando nova porta
			Porta p;
			p.numero = porta;
			p.pid_responsavel = pid;
			
			//adicionando a lista
			portas.adicionar(p.numero, p);
			
			//criando evento
			Evento evt;
			evt.num = KERNEL_PORTA_ALOCADA;
			evt.param1 = pid;
			evt.param2 = porta;
			
			//notificando exec��o
			char msg[100];
			memcpy(msg, (char *)&evt, sizeof(Evento));
			notificar_evento(msg, KERNEL_PORTA_ALOCADA);
			portas_alocadas.adicionar(porta);
			
			return MSG_SUCESSO;
		}
	}
	else
	{
	   //Adicionando a Lista de listerners correspondente a porta	
	   switch(porta)
	   {
		 case KERNEL_PORTA_PROC_CRIADO : 
			kernel_listerners_evt_proc_criado.adicionar(pid); break;
		 case KERNEL_PORTA_PROC_DESTRUIDO : 
			kernel_listerners_evt_proc_destruido.adicionar(pid); break;
		 case KERNEL_PORTA_ALOCADA : 
			kernel_listerners_evt_porta_alocada.adicionar(pid); break;
		 case KERNEL_PORTA_DESALOCADA:
			kernel_listerners_evt_porta_desalocada.adicionar(pid); break;
		 case KERNEL_PORTA_VFS: 
			kernel_listerners_evt_vfs.adicionar(pid); break;
		 case KERNEL_PORTA_EXCEPTION:
			kernel_listerners_exception.adicionar(pid); break;
	   }
	   
	   return MSG_SUCESSO;
	}
	
}

//Remove listerner de uma determinada porta 
void Entregador::remover_listerner(unsigned porta)
{
	//criando evento
	Evento evt;
	evt.num = KERNEL_PORTA_DESALOCADA;
	evt.param1 = porta;
			
    //notificando exce��o
    char msg[100];
    memcpy(msg, (char *)&evt, sizeof(Evento));
    notificar_evento(msg, KERNEL_PORTA_DESALOCADA);
			
	//removendo listerner da porta
    portas.remover(porta);

}

//envio de mensagem ass�ncrona para um pid
int Entregador::tratar_envio_pid(unsigned pid_origem, unsigned pid_destino, int tipo, unsigned char msg[TAM_MAX_MSG])
{	
	//enviar mensagem
	Mensagem m;
	m.pid_remetente = pid_origem;
	m.tipo = tipo;
	m.sinalizar_recebimento = 0;
	memcpy(m.conteudo, msg, sizeof(char)*TAM_MAX_MSG);

	//adicionando mensagem
	if(escalonador.adicionar_mensagem(pid_destino, m))
	{
		return MSG_SUCESSO;
	}
	else
	{ 
	    return MSG_ERR_PROC_NAO_EXISTE;
	}
	
}

//Envio de Mensagem S�ncrona para uma porta
int Entregador::tratar_envio(struct REGS *r, unsigned pid, unsigned porta, int sinalizar ,unsigned char msg[TAM_MAX_MSG])
{
	//obtendo porta
	Porta& p = portas[porta];
	
	//Verificando se a porta j� foi alocada
	if(&p != NULL)
	{   
		//criando uma nova mensagem
		Mensagem m;
		m.pid_remetente = pid;
		m.tipo = MSG_COMUM;
		m.sinalizar_recebimento = sinalizar;
		memcpy(m.conteudo, msg, sizeof(char)*TAM_MAX_MSG);
		m.porta_destino = porta;
		
		//adicionando mensagem a fila de mensagens da thread
		if(escalonador.adicionar_mensagem(p.pid_responsavel, m))
		{
			//se a mensagem n�o for origin�ria do KERNEL	
			if(pid != KERNEL_MSG)
			{
				//colocando thread atual em espera
				escalonador.colocar_thread_atual_em_espera();
			}	
				
			//alternar thread
			escalonador.executar_prox_thread(r);
			
			//retorando sucesso	
			return MSG_SUCESSO;
		}
		else
		{		
			//removendo porta
			portas.remover(p.numero);
			return MSG_ERR_PROC_NAO_EXISTE;
		}
	}
	else
	{
		return MSG_ERR_PORTA_NAO_ATIVA;
	}

}

//Recebimento de Mensagem S�ncrona em uma porta
int Entregador::tratar_recebimento(struct REGS *r, unsigned pid, unsigned char * msg, int porta)
{
	int pid_resp = -1;
	
    //Obtendo thread atual
	Porta& p = portas[porta];
	
    //verificando se a porta � v�lida
	if(&p != NULL)
	{
	   pid_resp = p.pid_responsavel;		
	}
	
	//Obtendo �ltima mensagem
	Mensagem& m = escalonador.obter_mensagem(pid, porta, pid_resp);
	
	//Se a refer�ncia for nula, n�o h� mensagens
	if(&m != NULL)
	{		
		//acordando processo que emitiu a mensagem, caso este tenha solicitado
		if(m.sinalizar_recebimento && m.pid_remetente != 0)
		{
			escalonador.acordar_thread(m.pid_remetente);
		}

		//copiando mensagem para espa�o de endere�ameno do processo
		memcpy(msg, (unsigned char *)&m.conteudo, 100);
		
		if(m.tipo != MSG_COMUM)
			r->eax = m.tipo;
		else
			r->eax = m.pid_remetente;

		
		return m.tipo;
	}
	else
	{
		//colocando processo atual em espera
		escalonador.colocar_thread_atual_em_espera();
	    
	    r->eax = 0;
		
		//alternar processo
		escalonador.executar_prox_thread(r);
	
		return NAO_HA_MSGS;
	}	
}
	
//Obt�m lista de Portas e seus respectivos processos respons�veis
void Entregador::obter_info(char * arq)	
{
	
	
	int descritor;
	char linha[150];

	vfs.abrir(arq, 'a', &descritor);
	
	struct Port{
		int pid;
		int porta;
	};
	
	for(int i = 0; i < portas_alocadas.tamanho(); i++)
	{
		Porta &p = portas[portas_alocadas[i]];
		
		if(&p != NULL)
		{
			
			Port info_porta;
			info_porta.pid = p.pid_responsavel;
			info_porta.porta = p.numero;
			
			vfs.escrever(descritor, (char*)&info_porta, sizeof(Port));	
		}
	}
	
	vfs.fechar(descritor);
	
	
}
