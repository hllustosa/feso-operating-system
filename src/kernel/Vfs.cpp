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
#include "include/Vfs.h"
#include "include/Mensagem.h"
#include "include/Processo.h"
#include "include/Util.h"
 
//Inst�ncia glocal do vfs
volatile Vfs vfs;

//Adiciona arquivo a um diret�rio
int Diretorio::adicionar_arquivo(unsigned int desc)
{
	//se n�o houver alcan�ado n�mero m�ximo de arquivos
	if(this->qtd_arquivos < Diretorio::MAX_ARQ)
	{
		this->arqs[this->qtd_arquivos] = desc;
		this->qtd_arquivos++;
		return 1;
	}
	else
	{
		return 0;
	}
}

//Remove arquivo de um diret�rio
int Diretorio::remover_arquivo(unsigned int desc)
{
	int encontrou =0, i;
	
	for(i =0; i <qtd_arquivos; i++)
	{
		if(arqs[i] == desc)
		{
			encontrou = 1;
			break;
		}
	}
	
	unsigned int aux[Diretorio::MAX_ARQ];
	
	if(i != Diretorio::MAX_ARQ -1)
	{
		memcpy((unsigned char *)aux,(unsigned char *)arqs, Diretorio::MAX_ARQ*sizeof(unsigned int));
		memcpy((unsigned char *)&aux[i],(unsigned char *)&arqs[i+1], (Diretorio::MAX_ARQ-i+1)*sizeof(unsigned int));
		memcpy((unsigned char *)arqs,(unsigned char *)aux, Diretorio::MAX_ARQ*sizeof(unsigned int));
	}
	
	qtd_arquivos--;
	
}

//Inicializar sistema de arquivos virtual
void Vfs::inicializar()
{
	mutex = 0;
	id_dono = -1;
    prox_descritor = 1;
	
	memcpy(raiz.nome, "\\\0", 2);
	raiz.offset  = 0;
	raiz.tamanho = 0;
	raiz.tipo = Vfs::DIR;
	raiz.pai = 0;
	
	//salvando estrutura
	Bloco c;
	Diretorio dir;
	dir.qtd_arquivos = 0;
	
	memcpy(c.dados, (unsigned char *)&dir, sizeof(dir)); 
	raiz.clusters.adicionar(c);
	
	//atribuindo novo descritor
	raiz.descritor = prox_descritor++;
	
	//acrescentando a estrutura dos arquivos.
	arquivos.adicionar(raiz.descritor, raiz);

	//criandos pastas
	int desc, serv;
	abrir("/dev", 'D', &desc);
	fechar(desc);	
	abrir("/usr", 'D', &desc);
	fechar(desc);
	abrir("/temp", 'D', &desc);
	fechar(desc);

}

//Obt�m arquivo
Arquivo& Vfs::obter_arquivo(unsigned int descritor)
{
	return arquivos[descritor];
}

//Carrega arquivo em um bloco cont�nuo de mem�ria
unsigned char * Vfs::obter_imagem(unsigned int descritor)
{
	Arquivo& arq = obter_arquivo(descritor);
	
	if(&arq != NULL)
	{
		//arq.//down;
		unsigned char * imagem = (unsigned char *)kmalloc(arq.tamanho);
		ler(descritor, imagem, arq.tamanho);
		arq.up();
		
		return imagem;
	}
	else
	{
		return NULL;
	}
}

//Valida nome do arquivo
unsigned int Vfs::validar_nome(unsigned char * nome)
{
	int tam = strlen(nome);
	
	if(tam > MAX_TAM_ARQ)
	{
		return 0;
	}
	else
	{
		//verificando caracteres inv�lidos
		for(int i =0; i < tam; i++)
		{
			if( (nome[i] == '\\') || (nome[i] == '/') || (nome[i] == ' '))
			{
				return 0;
			}
		}
		
		return 1;
	}
	
}

