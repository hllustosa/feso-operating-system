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
#ifndef _PROC
#define _PROC

#include "Lista.h"
#include "Avl.h"
#include "Idt.h"
#include "Mensagem.h"
#include "Multiprocessamento.h"

#define QTD_FRAMES_PILHA 10

/**
* Constantes que definem o resultado do carregamento de um processo.
*/
const int PROC_SUCESSO  = 1;
const int PROC_FORMATO_INVALIDO = 2;
const int PROC_SEM_MEMORIA = 3;


/**
*Estrutura de controle que representa uma thread no sistema. 
*/
class Thread : public Recurso
{
   public :
   
   Lista<Mensagem> mensagens; 				/*!<Lista de mensagens.*/
   REGS registradores;                    		/*!<Contexto de hardware, ou a estrutura contendo estado dos registradores da thread.*/
   unsigned int id_processo;				/*!<PID do processo ao qual a thread está alocada.*/   
   unsigned int pid;               		  	/*!<Identificador da thread.*/   
   unsigned int pid_pai;               	  		/*!<Identificador do pai.*/   
   unsigned int tipo;            		  	/*!<Tipo da thread: (COMUM, DRIVER e IDLE). */
   unsigned int estado;            		        /*!<Estado da thread: (PRONTO, ESPERANDO e FINALIZADO). */
   unsigned int pdir;			   		/*!<Endereço físico da page directory (espaÃ§o de endereÃ§amento). */
   unsigned int entrada;		   		/*!<Endereço do ponto de entrada para a thread). */
   unsigned int inicio_pilha;			  	/*!<Endereço da primeira posição do frame da pilha da thread). */
	
   /**
   * Inicializa o escalonador, atribui valor 0 para todas as posiÃ§Ãµes do array threads_em_execucao. E também configurar os bits IOPL do registrador EFLAGS para permitir que o código funcionando no
   * ring1 possa executar instruÃ§Ãµes de E/S.
   */  	
   void iniciar(unsigned int);

   /**
   *Alterna a execução para a nova thread (após o retorno da ISR).
   */  
   void alternar_para(struct REGS *);

   /**
   *Adiciona mensagem a lista de mensagens.
   */
   void adicionar_mensagem(Mensagem msg);

   /**
   *Interrompe a thread.
   */
   void destruir();

   /**
   *Remove mensagem da lista a retorna.
   */
   Mensagem& obter_mensagem(int pid, int porta);
   
};


/**
* Esta classe armazena as informaÃ§Ãµes sobre um processo (PCB - Process Controle Block). Nesta classe, ficam armazenados o tipo do processo, sua lista de threads e os frames de memória ocupados. 
* Essa classe também contém métodos utilizados na criação e remoção de processos da memória.
*/
class Processo: public Recurso
{
  
   public :
   
   static const int NAO_INICIADO = 0;	/*!<Constante que indica o estado de um processo ainda não iniciado.*/
   static const int PRONTO       = 1;	/*!<Constante que indica o estado de um processo pronto para a execução.*/
   static const int ESPERANDO 	 = 2;	/*!<Constante que indica o estado de um processo em estado de espera.*/ 
   static const int EXECUTANDO 	 = 3;	/*!<Constante que indica o estado de um processo em execução na CPU.*/ 
   static const int FINALIZADO 	 = 4;	/*!<Constante que indica o estado de um processo que foi finalizado.*/ 

   static const int DRIVER       = 0;	/*!<Constante que indica que o processo atual é de um driver de dispotivo.*/ 
   static const int USUARIO      = 1;	/*!<Constante que indica que o processo atual é de um processo de usuário.*/ 
   static const int IDLE         = 2;   /*!<Constante que indica que o processo atual é um processo especial para manter a CPU parada.*/ 
   
   char nome[15];			/*!<Nome do processo.*/ 
   Thread thread_inicial;		/*!<Thread principal criada junto com o processo.*/

   Lista<unsigned int> enderecos_fisicos;  /*!<Lista encadeada contendo os frames (blocos de 4kb) Endereços fÃ­sicos ocupados pelo processo. Sempre que um novo frame são alocado por um processado, ele é adicionado a essa lista.*/
   Lista<unsigned int> threads;		   /*!<Uma lista encadeada contendo os PIDs das threads do processo. As instâncias da threads ficam em uma estrutura na classe escalonador. */	
   Lista<unsigned int> pilhas_livres;	   /*!<Uma lista encadeada contendo Endereços de pilhas livres. */	
   REGS registradores;                    
   
