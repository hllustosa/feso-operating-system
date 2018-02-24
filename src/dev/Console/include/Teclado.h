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
#ifndef _TECLADO
#define _TECLADO
#include "Lista.h"

//constantes para as teclas
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


#define ALTERAR_CONF_TECLADO 3
#define LER_TECLAS 2

//Struct para determinar o 
//estado do teclado para um terminal
struct Tec_Status
{
	Lista<unsigned char> lista1;
	Lista<unsigned char> lista2;
	int teclado_ativado;
	int ecoamento;
	int modo_canonico;
	int tec_pid_foreground;
	char arquivo[50];	
	
};

extern int tec_pid_foreground;

//inicializar
void tec_inicializar();

//tratamento de pressionamento
int tec_tratar_pressionamento();
void evento_teclado(unsigned char tecla, int ctrl, int alt, int shift);

//atendimento de solicação
void tec_atender_solicitacao(int pid, char * arq);

//configuração de estado
void tec_alterar_estado(int pid, int e, int m);

//recuperação e salvamento de estados
void tec_alterar_processo_foreground(Tec_Status * status);
void tec_recuperar_status(Tec_Status * status);


#endif