//Cria um arquivo 
unsigned int Vfs::criar_arquivo(Arquivo arq, unsigned char * trecho, char tipo, unsigned int * novo_descritor)
{		
	//verifica se o nome � v�lido
	if(validar_nome(trecho))
	{
		//caso o diret�rio pai seja um diret�rio montado
		if(arq.tipo == Vfs::MNT_DIR)
		{
			//obter porta do processo respons�vel
			Porta &p = entregador.obter_porta(arq.porta_servidor);
			
			/*
			  Se o pid respons�vel for diferente do
			  pid em execu��o, significa que outro processo est�
			  tentando criar um arquivo na pasta, o servidor deve ser notificado.
			*/
			if(p.pid_responsavel != escalonador.obter_pid_em_execucao())
			{
				*novo_descritor = 0;
				return VFS_ARQUIVO_EXTERNO;
			}
		}
	
	    //criando arquivo
		Arquivo novo_arq;
		memset(novo_arq.nome, 0, MAX_TAM_ARQ);
		memcpy(novo_arq.nome, trecho, strlen(trecho)+1);
		
		
		//atribuindo dados
		novo_arq.offset  		  = 0;
		novo_arq.tamanho 		  = 0;
		novo_arq.tamanho_em_disco = 0;
		novo_arq.pai 			  = arq.descritor;
		novo_arq.porta_servidor   = 0;
		
		//tipo 'A' e 'a' = arquivos comuns
		if(tipo == 'A' || tipo == 'a')
		{
			novo_arq.tipo = Vfs::ARQ;
		}
		//tipo 'D' = diret�rio
		else if(tipo == 'D')
		{
			Bloco c; Diretorio dir;
			novo_arq.tipo = Vfs::DIR;
			dir.qtd_arquivos = 0;
			memcpy(c.dados, (unsigned char *)&dir, sizeof(dir)); 
			novo_arq.clusters.adicionar(c);
		}
		
		//atribu�ndo descritor ao arquivo
		novo_arq.descritor = prox_descritor++;
		
		//adicionado a pasta
		Diretorio * d = (Diretorio *)&arq.clusters[0].dados;		
		*novo_descritor = novo_arq.descritor;
		
		if(d->adicionar_arquivo(novo_arq.descritor))
		{
			//configurando arquivo como aberto
			novo_arq.aberto_por = escalonador.obter_pid_em_execucao();
			
			//adicionando a lista de arquivos
			arquivos.adicionar(novo_arq.descritor, novo_arq);
				
			//armazenando descritor
			*novo_descritor = novo_arq.descritor;
			
			//criando evento
			Evento evt;
			evt.num = KERNEL_PORTA_VFS;
			evt.param1 = 0;
			evt.param2 = novo_descritor;		
					
			//notificando exec��o
			char msg[100];
			memcpy(msg, (char *)&evt, sizeof(Evento));
			entregador.notificar_evento(msg, KERNEL_PORTA_VFS);			
			//retornando sucesso
			return VFS_SUCESSO;
		}
		else
		{
			*novo_descritor = 0;
			return VFS_ERR_TAM_MAX_DIR;	
		}
	}
	else
	{
		return VFS_NOME_INVALIDO;
	}
}

//Sobrecargas para o m�todo de abertura de arquivo
unsigned int Vfs::abrir(unsigned char * nome, char modo, unsigned int * novo_descritor)
{
	int serv, ret;
	ret = _abrir(nome, modo, novo_descritor, &serv);
	return ret;
}

unsigned int Vfs::abrir(unsigned char * nome, char modo, unsigned int * novo_descritor, unsigned int * servidor)
{
	int ret;
	
	ret = _abrir(nome, modo, novo_descritor, servidor);
	
	return ret;
}

