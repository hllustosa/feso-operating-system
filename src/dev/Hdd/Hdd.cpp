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
#include "include/Sistimer.h"

#define TIMEOUT 1000000

#define INVALIDO -1
#define ATA 	  0
#define ATAPI 	  1

//Tamanho padrão para um setor de um driver de CD-ROM 
#define ATAPI_TAM_SETOR 2048
#define ATA_TAM_SETOR 512

//IRQ padrão para os drivers IDE primário e secundário
#define ATA_IRQ_PRIM        0x0E
#define ATA_IRQ_SEC         0x0F

// As portas de E/S 
#define ATA_DADOS(x)         (x)
#define ATA_FEATURES(x)     (x+1)
#define ATA_SETOR_COUNT(x)  (x+2)
#define ATA_END1(x)		    (x+3)
#define ATA_END2(x)         (x+4)
#define ATA_END3(x)         (x+5)
#define ATA_DRIVE_SEL(x)    (x+6)
#define ATA_CMD(x)          (x+7)
#define ATA_DCR(x)          (x+0x206)   

//Valores válidos para o BUS da função de leituras de setores em um driver ATAPI
#define ATA_BUS_PRIM     0x1F0
#define ATA_BUS_SEC      0x170

//Valores válidos para  Driver da função de leituras de setores em um driver ATAPI
#define ATA_DRIVE_MASTER    0xA0
#define ATA_DRIVE_SLAVE     0xB0

//Macro para Delay de 400 ns necessário para a seleção do driver ATA/ATAPI 
#define ATA_SEL_DELAY(bus) \
{inportb(ATA_DCR(bus));inportb(ATA_DCR(bus));inportb(ATA_DCR(bus));inportb(ATA_DCR(bus));}

//Macro para testar se o comando estorou o tempo
#define ATA_VERIFICAR_TIMEOUT(x) \
 if(x >= TIMEOUT) \
    {\
       return INVALIDO;\
    }\
    else\
    {\
        x = 0;\
    }



int ide_identificar(unsigned short * buf, unsigned int bus, unsigned int drive, unsigned int tipo);
void ide_inicializar();
void ide_adicionar_solicitacoes(char * msg);
void tratar_solicitacoes();
void aguardar_timeout();
int ide_selecionar_drive(unsigned int bus, unsigned int drive, unsigned int tam_setor);
int ide_ler_setor_atapi(unsigned int bus, unsigned int drive, unsigned int lba, unsigned char * buffer);
int ide_ler_setor_ata(unsigned int bus, unsigned int drive, unsigned int lba, unsigned char * buffer);
int ide_escrever_setor_ata(unsigned int bus, unsigned int drive, unsigned int lba, unsigned char * buffer);
 
struct Bloco_Ide
{
   int codigo;
   int pid;
   int lba;
   int param1;
   int param2;
   int param3;
   unsigned char arquivo[50]; 
};

Lista<Bloco_Ide> solicitacoes;

int driver_ata;
int bus_ata;
int driver_atapi;
int bus_atapi;
volatile int timeout;
volatile int em_espera;

int ide_identificar3(unsigned short * buf, unsigned int bus, unsigned int drive, unsigned int tipo)
{
   int ret = 0;

   outportb(ATA_DRIVE_SEL(bus), drive);
   ATA_SEL_DELAY(bus);//delay de 400ns
   
   outportb(ATA_SETOR_COUNT(bus),0);
   outportb(ATA_END1(bus),0);
   outportb(ATA_END2(bus),0);
   outportb(ATA_END3(bus),0);
   
   if(tipo == ATA)
   {
	   //comando para identificar drives ATA
	   outportb(ATA_CMD(bus),0xEC);
   }
   else if(tipo == ATAPI)
   {
	   //comando para identificar drives ATAPI
	   outportb(ATA_CMD(bus),0xA1);
   }
   else
   {
		//parâmetros errados
		return ret;
   }
   
   //delay para seleção de novo drive
   ATA_SEL_DELAY(bus);
	
   //polling	
   for(unsigned int i = 0; i< 1000000; i++)
   {
		char val = inportb(ATA_CMD(bus));
		
		if(val == 0)
		{
			break;
		}
		
		if((val & 0x88) ==0)
		{
		   // travar
		   break;
		}
		
		if(val !=0x80)
		{
		   ret = 1;
		   // ler
		   break;
		}
   }		   
   
   //lendo dados
   if(ret)
   {
		inportsw(ATA_DADOS(bus), buf, 256);
   }
   
   inportb(ATA_CMD(bus));
   
   return ret;
   //identify(buf); 
}