   unsigned int pid;               	   /*!<Id do processo (é igual ao id da thread principal). */
   unsigned int pid_pai;               	   /*!<Id do processo pai. */
   unsigned int tipo;            	   /*!<Tipo do processo. */
   unsigned int estado;            	   /*!<Estado do processo. */
   unsigned int end_virtual;       	   /*!<variável que aponta para o próximo endereço virtual a ser alocado para o heap do processo. Sempre que um novo bloco de memória é alocado para o heap essa variável é incrementada em 4096 (4kb). */
   unsigned int pdir;			   /*!<Endereço da Page Directory. */
   unsigned int heap;			   /*!<Armazena o Endereço virtual do topo do heap do processo. Sempre que uma quantidade N de bytes é alocada, essa variável é incrementada em N. */							
   unsigned int ult_end_ret;		   /*!<Armazena o ultimo Endereço virtual retornando pelo método que aumenta o heap do processo. */
   unsigned int proxima_pilha;		   /*!<Endereço virtual da próxima posição de memória em que será localizada o pilha da thread. O SO por padrão dá a cada thread uma área de pilha igual a constante QTD_FRAMES_PILHA vezes (4096). Essa variável é incrementada sempre que uma thread é criada, e indica a posição de memória virtual na qual a próxima pilha estará. */
   unsigned int entrada;		   /*!<Ponto de entrada do processo (é o mesmo ponto de entrada da thread principal).*/
    
   /**
   *Método chamado pelo escalonador ao se adicionar um novo processo. Esse método recebe um inteiro P que é ID do novo processo, uma string nome, contendo o nome do novo processo, o ponteiro imagem, que contém o Endereço da primeira posição de memória ocupada pela imagem do arquivo executável que será carregado, ptipo que indica qual será o tipo de processo criado, argc e args[] que sÃ£o respectivamente a quantidade de parâmetros, e os parâmetros. Esse método analisa a imagem do arquivo executável, e obtém uma lista de setores. Esses setores sÃ£o carregados na memória nas posiÃ§Ãµes indicadas pelo cabeÃ§alho do arquivo executável. O método também cria a pilha armazena os parâmetros nela,  e aloca a primeira regiÃ£o do heap.
   */
   int criar_processo(unsigned int p, unsigned char * nome, unsigned char * imagem, int ptipo, int argc, char * args[]);
   
   /**
   *Método que cria uma nova thread em um processo. Ele recebe por parâmetro o novo PID que será atribuída a thread. o ponto de entrada a partir do qual a thread será executada, e a nova pilha para  thread. 
   */
   Thread criar_thread(int id, int ponto_entrada, int pilha);

   /**
   *Aloca os frames de memória necessários para a nova pilha, retorna o Endereço do começo da nova pilha, e atualiza o valor da variável prox_pilha.
   */
   int obter_nova_pilha();

   /**
   *Recebe por parâmetro a page directory padrão (com todas as page tables do Kernel já criadas e mapeadas corretamente) e cria uma nova page directory como copia dessa.
   */
   void criar_espaco_enderecamento(unsigned int *);

   /**
   *Incrementa o heap do kernel pela quantidade de bytes especificada pela variável tam. Coloca o Endereço da nova área alocada no Endereço de memória indicado por end.
   */
   void aumentar_heap(unsigned int * end, unsigned int tam);

   /**
   *Liberta todos os frames de memória ocupados pelo processo, e altera o estado de todas as threads para finalizado. 
   */	
   void destruir();
   
};

/**
* Esta classe é responsável por manter o sistema de gerência de processos. Ela é responsável por decidir qual é próximo processo que deve ser posto em execução. É ela que realiza a criação e eliminação dos processos.
*/
class Escalonador : public Recurso
{
	public :
	
	unsigned int prox_pid;                    /*!<Cada thread possui um PID, e cada processo também possui um PID. De forma que a primeira thread (ou a thread principal) possui o mesmo PID do processo ao qual ela está vinculada. A cada criação de thread/processo é atribuída a ele o valor contido na variável prox_pid, sendo essa variável incrementada em 1 sempre que isso ocorre..*/ 
	unsigned int pid_idle;			  /*!<A thread IDLE é a thread executada quando todas as outras estÃ£o impossibilitadas de executar. O escalonador coloca em execução a thread com esse PID sempre que nÃ£o há outra disponÃ­vel. Essa thread paralisa o processador até que uma interrupção ocorra.  */
        volatile int escalonamento_ativo; 	  /*!<Flag utilizada para ativar o escalonamento de processos após a inicialização do SO. (0 = inativo, 1 = ativo). */
	unsigned int thread_em_execucao[16];      /*!<Array de 16 posiÃ§Ãµes contendo a thread em execução em cada um dos núcleos. Os núcleos possuem um ID (que é o ID dos seu respectivo APIC). O id do núcleo é utilizado como Ã­ndice dentro do array.  EntÃ£o o PID da thread que estiver em execução no processador 0, está em thread_em_execucao[0]. */   	

	Lista<unsigned int> threads_prontas;      /*!<Uma lista encadeada contendo os PIDs das threads que estÃ£o prontas para a execução. Esta lista contém apenas as threads prontas para execução de processos do usuário. Ou seja, as threads com baixa prioridade. */
	Lista<unsigned int> drivers_prontos;      /*!<Uma lista encadeada contendo os PIDs das threads dos drivers de dispostivos que estÃ£o prontos para a execução. SÃ£o as threads com prioridade mais alta. */

	Arvore<Processo> processos; 	          /*!<Estrutura que armazena todos os processos que estÃ£o em memória no momento.  */
	Arvore<Thread> threads; 		  /*!<Estrutura que armazena todas as threads que estÃ£o em memória no momento. */
	

