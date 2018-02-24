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
#ifndef _MENSAGEM
#define _MENSAGEM

#include "Avl.h"
#include "Irq.h"
#include "Lista.h"

#define PROC_CRIADO 0
#define PROC_DESTRUIDO 1

#define KERNEL_MSG 0
#define KERNEL_PORTA_LIM_INF 20
#define KERNEL_PORTA_LIM_SUP 40

#define KERNEL_PORTA_PROC_CRIADO 20
#define KERNEL_PORTA_PROC_DESTRUIDO 21
#define KERNEL_PORTA_ALOCADA 22
#define KERNEL_PORTA_DESALOCADA 23
#define KERNEL_PORTA_VFS 24
#define KERNEL_PORTA_EXCEPTION 25
	

/**
* Constantes para os tipos de mensagens.
*/
const int MSG_COMUM = -1;
const int MSG_IRQ = -2;
const int MSG_EVT = -3;
const int MSG_QLQR_PORTA = -1;
const int NAO_HA_MSGS = 0;
const int MSG_SUCESSO = 1;
const int MSG_ERR_PROC_NAO_EXISTE = 5;
const int MSG_ERR_PORTA_NAO_ATIVA = 6;
const int MSG_ERR_PORTA_OCUPADA = 7;

#define TAM_MAX_MSG 100
#define TAM_BUFFER_EVENTO 80

/**
* Struct que representa eventos do sistema operacional.
*/
struct Evento
{
	char num; 			/*!<Número do evento. */
	int param1; 		        /*!<Parâmetro 1 do evento. */
	int param2;			/*!<Parâmetro 2 do evento. */
	char dados[TAM_BUFFER_EVENTO];	/*!<Buffer com os dados. */
};

/**
* Struct que representa uma porta para a troca de mensagens.
*/
struct Porta
{
	unsigned int numero;		/*!<Número da porta. */
	unsigned int pid_responsavel;	/*!<Pid do processo servidor que alocou a porta. */
};

/**
* Struct que representa uma mensagem .
*/
struct Mensagem
{
	int sinalizar_recebimento;		/*!<Valor lógico para indicar se o recebimento da mensagem deve ser sinalizado ao emissor. */
	int tipo;				/*!<Tipo da mensagem dada pelas constantes no começo do arquivo. */
	unsigned int pid_remetente;		/*!<PID do processo de origem.. */
	unsigned int porta_destino;		/*!<Número da porta de destino. */
	unsigned char conteudo[TAM_MAX_MSG];	/*!<Buffer com conteúdo da mensagem. */
};


/**
* Esta classe é responsável pelo sistema de troca de mensagens. Através dela ocorrem os envios e recebimentos de mensagens entre threads e processos.
* É ela a responsável por notificar os processos listeners dos eventos do Kernel. E também por informar aos processos que interrupções ocorreram.
*/
class Entregador : public Recurso
{
	Lista<int> kernel_listerners;				/*!<Lista de processos listeners de todos os eventos do kernel. */
	Lista<int> kernel_listerners_evt_proc_criado;		/*!<Lista de processos listeners do evento de criação de processos. */
	Lista<int> kernel_listerners_evt_proc_destruido;	/*!<Lista de processos listeners do evento de remoção de processos. */
	Lista<int> kernel_listerners_evt_porta_alocada;		/*!<Lista de processos listeners do evento de porta alocada. */
	Lista<int> kernel_listerners_evt_porta_desalocada;	/*!<Lista de processos listeners do evento de porta desalocada. */
	Lista<int> kernel_listerners_evt_vfs;			/*!<Lista de processos listeners de eventos relativos ao VFS. */
	Lista<int> kernel_listerners_exception;			/*!<Lista de processos listeners de evetos relativos à exceções. */
	Arvore<Porta> portas;					/*!<Estrutura contendo dados sobre as portas alocadas. */
	Lista<int> portas_alocadas;				/*!<Lista de portas alocadas atualmente. */

	public:
	
	/**
	* Inicializa o sistema de troca de mensagens e configura a classe IRQ para que o método para notificar IRQs seja executado sempre que uma IRQ ocorrer.
	* @param irq é a IRQ a ser configurada.
	*/ 
	void inicializar(Irq irq);


	/**
	*Método executado sempre que uma IRQ ocorre.  As portas com números entre 0 e 15 são reservadas para IRQs, de forma que o processo (obrigatoriamente um servidor) que alocou portas dentro 		*dessa faixa recebem uma mensagem sempre que a IRQ com o número da porta alocada ocorre. Esse método verifica se existe uma thread que tenha alocado a porta e envia uma mensagem para essa 		*thread.
	*/
	void notificar_irq(struct REGS *r, int irq_num);

	/**
	*Método executado sempre que algum evento ocorre do Kernel ocorre.  O método consulta as listas de listeners e envia uma mensagem para cada processo contido na lista de listerners referente ao 	 *evento ocorrido.
	*/
	void notificar_evento(char * dados, int evt_num);
	
	/**
	*Obtém os dados da porta com base em seu número.
	*/
	Porta& obter_porta(int num);

	/**
	*Aloca a porta cujo número é indicado pelo parâmetro porta para thread indicada pelo parâmetro PID. Apenas processos servidores podem alocar portas abaixo do número 1000.
	*/
	int adicionar_listerner(unsigned pid, unsigned porta);

	/**
	*Desaloca a porta cujo número é especificado pelo parâmetro porta.
	*/
	void remover_listerner(unsigned porta);

	/**
	*Envia uma mensagem com o conteúdo da cadeia indicada pelo parâmetro msg da thread especificada pelo parâmetro pid_origem, para a thread especificada pelo parâmetro pid_destino. O envio de 		*mensagens diretamente para um PID é não bloqueante, de forma que o processo que envia a mensagem não tem seu estado alterado para em espera. Já a thread de destino tem seu estado alterado 		*para pronto, caso ela esteja em estado de espera. O parâmetro indica se a mensagem é uma IRQ, evento do Kernel, ou mensagem convencional.
	*/
	int tratar_envio_pid(unsigned pid_origem, unsigned pid_destino, int tipo, unsigned char msg[TAM_MAX_MSG]);

	/**
	*Envia uma mensagem contida no parâmetro msg, da thread especificada pelo parâmetro pid, para a porta cujo número é indicado pelo parâmetro porta. O método encaminha a mensagem para a thread 		*responsável pela porta de destino. A thread remetente da mensagem é bloqueada, e entra em estado de espera, e uma nova thread é executada. A thread de destino é acordada tendo seu estado 		*alterado para pronto. A variável sinalizar indica se a thread remetente deve ser acordada no momento em que thread de destino executada uma chamada "receber" ao sistema operacional.
	*/
	int tratar_envio(struct REGS *r, unsigned pid, unsigned porta, int sinalizar, unsigned char msg[TAM_MAX_MSG]);

	/**
	*Verifica se a thread especificada pelo parâmetro pid possui em sua lista de mensagens alguma mensagem provinda da porta indicada pelo parâmetro porta. Caso haja, o conteúdo da mensagem é 		*copiada para posição de memória especificado pelo parâmetro mensagem. Caso não existam mensagens, o estado da thread é alterado para em espera.
	*/
	int tratar_recebimento(struct REGS *r, unsigned pid, unsigned char * msg, int porta);

	/**
	*Armazena no arquivo especificado pelo parâmetro arq, uma lista contendo as portas alocadas, e suas respectivas threads.
	*/
	void obter_info(char * arq);	
};

/**
* Variável global para o módulo entregador.
*/
extern volatile Entregador entregador;

#endif
