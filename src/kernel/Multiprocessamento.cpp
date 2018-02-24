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
#include "include/Multiprocessamento.h"
#include "include/Processo.h"
#include "include/Memoria.h"
#include "include/Util.h"
#include "include/Vfs.h"
#include "include/Idt.h"
#include "include/Gdt.h"

#define DELAY_200 \
for(int delay = 0; delay < 1000000;delay++);


volatile Multiproc multiproc;
volatile int mutex = 0;


//========================================================================================================================
//Mutex

void down()
{	
	_down(&mutex);
}

void up()
{
	mutex = 0;
}



//========================================================================================================================
//Multiproc
void Multiproc::inicializar()
{
	int desc, reg, _processadores_iniciados; 
	MP_FLOAT_POINT mpfloatpoint;
	mutex = 0;

	//configurando vari�vel para multiprocessamento
	multiproc_habilitado = 0;
	
	//configurando vari�vel para controle de processadores iniciados
	processadores_iniciados =1;
	
	/*
		Buscando tabela APIC dentro das estruturas ACPI.
		Indica a presen�a de multiplos processadores.	
	*/
	if(buscar_rsdt())
	{
		multiproc_habilitado = 1;
		
		//alocando mem�ria virtual para o apic
		apic = POS_INICIAL_KERNEL + 0x4B000;
	
		//mapeando a mem�ria f�sica do APIC na mem�ria virtual 
		mem_virtual.mapear_page_table(0xFEE00000, apic, 0x03);

		//habilitando apic do processador (BSP)		
		habilitar_apic();
		
		//colocando o endere�o da pagedirectory para acesso em outros processadores
		unsigned int * ptr_pdir = (0x7D008 + POS_INICIAL_KERNEL);
		*ptr_pdir = mem_virtual.obter_pdir();
		
		//colocando endere�o do c�digo trampolim
		unsigned int * ptr_jmp = (0x7D000 + POS_INICIAL_KERNEL);
		*ptr_jmp = (trampolim - POS_INICIAL_KERNEL);
		
		//verificando se algum ioapic foi detectado
		if(ioapics.tamanho() > 0)
		{
			//alocando mem�ria virtual para o ioapic
			ioapic = POS_INICIAL_KERNEL + 0x4C000;
		
			//mapeando a mem�ria f�sica do IOAPIC na mem�ria virtual 
			mem_virtual.mapear_page_table(0xFEC00000, ioapic, 0x03);
			
			//habilitando e inicializando ioapic
			inicializar_ioapic();
		}
		
		//adicionando uma TSS para cada processador
		for(int i =1; i < processadores.tamanho(); i++)
		{
			unsigned int pilha = (unsigned int)kmalloc(4096);
			gdt.adicionar_tss(pilha + 4092);
		}
		
		//Escrevendo dados sobre os processador
		vfs.abrir("/dev/mpinfo", 'a', &desc);
		
		for(int i =0; i < processadores.tamanho(); i++)
		{
			//salva a quantidade de processadores iniciados at� o momento
			_processadores_iniciados = processadores_iniciados;
			
			vfs.escrever(desc,"id lapic: ", strlen("id lapic: "));	
			vfs.escrever(desc, itoa(processadores[i].lapicid, 10), 3);	
			vfs.escrever(desc, "\n", 1);	
			
			vfs.escrever(desc,"cpu habilitada : ", strlen("cpu habilitada : "));	
			vfs.escrever(desc, itoa( obter_estado_bit(processadores[i].cpu_enable_bsp,0), 10), 3);	
			vfs.escrever(desc, "\n", 1);
			
			//verificando se a CPU est� habilitada e n�o � a BSP
			if(obter_estado_bit(processadores[i].cpu_enable_bsp,0) && 
		           obter_apicid() != processadores[i].lapicid)
			{
				//enviar INIT IPI para o processador
				reg = 0 | (processadores[i].lapicid << 24); 
				escrever_reg_apic(0x310, reg);
				
				reg = 0 | (0x01 << 14)|(0x05 << 8) | 0x00 >> 12;
				escrever_reg_apic(0x300, reg);
				
				DELAY_200;
				
				//enviar sequ�ncia SIPI-SIPI para um processador 
				for(int cont =0; cont < 1; cont++)
				{
					// enviando para o apic id do processador		
					reg = 0 | (processadores[i].lapicid << 24); 
					escrever_reg_apic(0x310, reg);
					
					//enviando ponto de entrada 
					reg = 0 | (0x01 << 14)|(0x06 << 8) | entrada >> 12;
					escrever_reg_apic(0x300, reg);
					
					DELAY_200;
				}
				
				//espera que um novo processador seja iniciado
				while(_processadores_iniciados == processadores_iniciados); 
				
			}//fim do if
			
			
		}//fim do for
		
		
		vfs.escrever(desc,"ioapic id : ", strlen("ioapic id : "));	
		vfs.escrever(desc, itoa(ioapics[0].iopicid, 16), 3);	
		vfs.escrever(desc, "\n", 1);	
		
		vfs.escrever(desc,"ioapic habilitado: ", strlen("ioapic habilitado: "));	
		vfs.escrever(desc, itoa(ioapics[0].iopaic_enable, 16), 3);	
		vfs.escrever(desc, "\n", 1);	
			
		vfs.escrever(desc,"endereco ", strlen("endereco "));	
		vfs.escrever(desc, itoa(ioapics[0].ioapic_add, 10), 32);	
		vfs.escrever(desc, "\n\n", 2);	
		
		//Escrevendo informa��es sobre interrup��es		
		for(int i =0; i < interrupcoes.tamanho(); i++)
		{
			vfs.escrever(desc,"source_bus_irq ", strlen("source_bus_irq "));	
			vfs.escrever(desc, itoa(interrupcoes[i].source_bus_irq, 10), 32);	
			vfs.escrever(desc, "\n", 1);
			
			vfs.escrever(desc,"tipo ", strlen("tipo "));	
			vfs.escrever(desc, itoa(interrupcoes[i].tipo_interrupcao, 10), 32);	
			vfs.escrever(desc, "\n", 1);
			
			vfs.escrever(desc,"flag ", strlen("flag "));	
			vfs.escrever(desc, itoa(interrupcoes[i].flag[0], 10), 32);	
			vfs.escrever(desc, "\n", 1);
			
			vfs.escrever(desc,"flag ", strlen("flag "));	
			vfs.escrever(desc, itoa(interrupcoes[i].flag[1], 10), 32);	
			vfs.escrever(desc, "\n", 1);
			
			vfs.escrever(desc,"dest #intin ", strlen("dest #intin "));	
			vfs.escrever(desc, itoa(interrupcoes[i].indice, 10), 32);	
			vfs.escrever(desc, "\n\n", 2);			
		} 
		
		vfs.fechar(desc);		
	} 

}


