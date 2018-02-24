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
#ifndef _MULTIBOOT
#define _MULTIBOOT

/** 
 * Estrutura utilizadas pelo GRUB.
 */
typedef struct ELF_SECTION_HEADER_TABLE
{
   unsigned long num;
   unsigned long size;
   unsigned long addr;
   unsigned long shndx;
};

/** 
 * Estrutura utilizadas pelo GRUB.
 */
typedef struct AOUT_SYMBOL_TABLE
{
   unsigned long tabsize;
   unsigned long strsize;
   unsigned long addr;
   unsigned long reserved;
} ;

/** 
 * Estrutura que contém informações sobre a memória obtidas pelo GRUB.
 */
struct MULTIBOOT_INFO
{
   unsigned long flags;
   unsigned long mem_lower;
   unsigned long mem_upper;
   unsigned long boot_device;
   unsigned long cmdline;
   unsigned long mods_count;
   unsigned long mods_addr;
   union
   {
	 AOUT_SYMBOL_TABLE aout_sym;
	 ELF_SECTION_HEADER_TABLE elf_sec;
   } u;
   unsigned long mmap_length;
   unsigned long mmap_addr;
   
};
 
/** 
 * Estrutura do mapa de memória gerado pelo GRUB.
 */
typedef struct MULTIBOOT_MEMORY_MAP 
{
	unsigned int size;
	unsigned int base_addr_low,base_addr_high;
	unsigned int length_low,length_high;
	unsigned int type;
	
}multiboot_memory_map_t;

/**
 * Estrutura que armazena informações dos módulos carregados pelo GRUB.
 */
struct MULTIBOOT_MOD_LIST
{
   unsigned int mod_start;      /*!<Posição inicial de memória usada pelo módulo.*/
   unsigned int mod_end;        /*!<Posição final de memória usada pelo módulo.*/
   unsigned int cmdline;        /*!<Linha de comando usada pelo módulo.*/
   unsigned int pad;            /*!<Enchimento.*/
};
typedef struct MULTIBOOT_MOD_LIST MULTIBOOT_MOD_LIST_T;
	
#endif	