unsigned int Vfs::_abrir(unsigned char * nome, char modo, unsigned int * novo_descritor, unsigned int * servidor)
{
	//buscando arquivo
	Arquivo arq = raiz;
	Arquivo aux;
	unsigned int encontrou;
	int pos_final = strlen(nome) -1;
	char * trecho = strtok(nome, "/");
	
	*servidor = 0;
	*novo_descritor = 0;
	encontrou = 1;
	
	//varrendo string com o caminho
	while(trecho != NULL)
	{
	   encontrou = 0;
		 
	   //caso o arquivo seja um diretorio, continuar procura
	   if(arq.tipo == Vfs::DIR || arq.tipo == Vfs::MNT_DIR)
	   {
			//obtendo lista de arquivos em diretorio (m�ximo 127)
			Diretorio * dir = (Diretorio *)arq.clusters[0].dados;	
			
			//varrendo lista de diret�rios
			for(int i =0; i < dir->qtd_arquivos; i++)
			{	
				//obtendo arquivo
				unsigned descritor = dir->arqs[i];
				aux = arquivos[descritor];
					
				//comparando nome
				if(!strcmp(aux.nome, trecho))
				{	
					 encontrou = 1;
					 break;
				}
				
			}//fim do for
	   }
	   
	   if(encontrou)
	   {	
		  arq = aux;
	      trecho = strtok(NULL, "/");   
	   }
	   else
	   {
		  char * token = strtok(NULL, "/");   

		  //se o token for nulo, a pasta foi encontrara
		  //e o arquivo n�o existe
		  if(token == NULL)
		  {
			 break;
		  }
		  else //sen�o, o diret�rio n�o foi encontrado
		  {
			 return VFS_DIR_NAO_ENCONTRADO;
		  }
		  
	   }//fim do else
	   
	}//fim do while
	
	//Verifica se o arquivo foi encontrado
	if(!encontrou)
	{
		 *servidor = 0;
	     *novo_descritor = 0; 
		 
		 //'d' = verificar se diret�rio existe
		 if(modo == 'd')
	     {	
			return VFS_DIR_NAO_ENCONTRADO;
	     }
		 
		 //'b' = abrir arquivo, somente se ele existir
		 if(modo == 'b')
		 {
			return VFS_ERR_ARQ_NAO_ENCONTRADO;
		 }
	
		*servidor = arq.porta_servidor;
		
		return criar_arquivo(arq, trecho, modo, novo_descritor);		
	}
	else
	{	 
		//verificando se o arquivo esta sendo utilizado
		if(arq.aberto_por == 0)
		{
			
		   //caso seja um arquivo externo ao VFS
		   if(arq.tipo == Vfs::MNT)
		   {
				*servidor = arq.porta_servidor;
				*novo_descritor = arq.descritor;	
				return VFS_ARQUIVO_EXTERNO;	
		   }
		   //abrindo arquivo normalmente
		   else if(modo == 'A' || modo == 'b')
		   {
				if((arq.tipo == Vfs::ARQ) || (arq.tipo == MNT_ABERTO))
				{
					*novo_descritor = arq.descritor;
					arquivos[arq.descritor].aberto_por = escalonador.obter_pid_em_execucao();
				}
				else
				{
					return VFS_DIR_NAO_ENCONTRADO;
				}
		   }
		   //recirando arquivo
		   else if(modo == 'a')
		   {
				if((arq.tipo == Vfs::ARQ) || (arq.tipo == MNT_ABERTO))
				{
					*novo_descritor = arq.descritor;
					Arquivo &novo = arquivos[arq.descritor];
					novo.aberto_por = escalonador.obter_pid_em_execucao();
					novo.tamanho = 0;
					novo.clusters.limpar();	
					novo.offset = 0;
				}
				else
				{
					return VFS_DIR_NAO_ENCONTRADO;
				}
		   }
		   //criando diret�rio
		   else if (modo == 'D')
		   {
				if((arq.tipo == Vfs::DIR) || (arq.tipo == Vfs::MNT_DIR))
				{
					*servidor = 0;
					arquivos[arq.descritor].offset = 0;
					*novo_descritor = arq.descritor;
				}
				else
				{
					return VFS_ERR_ARQ_NAO_ENCONTRADO;
				}
		   }
		   //retorna descritor caso diret�rio exista
		   else if(modo == 'd')
	       {
			  if( (arq.tipo != Vfs::MNT) 
			      && (arq.tipo != Vfs::MNT_DIR) 
				  && (arq.tipo != Vfs::DIR))
			  {
			    *servidor = 0;
	            *novo_descritor = 0; 
			    return VFS_ERR_ARQ_NAO_ENCONTRADO;
			  }
			  else
			  {
				 *novo_descritor = arq.descritor;
				 return VFS_SUCESSO;
			  }
	       }
		
		   return VFS_SUCESSO;
		}
		else if (arq.aberto_por == escalonador.obter_pid_em_execucao())
		{
		   *novo_descritor = arq.descritor;		
		   return VFS_SUCESSO;
		}
		else
		{
		   return VFS_ARQUIVO_ABERTO;
		}
	}
}

