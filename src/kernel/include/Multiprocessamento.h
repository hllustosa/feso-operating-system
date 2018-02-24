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
#ifndef _MULTIPROC
#define _MULTIPROC

#include "Memoria.h"
#include "Loader.h"
#include "Avl.h"

/**
 * Estrutura utilizada no suporte a múltiplos processadores.
 */
struct MP_FLOAT_POINT
{
	unsigned char assinatura[4]; 	/*!<assinatura "_MP_" */
	unsigned char * config_ptr; 	/*!<Poteiro para a MP Configuration Table */
	unsigned char tam; 		/*!<tamanho da estrutura */
	unsigned char versao;		/*!<versão da especificação */
	unsigned char checksum;			
	unsigned char mp_feat1; 	/*!<flags de características */
	unsigned char mp_feat2; 	/*!<flags de características */
	unsigned char mp_feat3_5[3]; 	/*!<flags de características */
	
}__attribute__((packed));

/**
 * Estrutura utilizada no suporte a múltiplos processadores.
 */
struct MP_CONFIG_TABLE
{
	unsigned char assinatura[4];	/*!< assinatura "PCMP" */
	unsigned short tam;             /*!< tamanho da tabela base */
	unsigned char spec_rev;		/*!< versão */
	unsigned char checksum;			
	unsigned char oemid[8];		/*!<string com id do fabricante	*/ 
	unsigned char productid[12];    /*!<string com id do produto */
	unsigned int * oem_table_ptr;	/*!<ponteiro para oem table */
	unsigned short oem_table_tam;	/*!<tamanho da oem table */
	unsigned short qtd_itens;	/*!<Quantidade de itens na MP Config Table */
	unsigned int lapic_add;		/*!<Endereço físico do local do LAPIC */
	unsigned short xtable_tam;	/*!<Tamanho da Extended Table */
	unsigned short xtable_checksum;
	
}__attribute__((packed));

/**
 * Estrutura utilizada no suporte a múltiplos processadores.
 */
struct MP_CONFIG_TABLE_PROC
{
	unsigned char tipo;		/*!<entrada da MP_CONFIG_TABLE referente a um processador (valor 0) */
	unsigned char lapicid;  	/*!<id do Local APIC */
	unsigned char lapicver; 	/*!<versão do LAPIC */
	unsigned char cpu_enable_bsp;	/*!<Bit 0 indica se a CPU está habilitada, e o 1 se é o bootstrap processor */
	unsigned char cpu_assinatura[4];/*!<Assinatura da CPU */
	unsigned char cpu_flags[4];	/*!<Flags da CPU*/
	
}__attribute__((packed));

/**
 * Estrutura utilizada no suporte a múltiplos processadores.
 */
struct MP_CONFIG_TABLE_IOAPIC
{
	unsigned char tipo;		/*!<entrada da MPConfig Table referente a um processador (valor 0) */
	unsigned char iopicid;  	/*!<Id do IOAPIC */
	unsigned char ioapicver; 	/*!<versão do IOAPIC */
	unsigned char iopaic_enable;	/*!<Bit 7 indica se IO apic está habilitado */
	unsigned int ioapic_add;	/*!<Endereço físico da localização do IO APIC */

}__attribute__((packed));

/**
 * Estrutura utilizada no suporte a múltiplos processadores.
 */
struct MP_CONFIG_INTERRUPT
{
	unsigned char tipo;		/*!<entrada da MPConfig Table referente a uma interrupt (valor 3) */
	unsigned char tipo_interrupcao; /*!<tipo da interrupcao */
	unsigned char flag[2]; 		/*!<flags */
	unsigned char source_bus;	/*!<barramento de origem */
	unsigned char source_bus_irq;	
	unsigned char idioapicdest;	/*!<id do ioapic de destino */
	unsigned char indice;		/*!<id do ioapic de destino */

}__attribute__((packed));

/**
 * Estrutura utilizada no suporte a múltiplos processadores.
 */
