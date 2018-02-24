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
#include "include/Sistema.h"
#include "include/Util.h"
#include "include/Lista.h"
#include "include/Io.h"

#define MAX_FREQ 1193180

struct Bloco_Timer
{
	int codigo;
	int pid;
	int param1;
	int param2;
};

struct Relogio
{
	unsigned int hora;
	unsigned int min;
	unsigned int seg;
	unsigned int ms;
	unsigned int ms_dia;
	int bcd;
	int hora24;
	
} relogio;

Lista<Bloco_Timer> solicitacoes;
int contador_atual = 0;

void atualizar_relogio()
{
	relogio.ms += 55;
	relogio.ms_dia += 55; 
	
	//caso 1000 ms tenham sido contados, incrementa 1 seg
	if(relogio.ms >= 1000)
	{
		relogio.ms = relogio.ms - 1000;
		relogio.seg++;
	}
	
	//caso 60s tenham sido contados, incrementa 1 min
	if(relogio.seg == 60)
	{
		relogio.seg = 0;
		relogio.min++;
	}
	
	//caso 60min tenham sido contados, incrementa 1h
	if(relogio.min == 60)
	{
		relogio.min = 0;
		relogio.hora++;
	}
	
	//caso 24h tenham sido contadas, zerar relógio
	if(relogio.hora == 24)
	{
		relogio.hora = 0;
		relogio.ms_dia = 0; 
	}
	
}

void obter_hora_cmos()
{
	//esperando relógio começar a ser atualizado
	while(true)
	{
		outportb(0x70, (0x01 << 7) | (0x0A));
		int reg = inportb(0x71);
		
		//verificando se o bit 7 (atualizando em progresso) foi setado
		if(reg & 0x80)
			break;
	}
	
	//esperando relógio terminar atualização
	while(true)
	{
		outportb(0x70, (0x01 << 7) | (0x0A));
		int reg = inportb(0x71);
		
		//verificando se o bit 7 (atualizando em progresso) foi setado
		if(!(reg & 0x80))
			break;
	}
	
	//lendo registrador 0x00 para segundos
	outportb(0x70, (0x01 << 7) | (0x00));
	relogio.seg = inportb(0x71);
	
	//lendo registrador 0x00 para minutos
	outportb(0x70, (0x01 << 7) | (0x02));
	relogio.min = inportb(0x71);
	
	//lendo registrador 0x00 para horas
	outportb(0x70, (0x01 << 7) | (0x04));
	relogio.hora = inportb(0x71);
	
	//lendo registrador 0x0b (status register b)
	outportb(0x70, (0x01 << 7) | (0x0B));
	int regb = inportb(0x71);
	
	//se bit 2 estiver setado, o relógio está no modo 24h
	relogio.hora24 = regb & 0x02;
	
	//se o bit 4 estiver setado os dados estão em binário, 
	//senão esntão em BCD
	relogio.bcd = !(regb & 0x04);
	relogio.ms = 0;
	
	//Caso os dados estejam em bcd, converter para binário
	if(relogio.bcd)
	{
		relogio.seg   =  ((relogio.seg/16)  * 10) + (relogio.seg  & 0xf);
		relogio.min   =  ((relogio.min/16)  * 10) + (relogio.min  & 0xf);
		relogio.hora  =  ((relogio.hora/16) * 10) + (relogio.hora & 0xf);
	}
	
	//calculando a quantidade de milisegundos do dia 1000 ms por seg, 
	//60000ms por minuto e 3.6 mi de ms por hora
	relogio.ms_dia = relogio.seg*1000 + relogio.min*60000 + relogio.hora*3600000; 
	
	//finalizando thread;
	sair();
}

//inicializar funções do driver
void tim_inicializar()
{
    //A frequência máxima do PIT é 119 MHz.
    //Para obter outras frequencias deve-se dividir esse valor e armazenar na porta 0x40. 
	//Divindo por 0xFFFF para obter uma interrupção a cada 55 ms
	int divisor = 0xFFFF;
	 
    //Enviando o MSB e LSB para a porta 0x40, especificando a frequência com que serão geradas as interrupções
    outportb(0x43, 0x34);               
    outportb(0x40, divisor & 0xFF);      
    outportb(0x40, divisor >> 8);   
}

void tim_obter_valor_pit(Bloco_Timer* b)
{
	unsigned char lb;//low
	unsigned char hb;//high
	unsigned int val;
	double retorno;
	char resp[100];
	
	outportb(0x43, 0x00);
	lb = inportb(0x40);
	hb = inportb(0x40);
	val = (hb << 8) | lb;
	retorno = val;
	
	
	memcpy(resp, (char *)&retorno, sizeof(double));
	enviar_msg_pid(b->pid, resp);
}

void tim_tratar_solicitacao(char * msg)
{
	Bloco_Timer * b = (Bloco_Timer*) msg;
	
	if(b->codigo == 1)
	{
		solicitacoes.adicionar(*b);
	}
	else if(b->codigo == 2)
	{
		char resp[100];
		memcpy(resp, (char *)&relogio, sizeof(Relogio));
		enviar_msg_pid(b->pid, resp);
	}
	else if(b->codigo == 3)
	{
		tim_obter_valor_pit(b);
	}
	
}

void tim_tratar_irq()
{	
	  atualizar_relogio();
	  
      Lista<int> proc_a_acordar;
	  for(int i =0; i <solicitacoes.tamanho(); i++)
	  {
		 //obtendo processo
		 Bloco_Timer &b = solicitacoes[i];
		 
		 //decrementando contador
		 b.param1-=55;
		 
		 if(b.param1 <= 0)
		 {
			proc_a_acordar.adicionar_em(0, i);
		 }
		 
	  }
	  
	  char msg[100] = "acordar";
	  
	  //acordando processos
	  for(int i =0; i < proc_a_acordar.tamanho(); i++)
	  {
		 int pos = proc_a_acordar[i];
		 Bloco_Timer b = solicitacoes[pos];
		 enviar_msg_pid(b.pid, msg);
		 solicitacoes.remover(pos);
	  }
	  
	  proc_a_acordar.limpar();
}

main(int argc, char * args[])
{

	unsigned char mensagem[100];
	
	//escutar porta 0 (IRQ do timer)
	escutar_porta(0);

	//escutar porta 81 comunicação com clientes
	escutar_porta(81);
	
	//inicializar timer
	tim_inicializar();
	
	//criar thread()
	criar_thread(obter_hora_cmos);
	
	//laço de repetição do driver
	while(TRUE)
	{
		//aguardar
		int tipo = receber_msg(mensagem, MSG_QLQR_PORTA);
		
		if(tipo == MSG_IRQ)
		{
			tim_tratar_irq();
		}
		else 
		{
			tim_tratar_solicitacao(mensagem);
		}
	
	}
}