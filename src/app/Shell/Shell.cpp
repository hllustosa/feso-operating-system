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

#define QTD_CMD 15
#define QTD_PARAM 10
#define TAM_PARAM 100

char pasta_atual[100] = "/\0";
char comando_analisado[QTD_PARAM][TAM_PARAM];

enum codigos{EXEC, NOVO, NOVA_PASTA, LISTAR, REMOVER, COPIAR, IR, VOLTAR,
			 RENOMEAR, PROC, PORTAS, MEM, AJUDA, LIMPAR, FINALIZAR};
			 
char * lista_comandos[] = {"exec", "novo", "nova_pasta", 
                           "listar", "remover", "copiar", 
						   "ir", "voltar", "renomear",
				           "proc", "portas", "mem", "ajuda", 
						   "limpar", "finalizar"};

char * conteudo_ajuda[] = 
{
  "exec          \"nome do arquivo\" [param1] [param2] .. [param10] \n",
  "novo          \"nome do arquivo\"\n",
  "nova_pasta    \"nome da pasta\" \n",
  "listar\n",
  "remover       \"nome do arquivo\\pasta\" \n",
  "copiar        \"arquivo de origem\" \"arquivo de destino\" \n",
  "ir            \"pasta\" \n",
  "voltar        \n",
  "renomear      \"arquivo de origem\" \"arquivo de destino\" \n",
  "proc          \n",
  "portas        \n",
  "mem           \n",
  "limpar        \n",
  "finalizar     \"pid\" \n"
};
						   
					
void exec(char cmd[QTD_PARAM][TAM_PARAM])
{
	char comando[256], param[512];
	int argc = 0;
	
	//zerando o nome do arquivo e a lista de parametros
	memset(comando, '\0', 256);
	memset(param, '\0', 512);
	
	//adicionando pasta ao nome do arquivo
	if(cmd[1][0] != '/')//se comeca com / o caminho e absoluto
		strcat(comando, pasta_atual);
		
	//concatenando parâmetro 1 do comando	
	strcat(comando,cmd[1]);
		
	//unindo parâmetros	em uma única string
	for(int i = 2; i < 10; i++)
	{
		if(cmd[i][0] != '\0')
		{
			strcat(param, cmd[i]);
			strcat(param, " ");
			argc++;
		}
		else
		{
			break;
		}
	}		
	
	//executando
    int ret = executar(comando, argc, param);
	
	//verificando resultado
	if(ret != PROC_SUCESSO)
	{
		switch(ret)
		{
			case VFS_ERR_ARQ_NAO_ENCONTRADO : printf("Arquivo %s nao encontrado!\n", comando); break;
			case PROC_FORMATO_INVALIDO      : printf("Arquivo invalido!\n"); break;
			case PROC_SEM_MEMORIA           : printf("Memoria insuficiente para completar a operacao!\n"); break;
			
		}//fim do switch
		
	}//fim do if
}

void novo(char cmd[QTD_PARAM][TAM_PARAM])
{
    char path[100];
    memset(path, '\0', 100);
   
    if(cmd[1][0] != '/')//se comeca com / o caminho e absoluto
		strcat(path, pasta_atual);
	
    strcat(path, cmd[1]);
	
	//verificando se o aquivo já existe
	int descritor = abrir(path, 'b');
	
	//caso o arquivo não exista, então criá-lo
	if(descritor < 0)
	{
		descritor = abrir(path, 'a');
		
		switch(descritor)
		{
			case VFS_ERR_ARQ_NAO_ENCONTRADO : printf("Diretorio inexistente!\n");break;
			case VFS_DIR_NAO_ENCONTRADO 	: printf("Diretorio inexistente!\n");break;
			case VFS_SEM_PERMISSAO      	: printf("Arquivo esta sendo utilizado!\n");break;
			case VFS_ERR_TAM_MAX_DIR    	: printf("O diretorio atual nao comporta mais arquivos!\n");break;	
			case VFS_NOME_INVALIDO      	: printf("Nome selecionado e invalido!\n");break;	
			case VFS_ARQUIVO_EXTERNO    	: printf("Nao foi possivel criar o arquivo!\n");break;
			default : printf("Arquivo criado com sucesso!\n");break;
		}
		
		fechar(descritor);
	}
	else
	{
		printf("O arquivo %s ja existe!\n", path);	
	}
	
}

