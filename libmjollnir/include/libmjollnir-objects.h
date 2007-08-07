/**
 * @file libmjollnir-objects.h
 *
 * 2001-2007 Devhell Labs, Asgardlabs
 * 
 * @brief All structures of libmjollnir
 *
 * $Id: libmjollnir-objects.h,v 1.3 2007-08-07 07:13:27 may Exp $
 *
 */
#if !defined(__MJR_BLOCKS__)
 #define __MJR_BLOCKS__ 1

#include "libelfsh.h"

#define MJR_BLOCK_GET_STRICT 0
#define MJR_BLOCK_GET_FUZZY  1

/* XXX: Change size in libaspect/types.c if this struct is changed */
/* XXX: Always let the vaddr and size field in first for the next 2 structures !!!! */

/**
 * @brief Structure used to describe blocks in memory 
 */
typedef struct	s_iblock 
{
  elfsh_Addr	vaddr;		/* !< @brief block starting virtual address    */
  u_int		size;		/* !< @brief block code size                   */
  u_int		symoff;		/* !< @brief block name offset in string table */
}		mjrblock_t;

/** 
 * @brief Structure used to described functions in memory 
 */
typedef struct	s_function 
{
  elfsh_Addr	vaddr;		/* !< @brief Function virtual address */
  u_int		size;		/* !< @brief Function size */
  char		name[64];	/* !< @brief Function name */
  mjrblock_t	*first;		/* !< @brief First function block */
  char		md5[34];	/* !< @brief MD5 Checksum */
}		mjrfunc_t;


/*
 * struct s_link is to reference links between functions or blocks
 * field type may help us to analyse blocks to build higher
 * logical structures. It contains :
 *
 * - a type field specifying which kind of link this is
 * - an id field to reference the destination container of this link
 * - a pointer to the next link in this list of links
 */

/* XXX: the type field has to be changed for a real eresi type id if 
   we hope to do any shape analysis in ERESI */
typedef	struct	s_link 
{
#define MJR_LINK_FUNC_CALL		0 /* !< @brief a call between functions	*/
#define MJR_LINK_FUNC_RET		1 /* !< @brief a function returning control */
#define MJR_LINK_BLOCK_COND_TRUE	2 /* !< @brief 'true' condition of a branch */
#define MJR_LINK_BLOCK_COND_FALSE	3 /* !< @brief 'false' condition of a branch */
#define MJR_LINK_BLOCK_COND_ALWAYS	4 /* !< @brief uncoditional branch */
#define MJR_LINK_TYPE_DELAY		5 /* !< @brief generally ignored but useful */
#define	MJR_LINK_UNKNOWN		6 /* !< @brief unknown type */
  unsigned int	id;
  int		type;
}		mjrlink_t;


 
/** 
 * @brief An history entry 
 */
typedef struct		s_history
{
  elfsh_Addr  		vaddr;
  asm_instr		instr;
}			mjrhistory_t;

/**
 * @brief The context of a single session 
*/
typedef struct		_mjrContext 
{
  elfshobj_t		*obj;			/* !< @brief ELF binary object */
  asm_processor		proc;			/* !< @brief ASM processor */
  container_t	*curblock;		/* !< @brief Current working block */
  container_t	*curfunc;		/* !< @brief Current working function */
  container_t	**reg_containers;	/* !< @brief Registered containers */
  btree_t		*block_btree;		/* !< @brief Blocks Binary tree (speedup parent search) */
  u_int			cntnrs_size;		/* !< @brief Size of current containers */
  u_int			next_id;		/* !< @brief Next free container id */

#define			MJR_HISTORY_LEN		5
#define			MJR_HISTORY_PPREV	(MJR_HISTORY_LEN - 3)
#define			MJR_HISTORY_PREV	(MJR_HISTORY_LEN - 2)
#define			MJR_HISTORY_CUR		(MJR_HISTORY_LEN - 1)
  mjrhistory_t		hist[MJR_HISTORY_LEN];  /* !< @brief History of instructions */

  hash_t		funchash;		/* !< @brief functions hash table */
  hash_t		blkhash;		/* !< @brief blocks hash table for this obj */
  unsigned char		analysed;		/* !< @brief do we analysed it */
  u_int			calls_seen;		/* !< @brief how many CALL we have seen */
  u_int			calls_found;		/* !< @brief how many dest has beed resolved */
}			mjrcontext_t;


/** 
 * @brief The session structure. Yes, libmjollnir is multisession 
 */
typedef struct	s_session
{
  mjrcontext_t	*cur;
  hash_t	ctx;
}		mjrsession_t;


/**
 * @brief Control flow dump options 
 */
typedef struct	s_disopt
{
  elfshobj_t	*file;
  u_int		counter;
  int		level;
}		mjropt_t;

/* Mjollnir buffer */
typedef struct	s_buf
{
  char		*data;
  u_int		maxlen;
  u_int		allocated;
  u_int		counter;
  elfshobj_t	*obj;
}		mjrbuf_t;


/***** That structure is not used yet ******/
/* Abstract representation for a condition */
typedef struct	s_condition 
{
#define CONDITION_SIGNED
#define CONDITION_UNSIGNED
  int			sign;
  char		*dst;
  char		*src;
  char		*cmp;
}		mjrcond_t;


#endif
