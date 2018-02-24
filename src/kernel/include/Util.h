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
#ifndef _UTILIDADES
#define _UTILIDADES

#define NULL 0

/**
Classe que representa um recurso no sistema feSO, e mantém um mutex para controle de acesso assoaciado.
*/
class Recurso
{
	public :

	int qtd;
	int rec_num;	
	int mutex;
	int id_dono;
	
	void down();
	void up();

};

/**
Operação up do mutex.
*/
extern "C" void _up();

/**
Operação down do mutex.
*/
extern "C" void _down(int * mutex);

/**
Função memcpy da biblioteca padrão.
*/
extern "C" unsigned char *memcpy(unsigned char*, unsigned char*, int);

/**
Função memset da biblioteca padrão.
*/
unsigned char *memset(unsigned char *, unsigned char, int);

/**
Função memsetw da biblioteca padrão.
*/
unsigned short *memsetw(unsigned short *, unsigned short, int);

/**
Função strcmp da biblioteca padrão.
*/
int strcmp(const char *s1, const char *s2);

/**
Função strlen da biblioteca padrão.
*/
int strlen(const char *);

/**
Função strtok da biblioteca padrão.
*/
char* strtok(char *s, const char *delim);

/**
Função strcat da biblioteca padrão.
*/
char * strcat(char *dest,  char *src);

//conversão de dados

/**
Função ftoa da biblioteca padrão.
*/
char * ftoa(double valor);

/**
Função strtod da biblioteca padrão.
*/
double strtod(const char *str, char **endptr);

/**
Função atoi da biblioteca padrão.
*/
int atoi( const char *c );

/**
Função itoa da biblioteca padrão.
*/
char* itoa(int val, int base);

/**
Função itoa da biblioteca padrão.
*/
char* itoa2(int val, int base, char * buf);

//leitura de portas

/**
Encapsulamento da função inb em assembly x86.
*/
unsigned char inportb (unsigned short);

/**
Encapsulamento da função outb em assembly x86.
*/
void outportb (unsigned short, unsigned char);

/**
Encapsulamento da função outw em assembly x86.
*/
void outportw(unsigned short port, unsigned short val );

/**
Encapsulamento da função inw em assembly x86.
*/
unsigned short inportw(unsigned short port );

/**
Encapsulamento da função outsw em assembly x86.
*/
void outportsw(unsigned short port, const void *addr, int cnt);

/**
Encapsulamento da função insw em assembly x86.
*/
void inportsw(unsigned short port, void *addr, int cnt);

//misc
/**
Obtém o valor do bit em uma posição específica.
*/
int obter_estado_bit(unsigned char bloco, short index);

/**
Seta o valor do bit em uma posição específica.
*/
void setar_byte(unsigned char *bloco, short index, short val);

/**
Retorna o valor maior entre a e b.
*/
int MAIOR(int a, int b);

#endif
