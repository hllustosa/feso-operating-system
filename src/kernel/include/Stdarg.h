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
#ifndef _INC_STDARG
#define _INC_STDARG

#define __NEEDS_ROUND 1
#include "__def.h"

#ifndef _VA_LIST_DEFINED

#if defined(arm)
#if defined(_M_ARM)
typedef char * va_list;
#elif defined(__HIGHC__) || defined(__GNUC__) || defined(ADS)
typedef void *va_list;
#else
typedef char *va_list[1];
#endif /*_M_ARM*/
#define _VA_LIST_DEFINED

#elif defined(__TCS__)
typedef void * _VA_LIST_;
typedef _VA_LIST_ va_list;
#define _VA_LIST_DEFINED

#elif defined(mips)
typedef char * va_list;
#ifdef __MIPSEB__
#define va_start(_ap_, _v_) \
  (_ap_ = (char*) __builtin_next_arg (_v_))
#define va_arg(_ap_, _type_) \
  ((_ap_ = (char *) ((__alignof__ (_type_) > 4 \
                       ? __ROUND((int)_ap_,8) : __ROUND((int)_ap_,4)) \
                     + __ROUND(sizeof(_type_),4))), \
   *(_type_ *) (void *) (_ap_ - __ROUND(sizeof(_type_),4)))
#define va_end(list)
#else
/* These are from NT */
#define va_start(ap,v) ap  = (va_list)&v + sizeof(v)
#define va_end(list)
#define va_arg(list, mode) ((mode *)(list =\
 (char *) ((((int)list + (__alignof__(mode)<=4?3:7)) &\
 (__alignof__(mode)<=4?-4:-8))+sizeof(mode))))[-1]
#endif
#define _VA_LIST_DEFINED

#elif defined(__H8300__)
/* later */

#else
typedef char *  va_list;
#define _VA_LIST_DEFINED
#endif
#endif /* _VA_LIST_DEFINED */


#ifdef  _M_IX86

#define _INTSIZEOF(n)   ((sizeof(n) + sizeof(INT) - 1) & ~(sizeof(INT) - 1)) 

#define va_start(ap,v)  (ap = (va_list)&v + _INTSIZEOF(v))
#define va_arg(ap,t)    (*(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)))
#define va_end(ap)      (ap = (va_list)0)

#elif   defined(_M_MRX000)

/* Use these types and definitions if generating code for MIPS */

#define va_start(ap,v)  ap = (va_list)&v + sizeof(v)
#define va_end(list)
#define va_arg(list, mode) ((mode *)(list = \
            (BYTE *) ((((INT)list + (__builtin_alignof(mode)<=4?3:7)) &\
            (__builtin_alignof(mode)<=4?-4:-8))+sizeof(mode))))[-1]

#elif   defined(arm)

#if defined(_M_ARM)

#define _INTSIZEOF(n)   ((sizeof(n) + sizeof(INT) - 1) & ~(sizeof(INT) - 1)) 

#define va_start(ap,v)  (ap = (va_list)&v + _INTSIZEOF(v))
#define va_arg(ap,t)    (*(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)))
#define va_end(ap)      (ap = (va_list)0)

#elif defined(__HIGHC__)

/*
 * __vararg_char is used as a pseudonym for "char". The compiler would
 * ordinarily warn about "misbehaviour" if an arbitrary pointer is cast 
 * to or from (char *) at high optimization levels.
 * To avoid this problem we use "__vararg_char" inplace of "char".
 * The compiler special-cases the type (__vararg_char *) and doesn't put
 * out the warning.
 */
typedef char __vararg_char;
#define va_start(_ap_,_parmN_) ((_ap_)=(__vararg_char *)&(_parmN_) + \
                ((sizeof(_parmN_)+(sizeof(int)-1)) & ~(sizeof(int)-1)))
#define _NNVAARG (-1)
#define va_arg(_ap_,_type_)\
      ( *(_type_ *) ((__vararg_char *)(\
                    _ap_ = (__vararg_char *)_ap_ + ((sizeof(_type_) +3) &~3)\
                              ) - ((sizeof(_type_) +3) &~3)\
                        + (sizeof(_type_)<=_NNVAARG?4-sizeof(_type_):0)\
                     )\
      )
#define va_end(_ap_) ((void)0)

#elif defined(__GNUC__) || defined(ADS)

/* Define __gnuc_va_list. */

#ifndef __GNUC_VA_LIST
#define __GNUC_VA_LIST
typedef void *__gnuc_va_list;
#endif

#define __va_rounded_size(TYPE)  \
  (((sizeof (TYPE) + sizeof (long) - 1) / sizeof (long)) * sizeof (long))

#ifdef ADS
#define va_start(AP, LASTARG) (void)((AP) = __va_start(LASTARG))
#else
#define va_start(AP,LASTARG) \
  (AP = ((__gnuc_va_list) __builtin_next_arg (LASTARG)))
#endif

