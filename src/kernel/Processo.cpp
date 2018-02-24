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
#include "include/Processo.h"
#include "include/Mensagem.h"
#include "include/Memoria.h"
#include "include/Vfs.h"
#include "include/Loader.h"
#include "include/Gdt.h"
#include "include/Util.h"

//========================================================================================================================
//Thread

//realiza as configura��es necess�rias para iniciar uma thread
void Thread::iniciar(unsigned int flags)
{
    unsigned int indice_seg_dados, indice_seg_codigo;
	
	mutex = 0;
	id_dono = -1;
	
    //alterando o estado da thread para pronto
    estado = Processo::PRONTO;
	
	//zerando GPRs
	registradores.eax = registradores.ebx = registradores.ecx = registradores.edx = 0;
	
	//definindo segmentos de acordo com o tipo do processo
	if(tipo == Processo::DRIVER)
	{
	   indice_seg_dados  = 0x21;
	   indice_seg_codigo = 0x19;
	}
	else if(tipo == Processo::USUARIO)
	{
	   indice_seg_dados  = 0x33;
	   indice_seg_codigo = 0x2B;
	}
	else if(tipo == Processo::IDLE)
	{
	   indice_seg_dados  = 0x10;
	   indice_seg_codigo = 0x08;
	   registradores.esp = POS_INICIAL_KERNEL - 0X04;
	}
	
	//configurando registradores de segmento
	registradores.gs  = registradores.es = registradores.fs = registradores.ds = registradores.ss  = indice_seg_dados;
	
	//configurando registrador cs
	registradores.cs  = indice_seg_codigo;
	
	//configurando eflags
	registradores.eflags = flags;
	
	//colocando o valor de entrada do execut�vel em EIP
	registradores.eip  = entrada;
	
	//colocando o valor da pilha em ESP
	inicio_pilha = registradores.useresp = registradores.esp;
}

//Alterna a execu��o para a nova thread (ap�s o retorno da ISR)
void Thread::alternar_para(struct REGS *r)
{
        //carregando novo espa�o de endere�amento
	mem_virtual.carregar_page_directory(pdir);
	
	/*Copiando os registradores da thread para a pilha que ser� restaurada
	ao final do tratamento da interrup��o, trap ou exception*/
        memcpy((unsigned char *)r, (unsigned char *)&registradores, sizeof(REGS));
}

//adiciona mensagem a lista de mensagens
void Thread::adicionar_mensagem(Mensagem msg)
{
    mensagens.adicionar(msg);
}

//remover mensagem da lista e retorn�-la 
Mensagem& Thread::obter_mensagem(int pid, int porta)
{	
	//verificando se existem mensagens
	if(mensagens.tamanho() == 0)
	{
		Mensagem * ret = NULL;
		return *ret;
	}

	//Verficando se h� restri��o de remetente
	if(pid == 0)
	{
		//Caso n�o haja restri��o, retorna primeira mensagem
		Mensagem &m = mensagens[0];
		mensagens.remover(0);
		return m;
	}
	else
	{		
		//Varrendo lista de mensagens
		for(int i =0; i < mensagens.tamanho(); i++)
		{
			//Obtendo mensagem
			Mensagem &m = mensagens[i];
			
			//Verificando se o PID do remetente seja igual ao pid do respons�vel pela porta
			//Ou a porta de destino � igual a porta desejada
			if(m.pid_remetente == pid || m.porta_destino == porta)
			{
				//Retornar a mensagem
				mensagens.remover(i);
				return m;
			}
		}
		
		//Nenhuma mensagem daquela porta foi encontrada
		Mensagem * ret = NULL;
		return *ret;
	}
	
}

void Thread::destruir()
{
    //liberando mem�ria ocupada pelas mensagens
	mensagens.limpar();
	
	//obter processo ao qual a thread pertence
	Processo& p = escalonador.obter_processo(id_processo);
	
	//verificando se o processo ainda existe
	if(&p != NULL)
	{
		//buscando e removendo id da thread
		//da lista de threads do processo
		for(int i =0; i < p.threads.tamanho(); i++)
		{
			if(p.threads[i] == pid)
			{
				p.threads.remover(i);
				break;
			}
		}
		
		p.pilhas_livres.adicionar(inicio_pilha);
	}
}

