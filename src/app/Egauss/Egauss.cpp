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
#include "include/Primos.h"

const int TAM = 5;//ate 512 por enquanto
const int QUANT_THREADS = 1;
float a[5][6];
float x[5];
int teste;
int aux;

int rand (unsigned int limit){
	static long a = 3;
	a = (((a*214013L + 2531011L) >> 16) & 32768);
	return ((a % limit) + 1);
}

void paralelo(){
	int j,k,i, posicao_thread, limite_thread, incremento;
	float m;
	unsigned char thread_data[100];
	receber_msg(thread_data, MSG_QLQR_PORTA);
	
	posicao_thread = thread_data[1];
	k = thread_data[0]; 
	
	limite_thread = posicao_thread+(TAM / QUANT_THREADS);
		
	for (i=k+1; i < TAM; i++)
	{
		if (a[k][k] == 0)
		{
			a[k][k]++;
			i--;
		}
		else
		{
			m = a[i][k] / a[k][k];
			a[i][k] = 0;
			
			for(j=k+1;j<=TAM-1;j++)
			{
				a[i][j]=a[i][j]-(m*a[k][j]);
			}
			
			a[i][TAM]= a[i][TAM]-(m*a[k][TAM]);
		}
	}

	/*unsigned char msg[100];
	int pid = obter_pid();
	enviar_msg(1002, msg);
	//printf("Sou a thread: %d\n", obter_pid());
	
	unsigned char buffer[10] = "_Sou: ";
	strcat(buffer, itoa(obter_pid(), 10));
	enviar_msg(1002, buffer);*/
	enviar_msg_pid(thread_data[2], thread_data);
	
	sair();
}
	
main(int argc, char ** args)
{
	float m,s;
	int i, j, k, msg, pid_main_thread,z=0;
	double t_inicial, t_final, delta_t;
	int out =  abrir("/egauss.txt",'a');//arquivo 
	//receber_msg(msg,MSG_QLQR_PORTA);
	Relogio r;//relogio  ^^
	pid_main_thread = obter_pid();
	
	escrever(out, "Escrevendo os dados iniciais\n",strlen("Escrevendo os dados iniciais\n"));
	for(i=0; i<TAM; i++)//alterar para carregar de um arquivo
	{
		a[i][TAM]=0;
	   for(j=0; j<TAM; j++)
	   {
			obter_relogio(r);
			printf("%d\n", rand(8));
			a[i][j] = /*primos_512[z];//(i+j+1)*(i+1);*/(rand(r.ms_dia)%8 +1)*4;//alterar esta linha
			z++;
			/*teste= a[i][j];
			aux= teste%3;

			if(aux==0){
				a[i][j] = a[i][j] *(-1);
			}*/
			escrever(out, itoa(a[i][j],10),strlen(itoa(a[i][j],10)));
		}
		escrever(out, "\n",strlen("\n"));
	}
	escrever(out, "Fim da escrita",strlen("Fim da escrita\n\n"));
	
	
	for (i=0;i<TAM;i++)
	{
		for (j=0;j<TAM;j++)
		{
			a[i][TAM]= a[i][j]+ a[i][TAM];
		}
	}
	
	obter_relogio(r);
	t_inicial = r.ms_dia;	
	
	for (k=0;k<TAM-1;k++)
		{
			for (i=k+1; i < TAM; i++)
			{
				m = a[i][k] / a[k][k];
				a[i][k] = 0;
				
				for(j=k+1;j<=TAM-1;j++)
				{
					a[i][j]=a[i][j]-(m*a[k][j]);
				}
				
				a[i][TAM]= a[i][TAM]-(m*a[k][TAM]);
			}
		}
		
	obter_relogio(r);
	t_final = r.ms_dia;
	delta_t=t_final-t_inicial;
	printf("Fim da execucao!");
	
	// resolução
	char* string_buffer;
	int string_lenght;          
	int l;
	
	
	x[TAM-1]= a[TAM-1][TAM]/a[TAM-1][TAM-1];
	for(k=(TAM-1); k>=0;k--)
	{
		s=0;
		for (j=k+1; j<TAM; j++)
		{
			s= s+(a[k][j]*x[j]);  
			x[k]= (a[k][TAM]-s)/a[k][k];
		}
	}
	
	escrever(out, "A - Matriz Gerada ----------\n\n",strlen("A - Matriz Gerada ----------\n\n"));  
	for (k=0; k<TAM; k++)
	{
		for (l=0; l<TAM; l++)
		{
			/*string_lenght =*/string_buffer = itoa(a[k][l],10);
			//printf("%s", string_buffer);
			escrever(out,string_buffer,strlen(string_buffer));
			escrever(out," ", 1);
		}
		escrever(out,"\n", 1);
	} 

	escrever(out,"Vetor B ----------\n\n", strlen("Vetor B ----------\n\n"));
	for (k=0; k<TAM; k++)
	{
		/*string_lenght =*/string_buffer = itoa(a[k][TAM],10);
		escrever(out,string_buffer,strlen(string_buffer));
		escrever(out,"\n",1);
	} 


	escrever(out,"Resolução ----------\n\n", strlen("Resolução ----------\n\n"));
	for (k=0; k<TAM; k++)
	{	
		/*string_lenght =*/string_buffer = itoa(x[k],10);
		escrever(out,string_buffer,strlen(string_buffer));
		
		escrever(out,"\n", 1);
	} 
	
	escrever(out,strcat("\nTempo = ", string_buffer) , strlen(strcat("\nTempo = ", string_buffer))); 
	fechar(out);
	printf("\nTempo = ");
	printf("%f", delta_t);
	unsigned char tecla;
	getchar(&tecla);
	sair();
}

