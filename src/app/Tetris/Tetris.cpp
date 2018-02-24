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

const int ESQ 	 	 = 0;
const int DIR 	 	 = 1;
const int ABAIXO 	 = 2;
const int LIM_ESQ 	 = 0;
const int LIM_DIR 	 = 15;
const int LIM_ACIMA  = 0;
const int LIM_ABAIXO = 22;
const int TAM_PECA 	 = 4;

int Peca1X[TAM_PECA] = {7, 8, 7, 8};
int Peca1Y[TAM_PECA] = {0, 0, 1, 1};
int Peca2X[TAM_PECA] = {6, 7, 8, 9};
int Peca2Y[TAM_PECA] = {0, 0, 0, 0};
int Peca3X[TAM_PECA] = {7, 7, 7, 7};
int Peca3Y[TAM_PECA] = {0, 1, 2, 3};
int Peca4X[TAM_PECA] = {7, 8, 9, 8};
int Peca4Y[TAM_PECA] = {0, 0, 0, 1};
int Peca5X[TAM_PECA] = {7, 8, 8, 9};
int Peca5Y[TAM_PECA] = {0, 0, 1, 1};
int Peca6X[TAM_PECA] = {7, 8, 8, 9};
int Peca6Y[TAM_PECA] = {1, 1, 0, 0};
int Peca7X[TAM_PECA] = {7, 8, 7, 9};
int Peca7Y[TAM_PECA] = {1, 0, 0, 0};
int Peca8X[TAM_PECA] = {7, 8, 9, 9};
int Peca8Y[TAM_PECA] = {0, 0, 0, 1};


static unsigned long int next = 1;
 
int _rand()
{
    next = next * 1103515245 + 12345;
    return (unsigned int)(next / 65536) % 32768;
}

void _srand( unsigned int seed )
{
    next = seed;
}

class Peca
{

	public:
	
	int x[TAM_PECA];
	int y[TAM_PECA];
	int rotacao;
	char cor;
	
	void criar_peca(int pontosX[TAM_PECA], int pontosY[TAM_PECA], char c)
	{
		memcpy((char *)x , (char *)pontosX, sizeof(int)*TAM_PECA);
		memcpy((char *)y , (char *)pontosY, sizeof(int)*TAM_PECA);
		cor = c;
		rotacao = 1;
	}
	
	void copiar(Peca peca_copiar)
	{
		memcpy((char *)x , (char *)peca_copiar.x, sizeof(int)*TAM_PECA);
		memcpy((char *)y , (char *)peca_copiar.y, sizeof(int)*TAM_PECA);
		cor = peca_copiar.cor;
		rotacao = peca_copiar.rotacao;
	}
	
};

class Frame
{
	public:
	
	short * dados;
	char cor;
	char fundo;
	int  pos_x;
	int  pos_y;
	int  qtd_linhas;
	int  qtd_colunas;
	int  qtd_colunas_duplas;
	int  ult_linha;
	int  ult_coluna;
	unsigned int pontuacao;
	int nivel;
	char blocos[16][23];
	
	void criar_frame(int x, int y, char cr_fundo, char cr_marg, int tam_c, int tam_l, short * d)
	{
		fundo = cr_fundo;
		cor = cr_marg;
		pos_x = x;
		pos_y = y;
		qtd_linhas = tam_l;
	    qtd_colunas = tam_c;
		qtd_colunas_duplas = tam_c/2;
		ult_linha = qtd_linhas -1;
		ult_coluna = qtd_colunas -1;
		dados = d;
		pontuacao = 0;
		nivel = 1;
		Relogio relogio;
		obter_relogio(relogio);
		
		_srand(relogio.ms_dia);
		
		for(int i=0; i < (qtd_colunas_duplas); i++)
			for(int j=0; j < qtd_linhas; j++)
				blocos[i][j] = cr_fundo;
			
		imprimir_pontuacao();
		
		unsigned char c = ' ';
		short val =  (fundo << 12 ) |(cor << 8) |c;		
		memsetw(dados, val, 2000);
	
		esc_mem_video(dados, x, y, x+qtd_colunas-1, y+qtd_linhas-1);
		
	}
	
	void apagar(Peca peca)
	{
		peca.cor = fundo;
		pintar(peca);
	}
	
	void pintar(Peca peca)
	{
		for(int i =0; i < TAM_PECA; i++)
		{	
			int pos1 = peca.y[i]*qtd_colunas + (peca.x[i]*2);
			int pos2 = pos1+1;
			dados[pos1] = dados[pos2] = (peca.cor << 12 ) |(cor << 8) |' ';		
		}			
	}
	