//========================================================================================================================
//Processo

//copia uma page directory
void Processo::criar_espaco_enderecamento(unsigned int * fonte)
{	

	((int *)mem_virtual.pt_aux)[0] = (unsigned int )0|2;
	
    //copiando o espaco de enderecos
	for(int i = 1; i < 1024; i++)
	{
	   ((int *)mem_virtual.pt_aux)[i] = fonte[i];
	}
	
	//configurando a �ltima entrada para apontar para a pr�pria page directory
	((int *)mem_virtual.pt_aux)[1023] = ((unsigned int)(pdir)) | 7;
	
	
}

//cria um processo na mem�ria a partir da imagem de arquivo execut�vel ELF
int Processo::criar_processo(unsigned int p, unsigned char * n,unsigned char * imagem, int ptipo, int argc, char * args[])
{
   //declara��es de var�aveis
   Lista<ELF_SEG_T> secoes;
   unsigned int pos_virtual  = 0;
   unsigned int mem_a_alocar = 0;
   unsigned int backup_pdir  = 0;	
   unsigned int blocos_a_alocar;
   unsigned int pdir_virtual;
   
   
   mutex = 0;
   id_dono = -1;
   
   //atribui tipo ao processo
   tipo = ptipo;
   
   //atribui nome ao processo
   memcpy(this->nome, n, strlen(n)+1);

   //obtendo valor de CR3 (endere�o da page direcoty atual)
   __asm__ __volatile__ ("mov %%cr3, %0":"=b"(backup_pdir));	 
   
   //atribuindo pid;	
   pid = p;
   
   //atribuindo estado inicial (ainda n�o iniciado)
   estado = 0;
		
   //iniciando o espa�o de endere�amento com 0		
   end_virtual  = 0;
   
   //lendo o cabe�alho do arquivo ELF e obtendo as informa��es sobre as se��es
   secoes = load_elf_exec(imagem, &entrada, secoes);
   
   //caso n�o existam se��es, o arquivo � inv�lido
   if(secoes.tamanho() == 0)
   {   
	  free(imagem); 
      return PROC_FORMATO_INVALIDO;
   }
   
   //somando o tamanho ocupado por cada se��o
   for(unsigned int i =0; i < secoes.tamanho(); i++)
   {
        //acumulando a quantidade de mem�ria que ser� ocupada pela se��o
		mem_a_alocar += secoes[i].mem_size;
   }
  
   //calculando quantos blocos de 4kb ser�o necess�rios
   blocos_a_alocar = mem_a_alocar/4096 + 1; 

   //zerando o espa�o de endere�amento (endere�o f�sico da page directory)
   pdir = mem_fisica.alocar(1);
   if(pdir == 0) return PROC_SEM_MEMORIA;
   
   //mapeando endere�o f�sico da nova page table para o endere�o virtual tempor�rio 
   mem_virtual.mapear_page_table(pdir, mem_virtual.pt_aux, 0x07);
	
   //zerando o conte�do da page directory 
   memset((unsigned char *)mem_virtual.pt_aux, 0, sizeof(unsigned int) * 1024);
   
   //faz uma copia da page directory do kernel para a nova page directory do processo
   criar_espaco_enderecamento(mem_virtual.obter_pdir());
   
   /*carrega a page directory do processo, para realizar a copia 
   das se��es do arquivo para o novo espa�o de endere�amento*/
   mem_virtual.carregar_page_directory(pdir);
    
   //alocando a mem�ria f�sica e mapeando a mem�ria virtual
   for(unsigned int i =0; i < blocos_a_alocar; i++)
   {
     //alocando um bloco de endere�os f�sicos
     unsigned int end_fisico = mem_fisica.alocar(1);	 
	 if(end_fisico == 0) return PROC_SEM_MEMORIA;
	 enderecos_fisicos.adicionar(end_fisico);	  

	 //mapeando novo endere�o f�sico em um endere�o virtual
	 unsigned int end_pt = (unsigned int)mem_virtual.mapear_page_table_espaco_end(end_fisico, end_virtual, 0x07, mem_virtual.pt_aux);		
     enderecos_fisicos.adicionar(end_pt);	  
	 
	 //aumentando o endere�o virtual
     end_virtual+=4096;	  
   }
    
	//mapeando a Page Directory no novo espa�o de endere�os
    unsigned int end_pt;
    pdir_virtual = end_virtual;
	end_pt = (unsigned int)mem_virtual.mapear_page_table_espaco_end(pdir, pdir_virtual, 0x07, mem_virtual.pt_aux);		
	end_virtual+=4096;
	
	//alocando pilha
	for(int i =0; i < QTD_FRAMES_PILHA; i++)
	{
		//mapeando a pilha no novo espa�o de endere�os
		unsigned stack = mem_fisica.alocar(1);
		if(stack == 0) return PROC_SEM_MEMORIA;
		end_pt = (unsigned int)mem_virtual.mapear_page_table_espaco_end(stack, 0xC0000000 - (4096 * i), 0x07, mem_virtual.pt_aux);		
		enderecos_fisicos.adicionar(stack);
		enderecos_fisicos.adicionar(end_pt);
	}
	
	//Apontando a pilha para o final do espa�o de endere�amento
	registradores.esp = 0xC0000000 - 4; 
	registradores.ebp = 0xC0000000 - 4; 
	
	//calculando a posi��o do come�o da pr�xima pilha
	proxima_pilha = 0xC0000000 - QTD_FRAMES_PILHA*4096;
	
    //alocando primeiro bloco para o HEAP
    unsigned int inicio_heap = mem_fisica.alocar(1);
    end_pt = (unsigned int)mem_virtual.mapear_page_table_espaco_end(inicio_heap, end_virtual, 0x07, mem_virtual.pt_aux);		  
    enderecos_fisicos.adicionar(end_pt);
    enderecos_fisicos.adicionar(inicio_heap);
  
    //zerando heap; 
    heap = 0;	
    ult_end_ret = end_virtual;
   
    //configurando a pilha
    //nos primeiros endere�os do heap, deve existir um array que aponte para os par�metros	
	unsigned char ** end_args = end_virtual;
	unsigned char * inicio_parametros = end_virtual + sizeof(char*) * argc;
	
	//copiando par�metros para espa�o de endere�os do processo
	for(int i =0; i < argc; i++)
	{
	    //obtendo o tamanho do parametro
	    int tam = strlen(args[i])+1;
		
	    //criando o array de ponteiros que aponta para cada par�metro
		end_args[i] = (unsigned char*)((unsigned int)inicio_parametros + heap);
		
		//copiando os dados para o heap do processo. Fazendo com que seja poss�vel acessar os dados de dentro
		//do processo
		memcpy( (unsigned char*)((unsigned int)inicio_parametros + heap), args[i], tam);
		
		//incrementando o topo do heap
		heap+=tam;
	}
	
	//adicionando par�metros na pilha
	int * ptr_pilha = (unsigned int) registradores.esp ; 
	registradores.esp-=4;
	
	//adicionando ponteiro para array de parametros;
	*ptr_pilha = end_args;
	
	//diminuindo pilha para armazenar pr�ximo ponteiro
	ptr_pilha = (unsigned int) registradores.esp ; 
	registradores.esp-=4;
	
	//adicionando qtd de argumentos na pilha
	*ptr_pilha = argc;
	registradores.ebp = registradores.esp;
	end_virtual+=4096;
	
    //copiando os dados do heap do kernel para a nova �rea da mem�ria
    for(unsigned int i =0; i < secoes.tamanho(); i++)
    {
      unsigned char * pos = (unsigned char *)imagem;
	  pos += secoes[i].offset;
	  
	  //caso o tamanho na mem�ria seja maior que o tamanho no disco, setor BSS
	  if(secoes[i].mem_size > secoes[i].disk_size)
	  {
	    memset(secoes[i].virt_adr, 0, secoes[i].mem_size);
	  }
	  
	  memcpy(secoes[i].virt_adr, pos, secoes[i].disk_size);
    } 
	
	//limpando lista de se��es
	secoes.limpar();
	
    //restaurando espa�o de endere�amento do kernel
    //mem_virtual.carregar_page_directory(mem_virtual.pdirptr);
    mem_virtual.carregar_page_directory(backup_pdir);
  
    //liberando o espa�o ocupado pelo arquivo no heap do kernel
    free(imagem); 
   
    //criando thread inicial para o processo
	thread_inicial = criar_thread( p, entrada, registradores.esp);
	
    return PROC_SUCESSO;   
}