int ide_identificar2(unsigned short * buffer, unsigned int bus, unsigned int drive, unsigned int tipo)
{
	int sucesso = 1, status;
	
	while ((status = inportb(ATA_CMD(bus))) & 0x80)    
    asm volatile("pause");
  
	//Selecionando o driver
	outportb(ATA_DRIVE_SEL(bus), 0xE0 | ((0 >> 24) & 0x0F));
	
	//delay de 400ns
	ATA_SEL_DELAY(bus);
	
	while ((status = inportb(ATA_CMD(bus))) & 0x80)    
	  asm volatile("pause");
	  
	//enviando endereço LBA (no caso 0)
	outportb(ATA_END1(bus),0);
	outportb(ATA_END2(bus),0);
	outportb(ATA_END3(bus),0);
		
	//configurando qtd de setores para 0
	outportb(ATA_SETOR_COUNT(bus), 0);

    if(tipo == ATA)
    {
	   //comando para identificar drives ATA
	   outportb(ATA_CMD(bus),0xEC);
    }
    else if(tipo == ATAPI)
    {
	   //comando para identificar drives ATAPI
	   outportb(ATA_CMD(bus),0xA1);
    }
    else
    {
		//parâmetros errados
		return 0;
    }
	
	//lendo Alternate Status Port
	inportb(ATA_DCR(bus));
	
	while ((status = inportb(ATA_CMD(bus))) & 0x80)    
     asm volatile ("pause");
  
    while (!((status = inportb(ATA_CMD(bus))) & 0x8) && !(status & 0x1))
     asm volatile ("pause");
	
	if(status & 0x1) 
	{
	  return 0;
	}
	
	//lendo dados
	inportsw(ATA_DADOS(bus), buffer, 256);
		
	//lendo Alternate Status Port
	inportb(ATA_DCR(bus));
	
	//lendo o STATUS register para não receber interrupções	
	inportb(ATA_CMD(bus));
	
	
	return 1;
}



int ide_identificar(unsigned short * buffer, unsigned int bus, unsigned int drive, unsigned int tipo)
{
	int  cont = 0, status;
	
	while ((status = inportb(ATA_CMD(bus))) & 0x80 && ((cont++) < TIMEOUT ))    
      asm volatile ("pause");
  
    ATA_VERIFICAR_TIMEOUT(cont);
    
	//Selecionando o driver
	outportb(ATA_DRIVE_SEL(bus), 0xE0 | ((0 >> 24) & 0x0F));
	
	//delay de 400ns
	ATA_SEL_DELAY(bus);
	
	while ((status = inportb(ATA_CMD(bus))) & 0x80 && ((cont++) < TIMEOUT ))    
	  asm volatile ("pause");
	  
	ATA_VERIFICAR_TIMEOUT(cont);  
	  
	//enviando endereço LBA (no caso 0)
	outportb(ATA_END1(bus),0);
	outportb(ATA_END2(bus),0);
	outportb(ATA_END3(bus),0);
		
	//configurando qtd de setores para 0
	outportb(ATA_SETOR_COUNT(bus), 0);

    if(tipo == ATA)
    {
	   //comando para identificar drives ATA
	   outportb(ATA_CMD(bus),0xEC);
    }
    else if(tipo == ATAPI)
    {
	   //comando para identificar drives ATAPI
	   outportb(ATA_CMD(bus),0xA1);
    }
    else
    {
		//parâmetros errados
		return INVALIDO;
    }
	
	//lendo Alternate Status Port
	inportb(ATA_DCR(bus));
	
	if(inportb(ATA_CMD(bus)) == 0x00)
	{
          return INVALIDO;
    }
    
    if(tipo == ATA)
    {
            if( inportb(ATA_END2(bus))  != 0x00 ||
                 inportb(ATA_END3(bus)) != 0x00)
	        {
                return INVALIDO;
            }
    }
	
	while ((status = inportb(ATA_CMD(bus))) & 0x80 && ((cont++) < TIMEOUT ))    
     asm volatile ("pause");
  
 	ATA_VERIFICAR_TIMEOUT(cont);   
  
    while (!((status = inportb(ATA_CMD(bus))) & 0x8) && !(status & 0x1) && ((cont++) < TIMEOUT ))
     asm volatile ("pause");
	
	ATA_VERIFICAR_TIMEOUT(cont);
	
	if(status & 0x1) 
	{
	  return INVALIDO;
	}
	
	//lendo dados
	inportsw(ATA_DADOS(bus), buffer, 256);
		
	//lendo Alternate Status Port
	inportb(ATA_DCR(bus));
	
	//lendo o STATUS register para não receber interrupções	
	inportb(ATA_CMD(bus));
	
	
	return tipo;
}

