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
#include "include/Sistimer.h"
#include "include/Sistema.h"
#include "include/Util.h"

static unsigned long int proximo = 1;//usado pelo rand

void esperar(int milisegundos)
{
	unsigned char msg[100];
	
	//criando bloco de comunicação com o servidor
	Bloco_Timer b;
	b.codigo = 1;
	b.pid = obter_pid();
	b.param1 = milisegundos;
	memcpy(msg, (unsigned char *) &b, sizeof(Bloco_Timer));
	
	//enviando mensagem para porta 81 (timer)
	enviar_receber_msg(81, msg);	
}

void obter_relogio(Relogio &r)
{
	unsigned char msg[100];
	
	//criando bloco de comunicação com o servidor
	Bloco_Timer b;
	b.codigo = 2;
	b.pid = obter_pid();
	memcpy(msg, (unsigned char *) &b, sizeof(Bloco_Timer));
	
	//enviando mensagem para porta 81 (timer)
	enviar_receber_msg(81, msg);
	
    memcpy((unsigned char *) &r, msg, sizeof(Relogio));
}

double pit()
{
	double retorno;
	unsigned char msg[100];
	
	//criando bloco de comunicação com o servidor
	Bloco_Timer b;
	b.codigo = 3;
	b.pid = obter_pid();
	memcpy(msg, (unsigned char *) &b, sizeof(Bloco_Timer));
	
	//enviando mensagem para porta 81 (timer)
	enviar_receber_msg(81, msg);
	
    memcpy((unsigned char *) &retorno, msg, sizeof(double));
	return retorno;
}
 
int rand( void ) // RAND_MAX assumed to be 32767
{
    proximo = proximo * 1103515245 + 12345;
    return (unsigned int)(proximo / 65536)/* % 32768*/;//< sem mod, valor max = 2^16
}
 
void srand( unsigned int semente )
{
    proximo = semente;
}