void Multiproc::inicializar_ioapic()
{
	unsigned int dados1, dados2, arq;
	unsigned int deliverymode, destinationmode, pinpolarity, triggermode, mask,destino;
	
	//desabilitando PIC (mascarando interrup��es)
	outportb(0xa1, 0xff);
	outportb(0x21, 0xff);
	
	//abrindo arquivo para salvar dados sobre configura��o do ioapic
	vfs.abrir("/dev/ioapic.txt", 'a', &arq);
	
	//configurando todas as interrup��es
	for(int i = 0; i< interrupcoes.tamanho(); i++)
	{
		//calculando posi��o do primeiro registrador para a IRQ i
		unsigned int registrador_irq = 0x10+(interrupcoes[i].indice*2);
		
		//configurando modo de destino
		destinationmode = 0x00; //(0x00 physical mode | 0x01 logical mode)	
		
		//obtendo modo de entrega(delivery mode)
		switch(interrupcoes[i].tipo_interrupcao)
		{
			case 0: deliverymode = 0x00; break; //(modo fixed)
			case 1: deliverymode = 0x04; break; //(modo nmi)
			case 2: deliverymode = 0x02; break; //(modo smi)
			case 3: deliverymode = 0x00; break; //(modo extint)
		}
		
		//configurando polaridade do pino
		int po = interrupcoes[i].flag[0] & 0x03;
		switch(po)
		{
			case 0: pinpolarity = 0x00; break; //(conforme especifica��o)
			case 1: pinpolarity = 0x00; break; //(active high)
			case 2: pinpolarity = 0x00; break; //(reserved)
			case 3: pinpolarity = 0x01; break; //(active low)
		}
		
		//configurando trigger mode
		int el = interrupcoes[i].flag[0] & 0x0C;
		switch(el)
		{
			case 0: triggermode = 0x00; break; //(conforme especifica��o)
			case 1: triggermode = 0x00; break; //(edge trigger)
			case 2: triggermode = 0x00; break; //(reserved)
			case 3: triggermode = 0x01; break; //(level trigger)
		}
		
		//configurando m�scara
		mask = 0x00;
			
		//definindo destino (dependendo do destination mode)
		destino = 0x00; // cpu 0

		//configurando n�mero do vetor e outros flags	
		dados1 = interrupcoes[i].source_bus_irq + 32;
		dados1 =(mask << 16)| (triggermode << 15) |(pinpolarity << 13)|(destinationmode << 11) | (deliverymode << 8) | dados1;		
				
		//configurando o destino da irq		
		dados2 = (destino << 24);	
				
		//escrevendo dados no arquivo		
		vfs.escrever(arq, itoa(dados1, 16), 10);
		vfs.escrever(arq, "\n", 1);
		vfs.escrever(arq, itoa(interrupcoes[i].indice, 16), 10);
		vfs.escrever(arq, "\n", 1);
				
		//escrevendo os dados nos registradores do ioapic
		escrever_reg_ioapic(registrador_irq + 0x01, dados2);
		escrever_reg_ioapic(registrador_irq, dados1);

	}//fim do for
		
	vfs.fechar(arq);	
}

