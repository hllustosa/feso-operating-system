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
#ifndef _FAT
#define _FAT

/*
	Partes do Código foram obtidas e modificadas com base no código obtido em: http://wiki.osdev.org/FAT#Implementation_Details
	Baseado também nas informações obtidas nas epscificações do formato FAT. disponível em http://msdn.microsoft.com/en-us/windows/hardware/gg463080
*/


#include "Lista.h"
#define TAM_CLUSTER 512
#define LER_ENTRADA 0
#define ALTERAR_ENTRADA 1

#define ATR_SOM_LEITURA 0x01
#define ATR_OCULTO 		0x02
#define ATR_SISTEMA 	0x04
#define ATR_VOLUME_ID 	0x08
#define ATR_DIRETORIO	0x10
#define ATR_ARQUIVO  	0x20


const unsigned int EOC = 0x0FFFFFFF; // indica final da cadeia de cluster

struct Cluster
{
  char * dados;
};

//extensão para FAT 32
typedef struct fat_extBS_32
{

	unsigned int		tam_tabela_32;    //Tamanho da Tabela FAT 
	unsigned short		ext_flags;        //Flags (controle de FAT espelho?)
	unsigned short		fat_versao;       //Versao do FAT, colocar 0
	unsigned int		cluster_raiz;     //Aponta para o primeiro cluster ocupado pelo diretório raiz
	unsigned short		fat_info;         //O número do setor em que está localizada a estrutura FSINFO(geralmente o valor é 1) 
	unsigned short		backup_BS_setor;  //Aponta para a setor em qu está localizada a copia do setor de boot. (Valor recomendado 6)
	unsigned char 		reservado_0[12];  //Área reservada para futuras expansões do formato
	unsigned char		num_drive;        //Número do Drive (0x80 para HDs e 0x00 para disquetes)  
	unsigned char 		reservado_1;      //Campo reservado (deixar em 0)  
	unsigned char		boot_ass;         //Assinatura de boot (0x29).Indica que os próximos 3 valores estão presentes
	unsigned int 		volume_id;        //Número de série do volume. Gerado a partir da combinação da data e hora em um valor de 32bits
	unsigned char		rotulo_volume[11];//Nome volume. Valor padrão "NO NAME"
	unsigned char		tipo_sistema[8];  //String com o nome do sistema de arquivos. (FAT12, FAT16, FAT) 
 
}__attribute__((packed)) fat_extBS_32_t;
 

 //Estrutura armazena no setor 0 do disco rígido (área de boot) com as informações do sistema de arquivos FAT32
typedef struct fat_BS
{
	unsigned char 		bootjmp[3];  		  //Contém o código para a instrução JUMP para o bootloader
	unsigned char 		oem_nome[8];          //Colocar “MSWIN4.1” para maior compatiblidade
	unsigned short 	    bytes_por_setor;      //Quantidade de bytes por setor (512)
	unsigned char		setores_por_cluster;  //Quantidade de setores que um cluster ocupa (1)
	unsigned short		qtd_setores_reservados;//Número de setores reservados. 
	unsigned char		qtd_tabelas;	      //Qtd de tabelas FAT no disco. Geralmente 2 (original e backup)
	unsigned short		qtd_dados_raiz;		  //Qtd de entradas no direito raiz. Usado apenas no fat12 e fat16. No fat 32 o valor deve ser 0.	
	unsigned short		num_total_setores_16; //Qtd total de setores. Usado para fat16, colocar em 0 para fat32             
	unsigned char		tipo_midia;			  //0xF8 para HDs e 0xF0 para mídias removíveis	
	unsigned short		tam_tabela_16;		  //Tamanho ocupado por uma tabela FAT. Apenas para fat16 e 12. colocar em 0
	unsigned short		setores_por_trilha;   //Setores por trilha. Em caso de acesso usando CHS
	unsigned short		qtd_cabecas_leitura;  //Qtd de cabeças de leitura. Em caso de acesso usando CHS.
	unsigned int 		num_set_escondidos;	  //Colocar em 0
	unsigned int 		qtd_total_setores_32; //Quantidade total de setores no disco
	fat_extBS_32_t		secao_extendida;	  //Seção extendida para FAT32
 
}__attribute__((packed)) fat_BS_t;