unsigned short info_identificar[8][256];

void ide_inicializar()
{
  timeout	   = 0x00;	
  driver_ata   = 0x00;
  bus_ata	   = 0x00;
  driver_atapi = 0x00;
  bus_atapi    = 0x00;
  
  if(ide_identificar(info_identificar[0], ATA_BUS_PRIM, ATA_DRIVE_MASTER, ATA) == ATA)
  {
	 bus_ata = ATA_BUS_PRIM;
	 driver_ata = ATA_DRIVE_MASTER;
  }
  else if(ide_identificar(info_identificar[1], ATA_BUS_PRIM, ATA_DRIVE_SLAVE, ATA) == ATA)
  {
	 bus_ata = ATA_BUS_PRIM;
	 driver_ata = ATA_DRIVE_SLAVE;
  }
  else if(ide_identificar(info_identificar[2], ATA_BUS_SEC, ATA_DRIVE_MASTER, ATA) == ATA)
  {
	 bus_ata = ATA_BUS_SEC;
	 driver_ata = ATA_DRIVE_MASTER;
  }
  else if(ide_identificar(info_identificar[3], ATA_BUS_SEC, ATA_DRIVE_SLAVE, ATA) == ATA)
  {
	 bus_ata = ATA_BUS_SEC;
	 driver_ata = ATA_DRIVE_SLAVE;
  } 
  
  //identificando drives ATAPI
  if(ide_identificar(info_identificar[4], ATA_BUS_PRIM, ATA_DRIVE_MASTER, ATAPI) == ATAPI)
  {
	 bus_atapi 	   = ATA_BUS_PRIM;
     driver_atapi  = ATA_DRIVE_MASTER; 
  }
  else if(ide_identificar(info_identificar[5], ATA_BUS_PRIM, ATA_DRIVE_SLAVE,  ATAPI) == ATAPI)
  {
	 bus_atapi 	   = ATA_BUS_PRIM;
     driver_atapi  = ATA_DRIVE_SLAVE; 
  }
  else if(ide_identificar(info_identificar[6], ATA_BUS_SEC, ATA_DRIVE_MASTER, ATAPI) == ATAPI)
  {
	 bus_atapi 	   = ATA_BUS_SEC;
     driver_atapi  = ATA_DRIVE_MASTER;
  }
  else if(ide_identificar(info_identificar[7], ATA_BUS_SEC, ATA_DRIVE_SLAVE,  ATAPI) == ATAPI)
  {
	 bus_atapi 	   = ATA_BUS_SEC;
     driver_atapi  = ATA_DRIVE_SLAVE;
  } 
 
  //escutando porta 82 para solicitações de clientes
  escutar_porta(82);
  
  //dados;
  int arq = abrir("/dev/ide",'a'); 
  escrever(arq, itoa(bus_ata,16),10 );	
  escrever(arq, "\n",1 );	
  escrever(arq, itoa(driver_ata,16),10 );	
  escrever(arq, "\n",1 );	
  escrever(arq, itoa(bus_atapi,16),10 );	
  escrever(arq, "\n",1 );	
  escrever(arq, itoa(driver_atapi,16),10 );	
  escrever(arq, "\n",1 );	
  fechar(arq);
}

void ide_adicionar_solicitacoes(char * msg)
{
	//convertendo mensagem para bloco
	Bloco_Ide * b = (Bloco_Ide *) msg;
	
	//adicionando a lista de solicitações
	solicitacoes.adicionar(*b);
}

