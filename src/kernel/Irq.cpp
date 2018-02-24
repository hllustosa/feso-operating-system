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
#include "include/Irq.h"
#include "include/Idt.h"
#include "include/Mensagem.h"
#include "include/Multiprocessamento.h"
#include "include/Util.h"


//Fun��es IRQ externas, definidas em start.asm
extern "C" void irq0();
extern "C" void irq1();
extern "C" void irq2();
extern "C" void irq3();
extern "C" void irq4();
extern "C" void irq5();
extern "C" void irq6();
extern "C" void irq7();
extern "C" void irq8();
extern "C" void irq9();
extern "C" void irq10();
extern "C" void irq11();
extern "C" void irq12();
extern "C" void irq13();
extern "C" void irq14();
extern "C" void irq15();
extern "C" void irq_spu();

//Inst�ncia da classe irq
Irq irq;
   
//array de ponteiros para fun��es, armazenam o endere�o das fun��es de tratamento para IRQ
void * funcoes[16];

//Construtor que inicializa vetor de ponteiros
Irq::Irq(void)
{
    memset((unsigned char *) funcoes,0,sizeof(funcoes));
}

//Inicializano IRQS
void Irq::inicializar(Idt idt)
{
    remapear_irqs();
   
    idt.adicionar_registro(32, (unsigned)irq0,  0x08, 0x8E);
    idt.adicionar_registro(33, (unsigned)irq1,  0x08, 0x8E);
    idt.adicionar_registro(34, (unsigned)irq2,  0x08, 0x8E);
    idt.adicionar_registro(35, (unsigned)irq3,  0x08, 0x8E);
    idt.adicionar_registro(36, (unsigned)irq4,  0x08, 0x8E);
    idt.adicionar_registro(37, (unsigned)irq5,  0x08, 0x8E);
    idt.adicionar_registro(38, (unsigned)irq6,  0x08, 0x8E);
    idt.adicionar_registro(39, (unsigned)irq7,  0x08, 0x8E);
    idt.adicionar_registro(40, (unsigned)irq8,  0x08, 0x8E);
    idt.adicionar_registro(41, (unsigned)irq9,  0x08, 0x8E);
    idt.adicionar_registro(42, (unsigned)irq10, 0x08, 0x8E);
    idt.adicionar_registro(43, (unsigned)irq11, 0x08, 0x8E);
    idt.adicionar_registro(44, (unsigned)irq12, 0x08, 0x8E);
    idt.adicionar_registro(45, (unsigned)irq13, 0x08, 0x8E);
    idt.adicionar_registro(46, (unsigned)irq14, 0x08, 0x8E);
    idt.adicionar_registro(47, (unsigned)irq15, 0x08, 0xEE);

	for(int i = 48; i < 256; i++)
		idt.adicionar_registro(i, (unsigned)irq_spu, 0x08, 0xEE);
}
 
//Instala um nova rotina de tratamento de interrup��o para o IRQ especificado
void Irq::instalar_funcao(int irq, void (*funcao)(struct REGS *r))
{
    funcoes[irq] = (void *)funcao;
}

//Remove a fun��o de tratamento para o IRQ especificado
void Irq::desinstalar_funcao(int irq)
{
    funcoes[irq] = 0;
}

void Irq::remapear_irqs()
{
	/* Geralmente, IRQs 0 at� 7 s�o mapeados para as entradas de 8 a 15 da IDT. 
	o problema � que em modo protegido, as entradas de IDT de 0 31 s�o reservadas para exceptions.
	Sem o remapeamento, toda vez que IRQ0 dispara, temos uma
	Double Fault Exception, que n�o ocorreu de fato
	Ent�o enviamos comandos para o (Programmable
	Interrupt Controller)  com o objetivo de remapear as IRQ0 at� IRQ7 para a
	as entradas 32 a 47 da IDT */

    outportb(0x20, 0x11);
    outportb(0xA0, 0x11);
    outportb(0x21, 0x20);
    outportb(0xA1, 0x28);
    outportb(0x21, 0x04);
    outportb(0xA1, 0x02);
    outportb(0x21, 0x01);
    outportb(0xA1, 0x01);
    outportb(0x21, 0x0);
    outportb(0xA1, 0x0);
}


//fun��o que executa o tratamendo da IRQ
extern "C" void tratar_irq(struct REGS *r)
{
		down();
		
		//Criando um ponteiro para uma fun��o em branco
		void (*funcao)(struct REGS *r) = (void (*) (REGS*)) funcoes[r->int_no - 32];
		
		int int_no = r->int_no;
		
		//Verifica se existe uma fun��o de tratamento personalizada para a IRQ
		if (funcao)
		{
			funcao(r);
		}
		
		/* Enviando sinal para PIC secund�rio, caso n�mero IRQ seja maior que 40 
		pois o PIC secund�rio trata as IRQ de 8 a 15 que est�o mapeadas nas entradas
		da IDT acima de 40. Nota: O PIC secund�rio est� ligado na IRQ2 do PIC principal
		sempre que uma IRQ entre 8 e 15 � gerada, a 2 � gerada ao mesmo tempo*/
		if (int_no >= 40)
		{
			outportb(0xA0, 0x20);
		}
			
		//Notifica o PIC do final do tratamento do IRQ
		outportb(0x20, 0x20);		
				
		//notifica o apic
		multiproc.enviar_eoi_apic();
		
		up();
}