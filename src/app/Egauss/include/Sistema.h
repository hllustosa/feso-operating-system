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
#ifndef _SISTEMA
#define _SISTEMA

#define NULL 0

#define INFO_PROCESSOS 	1
#define INFO_MENSAGENS 	2
#define INFO_MEMORIA 	3
#define INFO_VFS 		4

//constantes sistema de troca de mensagems
const int MSG_COMUM = -1;
const int MSG_IRQ = -2;
const int MSG_EVT = -3;
const int MSG_QLQR_PORTA = -1;
const int NAO_HA_MSGS = 0;
const int MSG_SUCESSO = 1;
const int MSG_ERR_PROC_NAO_EXISTE = 5;
const int MSG_ERR_PORTA_NAO_ATIVA = 6;
const int MSG_ERR_PORTA_OCUPADA = 7;

//porta para mensagens do kernel
const int KERNEL_PORTA_PROC_CRIADO = 20;
const int KERNEL_PORTA_PROC_DESTRUIDO = 21;
const int KERNEL_PORTA_ALOCADA = 22;
const int KERNEL_PORTA_DESALOCADA = 23;
const int KERNEL_PORTA_VFS = 24;
const int KERNEL_PORTA_EXCEPTION = 25;

//constantes do sistem de arquivos
const int VFS_SUCESSO = 1;
const int VFS_ARQUIVO_EXTERNO = -2;
const int VFS_EOF = -2;
const int VFS_POSICAO_INVALIDA = -5;
const int VFS_ARQUIVO_FECHADO = -6;
const int VFS_ARQUIVO_ABERTO = -7;
const int VFS_DIR_NAO_ENCONTRADO = -8;
const int VFS_SEM_PERMISSAO = -9;
const int VFS_ERR_TAM_MAX_DIR = -10;
const int VFS_ERR_ARQ_NAO_ENCONTRADO = -11;
const int VFS_NOME_INVALIDO = -12;
const int VFS_TENTATIVA_ESCREVER_PASTA = -13;

const int VFS_BLOCO_ABRIR    = 1;
const int VFS_BLOCO_LER      = 2;
const int VFS_BLOCO_ESCREVER = 3;
const int VFS_BLOCO_EXCLUIR  = 4;
const int VFS_BLOCO_FECHAR   = 5;

const int VFS_ARQ = 0;
const int VFS_DIR = 1;
const int VFS_MNT = 2;
const int VFS_MNT_DIR = 3;
const int VFS_MNT_ABERTO = 4;

//constantes da gerência de processos
const int PROC_SUCESSO  = 1;
const int PROC_FORMATO_INVALIDO = 2;
const int PROC_SEM_MEMORIA = 3;

//struct de eventos do sistema operacional
struct Evento
{
	char num;
	int param1;
	int param2;
	char dados[80];
};

struct Arq_Info
{
	unsigned char nome[15];
	unsigned int offset;
	unsigned int tamanho;
	unsigned int tamanho_em_disco;
	unsigned int pai;
	unsigned int tipo;
	unsigned int descritor;
	unsigned int porta_servidor;
	unsigned int flags;
	int aberto_por;
	char enchimento[60];
}__attribute__((packed));

struct Bloco_Vfs
{
	unsigned int cod;
	unsigned int pid;
	unsigned int descritor;
	unsigned char param1;
	unsigned char param2;
	unsigned char nome[80];
	
}__attribute__((packed));

//para comunicação com servidor do shell
struct Bloco_Shell
{
   int pid;
   char arquivo[80];   
};

struct Bloco_Resposta
{
   int retorno;
   char msg[80];   
};


//informações do kernel
extern "C" void obter_info_kernel(unsigned char * arquivo, int codigo);

//executar operação do sistema
void sistema(char * comando, Bloco_Resposta * br);

//gerência de memória
extern "C" int  sbk(unsigned int tam);
void *kmalloc(unsigned nbytes);
void free(void *ap);

//sistema de troca de mensagens
extern "C" int enviar_msg(unsigned int porta, unsigned char * mensagem);
extern "C" int enviar_msg_pid(unsigned int pid, unsigned char * mensagem);
extern "C" int enviar_receber_msg(unsigned int porta, unsigned char * mensagem);
extern "C" int escutar_porta(unsigned int porta);
extern "C" int receber_msg(unsigned char mensagem[100], int porta);

//sistema de arquivos
extern "C" int _abrir(unsigned char * caminho, char modo, unsigned int * servidor);
extern "C" int _ler(unsigned int descritor, unsigned char * buffer ,unsigned int tam);
extern "C" int _escrever(unsigned int descritor, unsigned char * buffer ,unsigned int tam);
extern "C" int _buscar(unsigned int descritor, unsigned int pos);
extern "C" int _excluir(unsigned int descritor);
extern "C" int _fechar(unsigned int descritor);
extern "C" int _obter_info_arq(unsigned descritor, unsigned char * dados);
extern "C" int _adicionar_no(unsigned char * nome, unsigned int tamanho, unsigned int * novo_descritor, int porta_servidor);
extern "C" int _montar_no(unsigned int novo_descritor);
extern "C" int _remover_no(unsigned int novo_descritor);

//wrappers em c++
int abrir(unsigned char * caminho, char modo);
int ler(unsigned int descritor, unsigned char * buffer ,unsigned int tam);
int escrever(unsigned int descritor, unsigned char * buffer ,unsigned int tam);
int buscar(unsigned int descritor, unsigned int pos);
int excluir(unsigned int descritor);
int fechar(unsigned int descritor);
int obter_info_arq(unsigned descritor, unsigned char * dados);
int adicionar_no(unsigned char * nome, unsigned int tamanho, unsigned int * novo_descritor, int porta_servidor);
int montar_no(unsigned int novo_descritor);
int remover_no(unsigned int novo_descritor);

//processos
extern "C" int _executar(unsigned char * nome, unsigned int argc, unsigned char * args, int descritor);
extern "C" int _obter_pid(int pid_thread);
extern "C" void sair(void);
extern "C" void finalizar_proceso(unsigned int pid);
extern "C" int criar_thread(unsigned int entrada);
int executar(unsigned char * caminho, unsigned int argc, unsigned char * args);
int obter_pid(void);
int obter_pid(int pid_thread);

#endif 