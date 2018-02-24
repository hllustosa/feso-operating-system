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
#include "include/Util.h"
#include "include/Sistema.h"
#include "include/Sistimer.h"
#include "include/Video.h"
#include "include/Teclado.h"

//constantes para o controle da tela
#define MAX_LINHAS 25
#define MAX_COLUNAS 80
#define PRIM_LINHA 1
#define PRIM_COLUNA 0
#define MEMORIA_VIDEO 0xC00B8000

//ponteiro para a memória do video
unsigned short *textmemptr = (unsigned short *)MEMORIA_VIDEO;
unsigned short * double_buffer;

//varíaveis de controle do cliente
int atributo = VID_COLOR_BRANCO;
int cursor_habilitado  = 0;
int mover_cursor_auto  = 1;
int vid_pid_foreground = 0;
int csr_x=0, csr_y =0;
char vid_arquivo_out[50];
char terminal_backup[50];

//--------------------------------INICIALIZAÇÃO-------------------------------------------
void clock()
{
	Relogio r;
	char num[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
	int cont =0;
	while(true)
	{
		esperar(1000);
		obter_relogio(r);
		
		//vid_config_color(VID_COLOR_BRANCO,VID_COLOR_AZUL);
		vid_imp_char_cor(num[r.hora/10], 72, 0);
		vid_imp_char_cor(num[r.hora%10], 73, 0);
		vid_imp_char_cor(':', 74, 0);
		vid_imp_char_cor(num[r.min/10], 75, 0);
		vid_imp_char_cor(num[r.min%10], 76, 0);
		vid_imp_char_cor(':', 77, 0);
		vid_imp_char_cor(num[r.seg/10], 78, 0);
		vid_imp_char_cor(num[r.seg%10], 79, 0);
		
		//vid_config_color(VID_COLOR_BRANCO,VID_COLOR_PRETO);
	}
	
	sair();
}

void vid_inicializar()
{
	//remover todos os caracteres da tela
	vid_limpar_tela();
  
	//desabilitando o cursor
	vid_habilitar_cursor(0);	
  
	//adicionando listra azul ao topo da tela
	vid_config_color(VID_COLOR_BRANCO,VID_COLOR_AZUL);	
	
	char mensagem_topo[80];
	memset(mensagem_topo, ' ', 80);
	memcpy(mensagem_topo,"feSO - SISTEMA OPERACIONAL", strlen("feSO - SISTEMA OPERACIONAL"));

	for(int i =0; i < 80; i++)
	{
		vid_imp_char(mensagem_topo[i], i, 0);
	}
	
	//retornando a cor padrão
	vid_config_color(VID_COLOR_BRANCO,VID_COLOR_PRETO);
	
	//alocando memória para o buffer duplo
	double_buffer = (unsigned short *)malloc(sizeof(short)* MAX_COLUNAS * MAX_LINHAS);
	
	criar_thread(clock);
}


//--------------------------------CARREGAMENTO E SALVAMENTO DE TERMINAIS-------------------------------------------
void vid_recuperar_status(Vid_Status * status)
{
	 //carregando valores nas variáveis	
	 status->atributo            =  atributo;           
	 status->cursor_habilitado   =  cursor_habilitado;  
	 status->mover_cursor_auto   =  mover_cursor_auto;  
	 status->vid_pid_foreground  =  vid_pid_foreground; 
	 status->csr_x               =  csr_x; 	    		
	 status->csr_y               =  csr_y; 				
	 memcpy(status->vid_arquivo_out, vid_arquivo_out, strlen(vid_arquivo_out)+1);
	 memcpy(status->terminal_backup, terminal_backup, strlen(terminal_backup)+1);
	  
	 //salvando console atual no arquivo
	 int arq = abrir(terminal_backup, 'a');
	 for(int i =PRIM_LINHA; i < MAX_LINHAS; i++)
	 {
		escrever(arq, (unsigned char *) (textmemptr + MAX_COLUNAS * i), MAX_COLUNAS*2);
	 } 
	 fechar(arq); 
}

void vid_alterar_processo_foreground(Vid_Status * status)
{
	//if(vid_pid_foreground != status->vid_pid_foreground)
	//{
		 //carregando valores nas variáveis	
		 atributo            = status->atributo;
		 cursor_habilitado   = status->cursor_habilitado;
		 mover_cursor_auto   = status->mover_cursor_auto;
		 vid_pid_foreground  = status->vid_pid_foreground;
		 csr_x 	    		 = status->csr_x;
		 csr_y 				 = status->csr_y;
		 memcpy(vid_arquivo_out, status->vid_arquivo_out, strlen(status->vid_arquivo_out)+1);
		 memcpy(terminal_backup, status->terminal_backup, strlen(status->terminal_backup)+1);
		 
		 //gravando console atual em arquivo
		 int arq = abrir(terminal_backup, 'A');
		 for(int i = PRIM_LINHA; i < MAX_LINHAS; i++)
		 {
			ler(arq, (unsigned char *) (textmemptr + MAX_COLUNAS * i), MAX_COLUNAS*2);
		 } 
	     fechar(arq);
		 vid_habilitar_cursor(cursor_habilitado);
    //}
}

void vid_mudar_para_background(Vid_Status * status, Vid_Status * status_background)
{
	//altera ponteiro da memória do vídeo
	textmemptr = double_buffer;
	
	//salva dados em status
	 status->atributo            =  atributo;           
	 status->cursor_habilitado   =  cursor_habilitado;  
	 status->mover_cursor_auto   =  mover_cursor_auto;  
	 status->vid_pid_foreground  =  vid_pid_foreground; 
	 status->csr_x               =  csr_x; 	    		
	 status->csr_y               =  csr_y; 				
	 memcpy(status->vid_arquivo_out, vid_arquivo_out, strlen(vid_arquivo_out)+1);
	 memcpy(status->terminal_backup, terminal_backup, strlen(terminal_backup)+1);
	
	//coloca dados do cliente em background
	//carregando valores nas variáveis	
	 atributo            = status_background->atributo;
	 cursor_habilitado   = status_background->cursor_habilitado;
	 mover_cursor_auto   = status_background->mover_cursor_auto;
	 vid_pid_foreground  = status_background->vid_pid_foreground;
	 csr_x 	    		 = status_background->csr_x;
	 csr_y 				 = status_background->csr_y;
	 memcpy(vid_arquivo_out, status_background->vid_arquivo_out, strlen(status_background->vid_arquivo_out)+1);
	 memcpy(terminal_backup, status_background->terminal_backup, strlen(status_background->terminal_backup)+1);
	 
	 //lendo console atual em arquivo
	 int arq = abrir(terminal_backup, 'A');
	 for(int i = PRIM_LINHA; i < MAX_LINHAS; i++)
	 {
		ler(arq, (unsigned char *) (textmemptr + MAX_COLUNAS * i), MAX_COLUNAS*2);
	 } 
	 fechar(arq);
}

void vid_mudar_para_foreground(Vid_Status * status, Vid_Status * status_background)
{
	
	//salva dados em status
	 status_background->atributo            =  atributo;           
	 status_background->cursor_habilitado   =  cursor_habilitado;  
	 status_background->mover_cursor_auto   =  mover_cursor_auto;  
	 status_background->vid_pid_foreground  =  vid_pid_foreground; 
	 status_background->csr_x               =  csr_x; 	    		
	 status_background->csr_y               =  csr_y; 				
	 memcpy(status_background->vid_arquivo_out, vid_arquivo_out, strlen(vid_arquivo_out)+1);
	 memcpy(status_background->terminal_backup, terminal_backup, strlen(terminal_backup)+1);
	
    //salvando console em background
	 int arq = abrir(terminal_backup, 'a');
	 for(int i =PRIM_LINHA; i < MAX_LINHAS; i++)
	 {
		escrever(arq, (unsigned char *) (textmemptr + MAX_COLUNAS * i), MAX_COLUNAS*2);
	 } 
	 fechar(arq); 
	
	 //restaura informações do processo em foreground
	 textmemptr = MEMORIA_VIDEO;

	 atributo            = status->atributo;
	 cursor_habilitado   = status->cursor_habilitado;
	 mover_cursor_auto   = status->mover_cursor_auto;
	 vid_pid_foreground  = status->vid_pid_foreground;
	 csr_x 	    		 = status->csr_x;
	 csr_y 				 = status->csr_y;
	 memcpy(vid_arquivo_out, status->vid_arquivo_out, strlen(status->vid_arquivo_out)+1);
	 memcpy(terminal_backup, status->terminal_backup, strlen(status->terminal_backup)+1);
	
}

void vid_atender_solicitacao(int cod, char * out, int param1, int param2, int tam)
{	

    unsigned char * buf;
	
	//ler arquivo
	if(tam > 0)
	{
		if(tam > 512) tam = 512;
		 
		buf = (unsigned char *)malloc(tam);
		int arq = abrir(out, 'A');
		ler(arq, buf, tam);
		fechar(arq);
	}
	
	switch(cod)
	{
		case IMPRIMIR 		  : vid_imprimir_string(buf, tam); break;
		case ALTERAR_POS      : csr_x=param1; csr_y =param2; vid_mover_cursor(csr_x, csr_y+1);break;
		case CONFIG_COR       : vid_config_color(param1, param2); break;
		case HABILITAR_CURSOR : vid_habilitar_cursor(param1); break;	
		case LIMPAR_TELA      : vid_limpar_tela(); break;	
		case LER_VIDEO_MEM	  : vid_ler_video_mem(out, (param1 & 0xFFFF0000)>>16, (param1 & 0x0000FFFF), (param2 & 0xFFFF0000)>>16, (param2 & 0x0000FFFF)); break;	
		case ESC_VIDEO_MEM	  : vid_esc_video_mem(out, (param1 & 0xFFFF0000)>>16, (param1 & 0x0000FFFF), (param2 & 0xFFFF0000)>>16, (param2 & 0x0000FFFF)); break;
	}
		
	//enviar msg para acordar o cliente
	unsigned char msg[100] = "sucesso";
	enviar_msg_pid(vid_pid_foreground,msg);
	
	if(tam > 0)
	   free(buf);
}

//--------------------------------FUNÇÕES DE CONFIGURAÇÃO-------------------------------------------
void vid_mover_cursor(short x, short y)
{
   if(cursor_habilitado)
   {
		//Calculando o endereço de memória para uma posição x e y na tela
		unsigned int temp = y * 80 + x;
		outportb(0x3D4, 14);
		outportb(0x3D5, temp >> 8);
		outportb(0x3D4, 15);
		outportb(0x3D5, temp);
   }
}

void vid_limpar_tela()
{
    unsigned branco;
   
    // Colocar o valor branco em toda a tela
    branco = 0x20 | (atributo << 8);

    //Coloca branco em toda a tela
    for(int i = PRIM_LINHA; i < MAX_LINHAS; i++)
        memsetw (textmemptr + i * MAX_COLUNAS, branco, MAX_COLUNAS);
  
    csr_x = 0;
	csr_y = 1;
    vid_mover_cursor(0,1);
}

//habilita (estado = 1) ou desabilita (estado = 0) o cursor
void vid_habilitar_cursor(short estado)
{
  //salvando estado do cursor
  cursor_habilitado = estado;
  
  //lendo a quantidade máxima de linhas por caractere
  outportb(0x3D4, 0x09);
  
  //obtendo valor para configurar tamanho do cursor
  int maxlines = inportb(0x3D5);
  maxlines     = maxlines && 0x1f;
  outportb(0x3D4, 0x0A);
  
  //desabilitando ou habilitando o cursor
  if(cursor_habilitado)
  {
     outportb(0x3D5, 0x0D);
  }
  else //desabilitar cursor
  {
	 outportb(0x3D5, maxlines | 0x10);
  }
  
  //atualizando posição do cursor
  vid_mover_cursor(csr_x, csr_y);
   
}

//Altera a cor do texto e do plano de fundo. Ver constantes em video.h
void vid_config_color(short cor_texto, short cor_background)
{
	atributo = (cor_background << 4) | (cor_texto & 0x0F);
}

//altera a cor do fundo de um posição especifica na tela, sem modificar o caractere
void vid_alterar_cor_fundo(short cor, int x, int y)
{
   short aux;
   unsigned short * pos = textmemptr + (y * MAX_COLUNAS + x);

   //aux recebe pos  
   aux = *pos;
   
   //zerando os bits referentes a cor de fundo
   aux &= 0x0FFF; 
   
   //shl na cor
   cor = cor << 12;
   
   //Bitwise or para setar o caractere
   *pos = aux | cor;
}

//altera a cor da fonte de um posição especifica na tela, sem modificar o caractere
void vid_alterar_cor_fonte(short cor, int x, int y)
{
   short aux;
   unsigned short * pos = textmemptr + (y * MAX_COLUNAS + x);

   //aux recebe pos  
   aux = *pos;
   
   //zerando os bits referentes a cor de fundo
   aux &= 0xF0FF; 
   
   //shl na cor
   cor = cor << 8;
   
   //Bitwise or para setar a cor
   *pos = aux | cor;
}

//rola a tela para baixo
void vid_scroll(int * y)
{
	 unsigned branco, temp;
	  
     branco = 0x20 | (VID_COLOR_BRANCO << 8);
	 
	 //caso o valor de y seja maior que a máximo de linhas
     if(*y >= MAX_LINHAS)
     {
		 //variável temp armazena quantas linhas a mais foram adicionadas
         temp = *y - MAX_LINHAS + 2;
		 
		 //copia os dados de uma linha, para a linha superior
         memcpy ((unsigned char *)(textmemptr + MAX_COLUNAS),(unsigned char *) (textmemptr + temp * MAX_COLUNAS), (MAX_LINHAS - temp) * MAX_COLUNAS * 2 );

		 //limpa a última linha	
         memsetw (textmemptr + (MAX_LINHAS - 1) * MAX_COLUNAS, branco, MAX_COLUNAS);
		 
		 //y recebe o valor da última linha
         *y = MAX_LINHAS - 1;
     }
}

//--------------------------------FUNÇÕES DE IMPRESSÃO-------------------------------------------
//imprime uma string na tela
void vid_imprimir_string(char * texto, int tam)
{
	mover_cursor_auto = 0;	
	
	for(int i =0; i < tam; i++)
	{
		vid_imp_char(texto[i]);
	}
	
	mover_cursor_auto = 1;
}

//imprime uma string na tela na posição especificada
void vid_imprimir_string_pos(char * texto, int tam, int x, int y)
{
	if( (x >= PRIM_COLUNA) && (x < MAX_COLUNAS) && (y >= PRIM_LINHA) && (y<MAX_COLUNAS))
	{
		csr_x = x;
		csr_y = y;
		vid_imprimir_string(texto, tam);
	}
}

//imprime um caractere na tela na posição especificada
void vid_imp_char(unsigned char c, int x, int y)
{
   //montando atributos
   unsigned att = atributo << 8;
   
   //calculando posição em que o caractere será impresso
   unsigned short * pos = textmemptr + (y * MAX_COLUNAS + x);
   
   //colocando caractere e atributos na posição desejada
   *pos =  c | att;
}

void vid_limpar_linha(unsigned int caracteres)
{
	for (int i=0;i<caracteres;i++)
	{
		//caso primeiro caractere da linha não tenha sido alcançado
        if(csr_x != PRIM_COLUNA)
		{
		   //decrementa posição na linha
		   csr_x--;
		}
		//primeira posição da linha, mas não na primeira linha
		else if(csr_y != PRIM_LINHA)
		{
			//volta para o final da linha anterior
			csr_x = MAX_COLUNAS - 1;
			csr_y--;
		}
	}
	//coloca o espaço vazio na posição atual
	vid_imp_char(' ',csr_x, csr_y);
	
	//copia todos os dados da tela, uma caracatere para a direta
	//a partir da posição do csr_x e do csr_y atual
	unsigned short * pos = textmemptr + (csr_y * MAX_COLUNAS + csr_x);
	memcpy((unsigned char * )pos, (unsigned char *)(pos+1), MAX_COLUNAS*MAX_LINHAS*sizeof(short));
	//testando o o cursor para verificar se ele chegou ao final da linha
	if(csr_x >= MAX_COLUNAS)
	{
	   //retorna a primeira posição da linha	
	   csr_x = PRIM_COLUNA;
	   //coloca o cursor na próxima linha
	   csr_y++;
	}
	//move a tela em caso de impressão de apenas um caractere
	//caso uma string seja impresa, não se deve alterar a posição
	//do cursor após impressão de cada caractere, pois resulta em perda
	//de desempenho. Alterar a posição do caractere é uma operação demorada
	//if(mover_cursor_auto) vid_mover_cursor(csr_x, csr_y); 
}

//imprime um caractere na tela 
void vid_imp_char(unsigned char c)
{
	//Faixa de Caracteres imprimíveis
    if( (c >= ' ') && (c <= 127))
    {
        vid_imp_char(c,csr_x, csr_y);
        csr_x++;
    }
    else if(c == BACKSPACE) //Backspace pressionado
    {
		//caso primeiro caractere da linha não tenha sido alcançado
        if(csr_x != PRIM_COLUNA)
		{
		   //decrementa posição na linha
		   csr_x--;
		}
		//primeira posição da linha, mas não na primeira linha
		else if(csr_y != PRIM_LINHA)
		{
			//volta para o final da linha anterior
			csr_x = MAX_COLUNAS - 1;
			csr_y--;
		}
		
		//coloca o espaço vazio na posição atual
		vid_imp_char(' ',csr_x, csr_y);
		
	    //copia todos os dados da tela, uma caracatere para a direta
		//a partir da posição do csr_x e do csr_y atual
	    unsigned short * pos = textmemptr + (csr_y * MAX_COLUNAS + csr_x);
	    memcpy((unsigned char * )pos, (unsigned char *)(pos+1), MAX_COLUNAS*MAX_LINHAS*sizeof(short));
    }
	else if(c == SETA_ESQ) //Seta para esquerda pressionada
	{
		//se não estiver na primeira posição da linha
		if(csr_x != PRIM_COLUNA)
		{
		   //decrementa posição na linha
		   csr_x--;
		}
		//primeira posição da linha, mas não na primeira linha
		else if(csr_y != PRIM_LINHA)
		{
			//volta para o final da linha anterior
			csr_x = MAX_COLUNAS - 1;
			csr_y--;
		}
	}
	else if(c == SETA_DIR)//Seta para direita pressionada
	{
		//avança uma posição na linha
	    csr_x++;
	}
    else if(c == TAB)//tab pressionado
    {
        csr_x = (csr_x + 8) & ~(8 - 1);
    }
    else if(c == '\r' || c == HOME)//home pressionado
    {
	    //volta a primeira posição na linha	
        csr_x = PRIM_COLUNA;
    }
	else if(c == DEL)//Del pressionado
    {  
	   //envia todos os caracteres uma posição para a esquerda	
	   unsigned short * pos = textmemptr + (csr_y * MAX_COLUNAS + csr_x);
	   memcpy((unsigned char * )pos, (unsigned char *)(pos+1), MAX_COLUNAS*MAX_LINHAS*sizeof(short));
    }
    else if(c == '\n' || c == ENTER)//enter pressionado
    {
	   //retorna a primeira posição da linha	
       csr_x = PRIM_COLUNA;
	   //coloca o cursor na próxima linha
       csr_y++;
    }
    
    //testando o o cursor para verificar se ele chegou ao final da linha
    if(csr_x >= MAX_COLUNAS)
    {
	   //retorna a primeira posição da linha	
       csr_x = PRIM_COLUNA;
	   //coloca o cursor na próxima linha
       csr_y++;
    }

    //rola a tela para baixo se necessário
    vid_scroll(&csr_y);
	
	//move a tela em caso de impressão de apenas um caractere
	//caso uma string seja impresa, não se deve alterar a posição
	//do cursor após impressão de cada caractere, pois resulta em perda
	//de desempenho. Alterar a posição do caractere é uma operação demorada
	if(mover_cursor_auto) vid_mover_cursor(csr_x, csr_y); 
}

//imprime um caractere na tela sem alterar a cor atual do fundo
void vid_imp_char_cor(unsigned char c, int x, int y)
{
   //calculando posição em que o caractere será impresso
   short aux;
   unsigned short * pos = textmemptr + (y * MAX_COLUNAS + x);
	
   //aux recebe pos  
   aux = *pos;
   
   //zerando os bits referentes ao caractere
   aux &= 0xFF00; 
   
   //Bitwise or para setar o caractere
   *pos =  c | aux;
}

//--------------------------------FUNÇÕES DE LEITURA E ESCRITA NA MEMÓRIA DE VIDEO-------------------------------------------
void vid_ler_video_mem(char * arq, int x1, int y1, int x2, int y2)
{
	//incrementado valores de y
	y1++; y2++;
	
	if((x1 < 80) && (x2 < 80) && (y1 < 25) && (y2 < 25))
	{
		if((x1 <= x2) && (y1 <= y2))
		{
			int tam_linha = x2 - x1;
			int desc = abrir(arq, 'a');
				
			for(int i = y1; i <= y2; i++)
			{	
				unsigned short * pos = textmemptr + (i * MAX_COLUNAS + x1);
				escrever(desc, (char *)pos, tam_linha*2);
			}
			
			fechar(desc);
		}
	}
}

void vid_esc_video_mem(char * arq, int x1, int y1, int x2, int y2)
{
	//incrementado valores de y
	y1++; y2++;
	
	if((x1 < 80) && (x2 < 80) && (y1 < 25) && (y2 < 25))
	{
		if((x1 <= x2) && (y1 <= y2))
		{
			int tam_linha = (x2 - x1)+1;
			int desc = abrir(arq, 'A');
				
			for(int i = y1; i <= y2; i++)
			{	
				unsigned short * pos = textmemptr + (i * MAX_COLUNAS + x1);
				ler(desc, (char *)pos, tam_linha*2);
			}
			
			fechar(desc);
		}
	}
	
}