void tratar_solicitacoes()
{
    int serv;
	char msg[100] = "sucesso";
		
	//enquanto houverem solicitações pendentes
	while(solicitacoes.tamanho() > 0)
	{
		Bloco_Ide &b = solicitacoes[0];
		
		
		//codigo = 0, ler dados ATA
		if(b.codigo == 0 && driver_ata != 0x00)
		{
			unsigned char dados[ATA_TAM_SETOR];
			while(!ide_ler_setor_ata(bus_ata, driver_ata, b.lba, dados));
			int arq = _abrir(b.arquivo, 'A', &serv);
			escrever(arq, dados, ATA_TAM_SETOR);
			_fechar(arq);
			enviar_msg_pid(b.pid, msg);
		}
		//codigo = 1, escrever dados ATA
		else if(b.codigo == 1 && driver_ata != 0x00)
		{
			unsigned char dados[ATA_TAM_SETOR];
			int arq = _abrir(b.arquivo, 'A', &serv);
			ler(arq, dados, ATA_TAM_SETOR);
			_fechar(arq);
			while(!ide_escrever_setor_ata(bus_ata, driver_ata, b.lba, dados));
			enviar_msg_pid(b.pid, msg);
		}
		//codigo = 2, ler dados ATAPI
		else if(b.codigo == 2 && driver_atapi != 0x00)
		{
			unsigned char dados[ATAPI_TAM_SETOR];
			while(!ide_ler_setor_atapi(bus_atapi, driver_atapi, b.lba, dados));
			int arq = _abrir(b.arquivo, 'A',&serv);
			escrever(arq, dados, ATAPI_TAM_SETOR);
			_fechar(arq);
			enviar_msg_pid(b.pid, msg);
		}
		//código 3, comando identificar
		else if(b.codigo == 3)
		{
			unsigned char dados[ATA_TAM_SETOR];
			unsigned int bus, drive, tipo;
			
			if(b.param1)
			{
			   bus = ATA_BUS_PRIM;
			}
			else
			{
			   bus = ATA_BUS_SEC;
			}
			
			if(b.param2)
			{
			   drive = ATA_DRIVE_MASTER;
			}
			else
			{
			   drive = ATA_DRIVE_SLAVE;
			}
			
			if(b.param3)
			{
				tipo = ATA;
			}
			else
			{
				tipo = ATAPI;
			}
			
			ide_identificar((unsigned short *)dados, bus, drive, tipo);

			int arq = _abrir(b.arquivo, 'A',&serv);
			escrever(arq, (unsigned char *)dados, ATA_TAM_SETOR);
			_fechar(arq);
			
			enviar_msg_pid(b.pid, msg);
		}
		
		//removendo solicitações após tratamento
		solicitacoes.remover(0);
	}
}

int ide_selecionar_drive(unsigned int bus, unsigned int drive, unsigned int tam_setor)
{
	int sucesso = 1, status, cont = 0;
	
	//Selecionando o driver
	outportb (ATA_DRIVE_SEL(bus), drive & (1 << 4));      
	
	//delay de 400ns
	ATA_SEL_DELAY(bus);
	
	//selecionando modo PIO
	outportb (ATA_FEATURES(bus), 0x00); 
	
	//colocando MSB e LSB do tamanho do setor
	outportb (ATA_END2(bus), tam_setor & 0xFF);
	outportb (ATA_END3(bus), tam_setor >> 8);
	
	//comando 0xA0, para selecionar o driver
	outportb (ATA_CMD(bus), 0xA0);  
	
	while ((status = inportb(ATA_CMD(bus))) & 0x80  && (!timeout) && ( (cont++) < TIMEOUT))    
	  asm volatile ("pause");
	
	if(cont >= TIMEOUT )
	{
		return 0;  
	}
	cont = 0;
	  
	while (!((status = inportb(ATA_CMD(bus))) & 0x8) && !(status & 0x1) && (!timeout) && ( (cont++) < TIMEOUT))
	  asm volatile ("pause");
	
	if(status & 0x1) 
	{
	  sucesso = 0;
	}
	
	if(cont >= TIMEOUT )
	{
		return 0;  
	}
	cont = 0;
	
	return sucesso;
}

