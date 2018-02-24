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
#ifndef _IO
#define _IO

//constantes para o teclado
#define ESC 				0x1B
#define F1    				0xE0
#define F2     				0xE1
#define F3  				0xE2
#define F4  				0xE3
#define F5  				0xE4
#define F6  				0xE5
#define F7  				0xE6
#define F8  				0xE7
#define F9   				0xE8
#define F10 				0xE9
#define F11					0xEA
#define F12         		0xEB
#define BACKSPACE			0x08
#define CAPS				0xEC
#define TAB					0x09 
#define ALT         		0xED
#define ALT_GR      		0xEE
#define CTRL_ESQ    		0xEF
#define CTRL_DIR    		0xF0 
#define SHIFT				0xF1
#define ENTER               0x0A
#define BARRA_ESPACO 	    0x20
#define HOME         		0xF3
#define END          		0xF4
#define PG_UP        		0xF5
#define PG_DW        		0xF6
#define SETA_PARA_CIMA		0xF7
#define SETA_PARA_BAIXO     0xF8
#define SETA_ESQ			0xF9
#define SETA_DIR			0xFA
#define AGUDO				0x7F 
#define TIL					0x7E
#define CRASE				0x60
#define CIRCUNFLEXO			0x5E
#define TREMA				0xA8
#define CCEDILHA_MIN		0x87
#define CCEDILHA_MAI		0x80
#define NUM_LOCK			0xFB
#define SCR_LOCK			0xFC
#define PRT_SCR				0xFD
#define DEL					0xFE
#define PAUSE				0xFF
#define INSERT				0xDF

//constantes para o video
#define VID_COLOR_PRETO     0x00
#define VID_COLOR_AZUL      0x01
#define VID_COLOR_VERDE     0x02
#define VID_COLOR_CIANO     0x03
#define VID_COLOR_VERMELHO  0x04
#define VID_COLOR_MARGENTA  0x05
#define VID_COLOR_MARRON    0x06
#define VID_COLOR_CZA_CLARO 0x07
#define VID_COLOR_CZA_ESCUR 0x08
#define VID_COLOR_AZL_CLARO 0x09
#define VID_COLOR_VED_CLARO 0x0A
#define VID_COLOR_CIA_CLARO 0x0B
#define VID_COLOR_VER_CLARO 0x0C
#define VID_COLOR_MAR_CLARO 0x0D
#define VID_COLOR_MRO_CLARO 0x0E
#define VID_COLOR_BRANCO 	0x0F

int getchar(char * c);	
void puts(unsigned char * texto);
void scanf(const char * mascara, void *param); 
void printf(const char * text, ...);
void alterar_cor(int texto, int fundo);
void config_cursor(int val);
void posicionar_cursor(int x, int y);
void ler_mem_video(short * dados, int x1, int y1, int x2, int y2);
void esc_mem_video(short * dados, int x1, int y1, int x2, int y2);
void cls();


#endif