//Escrever dados em um arquivo
unsigned int Vfs::escrever(unsigned int descritor, unsigned char * dados, unsigned int tamanho)
{
	//obtendo arquivo
	Arquivo& arq = arquivos[descritor];
	
	if(&arq != NULL)
	{		
		//verificando se o arquivo est� aberto e n�o � um diret�rio
		if( (arq.aberto_por == escalonador.obter_pid_em_execucao()) && (arq.tipo != Vfs::DIR))
		{
			unsigned int bytes_a_escrever = tamanho;
			unsigned int offset_arq =  arq.tamanho % 512;
			unsigned int offset_dados = 0;
			
			arq.down();
			
			//enquanto houverem bytes a escrever
			while(bytes_a_escrever > 0)
			{
				unsigned int qtd_copiar = bytes_a_escrever;
				
				//caso a quantidade de bytes a escrever n�o caiba no bloco atual
				if(bytes_a_escrever > ((512) - offset_arq))
				{
					//copiar apenas o suficiente par completar o bloco
					qtd_copiar = ((512) - offset_arq);
				}
				else
				{
					//copiar tudo que for poss�vel
					qtd_copiar = bytes_a_escrever;
				} 
				
				
				//diminuir a quantidade de bytes a escrever de acordo com o que ser� copiado
				bytes_a_escrever -= qtd_copiar;
				
				//caso o offset seja 0, um novo bloco deve ser adicionado ao arquivo
				if(offset_arq == 0)			
				{
					//memset(novo_bloco.dados,0, 512);
					arq.clusters.adicionar(novo_bloco);
				} 
				
				//obtendo �ltimo bloco do arquivo
				Bloco &b = arq.clusters[arq.clusters.tamanho() - 1];
				
				//copiando os dados para o bloco
				memcpy((unsigned char *)(b.dados + offset_arq), (unsigned char *)(dados + offset_dados), qtd_copiar);
				
				//calculando o offset dos dados a serem copiados
				offset_dados += qtd_copiar;
				
				//calculando o offset dentro do bloco do arquivo (caso chegue a 512, voltar para 0 )
				offset_arq  = ((qtd_copiar + offset_arq) >= 512)? 0 : (qtd_copiar + offset_arq);
			}
			
			//incrementando tamanho do arquivo
			arq.tamanho += tamanho;
			//arq.offset += tamanho;
			
			arq.up();
			
			//retornando 1 (sucesso)
			return VFS_SUCESSO;
		}
		else
		{
			return VFS_ARQUIVO_FECHADO;
		}
	}
	else
	{
		//Retornando erro de arquivo n�o encontrado
		return VFS_ERR_ARQ_NAO_ENCONTRADO;
	}
}

//Ler dados de um arquivo
unsigned int Vfs::ler(unsigned int descritor, unsigned char * dados, unsigned int tamanho)
{
	//obtendo arquivo
	Arquivo& arq = arquivos[descritor];
	
	if(&arq != NULL)
	{
		if(arq.aberto_por == escalonador.obter_pid_em_execucao() 
			|| arq.tipo == Vfs::DIR || arq.tipo == Vfs::MNT_DIR)
		{
			unsigned int bytes_a_ler   = tamanho;
			unsigned int offset_arq    = arq.offset;
			unsigned int offset_bloco  = arq.offset %512;
			unsigned int offset_dados = 0;
					
			arq.down();
			
			//enquanto houverem bytes a ler
			while(bytes_a_ler > 0)
			{
				unsigned int qtd_copiar;
				
				//caso a quantidade de bytes a ler seja maior que a do bloco
				if(bytes_a_ler > ( 512 - (offset_arq %512) ))
				{
					//copiar apenas o suficiente par completar o bloco
					qtd_copiar = ((512) - (offset_arq %512));
				}
				else
				{
					//copiar tudo que for poss�vel
					qtd_copiar = bytes_a_ler;
				}
				
				//diminuir a quantidade de bytes a escrever de acordo com o que ser� copiado
				bytes_a_ler -= qtd_copiar;
					
				//obtendo bloco do arquivo
				unsigned bloco_a_ler = offset_arq/512;
				Bloco &b = arq.clusters[bloco_a_ler];
				
				//copiando os dados para o bloco
				memcpy((unsigned char *)(dados + offset_dados),(unsigned char *)(b.dados + offset_bloco), qtd_copiar);
				
				//calculando o offset dos dados a serem copiados
				offset_dados += qtd_copiar;
				
				//calculando o offset dentro do bloco do arquivo 
				offset_arq   += qtd_copiar;
				
				//calculando o offset dentro do bloco do arquivo (caso chegue a 512, voltar para 0 )
				offset_bloco  = ((qtd_copiar + offset_bloco) >= 512)? 0 : (offset_bloco + qtd_copiar);
				
				//caso tenha chegado ao final do arquivo
				if(offset_arq >= arq.tamanho)
				{
					arq.offset = 0;
					arq.up();
					return VFS_EOF;
				}
			}
			
			//incrementando o offset do arquivo
			arq.offset = ((arq.offset + tamanho) < arq.tamanho )? arq.offset + tamanho : 0;
			
			arq.up();
			
			//retornando 1 (sucesso)
			return VFS_SUCESSO;
		}
		else
		{
			return VFS_ARQUIVO_FECHADO;
		}
	}
	else
	{
		
		return VFS_ERR_ARQ_NAO_ENCONTRADO;
	}
}

