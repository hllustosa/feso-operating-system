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
#ifndef _IDE_USR
#define _IDE_USR

struct Bloco_Ide
{
   int codigo;
   int pid;
   int lba;
   int param1;
   int param2;
   int param3;
   unsigned char arquivo[50]; 
};

void ler_setor_ata(unsigned char * arq, int lba);
void escrever_setor_ata(unsigned char * arq, int lba);
void ler_setor_atapi(unsigned char * arq, int lba);
void identificar(unsigned char * arq, unsigned int bus, unsigned int drive, unsigned int tipo);
#endif