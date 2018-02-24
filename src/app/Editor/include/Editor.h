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
#ifndef _EDITOR
#define _EDITOR

#include "Sistema.h"
#include "Util.h"
#include "Lista.h"
#include "Io.h"

class Texto
{
	public :
	
	Lista< Lista<unsigned char> > conteudo;
	
	//métodos
	void adicionar_linha();
	void remover_char(unsigned int pos_x, unsigned int pos_y);
	void adicionar_char(unsigned char c, unsigned int pos_x, unsigned int pos_y);
	void unir_linhas(unsigned int pos_linha, unsigned int tam_max);
	void dividir_linha(unsigned int pos_x, unsigned int pos_y, unsigned int tam_max);
	int tamanho();
	Lista<unsigned char>& operator[](unsigned long indice);
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
	int  ult_linha;
	int  ult_coluna;
	
	void criar_frame(int x, int y, char cr_fundo, char cr_marg, int tam_c, int tam_l, short * d)
	{
		fundo = cr_fundo;
		cor = cr_marg;
		pos_x = x;
		pos_y = y;
		qtd_linhas = tam_l;
	    qtd_colunas = tam_c;
		ult_linha = qtd_linhas -1;
		ult_coluna = qtd_colunas -1;
		dados = d;
		
		unsigned char c = ' ';
		short val =  (fundo << 12 ) |(cor << 8) |c;		
		memsetw(dados, val, 2000);
	
		c = 0xCD;		
		val =  ( (fundo & 0x0F) << 12 ) |((cor & 0x0F) << 8) |c;
		
		for(int i = 1; i < qtd_colunas-1; i++)
		{
			dados[0*qtd_colunas + i] = val;
			dados[ult_linha*qtd_colunas + i] = val;
		}
		
		c = 0xBA;	
		val =  ( (fundo & 0x0F) << 12 ) |((cor & 0x0F) << 8) |c;
		for(int i = 1; i < qtd_linhas-1; i++)
		{
			dados[i*qtd_colunas + 0] = val;
			dados[i*qtd_colunas + ult_coluna] = val;
		}
		
		c = 0xC9;	
		val =  ( (fundo & 0x0F) << 12 ) |((cor & 0x0F) << 8) |c;
		dados[0] = val;
		
		c = 0xC8;	
		val =  ( (fundo & 0x0F) << 12 ) |((cor & 0x0F) << 8) |c;
		dados[ult_linha*qtd_colunas] = val;
		
		c = 0xBB;	
		val =  ( (fundo & 0x0F) << 12 ) |((cor & 0x0F) << 8) |c;
		dados[ult_coluna] = val;
		
		c = 0xBC;	
		val =  ( (fundo & 0x0F) << 12 ) |((cor & 0x0F) << 8) |c;
		dados[ult_linha*qtd_colunas + ult_coluna] = val; 
		
		
		esc_mem_video(dados, x, y, x+qtd_colunas-1, y+qtd_linhas-1);
		
	}
	
	
	void escrever(char * texto, char cr_fundo, char cr_texto, int x, int y)
	{
		int tam = strlen(texto);
		
		int pos = y*qtd_colunas + x;
		
		for(int i =0; i < tam; i++)
		{
			dados[pos++] = ( (cr_fundo & 0x0F) << 12 ) |((cr_texto & 0x0F) << 8) |texto[i];
			
		}
	}
	
	void escrever(char c, char cr_fundo, char cr_texto, int x, int y)
	{	
		int pos = y*qtd_colunas + x;
	    dados[pos++] = ( (cr_fundo & 0x0F) << 12 ) |((cr_texto & 0x0F) << 8) | c;
	}
	
	void atualizar()
	{
		esc_mem_video(dados, pos_x, pos_y, pos_x+qtd_colunas-1, pos_y+qtd_linhas-1);
	}

};

class AreaDeTexto 
{
   public :
   Texto texto;
   Frame frame;
   short dados[2000];
   short backup[2000];
   
   short x_ini, y_ini;
   short x_fim, y_fim;
   short colunas_max;
   short linhas_max; 
   short pos_x;
   short pos_y_texto;
   short prim_linha_exibida;
   short pos_x_tela;
   short pos_y_tela;
  
   short obter_x_abs(short x_rel);
   short obter_y_abs(short y_rel);
   
   
   AreaDeTexto::AreaDeTexto();
   AreaDeTexto(short x_i, short y_i, short x_f, short y_f);
   void inicializar(char * titulo);
   void executar_operacao(unsigned char c);
   void executar_operacao_background(unsigned char c);
   void reescrever();
   void carregar(short x_i, short y_i, short x_f, short y_f);
   void carregar_arquivo(char *nome);
   void salvar(char *nome);
  
};



#endif