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
#include "include/Sistema.h"
#include "include/Util.h"
#include "include/Stdarg.h"
#include "include/Io.h"

unsigned char buffer[512];
unsigned int pos =0;

char arquivo_out[35];
char arquivo_in[35];

volatile int pid = 0;
volatile int pid_retorno = 0;

struct Bloco_Console
{
	unsigned int codigo;
	unsigned int pid;
	unsigned int param1, param2;
	unsigned int tam;
	unsigned char in[35];
	unsigned char out[35];
}__attribute__((packed));

Bloco_Console criar_bloco(int cod, int param1, int param2, int tam)
{
	//obtendo pid do processo atual
	if(pid == 0)
	{
		pid = obter_pid(obter_pid());
	}
	
	//definindo nomes para arquivos de entrada e saída
	//os nomes tem a forma stdin[pid] e stdout[pid]
	if(strlen(arquivo_out) == 0)
	{
	   //convertendo o pid para string
	   char* num = itoa(pid,10);
	   
	   //criando nome stdout
	   memcpy(arquivo_out, "/dev/stdout", strlen("/dev/stdout")+1);
	   strcat(arquivo_out, num);
	  
	   //criando nome stdin
	   memcpy(arquivo_in, "/dev/stdin", strlen("/dev/stdin")+1);
	   strcat(arquivo_in, num);
	}
	
	//criando bloco
	Bloco_Console bloco; 
	bloco.codigo = cod;
	bloco.pid = pid;
	bloco.param1 = param1;
	bloco.param2 = param2;
	bloco.tam = tam;
	memcpy(bloco.in, arquivo_in, strlen(arquivo_in)+1);
    memcpy(bloco.out, arquivo_out, strlen(arquivo_out)+1);
	
		
	//retornando bloco
	return bloco;
}

void alterar_cor(int texto, int fundo)
{
   unsigned char msg[100];

   //obter pid do processo
   int pid = obter_pid();
  
   //criando dados para enviar ao servidor
   Bloco_Console b = criar_bloco(4, texto, fundo, 0);
   
   //copiando dados para a mensagem	
   memcpy(msg, (unsigned char *)&b, sizeof(Bloco_Console));
   
   //enviando para a porta 80
   enviar_receber_msg(80,msg);
}

void config_cursor(int val)
{  
   //criando dados para enviar ao servidor
   unsigned char msg[100];
   
   //criando bloco
   Bloco_Console b = criar_bloco(5, val, 0, 0);
  
   //copiando dados para a mensagem	
   memcpy(msg, (unsigned char *)&b, sizeof(Bloco_Console));
   
   //enviando para a porta 80
   enviar_receber_msg(80,msg);
}

void posicionar_cursor(int x, int y)
{
   //criando dados para enviar ao servidor
   unsigned char msg[100];
   
   //criando bloco
   Bloco_Console b = criar_bloco(1, x, y, 0);
   
   //copiando dados para a mensagem	
   memcpy(msg, (unsigned char *)&b, sizeof(Bloco_Console));
   
   //enviando para a porta 80
   enviar_receber_msg(80,msg);
}

void cls()
{
   //criando dados para enviar ao servidor
   unsigned char msg[100];
   
   //criando bloco
   Bloco_Console b = criar_bloco(6, 0, 0, 0);
   
   //copiando dados para a mensagem	
   memcpy(msg, (unsigned char *)&b, sizeof(Bloco_Console));
   
   //enviando para a porta 80
   enviar_receber_msg(80,msg);
}


//leitura e escrita na memoria de video
void ler_mem_video(short * dados, int x1, int y1, int x2, int y2)
{
   unsigned char msg[100];

   //unindo x1 e y1 e x2 e y2 em dois parâmetros 16 bits para x e 16 para o y
   int param1 = ((x1 & 0x0000FFFF) << 16) | (y1 & 0x0000FFFF);
   int param2 = ((x2 & 0x0000FFFF) << 16) | (y2 & 0x0000FFFF);
	

   //criando dados para enviar ao servidor
   Bloco_Console b = criar_bloco(7, param1, param2, 0);
   
   //copiando dados para a mensagem	
   memcpy(msg, (unsigned char *)&b, sizeof(Bloco_Console));
   
   //enviando para a porta 80
   enviar_receber_msg(80,msg);
   
   if((x1 < 80) && (x2 < 80) && (y1 < 25) && (y2 < 25))
   {
	  int tam = (x2-x1)+1	* (y2 - y1)+1;
      int desc = abrir(arquivo_out,'A');
	  ler(desc, (char *)dados, tam*2);
	  fechar(desc);
   }
}

