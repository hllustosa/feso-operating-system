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
#include "include/Memoria.h"
#include "include/Vfs.h"
#include "include/Util.h"
 
#define TAM_BLOC32 32768
#define LIM_1MB 1048576
#define NALLOC 1024

//Inst�ncia das classes de mem�ria f�sica e virtual
volatile MemoriaFisica mem_fisica;
volatile MemoriaVirtual mem_virtual;

//========================================================================================================================
//Mem�ria f�sica
 /*
    mapa da mem�ria f�sica
	 ___________________
	|					|
	|					| M�moria livre para o usu�rio
	|___________________|
	|___________________| bitmap com estados da memoria
    |___________________| byte final do kernel
    |                   |
    |______kernel_______|
 */
  
//Tamanho de uma p�gina de mem�ria
const unsigned int TAM_BLOC = 4096;
 
//recebe o endere�o onde o kernel termina e o tamanho total da mem�ria em bytes
void MemoriaFisica::inicializar(MULTIBOOT_INFO* mbd, int end_fim_kernel)
{
   unsigned long  * base;
   unsigned long  * comprimento;
   unsigned int end_final_modulos = 0;
   pos_x_ult_bloco = 0; 
   pos_y_ult_bloco = 0;
   mutex = 0;
   id_dono = -1;
   rec_num = 4;
   
   /*apenas os 4 primeiros mb de mem�ria est�o mapeados quando esse c�digo � executado 
   dessa forma, caso o GRUB coloque a estrutura do mapa de mem�ria em um endere�o 
   ap�s os 4 primeiros mb. O  c�digo abaixo n�o funcionar� */
   mbd = (MULTIBOOT_INFO*)((unsigned int)mbd + POS_INICIAL_KERNEL);
   multiboot_memory_map_t* mmap = (multiboot_memory_map_t*) ((unsigned int)mbd->mmap_addr + POS_INICIAL_KERNEL);
   
   //varrendo dados obtidos pelo GRUB	
   while(mmap < (multiboot_memory_map_t *)(  ((unsigned int)mbd->mmap_addr + POS_INICIAL_KERNEL) + ((unsigned int)mbd->mmap_length) ))
   {  
		base        = (unsigned long *) ((unsigned long )&mmap->base_addr_low );
		comprimento = (unsigned long *) ((unsigned long )&mmap->length_low  );
		
		//Buscando primeiro bloco de mem�ria livre ap�s o 1MB de mem�ria
		if ((mmap->type == 1)  && (*base >= LIM_1MB)) 
		{
		  break;
		}

		mmap = (multiboot_memory_map_t*) ( (unsigned int)mmap + mmap->size + sizeof(unsigned int));
   }
  
   //obtendo o espa�o total ocupado pelos m�dulos do GRUB
   MULTIBOOT_MOD_LIST * modulos = (unsigned int)mbd->mods_addr + POS_INICIAL_KERNEL;
   
   //obtendo o endere�o final ocupado pelo �ltimo m�dulo
   if(mbd->mods_count > 0)
   {
	 end_final_modulos = modulos[ mbd->mods_count -1].mod_end;
   }
	  
   /*Calculando o primeiro endere�o dispon�vel de forma que ele seja um
   Page boundary valido para as Page Tables (ou seja, possua os 12 bits finais iguais a 0)
   Esse � um pre-requisito para implementar a pagina��o na arquitetura x86*/
   inicio = (end_fim_kernel > end_final_modulos ? end_fim_kernel : end_final_modulos) + POS_INICIAL_KERNEL; 
   inicio = (inicio+TAM_BLOC);
   inicio = inicio + TAM_BLOC -(inicio%TAM_BLOC);
  
   tamanho = (*comprimento - end_fim_kernel);	 
   
   //posicionando a memoria ap�s o kernel
   memoria = (unsigned char *) (inicio) + TAM_BLOC;
  
   //ser� necess�rio utilizar 1 bit para indicar o estado de 4 kb de mem�ria
   long num_bits_necessario = blocos_livres = tamanho/TAM_BLOC;
  
   //o tamanho bitmap esta em bytes (numero de bits necess�rio dividido por 8)
   tamanho_bitmap = num_bits_necessario/8;
  
   //zerando todos os endere�os
   memset(memoria, 0 ,tamanho_bitmap);
  
   //o bitmap ocupara em unidades de 4 kb; 
   if(tamanho_bitmap/TAM_BLOC <= 0)
     alocar(1);
   else	
     alocar(tamanho_bitmap/TAM_BLOC);	 
}

