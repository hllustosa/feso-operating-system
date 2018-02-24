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
#ifndef _SISTIMER
#define _SISTIMER

struct Bloco_Timer
{
	int codigo;
	int pid;
	int param1;
	int param2;
};

struct Relogio
{
	unsigned int hora;
	unsigned int min;
	unsigned int seg;
	unsigned int ms;
	unsigned int ms_dia;
	int bcd;
	int hora24;
	
};

void esperar(int milisegundos);
void obter_relogio(Relogio&);
int rand();

#endif