//cria uma nova thread
Thread Processo::criar_thread(int id, int ponto_entrada, int pilha)
{
	Thread thread;
	
	//atribuindo pid do processo ao qual a thread est� atrelada
	thread.id_processo = pid;
	
	//atribu�ndo id da thread
	thread.pid = id;
	
	//zerando valor dos registradores
    memset((unsigned char *)&thread.registradores, 0, sizeof(REGS));
	
	//atribuindo valores a thread
    thread.tipo 			= tipo;
    thread.estado 			= estado;            		  
	thread.pdir				= pdir;			   		  
   
	//atribu�ndo ponto de entrada
    thread.entrada 			= ponto_entrada;
	
	//atribu�ndo endere�o do come�o da pilha
	thread.registradores.esp = pilha;
	thread.registradores.ebp = pilha;
	
	//adicionando id da thread a lista de threads
	threads.adicionar(id);
	
	//retornando thread
	return thread;
}

//aumenta a quantidade de mem�ria no heap do processo
void Processo::aumentar_heap(unsigned int * end, unsigned int tam)
{
    //mapeando endere�o f�sico da nova page table para o endere�o virtual tempor�rio 
    mem_virtual.mapear_page_table(pdir, mem_virtual.pt_aux, 0x07);

    /*
	  A vari�vel heap armazena a marca��o
	  de quanta mem�ria dentro de um frame est� sendo utilizada.
	  Quando o valor dessa var�avle estoura o tamanho do fram 4096
	  mais mem�ria f�sica deve ser alocada. 
	*/
	heap += tam;
	
	/*
	  A vari�vel ult_end_ret armazena o �ltimo endere�o retornado;
	  � partir dele � feito o calculo para definiri qual ser� o pr�ximo
	  endere�o a ser retornado.
	*/
	ult_end_ret += tam;
	
	//a vari�vel ret recebe o endere�o do novo bloco de mem�ria
	*end = (void *)ult_end_ret;
	
	if( heap >= 4096)
	{
	   //calculando quantos blocos de mem�ria dever�o ser alocados
	   unsigned int num_blocos_alocar = (heap)/4096 + 1;
	   
	   //mapeando as p�ginas na mem�ria virtual � medida que forem necess�rias
	   for(unsigned int i =0; i < num_blocos_alocar; i++)
	   {
	      //alocando mem�ria f�sica
		  unsigned int ret = mem_fisica.alocar(1);
	
		  if (ret == 0)
		  {
             *end = 0; 		  
			 break;
		  }		
		  else
		  {
		    //adicionando mem�ria a lista de blocos f�sicos utilizados pelo processo
			enderecos_fisicos.adicionar_em(0, ret);
		  }
		  
		  //fazendo o mapeando do bloco para o endere�o virtual
	      ret = (unsigned int)mem_virtual.mapear_page_table_espaco_end(ret, end_virtual, 0x07, mem_virtual.pt_aux);		
		  
		  if(ret != 0)
		    enderecos_fisicos.adicionar_em(0,ret);
			
		  //incrementado o endere�o virtual que ser� utilizado no pr�ximo mapeamento.	
	      end_virtual+=4096;
		
	   }  
		
	   //o heap dever� conter a quantidade de mem�ria que est� sendo utilizada do �ltimo frame alocado	
	   heap = heap%4096;
	}
	
	//up();	
}