#if BYTE_ORDER == LITTLE_ENDIAN
/* This is for little-endian machines; small args are padded upward.  */
#define va_arg(AP, TYPE)                                                \
 (AP = (__gnuc_va_list) ((char *) (AP) + __va_rounded_size (TYPE)),     \
  *((TYPE *) (void *) ((char *) (AP) - __va_rounded_size (TYPE))))
#else
/* This is for big-endian machines; small args are padded downward.  */
#define va_arg(AP, TYPE)                                                \
 (AP = (__gnuc_va_list) ((char *) (AP) + __va_rounded_size (TYPE)),     \
  *((TYPE *) (void *) ((char *) (AP)                                    \
                       - ((sizeof (TYPE) < __va_rounded_size (char)     \
                           ? sizeof (TYPE) : __va_rounded_size (TYPE))))))
#endif

#define va_end(AP)      ((void) 0)

/* Copy __gnuc_va_list into another variable of this type.  */
#define __va_copy(dest, src) (dest) = (src)



/* compressed version of GCC's stdarg.h extra junk */

/* The macro _VA_LIST_DEFINED is used in Windows NT 3.5  */
#ifndef _VA_LIST_DEFINED
typedef __gnuc_va_list va_list;
#define _VA_LIST_DEFINED
#endif

#define _VA_LIST_   /* not sure about this */

#else

/* Use this for ARM and Norcroft C compiler */

#define __alignof(_t_) \
   ((char *)&(((struct{char __member1; \
                       ___type _t_ __member2;}*) 0)->__member2) - \
    (char *)0)
#define __alignuptotype(_p_,_t_) \
   ((char *)((int)(_p_) + (__alignof(_t_)-1) & ~(__alignof(_t_)-1)))

#define va_start(_ap_,_parmN_) \
    (void)(*(_ap_) = (char *)&(_parmN_) + sizeof(_parmN_))
#define va_end(_ap_) ((void)(*(_ap_) = (char *)-256))
#define va_arg(_ap_,_t_) \
   *(___type _t_ *)((*(_ap_)=__alignuptotype(*(_ap_),_t_)+sizeof(___type _t_))-\
                     sizeof(___type _t_))

#endif /* arm */

#elif defined(__TCS__)

/* Definitions for TriMedia compiler */
/* Rounding macros. */
#define _va_round_var(var)      ((sizeof(var)  < 4) ? 4 - sizeof(var)  : 0)
#define _va_roundup_var(var)    ((sizeof(var)  < 4) ? 4                : sizeof(var))
#define _va_round(type)         ((sizeof(type) < 4) ? 4 - sizeof(type) : 0)
#define _va_roundup(type)       ((sizeof(type) < 4) ? 4                : sizeof(type))

#define va_start(ap,lastarg) (ap = (va_list) (void *) &lastarg + _va_roundup_var(lastarg))

#define va_arg(ap,type) ((ap = ((char *)ap) + (_va_roundup(type))), *(type *) (ap + _va_round(type) - _va_roundup(type)))

#define va_end(ap)      ((void)0)


#elif defined(mips)

//#include <va-mips.h>


#elif defined(__H8300__)

#include <va-h8300.h>

#elif   defined(ppc) && defined(_MSC_VER)

#ifdef  __cplusplus
#define _ADDRESSOF(v)   ( &reinterpret_cast<const char &>(v) )
#else
#define _ADDRESSOF(v)   ( &(v) )
#endif

#define _VA_ALIGN(t)    8
#define _VA_IS_LEFT_JUSTIFIED(t) (sizeof(t) > _VA_ALIGN(t) || 0 != (sizeof(t) & (sizeof(t)-1)))

/* bytes that a type occupies in the argument list */
#define _INTSIZEOF(n)   ( (sizeof(n) + _VA_ALIGN(n) - 1) & ~(_VA_ALIGN(n) - 1) )
/* return 'ap' adjusted for type 't' in arglist */
#define _ALIGNIT(ap,t)  ( ((int)(ap) + _VA_ALIGN(t) - 1) & ~(_VA_ALIGN(t) - 1) )

#define va_start(ap,v)  ( ap = ( _VA_IS_LEFT_JUSTIFIED(v) ? (va_list)_ADDRESSOF(v) + _INTSIZEOF(v) \
                                                          : (va_list)(&(v)+1) ))

#define va_arg(ap,t) (ap = (va_list) (_ALIGNIT(ap, t) + _INTSIZEOF(t)), \
                      _VA_IS_LEFT_JUSTIFIED(t) ? *(t*)((ap) - _INTSIZEOF(t)) \
                                               : ((t*)(ap))[-1] )

#define va_end(ap)      ( ap = (va_list)0 )

#else

/* A guess at the proper definitions for other platforms */

#define _INTSIZEOF(n)   ((sizeof(n) + sizeof(INT) - 1) & ~(sizeof(INT) - 1))

#define va_start(ap,v)  (ap = (va_list)&v + _INTSIZEOF(v))
#define va_arg(ap,t)    (*(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)))
#define va_end(ap)      (ap = (va_list)0)

#endif

#endif  /* _INC_STDARG */