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
#ifndef _VFS
#define _VFS

#include "Avl.h"
#include "Lista.h"


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


#define EOF -1
#define MAX_TAM_ARQ 15
#define TAM_BLOCO 512

/**
* Estrutura que representa um bloco de um arquivo.
*/
struct Bloco
{
	unsigned char dados[TAM_BLOCO]; /*!<Buffer contendo os dados do bloco. */
};

/**
* Estrutura que representa um diretório no VFS.
*/
class Diretorio
{
	static int const MAX_ARQ = 127; 	/*!<Constantes limitando o número máximo de arquivos em um diretório. */
	
	public:
	
	int qtd_arquivos;			/*!<Quantidade de arquivos no diretório. */
	unsigned int arqs[127];			/*!<Vetor com os descritores dos arquivos. */

	/**
	* Adicionar um arquivo no diretórios.
	*/
    	int adicionar_arquivo(unsigned int);	

	/**
	* Remove arquivo do diretório.
	*/
	int remover_arquivo(unsigned int);
	
} __attribute__((packed));

/**
* Estrutura que representa um arquivo no VFS.
*/
class Arquivo : public Recurso
{
	public :
	
	unsigned char nome[MAX_TAM_ARQ];	/*!<Nome completo do arquivo. */
	unsigned int offset;			/*!<Posição em que o cursor se encontra dentro do arquivo. */
	unsigned int tamanho;			/*!<Tamanho do arquivo em bytes. */
	unsigned int tamanho_em_disco;		/*!<Tamanho do arquivo em bytes no  sistema de arquivos. */
	unsigned int pai;			/*!<Descritor do diretório no qual o arquivo se encontra. */
	unsigned int tipo;			/*!<Tipo do arquivo. */
	unsigned int descritor;			/*!<Número inteiro que serve como identificador do arquivo. */
	unsigned int porta_servidor;		/*!<Porta do servidor responsável por atualizar o arquivo. */
	unsigned int flags;			/*!<Flags com atributos do arquivo. */
	int aberto_por;				/*!<PID do processo que tem o arquivo aberto atualmente. */
	Lista<Bloco> clusters;			/*!<Lista de clusters em memória com os dados arquivo. */
	
	Arquivo()
	{
		id_dono          =-1;
		mutex 	         =0;
		tamanho 	 =0;
		tamanho_em_disco =0;
	}
};


/**
* Estrutura utilizada para retornar os dados de um arquivo.
*/
struct ArqInfo
{
	unsigned char nome[MAX_TAM_ARQ];
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


/**
* Estrutura que representa um bloco no VFS.
*/
struct Bloco_Vfs
{
	unsigned int cod;
	unsigned int pid;
	unsigned int descritor;
	
}__attribute__((packed));


/**
* Estrutura que representa um bloco no VFS.
*/
class Vfs : public Recurso
{
	static const int ARQ = 0;		/*!<Constante indicando que o nó do VFS é um arquivo. */
	static const int DIR = 1;		/*!<Constante indicando que o nó do VFS é um diretório.. */
	static const int MNT = 2;		/*!<Constante indicando que o nó do VFS é um arquivo externo. */
	static const int MNT_DIR = 3;		/*!<Constante indicando que o nó do VFS é um diretório externo.. */
	static const int MNT_ABERTO = 4;	/*!<Constante indicando que o nó do VFS é um arquivo externo aberto. */

	Arquivo raiz;				/*!<Descritor do diretório raiz. */
	Arvore<Arquivo> arquivos;		/*!<Estrutura de dados que armazena as informações de todos os arquivos.. */
	unsigned int prox_descritor;		/*!<O descritor é o número sequencial atribuído aos arquivos no momento de sua criação. Sempre que um novo arquivo é criado, ele recebe o valor dessa variável como descritor. Depois ela é incrementada em 1. */
	Bloco novo_bloco;			
	
	unsigned int criar_arquivo(Arquivo,unsigned char *, char, unsigned int *);
	unsigned int _abrir(unsigned char *, char, unsigned int *, unsigned int *);
	unsigned int validar_nome(unsigned char *);
	