//cria uma nova pilha para uma thread
int Processo::obter_nova_pilha()
{
	unsigned int aux;

	if(pilhas_livres.tamanho() > 0)
	{
		aux = pilhas_livres[0];
		pilhas_livres.remover(0);
	}
	else
	{
		aux = proxima_pilha;
		
		//mapeando endere�o f�sico da nova page table para o endere�o virtual tempor�rio 
		mem_virtual.mapear_page_table(pdir, mem_virtual.pt_aux, 0x07);
		
		//alocando pilha
		for(int i =0; i < QTD_FRAMES_PILHA; i++)
		{
			//mapeando a pilha no novo espa�o de endere�os
			unsigned stack = mem_fisica.alocar(1);
			if(stack == 0) return PROC_SEM_MEMORIA;
			unsigned end_pt = (unsigned int)mem_virtual.mapear_page_table_espaco_end(stack, proxima_pilha - (4096 * i), 0x07, mem_virtual.pt_aux);		
			enderecos_fisicos.adicionar(stack);
			enderecos_fisicos.adicionar(end_pt);
		}
		
		//calculando a posi��o da pr�xima pilha a ser retornada
		proxima_pilha = proxima_pilha - QTD_FRAMES_PILHA*4096;
		
	}
	
	return aux;
}

//Desaloca todos os frames de mem�ria f�sica ocupados pelo processo
void Processo::destruir()
{	
    //liberando mem�ria f�sica ocupada pelo processo
    for(unsigned int i = 0; i < enderecos_fisicos.tamanho(); i++) 
	{
		unsigned end = enderecos_fisicos[i]; 
		
	    if(end != 0);
		  mem_fisica.desalocar(end,1);
	}
	
    //liberando mem�ria ocupada pela page directory do processo
	mem_fisica.desalocar(pdir,1);
	
	//liberando mem�ria da lista de endere�os f�sicos
	enderecos_fisicos.limpar();	
	
	//liberando mem�ria da lista pilhas livres
	pilhas_livres.limpar();
	
	//limpando a lista de threads
	threads.limpar();
	
	
}

