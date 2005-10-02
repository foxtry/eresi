/*
** rel.c for elfsh
**
** Started on  Fri Nov  2 15:19:19 2001 mayhem
**
*/
#include "elfsh.h"


/* Choose relocation tables strings array giving the architecture */
elfshconst_t    *vm_getrelascii(elfshobj_t *file)
{

  ELFSH_PROFILE_IN(__FILE__, __FUNCTION__, __LINE__);

  switch (elfsh_get_arch(file->hdr))
    {
    case EM_386:
      /* XXX: case EM_486: */
      ELFSH_PROFILE_ROUT(__FILE__, __FUNCTION__, __LINE__, 
			 (elfsh_rel_type_i386));
    case EM_SPARC:
    case EM_SPARC32PLUS:
    case EM_SPARCV9:
      ELFSH_PROFILE_ROUT(__FILE__, __FUNCTION__, __LINE__, 
			 (elfsh_rel_type_sparc));
    case EM_ALPHA:
#if EM_ALPHA != EM_ALPHA_EXP
    case EM_ALPHA_EXP:
#endif
      ELFSH_PROFILE_ROUT(__FILE__, __FUNCTION__, __LINE__, 
			 (elfsh_rel_type_alpha));
    case EM_IA_64:
      ELFSH_PROFILE_ROUT(__FILE__, __FUNCTION__, __LINE__, 
			 (elfsh_rel_type_ia64));
    case EM_MIPS:
    case EM_MIPS_RS3_LE:
      ELFSH_PROFILE_ROUT(__FILE__, __FUNCTION__, __LINE__, 
			 (elfsh_rel_type_mips));
    default:
      ELFSH_PROFILE_ERR(__FILE__, __FUNCTION__, __LINE__, 
			"Relocations ASCII tables not available", (NULL));
    }
}


/* Return the max number of relocation type for this architecture */
int           vm_getmaxrelnbr(elfshobj_t *file)
{

  ELFSH_PROFILE_IN(__FILE__, __FUNCTION__, __LINE__);

  switch (elfsh_get_arch(file->hdr))
    {
    case EM_386:
      /* XXX: case EM_486: */
      ELFSH_PROFILE_ROUT(__FILE__, __FUNCTION__, __LINE__, (ELFSH_RELOC_i386_MAX));
    case EM_SPARC:
    case EM_SPARC32PLUS:
      ELFSH_PROFILE_ROUT(__FILE__, __FUNCTION__, __LINE__, (ELFSH_RELOC_SPARC_MAX));
    case EM_SPARCV9:
      ELFSH_PROFILE_ROUT(__FILE__, __FUNCTION__, __LINE__, (ELFSH_RELOC_SPARC64_MAX));
    case EM_ALPHA:
#if EM_ALPHA != EM_ALPHA_EXP
    case EM_ALPHA_EXP:
#endif
      ELFSH_PROFILE_ROUT(__FILE__, __FUNCTION__, __LINE__, (ELFSH_RELOC_ALPHA_MAX));
    case EM_IA_64:
      ELFSH_PROFILE_ROUT(__FILE__, __FUNCTION__, __LINE__, (ELFSH_RELOC_IA64_MAX));
    case EM_MIPS:
    case EM_MIPS_RS3_LE:
      ELFSH_PROFILE_ROUT(__FILE__, __FUNCTION__, __LINE__, (ELFSH_RELOC_MIPS_MAX));
    default:
      ELFSH_PROFILE_ERR(__FILE__, __FUNCTION__, __LINE__, "Unknown architecture", (-1));
    }
}


