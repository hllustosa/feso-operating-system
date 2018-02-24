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
#ifndef _GDT
#define _GDT

#include "Memoria.h"

/**
* Estrutura do TSS (task state segment) estrutura utilizada no troca de contexto via hardware.
* Necessário para indicar qual é o ESP da pilha do kernel durante uma interrupção.
*/
struct TSS {

	unsigned short link;
	unsigned short link_h;
	unsigned long esp0;
	unsigned short ss0;
	unsigned short ss0_h;
	unsigned long esp1;
	unsigned short ss1;
	unsigned short ss1_h;
	unsigned long esp2;
	unsigned short ss2;
	unsigned short ss2_h;
	unsigned long cr3;
	unsigned long eip;
	unsigned long eflags;
	unsigned long eax;
	unsigned long ecx;
	unsigned long edx;
	unsigned long ebx;
	unsigned long esp;
	unsigned long ebp;
	unsigned long esi;
	unsigned long edi;
	unsigned short es;
	unsigned short es_h;
	unsigned short cs;
	unsigned short cs_h;
	unsigned short ss;
	unsigned short ss_h;
	unsigned short ds;
	unsigned short ds_h;
	unsigned short fs;
	unsigned short fs_h;
	unsigned short gs;
	unsigned short gs_h;
	unsigned short ldt;
	unsigned short ldt_h;
	unsigned short trap;
	unsigned short iomap;

}__attribute__((packed));


/**
*Struct para definir um registro na GDT
*/
struct REGISTRO_GDT
{
    unsigned short limite_low;
    unsigned short base_low;
    unsigned char  base_meio;
    unsigned char  accesso;
    unsigned char  granularidade;
    unsigned char  base_high;
} __attribute__((packed));

/** 
*Ponteiro para a GDT
*/
struct GDT_PTR
{
    unsigned short limite;
    unsigned int base;
	
} __attribute__((packed));


/**
*Declaração da Classe GDT (Global Descriptors Table) com vetor de 8 entradas
*/
class Gdt
{	
	TSS tss;
	REGISTRO_GDT registros[24];
	int qtd_registros;

	public:

	//Constantes para tidos de entradas na GDT
	static const int ANEL0_CODIGO  = 0x9A;
	static const int ANEL0_DATA    = 0x92;
	static const int ANEL1_CODIGO  = 0xBA;
	static const int ANEL1_DATA    = 0xB2;
	static const int ANEL3_CODIGO  = 0xFA;
	static const int ANEL3_DATA    = 0xF2;

	//Protótipos
	void inicializar(int bss);
	void adicionar_registro(int, unsigned long, unsigned long, unsigned char, unsigned char);	
	void adicionar_tss(unsigned int);
	void alterar_tss_esp(unsigned int);
	
};

extern volatile Gdt gdt;
extern "C" void carregar_gdt();

#endif
