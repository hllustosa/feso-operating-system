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
#include "include/Gdt.h"
#include "include/Util.h"

//Ver arquivo start.asm	para defini��o da estrutura
struct GDT_PTR gp;

//Inst�ncia da global da classe GDT
volatile Gdt gdt;

//Fun��o respos�vel por configurar a GDT
void Gdt::inicializar(int bss)
{
    //Configurando o ponteiro para o GDT 
    gp.limite = (sizeof(struct REGISTRO_GDT) * 24) - 1;     //O limite da GDT para 24 registros (8registros b�sicos + 1 TSS por CPU (max 16))
    gp.base   = (unsigned int) &this->registros;	    //O Endere�o base da GDT � o endere�o do array
	
    //Adicionando registro nulo. (O Primeiro registro da GDT dever ser nulo)
    adicionar_registro(0, 0, 0, 0, 0);

    //Adicionando entradas na GDT. A base � Sempre 0, o Limite � de 4GB (flat memory model)
    adicionar_registro(1, 0, 0xFFFFFFFF, Gdt::ANEL0_CODIGO, 0xCF); //indice 0x08
    adicionar_registro(2, 0, 0xFFFFFFFF, Gdt::ANEL0_DATA,   0xCF); //indice 0x10
    adicionar_registro(3, 0, 0xFFFFFFFF, Gdt::ANEL1_CODIGO, 0xCF); //indice 0x18
    adicionar_registro(4, 0, 0xFFFFFFFF, Gdt::ANEL1_DATA,   0xCF); //indice 0x20
    adicionar_registro(5, 0, 0xFFFFFFFF, Gdt::ANEL3_CODIGO, 0xCF); //indice 0x28
    adicionar_registro(6, 0, 0xFFFFFFFF, Gdt::ANEL3_DATA,   0xCF); //indice 0x30

	//instalando o TSS
	unsigned int end = (unsigned int)&tss;
	
    //adicionar_registro(7, end, end+sizeof(Tss), 0xE9, 0x00);	
	adicionar_registro(7, end, sizeof(TSS) -1, 0xE9, 0x00);	
	
	//limpando estrutura da TSS
	memset((unsigned char *)&tss, 0, sizeof(tss));
	
	//configurando os campos da TSS
	tss.iomap = (unsigned short) sizeof(TSS);
	tss.esp0  = bss;
	tss.ss0   = 0x10;
	tss.cs    = 0x00;
    tss.ss    = tss.ds = tss.es = tss.fs = tss.gs = 0x13;

	//Apagando a GDT antiga (falsa) e instalando a nova
	carregar_gdt();

	//Alterando o registrador que armazena a posi��o do TSS, 0x38 representa a 7� entrada na GDT
	asm volatile(" mov $0x38, %ax; ltr %ax"); //asm volatile(" mov $0x2B, %ax; ltr %ax");
	
	//configurando a quantidade de registros presente na GDT
	qtd_registros = 8;
	
	
}

//Adicionar um novo registro � Global Descriptor Table 
void Gdt::adicionar_registro(int num, unsigned long base, unsigned long limite, unsigned char accesso, unsigned char gran)
{
    //Configura o endere�o base dos registro
    this->registros[num].base_low     = (base & 0xFFFF);
    this->registros[num].base_meio    = (base >> 16) & 0xFF;
    this->registros[num].base_high    = (base >> 24) & 0xFF;

    //Configura o limite do registro 
    this->registros[num].limite_low    = (limite & 0xFFFF);
    this->registros[num].granularidade = ((limite >> 16) & 0x0F);

    //Configura a granularidade e o n�vel de acesso (Ring0, Ring1, Ring3)
    this->registros[num].granularidade |= (gran & 0xF0);
    this->registros[num].accesso       = accesso;
}

//adiciona um novo registro do tipo TSS � Global Descritor Table
void Gdt::adicionar_tss(unsigned int pilha)
{
	//alocando mem�ria para o TSS
	TSS * nova_tss	= (TSS*)kmalloc(sizeof(TSS));
	
	//limpando estrutura da TSS
	memset((unsigned char *)nova_tss, 0, sizeof(TSS));
	
	//adicionando um novo registro do tipo tss na GDT;	
	adicionar_registro(qtd_registros, nova_tss, sizeof(TSS) -1, 0xE9, 0x00);	
	
	//configurando os campos da TSS
	nova_tss->iomap = (unsigned short) sizeof(TSS);
	nova_tss->esp0  = pilha;
	nova_tss->ss0   = 0x10;
	nova_tss->cs    = 0x00;
    nova_tss->ss    = nova_tss->ds = nova_tss->es = nova_tss->fs = nova_tss->gs = 0x13;
	
	//incrementando a quantidade de registros na GDT
	qtd_registros++;
}

//Altera o endere�o para o registrador ESP que ser� carregado quando uma interrup��o ocorrer
void Gdt::alterar_tss_esp(unsigned int esp)
{
	tss.esp0 = esp;
}
