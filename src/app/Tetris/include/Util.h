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

#define TRUE 1
#define NULL 0

//biblioteca padrão
unsigned char *memcpy(unsigned char*, unsigned char*, int);
unsigned char *memset(unsigned char *, unsigned char, int);
unsigned short *memsetw(unsigned short *, unsigned short, int);
int strcmp(const char *s1, const char *s2);
int strlen(const char *);
char* strtok(char *s, const char *delim);
char * strcat(char *dest,  char *src);

//memoria
void *malloc(unsigned nbytes);
void free(void *ap);

//leitura de portas
unsigned char inportb (unsigned short);
void outportb (unsigned short, unsigned char);
void outportw(unsigned short port, unsigned short val );
unsigned short inportw(unsigned short port );
void outportsw(unsigned short port, const void *addr, int cnt);
void inportsw(unsigned short port, void *addr, int cnt);

//conversão de dados
char * ftoa(double valor);
double strtod(const char *str, char **endptr);
int atoi( const char *c );
char* itoa(int val, int base);
char* itoa2(int val, int base, char * buf);
 

//outras
int obter_estado_bit(unsigned char bloco, short index);
void setar_byte(unsigned char *bloco, short index, short val);
int MAIOR(int a, int b);

#endif
