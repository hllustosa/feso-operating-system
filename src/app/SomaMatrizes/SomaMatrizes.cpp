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


//Função para calcula a soma de duas matrizes
void calcular()
{
	int pid = obter_pid();
	TBloco b;
	b.pid = pid;
	char msg[100];
	
	memcpy(msg, (char *)&b, sizeof(TBloco));
	enviar_receber_msg(1001, msg);
	memcpy((char *)&b, msg, sizeof(TBloco));
	
	for(int i = b.ini; i < b.fim; i++)
	{
		for(int j = 0; j < 10000000; j++);
		C[i] = B[i] + A[i];
	}
	
	enviar_msg(1001, msg);

	sair();
}

	
//------------processo para testes
main(int argc, char ** args)
{
	unsigned char tecla, msg[100];
	unsigned int cont = 0;
	int pid = obter_pid();
	
	Relogio r;
	
	int t0;
	obter_relogio(r);
	t0 = r.ms_dia;
	
	if(argc == 1)
	{
		escutar_porta(1001);
		int qtd_threads =  atoi(args[0]);
	
		for(int i =0; i < TAM; i++)
			A[i] = B[i] = i;
	
		printf("%d\n", qtd_threads);
		
		for(int i =0;i<qtd_threads; i++)
		{
			int t = criar_thread(calcular);
			receber_msg(msg,MSG_QLQR_PORTA);
			
			TBloco b;
			b.ini = i*TAM/qtd_threads;
			b.fim = b.ini + TAM/qtd_threads; 
	
			printf("recebeu %d\n", t);
			memcpy(msg, (char *)&b, sizeof(TBloco));
			
			enviar_msg_pid(t, msg);
			
		}
		
		
		for(int i =0;i<qtd_threads; i++)
			receber_msg(msg, MSG_QLQR_PORTA);
	
	
		obter_relogio(r);
		int t1 = r.ms_dia;
		
		int desc = abrir("/resultado",'a');
		
	    for(int i = 0; i < TAM; i++)
		{
			escrever(desc, itoa(C[i],10), strlen(itoa(C[i],10)));
			escrever(desc, "\n", 1);
		}
		
		escrever(desc, "\n", 1);
		escrever(desc, itoa(t1-t0,10), strlen(itoa(t1-t0,10)));
		fechar(desc);

	}
	
	sair();

}//fim da main