void esc_mem_video(short * dados, int x1, int y1, int x2, int y2)
{
   unsigned char msg[100];

   //unindo x1 e y1 e x2 e y2 em dois parâmetros 16 bits para x e 16 para o y
   int param1 = ((x1 & 0x0000FFFF) << 16) | (y1 & 0x0000FFFF);
   int param2 = ((x2 & 0x0000FFFF) << 16) | (y2 & 0x0000FFFF);
	
   //criando dados para enviar ao servidor
   Bloco_Console b = criar_bloco(8, param1, param2, 0);
   
   if((x1 < 80) && (x2 < 80) && (y1 < 25) && (y2 < 25))
   {
	  int tam = (x2-x1+1)*(y2-y1+1);
      int desc = abrir(arquivo_out,'a');
	  escrever(desc, (char *)dados, tam*2);
	  fechar(desc);
   } 
   
   //copiando dados para a mensagem	
   memcpy(msg, (unsigned char *)&b, sizeof(Bloco_Console));
   
   //enviando para a porta 80
   enviar_receber_msg(80,msg);
}

//impressão e leitura de caracteres
void flush()
{
   int serv;
   
   //criando dados para enviar ao servidor
   unsigned char msg[100];

   //criando bloco 
   Bloco_Console b = criar_bloco(0, 0, 0, pos);
   
   //abrindo stdout e escrevendo dados
   int arq = _abrir(arquivo_out, 'a', &serv);
   escrever(arq, buffer, pos);
   _fechar(arq);
   
   //copiando bloco para espaço da mensagem
   memcpy(msg, (unsigned char *)&b, sizeof(Bloco_Console));
   
   //enviando para a porta 80
   enviar_receber_msg(80,msg);
  
   //rebobinando buffer	
   pos = 0;	
}

void puts(unsigned char * texto)
{
  //calculando tamanho da texto
  int tam = strlen(texto);
  
  //caso o texto seja maior que tamanho disponível no buffer
  if((pos + tam) > 512)
  {
	 //enviar dados 
     flush();
  }
  
  if(tam < 512)
  {
	  //copiar dados para a buffer, a partir da posição especificada pela varíavel pos
	  memcpy(&buffer[pos], texto, tam);
	  
	  //incrementando pos com o tamanho de dados escrito no buffer
	  pos+=tam;
  }
  
}

void putch(unsigned char c)
{
   //escrevendo char no buffer	
   buffer[pos] = c;
   
   //incrementando posição no buffer
   pos++;
}

void scanf(const char * mascara, void * param)
{
   //va_list armazena todos os argumentos que são passados à função
   //va_list argumentos;  
   
   //inicializa a busca pelos parâmetros, começando a partir do parâmetro mascara
  // va_start(argumentos, mascara);
   int serv;
   
   //declarando buffer
   char buf_entrada[100];
   
   //criando dados para enviar ao servidor
   unsigned char msg[100];
   
   //criando bloco
   Bloco_Console b = criar_bloco(2, 0, 0, 0);
   
   //copiando dados para a área da mensagem
   memcpy(msg, (unsigned char*)&b, sizeof(Bloco_Console));
   
   //enviando para a porta 80
   enviar_receber_msg(80,msg);
   
   //abrir stdin para ler dados
   int arq = _abrir(arquivo_in, 'A', &serv);
   int ret = ler(arq, buf_entrada, 100);
   _fechar(arq);
   
   //definindo como os dados serão lidos
   if(!strcmp("%d", mascara))
   {
	 //caso expressão seja %d, converter para int
	 //int * valor = va_arg(argumentos, int*);
	 int * valor = param;
	 *valor = atoi(buf_entrada);
   }
   else if(!strcmp("%f", mascara))
   {
     //caso expressão seja %f, converter para double
	 //double * valor = va_arg(argumentos, double*);
	 double * valor = param;
 
	 *valor = strtod(buf_entrada, NULL);
   }
   else if(!strcmp("%c", mascara))
   {
	 //caso expressão seja %c, converter para char
	 //char * valor = va_arg(argumentos, char*);
	 char * valor = param;
	 *valor = buf_entrada[0];
   }
   else if(!strcmp("%s", mascara))
   {
	 //caso expressão seja %s, converter para string
	 //char * valor = va_arg(argumentos, char*);
	 char * valor = param;
	 int tam = strlen(buf_entrada);
	 memcpy(valor, buf_entrada, tam);
   }
   
   //limpar lista de argumentos até o final	  
   //va_end(argumentos);     
   
}