//Alocar mem�ria em blocos de 4kb
void * MemoriaFisica::alocar(unsigned int tam)
{	
   //declarando endere�o que ser� retornado
   void * endereco = (void *)0;
     
   //acumular a quantidade de endere�os livres consecutivos
   int qtd_livres_seguidos = 0;
   int pos_x = 0;
   int pos_y = 0;
   unsigned int valor;	 
	 
   //verificando se existe mem�ria suficiente	 
   if(blocos_livres>= tam)
   {  
	   blocos_livres -= tam;
	  
	   //procurar por bloco com tamanho requisitado
	   for(int i =0; i < tamanho_bitmap; i++)
	   {
		   //obtendo o valor do byte na posi��o i
		   valor = (unsigned int) memoria[i];
		  	  
		   //varrendo cada bit do byte	  
		   for(int j =0; j < 8; j++)
		   {
			  
			  //caso o estado do bit seja 0, o frame est� livre
			  if(obter_estado_bit(valor,j) == 0)
			  {
					if(qtd_livres_seguidos == 0)
					{
						pos_x = i;
						pos_y = j;
					}
					
					qtd_livres_seguidos++;
			  }
			  else
			  {
					qtd_livres_seguidos = 0;
			  }
			  
             //para quando encontra frames livres suficientes
			 if(qtd_livres_seguidos >= tam) break;
			 
		   }
		  
		  //para quando encontra frames livres suficientes
		  if(qtd_livres_seguidos >= tam) break;
	   }
	   
	   
	   if(qtd_livres_seguidos >= tam)
	   {
		   //alocando blocos
		   unsigned int num_blocos_alocados = 0;
		   unsigned int pos = pos_x*8 + pos_y;
		   
		   while(num_blocos_alocados < tam)
		   {
				//marcando blocos como alocados
				setar_byte(&memoria[pos/8], pos%8,1); 
				pos++;
				num_blocos_alocados++;
		   }
		   
		   //calculando o endere�o do bloco
		   endereco = (void *) (inicio + pos_x*TAM_BLOC32 + pos_y*TAM_BLOC) - POS_INICIAL_KERNEL;  
	   }
  }
    
  return endereco;
}

//Desaloca mem�ria em bloco de 4kbs
void MemoriaFisica::desalocar(void * end, unsigned int num_blocos)
{
	//verificando se o endere�o � diferente de 0 e � m�ltiplo de 4kb
    if( (end != 0) &&  ( ((unsigned int)end % 4096) == 0))
	{
	    //Calculando posi��o x e y no bitmap em fun��o do endere�o 
		unsigned int aux   = ((unsigned int)end) - inicio + POS_INICIAL_KERNEL;
		unsigned int pos_y = (aux%TAM_BLOC32)/TAM_BLOC;
		unsigned int pos_x = (aux - pos_y*TAM_BLOC)/TAM_BLOC32;
		unsigned int pos   = pos_x*8 + pos_y;   
		
		//varrendo a partir de pos e marcando endere�os como livres
		for(unsigned int i = 0; i < num_blocos; i++)
		{
			//colocando 0 na posi��o respectiva do bitmap
			setar_byte(&memoria[pos/8], pos%8,0); 
			pos++;
			blocos_livres++;
		}	   
	}
	
}


//Obtendo informa��es sobre a mem�ria (Tamanho total, e espa�o livre)
void MemoriaFisica::obter_info(char * arq)
{
    int descritor;
	char linha[150];

	vfs.abrir(arq, 'a', &descritor);
						
	strcat(linha, itoa(tamanho, 10));
	strcat(linha," ");
	strcat(linha, itoa(blocos_livres * TAM_BLOC, 10));
	strcat(linha, "\n");
	
	vfs.escrever(descritor, linha, strlen(linha));	
	
	vfs.fechar(descritor);
}

