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
#ifndef _GERENCIADOR
#define _GERENCIADOR
#define POS_INICIAL_KERNEL 0xC0000000
#define POS_INICIAL_KERNEL_HEAP 0xD0000000

#include "Multiboot.h"
#include "Util.h"

/**
*Esta classe é responsável por manter o controle dos frames de memória física.
*/
class MemoriaFisica : public Recurso
{ 
  unsigned char * memoria; 				/*!<Indica o começo do mapa de bits utilizado para o controle da memória física. Cada bit representa um bloco de 4kb de memória. De forma que a posição do bit dentro da cadeia indica a posição de memória à qual ele está relacionado.  */
  unsigned int pos_x_ult_bloco;				/*!<Valor de x no dentro do bitmap do último bloco retornado.*/
  unsigned int pos_y_ult_bloco;				/*!<Valor de y no dentro do bitmap do último bloco retornado.*/		
  unsigned int tamanho;					/*!<Armazena o tamanho (em bytes) da memória física disponível. */
  unsigned int inicio;					/*!<Armazena o endereço da primeira posição de memória do primeiro frame de memória. Os frames são todos alinhados em 4kb. */
  unsigned int tamanho_bitmap;				/*!<Armazena o tamanho (em bytes) ocupado pelo bitmap utilizado no controle dos blocos de memória física. */
  unsigned int blocos_livres;				/*!<Armazena a quantidade de blocos de memória que ainda podem ser alocados. */
  
  public :

  /**
  *Inicializa o controle da memória física. Calcula, se baseando nas informações fornecidas pelo GRUB, a quantidade de memória física disponível. Calcula o tamanho e a posição inicial do bitmap para controle.
  */	
  void inicializar(MULTIBOOT_INFO*, int); 

  /**
  *Aloca uma quantidade de blocos de memória consecutivos indicado pelo variável tam. E retorna o endereço de memória do começo do primeiro bloco. Os blocos alocados estão sempre alinhados em 4kb. 
  */
  void * alocar(unsigned int);

  /**
  *Desaloca uma quantidade blocos de memória física adjacentes indicado pela variável num_blocos que comecem a partir do endereço especificado por end.
  */
  void desalocar(void *, unsigned int);

  /**
  *Armazena no arquivo especificado por arq, a quantidade memória disponível, e a quantidade de memória sendo utilizada.
  */	
  void obter_info(char * arq);

  /**
  *Retorna o número total de blocos livres na memória.
  */	
  int obter_blocos_livres();	
  
};

/**
*Variável global do módulo de gerência de memória virtual.
*/
extern volatile MemoriaFisica mem_fisica;

/**
*Esta classe é responsável inicializar a paginação, e mapear endereços físicos em endereços virtuais e controlar o heap do Kernel.
*/
class MemoriaVirtual : public Recurso
{
   unsigned int memoria_a_alocar; 		/*!<Utilizado para o controle de quanta memória foi alocada do último frame de memória do heap.*/
   unsigned int  * pdir;			/*!<Endereço virtual da page directory inicial do Kernel.*/
   unsigned char * ult_end_ret;			/*!<Variável usada para armazenar o endereço do último bloco de memória virtual alocado no topo do heap do Kernel.*/
   unsigned char * topo_kernel_heap;		/*!<Variável que armazena o próximo endereço a ser retornando para alocação no topo do Kernel.*/
 
   public:	
   
   unsigned int  * pt_aux;			/*!<Bloco de memória virtual auxiliar utilizado durante a criação de novos espaços de endereçamento..*/
  
   /**
   *Inicializa o controle da memória virtual. Ativa a paginação e mapeia os primeiros 4 mb de memória virtual nos primeiros 4 mb de memória física, e também nos primeiros 4 mb de memória a partir dos 3 GB (higher half).
   */
   void inicializar();

   /**
   *Com a última page table apontando para a própria page directory é possível que acessar cada page table através de um endereço virtual. Esse método retorna o endereço de memória virtual da page table cujo índice é especificado pelo parâmetro pdindex.
   */	
   void *obter_endereco_page_table(int pdindex);
   
   /**
   *Método que recebe como parâmetro um endereço virtual e retorna seu endereço físico equivalente caso este esteja mapeado.
   */
   void *obter_end_fisico(unsigned int);
   
   /**
   *Método responsável por finalizar a instalação da paginação após a inicialização da GDT definitiva. Este método aloca memória para todas as page tables na parte superior aos 3 GB de memória, de forma que o heap do Kernel possa crescer sem a necessidade de alocar memória para mais page tables. Como cada processo tem seu espaço de endereçamento e o Kernel e o heap do Kernel devem estar mapeados da mesma forma em todos eles, toda nova page table alocada deveria ser configurada em todos os espaços de endereçamento existentes. Com as page tables previamente alocadas, não há necessidade de alterar outros espaços de endereçamento, basta apenas que todos compartilhem a mesma parte superior dos espaço de endereçamento.
   */
   void remover_entrada_inicial();
   
   /**
   *Mapeia o endereço físico indicado pela variável end_fisico (que deve estar alinhado em 4kb) no endereço virtual especificado por end_virtual no espaço de endereçamentos indicado pela variável pdir. A variável flag indica o nível de prioridade que o código em execucação precisa ter para acessar esse novo bloco de memória. Esse método possui uma sobrecarga na qual o última parâmetro não deve ser especificado. Nesse caso o mapeamento é feito no espaço de endereçamento inicial utilizado durante o boot do Kernel.
   */
   void *mapear_page_table_espaco_end(void * end_fisico, void * end_virtual, unsigned int flags, unsigned int * pdir);
   
   /**
   *
   */
   void mapear_page_table(void * end_fisico, void * end_virtual, unsigned int flags);		
   
   /**
   *Recarrega o table lookaside buffer recarregando o endereço da page directory.
   */
   void recarregar_tlb();
   
   /**
   *Altera o espaço de endereçamento atual configurando o registrador da CPU com o endereço da page directory especificado pelo parâmetro pdir.
   */
   void carregar_page_directory(unsigned int * pdir);

   /**
   *Aloca uma quantidade de bytes igual ao parâmetro tam no topo do Kernel e retorna o endereço.
   */
   void *sbk(unsigned int tam);

   /**
   *Obtem o endereço da page directory do espaço de endereçamento atual.
   */
   int * obter_pdir();
};

/**
*Variável global do módulo de gerência de memória virtual.
*/
extern volatile MemoriaVirtual mem_virtual;

/**
*Função kmalloc aloca tam bytes a partir do heap do kernel. Utilizada para alocação dinâmica de memória dentro do núcleo.
*/
void * kmalloc(unsigned int tam);

/**
*Libera um bloco de memória do heap do kernel alocado com a função kmalloc.
*/
void free(void *ap);

#endif