void Multiproc::habilitar_apic()
{
	//lendo registrador do apic
	int reg = 0;
	
	//setando bit para habilitar apic
	reg = reg | 0x100;  
	
	//configurando vetor da spurious interrupt
	reg = reg | 0xFF;   
	
	//escrevendo no registrador do apic
	escrever_reg_apic(0xf0, reg);	
}


/*
	Cada LAPIC possui um timer. A frequ�ncia desse timer
	� igual a frequ�ncia do barramento da CPU. Para calcular a frequ�ncia,
	� necess�rio contar quantos ticks (ou quantos pulsos) s�o gerados 
	pelo timer do LAPIC em um per�odo de tempo determinado por outro temporizador.
	No caso, o outro temporizador � o pr�prio PIT.
*/
void Multiproc::habilitar_apic_timer()
{
	unsigned int frequencia, arq;
	unsigned char aux;

	//inicializando LAPIC em um estado conhecido
	escrever_reg_apic(0xE0, 0xFFFFFFFF);
	escrever_reg_apic(0xD0, (ler_reg_apic(0xD0)&0x00FFFFFF)|1);
	escrever_reg_apic(0x320, 0x10000);
	escrever_reg_apic(0x350, 0x10000);
	escrever_reg_apic(0x360, 0x10000);
	escrever_reg_apic(0x80, 0x00);
	
	//dividindo por 16 o clock do bus
	escrever_reg_apic(0x03E0, 0x03);
	
	//one-shot mode para interrup��o 0x22 ou 34d
	escrever_reg_apic(0x320, 0x22); 
	
	//configurando PIT
	outportb(0x61, (inportb(0x61) & 0xFD)|1);
	outportb(0x43,0xB2);
	outportb(0x42,0x9B);	
	inportb(0x60);	
	outportb(0x42,0x2E);
	aux = inportb(0x61)&0xFE;
	outportb(0x61, aux);
	outportb(0x61, aux|1);
	
	//resetando contagem do TIMER APIC
	escrever_reg_apic(0x380,0xFFFFFFFF);
	
	//aguarando o PIT ser zerado
	while(!(inportb(0x61) & 0x20));
	
	//desabilitando o LAPIC timer
	escrever_reg_apic(0x320, 0x10000);
	
	//configurando n�mero m�ximo de tics
	escrever_reg_apic(0x380, 0xFFFFFF); 
	
	/*
		calculando frequencia da CPU 
		(Frequ�ncia em hz � igual a (0xFFFFFFFF - qtd de ticks do lapic) * 16 * 100)
		16 � o divisor da frequ�ncia do timer do LAPIC configurando anteriomente
		100 � o tempo em ms esperado at� que o PIT zerasse seu contador
	*/
	frequencia = ((0xFFFFFFFF- ler_reg_apic(0x380))+1)*16*100;
	
	//Configurando frequ�ncia para gerar uma interrup��o
	frequencia = frequencia/100/16;
	
	vfs.abrir("/dev/lapict", 'a', &arq);
	vfs.escrever(arq, itoa(frequencia, 10), 10);
	vfs.fechar(arq);
	
	escrever_reg_apic(0x380, frequencia);
	escrever_reg_apic(0x320, 0x22|0x20000);
	escrever_reg_apic(0x03E0, 0x03);
}

