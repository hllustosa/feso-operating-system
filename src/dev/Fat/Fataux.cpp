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
#include "include/Lista.h"
#include "include/Io.h"
#include "include/Ide.h"
#include "include/Sistimer.h"
#include "include/Fat.h"

//---------------Funções Auxiliares-------------------------------
//calcula o setor real que um cluster de dados ocupa no disco
unsigned int calcular_setor_absoluto(unsigned int cluster)
{
  return cluster - 2 + primeiro_setor_dados;
}

//obtem o próximo cluster livre na FAT
unsigned int obter_prox_cluster_livre()
{
   unsigned int cluster_livre;
   unsigned int valor;
   
  //Procura o próximo cluster livre para alocação   
  for(unsigned int i = (fat_info.prox_cluster_livre); i < num_total_clusters; i++)
  {
	 //lendo entrada na FAT
	 acessar_fat(i,LER_ENTRADA,&valor);
	 
	 //para caso tenha encontrado um cluster livre
     if(valor == 0) break;
  }
   
  //atualiza posição do próximo possível cluster livre
  fat_info.prox_cluster_livre = i+1;
  
  //atualiza FAT info no disco
  salvar_fatinfo();
   
  //retorna o número do próximo cluster livre 
  return i;
}

//carrega diretorio raiz do disco para a memória
void carregar_diretorio_raiz()
{ 
  
  diretorio_raiz.limpar();
  
  int encontrou_ultima_entrada =0, serv, ret;
  entrada_diretorio_t ent; 
   
  //abrindo arquivo do diretório raiz   
  abrir_arquivo("", "/dev/hdd_root");
 
  //abrindo arquivo no vfs
  int descritor = _abrir("/dev/hdd_root", 'A', &serv);
 
  do 
  {
	//lendo entrada do diretório padrão
	ret = ler(descritor, (char *)&ent, sizeof(entrada_diretorio_t));
     
	//caso não tenha nome, o registro é o último do diretório 
	if(ent.dir_nome[0] != '\0')
	{
		diretorio_raiz.adicionar(ent);
	}
	else
	{
		encontrou_ultima_entrada = 1;
		break;
	}	  
    
	
	//para ao encontrar última entrada no diretório
	if(encontrou_ultima_entrada) break;
	
  //}while(true);	
  }while(ret != VFS_EOF);
 
  _fechar(descritor);	
}

//atualiza o diretorio raiz no disco
void salvar_diretorio_raiz()
{
  //criando lista de cluster para armazenar arquivo 
  Lista<Cluster> arq_aux;
  int serv;
  
  //Adicionando uma nova entrada no diretório para indicar última entrada  
  dir_final.dir_nome[0] = '\0'; //Primeiro char do nome = 0x00 indica última entrada
  diretorio_raiz.adicionar(dir_final);
  
  //entrada_diretorio_t * caux = (entrada_diretorio_t *) malloc(sizeof(entrada_diretorio_t)*diretorio_raiz.tamanho());
  
  char * caux = (char *)malloc(sizeof(TAM_CLUSTER));
 
  //salvando dados no VFS
  int descritor = _abrir("/dev/hdd_root", 'a', &serv);	
 
  for(int i =0; i < diretorio_raiz.tamanho(); i++)
  {
    //copiando a entrada do diretorio raiz para  um array de char
	//memcpy((unsigned char *)&caux[i], (unsigned char *)&diretorio_raiz[i], sizeof(entrada_diretorio_t));
	//memcpy(caux, (unsigned char *)&diretorio_raiz[i], sizeof(entrada_diretorio_t));
	//printf("aqui %s\n", diretorio_raiz[i].dir_nome);
	escrever(descritor, (unsigned char *)&diretorio_raiz[i],  sizeof(entrada_diretorio_t));  	
  }   
  
  
  _fechar(descritor);
  
  //salvando informações sobre a pasta no disco 
  salvar_arquivo("", "/dev/hdd_root");
  
  //liberando memória
  free(caux);  
  
  //removendo o registro em branco inserido no final
  diretorio_raiz.remover(diretorio_raiz.tamanho() -1);
  
}