//========================================================================================================================
//Escalonador
volatile Escalonador escalonador;

//Inicializa o escalonador
void Escalonador::inicializar()
{
	mutex = 0;
	id_dono = -1;
	prox_pid = 1;
	rec_num = 8;
	threads.rec_num = 1;
	processos.rec_num = 2;
	
	
	memset((char *)thread_em_execucao,0, sizeof(int)*16);
	//thread_em_execucao[0] = 1;
	//thread_em_execucao[1] = 1;
	escalonamento_ativo = 0;

	/*
	  Alterando IOPL (n�vel de privil�gio para opera��es de entrada e sa�da).
	  Para permitir apenas que processo que esteja no RING01 possam fazer E\S.
	*/
	
	int val;
    	asm volatile("pushf"); 						//Empilhando conte�do de EFLAGS.
	asm volatile("pop %eax"); 					//Salvando em EAX.
	asm volatile("movl %%eax, %0" : "=r"(val)); 			//Salvando valor de EAX em val.
	val = val | 0x1000; 						//Setando o 12� bits.
    	asm volatile("push %0" :: "r" (val)); 				//Empilhando VAL.
	asm volatile("popf");						//Alterando o valor de EFLAGS.
}
	
//Adiciona um novo processo
int Escalonador::adicionar_processo(unsigned char * nome, unsigned char * imagem, int tipo, int argc, char * args[])
{

	//Declarando um novo processo
	Processo novo_processo;
	
	//Criando processo
	int retorno = novo_processo.criar_processo(prox_pid, nome, imagem, tipo, argc, args);

	//incrementando contador para atribui��o do pr�ximo PID
	prox_pid++;
	
	//Verificando se o processo foi carregado com sucesso
	if(retorno == PROC_SUCESSO)
	{		
		//Atribu�ndo pid do processo pai
		novo_processo.pid_pai = obter_pid_em_execucao();
		
		//adicionando a lista de processos, sendo o PID a chave para acess�-lo
	    	processos.adicionar(novo_processo.pid, novo_processo);
		
		//adicionando thread principal a lista de threads
		threads.adicionar(novo_processo.thread_inicial.pid, novo_processo.thread_inicial);
		
		//colocando o processo na lista dos processos prontos
		if(novo_processo.tipo == Processo::USUARIO)
		{
			threads_prontas.adicionar(novo_processo.thread_inicial.pid);
		}
		else if(novo_processo.tipo == Processo::DRIVER)
		{
			drivers_prontos.adicionar(novo_processo.thread_inicial.pid);
		}
		
		if(escalonamento_ativo)
		{
			//criando evento
			Evento evt;
			evt.num = KERNEL_PORTA_PROC_CRIADO;
			evt.param1 = novo_processo.pid;
			
			char msg[100];
			memcpy(msg, (char *)&evt, sizeof(Evento));
			entregador.notificar_evento(msg, KERNEL_PORTA_PROC_CRIADO);
		}
	}
		
	return retorno;
}

