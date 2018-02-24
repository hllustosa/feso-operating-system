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
#ifndef _AVL
#define _AVL


#include "Memoria.h"
#include "Util.h"

//========================================================================================================================


/**
*Árvore balanceçada utilizada para estruturas com acesso pontual rápido.
*/
template <class T>
class Avl
{
   int altura; 			/*!<Altura da árvore. */
   unsigned int indice;		/*!<Variável que mantém o índice sendo buscado ou inserido durante uma operação. */
   unsigned int qtd_itens;	/*!<Número de itens armazenado na árvore. */
   Avl<T> * esq;		/*!<Sub-árvore à esquerda. */
   Avl<T> * dir;		/*!<Sub-árvore à direta. */
   
   /**
   *Método de balanceçamento da árvore.
   */
   Avl<T> * rot_esq();

   /**
   *Método de balanceçamento da árvore.
   */
   Avl<T> * rot_dir();

   /**
   *Método de balanceçamento da árvore.
   */
   Avl<T> * rot_esq_dir();

   /**
   *Método de balanceçamento da árvore.
   */
   Avl<T> * rot_dir_esq();
   
   public:
   
   T * elemento;	/*!<Item armazenado no nó da árvore. */
   
   /**
   *Insere um novo item na árvore.
   */
   static Avl<T> * inserir(Avl<T> ** arv, unsigned int indice, T elemento);

   /**
   *Remove um item da árvore.
   */
   static Avl<T> * remover(Avl<T> * arv, unsigned int indice);  

   /**
   *Obtém o item com maior índice associado.
   */
   static Avl<T> * maximo(Avl<T> * arv);  

   /**
   *Obtém o item com menor índice associado.
   */
   static Avl<T> * minimo(Avl<T> * arv);  

   /**
   *Obtém um elemento com um índice especifico.
   */
   static Avl<T> * obter_elemento(Avl<T> * arv, unsigned int indice);  
   
   /**
   *Função usada para depuração.
   */
   static void dump(Avl<T> * arv, int n);
};

template <class T>
static void Avl<T>::dump(Avl<T> * arv, int n)
{
	
	 if(arv != NULL )
	 {
			for(int i =0; i<= n; i++)
			{
				//my_printf("--");
			}	
			  //my_printf(" %d\n", arv->indice);	
	 
	       
			Avl<T>::dump( arv->esq,n+1);
			Avl<T>::dump( arv->dir,n+1);
	 }
	    
}

template <class T>
Avl<T> * Avl<T>::rot_esq()
{
		Avl<T> * arv; 
		arv = this->esq;
	    this->esq = arv->dir;
	    arv->dir = this;
	
	    unsigned int alt1 = MAIOR( this->esq != NULL ? this->esq->altura :  -1, this->dir != NULL ? this->dir->altura :  -1 );
		this->altura = alt1 + 1;
		unsigned int alt2 = MAIOR( arv->esq != NULL ? arv->esq->altura :  -1, this->altura );
		arv->altura  = alt2 + 1;

		return arv;
}

template <class T>
Avl<T> * Avl<T>::rot_dir()
{
		Avl<T> * arv; 
		
		arv = this->dir;
	    this->dir = arv->esq;
	    arv->esq = this;
		
		int alt1 = MAIOR( this->esq != NULL ? this->esq->altura : -1, this->dir != NULL ? this->dir->altura :  -1 );
		this->altura = alt1 + 1;
		
		int alt2 = MAIOR( arv->dir != NULL ? arv->dir->altura :  -1, this->altura);
        arv->altura  = alt2 + 1;
			
		return arv;
}

template <class T>
Avl<T> * Avl<T>::rot_esq_dir()
{
	this->dir = this->dir->rot_esq();  
	return this->rot_dir();
}

template <class T>
Avl<T> * Avl<T>::rot_dir_esq()
{
	this->esq = this->esq->rot_dir();  
	return this->rot_esq();
}

template <class T>
static Avl<T> * Avl<T>::maximo(Avl<T> * arv)
{
	if(arv == NULL)
	{
		return NULL;
	}
	else if(arv->dir == NULL)
	{
	   return arv;
	}
	else
	{
	    return Avl<T>::maximo(arv->dir);
	}
}

template <class T>
static Avl<T> * Avl<T>::minimo(Avl<T> * arv)
{
	if(arv == NULL)
	{
	   return NULL;
	}
	else if(arv->esq == NULL)
	{
	   return arv;
	}
	else
	{
	   return Avl<T>::minimo(arv->esq);
	}
}  

