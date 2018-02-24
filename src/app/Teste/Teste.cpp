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
#include "include/Ide.h"

const int TAM = 1024;

int A[TAM];
int B[TAM];
int C[TAM];

struct TBloco
{
	int pid;
	int ini;
	int fim;
};


void thread_teste()
{
	char msg[100];
	memcpy(msg, "MENSAGEM TESTE", strlen("MENSAGEM TESTE")+1);
	enviar_receber_msg(1001, msg);
	
	sair();
}

	
//------------processo para testes
main(int argc, char ** args)
{
	unsigned char tecla, msg[100];
	unsigned int cont = 0;
	int pid = obter_pid();

	//TESTE das CHAMADAS DO SISTEMA
	printf("Testando SBK   ");
	int topo  = sbk(4096);
	int topo2 = sbk(4096);
	
	if(topo == (topo2-4096))
	{
		alterar_cor(VID_COLOR_VERDE ,VID_COLOR_PRETO);
		printf("ok\n");
		alterar_cor(VID_COLOR_BRANCO ,VID_COLOR_PRETO);
	}
	else
	{
		alterar_cor(VID_COLOR_VERMELHO ,VID_COLOR_PRETO);
		printf("erro\n");
		alterar_cor(VID_COLOR_BRANCO ,VID_COLOR_PRETO);
	}
	
	escutar_porta(1001);
	printf("Testando Criacao de Threads ");
	
	
	int pid_thread = criar_thread(thread_teste);

	if(pid_thread != 0)
	{
		alterar_cor(VID_COLOR_VERDE ,VID_COLOR_PRETO);
		printf("ok\n");
		alterar_cor(VID_COLOR_BRANCO ,VID_COLOR_PRETO);
	}
	else
	{
		alterar_cor(VID_COLOR_VERMELHO ,VID_COLOR_PRETO);
		printf("erro\n");
		alterar_cor(VID_COLOR_BRANCO ,VID_COLOR_PRETO);
	}
	
	printf("Testando comunicação entre processos  ");
	receber_msg(msg, 1001);
	
	if(!strcmp(msg,"MENSAGEM TESTE"))
	{
		alterar_cor(VID_COLOR_VERDE ,VID_COLOR_PRETO);
		printf("ok\n");
		alterar_cor(VID_COLOR_BRANCO ,VID_COLOR_PRETO);
	}
	else
	{
		alterar_cor(VID_COLOR_VERMELHO ,VID_COLOR_PRETO);
		printf("erro\n");
		alterar_cor(VID_COLOR_BRANCO ,VID_COLOR_PRETO);
	}
	
	printf("Testando VFS ");
	int erro = 0, descritor = abrir("/arquivoteste",'a');
	char buf[10];
	escrever(descritor, "A", 1);
	fechar(descritor);
	
	descritor = abrir("/arquivoteste",'A');
	escrever(descritor, "A", 1);
	fechar(descritor);
	
	descritor = abrir("/arquivoteste",'A');
	ler(descritor, buf, 2);
	fechar(descritor);
	
	if(buf[0] != 'A' || buf[1] != 'A')
	{
		erro = 1;
	}
	
	buf[0] = ' ';
	buf[1] = ' ';
	
	descritor = abrir("/arquivoteste",'a');
	ler(descritor, buf, 2);
	fechar(descritor);
	
	if(buf[0] != ' ' || buf[1] != ' ')
	{
		erro = 1;
	}
	
	descritor = abrir("/arquivoteste",'a');
	excluir(descritor);
	descritor = abrir("/arquivoteste",'b');
	
	if(descritor != 0)
	{
		erro = 1;
	}
	
	if(erro)
	{
		alterar_cor(VID_COLOR_VERDE ,VID_COLOR_PRETO);
		printf("ok\n");
		alterar_cor(VID_COLOR_BRANCO ,VID_COLOR_PRETO);
	}
	else
	{
		alterar_cor(VID_COLOR_VERMELHO ,VID_COLOR_PRETO);
		printf("erro\n");
		alterar_cor(VID_COLOR_BRANCO ,VID_COLOR_PRETO);
	}
	
	printf("Pressione qualquer tecla para finalizar\n");
	getchar(&tecla);
	sair();
	
}//fim da main
