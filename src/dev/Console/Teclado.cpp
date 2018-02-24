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
#include "include/Video.h"
#include "include/Teclado.h"
#include "include/Util.h"
#include "include/Sistema.h"
#include "include/Lista.h"

//controle do cliente atual
Lista<unsigned char> lista1;
Lista<unsigned char> lista2;
Lista< Lista<unsigned char> > log_comandos;
int posicao_log = 0;
int teclado_ativado = 0;
int ecoamento       = 1;
int modo_canonico   = 0;
int tec_pid_foreground  = 0;
char arquivo[50];		

//controle de pressionamento de teclas e acentos
int ctrl_press  = 0;
int alt_press   = 0;
int shift_press = 0;
int caps_estado = 0; 
int agudo = 0, til = 0, circ = 0, trema = 0, crase = 0; 


//--------------------------------LAYOUT DAS TECLAS-------------------------------------------
//layout comum do teclado
unsigned char layout[128] =
{
   0,  ESC, '1', '2', '3', '4', '5', '6', '7', '8',	'9', '0', '-', '=', BACKSPACE,	  // 0-14
  TAB, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', AGUDO, '[', ENTER, CTRL_ESQ, //15-29
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', CCEDILHA_MIN, TIL, '\'' ,SHIFT,   //30-42 
  ']', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', SHIFT, //43-54				
  '*', ALT,	BARRA_ESPACO, CAPS, //55-58	
   F1, F2, F3, F4, F5, F6, F7, F8, F9, F10,	 //59-68
   NUM_LOCK, SCR_LOCK,  HOME, SETA_PARA_CIMA, PG_UP, '-', SETA_ESQ, '5', SETA_DIR, '+', END, SETA_PARA_BAIXO, PG_DW, //69-81
   INSERT, DEL , 0, 0, '\\',F11, F12, '/', 0, 0, 0, 0, 0, 0, ENTER, //82-96
   CTRL_DIR, '/', PRT_SCR, ALT_GR, 0, HOME, SETA_PARA_CIMA,	//97-103
   PG_UP, SETA_ESQ, SETA_DIR, END, SETA_PARA_BAIXO,//104-108
   PG_DW, INSERT, DEL, 0, 0, 0, '/' // 109-115 
};

//layout quando CAPS LOCK ESTÁ ATIVADO
unsigned char layout_caps[128] =
{
   0,  ESC, '1', '2', '3', '4', '5', '6', '7', '8',	'9', '0', '-', '=', BACKSPACE,	  // 0-14
  TAB, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', AGUDO, '[', ENTER, CTRL_ESQ, //15-29
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', CCEDILHA_MAI, TIL, '\'' ,SHIFT,   //30-42 
  ']', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', SHIFT, //43-54				
  '*', ALT,	BARRA_ESPACO, CAPS, //55-58	
   F1, F2, F3, F4, F5, F6, F7, F8, F9, F10,	 //59-68
   NUM_LOCK, SCR_LOCK,  HOME, SETA_PARA_CIMA, '9', '-', SETA_ESQ, '5', SETA_DIR, '+', END, SETA_PARA_BAIXO, PG_DW, //69-81
   INSERT, DEL , 0, 0, '\\',F11, F12, '/', 0, 0, 0, 0, 0, 0, ENTER, //82-96
   CTRL_DIR, '/', PRT_SCR, ALT_GR, 0, HOME, SETA_PARA_CIMA,	//97-103
   PG_UP, SETA_ESQ, SETA_DIR, END, SETA_PARA_BAIXO,//104-108
   PG_DW, INSERT, DEL, 0, 0, 0, '/' // 109-115 
};

//layout quando shift está sendo pressionado
unsigned char layout_shift[128] =
{
   0,  ESC, '!', '@', '#', '$', '%', TREMA, '&', '*', '(', ')', '_', '+', BACKSPACE,	  // 0-14
  TAB, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', CRASE, '{', ENTER, CTRL_ESQ, //15-29
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', CCEDILHA_MAI, CIRCUNFLEXO, '"' ,SHIFT,   //30-42 
  '}', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', ':', SHIFT, //43-54				
  '*', ALT,	BARRA_ESPACO, CAPS, //55-58	
   F1, F2, F3, F4, F5, F6, F7, F8, F9, F10,	 //59-68
   NUM_LOCK, SCR_LOCK,  HOME, SETA_PARA_CIMA, '9', '-', SETA_ESQ, '5', SETA_DIR, '+', END, SETA_PARA_BAIXO, PG_DW, //69-81
   INSERT, DEL , 0, 0, '|',F11, F12, '/', 0, 0, 0, 0, 0, 0, ENTER, //82-96
   CTRL_DIR, '?', PRT_SCR, ALT_GR, 0, HOME, SETA_PARA_CIMA,	//97-103
   PG_UP, SETA_ESQ, SETA_DIR, END, SETA_PARA_BAIXO,//104-108
   PG_DW, INSERT, DEL, 0, 0, 0, '?' // 109-115 
};

//--------------------------------FUNÇÕES PARA ATENDER SOLICITAÇÃO E TROCAR PROCESSO FOREGROUND-------------------------------------------
void tec_inicializar()
{
	char ant = -1, prox = 0;

	while (ant != prox)
	{ 
		ant = prox;
		prox = inportb(0x60);
	}
	 
	while ((inportb(0x60) & 1) != 0);

	outportb(0x64,0xF3);
	outportb(0x60,0x00);
}

void tec_alterar_processo_foreground(Tec_Status * status)
{
   lista1.limpar();
   for(int i =0; i< status->lista1.tamanho(); i++)
		lista1.adicionar(status->lista1[i]);
		
   lista2.limpar();
   for(int i =0; i< status->lista2.tamanho(); i++)
		lista2.adicionar(status->lista2[i]);
   
   teclado_ativado     = status->teclado_ativado;
   ecoamento           = status->ecoamento;
   modo_canonico       = status->modo_canonico;
   tec_pid_foreground  = status->tec_pid_foreground;
   memcpy(arquivo, status->arquivo, strlen(status->arquivo)+1); 
   
}

void tec_recuperar_status(Tec_Status * status)
{
   status->lista1.limpar();
   for(int i =0; i< lista1.tamanho(); i++)
		status->lista1.adicionar(lista1[i]);
		
   status->lista2.limpar();
   for(int i =0; i< lista2.tamanho(); i++)
		status->lista2.adicionar(lista2[i]);		
		
   status->teclado_ativado    = teclado_ativado;
   status->ecoamento       	  = ecoamento;
   status->modo_canonico      = modo_canonico;
   status->tec_pid_foreground = tec_pid_foreground; 
   memcpy(status->arquivo, arquivo,  strlen(arquivo)+1);
}

void tec_alterar_estado(int pid, int e, int m)
{
	ecoamento       = e;
    	modo_canonico   = m;
	teclado_ativado = 1;
	tec_pid_foreground = pid;
}

void tec_atender_solicitacao(int pid, char * arq)
{
	teclado_ativado = 1;
	modo_canonico   = 0;
	tec_pid_foreground = pid;
	memcpy(arquivo, arq, strlen(arq));
	vid_habilitar_cursor(1);
}


//--------------------------------FUNÇÕES DE CONTROLE DO TECLADO-------------------------------------------

//funções para ligar e delisgar o LED do caps lock
void tec_espera() 
{
   asm volatile("1:inb   $0x64, %al\n" \
				"  testb $0x02, %al\n" \
				"  jne   1b");
}

void tec_ligar_caps()
{
   while ((inportb(0x64) & 2) != 0);
   outportb(0x60,0xED);
   while ((inportb(0x64) & 2) != 0);
   outportb(0x60,0x04);
   while ((inportb(0x64) & 2) != 0);
}

void tec_des_caps()
{
   while ((inportb(0x64) & 2) != 0);
   outportb(0x60,0xED);
   outportb(0x60,0x00);
}

//função para imprimir caracteres acentuados
int tec_caractere_acentuado(unsigned char c)
{
   int char_imp = 1;
   
   if(agudo)
   {
       agudo = 0;
       switch(c)
	   {
		 case 'a' : evento_teclado(0xA0, ctrl_press, alt_press, shift_press); break;
		 case 'i' : evento_teclado(0x8D, ctrl_press, alt_press, shift_press); break;
		 case 'e' : evento_teclado(0x82, ctrl_press, alt_press, shift_press); break;
		 case 'o' : evento_teclado(0x95, ctrl_press, alt_press, shift_press); break;
		 case 'u' : evento_teclado(0x97, ctrl_press, alt_press, shift_press); break;
		 default  : evento_teclado(c, ctrl_press, alt_press, shift_press); char_imp = 0; break;
	   }
   }
   else if(til)
   {
	   til = 0;
       char_imp = 0;
   }
   else if(circ)
   {
       circ = 0;
       switch(c)
	   {
		 case 'a' : evento_teclado(0x83, ctrl_press, alt_press, shift_press); break;
		 case 'e' : evento_teclado(0x88, ctrl_press, alt_press, shift_press); break;
		 case 'i' : evento_teclado(0x8C, ctrl_press, alt_press, shift_press); break;
		 case 'o' : evento_teclado(0x93, ctrl_press, alt_press, shift_press); break;
		 case 'u' : evento_teclado(0x96, ctrl_press, alt_press, shift_press); break;
		 default : evento_teclado(c, ctrl_press, alt_press, shift_press); char_imp = 0; break;
	   }
   }
   else if(trema)
   {
       trema = 0;
       switch(c)
	   {
		 case 'a' : evento_teclado(0x84, ctrl_press, alt_press, shift_press); break;
		 case 'e' : evento_teclado(0x89, ctrl_press, alt_press, shift_press); break;
		 case 'i' : evento_teclado(0x8B, ctrl_press, alt_press, shift_press); break;
		 case 'o' : evento_teclado(0x94, ctrl_press, alt_press, shift_press); break;
		 case 'u' : evento_teclado(0x81, ctrl_press, alt_press, shift_press); break;
		 default : evento_teclado(c, ctrl_press, alt_press, shift_press); char_imp = 0; break;
	   }
   }
   else if(crase)
   {   
       trema = 0;
       switch(c)
	   {
		 case 'a' : evento_teclado(0x85, ctrl_press, alt_press, shift_press); break;
		 case 'e' : 
		 case 'i' : 
		 case 'o' : 
		 case 'u' : evento_teclado(c, ctrl_press, alt_press, shift_press); break;
		 default  : evento_teclado(c, ctrl_press, alt_press, shift_press); char_imp = 0; break;
	   }
   
   }
   else
   {
     char_imp =0;
   }
   
  
   return char_imp;
  
}

//função de tratamento do teclado
int tec_tratar_pressionamento()
{
    //comando de troca processo em foreground
    int trocar_processo = 0;
	
    //lendo porta 60 para obter scancode
    unsigned char scancode;
    scancode = inportb(0x60);

	/*caso os maior bit esteja setado, 
	uma tecla foi liberada ao invés de pressionada*/
    if (scancode & 0x80)
    {
	  //zerando o primeiro bit
	  scancode &= 0x7F;
	  
	  //verificando se CTRL ou ALT ou SHIFT foram liberadas
	  if( (layout[scancode] == CTRL_ESQ) || (layout[scancode] == CTRL_DIR) )
	  {
	     ctrl_press = 0;
	  }
	  else if (layout[scancode] == SHIFT)
	  {
	     shift_press = 0;
	  }
	  else if( (layout[scancode] == ALT) || (layout[scancode] == ALT_GR) )
	  {
	     alt_press = 0;
	  }
	  
    }
    else if(evento_teclado) // Caso um tecla tenha sido pressionada e função evento_teclado esteja definida
    {   		
	    //Verifica se o CAPS LOCK foi pressionada
	    if (layout[scancode] == CAPS)
		{
		   //caso tenha sido, inverte o estado atual
		   caps_estado = !caps_estado;

		   //ligar ou desligar caps lock
		   if(caps_estado)
		     tec_ligar_caps();
		   else
		     tec_des_caps();
			 
			evento_teclado(0, ctrl_press, alt_press, shift_press); 
	    }
		else if((layout[scancode] == TAB) && alt_press)
	    {
			//caso alt e tab seja pressionada
			//retornar 1 para alterar processo em foreground
			trocar_processo = 1;
	    }
		//verificando se CTRL ou ALT ou SHIFT foram pressionadas
		else if( (layout[scancode] == CTRL_ESQ) || (layout[scancode] == CTRL_DIR) )
		{
			ctrl_press = 1;
			evento_teclado(0, ctrl_press, alt_press, shift_press);
	    }
	    else if (layout[scancode] == SHIFT)
		{
			shift_press = 1;
			evento_teclado(0, ctrl_press, alt_press, shift_press);
		}
		else if( (layout[scancode] == ALT) || (layout[scancode] == ALT_GR) )
		{
			alt_press = 1;
			evento_teclado(0, ctrl_press, alt_press, shift_press);
		}
		else
		{
		   if(shift_press)
		   {
		       if( (layout_shift[scancode] == CIRCUNFLEXO) && !circ )
			   {
			      circ = 1;
			   }
		       else if( (layout_shift[scancode] == TREMA) && !trema)
			   {
			      trema = 1;
			   }
			   else if( (layout_shift[scancode] == CRASE) && !crase)
			   {
			      crase = 1;
			   }
			   else
			   {
			     //if(!tec_caractere_acentuado(layout[scancode]))
				 evento_teclado(layout_shift[scancode], ctrl_press, alt_press, shift_press);
			   }			   
		   }
		   else if(caps_estado)
		   {
		       if( (layout_caps[scancode] == TIL) && !til )
			   {
			      til = 1;
			   }
		       else if( (layout_caps[scancode] == AGUDO) && !agudo)
			   {
			      agudo = 1;
			   }
			   else
			   {
			     //if(!tec_caractere_acentuado(layout[scancode]))
				 evento_teclado(layout_caps[scancode], ctrl_press, alt_press, shift_press);
			   }
		   }
		   else
		   {
		       if( (layout[scancode] == TIL) && !til )
			   {
			      til = 1;
			   }
		       else if( (layout[scancode] == AGUDO) && !agudo)
			   {
			      agudo = 1;
			   }
			   else
			   {
			     //if(!tec_caractere_acentuado(layout[scancode]))
		         evento_teclado(layout[scancode], ctrl_press, alt_press, shift_press);
			   }
		   }
		}
		
    }
	
	return trocar_processo;
}

//corrige o pressionamento das teclas para criar o buffer usando duas filas de caracteres
void tec_corrigir_pressionamento(unsigned char c)
{
    //Backspace pressionado
    if(c == BACKSPACE)
    {
		//remover caractere do fim da primeira lista
        if(lista1.tamanho() > 0)
		{
			lista1.remover(lista1.tamanho() -1);
			vid_imp_char(c);
		}
	
    }
	else if(c == SETA_PARA_CIMA)
	{
		if (posicao_log < 0 && log_comandos.tamanho() >= 0)
			posicao_log = 0;
			
		if ((posicao_log < log_comandos.tamanho()) && (posicao_log >= 0))
		{
			int qnt_caracteres = lista1.tamanho() + lista2.tamanho();
			int i;
			int tamanho_comando = log_comandos[posicao_log].tamanho();
			unsigned char caracter;
			for (i=0;i<qnt_caracteres;i++)
			{
				vid_imp_char(BACKSPACE);
			}
			lista1.limpar();
			lista2.limpar();
			for(i=0;i<tamanho_comando;i++)
			{
				caracter = log_comandos[posicao_log][i];
				lista1.adicionar(caracter);
				vid_imp_char(caracter);
			}
			posicao_log++;
		}
	}
	else if(c == SETA_PARA_BAIXO)
	{
		if (posicao_log >= log_comandos.tamanho() && posicao_log > 0)
		{
			posicao_log = log_comandos.tamanho()-2;
		}	
		if ((posicao_log >= 0) && (log_comandos.tamanho() > 0))
		{
			unsigned int qnt_caracteres = lista1.tamanho() + lista2.tamanho();
			unsigned int i;
			unsigned int tamanho_comando = log_comandos[posicao_log].tamanho();
			unsigned char caracter;
			for (i=0;i<qnt_caracteres;i++)
			{
				vid_limpar_linha(qnt_caracteres);
			}
			lista1.limpar();
			lista2.limpar();
			for(i=0;i<tamanho_comando;i++)
			{
				caracter = log_comandos[posicao_log][i];
				lista1.adicionar(caracter);
				vid_imp_char(caracter);
			}
			posicao_log--;
		}
		else
		{
			unsigned int qnt_caracteres = lista1.tamanho() + lista2.tamanho();
			for (int i=0;i<qnt_caracteres;i++)
			{
				vid_imp_char(BACKSPACE);
			}
			lista1.limpar();
			lista2.limpar();
		}	
	}
	else if(c == SETA_ESQ)
	{
		if(lista1.tamanho() > 0)
		{
			//remover caractere do fim da primeira lista
			char val = lista1[lista1.tamanho() -1];
			lista1.remover(lista1.tamanho() -1);
			
			//adicionar no começo da segunda lista
			lista2.adicionar_em(0, val);
			
			//enviar tratamento para controlador do video
			vid_imp_char(c);
		}
	}
	else if(c == SETA_DIR)
	{
		if(lista2.tamanho() > 0)
		{
			//remover caractere do começo da segunda lista
			vid_imp_char(c);
			char val = lista2[0];
			lista2.remover(0);
			
			//adicionar ao final da primeira 
			lista1.adicionar(val);
		}
	}
    else if(c == '\r' || c == HOME)
    {
	    //remove todos os caracteres do final da
		//lista 1 e adiciona no começo da lista dois
        while(lista1.tamanho() > 0 )
		{
			char val = lista1[lista1.tamanho()-1];
			lista1.remover(lista1.tamanho()-1);
			lista2.adicionar_em(0, val);
			vid_imp_char(SETA_ESQ);
		}
    }
	else if(c == END)
	{
		//remove todos os caracteres da
		//lista 2 e adiciona no final da lista 1
		while(lista2.tamanho() > 0)
		{
			char val = lista2[0];
			lista2.remover(0);
			lista1.adicionar(val);
			vid_imp_char(SETA_DIR);		
		}
		
	}
	else if(c == DEL)
	{
		if(lista2.tamanho() > 0)
		{
			//remove um caractere do começo
			//da segunda lista
			lista2.remover(0);
			vid_imp_char(DEL);
		}
	}
    else if(c >= ' ' && c <= 127)
    {
		//imprime um caractere
        vid_imp_char(c);		
		
		//adiciona ao final da lista1
		lista1.adicionar(c);
		
		//caso existam caracteres na lista2, eles devem ser
		//reimpressos uma posição para direita
		if(lista2.tamanho() > 0)
		{ 	
			//reimprimindo os caracteres da lista2
			for(int i = 0; i < lista2.tamanho(); i++)
			{
				vid_imp_char(lista2[i]);
			}
			
			//andando com o cursor para a esquerda
			for(int i = 0; i < lista2.tamanho(); i++)
			{
				vid_imp_char(SETA_ESQ);
			}
		}
    }

}

void evento_teclado(unsigned char tecla, int ctrl, int alt, int shift)
{
	char mensagem[100];
	if(teclado_ativado)
	{
		//no modo canonico, fazer a correção antes de enviar para a tela
		if(!modo_canonico)
		{
			//se o enter tiver sido pressionado, enviar dados para o cliente
			if(tecla == ENTER)
			{
				int tamanho = lista1.tamanho() + lista2.tamanho();
				int pos = 0;
				
				//alocando espaço
				unsigned char * buf = (unsigned char*)malloc(sizeof(unsigned char)*tamanho);

				//adicionando dados ao buffer
				for(int i = 0; i < lista1.tamanho(); i++)
				{
					buf[pos] = lista1[i];
					pos++;
				}
				
				for(int i = 0; i < lista2.tamanho(); i++)
				{
					buf[pos] = lista2[i];
					pos++;
				}
				
				//escrevendo dados no stdin
				int arq = abrir(arquivo, 'a');
				escrever(arq, buf, tamanho);
				fechar(arq);
				
				//log de comandos
				if (strlen(buf) > 1)
				{
					//resetando posicao do log
					posicao_log = 0;
					int tamanho_log = log_comandos.tamanho();
					int posicao = 0;
					Lista<unsigned char> lista_aux;
					for (int i=0; i < strlen(buf);i++)
						lista_aux.adicionar(buf[i]);
						
					if (tamanho_log < 10)
					{
						log_comandos.adicionar_em(0, lista_aux);
					}
					else
					{
						log_comandos.remover(9);
						log_comandos.adicionar_em(0, lista_aux);
					}
					lista_aux.limpar();
				}
				//liberando memória
				free(buf);
				lista1.limpar();
				lista2.limpar();
					
				//sinalizando processo
				enviar_msg_pid(tec_pid_foreground, mensagem);
				
				//colocando o ENTER na tela
				vid_imp_char(tecla);
				
				//desativando o teclado
				teclado_ativado = 0;
					
				//desabilitando cursor	
				vid_habilitar_cursor(0);	
			}
			else
			{
				//senão, continuar tratando os pressionamentos
				tec_corrigir_pressionamento(tecla);
			}
		}
		else
		{
			//modo cru, enviando uma tecla por vez
			teclado_ativado = 0;
			mensagem[0] = tecla; //tecla
			mensagem[1] = 1; // tecla foi pressionada
			enviar_msg_pid(tec_pid_foreground, mensagem);
		}
	}
}

