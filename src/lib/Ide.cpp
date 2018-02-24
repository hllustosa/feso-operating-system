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
#include "include/Ide.h"
#include "include/Sistema.h"
#include "include/Util.h"
#include "include/Io.h"


void ler_setor_ata(unsigned char * arq, int lba)
{
	unsigned char msg[100];
	
	//criando bloco
	Bloco_Ide b;
	b.pid = obter_pid();
	b.codigo = 0;
	b.lba = lba;
	memcpy(b.arquivo, arq, strlen(arq)+1);
	
	//convertendo para mensagem
	memcpy(msg, (unsigned char *)&b, sizeof(Bloco_Ide));
	
	//enviando
	enviar_receber_msg(82, msg);
	
}

void escrever_setor_ata(unsigned char * arq, int lba)
{
	unsigned char msg[100];
	
	//criando bloco
	Bloco_Ide b;
	b.pid = obter_pid();
	b.codigo = 1;
	b.lba = lba;
	memcpy(b.arquivo, arq, strlen(arq)+1);
	
	//convertendo para mensagem
	memcpy(msg, (unsigned char *)&b, sizeof(Bloco_Ide));
	
	//enviando
	enviar_receber_msg(82, msg);
}

void ler_setor_atapi(unsigned char * arq, int lba)
{
	unsigned char msg[100];
	
	//criando bloco
	Bloco_Ide b;
	b.pid = obter_pid();
	b.codigo = 2;
	b.lba = lba;
	memcpy(b.arquivo, arq, strlen(arq)+1);
	
	//convertendo para mensagem
	memcpy(msg, (unsigned char *)&b, sizeof(Bloco_Ide));
	
	//enviando
	enviar_receber_msg(82, msg);
}

void identificar(unsigned char * arq, unsigned int bus, unsigned int drive, unsigned int tipo)
{
	unsigned char msg[100];
	
	//criando bloco
	Bloco_Ide b;
	b.pid = obter_pid();
	b.codigo = 3;
	b.param1 = bus;
	b.param2 = drive;
	b.param3 = tipo;
	memcpy(b.arquivo, arq, strlen(arq)+1);
	
	//convertendo para mensagem
	memcpy(msg, (unsigned char *)&b, sizeof(Bloco_Ide));
	
	//enviando
	enviar_receber_msg(82, msg);
	
}

