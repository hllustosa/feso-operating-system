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

int MATRIX_LENGTH = 4;
unsigned char tecla;

void strassen(double **a, double **b, double **c, int tam);
void sum(double **a, double **b, double **result, int tam);
void subtract(double **a, double **b, double **result, int tam);
double **allocate_real_matrix(int tam, int random);
double **free_real_matrix(double **v, int tam);
void print_matrix(double **v, int tam);

int main(int argc, char ** argv)
{
	if (argc > 0)
	{
		MATRIX_LENGTH = atoi(argv[0]);
	}
	printf("Tamanho da matriz: %d\n\n\n", MATRIX_LENGTH);
	srand((unsigned int)pit());
	
    //alocando geral
	double **matrix_a = allocate_real_matrix(MATRIX_LENGTH, 1);
    double **matrix_b = allocate_real_matrix(MATRIX_LENGTH, 1);
    double **matrix_c = allocate_real_matrix(MATRIX_LENGTH, 0);
    //rodando o algoritmo
	
	Relogio r;
	obter_relogio(r);
	int t_inicial = r.ms_dia;
	
    strassen(matrix_a, matrix_b, matrix_c, MATRIX_LENGTH);
	

    //imprimir
    //print_matrix(matrix_a, MATRIX_LENGTH);
    //print_matrix(matrix_b, MATRIX_LENGTH);
    //print_matrix(matrix_c, MATRIX_LENGTH);
    printf("Fim do algo\n");
	obter_relogio(r);
	int t_final = r.ms_dia;
	printf("Tempo final %d ms", (t_final - t_inicial));

    //liberando geral
    matrix_a = free_real_matrix(matrix_a, MATRIX_LENGTH);
    matrix_b = free_real_matrix(matrix_b, MATRIX_LENGTH);
    matrix_c = free_real_matrix(matrix_c, MATRIX_LENGTH);
	
	//pause
	
	getchar(&tecla);
	sair();

}

