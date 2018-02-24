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
#ifndef _LISTA
#define _LISTA

#include "Memoria.h"
#include "Util.h"


/**
*Template para item da lista encadeada. 
*/
template<class T>
struct Item
{
  T item;		/*!<Variável que armazena o item propriamente dito. */
  Item<T> *prox;	/*!<Ponteiro para o próximo item da lista. */
  Item<T> *ant;		/*!<Ponteiro para o item anterior da lista. */
};


/**
*Lista encadeada genérica baseada em containers. 
*/
template <class T>
class Lista: public Recurso
{
  unsigned long ult_elemento_acessado;		/*!<Cache do último element acessado. */
  unsigned long _tamanho;			/*!<Número de elementos na lista encadeada. */
  Item<T> *ult_elemento_acessado_ptr;		/*!<Ponteiro para o último elemento acessado. */
  Item<T> *primeiro;				/*!<Primeiro elemento da lista. */
  
  /**
  *Obtem um item da lista. Implementação base para o operador sobrecarregada []. 
  */
  Item<T> *obter_item(unsigned long);
      
  public :

  Lista();		
					
  /**
  *Retorna o item da lista na posição indicada entre []. 
  */
  T& operator[](unsigned long indice);		

  /**
  *Adiciona um item no final da lista encadeada. 
  */
  void adicionar(T item);

  /**
  *Adiciona um item na lista encadeada em uma posição específica. 
  */
  void adicionar_em(unsigned long pos, T item);

  /**
  *Remove um item específico da lista. 
  */
  void remover(unsigned long indice);

  /**
  *Obtém o número de elementos da lista. 
  */
  unsigned long tamanho();

  /**
  *Remove todos os elementos da lista. 
  */
  void limpar();
};

//Construtor
template <class T>
Lista<T>::Lista()
{
	ult_elemento_acessado = -1;
	_tamanho = 0;
	id_dono=-1;
	mutex =0;
	rec_num = 0;
	primeiro = NULL;
}

//Obtem um item da Lista
template <class T>
Item<T> *Lista<T>::obter_item(unsigned long indice)
{
 
  //indice deve ser maior que 0 e menor que o tamanho da estrutura
  int pos = 0;
  Item<T> *aux = primeiro;
  
  if( (indice > 0) && (indice < _tamanho))
  {
		//Varrendo a estrutura até chegar na posição desejada
		while(pos != indice)
		{
		   aux = aux->prox;
		   pos++;
		}
		
		return aux;
  }
  else if(indice == 0)     
  {
		return  primeiro;   
  }
  else
  { 
		return NULL;
  }

}

//Operador [] para acessar elemenos como uma em matriz
template <class T>
T& Lista<T>::operator[](unsigned long indice)
{  
  //otimizacao para acessos sequenciais. Caso o item solicitado seja o sucessor do ultimo
  //item selecionado, retornar o que está em cache
  if((ult_elemento_acessado + 1 == indice) && (ult_elemento_acessado != -1))
  {
	   down();
	   ult_elemento_acessado = indice;
   
	   if (ult_elemento_acessado_ptr->prox != NULL)
	   {
		   ult_elemento_acessado_ptr = ult_elemento_acessado_ptr->prox;
		
		   up();	
		   return ult_elemento_acessado_ptr->item;
	   }
	   else
	   {
			up();
			return primeiro->item;
	   }
  }
  else
  {
	   ult_elemento_acessado = indice;
       ult_elemento_acessado_ptr = obter_item(indice);
	   
       return ult_elemento_acessado_ptr->item;
  }
   
}

//adicionar elementos no final da lista
template <class T>
void Lista<T>::adicionar(T item)
{
	down();
	
    //alocando
    Item<T> *novo = (Item<T>*) kmalloc(sizeof(Item<T>));
   
    novo->item = item; 
	novo->prox = NULL;
    novo->ant  = NULL;
    
    if(_tamanho > 0) 
    {
		novo->ant = obter_item(_tamanho - 1);         
		novo->ant->prox = novo;
    } 
    else
    {
		primeiro = novo;
    }
    
    _tamanho++;

	up();
}


//Adicionar elementos em uma posição especifíca
template <class T>
void Lista<T>::adicionar_em(unsigned long pos, T item)
{
   down();
   
   if( (pos > 0) && (pos < _tamanho))
   {
   
     //obtendo item na posicao especificada  
     Item<T> *atual    = obter_item(pos);
     Item<T> *anterior = atual->ant;
	 
	 
	 //alocando
     Item<T> *novo = (Item<T>*) kmalloc(sizeof(Item<T>));
	 //novo->item = item;
	 memcpy((char *)&novo->item, (char *)&item, sizeof(T));
	 
	 //se houver valor anterior
	 if(anterior == NULL)
	 {
	   primeiro = novo;
	   novo->ant = NULL;
	 }
	 else
	 {
       anterior->prox = novo;  
	   novo->ant = anterior;
	 }
	 
	 novo->prox = atual;
	 atual->ant = novo;
	 
	 _tamanho++;
   }
   else if(pos == 0)
   {
      //alocando
     Item<T> *novo = (Item<T>*) kmalloc(sizeof(Item<T>));
	 novo->item = item;
	 novo->prox = primeiro;
	 
	 if(primeiro!= NULL)
	   primeiro->ant = novo;
	   
	 primeiro = novo;
	 _tamanho++;
	 
   }
   else
   {
     adicionar(item);
   }
   
	up();
}



//Remover elementos de uma posição da lista
template <class T>
void Lista<T>::remover(unsigned long indice)
{
   down();
   if(_tamanho > 0)
   {
	   if(indice == 0)
	   {
			Item<T> *aux = primeiro;
			
			if(primeiro->prox != NULL)
			{
				primeiro = primeiro->prox;
				
				if(primeiro->prox != NULL)
					primeiro->prox->ant = primeiro;
			}
			else
			{
				primeiro = NULL;
			}
			
			free(aux);
			_tamanho--;
	   }
	   else 
	   {
		
			Item<T> *aux = obter_item(indice);
			if((aux != NULL))
			{
				aux->ant->prox = aux->prox;
				 
				if(aux->prox != NULL)
					aux->prox->ant = aux->ant;
					
				free(aux);
				_tamanho--;
			}
		 
	   }
   }
   
   up();
}

//Obter o tamanho da lista
template <class T>
unsigned long Lista<T>::tamanho()
{
  return _tamanho;
}

//Remover todos os elementos da lista
template <class T>
void Lista<T>::limpar()
{
	down();
	
    Item<T> *aux = primeiro;
   
	//Varrendo a estrutura até chegar na posição desejada
	while(aux != NULL)
	{
	   Item<T> * rem = aux;
	   aux = aux->prox;
	   free(rem);
	}
	
  
   primeiro = NULL;
   _tamanho = 0;   
   
   up();
}


#endif