int MemoriaFisica::obter_blocos_livres()
{
	return blocos_livres;
}

//========================================================================================================================
/*Mem�ria virtual
A page table e a Page directory devem estar em endere�os alinhados em 4kb. 
De forma que os 12 bits menos significativos do endere�o sejam 0.
Isso � necess�rio pois, os 12 primeiros bits das entradas da PD e PT servem para armazenar
os flags referentes as PTs e PDs*/

unsigned int page_directory[1024]      __attribute__ ((aligned (4096)));
unsigned int primeira_page_table[1024] __attribute__ ((aligned (4096)));
unsigned int page_table_aux[1024] 	   __attribute__ ((aligned (4096)));

//M�todo de inicializa��o da mem�ria virtual
void MemoriaVirtual::inicializar()
{  
	unsigned int end = 0; 	
	memoria_a_alocar = 0;
	
    void *page_directoryPtr = 0;
	void *primeira_page_tablePtr = 0;
	void *page_aux_Ptr = 0;
    
    pdir   = (unsigned int *)page_directory;
	pt_aux = (unsigned int *)page_table_aux;
	
    // Endere�o f�sico da page directory
	page_directoryPtr =(char *)page_directory + 0x40000000;	        
	
	// Endere�o f�sico da page table
	primeira_page_tablePtr = (char *)primeira_page_table + 0x40000000;	
   // pdirptr = page_directoryPtr;
	
    //Preenchendo a Page Diretory
    for(unsigned int i = 0; i < 1024; i++)
    {
		//atributos: n�vel supervisor, read/write, n�o presente.
		page_directory[i] = (unsigned int )0 | 2; 
		
		//Preenchendo os primeiros 4 MB da mem�ria
		primeira_page_table[i] = end | 3; //atributos: n�vel supervisor, read/write, presente.
		end = end + 4096; //incrementando o endere�o em 4096 (pr�xima p�gina)
    }
  
	page_directory[0]  = (unsigned int)primeira_page_tablePtr; 
	page_directory[0] |= 3;//atributos: n�vel supervisor, read/write, presente.
	  
	//mapeando o 1/4 da parte de cima da Page Directory. Espa�o compartilhado em todos os processos  
	page_directory[768]  = (unsigned int)primeira_page_tablePtr; 
	page_directory[768] |= 3;// //atributos: n�vel supervisor, read/write, presente.

	//Mapeando a Page Directory sobre si pr�pria. Para poder acessar as Page Tables com endere�os virtuais
	page_directory[1023]  = (unsigned int)page_directoryPtr; 
	page_directory[1023] |= 3; //atributos: n�vel supervisor, read/write, presente.

	//habilitando pagina��o. colocando endere�o da Page directory no registrador cr3
	asm volatile("mov %0, %%cr3":: "b"(page_directoryPtr));

	//recarregando o valor do registrador cr0 para ativar a pagina��o
	unsigned int cr0;
	asm volatile("mov %%cr0, %0": "=b"(cr0));
	cr0 |= 0x80000000;
	asm volatile("mov %0, %%cr0":: "b"(cr0)); 
	 
}