void Multiproc::carregar_codigo_trampolim(char * imagem)
{
	Lista<ELF_SEG_T> secoes;
	
	//carregando alterando ponteiro para posi��o do c�digo
	char * ptr = (POS_INICIAL_KERNEL + 0x64000);
		
	//lendo o cabe�alho do arquivo ELF e obtendo as informa��es sobre as se��es
	secoes = load_elf_exec(imagem, &entrada, secoes);
		
	//copiando os dados do heap do kernel para a nova �rea da mem�ria
	for(unsigned int i =0; i < secoes.tamanho(); i++)
	{
	  unsigned char * pos = (unsigned char *)imagem;
	  pos += secoes[i].offset;
	  memcpy(secoes[i].phys_adr + POS_INICIAL_KERNEL, pos, secoes[i].disk_size);
	} 
	
}

int Multiproc::buscar_rsdt()
{
	RSDT rsdt;
	RSDT_HEADER * rsdtaux;

	unsigned int end, arq;
	char * ptr = POS_INICIAL_KERNEL + 639*1024;
	char * ptr_final = POS_INICIAL_KERNEL + 640*1024;
	char encontrado = 0;
	char cont = 0;
	
	vfs.abrir("/dev/acpi",'a', &arq);
	
	do
	{
		//buscando n� ultimo kb do mem�ria base
		for(; ptr < ptr_final; ptr+=16)
		{
			/*
				procurando pela string RSD PTR
				� assinatura da RSDP
			*/
			if(ptr[0] == 'R')
			{
				if( ptr[1] == 'S' 
				    && ptr[2] == 'D'
				    && ptr[3] == ' '
				    && ptr[4] == 'P'
				    && ptr[5] == 'T'
				    && ptr[6] == 'R'
				   )
				{
					
					vfs.escrever(arq, ptr, 4);
					
					RSDP rsdp;
					memcpy((char *)&rsdp , ptr, sizeof(RSDP));
					
					
					end = rsdp.rsdt_end - (rsdp.rsdt_end %4096);
					mem_virtual.mapear_page_table(end, end, 0x03);
				
					
					rsdtaux = rsdp.rsdt_end;
					memcpy((char *) &rsdt, (char *) rsdtaux, sizeof(RSDT));
					
					vfs.escrever(arq, "\n", 1);
					vfs.escrever(arq, itoa((int)rsdp.rsdt_end, 10), 30);
					vfs.escrever(arq, "\n", 1);
					encontrado = 1;	
					
					vfs.escrever(arq, rsdt.cabecalho.assinatura, 4);
					vfs.escrever(arq, "\n", 1);
					vfs.escrever(arq, itoa((int)rsdt.cabecalho.tam, 10), 30);
					vfs.escrever(arq, "\n", 1);
				
				
					break;
				}
					
					
			}
		}
		
		/* caso a estrutura n�o tenha sido encontrada 
		na mem�ria base buscar na mem�ria ROM da BIOS */
		if(!encontrado)
		{
			ptr 	  = POS_INICIAL_KERNEL + 0x000E0000;
			ptr_final = POS_INICIAL_KERNEL + 0x000FFFFF;
		}
		else
		{
			break;
		}
		
		cont++;
		
	}while(cont < 2);
	
	
	if(encontrado)
	{
		int qtd_registros = (rsdt.cabecalho.tam - sizeof(RSDT_HEADER)) / 4;
 
		for (int i = 0; i < qtd_registros; i++)
		{
			RSDT h, *haux;
		   	end = (RSDT *)(rsdt.conteudo[i] - (rsdt.conteudo[i] %4096));
			
			mem_virtual.mapear_page_table(end, end, 0x03);
			haux = rsdt.conteudo[i];
			
			memcpy((char *)&h, (char * )haux, sizeof(RSDT_HEADER));
		
			vfs.escrever(arq, h.cabecalho.assinatura, 4);
			
			if (   h.cabecalho.assinatura[0] == 'A'
				&& h.cabecalho.assinatura[1] == 'P'
				&& h.cabecalho.assinatura[2] == 'I'
				&& h.cabecalho.assinatura[3] == 'C'
			   )
			{
				analisar_madt((unsigned int *)haux);		
				vfs.fechar(arq);
				return 1;
			} 
			
			vfs.escrever(arq, "\n", 1); 
		}
		
		vfs.fechar(arq);
		return 0;
	}
	else
	{
		vfs.fechar(arq);
		return 0;
	}
	
}