/* Display relocation entries */
int		cmd_rel()
{
  elfshsect_t	*sect;
  elfsh_Rel	*rel;
  regex_t	*tmp;
  char		*type;
  char		*typeshort;
  char		*name;
  u_int		index;
  u_int		index2;
  u_int		typenum;
  char		buff[256];
  u_int         size;
  elfshconst_t  *types;
  char		addstr[32];
  char		logbuf[BUFSIZ];
  void		*data;

  ELFSH_PROFILE_IN(__FILE__, __FUNCTION__, __LINE__);

  /* Sanity checks */
  sect = elfsh_get_reloc(world.curjob->current, 0, &size);
  if (sect == NULL)
    RET(-1);

  /* Choose between global or local regx */
  FIRSTREGX(tmp);
  snprintf(logbuf, BUFSIZ - 1, " [RELOCATION TABLES]\n [Object %s]\n\n", 
	   world.curjob->current->name);
  vm_output(logbuf);

  /* We need to iterate as much as there is .rel* sections */
  for (index2 = 0; sect; index2++)
    {

      snprintf(logbuf, BUFSIZ - 1,
	       " {Section %s} \n", elfsh_get_section_name(world.curjob->current, sect));
      vm_output(logbuf);

      /* Iterate on the .rel entries array for each .rel section */
      data = elfsh_get_raw(sect);
      for (index = 0; index < size; index++)
	{

	  /* Get the current relocation entry */
	  if (sect->shdr->sh_type == SHT_RELA)
	    {

	      rel = (void *) ((elfsh_Rela *) data + index);
	      snprintf(addstr, sizeof(addstr), "add[%s]",
		       vm_colornumber("%08u", (unsigned int) ((elfsh_Rela *) rel)->r_addend));
	    }
	  else
	    {
	      rel = (elfsh_Rel *) data + index;
	      addstr[0] = 0x00;
	    }


	  /* Get linked symbol name */
	  name = elfsh_get_symname_from_reloc(world.curjob->current, rel);
	  typenum  = elfsh_get_reltype(rel);
	  types = vm_getrelascii(world.curjob->current);

	  type      = (char *) (typenum > ELFSH_RELOC_MAX(world.curjob->current) ? NULL :
				types[typenum].desc);
	  typeshort = (char *) (typenum > ELFSH_RELOC_MAX(world.curjob->current) ? NULL :
				types[typenum].name);

	  /* Output is different depending on the quiet flag */
	  if (!world.state.vm_quiet)
	    snprintf(buff, sizeof(buff),
		     " [%s] %s %s %s%s%s : %s %s => %s\n",
		     vm_colornumber("%03u", index), 
		     vm_colortypestr_fmt("%-15s", typeshort),
		     vm_coloraddress(XFMT, elfsh_get_reloffset(rel)),
		     vm_colorfieldstr("sym["),
		     vm_colornumber("%03u", elfsh_get_relsym(rel)),
		     vm_colorfieldstr("]"),
		     (name != NULL ? vm_colorstr_fmt("%-30s", name) : vm_colorwarn_fmt("%-30s", "<?>")), addstr, 
		     vm_colortypestr(type));
	  else
	    snprintf(buff, sizeof(buff),
		     " [%s] %s %s %s%s%s : %s %s\n",
		     vm_colornumber("%03u", index), 
		     vm_colortypestr_fmt("%-15s", typeshort),
		     vm_coloraddress(XFMT, elfsh_get_reloffset(rel)),
		     vm_colorfieldstr("sym["),
		     vm_colornumber("%03u", elfsh_get_relsym(rel)),
		     vm_colorfieldstr("]"),
		     (name != NULL ? vm_colorstr_fmt("%-22s", name) : vm_colorwarn_fmt("%-22s", "<?>")),
		     addstr);

	  /* Print it if it matchs the regex */
	  if (NULL == tmp || (tmp != NULL && name != NULL &&
			      NULL == regexec(tmp, buff, 0, 0, 0)))
	    vm_output(buff);
	  
	  vm_endline();
	}
      
       sect = elfsh_get_reloc(world.curjob->current, index2 + 1, &size);
       vm_output("\n");
    }

  vm_output("\n");
  ELFSH_PROFILE_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}
