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
#include "include/Multiprocessamento.h"

#define NULL 0
#define DBL_DIG        15
#define DBL_EPSILON    2.2204460492503131e-016
#define DBL_MANT_DIG   53
#define DBL_MAX        1.7976931348623158e+308
#define DBL_MAX_10_EXP 308
#define DBL_MAX_EXP    1024
#define DBL_MIN        2.2250738585072014e-308
#define DBL_MIN_10_EXP (-307)
#define DBL_MIN_EXP    (-1021)


/* Classe recurso */

void imprimir(int n, int pos)
{

	unsigned short *textmemptr = 0xC00B8000;
	char num[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
	
	textmemptr[pos] = num[n/100] | (0x0F << 8);
	textmemptr[pos+1] = num[n/10] | (0x0F << 8);
	textmemptr[pos+2] = num[n%10] | (0x0F << 8);

}

void Recurso::down()
{			
	int apicid = multiproc.obter_apicid();
	
	//if(id_dono != apicid)
	{
	   //_down(&mutex);
	   //id_dono = apicid;
	}
}

void Recurso::up()
{
	/*int apicid = multiproc.obter_apicid();
	
	if(id_dono == apicid)
	{		
		//mutex   = 0;
		//id_dono = -1;
	}*/
}


/*Funçõe úteis utilizadas pelos outros módulos
do SO, algumas são implementações da biblioteca padrão do C */
int MAIOR(int a, int b)
{
	return a > b ? a : b;
}

//biblioteca padrão
extern "C" unsigned char *memcpy(unsigned char *dest, unsigned char *src, int count)
{
  int i;
  for(i = 0; i < count; i++)    
  {
    dest[i] = src[i];
  }
  return dest;

}

int strcmp(const char *s1, const char *s2)
{
        while (*s1 == *s2++)
                if (*s1++ == 0)
                        return (0);
        return (*(const unsigned char *)s1 - *(const unsigned char *)(s2 - 1));
}

unsigned char *memset(unsigned char *dest, unsigned char val, int count)
{
   int i;
   for(i = 0; i < count; i++)    
   {
    dest[i] = val;
   }
   return dest;
}

unsigned short *memsetw(unsigned short *dest, unsigned short val, int count)
{
   int i;
   for(i = 0; i < count; i++)    
   {
    dest[i] = val;
   }
   return dest;
}

int strlen(const char *str)
{
  int i=0;
  while(str[i] != '\0')  i++;
  return i;

}

char* strtok(char *s, const char *delim)
{
  const char *spanp;
  int c, sc;
  char *tok;
  static char * last;
   
  if (s == NULL && (s = last) == NULL)
    return (NULL);

  /*
   * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
   */
 cont:
  c = *s++;
  for (spanp = delim; (sc = *spanp++) != 0;) {
    if (c == sc)
      goto cont;
  }

  if (c == 0) {			/* no non-delimiter characters */
    last = NULL;
    return (NULL);
  }
  tok = s - 1;

  /*
   * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
   * Note that delim must have one NUL; we stop if we see that, too.
   */
  for (;;) {
    c = *s++;
    spanp = delim;
    do {
      if ((sc = *spanp++) == c) {
	if (c == 0)
	  s = NULL;
	else
	  s[-1] = 0;
	last = s;
	return (tok);
      }
    } while (sc != 0);
  }
  /* NOTREACHED */
}

char * strcat(char *dest,  char *src)
{
    int i,j;
    for (i = 0; dest[i] != '\0'; i++);
	
    for (j = 0; src[j] != '\0'; j++)
        dest[i+j] = src[j];
		
    dest[i+j] = '\0';
    return dest;
}


//leitura de portas
unsigned char inportb (unsigned short _port)
{
    unsigned char rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

void outportb (unsigned short _port, unsigned char _data)
{
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (_data));
}

void outportw(unsigned short port, unsigned short val )
{
    asm volatile( "outw %0, %1"
                  : : "a"(val), "Nd"(port) );
} 

unsigned short inportw(unsigned short port )
{
    unsigned short ret;
    asm volatile( "inw %1, %0"
                  : "=a"(ret) : "Nd"(port) );
    return ret;
}

void outportsw(unsigned short port, const void *addr, int cnt)
{
   __asm volatile("rep; outsw" : "+S" (addr), "+c" (cnt) : "d" (port));
}
 
void inportsw(unsigned short port, void *addr, int cnt)
{
   __asm volatile(" rep; insw"
       : "+D" (addr), "+c" (cnt)
       : "d" (port)
       : "memory");
}

//conversão de dados
int isdigit(char c )
{
	if((c >= '0') && (c <= '9')) 
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

char* itoa(int val, int base)
{
    static char buf[32];
	
	memset(buf, '\0', 32);
	
    int i = 30;
	int negativo = 0;
	
	if(val < 0)
	{
		negativo = 1;
		val *= -1;
	}
	
    if (val > 0)
	{
		for(; val && i ; --i, val /= base)
			buf[i] = "0123456789abcdef"[val % base];
    }
	else
	{
		buf[i--] = '0';
	}
	
	if(negativo)
	{
		buf[i--] = '-';
	}

    return &buf[i+1];
}

char* itoa2(int val, int base, char * buf)
{
    int i = 30;
	int negativo = 0;
	
	if(val < 0)
	{
		negativo = 1;
		val *= -1;
	}
	
    if (val > 0)
	{
		for(; val && i ; --i, val /= base)
			buf[i] = "0123456789ABCDEF"[val % base];
    }
	else
	{
		buf[i--] = '0';
	}
	
	if(negativo)
	{
		buf[i--] = '-';
	}

    return &buf[i+1];
}

char * ftoa(double valor)
{
   static char fbuf[32] = {0};
   char * buf;
   
   buf = itoa((int)valor, 10);
   
   strcat(fbuf,buf);
   strcat(fbuf,".");
   
   float seg_parte = valor - (int)valor;
   float aux = 0.1;  
    	 
   while(seg_parte < aux && seg_parte > 0)
   {
	  aux /= 10;
	  strcat(fbuf,"0\0");
   }
	 
   while( (seg_parte - (int)seg_parte) != 0 ) 
   {
       seg_parte *=10;    
   }
    
   buf = itoa((int) seg_parte, 10);
   strcat(fbuf,buf);
   return fbuf;
}

double strtod(const char *str, char **endptr)
{
  double number;
  int exponent;
  int negative;
  char *p = (char *) str;
  double p10;
  int n;
  int num_digits;
  int num_decimals;

  // Skip leading whitespace
  while (*p == ' ') p++;

  // Handle optional sign
  negative = 0;
  switch (*p) 
  {             
    case '-': negative = 1; // Fall through to increment position
    case '+': p++;
  }

  number = 0.;
  exponent = 0;
  num_digits = 0;
  num_decimals = 0;

  // Process string of digits
  while (isdigit(*p))
  {
    number = number * 10. + (*p - '0');
    p++;
    num_digits++;
  }

  // Process decimal part
  if (*p == '.') 
  {
    p++;

    while (isdigit(*p))
    {
      number = number * 10. + (*p - '0');
      p++;
      num_digits++;
      num_decimals++;
    }

    exponent -= num_decimals;
  }

  if (num_digits == 0)
  {
    return 0.0;
  }

  // Correct for sign
  if (negative) number = -number;

  // Process an exponent string
  if (*p == 'e' || *p == 'E') 
  {
    // Handle optional sign
    negative = 0;
    switch (*++p) 
    {   
      case '-': negative = 1;   // Fall through to increment pos
      case '+': p++;
    }

    // Process string of digits
    n = 0;
    while (isdigit(*p)) 
    {   
      n = n * 10 + (*p - '0');
      p++;
    }

    if (negative) 
      exponent -= n;
    else
      exponent += n;
  }

  if (exponent < DBL_MIN_EXP  || exponent > DBL_MAX_EXP)
  {
    return -1;
  }

  // Scale the result
  p10 = 10.;
  n = exponent;
  if (n < 0) n = -n;
  while (n) 
  {
    if (n & 1) 
    {
      if (exponent < 0)
        number /= p10;
      else
        number *= p10;
    }
    n >>= 1;
    p10 *= p10;
  }
  
  if (endptr) *endptr = p;

  return number;
}

int atoi( const char *c ) 
{
    int value = 0, negativo =0;
	
	if(*c == '-')
	{
		negativo = 1;
		c++;
	}
	
    while ( isdigit( *c ) ) 
	{
        value *= 10;
        value += (int) (*c-'0');
        c++;
    }
	
	if(negativo) 
		value = value*(-1);
    return value;
}


//outras
int obter_estado_bit(unsigned char bloco, short index)
{
  switch(index)
  {
    case 0 : return (bloco & 0x01) == 0x01 ;break;
    case 1 : return (bloco & 0x02) == 0x02 ;break;
	case 2 : return (bloco & 0x04) == 0x04 ;break;
	case 3 : return (bloco & 0x08) == 0x08 ;break;
	case 4 : return (bloco & 0x10) == 0x10 ;break;
	case 5 : return (bloco & 0x20) == 0x20 ;break;
	case 6 : return (bloco & 0x40) == 0x40 ;break;
	case 7 : return (bloco & 0x80) == 0x80 ;break;
  }
  
  return 0;
}

void setar_byte(unsigned char *bloco, short index, short val)
{
 if(val == 1)
 {
	switch(index)
	{
		case 0 :*bloco = (*bloco | 0x01) ;break;
		case 1 :*bloco = (*bloco | 0x02) ;break;
		case 2 :*bloco = (*bloco | 0x04) ;break;
		case 3 :*bloco = (*bloco | 0x08) ;break;
		case 4 :*bloco = (*bloco | 0x10) ;break;
		case 5 :*bloco = (*bloco | 0x20) ;break;
		case 6 :*bloco = (*bloco | 0x40) ;break;
		case 7 :*bloco = (*bloco | 0x80) ;break;
	}
  }
  else
  {
    switch(index)
	{
		case 0 :*bloco = (*bloco & 0xFE) ;break;
		case 1 :*bloco = (*bloco & 0xFD) ;break;
		case 2 :*bloco = (*bloco & 0xFB) ;break;
		case 3 :*bloco = (*bloco & 0xF7) ;break;
		case 4 :*bloco = (*bloco & 0xEF) ;break;
		case 5 :*bloco = (*bloco & 0xDF) ;break;
		case 6 :*bloco = (*bloco & 0xBF) ;break;
		case 7 :*bloco = (*bloco & 0x7F) ;break;
	}
  }
  
}