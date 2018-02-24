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

int TAM = 2048;//tamanho da matriz quadrada
int QUANT_THREADS = 1; // numero de threads geradas
int **m_a;
int **m_b;
int **m_final;
unsigned char tecla;

int **allocate_real_matrix(int tam, int random);
int **free_real_matrix(int **v, int tam);

void paralelo(){
	unsigned char thread_data[100];
	int thread_num, inicio_matriz, fim_matriz,i,j,k;
	receber_msg(thread_data, MSG_QLQR_PORTA);
	
	thread_num = thread_data[0];
	inicio_matriz = thread_num*(TAM/QUANT_THREADS);
	/*
		se esta for a thread final, paso todos os valores restante para ela
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
	printf("ARGC: %d\n", argc);
	getchar(&tecla);
	if (argc > 1)
	{
		QUANT_THREADS = atoi(args[0]);
		TAM = atoi(args[1]);
		printf("ARGV: %d , %d \n", atoi(args[0]), atoi(args[1]));
	}
	
	printf("QUANT_THREADS: %d\n\n", QUANT_THREADS);
	printf("QUANT_THREADS: %d\n\n", TAM);
	getchar(&tecla);
	
	int i,j,k;
	unsigned char thread_data[100];
	
	//criando e preenchendo a matriz
	m_a = allocate_real_matrix(TAM, 1);
	printf("matrize 'a' alocada");
	getchar(&tecla);
	m_b = allocate_real_matrix(TAM, 1);
	printf("matrize 'b' alocada");
	getchar(&tecla);
	m_final = allocate_real_matrix(TAM, 0);
	
	printf("matrizes alocadas");
	getchar(&tecla);
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
	
	free_real_matrix(m_a, TAM);
	free_real_matrix(m_b, TAM);
	free_real_matrix(m_final, TAM);
	//travando a execucao
	getchar(&tecla);
	sair();
}

int **allocate_real_matrix(int tam, int random) {
    int i, j, n = tam, m = tam;
    int **v, a;         // ponteiro para o vetor
	int * aux;

    v = (int**) malloc(n * sizeof(int*));

    if (v == NULL) {
        printf ("** Erro: memoria insuficiente (L232)**");
		getchar(&tecla);
        return (NULL);
    }

    // alocando cada linha da matriz
    for (i = 0; i < n; i++) {
        //v[i] = (double*) malloc(m * sizeof(double));
        aux = (int*) malloc(m * sizeof(int));
		v[i] = aux;
		
		printf("%d %d\n ", aux, v[i]);
		for(;;);
		
        if (aux == NULL) {
            printf ("** Erro: memoria insuficiente (L242)**");
			getchar(&tecla);
            free_real_matrix(v, n);
            return (NULL);
        }

        // inicializa a matriz com zeros
        if (random == 0) {
            for (j = 0; j < m; j++)
                v[i][j] = 0.0;
        }

        // inicializa a matriz com valores aleatorios
        else {
            if (random == 1) {
                for (j = 0; j < m; j++) {
                    a = rand() % 100;// % MATRIX_LENGTH;
                    v[i][j] = a;
                }
            }
        }
    }

    return (v);
}

int **free_real_matrix(int **v, int tam) {

    int i;

    if (v == NULL) {
        return (NULL);
    }

    for (i = 0; i < tam; i++) {
        if (v[i]) {
            free(v[i]); // libera a matriz linha por linha
            v[i] = NULL;
        }
    }

    free(v);         // liberando o ponteiro /
    v = NULL;

    return (NULL);   //retornando um ponteiro nulo /
}