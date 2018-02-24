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
#ifndef _LOADER_H
#define _LOADER_H

#include "Lista.h"

//Tipos de seções em um arquivo ELF
#define	SF_ZERO		0x10	// BSS 
#define	SF_LOAD		0x08	// carregar do arquivo 
#define	SF_READ		0x04	// readable 
#define	SF_WRITE	0x02	// writable 
#define	SF_EXEC		0x01	// executável 
#define	SF_BSS		(SF_ZERO | SF_READ | SF_WRITE)

/**
*Estrutura do cabeçalho do arquivo ELF. Valores especficados em: https://wiki.osdev.org/ELF
*/
typedef struct ELF_FILE
{
	unsigned char magic[4];
	unsigned char bitness;
	unsigned char endian;
	unsigned char ver_1;
	unsigned char res[9];
	unsigned short file_type;
	unsigned short machine;
	unsigned int ver_2;
	unsigned int entry;
	unsigned int phtab_offset;
	unsigned int shtab_offset;
	unsigned int flags;
	unsigned short file_hdr_size;
	unsigned short phtab_ent_size;
	unsigned short num_phs;
	unsigned short shtab_ent_size;
	unsigned short num_shs;
	unsigned short shstrtab_index;
} __attribute__((packed));

typedef ELF_FILE ELF_FILE_T;

/**
*Estrutura do cabeçalho de um segmento de arquivo ELF.
*/
struct ELF_SEG
{
	unsigned int type;
	unsigned int offset;
	unsigned int virt_adr;
	unsigned int phys_adr;
	unsigned int disk_size;
	unsigned int mem_size;
	unsigned int flags;
	unsigned int align;
}  __attribute__((packed));

typedef ELF_SEG ELF_SEG_T; 

/**
*Estrutura do cabeçalho de um seção de um arquivo ELF.
*/
struct ELF32_SHDR_S
{
  unsigned int	sh_name;		/*!<Nome da seção.*/
  unsigned int	sh_type;		/*!<Tipo da seção.*/
  unsigned int	sh_flags;		/*!<Flags.*/
  unsigned int	sh_addr;		/*!<Endereço virtual para execução.*/
  unsigned int	sh_offset;		/*!<Offset da seção dentro do arquivo.*/
  unsigned int	sh_size;		/*!<Tamanho da seção em bytes.*/
  unsigned int	sh_link;		/*!<Link para outra seção.*/
  unsigned int	sh_info;		/*!<Informações Adicionais.*/
  unsigned int	sh_addralign;		/*!<Alinhamento da seção.*/
  unsigned int	sh_entsize;		/*!<Tamanho do registro.*/
};

typedef ELF32_SHDR_S ELF32_SHDR;
/**
*Função para carregamento de arquivos elf.
*/
Lista<ELF_SEG_T> load_elf_exec(unsigned char *imagem, unsigned int *ponto_entrada, Lista<ELF_SEG_T> secoes);

#endif