//Configurar posi��o dentro do arquivo no qual a pr�xima leitura/escrita deve ocorrer
unsigned int Vfs::buscar(unsigned int descritor, unsigned int pos)
{
	Arquivo& a = arquivos[descritor];
	
	//verificando se arquivo existe
	if(&a != NULL)
	{
		//verificando se a posi��o escolhida est� dentro dos limites do arquivo
		if(pos > a.tamanho)
		{
			return VFS_POSICAO_INVALIDA;
		}
		//verificando se o processo em execu��o est� com o arquivo aberto
		else if(a.aberto_por != escalonador.obter_pid_em_execucao())
		{
			return VFS_SEM_PERMISSAO;
		}
		//verificando se o arquivo est� aberto
		else if(a.aberto_por == 0)
		{
			return VFS_ARQUIVO_FECHADO;
		}
		//se a posi��o for menor que o arquivo e ele estiver aberto � poss�vel alterar o offset
		else
		{
			a.down();
			a.offset = pos;
			a.up();
			return VFS_SUCESSO;
		}	
	}
	else
	{
		return VFS_ERR_ARQ_NAO_ENCONTRADO;
	}
}

//Excluir arquivo
unsigned int Vfs::excluir(unsigned int descritor)
{
	Arquivo& a = arquivos[descritor];

	if(&a != NULL)
	{	
		if((a.tipo == Vfs::MNT_ABERTO) || (a.tipo == Vfs::MNT_DIR) )
		{
			return VFS_ARQUIVO_EXTERNO;
		}
		
		if(a.tipo == Vfs::DIR)
		{
		   Diretorio * d_arq = (Diretorio *)a.clusters[0].dados;
		   
		   for(int i =0; i < d_arq->qtd_arquivos; i++)
		   {
				excluir(d_arq->arqs[i]);
		   }
		}
		else if(a.aberto_por != escalonador.obter_pid_em_execucao())
		{
			return VFS_SEM_PERMISSAO;
		}
			
		Arquivo pasta = arquivos[a.pai];
		Diretorio * d = (Diretorio *)pasta.clusters[0].dados;
		d->remover_arquivo(a.descritor);
		arquivos.remover(a.descritor);
		
		//criando evento
		Evento evt;
		evt.num = KERNEL_PORTA_VFS;
		evt.param1 = 1;
		evt.param2 = descritor;		
				
		//notificando exec��o
		char msg[100];
		memcpy(msg, (char *)&evt, sizeof(Evento));
		entregador.notificar_evento(msg, KERNEL_PORTA_VFS);
		
		return VFS_SUCESSO;
	}
	else
	{
		return VFS_ERR_ARQ_NAO_ENCONTRADO;
	}
}