struct RSDP 
{
	unsigned char assinatura[8];
	unsigned char checksum;
	unsigned char oemid[6];
	unsigned char revisao;
	unsigned int rsdt_end;
  
}__attribute__((packed));

/**
 * Estrutura utilizada no suporte a múltiplos processadores.
 */
struct RSDT_HEADER 
{
  unsigned char assinatura[4];
  unsigned int tam;
  unsigned char revisao;
  unsigned char checksum;
  unsigned char oemid[6];
  unsigned char oemtableid[8];
  unsigned int oemrevisao;
  unsigned int criadorid;
  unsigned int criadorrevisao;
  
}__attribute__((packed));

/**
 * Estrutura utilizada no suporte a múltiplos processadores.
 */
struct RSDT
{
	RSDT_HEADER cabecalho;
	unsigned int conteudo[200];
	
}__attribute__((packed));

/**
 * Estrutura utilizada no suporte a múltiplos processadores.
 */
struct MADT_ENTRY
{
	unsigned char type;
	unsigned char size;
	
}__attribute__((packed));

/**
 * Estrutura utilizada no suporte a múltiplos processadores.
 */
struct MADT_HEADER
{
	RSDT_HEADER header;
	unsigned int lapic_end;
	unsigned int flags;

}__attribute__((packed));

/**
 * Estrutura utilizada no suporte a múltiplos processadores.
 */
struct MADT
{
	MADT_HEADER header;
	unsigned int * content;
	
}__attribute__((packed));

/**
 * Estrutura utilizada no suporte a múltiplos processadores.
 */
struct ACPI_LAPIC
{
	MADT_ENTRY cabecalho;
	unsigned char idprocessador;  /*!<ID do processador.*/
	unsigned char apicid;	      /*!<ID do APIC.*/
	unsigned int flags;           /*!<Bit zero indica se o processador está reservado.*/
					
}__attribute__((packed));

/**
 * Estrutura utilizada no suporte a múltiplos processadores.
 */
struct ACPI_IOAPIC
{
	MADT_ENTRY cabecalho;
	unsigned char ioapicid;		  /*!<ID do I/O apic.*/
	unsigned char reservado;	  /*!<Reservado.*/
	unsigned int ioapic_end;	  /*!<Endereço do I/O apic.*/
	unsigned int gsib;                /*!<Número da primeira interrupção.*/
	
}__attribute__((packed));

/**
 * Estrutura utilizada no suporte a múltiplos processadores.
 */
struct ACPI_INTERRUPT_SOURCE
{
	MADT_ENTRY cabecalho;
	unsigned char bus;		
	unsigned char origem;		
	unsigned int gsi;		
	unsigned char flags[2];
	
}__attribute__((packed));


/**
*Esta classe é responsável por detectar e inicializar múltiplos processadores.
*/
class Multiproc
{
	Lista<MP_CONFIG_TABLE_IOAPIC> ioapics;
	Lista<MP_CONFIG_TABLE_PROC> processadores;
	Lista<MP_CONFIG_INTERRUPT> interrupcoes;
	Arvore<int> apics;

	int multiproc_habilitado;
	unsigned int apic;
	unsigned int ioapic;
	unsigned int entrada;
	
	public:
		
	volatile int processadores_iniciados;	
	
	void inicializar();
	void inicializar_ioapic();
	void habilitar_apic();
	void habilitar_apic_timer();
	void inicializar_ioapic2();
	void analisar_madt(unsigned int * end);
	void carregar_codigo_trampolim(char * imagem);
	void analisar_MPConfigTable(char * ptr);
	void escrever_reg_apic(int reg, int dado);
	void escrever_reg_ioapic(int reg, int dado);
	void enviar_eoi_apic();
	int buscar_floating_point_struc(MP_FLOAT_POINT * mpfloatpoint);
	int buscar_rsdt();
	int obter_apicid();
	unsigned int ler_reg_apic(int reg);
	unsigned int ler_reg_ioapic(int reg);

};

extern volatile Multiproc multiproc;
extern "C" volatile int test_and_set(int novo_valor, int * ptr);
void trampolim();
void down();
void up();

#endif