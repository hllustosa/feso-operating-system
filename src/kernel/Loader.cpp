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
#include "include/Loader.h"

//Executa a an�lise de um imagem de arquivo e retorna uma lista de se��es caso seja um arquivo ELF v�lido
Lista<ELF_SEG_T> load_elf_exec(unsigned char *imagem, unsigned int *ponto_entrada, Lista<ELF_SEG_T> secoes)
{
	ELF_FILE_T * elf;
	ELF_SEG_T * seg;
	int i = 0, j =0;

    //obtendo cabe�alho do arquivo
	elf = (ELF_FILE_T *)imagem;
	
	//verificando se n�mero m�gico 0x7F e assinatura ELF
	if(elf->magic[0] != 0x7F || elf->magic[1] != 'E' ||
		elf->magic[2] != 'L' || elf->magic[3] != 'F')
			return secoes;
			
	//Verificando se um arquivo ELF v�lido (Estaticamento Linkado)		
	if( elf->bitness   != 1 ||	// 1=32-bit, 2=64-bit 
		elf->endian    != 1 ||	// 1=little-endian, 2= big-endian
		elf->ver_1     != 1 ||	// 1=current ELF spec 
		elf->file_type != 2 ||	// 1=reloc, 2=exec, 3=DLL 
		elf->machine   != 3 ||	// 3=i386 */
		elf->ver_2     != 1)
			return secoes;
		
    //obtendo o ponto de entrada do arquivo execut�vel
	(*ponto_entrada) = elf->entry;
	
	//buscando cabe�alho das se��es do arquivo
	imagem += elf->phtab_offset;
	
	//varrendo se��es do arquivo
	for(i = 0; i < elf->num_phs; i++)
	{
		//seg aponta para o o cabe�alho da se��o
		seg = (ELF_SEG_T *)(imagem + elf->phtab_ent_size * i);
			
		//Uma se��o do tipo 2=DYNAMIC e 5=SHLIB indicam que o arquivo
		//n�o � estaticamente linkado, e por isso n�o pode ser carregado
		if(seg->type == 2 || seg->type == 5)
			return secoes;
		else if(seg->type == 1) //tipo 1=LOAD (segmento que deve ser carregado na mem�ria)
		{
		    unsigned char * pos = (unsigned char *)elf;
			pos +=seg->offset;
			
		    //adicionando nova se��o � lsita
			secoes.adicionar(*seg);

		}//fim do else
		
	}//fim do for
	
	//retornando lista de se��es
	return secoes;
}