void nova_pasta(char cmd[QTD_PARAM][TAM_PARAM])
{
	char path[100];
    memset(path, '\0', 100);
   
    if(cmd[1][0] != '/')//se comeca com / o caminho e absoluto
		strcat(path, pasta_atual);
		
    strcat(path, cmd[1]);
	
	int descritor = abrir(path, 'd');
	
	if(descritor < 0)
	{
		descritor = abrir(path, 'D');
		fechar(descritor);
		
		switch(descritor)
		{
			case VFS_DIR_NAO_ENCONTRADO : printf("Diretorio inexistente!\n");break;
			case VFS_ERR_TAM_MAX_DIR    : printf("O diretorio atual nao comporta mais arquivos!\n");break;	
			case VFS_NOME_INVALIDO      : printf("Nome selecionado e invalido!\n");break;	
			case VFS_ARQUIVO_EXTERNO    : printf("Nao foi possivel criar o diretorio!\n");break;
			default : printf("Diretorio criado com sucesso!\n");break;
		}
	}
	else
	{
		printf("O diretorio %s ja existe!\n", path);
	}
	
}

void listar(char cmd[QTD_PARAM][TAM_PARAM])
{
   Arq_Info * arq;
   char info[100], path[100], dados[512], espaco[20];
   
   memset(path, '\0', 100);
   
   //se não houver parâmetros
   //listar diretório atual
   if(strlen(cmd[1]) == 0)
   {
		strcat(path, pasta_atual);
   }	
   else//senão, listar diretório especificado
   {
		if(cmd[1][0] != '/')//se comeca com / o caminho e absoluto
		   strcat(path, pasta_atual);
	
		strcat(path, cmd[1]);	
   }
   
   int pasta = abrir(path, 'd');
  
   if(pasta > 0)
   {
      
	  ler(pasta, dados, 512);
	  int * a_desc = (int *)dados;
	    
	  printf("\n");	
	  for(int i = 1; i <= a_desc[0]; i++)
	  {
		  obter_info_arq(a_desc[i], info);
		  arq = (Arq_Info *)info;
		  
		  int val = strlen(arq->nome);
		  int pos =0;
		  printf("%s",arq->nome);
		  
		  for(int aux = val; aux < 20; aux++)
		  {
			 espaco[pos++] = ' ';
		  }
		  
		  espaco[pos] = '\0';
		  printf("%s", espaco);
		  
		  if( (arq->tipo == VFS_DIR) || (arq->tipo == VFS_MNT_DIR) )
		  {
				printf(" DIR\n");
		  }
		  else
		  {
				int tamanho_do_arquivo;
				
				if(arq->tamanho == 0)
				{
					tamanho_do_arquivo = arq->tamanho_em_disco;
				}
				else
				{
					tamanho_do_arquivo = arq->tamanho;
				}
				
				if(tamanho_do_arquivo < 1024)
					printf(" TAM %d byte(s)\n", tamanho_do_arquivo);
				else
					printf(" TAM %d kb\n", tamanho_do_arquivo/1024);
				
		  }
	
	  }
	  printf("\n");	
	  fechar(pasta);
   }
   else
   {
		printf("O diretorio %s nao foi encontrado!\n", path);
   }
}

void remover(char cmd[QTD_PARAM][TAM_PARAM])
{
	char path[100];
    memset(path, '\0', 100);
   
    if(cmd[1][0] != '/')//se comeca com / o caminho e absoluto
		strcat(path, pasta_atual);
		
    strcat(path, cmd[1]);

	int descritor  = abrir(path, 'b');	
	int descritor2 = abrir(path, 'd');

	if((descritor < 0) && (descritor2 < 0))
	{
		switch(descritor)
		{
			case VFS_DIR_NAO_ENCONTRADO     : printf("Diretorio inexistente!\n");break;
			case VFS_ERR_ARQ_NAO_ENCONTRADO : printf("Arquivo nao encontrado!\n");break;
			case VFS_ARQUIVO_EXTERNO    : printf("Nao foi excluir o arquivo ou diretorio!\n");break;
		}
	}
	else
	{
		int ret;
		
		if(descritor > 0)
		  ret = excluir(descritor);
		else
		  ret = excluir(descritor2);
		
		switch(ret)
		{
			case VFS_ARQUIVO_EXTERNO : printf("Nao foi possivel excluir o arquivo!\n"); break;
			//default : printf("Arquivo %s removido com sucesso!\n", path);
		}
	}
}