	int mover(Peca * peca, int direcao)
	{
		Peca peca_aux;
		peca_aux.copiar(*peca);
		
		if(direcao == ESQ)
		{
			for(int i =0; i < TAM_PECA; i++)
				peca_aux.x[i]--;
		}
		else if(direcao == DIR)
		{
			for(int i =0; i < TAM_PECA; i++)
				peca_aux.x[i]++;
		}
		else if(direcao == ABAIXO)
		{
			for(int i =0; i < TAM_PECA; i++)
				peca_aux.y[i]++;
		}
		
		if(validarPosicao(peca_aux))
		{
			apagar(*peca);
			peca->copiar(peca_aux);
			pintar(*peca);
			return 1;
		}
		else
		{
			return 0;
		}
	}
	
	int rotacionar(Peca * peca)
	{
		if(peca->cor == VID_COLOR_VERMELHO)
			return 1;
			
		Peca peca_aux;
		peca_aux.copiar(*peca);
		
		for(int i =0; i < TAM_PECA; i++)
		{
			if(i != 1)
			{
				int x_relativo = peca_aux.x[i] - peca_aux.x[1];
				int y_relativo = peca_aux.y[i] - peca_aux.y[1];
				
				if(peca_aux.rotacao == 1)
				{
					peca_aux.x[i] = (-1)*y_relativo;
					peca_aux.y[i] = x_relativo;
				}
				else
				{
					peca_aux.x[i] = y_relativo;
					peca_aux.y[i] = (-1)*x_relativo;
				}

				peca_aux.x[i] += peca_aux.x[1];
				peca_aux.y[i] += peca_aux.y[1];
			}
		}
		
		if((peca_aux.cor == VID_COLOR_VED_CLARO)
			|| (peca_aux.cor == VID_COLOR_MARRON)
			|| (peca_aux.cor == VID_COLOR_AZUL)
			|| (peca_aux.cor == VID_COLOR_MRO_CLARO))
		{
			peca_aux.rotacao = 1 - peca_aux.rotacao;
		}
		
		if(validarPosicao(peca_aux))
		{
			apagar(*peca);
			peca->copiar(peca_aux);
			pintar(*peca);
			return 1;
		}
		else
		{
			return 0;
		}
	}
	
	void adicionar_blocos(Peca peca)
	{
		for(int i =0; i<TAM_PECA; i++)
		{
			blocos[peca.x[i]][peca.y[i]] = peca.cor;
		}
	}
	
	int validarPosicao(Peca peca)
	{
		for(int i =0;  i < TAM_PECA; i++)
		{
			if( !((peca.y[i] >= LIM_ACIMA) &&  (peca.y[i] <= LIM_ABAIXO)))
				return 0;
				
			if( !((peca.x[i] >= LIM_ESQ) &&  (peca.x[i] <= LIM_DIR)))
				return 0;	
				
			if(blocos[peca.x[i]][peca.y[i]] != fundo)
				return 0;
		}
		
		return 1;
	}
	
	void remover_linha(int linha)
	{
		for(int y=linha; y > 0; y--)
		{
			for(int x=0; x < qtd_colunas_duplas; x++)
			{
				blocos[x][y] = blocos[x][y-1];	
			}
		}
		
		for(int y=linha; y > 0; y--)
		{
			int pos_font = (y-1)*qtd_colunas;
			int pos_dest = y*qtd_colunas;
			memcpy((char*)&dados[pos_dest], (char*)&dados[pos_font], sizeof(short)*qtd_colunas);
		}
		
		for(int i =0; i <qtd_colunas; i++ )
		{
			dados[i] = (fundo << 12 ) |(cor << 8) |' ';	
		}
		
		marcar_pontos();
	}
	
	void verificar_linhas_completas()
	{
		for(int y=qtd_linhas; y >=0; y--)
		{
			int vazio_encontrado = 0;
			int bloco_encontrado = 0;
			
			for(int x=0; x < qtd_colunas_duplas; x++)
			{
				if(blocos[x][y] == fundo)
					vazio_encontrado = 1;
				else
					bloco_encontrado = 1;
			}
		
			if(bloco_encontrado == 1 && vazio_encontrado==0)
			{
				remover_linha(y);
				y++;
			}
		}	
	}

	Peca gerar_peca()
	{
		Peca  nova_peca;
		int num_aleatorio = _rand();
		int num_peca = num_aleatorio % 8;
		
		
		switch(num_peca)
		{
			case 0 : nova_peca.criar_peca(Peca1X, Peca1Y, VID_COLOR_VERMELHO); break;//nao rotaciona
			case 1 : nova_peca.criar_peca(Peca2X, Peca2Y, VID_COLOR_VED_CLARO); break;//rotaciona 90 -90
			case 2 : nova_peca.criar_peca(Peca3X, Peca3Y, VID_COLOR_MARRON); break;//rotaciona 90 -90
			case 3 : nova_peca.criar_peca(Peca4X, Peca4Y, VID_COLOR_CIA_CLARO); break;
			case 4 : nova_peca.criar_peca(Peca5X, Peca5Y, VID_COLOR_AZUL); break;//rotaciona 90 -90
			case 5 : nova_peca.criar_peca(Peca6X, Peca6Y, VID_COLOR_MRO_CLARO); break;//rotaciona 90 -90
			case 6 : nova_peca.criar_peca(Peca7X, Peca7Y, VID_COLOR_VERDE); break;
			case 7 : nova_peca.criar_peca(Peca8X, Peca8Y, VID_COLOR_CIANO); break;
			
		}
		
		return nova_peca;
	}
	