//Lê um setor de um driver ATAPI (cd-rom)
int ide_ler_setor_atapi(unsigned int bus, unsigned int drive, unsigned int lba, unsigned char * buffer)
{   
	//0xA8 é o comando para leitura de setores
	unsigned char msg[100];
	unsigned char cmd_leitura[12] = { 0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned char status;
	unsigned int tamanho = 0, cont = 0, tipo;
	
	//selecionando drive
	if(ide_selecionar_drive(bus, drive, ATAPI_TAM_SETOR))
	{
		//criando comando de leitura
		cmd_leitura[9] = 1;                      
		cmd_leitura[2] = (lba >> 0x18) & 0xFF;   
		cmd_leitura[3] = (lba >> 0x10) & 0xFF;
		cmd_leitura[4] = (lba >> 0x08) & 0xFF;
		cmd_leitura[5] = (lba >> 0x00) & 0xFF;   
		
		//Envia o comando ATAPI
		outportsw(ATA_DADOS(bus), (unsigned short *)cmd_leitura, 6);
		
		while ((status = inportb(ATA_CMD(bus))) & 0x80 && ((cont++) < TIMEOUT))    
	      asm volatile ("pause");
		  
		if(cont >= TIMEOUT )
		{
			return 0;  
		}
		cont = 0;  
		
	    while (!((status = inportb(ATA_CMD(bus))) & 0x8) && !(status & 0x1) && ((cont++)< TIMEOUT))
	      asm volatile ("pause");
		  
		if(cont >= TIMEOUT )
		{
			return 0;  
		}
		cont = 0;  
		
		//Obtendo quantidade de bytes lidos
		tamanho =  (((int) inportb(ATA_END3(bus))) << 8) | (int) (inportb(ATA_END2(bus)));
		
		//lendo dados
		inportsw(ATA_DADOS(bus), buffer, tamanho/2);
		
		
		while ((status = inportb(ATA_CMD(bus))) & 0x88 && ( (cont++) < TIMEOUT));// asm volatile ("pause");
		
		if(cont >= TIMEOUT )
		{
			return 0;  
		}
	cont = 0;
	}

	return tamanho;
}

//método para ler setor ata
int ide_ler_setor_ata(unsigned int bus, unsigned int drive, unsigned int lba, unsigned char * buffer)
{
	/*
		1) Read the status register of the primary or the secondary IDE controller.
		2) The BSY and DRQ bits must be zero if the controller is ready.
		3) Set the DEV bit to 0 for Drive0 and to 1 for Drive1 on the selected IDE controller using the Device/Head register and wait for approximately 400 nanoseconds using some NOP perhaps.
		4) Read the status register again.
		5) The BSY and DRQ bits must be 0 again for you to know that the IDE controller and the selected IDE drive are ready.
		6) Write the LBA28 address to the designated IDE registers.
		7) Set the Sector count using the Sector Count register.
		8) Issue the Read Sector(s) command.
		9) Read the Error register. If the ABRT bit is set then the Read Sector(s) command is not supported for that IDE drive. If the ABRT bit is not set, continue to the next step.
		10) If you want to receive interrupts after reading each sector, clear the nIEN bit in the Device Control register. If you do not clear this bit then interrupts will not be generated after the reading of each sector which might cause an infinite loop if you are waiting for them. The Primary IDE Controller will generate IRQ14 and the secondary IDE controller generates IRQ 15.
		11) Read the Alternate Status Register (you may even ignore the value that is read)
		12) Read the Status register for the selected IDE Controller.
		13) Whenever a sector of data is ready to be read from the Data Register, the BSY bit in the status register will be set to 0 and DRQ to 1 so you might want to wait until those bits are set to the mentioned values before attempting to read from the drive.
		14) Read one sector from the IDE Controller 16-bits at a time using the IN or the INSW instructions.
		15) See if you have to read one more sector. If yes, repeat from step 11 again.
		16) If you don't need to read any more sectors, read the Alternate Status Register and ignore the byte that you read.
		17) Read the status register. When the status register is read, the IDE Controller will negate the INTRQ and you will not have pending IRQs waiting to be detected. This is a MUST to read the status register when you are done reading from IDE ports. 
	*/
	
	int sucesso = 1, status;

    while ((status = inportb(ATA_CMD(bus))) & 0x80)    
     asm volatile ("pause");
  
	//Selecionando o driver
	outportb(ATA_DRIVE_SEL(bus), 0xE0 | ((lba >> 24) & 0x0F));
	
	//delay de 400ns
	ATA_SEL_DELAY(bus);
	
	while ((status = inportb(ATA_CMD(bus))) & 0x80)    
	  asm volatile ("pause");
	
	//enviando endereço LBA
	outportb(ATA_END1(bus),lba);
	outportb(ATA_END2(bus),lba >> 8);
	outportb(ATA_END3(bus),lba >> 16);
		
	//configurando qtd de setores para 1
	outportb(ATA_SETOR_COUNT(bus), 1);
		
	//enviando comando 0x20 (leitura)		
	outportb(ATA_CMD(bus), 0x20);
	
	//lendo Alternate Status Port
	inportb(ATA_DCR(bus));
	
	while ((status = inportb(ATA_CMD(bus))) & 0x80)    
     asm volatile ("pause");
  
    while (!((status = inportb(ATA_CMD(bus))) & 0x8) && !(status & 0x1))
     asm volatile ("pause");
	
	if(status & 0x1) 
	{
	  return 0;
	}
	
	//lendo dados
	inportsw(ATA_DADOS(bus), buffer, 256);
		
	//lendo Alternate Status Port
	inportb(ATA_DCR(bus));
	
	//lendo o STATUS register para não receber interrupções	
	inportb(ATA_CMD(bus));
	
	
	return 1;
		
}
    
//método para escrever em setor
int ide_escrever_setor_ata(unsigned int bus, unsigned int drive, unsigned int lba, unsigned char * buffer)
{ 
			/*
		1) Read the status register of the primary or the secondary IDE controller.
		2) The BSY and DRQ bits must be zero if the controller is ready.
		3) Set the DEV bit to 0 for Drive0 and to 1 for Drive1 on the selected IDE controller using the Device/Head register and wait for approximately 400 nanoseconds using some NOP perhaps.
		4) Read the status register again.
		5) The BSY and DRQ bits must be 0 again for you to know that the IDE controller and the selected IDE drive are ready.
		6) Write the LBA28 address to the designated IDE registers.
		7) Set the Sector count using the Sector Count register.
		8) Issue the Read Sector(s) command.
		9) Read the Error register. If the ABRT bit is set then the Read Sector(s) command is not supported for that IDE drive. If the ABRT bit is not set, continue to the next step.
		10) If you want to receive interrupts after reading each sector, clear the nIEN bit in the Device Control register. If you do not clear this bit then interrupts will not be generated after the reading of each sector which might cause an infinite loop if you are waiting for them. The Primary IDE Controller will generate IRQ14 and the secondary IDE controller generates IRQ 15.
		11) Read the Alternate Status Register (you may even ignore the value that is read)
		12) Read the Status register for the selected IDE Controller.
		13) Whenever a sector of data is ready to be read from the Data Register, the BSY bit in the status register will be set to 0 and DRQ to 1 so you might want to wait until those bits are set to the mentioned values before attempting to read from the drive.
		14) Read one sector from the IDE Controller 16-bits at a time using the IN or the INSW instructions.
		15) See if you have to read one more sector. If yes, repeat from step 11 again.
		16) If you don't need to read any more sectors, read the Alternate Status Register and ignore the byte that you read.
		17) Read the status register. When the status register is read, the IDE Controller will negate the INTRQ and you will not have pending IRQs waiting to be detected. This is a MUST to read the status register when you are done reading from IDE ports. 
	*/
	
	int sucesso = 1, status;

    while ((status = inportb(ATA_CMD(bus))) & 0x80)    
     asm volatile ("pause");
  
	//Selecionando o driver
	outportb(ATA_DRIVE_SEL(bus), 0xE0 | ((lba >> 24) & 0x0F));
	
	//delay de 400ns
	ATA_SEL_DELAY(bus);
	
	while ((status = inportb(ATA_CMD(bus))) & 0x80)    
	  asm volatile ("pause");
	 
	
	//enviando endereço LBA
	outportb(ATA_END1(bus),lba);
	outportb(ATA_END2(bus),lba >> 8);
	outportb(ATA_END3(bus),lba >> 16);
		
	//configurando qtd de setores para 1
	outportb(ATA_SETOR_COUNT(bus), 1);
		
	//enviando comando 0x30 (leitura)		
	outportb(ATA_CMD(bus), 0x30);
	
	//lendo Alternate Status Port
	inportb(ATA_DCR(bus));
	
	while ((status = inportb(ATA_CMD(bus))) & 0x80)    
     asm volatile ("pause");
  
    while (!((status = inportb(ATA_CMD(bus))) & 0x8) && !(status & 0x1))
     asm volatile ("pause");
	
	if(status & 0x1) 
	{
	  return 0;
	}
	
	
	//escrevendos dados
	//outportsw(ATA_DADOS(bus), buffer, 256); 

	for(int i =0; i < 256; i++)
	{
	    outportw(ATA_DADOS(bus), ((unsigned short *)buffer)[i]);
	}
		
	//lendo Alternate Status Port
	inportb(ATA_DCR(bus));	
	
	//lendo o STATUS register para não receber interrupções	
	inportb(ATA_CMD(bus));
	
	
	return 1;
}
  
unsigned char mensagem[100];
  
main(int argc, char * args[])
{
	//inicializando driver
	ide_inicializar();
	
	while(TRUE)
	{
		int tipo = receber_msg(mensagem, MSG_QLQR_PORTA);
		ide_adicionar_solicitacoes(mensagem);
		tratar_solicitacoes();
	}

}