//Estrutura da fat info
typedef struct fat_info
{
	unsigned int 		fat_info_ass;  		  //Assinatura da estrutura fat_info valor (0x41615252)
	unsigned char 		reservado[480];       //Espaço reservado para expansões do fat32
	unsigned int 	    fat_info_ass2;        //Segunda assinatura da estrutura, valor(0x61417272)
	unsigned int		qtd_cluster_livres;   //Qtd de clusters livres no disco. Colocar em 0xFFFFFFFF caso o valor precise ser calculado
	unsigned int		prox_cluster_livre;   //Local do próximo cluster livre
	unsigned char		reservado2[12];	      //Segundo espaço reservado para expansões do formato
	unsigned int		fat_info_final;		  //Assinatura final da estrutura valor (0xAA550000.)	
}__attribute__((packed)) fat_info_t;


typedef struct entrada_diretorio
{
	unsigned char 		dir_nome[11];  		   //nome do arquivo
	unsigned char 		dir_atributos;         //atributos do arquivo
	unsigned char 	    nt_reservado;          //manter em 0
	unsigned char		criacao_decimos_seg;   //Parte da informação referente a data de criacao
	unsigned short		criacao_hora;          //Hora em que o arquivo foi criado
	unsigned short		criacao_data;	       //Data em que o arquivo foi criado
	unsigned short		data_ult_acesso;	   //Data último acesso
	unsigned short		num_primeiro_clusterHI;//Parte alta do número do primeiro cluster do arquivo
	unsigned short		hora_ult_modificacao;  //hora da última modificação
	unsigned short		data_ult_modificacao;  //data da última modificação
	unsigned short		num_primeiro_clusterLO;//Parte baixa do número do primeiro cluster do arquivo
	unsigned int		tam_arquivo;		   //tamanho do arquivo em bytes	
	
}__attribute__((packed)) entrada_diretorio_t;


struct No
{
	unsigned int descritor;
	unsigned char * nome;
	entrada_diretorio * arq;
};


extern fat_BS bs;
extern fat_extBS_32_t ex; 
extern fat_info_t fat_info;
extern entrada_diretorio_t dir_final;

extern char dados[TAM_CLUSTER];
extern char dados1[TAM_CLUSTER];

extern char setor_boot[TAM_CLUSTER];
extern char setor_boot1[TAM_CLUSTER];

extern char buf[TAM_CLUSTER];
extern char buf1[TAM_CLUSTER];


extern Lista<entrada_diretorio> diretorio_raiz;
extern unsigned short info_identificar[1024];


extern unsigned int primeiro_setor_dados;       //Número do primeiro setor em que arquivos e diretorios podem ser alocados
extern unsigned int primeiro_setor_fat;         //Primeiro setor ocupado pela tabela FAT
extern unsigned int num_total_setores_dados;    //Numero total de setores de dados
extern unsigned int num_total_clusters; 


//prototipos
void fat_ler_setor(unsigned char * dados, unsigned int setor_lba);
void fat_escrever_setor(unsigned char * dados, unsigned int setor_lba);
void fat_identificar(unsigned char * dados, unsigned int bus, unsigned int drive, unsigned int tipo);
unsigned int calcular_setor_absoluto(unsigned int cluster);
unsigned int obter_prox_cluster_livre();
void carregar_diretorio_raiz();
void salvar_diretorio_raiz();
void carregar_fat32();
void formatar_fat32();
void acessar_fat(unsigned int cluster,unsigned short op, unsigned int *valor);
void salvar_fatinfo();
void abrir_arquivo(char * nome, char * nome_vfs);
unsigned int salvar_arquivo(char * nome, char * nome_vfs);

#endif