//carrega os dados da FAT32 para a memória
void carregar_fat32()
{
  
  fat_ler_setor(setor_boot, 0); 
  fat_BS *bsaux = (fat_BS *) setor_boot;
  memcpy((char*)&bs,(char*) bsaux, sizeof(fat_BS));
  
 
  if(strcmp(bs.oem_nome,"MSWIN4.1"))
  {
	formatar_fat32();
	//sair();
  }
 
  fat_ler_setor(setor_boot, bs.secao_extendida.fat_info);
  memcpy((char *)&fat_info, setor_boot, sizeof(fat_info));
   
  //calculando informações
  primeiro_setor_dados     = bs.qtd_setores_reservados + (bs.qtd_tabelas * bs.secao_extendida.tam_tabela_32);
  primeiro_setor_fat       = bs.qtd_setores_reservados;
  num_total_setores_dados  = bs.qtd_total_setores_32 - (bs.qtd_setores_reservados + bs.secao_extendida.tam_tabela_32);
  num_total_clusters       = bs.qtd_total_setores_32;
  
  carregar_diretorio_raiz();	
}

//armazena as estruturas do sistema de arquivos FAT no disco
void formatar_fat32()
{
  
  unsigned int  qtd_setores_lba28;
  
  fat_identificar((unsigned char *)info_identificar, 1, 1, 1);
  
  //carregando estrutura com inf 
  qtd_setores_lba28 = *(unsigned int *)&info_identificar[60];
  //qtd_setores_lba28 = 8192;
  
  bs.bootjmp[0] 	 		= 0x00; 
  bs.bootjmp[1]		 		= 0x00;
  bs.bootjmp[2]      		= 0x00;
  
  unsigned char oem_n[2] = {' ','\0'};
  memcpy(bs.oem_nome, "MSWIN4.1\0" ,strlen("MSWIN4.1\0")+1);
  
  //Carregando valores da BP
  bs.bytes_por_setor	    = 512;
  bs.setores_por_cluster 	= 1;
  bs.qtd_setores_reservados = 32;
  bs.qtd_tabelas 			= 1;
  bs.num_total_setores_16 	= 0;
  bs.tipo_midia 			= 0xF8;
  bs.tam_tabela_16 			= 0;
  bs.setores_por_trilha 	= 0;
  bs.qtd_cabecas_leitura 	= 0;
  bs.num_set_escondidos 	= 0;
  bs.qtd_total_setores_32 	= qtd_setores_lba28;
  
  //Carregando valores FAT32 extendido
  ex.tam_tabela_32	   = (bs.qtd_total_setores_32 -( bs.qtd_setores_reservados))/( (256+bs.qtd_tabelas)/2);//Tamanho da Tabela FAT em clusters
  ex.ext_flags         = 0x00;//Flags (controle de FAT espelho?)
  ex.fat_versao        = 0x00;//Versao do FAT, colocar 0
  ex.cluster_raiz      = 2;//Aponta para o primeiro cluster ocupado pelo diretório raiz
  ex.fat_info          = 1;//O número do setor em que está localizada a estrutura FSINFO(geralmente o valor é 1) 
  ex.backup_BS_setor   = 0;//Aponta para a setor em que está localizada a copia do setor de boot. (Valor recomendado 6)
  memset(ex.reservado_0,0,sizeof(ex.reservado_0));//Área reservada para futuras expansões do formato
  ex.num_drive         = 0x80;//Número do Drive (0x80 para HDs e 0x00 para disquetes)  
  memset(&ex.reservado_1,0,sizeof(ex.reservado_1));//Campo reservado (deixar em 0)  
  ex.boot_ass          = 0x29; //Assinatura de boot (0x29).Indica que os próximos 3 valores estão presentes
  ex.volume_id         = 1;//Número de série do volume. Gerado a partir da combinação da data e hora em um valor de 32bits
  unsigned char no_mame[8] = {'N','O',' ','N','A','M','E','\0'};
  //memcpy(ex.rotulo_volume,no_mame,7);//Nome volume. Valor padrão "NO NAME"
  unsigned char fat[4] = {'F','A','T','\0'};
  //memcpy(ex.tipo_sistema ,fat,3);//String com o nome do sistema de arquivos. (FAT12, FAT16, FAT) 
  bs.secao_extendida = ex;
  
  
  //gravando no setor 0 do disco
  char * setor_boot = (char *)malloc(sizeof(char)*512);
  memcpy(setor_boot ,(char *)&bs, sizeof(bs));
  fat_escrever_setor(setor_boot, 0); 
  free(setor_boot);
  
  //calcular
  primeiro_setor_dados = bs.qtd_setores_reservados + (bs.qtd_tabelas * bs.secao_extendida.tam_tabela_32);
  primeiro_setor_fat   = bs.qtd_setores_reservados;
  num_total_setores_dados  = bs.qtd_total_setores_32 - (bs.qtd_setores_reservados + bs.secao_extendida.tam_tabela_32);
  num_total_clusters = bs.qtd_total_setores_32;
  
  //criar fat info
  fat_info.fat_info_ass   = 0x41615252; //Assinatura da estrutura fat_info valor ()
  fat_info.fat_info_ass2  = 0x61417272; //Segunda assinatura da estrutura, valor(0x61417272)
  fat_info.qtd_cluster_livres = num_total_clusters - primeiro_setor_dados;   //Qtd de clusters livres no disco. Colocar em 0xFFFFFFFF//carregando dados na memória  
  fat_info.prox_cluster_livre = 2;//Local do próximo cluster livre
  fat_info.fat_info_final = 0xAA550000;//Assinatura final da estrutura valor (0xAA550000.)	//colocando 0 nas entradas da FAT
  salvar_fatinfo();
    
   memset((unsigned char *)info_identificar, '\0', 512);
   for(int i = primeiro_setor_fat; i < primeiro_setor_fat + bs.secao_extendida.tam_tabela_32; i++)
   {
		fat_escrever_setor((unsigned char *)info_identificar, i);
   }
    
  //Criar diretório raiz*
  entrada_diretorio_t * dir = (entrada_diretorio_t *)malloc(sizeof(entrada_diretorio_t)*2);
  memset(dir[0].dir_nome,'\0',11);  					//nome do arquivo
  dir[0].dir_atributos 			= 0x00 | ATR_VOLUME_ID; //atributos do arquivo (Volume ID é o atributo padrão do dir raiz)
  dir[0].nt_reservado  			= 0x00;          		//manter em 0
  dir[0].criacao_decimos_seg 	= 0x00;   				//Parte da informação referente a data de criacao
  dir[0].criacao_hora 			= 0x00;          		//Hora em que o arquivo foi criado
  dir[0].criacao_data 			= 0x00;	         		//Data em que o arquivo foi criado
  dir[0].data_ult_acesso 		= 0x00;	     			//Data último acesso
  dir[0].num_primeiro_clusterHI = 0x00;					//Parte alta do número do primeiro cluster do arquivo
  dir[0].hora_ult_modificacao 	= 0x00;  				//hora da última modificação
  dir[0].data_ult_modificacao 	= 0x00; 				//data da última modificação
  dir[0].num_primeiro_clusterLO = bs.secao_extendida.cluster_raiz ;//Parte baixa do número do primeiro cluster do arquivo
  dir[0].tam_arquivo 			= 0x00;					//tamanho do arquivo
  
   
  memset((unsigned char *)&dir[1], '\0', sizeof(entrada_diretorio_t));
 
 char * cdir_raiz = (char *)malloc(sizeof(char)*512);
  memcpy(cdir_raiz, (char *)dir, sizeof(dir));

    
  fat_escrever_setor(cdir_raiz, calcular_setor_absoluto(2)); 
   free(cdir_raiz);
 
  unsigned int val;
  val = EOC;
  acessar_fat(2, ALTERAR_ENTRADA, &val);
  free(dir);
  
  carregar_diretorio_raiz();

}


