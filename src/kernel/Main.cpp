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
#include "include/Idt.h"
#include "include/Irq.h"
#include "include/Syscall.h"
#include "include/Memoria.h"
#include "include/Processo.h"
#include "include/Mensagem.h"
#include "include/Multiprocessamento.h"
#include "include/Loader.h"
#include "include/Util.h"
#include "include/Vfs.h"
#include "include/Multiboot.h"
#include "include/Testes.h"

void inicalizar_modulos(MULTIBOOT_INFO* mbd)
{
  char nome[25], nome_modulo[25];
  int pasta, arq, tam_cmd, pos, pos_param, pos_nome; 
  
  //criando diret�rio para armazenar m�dulos
  vfs.abrir("/modulos",'D', &pasta);
  vfs.fechar(pasta);
  
  //carregando m�dulos do grub
  mbd = (MULTIBOOT_INFO*)((unsigned int)mbd + POS_INICIAL_KERNEL);
  MULTIBOOT_MOD_LIST * modulos = (unsigned int)mbd->mods_addr + POS_INICIAL_KERNEL;
 
  //varrendo todos m�dulos carregados no GRUB (ver arquivo menu.lst na pasta do GRUB)
  for(int i =0; i < mbd->mods_count; i++)
  {
    unsigned char * imagem = (unsigned char *) (modulos[i].mod_start + POS_INICIAL_KERNEL);
	unsigned char * cmdline = (unsigned char *) ( (int)modulos[i].cmdline + POS_INICIAL_KERNEL);
	
	//calculando tamanho do par�metro
	tam_cmd = strlen(cmdline);
	
	//obtendo par�metro informado no arquivo menu.lst
	for(pos = tam_cmd - 1; pos >=0; pos--)
	{
		if(cmdline[pos] == '-')
		{
			cmdline[pos] = '\0';
			pos_param = pos + 1;
			break;
		}
		else if(cmdline[pos] == ' ')
		{
			cmdline[pos] = '\0';
		}
	}
	
	//obtendo nome do m�dulo
	for(pos = pos_param - 2; pos >=0; pos--)
	{
		if(cmdline[pos] == '/')
		{
			pos_nome = pos + 1;
			break;
		}
		else if(cmdline[pos] == ' ')
		{
			cmdline[pos] = '\0';
		}
	}
	
	//configurando nome do arquivo
	memset(nome, '\0', 25);
	memset(nome_modulo, '\0', 25);
	memcpy(nome,"/modulos/", strlen("/modulo/")+1);
	memcpy(nome_modulo, &cmdline[pos_nome], strlen(&cmdline[pos_nome])+1);
	strcat(nome, nome_modulo);
	
	//verificando se o m�dulo � um driver
	if(!strcmp(&cmdline[pos_param],"DRIVER"))
	{
		//criando processo servidor
		escalonador.adicionar_processo(nome_modulo, imagem, Processo::DRIVER, 0, NULL);
	}
	//verificando se o m�dulo � um aplicativo que deve ser inicializado com o SO
	else if(!strcmp(&cmdline[pos_param],"APP_INI"))
	{
		escalonador.adicionar_processo(nome_modulo, imagem, Processo::USUARIO, 0, NULL);
	}
	//verificando se o m�dulo � o c�digo trampolim
	else if(!strcmp(&cmdline[pos_param],"TRAMP"))
	{
		//carregando c�digo trampolim
		multiproc.carregar_codigo_trampolim(imagem);
	}
	//verificando se o m�dulo � o processo IDLE
	else if(!strcmp(&cmdline[pos_param],"IDLE"))
	{
		//configurando processo IDLE
		escalonador.adicionar_idle(imagem);
	}
	
	//criando arquivo para o m�dulo
	vfs.abrir(nome, 'a', &arq);
	
	//escrevendo dados no arquivo
	vfs.escrever(arq, imagem, (modulos[i].mod_end - modulos[i].mod_start));
	
	//fechando arquivo
	vfs.fechar(arq);
	
  }//fim do for
  
}

//FUN��O MAIN - PONTO DE PARTIDA DO SISTEMA OPERACIONAL
int main(MULTIBOOT_INFO* mbd, int end_fim_kernel, int bss)
{
   //inicializando controle de mem�ria f�sica
   mem_fisica.inicializar(mbd, end_fim_kernel);

   //inicializando controle da mem�ria virtual
   mem_virtual.inicializar();
  
   //instalando gdt
   gdt.inicializar(bss);
 
   //removendo mapeamento da primeira page table ap�s a GDT original ser instaladas		 
   mem_virtual.remover_entrada_inicial();

   //instalando idt
   idt.inicializar();
    
   //instalando irqs padr�o
   irq.inicializar(idt);
   
   //instalando systemcalls
   syscall.inicializar(idt);
   
   //inicializando sistema de troca de mensagens
   entregador.inicializar(irq);
 
   //inicializando sistema de arquivos virtual
   vfs.inicializar();
   
   //incializando escalonador de processos
   escalonador.inicializar();
  
   //incializando m�dulos do GRUB
   inicalizar_modulos(mbd);
	
   //testes de unidade	
   testes.inicializar();	
	
   //ativando o escalonador
   escalonador.escalonamento_ativo = 1;

   //inicializando detec��o de m�ltiplos processadores
   multiproc.inicializar();
  
   //habilitando interrup��es
   __asm__ __volatile__ ("sti");  
   
   //la�o de repeti��o at� a interrup��o para carregar novo processo
   for(;;);
}