//Escalonador adicionar thread
int Escalonador::adicionar_thread(Processo &p, int ponto_entrada)
{
	if(&p != NULL)
	{
		//criando nova thread
		Thread nova_thread = p.criar_thread(prox_pid++, ponto_entrada, p.obter_nova_pilha());
		
		//adicionando thread  a lista de threads
		threads.adicionar(nova_thread.pid, nova_thread);
		
		//adicionando thread a lista de threads prontas
		if(nova_thread.tipo == Processo::USUARIO)
		{
			threads_prontas.adicionar(nova_thread.pid);
		}
		else if(nova_thread.tipo == Processo::DRIVER)
		{
			drivers_prontos.adicionar(nova_thread.pid);
		}
		
		return nova_thread.pid;
	}
	else
	{
		return 0;
	}
}

//Configura processo IDLE(tempo ocioso do sistema)
void Escalonador::adicionar_idle(unsigned char * imagem)
{
	//Declarando um novo processo
	Processo novo_processo;
		
	//Criando processo
	int retorno = novo_processo.criar_processo(prox_pid++, "idle", imagem, Processo::IDLE, 0, NULL);
	
	//Verificando se o processo foi carregado com sucesso
	if(retorno == PROC_SUCESSO)
	{
		//adicionando a lista de processos, sendo o PID a chave para acess�-lo
	    	processos.adicionar(novo_processo.pid, novo_processo);
		
		//adicionando a thread principal a lista de threads
		threads.adicionar(novo_processo.thread_inicial.pid, novo_processo.thread_inicial);
	}
		
	pid_idle = novo_processo.pid;
}

//Eliminar processo	
void Escalonador::eliminar_processo(int pid)
{
	eliminar_processo(processos[pid]);
}

void Escalonador::eliminar_processo(Processo &p)
{
	//Verificando se o processo existe
	if(&p != NULL)
	{
		//lista temporaria de threads
		Lista<int> threads_tmp;
		
	    	//Obtendo pid	
		unsigned int pid = p.pid;
			
		//copiando para a lista aux�liar
		for(unsigned int i = 0; i < p.threads.tamanho(); i++) 
		{
			threads_tmp.adicionar(p.threads[i]);
		}
		
		//colocando o estado das threads em finalizado
		for(unsigned int i = 0; i < threads_tmp.tamanho(); i++) 
		{
			Thread &t = escalonador.obter_thread(threads_tmp[i]);
			
			if(&t != NULL)
			{
				t.estado = Processo::FINALIZADO;	
				escalonador.eliminar_thread(t);
			}
		}
		
		//limpando lista tempor�ria
		threads_tmp.limpar();
		
		//Liberando mem�ria ocupada pelo processo
		p.destruir();
			
		//Removendo processo da lista de processos
		processos.remover(pid);
		
		//up no mutex
		up();
		
		//criando evento
		Evento evt;
		evt.num = KERNEL_PORTA_PROC_DESTRUIDO;
		evt.param1 = pid;
		
		char msg[100];
		memcpy(msg, (char *)&evt, sizeof(Evento));
		
        entregador.notificar_evento(msg, KERNEL_PORTA_PROC_DESTRUIDO);	
	}
}