int copiar(char cmd[QTD_PARAM][TAM_PARAM])
{
	Arq_Info * arq;
	int descritores[2], ret, erro =1;
	char path_origem[100], path_destino[100], info[100], bloco[512];
	
	//zerando caminhos
	memset(path_origem,  '\0', 100);
	memset(path_destino, '\0', 100);
	
	//adicionando complemento do caminho
	if(cmd[1][0] != '/')//se comeca com / o caminho e absoluto
		strcat(path_origem, pasta_atual);
	
	strcat(path_origem, cmd[1]);
	
	if(cmd[2][0] != '/')//se comeca com / o caminho e absoluto
		strcat(path_destino, pasta_atual);
	
    strcat(path_destino, cmd[2]);
	
    //abrindo arquivo de origem
	descritores[0] = abrir(path_origem, 'b');
   	
	switch(descritores[0])
	{
		case VFS_DIR_NAO_ENCONTRADO : 
			 printf("Diretorio inexistente!\n"); break;
		case VFS_ERR_ARQ_NAO_ENCONTRADO : 
			 printf("Arquivo %s inexistente!\n", path_origem); break;
		case VFS_SEM_PERMISSAO: 
			 printf("Arquivo esta sendo utilizado!\n"); break; 
		default : erro = 0;	 
	}
		
	if(!erro)
	{
		 descritores[1] = abrir(path_destino, 'a');	
		
		 erro = 1;
		 switch(descritores[1])
		 {
			case VFS_DIR_NAO_ENCONTRADO : 
				 printf("Diretorio inexistente!\n"); break;
			case VFS_ERR_ARQ_NAO_ENCONTRADO : 
				 printf("Arquivo %s inexistente!\n", path_origem); break;
			case VFS_SEM_PERMISSAO: 
				 printf("Arquivo esta sendo utilizado!\n"); break; 
			case VFS_ERR_TAM_MAX_DIR: 
				 printf("O diretorio atual nao comporta mais arquivos!\n"); break;
			case VFS_NOME_INVALIDO: 
				 printf("Nome selecionado e invalido!\n"); break;
			default : erro = 0;	 
		 }
	
		 if(erro) return erro;
	
		 obter_info_arq(descritores[0], info);
		 arq = (Arq_Info *)info;
		  
		 do
		 {
			ret = ler(descritores[0], (char*)bloco, 512);
			
			if(arq->tamanho >= 512)
				escrever(descritores[1], (char*)bloco, 512);
			else
			   escrever(descritores[1], (char*)bloco, arq->tamanho);	
			   
		 	arq->tamanho -= 512;
			
		 }while(ret != VFS_EOF);
	}
	else
	{
		excluir(descritores[1]);
	}
	
	fechar(descritores[0]);
	fechar(descritores[1]);
	
	return !erro;
}

void ir(char cmd[QTD_PARAM][TAM_PARAM])
{
   char	path[100];
   memset(path, '\0', 100);
  
   //se comeca com / o caminho e absoluto
   if(cmd[1][0] != '/')
	   strcat(path, pasta_atual);

   strcat(path, cmd[1]);	
   
   int ret = abrir(path, 'd');
  
   if(ret != VFS_DIR_NAO_ENCONTRADO)
   {
	  if(cmd[1][0] != '\0')
	  {
         //atribuindo novo valor ao diretório atuals
		 memcpy(pasta_atual, path, strlen(path)+1);
		 
		 //adiciona / caso não tenha sido selecionado o diretorio raiz
		 if(strcmp("/", cmd[1]))
			strcat(pasta_atual, "/");
	  }
   }
   else
   {
		printf("O diretorio %s nao foi encontrado!\n", path);
   }

}

void voltar(char cmd[QTD_PARAM][TAM_PARAM])
{
   int tam = strlen(pasta_atual);	
   if(tam > 1)
   {
	   int pos = tam - 1;	
       int qtd_barras = 0;
	   
	   while(qtd_barras < 2)
	   {
	      if(pasta_atual[pos] == '/') 
				qtd_barras++;
		  
		  if(qtd_barras < 2) 
				pasta_atual[pos] = '\0';
		
           pos--;		
	   }
	
   }
}

void renomear(char cmd[QTD_PARAM][TAM_PARAM])
{
	if(copiar(cmd))
	{
		remover(cmd);
	}
	
}

void proc(char cmd[QTD_PARAM][TAM_PARAM])
{
	int ret;
	char path[100];
	char linha[100];
	memset(path, '\0', 100);
	memcpy(path,"/dev/info_kernel", strlen("/dev/info_kernel")+1);
	strcat(path, itoa(obter_pid(), 10));
	
	printf("\n");
	obter_info_kernel(path ,INFO_PROCESSOS);
	
	int desc = abrir(path, 'A');
	
	struct Proc
	{
		int pid;
		char nome[40];
		int threads;
		int tamanho;
	};
	
	Proc p;
	printf("PID  NOME                                 THREADS  ESPACO OCUPADO\n");
	printf("---------------------------------------------------------------\n");
	do
	{
		ret = ler(desc, (char *)&p, sizeof(Proc));
		printf("%d    %s %d      %d kb\n",p.pid, p.nome, p.threads, p.tamanho);
		
	}while(ret != VFS_EOF);
	
	fechar(desc);
	printf("\n");
} 