//Fechar arquivo
unsigned int Vfs::fechar(unsigned int descritor)
{
	Arquivo& a = arquivos[descritor];
	
	if(&a != NULL)
	{
        if(a.aberto_por == escalonador.obter_pid_em_execucao())	
		{
			a.down();
			
			a.aberto_por = 0;
			a.offset = 0;
			
			if(a.tipo == Vfs::MNT_ABERTO)
			{
				//a.tipo == Vfs::MNT;
				a.up();	
				return VFS_ARQUIVO_EXTERNO;
			}
			else
			{
				//criando evento
				Evento evt;
				evt.num = KERNEL_PORTA_VFS;
				evt.param1 = 2;
				evt.param2 = descritor;		
						
				//notificando exec��o
				char msg[100];
				memcpy(msg, (char *)&evt, sizeof(Evento));
				entregador.notificar_evento(msg, KERNEL_PORTA_VFS);
				
				a.up();
				return VFS_SUCESSO;
			}
			
			
		}
		else
		{
			
			return VFS_SEM_PERMISSAO;
		}
	}
	else
	{
		
		return VFS_ERR_ARQ_NAO_ENCONTRADO;
	}
}

//Adicionar n� para um arquivo em um sistema de Arquivos Externo
unsigned int Vfs::adicionar_no(unsigned char * nome , int tamanho, unsigned int porta_servidor, unsigned int * descritor)
{
	int ret, tipo, serv;
	char modo;

	if(tamanho == -1)
	{
		modo = 'D'; //caso o tamanho seja -1, criar um diret�rio
		tipo = Vfs::MNT_DIR;
	}
	else
	{
		modo = 'a'; //sen�o, criar um arquivo 
		tipo = Vfs::MNT;
	}
	
	if((ret = abrir(nome, modo, descritor)) == VFS_SUCESSO )
	{
		
		Arquivo& a = arquivos[*descritor];
		
		//down;
		a.tipo = tipo;
		a.porta_servidor = porta_servidor;
		a.tamanho_em_disco = tamanho;
		a.aberto_por = 0;		
		up();
		
		return VFS_SUCESSO;
	}
	else
	{
		
		return ret;
	}
}

//Torna n� externo acess�vel dentro do VFS
unsigned int Vfs::montar_no(unsigned int descritor)
{
	//obtendo arquivo
	Arquivo& a = arquivos[descritor];
	
	//verificando se o arquivo existe
	if(&a != NULL)
	{		
		a.down();
		
		if(a.tipo == Vfs::MNT)
		{
			a.tipo = Vfs::MNT_ABERTO;
		}
		else if(a.tipo == Vfs::MNT_ABERTO)
		{
			a.tipo = Vfs::MNT;
		}
		
		a.up();
		
		return VFS_SUCESSO;
	}
	else
	{
		return VFS_ERR_ARQ_NAO_ENCONTRADO;
	}
}

//Torna n� externo inacess�vel dentro do VFS
unsigned int Vfs::remover_no(unsigned int descritor)
{
	//obtendo arquivo
	Arquivo& a = arquivos[descritor];
	
	//verificando se o arquivo existe
	if(&a != NULL)
	{
		a.down();
		
		if( (a.tipo == Vfs::MNT) || (a.tipo == Vfs::MNT_ABERTO))
		{
			a.tipo = Vfs::ARQ;
		}
		else if(a.tipo == Vfs::MNT_DIR)
		{
			a.tipo = Vfs::DIR;
		}
		
		a.up();
		
		return VFS_SUCESSO;
	}
	else
	{
		
		return VFS_ERR_ARQ_NAO_ENCONTRADO;
	}
}

//Obt�m informa��es sobre o arquivo
unsigned int Vfs::obter_info_arquivo(unsigned descritor, unsigned char * dados)
{
		
	Arquivo& a = arquivos[descritor];
	
	if(&a != NULL)
	{
	   ArqInfo info;
	   
	   a.down();
	   
	   info.tamanho 	     = a.tamanho;
	   info.tamanho_em_disco = a.tamanho_em_disco;
	   info.tipo             = a.tipo;
	   info.offset           = a.offset;
	   info.pai              = a.pai;
	   info.descritor        = a.descritor;
	   info.porta_servidor   = a.porta_servidor;
	   info.flags 		     = a.flags;
		
	   a.up();	
		
	   //copiando nome
	   memcpy((unsigned char *)info.nome, (unsigned char *)a.nome, MAX_TAM_ARQ);
	   
	   //copiando para dados	
	   memcpy(dados, (unsigned char *)&info, sizeof(ArqInfo));
	   	   
	   return VFS_SUCESSO;
	}
	else
	{
		return VFS_ERR_ARQ_NAO_ENCONTRADO;
	}
}