//Elmina uma thread
void Escalonador::eliminar_thread(Thread &t)
{
    if(&t != NULL)
	{
		//destruindo thread
		t.destruir();
		
		//removendo thread da lista
		threads.remover(t.pid);
	}
}

//M�todo que escalona a pr�xima thread a ser executada
void Escalonador::executar_prox_thread(struct REGS *r)
{	
	//obtendo id do processador
	int id_processador = multiproc.obter_apicid();
	
	//Obtendo a thread atualmente em execu��o na lista de processos		 
	Thread& atual = threads[thread_em_execucao[id_processador]];
	
	if( !((thread_em_execucao[id_processador] == pid_idle) 
		   && ( drivers_prontos.tamanho() == 0 )
		   && ( threads_prontas.tamanho() == 0 )
		   && ( atual.estado == Processo::PRONTO)))
	{
		 if(&atual != NULL)
		 {	  
			  //Em caso da thread estar no estado pronto(thread atual sofrendo preemp��o)	  
			  if(atual.estado == Processo::PRONTO)
			  {
				  //salvando contexto (copiando os registradores da pilha para o PCB)
				  memcpy((unsigned char *)&atual.registradores, (unsigned char *)r, sizeof(REGS));
				  
				  //colocando a thread de volta a lista de threads prontas
				  if(atual.tipo == Processo::USUARIO)
				  {
					  threads_prontas.adicionar(atual.pid);
				  }
				  else if(atual.tipo == Processo::DRIVER)
				  {
					  drivers_prontos.adicionar(atual.pid);
				  }
			  }
			  //Em caso de threads que executaram uma chamada bloqueante
			  else if(atual.estado == Processo::ESPERANDO)
			  {
				  //salvando contexto	
				  memcpy((unsigned char *)&atual.registradores, (unsigned char *)r, sizeof(REGS));
			  } 
		 }
		 
		 //declarando pr�xima thread
		 Thread * prox;	  
		 
		 do
		 {
			  //obtendo pr�xima thread a ser executada		
			  if(drivers_prontos.tamanho() > 0)
			  {
				 thread_em_execucao[id_processador] = drivers_prontos[0];
				 drivers_prontos.remover(0);
			  } 
			  else if(threads_prontas.tamanho() > 0)
			  {
				 thread_em_execucao[id_processador] = threads_prontas[0];
				 threads_prontas.remover(0);
			  }
			  else
			  {
				 thread_em_execucao[id_processador] = pid_idle;
			  }
				
			  //obtendo pr�ximo processo da fila
			  prox = &threads[thread_em_execucao[id_processador]];        	
			  
		 }while(prox == NULL);
		
		 //inicia a thread caso ela n�o tenha sido iniciada
		 if((prox->estado != Processo::PRONTO) 
		   && (prox->estado != Processo::FINALIZADO))
		 {
			  //executando tarefa pela primeira vez 
			  prox->iniciar(r->eflags);		  
		 }
			
		 //alternando para pr�xima thread
		 prox->alternar_para(r);
		 
		/*
		unsigned short *textmemptr = 0xC00B8000;
		char num[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
		static unsigned char c[] = {0, 0, 0, 0};
		textmemptr[35 + 3*id_processador] = num[prox->pid/10] | (0x0F << 8);
		textmemptr[36 + 3*id_processador] = num[prox->pid%10] | (0x0F << 8);
		*/
	}
}

//M�todo que adiciona processo a lista de processos prontos
void Escalonador::adicionar_thread_pronta(Thread t)
{
	if(t.tipo == Processo::DRIVER)
	{
		drivers_prontos.adicionar(t.pid);
	}
	else
	{
		threads_prontas.adicionar(t.pid);
	}
}