void portas(char cmd[QTD_PARAM][TAM_PARAM])
{
	int ret;
	char path[100];
	char linha[100];
	memset(path, '\0', 100);
	memcpy(path,"/dev/info_kernel", strlen("/dev/info_kernel")+1);
	strcat(path, itoa(obter_pid(), 10));
	
	printf("\n");
	obter_info_kernel(path ,INFO_MENSAGENS);
	
	int desc = abrir(path, 'A');
	
	struct Port
	{
		int pid;
		int porta;
	};
	
	printf("PID   PORTA\n");
	printf("-----------------\n");
	do
	{
		Port p;
		ret = ler(desc, (char*)&p, sizeof(Port));
		printf("%d     %d\n", p.pid, p.porta);
		
	}while(ret != VFS_EOF);
	
	fechar(desc);
	printf("\n");
}

void mem(char cmd[QTD_PARAM][TAM_PARAM])
{
	int ret;
	char path[100];
	char linha[100];
	memset(path, '\0', 100);
	memcpy(path,"/dev/info_kernel", strlen("/dev/info_kernel")+1);
	strcat(path, itoa(obter_pid(), 10));
	
	printf("\n");
	obter_info_kernel(path ,INFO_MEMORIA);
	
	int desc = abrir(path, 'A');
	
	do
	{
		ret = ler(desc, linha, 100);
		printf("%s",linha);
		
	}while(ret != VFS_EOF);
	
	fechar(desc);
	printf("\n");
}

void limpar(char cmd[QTD_PARAM][TAM_PARAM])
{
	cls();
}

void ajuda(char cmd[QTD_PARAM][TAM_PARAM])
{
	printf("\n");
	for(int i =0; i < QTD_CMD -1; i++)
	{
		printf("%s", conteudo_ajuda[i]);
	}
	printf("\n");
}

void finalizar(char cmd[QTD_PARAM][TAM_PARAM])
{
	int pid = atoi(cmd[1]);
	finalizar_proceso(pid);
}

void executar_comando(char cmd[QTD_PARAM][TAM_PARAM], char * cmd_inteiro)
{
  int cod_cmd = -1;	
  for(int i=0; i< QTD_CMD; i++)	
  {
		if(!strcmp(cmd[0], lista_comandos[i]))
		{
			cod_cmd = i;
			break;
		}
  }	
  
  switch(cod_cmd)
  {
	case EXEC: 		 exec(cmd); break;
	case NOVO: 		 novo(cmd); break;
	case NOVA_PASTA: nova_pasta(cmd); break;
	case LISTAR: 	 listar(cmd); break;
	case REMOVER: 	 remover(cmd); break;
	case COPIAR: 	 copiar(cmd); break;
	case IR: 		 ir(cmd); break;
	case VOLTAR: 	 voltar(cmd); break;
	case RENOMEAR: 	 renomear(cmd); break;
	case PROC: 		 proc(cmd); break;
	case PORTAS: 	 portas(cmd); break;
	case MEM: 		 mem(cmd); break;
	case AJUDA:		 ajuda(cmd); break;
	case LIMPAR: 	 limpar(cmd); break;
	case FINALIZAR:  finalizar(cmd); break;
	default : printf("Comando %s nao reconhecido. Digite \"ajuda\" para ver a lista de comandos\n", cmd[0]);
  } 
   
}

void analisar_comando(char * comando)
{
	int pos =0;
	char * tok;

    tok = strtok(comando," ");
    
	//limpando comando e parâmetros
	for(int i =0; i< QTD_PARAM; i++)
	   memset(comando_analisado[i], '\0', TAM_PARAM);
	
	//separando o comando nos espaços em branco
	while (tok != NULL)
    {
       memcpy(comando_analisado[pos++], tok, strlen(tok));	
	   tok = strtok(NULL," ");
	   
    }
}

volatile int entrou = 0;
volatile int cont = 0;

volatile char tecla;


main(int argc, char * args[])
{
	
	char comando[512];

	while(true)	
	{
		memset(comando, '\0', 512);
		printf("%s>",pasta_atual);
		scanf("%s", comando);
		analisar_comando(comando);
		executar_comando(comando_analisado, comando);
		memset(comando, '\0', 512);
		
		for(int i=0; i<QTD_PARAM; i++)
		  memset(comando_analisado[i], '\0', TAM_PARAM);
	}
}