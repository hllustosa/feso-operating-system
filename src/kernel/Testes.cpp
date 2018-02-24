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
#include "include/Testes.h"
Testes testes;

void Testes::inicializar()
{
	int desc;
	vfs.abrir("/testes", 'D', &desc);
	vfs.fechar(desc);

	testar_mem_fisica();
	testar_mem_virtual();
	testar_escalonador();
	
}

void Testes::testar_mem_fisica()
{
	unsigned int blocos_alocados[10], erro[5] = {0,0,0,0,0};
	int blocos_livres_ini, blocos_livres_fim, desc;
	
	vfs.abrir("/testes/mem_fisica",'a', &desc);
	
	blocos_livres_ini = mem_fisica.obter_blocos_livres();
	for(int i =0; i < 10; i++)
	{
		blocos_alocados[i] = mem_fisica.alocar(1);
		
		if((blocos_alocados[i] % 4096) != 0)
		{
			erro[0] = 1;
		}
		
		if(i > 0)
		{
			if(blocos_alocados[i] != (blocos_alocados[i-1] + 4096))
			{
				erro[1] = 1;
			}
		}
	}
	
	blocos_livres_fim = mem_fisica.obter_blocos_livres();
	
	if(blocos_livres_ini != (blocos_livres_fim + 10))
	{
		erro[2] = 1;
	}
	
	for(int i =0; i < 10; i++)
	{
		mem_fisica.desalocar(blocos_alocados[i], 1);
	}
	
	blocos_livres_fim = mem_fisica.obter_blocos_livres();
	if(blocos_livres_ini != blocos_livres_fim)
	{
		erro[3] = 1;
	}
	
	mem_fisica.desalocar(0x420042, 1);
	mem_fisica.desalocar(0x324212, 1);
	mem_fisica.desalocar(0, 1);
	
	blocos_livres_fim = mem_fisica.obter_blocos_livres();
	if(blocos_livres_ini != blocos_livres_fim)
	{
		erro[4] = 1;
	}
	
	char msg[100];
	
	for(int i =0; i < 5; i++)
	{
		switch(i)
		{
			case 0:memcpy(msg,"Alinhamento dos blocos ", 25);break;
			case 1:memcpy(msg,"Enderecos dos blocos ", 22);break;
			case 2:memcpy(msg,"Contabilizacao alocacao ", 25);break;
			case 3:memcpy(msg,"Contabilizacao desalocacao ", 28);break;
			case 4:memcpy(msg,"Contabilizacao desalocacao falsa ", 33);break;
		}
		
		vfs.escrever(desc, msg, strlen(msg));
		vfs.escrever(desc, " ", 1);
		
		if(erro[i])
		{
			vfs.escrever(desc, "erro\n", 6);
		}
		else
		{
			vfs.escrever(desc, "ok\n", 4);
		}
	}
	
	vfs.fechar(desc);
}

void Testes::testar_mem_virtual()
{
	unsigned int pdir_atual, pdir_copia, pdir_atual_fisico;
	unsigned int pdir_aux, desc;
	unsigned int erro[4] = {0,0,0,0};
	unsigned char msg[100];
	
	vfs.abrir("/testes/mem_virtual",'a', &desc);
	
	//backup do endereço físico
	__asm__ __volatile__ ("mov %%cr3, %0":"=b"(pdir_atual_fisico));
	
	//criando nova pdir
	pdir_copia = mem_fisica.alocar(1);
	
	//mapeando em um endereço virtual auxíliar
	mem_virtual.mapear_page_table(pdir_copia, mem_virtual.pt_aux, 0x03);
	
	//obtendo pdir atual
	pdir_atual = mem_virtual.obter_pdir();
	
	//copiando o espaco de enderecos
	for(int i = 1; i < 1024; i++)
	{
	   ((int *)mem_virtual.pt_aux)[i] = ((unsigned int *)pdir_atual)[i];
	}
	
	//configurando a última entrada para apontar para a própria page directory
	((int *)mem_virtual.pt_aux)[1023] = ((unsigned int)(pdir_copia)) | 0x03;

	//alterando pdir
	mem_virtual.carregar_page_directory(pdir_copia);
	
	//obtendo pdir_atual
	__asm__ __volatile__ ("mov %%cr3, %0":"=b"(pdir_aux));
	
	if(pdir_copia != pdir_aux)
	{
		erro[0] = 1;
	}
	
	for(int i = 0; i < 3; i++)
	{
		unsigned int end_virtual, end_fisico;
		
		end_virtual = end_fisico = 4096 * i;
		mem_virtual.mapear_page_table_espaco_end(end_fisico, end_virtual, 0x03, mem_virtual.pt_aux);
		
		if(end_fisico != mem_virtual.obter_end_fisico(end_virtual))
		{
			erro[i+1] = 1;
		}
		
		//Escrevendo e lendo valor em endereço virtual recém mapeando
		//A inicialização do kernel pode falhar aqui, caso haja erro
		*((unsigned int *)end_virtual) = *((unsigned int *)end_virtual) + 0x42;
	
	}
	
	
	for(int i =0; i < 4; i++)
	{
		switch(i)
		{
			case 0:memcpy(msg,"Alteracao de page directory ", 29);break;
			case 1:memcpy(msg,"Mapeamento 1 ", 14);break;
			case 2:memcpy(msg,"Mapeamento 2 ", 14);break;
			case 3:memcpy(msg,"Mapeamento 3 ", 14);break;
		}
		
		vfs.escrever(desc, msg, strlen(msg));
		vfs.escrever(desc, " ", 1);
		
		if(erro[i])
		{
			vfs.escrever(desc, "erro\n", 6);
		}
		else
		{
			vfs.escrever(desc, "ok\n", 4);
		}
	}
	
	vfs.fechar(desc);

	//retornando ao pdir padrão
	mem_virtual.carregar_page_directory(pdir_atual_fisico);
	mem_fisica.desalocar(pdir_copia, 1);
	vfs.fechar(desc);
}

