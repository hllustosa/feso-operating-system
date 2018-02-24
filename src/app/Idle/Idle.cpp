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
#include "include/Io.h"
#include "include/Sistema.h"


// processo de tempo ocioso do sistema(idle)
main(int argc, char * args[])
{	
	//ocupando CPU
	for(;;)
		 asm volatile("hlt"); //paralizando CPU até que uma interrupção ocorra
    //sair();
}