//M�todo executado ap�s a remo��o da GDT falsa da mem�ria
void MemoriaVirtual::remover_entrada_inicial()
{  
	//Primeiro bloco de 4mb de mem�ria configurado como n�o-presente
    //page_directory[0] = (unsigned int )0|2;  
   
	//alocando bloco de mem�ria f�sica
    mem_virtual.ult_end_ret = mem_fisica.alocar(1); 	  	
   
    /*Alocando espa�o para as Page Tables referentes a mem�ria do Kernel
	Como essas Page Tables s�o compartilhadas por todos os processos
	� mais f�cil mant�-las fixas, do que alocadas sob demanda */
   	for(int cont = 769; cont < 1023; cont++)
	{
		//alocando um bloco para a nova Page Table
	    unsigned int * nova_page_table = (unsigned int *)mem_fisica.alocar(1);
	    unsigned int nova_page_table_ptr = ((unsigned int)nova_page_table);
	
		//atualizando entrada na Page directory com a nova page table
		page_directory[cont] = (unsigned int) nova_page_table_ptr | 3;
	
        //obtendo o endere�o virtual para acessar a page table  		
		nova_page_table = obter_endereco_page_table(cont);
	     
		//preenchendo nova page table com entradas n�o presentes
		for(unsigned int j = 0; j < 1024; j++)
		{
		     nova_page_table[j] = (unsigned int ) 0 | 2; //atributos: n�vel supervisor, read/write, n�o presente.
			 
		}//fim do for mais interno	
		
	}//fim do for mais externo

	//Mapeando os primeiros 4kb do HEAP do Kernel
	mapear_page_table(mem_virtual.ult_end_ret, POS_INICIAL_KERNEL_HEAP, 0x03);
    mem_virtual.ult_end_ret = mem_virtual.topo_kernel_heap = POS_INICIAL_KERNEL_HEAP;
}

/*A �ltima entrada da Page Directory aponta para a pr�pria Page Directory
De forma que todas as Page Tables podem ser acessadas a partir de um endere�o virtual.
Esse m�todo retorna o endere�o virtual da page table, baseado em �ndice na page directory*/
void * MemoriaVirtual::obter_endereco_page_table(int pdindex)
{
	unsigned int end  = (1023 << 22);
	end += (pdindex << 12);     
	return (void*)end;   
}

//Mapeia do endere�o f�sico (end_fisico) no endere�o virtual (end_virtual) em uma determinada page directory(pdir)
void * MemoriaVirtual::mapear_page_table_espaco_end(void * end_fisico, void * end_virtual, unsigned int flags, unsigned int * pdir)
{
    void * ret = 0;
	
	//Calculando �ndices na page directory e page table para o endere�o virtual
	//Os endere�os devem estar alinhados em 4 kb
    unsigned long pdindex = (unsigned long)end_virtual >> 22;
    unsigned long ptindex = (unsigned long)end_virtual >> 12 & 0x03FF;
 
    //Verificar se a entrada na Page directory est� presente (ultimo bit deve ser 1)
	if( (pdir[pdindex] & 0x01) != 0x01)
	{   
	    //alocando um bloco para a nova Page Table
	    unsigned int * nova_page_table = ret =(unsigned int *)mem_fisica.alocar(1);
	    unsigned int nova_page_table_ptr = ((unsigned int)nova_page_table);
	    
		//atualizando entrada na Page directory com a nova page table
		pdir[pdindex] = (unsigned int) nova_page_table_ptr | flags;
		
        //obtendo o endere�o virtual para acessar a page table  		
		nova_page_table = obter_endereco_page_table(pdindex);
		
		//preenchendo nova page table com entradas n�o presentes
		for(unsigned int i = 0; i < 1024; i++)
		{
		   //atributos: n�vel supervisor, read/write, n�o presente.
		   nova_page_table[i] = (unsigned int ) 0 | 2; 
		   
		}//fim for
	    
	}//fim do if
	  
	//obtendo endere�o da page table referente ao endere�o virtual
	unsigned int * pt =  obter_endereco_page_table(pdindex); //(unsigned int *)( (page_directory[pdindex] & 0xFFFFF000) + POS_INICIAL_KERNEL );
	
	//Atualizando a entrada na page table
	pt[ptindex] = ((unsigned int)end_fisico) | flags; 
		
	/*retorna o endere�o f�sico de uma Page Table caso tenha 
	sido necess�rio alocar uma nova sen�o, retorna 0*/
	return ret;
}

//Mapeia do endere�o f�sico (end_fisico) no endere�o virtual (end_virtual) na Page Directory inicial do Kernel
void MemoriaVirtual::mapear_page_table(void * end_fisico, void * end_virtual, unsigned int flags)
{
	mapear_page_table_espaco_end(end_fisico, end_virtual, flags, (unsigned int *)page_directory);
	recarregar_tlb();
}