//manipula os registros da tabela FAT no disco
void acessar_fat(unsigned int cluster,unsigned short op, unsigned int *valor)
{
  
   unsigned int  fat_deslocamento = cluster * 4;
   unsigned int  fat_setor        = primeiro_setor_fat + (fat_deslocamento / TAM_CLUSTER);
   unsigned int  ent_deslocamento = fat_deslocamento % TAM_CLUSTER;
   
   fat_ler_setor(buf, fat_setor);
   
   //lendo dados
   if(op == LER_ENTRADA)
   {
	   //Ignorar o 4 primeiros bits da entrada da FAT.
	   *valor = *(unsigned int*)&buf[ent_deslocamento] & 0x0FFFFFFF;
   }
   else if(op == ALTERAR_ENTRADA)
   {
			
      //zerando os 4 primeiros bits do valor 
	  *valor = *valor & 0x0FFFFFFF;
		
	  //zerando o valor da entrada referente ao cluster indicado pelo parametro cluster
	  *((unsigned int *) &buf[ent_deslocamento]) = (*((unsigned int *) &buf[ent_deslocamento])) & 0xF0000000;
		
	  //escrevendo o valor na posição da fat referente ao cluster
	  *((unsigned int *) &buf[ent_deslocamento]) = (*((unsigned int *) &buf[ent_deslocamento])) | *valor;
	  fat_escrever_setor(buf, fat_setor);
   }
	
   
} 

//salva informações da fat info no disco
void salvar_fatinfo()
{
  //char * cfatinfo = malloc(sizeof(char)*512);
  memcpy(dados, (char *)&fat_info, sizeof(fat_info));
  fat_escrever_setor(dados, bs.secao_extendida.fat_info);
  //free(cfatinfo);
}