void Multiproc::analisar_madt(unsigned int * end)
{
	MADT * madt = (MADT *)end;
	MADT_ENTRY * madt_entrada = (unsigned int)madt + sizeof(MADT_HEADER);
	unsigned int endereco, idapics = 0;
	
	endereco = (unsigned int)end - ((unsigned int)end % 4096);
	mem_virtual.mapear_page_table(endereco, endereco, 0x03);
	
	int qtd_itens = (madt->header.header.tam - sizeof(MADT_HEADER)) / 4;
	
	int arq;
	vfs.abrir("/dev/madt",'a', &arq);
	vfs.escrever(arq, itoa((int)madt_entrada->type,10), 5);
	vfs.escrever(arq, "\n", 1);
	
	int pos = sizeof(MADT_HEADER);
	
	for(int i =0; i< 16; i++)
	{
		MP_CONFIG_INTERRUPT mp_nova_int;
			
		mp_nova_int.tipo = 2;
		mp_nova_int.tipo_interrupcao = 0;
		mp_nova_int.flag[0] = 0x00; 		
		mp_nova_int.flag[1] = 0x00; 		
		mp_nova_int.source_bus = 0;		
		mp_nova_int.source_bus_irq = i;	
		mp_nova_int.idioapicdest = 0;	
		mp_nova_int.indice = i;			
			
		interrupcoes.adicionar(mp_nova_int);
	}
	
	while(pos < madt->header.header.tam)
	{
		vfs.escrever(arq, itoa((unsigned int)madt_entrada->type,10), 5);
		vfs.escrever(arq, " ", 1);
		vfs.escrever(arq, itoa((unsigned int)madt_entrada->size,10), 5);
		vfs.escrever(arq, "\n", 1);
		
		pos+= madt_entrada->size;
		
		if(madt_entrada->type == 0)//lapic
		{
			ACPI_LAPIC * novo_proc = (ACPI_LAPIC *)madt_entrada;
			MP_CONFIG_TABLE_PROC mp_novo_proc;
			
			mp_novo_proc.tipo           = 0;			 	
			mp_novo_proc.lapicid        = novo_proc->apicid;  		
			mp_novo_proc.lapicver       = 0; 	 	
			
			if(novo_proc->flags & 0x01)
			{
			   setar_byte(&mp_novo_proc.cpu_enable_bsp, 0, 1);
			   setar_byte(&mp_novo_proc.cpu_enable_bsp, 1, 0);		  
			   processadores.adicionar(mp_novo_proc);
			   apics.adicionar( mp_novo_proc.lapicid, idapics++);
			}
			else   
			{
			   setar_byte(&mp_novo_proc.cpu_enable_bsp, 0, 0);		
			}  			
		}
		else if(madt_entrada->type == 1)//ioapic
		{
			ACPI_IOAPIC * novo_ioapic = (ACPI_IOAPIC *)madt_entrada;
			MP_CONFIG_TABLE_IOAPIC mp_novo_ioapic;
			
			mp_novo_ioapic.tipo          = 1;
			mp_novo_ioapic.iopicid       = novo_ioapic->ioapicid;
			mp_novo_ioapic.ioapicver     = 0;
			mp_novo_ioapic.iopaic_enable = 1;
			mp_novo_ioapic.ioapic_add    = novo_ioapic->ioapic_end;
			
			ioapics.adicionar(mp_novo_ioapic);
		}
		else if(madt_entrada->type == 2)//interrupt
		{
			ACPI_INTERRUPT_SOURCE * nova_int = (ACPI_INTERRUPT_SOURCE *)madt_entrada;
			MP_CONFIG_INTERRUPT mp_nova_int;
			
			mp_nova_int.tipo = 2;
			mp_nova_int.tipo_interrupcao = 0;
			mp_nova_int.flag[0] = 0x00; 		
			mp_nova_int.flag[1] = 0x00; 		
			mp_nova_int.source_bus = 0;		
			mp_nova_int.source_bus_irq = nova_int->origem;	
			mp_nova_int.idioapicdest = 0;	
			mp_nova_int.indice = nova_int->gsi;			
			
			interrupcoes.adicionar(mp_nova_int);
		}
		
		
		madt_entrada = (unsigned int)madt_entrada+madt_entrada->size;
	}
	
	vfs.fechar(arq);
}

