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
#ifndef _VIDEO
#define _VIDEO

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


#define IMPRIMIR 0
#define ALTERAR_POS 1
#define CONFIG_COR 4
#define HABILITAR_CURSOR 5
#define LIMPAR_TELA 6
#define LER_VIDEO_MEM 7
#define ESC_VIDEO_MEM 8

//estrutura para armazenar os dados do terminal
struct Vid_Status
{
	short * tela;
	int  atributo;
	int  cursor_habilitado;
	int  mover_cursor_auto;
	int  vid_pid_foreground;
	int  csr_x;
	int	 csr_y;
	char vid_arquivo_out[50];
	char terminal_backup[50];
};

extern int vid_pid_foreground;

//inicilizar
void vid_inicializar();

//carregar e salvar informações sobre o terminal
void vid_alterar_processo_foreground(Vid_Status * status);
void vid_recuperar_status(Vid_Status * status);
void vid_atender_solicitacao(int cod, char * out, int param1, int param2, int tam);
void vid_mudar_para_background(Vid_Status * s, Vid_Status * status_background);
void vid_mudar_para_foreground(Vid_Status * s, Vid_Status * status_background);

//funções de configuração
void vid_limpar_linha(unsigned int caracteres);
void vid_limpar_tela();
void vid_habilitar_cursor(short estado);
void vid_mover_cursor(short x, short y);
void vid_config_color(short cor_texto, short cor_background);
void vid_scroll(int*);
void vid_alterar_cor_fundo(short cor, int x, int y);
void vid_alterar_cor_fonte(short cor, int x, int y);

//impressão
void vid_imp_char(unsigned char c, int x, int y);
void vid_imp_char(unsigned char c);
void vid_imp_char_cor(unsigned char c, int x, int y);
void vid_imprimir_string(char * texto, int tam);
void vid_imprimir_string_pos(char * texto, int tam, int x, int y);

//acesso a memoria de video
void vid_ler_video_mem(char * arq, int x1, int y1, int x2, int y2);
void vid_esc_video_mem(char * arq, int x1, int y1, int x2, int y2);

#endif