void Testes::testar_escalonador()
{
	unsigned int desc, desc_imagem, desc_imagem_falsa;
	unsigned int pid, erro[4] = {0,0,0,0};
	unsigned char msg[100], * imagem;
	
	vfs.abrir("/testes/escalonador",'a', &desc);
	
	vfs.abrir("/modulos/editor.o",'A', &desc_imagem);
	imagem = vfs.obter_imagem(desc_imagem);
	vfs.fechar(desc_imagem);
	
	pid = escalonador.prox_pid;
	escalonador.adicionar_processo("proc_teste", imagem, Processo::USUARIO, 0, NULL);
	
	if(escalonador.prox_pid != (pid+1))
	{
		erro[0] = 1;
	}
	
	Processo &p = escalonador.obter_processo(pid);
	
	if(&p != NULL)
	{
		testar_processo(p, desc);
		escalonador.eliminar_processo(p);
	}
	else
	{
		erro[1] = 1;
	}
	
	
	
	
	pid = escalonador.prox_pid;
	
	vfs.abrir("/testes/arquivo_falos",'a', &desc_imagem_falsa);
	imagem = vfs.obter_imagem(desc_imagem_falsa);
	vfs.fechar(desc_imagem_falsa);
	escalonador.adicionar_processo("proc_teste", imagem, Processo::USUARIO, 0, NULL);
	
	Processo &p2 = escalonador.obter_processo(pid);
	
	if(&p2 != NULL)
	{
		erro[2] = 1;
	}

	for(int i =0; i < 3; i++)
	{
		switch(i)
		{
			case 0:memcpy(msg,"Incremento do PID ", 19);break;
			case 1:memcpy(msg,"Criacao do Processo ", 21);break;
			case 2:memcpy(msg,"Criacao do Processo falso ", 27);break;
		}
		
		vfs.escrever(desc, msg, strlen(msg));
		vfs.escrever(desc, " ", 1);
		
		if(erro[i])
		{
			vfs.escrever(desc, "erro\n", 6);
		}
		else
		{
			vfs.escrever(desc, "ok\n", 4);
		}
	}
	
	vfs.fechar(desc);
}

void Testes::testar_processo(Processo& p, int desc)
{
	unsigned int qtd_threads_proc, qtd_threads_prontas, pdir_atual_fisico;
	unsigned int pid_thread1, pid_thread2, pilha_thread1, pilha_thread2;
	unsigned int erro[4] = {0,0,0,0};
	unsigned char msg[100], * imagem;
	
	qtd_threads_proc 	= p.threads.tamanho();
	qtd_threads_prontas = escalonador.threads_prontas.tamanho();
	
	__asm__ __volatile__ ("mov %%cr3, %0":"=b"(pdir_atual_fisico));
	mem_virtual.carregar_page_directory(p.pdir);
	
	pid_thread1 = escalonador.adicionar_thread(p, p.entrada);
	Thread& thread1 = escalonador.obter_thread(pid_thread1);
	
	if(&thread1 != NULL)
	{
		pilha_thread1 = thread1.inicio_pilha;
		escalonador.eliminar_thread(thread1);
		
		pid_thread2 = escalonador.adicionar_thread(p, p.entrada);
		Thread& thread2 = escalonador.obter_thread(pid_thread2);
		
		if(&thread2 != NULL)
		{
			if(thread2.inicio_pilha != pilha_thread1)
			{
				erro[2] = 1;
			}
			
			escalonador.eliminar_thread(thread2);
		}
		else
		{
			erro[1] = 1;
		}
	}
	else
	{
		erro[1] = 1;
	}
	
	if(qtd_threads_proc !=  p.threads.tamanho() )
	{
		erro[0] = 1;//contabilização das threads
	}
	
	for(int i =0; i < 3; i++)
	{
		switch(i)
		{
			case 0:memcpy(msg,"contabilizacao dos threads ", 28);break;
			case 1:memcpy(msg,"Criacao de thread ", 19);break;
			case 2:memcpy(msg,"Enderecos das pilhas ", 22);break;			
		}
		
		vfs.escrever(desc, msg, strlen(msg));
		vfs.escrever(desc, " ", 1);
		
		if(erro[i])
		{
			vfs.escrever(desc, "erro\n", 6);
		}
		else
		{
			vfs.escrever(desc, "ok\n", 4);
		}
	}
	
	mem_virtual.carregar_page_directory(pdir_atual_fisico);
}