	/**
	*Inicializa o escalonador, atribui valor 0 para todas as posiÃ§Ãµes do array threads_em_execucao. E também configurar os bits IOPL do registrador EFLAGS para permitir que código funcionando no ring1 possa executar instruÃ§Ãµes de I/O.
	*/
	void inicializar();

	/**
        *Função que serve para decidir qual é a próxima thread a ser executada. Esse método recebe um ponteiro para uma estrutura reg. Sempre que um evento (IRQ, Exception, Interrupção de Software)
        *ocorre, os registradores da CPU (sejam os de uso geral, como os registradores de segmento) sÃ£o postos na pilha. O conteúdo do registrador ESP é passado como parâmetro para todas as funÃ§Ãµes que
        *realizam o tratamento de eventos. Dessa forma estrutura regs representa o estado da pilha no momento em que um evento está sendo tratado pelo SO. 
        *Se algum dado é escrito nessa estrutura, ele será automaticamente escrito na pilha, e quando a instrução IRET da função de tratamento de interrupÃ§Ãµes for executado, os dados contidos na pilha
        *voltarÃ£o ao registrador. Essa forma, a troca de contexto entre processos e feita através da leitura e da escrita dos dados contidos na struct regs. 
        */	
	void executar_prox_thread(struct REGS *r);

	/**
	*Cria um novo processo, chamando o método criar da classe Processo. Ele recebe por parâmetro o nome do processo, a Imagem do arquivo executável que será carregado, o tipo do processo (DRIVER, USUARIO ou IDLE), a quantidade de parâmetros a ser passado para a função main (argc), e os parâmetros propriamente ditos (* args[]).
	*/
	int adicionar_processo(unsigned char * nome, unsigned char * imagem, int tipo, int argc, char * args[]);

	/**
	*Adiciona uma nova thread a um processo já existente. O processo é especificado pelo parâmetro p, e o ponto de entrada (Endereço de memória no qual a thread comeÃ§a a sua execução) é especificado por ponto_entrada.
	*/
	int adicionar_thread(Processo &p, int ponto_entrada);

	/**
	*Retorna o PID do processo em execução.
	*/
	int obter_pid_em_execucao();

	/**
	* Retorna um valor lógico 1 se a thread em execução for um driver de dispositivo e zero caso contrário.
	*/
	int driver_em_execucao();

	/**
	* Cria um novo processo do tipo IDLE com base na imagem do arquivo executável especificada pelo parâmetro imagem.
	*/
	void adicionar_idle(unsigned char * imagem);

	/**
	*Adiciona a thread especificada pelo parâmetro à lista de threads prontas.
	*/
	void adicionar_thread_pronta(Thread p);

	/**
	*Remove o processo das estruturas da classe escalonador, e chama o método destruir da classe processo.
	*/
	void eliminar_processo(int pid);

	/**
	*Remove o processo das estruturas da classe escalonador, e chama o método destruir da classe processo.
	*/
	void eliminar_processo(Processo &p);

	/**
	*Remove a thread das estruturas da classe escalonador. Caso a thread eliminada seja a thread principal do processo, todo o processo é eliminado.
	*/
	void eliminar_thread(Thread &t);

	/**
	*Método chamado pela função de tratamento de ExceÃ§Ãµes para avisar ao escalonador que uma exception ocorreu enquanto um determinado processado estava em execução. Esse método elimina o processo que estava em execução sempre que uma exception ocorre.
	*/
	void notificar_exception(unsigned int, struct REGS *r);

	/**
	*Escreve no arquivo indicado em arq os dados dos processos em execução.
	*/
	void obter_info(unsigned char * arq);

	/**
	*Altera o status da thread indicada pela variável de pid de esperando para pronto. E adiciona a thread na lista de threads prontas.
	*/
	void acordar_thread(int pid);

	/**
	*Altera o status da thread atualmente em execução no processador em estado de espera.
	*/
	void colocar_thread_atual_em_espera();

	/**
	*Adiciona a mensagem m a lista de mensagens da thread indicada por pid. Caso a thread esteja em estado de espera, ele é acordada. Retorna 1 caso a mensagem tenha sido adicionada com sucesso, e caso o contrário retorna 0.
	*/
	int adicionar_mensagem(int pid, Mensagem m);

	/**
	*Retorna a uma mensagem da lista de mensagens de uma thread indicada pela variável pid. Caso nÃ£o haja restrição de portas (o valor da variável porta sendo igual a constante MSG_QLQR_PORTA), a primeira mensagem disponÃ­vel é retornada. Caso haja, a variável pid_responsavel indica o pid do remetente desejada para a mensagem. Caso nÃ£o haja mensagem disponÃ­vel, retorna uma referência nula. 
	*/
	Mensagem& obter_mensagem(int pid, int porta, int pid_responsavel);

	/**
	*Retorna a instância da classe Processo com o PID especificado pela variável PID. 
	*/
	Processo& obter_processo(unsigned int);

	/**
	*Retorna a instância da classe Thread com o PID especificado pela variável PID.
	*/
	Thread& obter_thread(unsigned int);
	
};
 

/**
*variável global do módulo escalonador.
*/
extern volatile Escalonador escalonador;



#endif