int Multiproc::buscar_floating_point_struc(MP_FLOAT_POINT * mpfloatpoint)
{
	char * ptr = POS_INICIAL_KERNEL + 639*1024;
	char * ptr_final = POS_INICIAL_KERNEL + 640*1024;
	char encontrado = 0;
	char cont = 0;
	
	do
	{
		//buscando n� ultimo kb do mem�ria base
		for(; ptr < ptr_final; ptr+=16)
		{
			/*
				procurando pela string _MP_ 
				� assinatura a Floating Point Structure
			*/
			if(ptr[0] == '_')
			{
				int arq;
				
				if(ptr[1] == 'M' 
				   &&  ptr[2] == 'P'
				   && ptr[3] == '_')
				{
					vfs.abrir("/dev/mp",'a', &arq);
					vfs.escrever(arq, ptr, 4);
					
					memcpy((char *)mpfloatpoint , ptr, sizeof(MP_FLOAT_POINT));
					
					
					vfs.escrever(arq, "\n", 1);
					vfs.escrever(arq, itoa((int)mpfloatpoint->config_ptr -100, 10), 30);
					
					MP_CONFIG_TABLE * mpconfig =  (MP_CONFIG_TABLE *) (mpfloatpoint->config_ptr + POS_INICIAL_KERNEL);
					
					vfs.escrever(arq, "\n", 1);
					vfs.escrever(arq, mpconfig->assinatura, 4);
					
						vfs.fechar(arq);
				
					encontrado = 1;
					break;
				}
			}
		}
		
		/* caso a estrutura n�o tenha sido encontrada 
		na mem�ria base buscar na mem�ria ROM da BIOS */
		if(!encontrado)
		{
			ptr = 0x0F0000;
			ptr_final = 0x0FFFFF;
		}
		else
		{
			break;
		}
		
		cont++;
		
	}while(cont < 2);
	
	return encontrado;
}

void Multiproc::analisar_MPConfigTable(char * ptr)
{
	MP_CONFIG_TABLE * mpconfig =  (MP_CONFIG_TABLE *) (ptr + POS_INICIAL_KERNEL);
	
	if( mpconfig->assinatura[0] == 'P'
		&& mpconfig->assinatura[1] == 'C'
		&& mpconfig->assinatura[2] == 'M'
		&& mpconfig->assinatura[3] == 'P')
	{
		//obtendo posi��o da primeira entrada na tabela
		char * entrada = (char *)ptr + POS_INICIAL_KERNEL + sizeof(MP_CONFIG_TABLE);
		
		//varrendo todos os itens da tabela
		for(int i=0; i < mpconfig->qtd_itens; i++)
		{
			//se o primeiro valor da entrada for 0, ent�o a entrada se refere a um processador
			if(entrada[0] == 0 )
			{
				MP_CONFIG_TABLE_PROC p;
				memcpy((char *)&p, entrada, sizeof(MP_CONFIG_TABLE_PROC));
				processadores.adicionar(p);
				
				entrada+=20;
			}
			//se o primeiro valor da entrada for 2, ent�o a entrada se refere a um IOAPIC
			else if(entrada[0] == 2 )
			{
			
				MP_CONFIG_TABLE_IOAPIC ioapic;
				memcpy((char *)&ioapic, entrada, sizeof(MP_CONFIG_TABLE_IOAPIC));
				ioapics.adicionar(ioapic);

				entrada+=8;
			}
			else if(entrada[0] == 3)
			{
				MP_CONFIG_INTERRUPT interrupt;
				memcpy((char *)&interrupt, entrada, sizeof(MP_CONFIG_INTERRUPT));
				interrupcoes.adicionar(interrupt);
				
				entrada+=8;
			}
			else //sen�o, qualquer outra entrada
			{
				entrada+=8;
			}
			
		} 

	} 
	
}