int getchar(char * c)
{
   //criando dados para enviar ao servidor
   unsigned char msg[100];
 
   //criando bloco 
   Bloco_Console b = criar_bloco(3, 0, 1, 0);
 
   //copiando dados para área da mensagem	
   memcpy(msg, (unsigned char *)&b, sizeof(Bloco_Console));
    
   //enviando para a porta 80
   enviar_receber_msg(80,msg);
   
   //armazendo msg[0] que contém o código do caractere pressionado
   *c = msg[0];
	   
   //retornando msg[1] que contém 1 caso uma tecla tenha sido pressionada
   //ou 0 caso uma tecla tenha sido liberada
   return msg[1];
}

//pode ter um número varíavel de parâmetros
void printf(const char * text, ...)
{
   //va_list armazena todos os argumentos que são passados à função
   va_list argumentos;  
   
   //inicializa a busca pelos parâmetros, começando a partir do parâmetro text
   //que é fixo
   va_start(argumentos, text);

   //obtendo  o tamanho do texto a ser impressos
   int tam = strlen(text);  
   
   //variável para controle dos caracteres de escape
   int escape =0;
   
   //for para varrer todos os caracteres especificados em teste
   for(int i =0 ; i <tam; i++)  
   {
         //caso seja encontrado o indicador de caractere de escape 
         if(text[i] == '%') 
         {
		    // se o escape já estiver sendo tratado
			// significa que % foi inserido duas vezes
			// então o caractere de escape deve ser impresso
		    if(escape)
			{
			  escape = 0;
			  putch('%');
			}
			else //senão, ativar flag de caractere de escape
			{
              escape = 1;
			}
            continue;
         }
         
		 //caso a letra d seja encontrado depois de um caracte
		 //de escape, pegar o próximo parâmetro e exibir como integer
         if((text[i] == 'd') && (escape))
         {
            char buf[32]; 
			memset(buf, '\0', 32);
			
			//obter próximo argumento como int
            const int valor = (int)va_arg(argumentos, const int);
			//int val = valor;
            char * snum = itoa2(valor, 10, buf);    
			puts(snum);
            escape = 0;            
            continue;
         }
		 
		  //caso a letra d seja encontrado depois de um caracte
		 //de escape, pegar o próximo parâmetro e exibir como integer
         if((text[i] == 'x') && (escape))
         {
            char buf[32]; 
			memset(buf, '\0', 32);
			
			//obter próximo argumento como int
            const int valor = (int)va_arg(argumentos, const int);
			//int val = valor;
            char * snum = itoa2(valor, 16, buf);    
			puts(snum);
            escape = 0;            
            continue;
         }
		 		 
		 //caso a letra f seja encontrado depois de um caracte
		 //de escape, pegar o próximo parâmetro e exibir como float
		 //controle de precisão não tratada
         if((text[i] == 'f') && (escape))
         {
            char * buf; 
            buf[0] = '\0';
			//obter próximo argumento como double
            double valor = (double)va_arg(argumentos, const double);
            buf = ftoa(valor);    
            //my_printf("%s",buf);   
			puts(buf);
            escape = 0;            
            continue;          
         }
		 
		 
         //caso a letra c seja encontrado depois de um caracte
		 //de escape, pegar o próximo parâmetro e exibir como char
         if((text[i] == 'c') && (escape))
         {
		    //obter próximo argumento como char
            const char valor = (char)va_arg(argumentos, const char);
            putch(valor);   
            escape = 0;           
            continue;
         }
         
         //caso a letra s seja encontrado depois de um caracte
		 //de escape, pegar o próximo parâmetro e exibir como string
         if((text[i] == 's') && (escape))
         {
            //obter próximo argumento como ponteiro para char
            const char * valor = va_arg(argumentos, const char*);
            puts(valor);   
            escape = 0;        
            continue;
         }
          
		 //se nenhum caractere de escape foi encontrado, imprimir caractere normalmente  
		 escape = 0;
         putch(text[i]); 
    }     
    
   //enviar dados para o servidor do console
   flush();   
   
   //limpar lista de argumentos até o final	  
   va_end(argumentos);  
}