//Recarrega o TLB (Table Lookaside Buffer), buffer das estruturas de controle da mem�ria virtal
void MemoriaVirtual::recarregar_tlb()
{
	//Apenas atualiza o valor do registrador CRO
	unsigned int cr0;
	asm volatile("mov %%cr0, %0": "=b"(cr0));
	cr0 |= 0x80000000;
	asm volatile("mov %0, %%cr0":: "b"(cr0)); 
}

//Instala a Page Directory de um novo Processo (usado pela fun��o de troca de contexto)
void MemoriaVirtual::carregar_page_directory(unsigned int * pdir)
{
	int * pdir_atual;

	//obtendo endere�o do page directory atual	
	asm volatile("mov %%cr3, %0": "=b"(pdir_atual));
	
	//atualiza o registrador cr3 apenas caso um novo espa�o de endere�os seja selecionada
	if(pdir != pdir_atual)
	{
		//habilitando pagina��o. colocando endere�o da Page directory no registrador cr3
		asm volatile("mov %0, %%cr3":: "b"(pdir));
		
		//recarregando Table Lookaside Buffer
		recarregar_tlb();
	}
}

//Retorna o endere�o f�sico que ser� mapeado para um dado endere�o virutal
void * MemoriaVirtual::obter_end_fisico(unsigned int endereco_virtual)
{
	//obtendo �ndice na Page Directory e na Page Table	
	unsigned long pdindex = (unsigned long)endereco_virtual >> 22;
	unsigned long ptindex = (unsigned long)endereco_virtual >> 12 & 0x03FF;
	  
	//Verificando se a Page Table est� presente na mem�ria
	if((page_directory[pdindex] & 0x01) == 0x01) 
	{
		//Obtendo endere�o virtual da Page Table
		unsigned int * pt =  obter_endereco_page_table(pdindex);
		
		/*Retorna endere�o do frame de mem�ria apontado pela page table, 
		somado aos 12 bits finais do endere�o f�sico (offset dentro do frame)*/
	    return (void *)((pt[ptindex] & ~0xFFF) + ((unsigned long)endereco_virtual & 0xFFF));
	}
	else
	{
		//page table n�o presente, retorna -1
	    return -1;
	}
  
}

//Retorna o endere�o do page directory padr�o do kernel
int * MemoriaVirtual::obter_pdir()
{
	return pdir;
}

//Aloca mais mem�ria dinamicamente para o Kernel
void * MemoriaVirtual::sbk(unsigned int tam)
{ 
    void * ret;
    void * ret_vadr = topo_kernel_heap;
	
	//Endere�o a ser retornando para o usu�rio � o topo do HEAP atual
    ret = (void*) (((unsigned int)mem_virtual.topo_kernel_heap) + tam);
	topo_kernel_heap = ret;

	/*
	  O atributo memoria_a_alocar � o marco de quanta
	  memoria foi alocada de fato no frame de mem�ria f�sica atual.
	  Quando o valor de memoria_a_alocar ultrapassa o tamanho do bloco (4096 bytes)
   	  mais mem�ria f�sica � alocada.
	*/
	mem_virtual.memoria_a_alocar += tam;
	
	if(mem_virtual.memoria_a_alocar >=TAM_BLOC)
	{
	   //calculando quantidade de blocos que deve ser alocado	
	   unsigned int num_blocos_alocar = mem_virtual.memoria_a_alocar/TAM_BLOC + 1;
	   
	   //mapeando as p�ginas na mem�ria virtual � medida que forem necess�rias
	   for(unsigned int i =0; i < num_blocos_alocar; i++)
	   {
	      //alocando frame de mem�ria f�sica
		  ret = mem_fisica.alocar(1);
		  
		  //caso endere�o retornando -1 (sem mem�ria)
		  if (ret == -1) return ret;
		  
		  //mapeando endere�o retornando no topo do HEAP
		  mem_virtual.mapear_page_table(ret, mem_virtual.ult_end_ret, 0x03);
		  
		  //incrementando valor do �ltimo endere�o retornando
		  mem_virtual.ult_end_ret += TAM_BLOC;
		  
	   }//fim do for  
		
       //atualizando valor de memoria_a_alocar 		
	   mem_virtual.memoria_a_alocar = mem_virtual.memoria_a_alocar%TAM_BLOC;
	   
	}//fim do if

	
	
	return ret_vadr;
}


