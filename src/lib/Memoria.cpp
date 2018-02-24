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

typedef long Align; /* for alignment to long boundary */
union header 
{   
    /* block header */
	struct {
		union header *ptr; /* next block if on free list */
		unsigned size; /* size of this block */
	} s;
	Align x; /* force alignment of blocks */
};

typedef union header Header;

static Header base; /* empty list to get started */
static Header *freep = NULL; /* start of free list */

void free(void *ap);
void *malloc(unsigned nbytes);
static Header *morecore(unsigned nu);

/* malloc: general-purpose storage allocator */
void *malloc(unsigned nbytes)
{
	Header *p, *prevp;
	
	Header *moreroce(unsigned);
	unsigned nunits;
	
	nunits = (nbytes+sizeof(Header)-1)/sizeof(header) + 1;
	
	if ((prevp = freep) == NULL)
	{ /* no free list yet */
		base.s.ptr = freep = prevp = &base;
		base.s.size = 0;
	}
	
	for (p = prevp->s.ptr; ; prevp = p, p = p->s.ptr) 
	{
		if (p->s.size >= nunits) 
		{ /* big enough */
			if (p->s.size == nunits) /* exactly */
				prevp->s.ptr = p->s.ptr;
			else 
			{ /* allocate tail end */
				p->s.size -= nunits;
				p += p->s.size;
				p->s.size = nunits;
			}
			
			freep = prevp;
			return (void *)(p+1);
		}
		
		if (p == freep) /* wrapped around free list */
		  if ((p = morecore(nunits)) == NULL)
		    return NULL; /* none left */
	}
}

#define NALLOC 1024
static Header *morecore(unsigned nu)
{
	char *cp, *sbrk(int);
	Header *up;

	if (nu < NALLOC)
		nu = NALLOC;
		
	cp = (char *)sbk(nu * sizeof(Header));
	
	if (cp == (char *) -1) /* no space at all */
		return NULL;
		
	up = (Header *) cp;
	up->s.size = nu;
	
	free((void *)(up+1));
	
	return freep;
}

void free(void *ap)
{
	Header *bp, *p;
	bp = (Header *)ap - 1; /* point to block header */
	
	for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
	    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
	           break; /* freed block at start or end of arena */
	
	if (bp + bp->s.size == p->s.ptr) 
	{ /* join to upper nbr */
		bp->s.size += p->s.ptr->s.size;
		bp->s.ptr = p->s.ptr->s.ptr;
	} 
	else
	{
		bp->s.ptr = p->s.ptr;
	}
			
	if (p + p->s.size == bp) 
	{ /* join to lower nbr */
		p->s.size += bp->s.size;
		p->s.ptr = bp->s.ptr;
	}
	else
	{
		p->s.ptr = bp;
	}
		
	freep = p;
}


