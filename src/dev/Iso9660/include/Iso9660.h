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
#ifndef _ISO_9660
#define _ISO_9660

#define TAM_SETOR

//Estrutura que descrever o VOLUME
struct Cvoldesc 
{
	unsigned char	magic[8];	        //Números mágicos 0x01, "CD001", 0x01, 0x00
	unsigned char	systemid[32];	    //id do sistema
	unsigned char	volumeid[32];	    //id do volume
	unsigned char	unused[8];		    //conjunto de caracteres
	unsigned char	volsize[8];		    //tamanho do volume
	unsigned char	charset[32];
	unsigned char	volsetsize[4];	    //volume set size = 1 
	unsigned char	volseqnum[4];	    //volume sequence number = 1 
	unsigned char	blocksize[4];	    //tamanho do bloco/
	unsigned char	pathsize[8];	    //tamanho da path table
	unsigned char	lpathloc[4];	    //lpath
	unsigned char	olpathloc[4];	    //lpath opcional
	unsigned char	mpathloc[4];	    //mpath
	unsigned char	ompathloc[4];	    //mpath opcional
	unsigned char	rootdir[34];	    //entrada do diretório raiz
	unsigned char	volumeset[128];	    //id do volume set 
	unsigned char	publisher[128];
	unsigned char	preparer[128];	    //id do data preparer
	unsigned char	application[128];	//id da aplicação
	unsigned char	notice[37];	        //aviso de copyright
	unsigned char	abstract[37];	    //arquivo abstrato
	unsigned char	biblio[37];	        //arquivo bibliográfico
	unsigned char	cdate[17];	        //data de criação
	unsigned char	mdate[17];	        //data de modificação
	unsigned char	xdate[17];	        //data de expiração
	unsigned char	edate[17];	        //data efetiva
	unsigned char	fsvers;		        //versão do sistema de arquivos = 1
}__attribute__((packed));


struct Cpath 
{
	unsigned char   namelen;
	unsigned char   xlen;
	unsigned char   dloc[4];
	unsigned char   parent[2];
	unsigned char   name[1];
}__attribute__((packed));

struct Cdir 
{
	unsigned char	len;
	unsigned char	xlen;
	unsigned char	dloc[8];
	unsigned char	dlen[8];
	unsigned char	date[7];
	unsigned char	flags;
	unsigned char	unitsize;
	unsigned char	gapsize;
	unsigned char	volseqnum[4];
	unsigned char	namelen;
	unsigned char	name[1];
}__attribute__((packed));

struct CArquivo
{
	char val[256];
};

struct No
{
   unsigned int lba;
   unsigned int tam;
   unsigned int descritor;
   char * nome;
};



#endif