//========================================================================================================================
/*Implementa��o das fun��es MALLOC e FREE 
obtidas no livro The C Programming Language (K & R).
Adaptadas para funcionar junto com o Kernel.*/

typedef long Align; 

//cabe�alho do bloco
union header 
{  
	struct 
	{
	    //Pr�ximo bloco na lista de blocos livres
		union header *ptr; 
		
		//Tamanho do pr�ximo bloco
		unsigned int size;     
	} s;
	
	Align x; //Para fazer com que os blocos fiquem alinhados
}__attribute__((packed));

typedef union header Header;

//Cria��o da lista vazia 
static Header base;  

//In�cio da lista de blocos livres
static Header *freep = NULL; 

//Prot�tipos
static Header *morecore(unsigned nu);
void   free(void *ap);
void   *malloc(unsigned nbytes);

//Kmalloc: Aloca mem�ria dinamicamente (Para uso dentro do Kernel)
void * kmalloc(unsigned int nbytes)
{
	mem_virtual.down();
	
	Header *p, *prevp;	
	unsigned nunits;
	void * end;
	
	nunits = (nbytes+sizeof(Header)-1)/sizeof(header) + 1;
	
	//Verificando se a lista de blocos livres foi inicializada
	if ((prevp = freep) == NULL)
	{ 
		base.s.ptr = freep = prevp = &base;
		base.s.size = 0;
	}

	//Varrendo lista de blocos livres
	for (p = prevp->s.ptr; ; prevp = p, p = p->s.ptr) 
	{
		//Verificando se o bloco livre � grande o sufciente
		if ( !(p->s.size < nunits)) 
		{ 
			//Verificando se o bloco tenho necess�rio
			if (p->s.size == nunits) 
			{
				prevp->s.ptr = p->s.ptr;
			}
			else //Caso seja maior, aloca por��o final do bloco
			{ 
				p->s.size = p->s.size - nunits;
				p += p->s.size;
				p->s.size = nunits;
				
			}//fim do else
				
			freep = prevp;
			end = (void *)(p+1);
			break;
		}
		
		//Verificando se o final da lista foi atingido
		if (p == freep) 
		{
			
		   //solicita mais mem�ria (incrementa o heap do kernel) 
		   if ((p = morecore(nunits)) == NULL)
		   {
			   end = NULL;
			   break;
		   }
		}
		
	}//fim do for
		
		
	mem_virtual.up();	
	return end;
}

//Fun��o para liberar ou criar um novo n� na lista de blocos livres
void _free(void * ap)
{
	if((int)ap >= POS_INICIAL_KERNEL_HEAP )
	{
		Header *bp, *p;
		bp = (Header *)ap - 1; 
		
		for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
			if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
				   break; 
		
		if (bp + bp->s.size == p->s.ptr) 
		{ 
			bp->s.size += p->s.ptr->s.size;
			bp->s.ptr = p->s.ptr->s.ptr;
		} 
		else
		{
			bp->s.ptr = p->s.ptr;
		}
				
		if (p + p->s.size == bp) 
		{ 
			p->s.size += bp->s.size;
			p->s.ptr = bp->s.ptr;
		}
		else
		{
			p->s.ptr = bp;
		}

		freep = p;
	}
}

void free(void *ap)
{
	_free(ap);
}

//Fun��o que solicita mais mem�ria do HEAP do Kernel
static Header *morecore(unsigned nu)
{
	char *cp;
	Header *up;

	if (nu < NALLOC)
		nu = NALLOC;
		
	//chamando fun��o SBK para solicitar mais mem�ria	
	cp = (char *)mem_virtual.sbk(nu * sizeof(Header));
	
	//caso retorno seja -1, n�o h� mem�ria dispon�vel
	if (cp == (char *) -1) 
		return NULL;
		
	up = (Header *) cp;
	up->s.size = nu;
	
	_free((void *)(up+1));
	
	return freep;
}