//Altera o estado da thread para PRONTO
void Escalonador::acordar_thread(int pid)
{
	////down no mutex
	down();
	
	Thread& t = threads[pid];
	
	if(&t != NULL)
	{
		if(t.estado == Processo::ESPERANDO)
		{
			//alterando o estado do thread destinat�ria para pronto
			t.estado = Processo::PRONTO;
				
			//adicionando a lista de prontos
			adicionar_thread_pronta(t);
		}
	}
	
	//up no mutex
	up();
}

//Coloca thread atualment em execu��o em estado de espera
void Escalonador::colocar_thread_atual_em_espera()
{
	//obtendo inst�ncia da thread atual
	Thread& t_atual = threads[obter_pid_em_execucao()];
	
	//alterando estado para ESPERANDO
	t_atual.estado = Processo::ESPERANDO;
}

//adiciona uma mensagem a um processo
int Escalonador::adicionar_mensagem(int pid, Mensagem m)
{
	int ret = false;
	
	//down no mutex
	down();

	Thread& t = threads[pid];
	
	if(&t != NULL)
	{
		//adicionando mensagem a lista
		t.adicionar_mensagem(m);
			
		//verificando se o processo est� em estado de espera	
		if(t.estado == Processo::ESPERANDO)
		{
			//alterando o estado do thread destinat�ria para pronto
			t.estado = Processo::PRONTO;
				
			//adicionando a lista de prontos
			adicionar_thread_pronta(t);
		}
		
		ret = true;
	}
	
	//up no mutex
	up();
	
	return ret;
}

//obtem mensagem 
Mensagem& Escalonador::obter_mensagem(int pid, int porta, int pid_responsavel)
{
	Thread& t = threads[pid];
	
	if(&t != NULL)
	{
		//Obtendo �ltima mensagem
		if(porta == MSG_QLQR_PORTA)
		{
			pid_responsavel = 0;
		}
	
		return t.obter_mensagem(pid_responsavel, porta);

	}
	
}
	
//M�todo que retorna processo pelo PID
Processo& Escalonador::obter_processo(unsigned int pid)
{
   return processos[pid];
}

Thread& Escalonador::obter_thread(unsigned int pid)
{
	return threads[pid];
}

//Retorna o pid do processo em execu��o no momento
int Escalonador::obter_pid_em_execucao()
{
	return thread_em_execucao[multiproc.obter_apicid()];
}

//M�todo para notificar o Escalonador da ocorr�ncia de uma Exception
void Escalonador::notificar_exception(unsigned int int_no, struct REGS *r)
{
   Thread& thread = threads[thread_em_execucao[multiproc.obter_apicid()]];
   Processo& atual = processos[thread.id_processo];
   eliminar_processo(atual);
   executar_prox_thread(r); 
}

//Retorna 1 caso uma thread de um processo servidor (Driver) esteja em execu��o, e 0 caso n�o esteja
int Escalonador::driver_em_execucao()
{
	return threads[thread_em_execucao[multiproc.obter_apicid()]].tipo == Processo::DRIVER;
}

//Obt�m lista de processos em mem�ria 
void Escalonador::obter_info(unsigned char * arq)
{
	down();
	int descritor;
	char linha[150], buf[32];

	vfs.abrir(arq, 'a', &descritor);
	
	for(int i = 1; i < prox_pid; i++)
	{
		Processo &proc = processos[i];
		
		if(&proc != NULL)
		{
				
			struct Proc
			{
				int pid;
				char nome[40];
				int threads;
				int tamanho;
			};
			
			Proc p;
			p.pid = proc.pid;
			p.threads = proc.threads.tamanho();	
				
			memset(p.nome, ' ', 40);
			p.nome[39] = '\0';
			memcpy(p.nome, proc.nome, strlen(proc.nome));
			
			p.tamanho = 0;
			
			for(int cont =0; cont < proc.enderecos_fisicos.tamanho(); cont++)
			{
				if(proc.enderecos_fisicos[cont] != 0)
					p.tamanho += 4;
			}
			
			vfs.escrever(descritor, (char *)&p, sizeof(Proc));	
			
		}
	}
	
	vfs.fechar(descritor);
	
	up();
}