	public :
	
	/**
	* Método que inicializa os VFS e cria algumas pastas padrão no sistema, tais como: dev, usr, temp.
	*/
	void inicializar();
	
	/**
	* Recebe um descritor por parâmetro e retorna uma Arquivo com as informações do arquivo que possuí aquele descritor.
	*/
	Arquivo& obter_arquivo(unsigned int);

	/**
	* Recebe um descritor de um arquivo e retorna um ponteiro para uma estrutura da memória onde o arquivo está armazenado de forma continua (não mais em clusters). Utilizado pela gerência de processos para carregar arquivos executáveis.
	*/
	unsigned char * obter_imagem(unsigned int);

	/**
	* Recebe uma string com o caminho do arquivo, o modo de abertura, e coloca na variável descritor o número do descritor do arquivo.
	*
	*Os modos válidos são
	*	‘a’ – Cria o arquivo caso ele não exista. Se existir, todo seu conteúdo é apagado.
	*	‘A’ – Cria o arquivo caso ele não exista. Se existir é aberto para adição de dados e leitura.
	*	‘b’ – Verifica se o arquivo existe. Se não existir ele não é criado e a função retorna a constante: VFS_ERR_ARQ_NAO_ENCONTRADO;
	*	‘D’ – Abre um diretório para leitura. Caso o diretório não exista, ele é criado.
	*	‘d’ – Abre um diretório para leitura. Caso o diretório não exista, retorna a cosntante : VFS_DIR_NAO_ENCONTRADO;
	*/
	unsigned int abrir(unsigned char *, char, unsigned int *);

	/**
	* Sobrecarga do método anterior que recebe o ponteiro para armazenar o pid do servidor que é responsável por manter o arquivo. Em caso de arquivos externos ao VFS.
	*/
	unsigned int abrir(unsigned char *, char, unsigned int *, unsigned int *);

	/**
	* Escreve no arquivo identificado pelo descriptor, a quantidade de dados especificada pela variável tamanho, contido na variável dados.
	*/
	unsigned int escrever(unsigned int, unsigned char *, unsigned int);

	/**
	* Copia do arquivo identificado pelo descriptor, uma quantidade de bytes igual a especificada na variável tamanho, na posição de memória especificada na variável dados.
	*/
	unsigned int ler(unsigned int, unsigned char *, unsigned int);

	/**
	* Cada arquivo possuí em sua estrutura uma variável que indica em que posição do arquivo as leituras e escritas devem ser realizadas. Sempre que uma leitura e escrita ocorre, esse valor é automaticamente autlizado. Esse método altera o valor dessa posição no arquivo especificado pela variável descritor. 
	*/
	unsigned int buscar(unsigned int, unsigned int);

	/**
	* Remove o arquivo especificado por descriptor do sistema de arquivos.
	*/
	unsigned int excluir(unsigned int);

	/**
	* Fecha o arquivo especificado pelo descritor. Coloca a posição para leitura e escrita em zero. E zera o pid referente ao processo que tem direito de leitura e escrita no arquivo. 
	*/
	unsigned int fechar(unsigned int);

	/**
	* Cria um novo nó externo no sistema de arquivos. Os nós externos são utilizados por sistemas de arquivos externos para que toda vez que um arquivo seja aberto, fechado, excluído, os servidores responsáveis possam tomar as medidas necessárias.
	*/
	unsigned int adicionar_no(unsigned char *, int, unsigned int, unsigned int *);

	/**
	* Altera o tipo do arquivo para um nó montando, permitindo que outros processos leiam e escrevam nesse arquivo.
	*/
	unsigned int montar_no(unsigned int);

	/**
	* Remove o nó do sistema de arquivos.
	*/
	unsigned int remover_no(unsigned int);

	/**
	* Armazena na posição de memória especificada na variável dados, as informações contidas nas estrutura do arquivo especificado na variável descritor.
	*/
	unsigned int obter_info_arquivo(unsigned, unsigned char *);

};


extern volatile Vfs vfs;





#endif
