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

const int TAM = 1024;//tamanho da matriz quadrada
int QUANT_THREADS = 2; // numero de threads geradas
int m_a[1024][1024];
int m_b[1024][1024];
int m_final[1024][1024];

void paralelo(){
	unsigned char thread_data[100];
	int thread_num, inicio_matriz, fim_matriz,i,j,k;
	receber_msg(thread_data, MSG_QLQR_PORTA);
	
	thread_num = thread_data[0];
	inicio_matriz = thread_num*(TAM/QUANT_THREADS);
	/*
		se esta for a thread final, passo todos os valores restante para ela
		pois a divisao pode nao ser inteira
	*/
	if (thread_num == QUANT_THREADS-1)
		fim_matriz = TAM;
	else
		fim_matriz = (inicio_matriz + (TAM/QUANT_THREADS)) -1;
	
	for(i=inicio_matriz;i<fim_matriz;i++)
	{
		for (j=0;j<TAM;j++)
		{
			for (k=0;k<TAM;k++)
			{
				m_final[i][j] += m_a[i][k]*m_b[j][k]; 
			}
		}
	}
	enviar_msg_pid(thread_data[1], thread_data);
	
	sair();
}
	
main(int argc, char ** args)
{
	int i,j,k;
	unsigned char thread_data[100];
	
	if(argc == 1)
	{
		QUANT_THREADS =  atoi(args[0]);
	
		//preenchendo a matriz
		for (i=0;i<TAM;i++)
		{
			for (j=0;j<TAM;j++)
			{
				m_a[i][j] = i+j;
				m_b[i][j] = i+j-3;
			}
		}
		Relogio r;
		obter_relogio(r);
		int t_inicial = r.ms_dia;
		
		//paralelizando
		int threads, pid;
		for(threads = 0; threads<QUANT_THREADS;threads++)
		{
			printf("Thread: %d\n", threads);
			printf("inicio da matriz: %d\n", threads*(TAM/QUANT_THREADS));
			printf("fim da matriz: %d\n", (threads*(TAM/QUANT_THREADS)+(TAM/QUANT_THREADS))-1);
			printf("\n\n");
			thread_data[0] = threads; // para saber a posicao na matriz dentro da thread
			thread_data[1] = obter_pid();
			pid = criar_thread(paralelo);
			enviar_msg_pid(pid, thread_data);
		}
		//para garantir que as threads terminaram antes de continuar
		for (threads = 0;threads < QUANT_THREADS;threads++)
			receber_msg(thread_data, MSG_QLQR_PORTA);
		
		//fim paralelizacao
		
		
		
		obter_relogio(r);
		int t_final = r.ms_dia;
		
		printf("Tempo final %d ms", (t_final - t_inicial));
		
		
		//impressao da matriz para conferencia, utlizar com matrizes pequenas
		/*for (i=0;i<TAM;i++)
		{
			for(j=0;j<TAM;j++)
			{
				printf("%d ", m_final[i][j]);
			}
			printf("\n");
		}*/
	
	}
	
	//travando a execucao
	unsigned char tecla;
	getchar(&tecla);
	sair();
}