void strassen(double **a, double **b, double **c, int tam) {
	
	//printf("Recusividade...");
	//getchar(&tecla);

    // Special case:we must write The following when the matrix is 1 X 1:
    if (tam == 1) {
        c[0][0] = a[0][0] * b[0][0];
        return;
    }

    // other cases are treated here:
        int newTam = tam/2;
        double **a11, **a12, **a21, **a22;
        double **b11, **b12, **b21, **b22;
        double **c11, **c12, **c21, **c22;
        double **p1, **p2, **p3, **p4, **p5, **p6, **p7;

        // memory allocation:
        a11 = allocate_real_matrix(newTam, -1);
        a12 = allocate_real_matrix(newTam, -1);
        a21 = allocate_real_matrix(newTam, -1);
        a22 = allocate_real_matrix(newTam, -1);

        b11 = allocate_real_matrix(newTam, -1);
        b12 = allocate_real_matrix(newTam, -1);
        b21 = allocate_real_matrix(newTam, -1);
        b22 = allocate_real_matrix(newTam, -1);

        c11 = allocate_real_matrix(newTam, -1);
        c12 = allocate_real_matrix(newTam, -1);
        c21 = allocate_real_matrix(newTam, -1);
        c22 = allocate_real_matrix(newTam, -1);

        p1 = allocate_real_matrix(newTam, -1);
        p2 = allocate_real_matrix(newTam, -1);
        p3 = allocate_real_matrix(newTam, -1);
        p4 = allocate_real_matrix(newTam, -1);
        p5 = allocate_real_matrix(newTam, -1);
        p6 = allocate_real_matrix(newTam, -1);
        p7 = allocate_real_matrix(newTam, -1);

        double **aResult = allocate_real_matrix(newTam, -1);
        double **bResult = allocate_real_matrix(newTam, -1);

        int i, j;

        //dividing the matrices in 4 sub-matrices:
        for (i = 0; i < newTam; i++) {
            for (j = 0; j < newTam; j++) {
                a11[i][j] = a[i][j];
                a12[i][j] = a[i][j + newTam];
                a21[i][j] = a[i + newTam][j];
                a22[i][j] = a[i + newTam][j + newTam];

                b11[i][j] = b[i][j];
                b12[i][j] = b[i][j + newTam];
                b21[i][j] = b[i + newTam][j];
                b22[i][j] = b[i + newTam][j + newTam];
            }
        }

        //we must here  Calculate from p1 to p7 :

        sum(a11, a22, aResult, newTam); // a11 + a22
        sum(b11, b22, bResult, newTam); // b11 + b22
        strassen(aResult, bResult, p1, newTam); // p1 = (a11+a22) * (b11+b22)

        sum(a21, a22, aResult, newTam); // a21 + a22
        strassen(aResult, b11, p2, newTam); // p2 = (a21+a22) * (b11)

        subtract(b12, b22, bResult, newTam); // b12 - b22
        strassen(a11, bResult, p3, newTam); // p3 = (a11) * (b12 - b22)

        subtract(b21, b11, bResult, newTam); // b21 - b11
        strassen(a22, bResult, p4, newTam); // p4 = (a22) * (b21 - b11)

        sum(a11, a12, aResult, newTam); // a11 + a12
        strassen(aResult, b22, p5, newTam); // p5 = (a11+a12) * (b22)

        subtract(a21, a11, aResult, newTam); // a21 - a11
        sum(b11, b12, bResult, newTam); // b11 + b12
        strassen(aResult, bResult, p6, newTam); // p6 = (a21-a11) * (b11+b12)

        subtract(a12, a22, aResult, newTam); // a12 - a22
        sum(b21, b22, bResult, newTam); // b21 + b22
        strassen(aResult, bResult, p7, newTam); // p7 = (a12-a22) * (b21+b22)

        // we must here calculate c21, c21, c11 e c22:

        sum(p3, p5, c12, newTam); // c12 = p3 + p5
        sum(p2, p4, c21, newTam); // c21 = p2 + p4

        sum(p1, p4, aResult, newTam); // p1 + p4
        sum(aResult, p7, bResult, newTam); // p1 + p4 + p7
        subtract(bResult, p5, c11, newTam); // c11 = p1 + p4 - p5 + p7

        sum(p1, p3, aResult, newTam); // p1 + p3
        sum(aResult, p6, bResult, newTam); // p1 + p3 + p6
        subtract(bResult, p2, c22, newTam); // c22 = p1 + p3 - p2 + p6

        // Aggregation the results obtained in a single matrix:
        for (i = 0; i < newTam ; i++) {
            for (j = 0 ; j < newTam ; j++) {
                c[i][j] = c11[i][j];
                c[i][j + newTam] = c12[i][j];
                c[i + newTam][j] = c21[i][j];
                c[i + newTam][j + newTam] = c22[i][j];
            }
        }

        // deallocating memory (free):
        a11 = free_real_matrix(a11, newTam);
        a12 = free_real_matrix(a12, newTam);
        a21 = free_real_matrix(a21, newTam);
        a22 = free_real_matrix(a22, newTam);

        b11 = free_real_matrix(b11, newTam);
        b12 = free_real_matrix(b12, newTam);
        b21 = free_real_matrix(b21, newTam);
        b22 = free_real_matrix(b22, newTam);

        c11 = free_real_matrix(c11, newTam);
        c12 = free_real_matrix(c12, newTam);
        c21 = free_real_matrix(c21, newTam);
        c22 = free_real_matrix(c22, newTam);

        p1 = free_real_matrix(p1, newTam);
        p2 = free_real_matrix(p2, newTam);
        p3 = free_real_matrix(p3, newTam);
        p4 = free_real_matrix(p4, newTam);
        p5 = free_real_matrix(p5, newTam);
        p6 = free_real_matrix(p6, newTam);
        p7 = free_real_matrix(p7, newTam);
        aResult = free_real_matrix(aResult, newTam);
        bResult = free_real_matrix(bResult, newTam);

} // the  end ..... of Strassen function

// function to sum two matrices
void sum(double **a, double **b, double **result, int tam) {

    int i, j;

    for (i = 0; i < tam; i++) {
        for (j = 0; j < tam; j++) {
            result[i][j] = a[i][j] + b[i][j];
        }
    }
}

// function to subtract two matrices
void subtract(double **a, double **b, double **result, int tam) {

    int i, j;

    for (i = 0; i < tam; i++) {
        for (j = 0; j < tam; j++) {
            result[i][j] = a[i][j] - b[i][j];
        }
    }
}

// This function allocates the matrix using malloc, and initializes it. If the variable random is passed
// as zero, it initializes the matrix with zero, if it's passed as 1, it initializes the matrix with random
// values. If it is passed with any other int value (like -1 for example) the matrix is initialized with no
// values in it. The variable tam defines the length of the matrix.

double **allocate_real_matrix(int tam, int random) {
    int i, j, n = tam, m = tam;
    double **v, a;         // ponteiro para o vetor
	double * aux;

    v = (double**) malloc(n * sizeof(double*));

    if (v == NULL) {
        printf ("** Erro: memoria insuficiente (L232)**");
		getchar(&tecla);
        return (NULL);
    }

    // alocando cada linha da matriz
    for (i = 0; i < n; i++) {
        //v[i] = (double*) malloc(m * sizeof(double));
        aux = (double*) malloc(m * sizeof(double));
		v[i] = aux;

        if (v[i] == NULL) {
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

    return (v);     // returns the pointer to the vector.
}

// Funcao para desalocar a matriz (free)
double **free_real_matrix(double **v, int tam) {

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

//funcao para imprimir uma matrizes X,Y
void print_matrix(double **v, int tam){
    int i, j;
    for (i=0;i<tam;i++){
        for (j=0;j<tam;j++)
            printf("%f\t", v[i][j]);
        printf("\n");
    }
    printf("\n\n\n");
}