template <class T>
static Avl<T> * Avl<T>::inserir(Avl<T> ** arv, unsigned int indice, T elemento)
{
	//inserindo em nó  folha
	if( (*arv) == NULL )
	{
		(*arv) = (Avl<T> *)kmalloc(sizeof(struct Avl<T>));
		(*arv)->elemento  = (T *)kmalloc(sizeof(T));
		//*(*arv)->elemento  = elemento; 
		memcpy((char *)(*arv)->elemento, (char *)&elemento, sizeof(T));
		
		(*arv)->indice    = indice;
		(*arv)->altura	  = 0;
		(*arv)->esq 	  = (*arv)->dir = NULL;
	}
	else if( indice < (*arv)->indice )
	{
		//inserindo na subárvore da esquerda, caso o índice a ser inserido
		//seja menor que o índice da nó raíz da subárvore atual
		(*arv)->esq = Avl<T>::inserir(&(*arv)->esq, indice, elemento);
	
		//verificando se a inserção deixou a árvore desbalanceada
		if( ( ((*arv)->esq != NULL? (*arv)->esq->altura :  -1 ) - ( (*arv)->dir != NULL? (*arv)->dir->altura :  -1 ))  == 2 )
		{
			if( indice < (*arv)->esq->indice )
				(*arv) = (*arv)->rot_esq(); 
			else
				(*arv) = (*arv)->rot_dir_esq();
		}
	}
	else if( indice > (*arv)->indice )
	{
		//inserindo na subárvore da direta, caso o índice a ser inserido
		//seja maior que o índice da nó raíz da subárvore atual
		(*arv)->dir = Avl<T>::inserir(&(*arv)->dir, indice, elemento);
	
		//verificando se a inserção deixou a árvore desbalanceada
		if( (( (*arv)->dir != NULL? (*arv)->dir->altura :  -1 ) - ( (*arv)->esq != NULL? (*arv)->esq->altura :  -1 ) ) == 2 )
		{
			if( indice > (*arv)->dir->indice )
				(*arv) = (*arv)->rot_dir();
			else
				(*arv) = (*arv)->rot_esq_dir();
		}
	}
	
	int aux = MAIOR( (*arv)->esq != NULL ? (*arv)->esq->altura :  -1, (*arv)->dir != NULL ? (*arv)->dir->altura :  -1 );	
	(*arv)->altura   = aux + 1;
	
	return (*arv);

}

template <class T>
static Avl<T> * Avl<T>::remover(Avl<T> * arv, unsigned int indice)
{
	if(arv == NULL)
	{
		return arv;
	}
	else if(arv->indice < indice)
	{
		arv->dir = Avl<T>::remover(arv->dir, indice);
			
		if( (( arv->dir != NULL? arv->dir->altura :  -1 ) - ( arv->esq != NULL? arv->esq->altura :  -1 ) ) == 2 )
		{
			if( indice > arv->dir->indice )
				arv = arv->rot_dir();
			else
				 arv = arv->rot_esq_dir();
		}
	}
	else if(arv->indice > indice)
	{
		arv->esq = Avl<T>::remover(arv->esq, indice);
		
		if( ( arv->esq != NULL? arv->esq->altura : -1 ) - ( arv->dir != NULL? arv->dir->altura : -1 )  == 2 )
		{
			if( indice < arv->esq->indice )
				arv = arv->rot_esq(); 
			else
				arv = arv->rot_dir_esq();
		}
	}
	else if( arv->indice == indice)
	{
		if( arv->esq == NULL )
		{
			Avl<T> * ptr = arv->dir;
			free(arv);
			return ptr;
		}
		else if(arv->dir == NULL )
		{
			Avl<T> * ptr = arv->esq;
			free(arv);
			return ptr;
		}
		else 
		{
			Avl<T> * min = Avl<T>::minimo(arv->dir);
			arv->indice =  min->indice;
			arv->elemento = min->elemento;
			
			arv->dir = Avl<T>::remover(arv->dir, min->indice);
			
			if( (( arv->dir != NULL? arv->dir->altura :  -1 ) - ( arv->esq != NULL? arv->esq->altura :  -1 ) ) == 2 )
			{
				if( indice > arv->dir->indice )
					arv = arv->rot_dir();
				else
					 arv = arv->rot_esq_dir();
			}
				
		}
	}
	
	unsigned int aux = MAIOR( arv->esq != NULL ? arv->esq->altura : -1, arv->dir != NULL ? arv->dir->altura : -1 );	
	arv->altura   = aux + 1;
			
	return arv;

}

template <class T>
static Avl<T> * Avl<T>::obter_elemento(Avl<T> * arv, unsigned int indice)
{
	if(arv == NULL)
	{
	    return NULL;
	}
	else if(arv->indice == indice)
	{
		return arv;
	}
	else if(indice < arv->indice)
	{
		return Avl<T>::obter_elemento(arv->esq, indice);
	}
	else
	{
		return Avl<T>::obter_elemento(arv->dir, indice);
	}
	
}


//========================================================================================================================
//Arvore
template <class T>
class Arvore :public Recurso
{

	Avl<T> * arvore;
	
	public :
	
	void adicionar(unsigned int chave, T item);
	void remover(unsigned int chave);
	T& operator[](unsigned int chave);
	void dump();
};

template <class T>
void Arvore<T>::adicionar(unsigned int chave, T item)
{
   down();
   Avl<T>::inserir(&arvore, chave, item);
   up();
}

template <class T>
void Arvore<T>::remover(unsigned int chave)
{
   down();
   arvore = Avl<T>::remover(arvore, chave);
   up();
}

template <class T>
T& Arvore<T>::operator[](unsigned int chave)
{
	down();
	Avl<T>* arv = Avl<T>::obter_elemento(arvore, chave);
	up();
	
	if(arv == NULL)
	{
	   T * ret = NULL;
	   return *ret;
	}
	else
	{
		return *arv->elemento;
	}
}

template <class T>
void Arvore<T>::dump()
{
	Avl<T>::dump(arvore, 0);
}

#endif