unsigned int Multiproc::ler_reg_apic(int registrador)
{	
	int * reg = (int)apic + registrador;	
	return reg[0];
}

void Multiproc::escrever_reg_apic(int registrador, int dado)
{
	int * reg = (int)apic + registrador;
	reg[0]	= dado;	
}

void Multiproc::escrever_reg_ioapic(int reg, int dado)
{
   volatile unsigned int * ioapic_ptr = (volatile unsigned int *)ioapic;
   ioapic_ptr[0] = (reg & 0xff);
   ioapic_ptr[4] = dado;
}

unsigned int Multiproc::ler_reg_ioapic(int reg)
{
   //colocando o endere�o do registrador no ioregsel
   volatile unsigned int * ioapic_ptr = (volatile unsigned int *)ioapic;
   ioapic_ptr[0] = (reg & 0xff);
  
   //lendo iowin
   return ioapic_ptr[4];
}

void Multiproc::enviar_eoi_apic()
{
     if(multiproc_habilitado)
	 {
		escrever_reg_apic(0x0B0, 0x00);
	 }
}

int Multiproc::obter_apicid()
{	
	if(multiproc_habilitado)
	{
		return (ler_reg_apic(0x20) >> 24) & 0x0f;
	}
	else
	{
		return 0;
	}
}

	

//FUN��O TRAMPOLIM UTILIZADA PARA CONFIGURAR OS APs (OUTROS PROCESSADORES)
void trampolim()
{
	//desabilitando interrup��es
	__asm__ __volatile__ ("cli");
	
	//habilitando pagina��o. colocando endere�o da Page directory no registrador cr3
	asm volatile("mov %0, %%cr3":: "b"(*((unsigned int*)0x7D008) - POS_INICIAL_KERNEL));

	//recarregando o valor do registrador cr0 para ativar a pagina��o
	unsigned int cr0;
	asm volatile("mov %%cr0, %0": "=b"(cr0));
	cr0 |= 0x80000000;
	asm volatile("mov %0, %%cr0":: "b"(cr0)); 
 
	//configurando iopl para 1
	int val;
    asm volatile("pushf"); 						//Empilhando conte�do de EFLAGS.
	asm volatile("pop %eax"); 					//Salvando em EAX.
	asm volatile("movl %%eax, %0" : "=r"(val)); //Salvando valor de EAX em val.
	val = val | 0x1000; 						//Setando o 12� bits.
    asm volatile("push %0" :: "r" (val)); 		//Empilhando VAL.
	asm volatile("popf");						//Alterando o valor de EFLAGS.

	//carrega gdt original
	carregar_gdt();
	
	//Alterando o registrador que armazena a posi��o do TSS
	asm volatile(" mov %0, %%eax; ltr %%ax"::"r"(0x38+multiproc.obter_apicid()*0x08)); //asm volatile(" mov $0x2B, %ax; ltr %ax");
	
	//carrega idt
	carregar_idt();	

	//habilitando apic do processador
	multiproc.habilitar_apic();
	
	//habilitando o apic timer	
	multiproc.habilitar_apic_timer();
	
	//configurando pilha
	unsigned int esp;
	asm volatile("mov %%esp, %0":"=r"(esp)); 
	asm volatile("mov %0, %%esp"::"r"(esp + POS_INICIAL_KERNEL - multiproc.obter_apicid()*512)); 
	asm volatile("mov %0, %%ebp"::"r"(esp + POS_INICIAL_KERNEL - multiproc.obter_apicid()*512)); 
	
	//incrementando contagem de processadores iniciados
	multiproc.processadores_iniciados++;
	
	//habilitando interrup��es
	asm volatile("sti");
	
	//aguardando interrup��o
	for(;;)
	{
		asm("hlt");
	} 
}