	void marcar_pontos()
	{
		pontuacao += nivel*30;
		
		if(nivel < 10)
		{
			if(pontuacao >= (nivel*nivel*100))
				nivel++;
		}
		
	}
	
	void imprimir_pontuacao()
	{
		posicionar_cursor(60,3);
		printf("SCORE");
		posicionar_cursor(60,4);
		printf("%d\n", pontuacao);
		
		posicionar_cursor(60,7);
		printf("LEVEL\n");
		posicionar_cursor(60,8);
		printf("%d\n", nivel);
		
	}
	
	void atualizar()
	{
		esc_mem_video(dados, pos_x, pos_y, pos_x+qtd_colunas-1, pos_y+qtd_linhas-1);
	}

};

Frame frame;
short dados[2000];
Peca minhaPeca;
volatile int jogo_em_pausa =0;
volatile int jogo_em_progresso =0;
volatile int finalizar = 0;

void thread()
{
	char msg[100];
	memcpy(msg, "thread1", strlen("thread1")+1);
	
	unsigned char tecla;
	while(true)
	{
		getchar(&tecla);
		
		if(jogo_em_progresso)
		{
			if(tecla == SETA_ESQ)
			{
				frame.mover(&minhaPeca, ESQ);
			}
			else if(tecla == SETA_DIR)
			{
				frame.mover(&minhaPeca, DIR);
			}
			else if(tecla == SETA_PARA_BAIXO)
			{
				frame.mover(&minhaPeca, ABAIXO);
			}
			else if(tecla == SETA_PARA_CIMA)
			{
				frame.rotacionar(&minhaPeca);
			}
			enviar_receber_msg(42000, msg);
		}
		
		if(tecla == ENTER)
			jogo_em_progresso = 1;
			
		if(tecla == ESC)
			finalizar =1;
			
		if(tecla == TAB)	
		{
			jogo_em_pausa = 1 - jogo_em_pausa;
			jogo_em_progresso = 1 - jogo_em_progresso;
		}
	}
}

void thread2()
{
	char msg[100];
	memcpy(msg, "thread2", strlen("thread2")+1);
	
	while(true)
	{
		while(jogo_em_progresso)
		{
			if(!frame.mover(&minhaPeca, ABAIXO))
			{
				frame.adicionar_blocos(minhaPeca);
				frame.verificar_linhas_completas();
				minhaPeca = frame.gerar_peca();//criar_peca(Peca5X, Peca5Y, VID_COLOR_VED_CLARO);
				
				if(!frame.validarPosicao(minhaPeca))
				{
					frame.pintar(minhaPeca);
					esperar(500);
					jogo_em_progresso = 0;
				}
			}
			
			enviar_receber_msg(42000, msg);
		
			esperar(700 - frame.nivel*50);
		}
	}
	
	sair();
}


main(int argc, char ** args)
{
	posicionar_cursor(27,0);
	printf("FESO TETRIS");
	int pidthread1 = criar_thread(thread);
	int pidthread2 = criar_thread(thread2);
	char msg[100];
	escutar_porta(42000);
	
	while(!finalizar)
	{
		frame.criar_frame(24, 1, VID_COLOR_CZA_CLARO, VID_COLOR_CZA_CLARO ,32, 23, dados);
		minhaPeca =  frame.gerar_peca();
		posicionar_cursor(25,12);
		printf("Pressione ENTER para comecar!");
		int ultima_pontuacao = frame.pontuacao;
		
		while((jogo_em_progresso || jogo_em_pausa) && !finalizar)
		{		
			frame.atualizar();
			//esperar(20);
			receber_msg(msg, MSG_QLQR_PORTA);
			
			if(!strcmp(msg, "thread1"))
			{
				enviar_msg_pid(pidthread1, msg); 
			}
			else
			{
				enviar_msg_pid(pidthread2, msg); 
			}
			
			if(ultima_pontuacao != frame.pontuacao)
			{
				ultima_pontuacao = frame.pontuacao;
				frame.imprimir_pontuacao();
			}
			
			if(jogo_em_pausa)
			{
				posicionar_cursor(33,12);
				printf("Jogo Pausado!");
			}
		}
	}
	
	sair();
}
