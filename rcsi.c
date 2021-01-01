/*----------------------------------------------------------------------
 *
 *	rcsi.c	rcsi, a program to give information on RCS files.
 *
 *	Copyright 1994, 1995, 1996, 1997, 1999 by Colin Brough
 *
 *	rcsi is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published
 *	by the Free Software Foundation; either version 2, or (at your
 *	option) any later version.
 *
 *	rcsi is distributed in the hope that it will be useful, but
 *	WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with rcsi; see the file COPYING. If not, write to the Free
 *	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *	If you find any bugs or have any comments or suggestions, please
 *	send them to: 'Colin.Brough@blueyonder.co.uk'
 *
 *----------------------------------------------------------------------
 * $Id: rcsi.c,v 1.28 2003/04/19 22:54:57 cmb Exp $
 *
 * $Log: rcsi.c,v $
 * Revision 1.28  2003/04/19 22:54:57  cmb
 * Update e-mail address
 *
 * Revision 1.27  2002/08/01 21:07:01  cmb
 * Apparently successful update to recognise symbolic links.
 *
 * Revision 1.26  1999/12/28 17:07:57  cmb
 * Fixed a bug in the colour excluding code, so that no escape sequences
 * are generated when --no-color is selected.
 *
 * Revision 1.25  1999/12/28 17:02:04  cmb
 * Updated version string.
 *
 * Revision 1.24  1999/12/28 17:01:40  cmb
 * Updated so that you can turn colour on and off from the command line.
 *
 * Revision 1.23  1997/12/05 08:54:30  cmb
 * Updated to allow only files NOT under RCS to be printed.
 *
 * Revision 1.22  1996/11/06  10:00:40  cmb
 * Updated.
 *
 * Revision 1.21  1995/03/17  14:49:31  cmb
 * Removed 'RCS' directory from normal output.
 *
 * Revision 1.20  1995/02/28  17:21:56  cmb
 * Added colour as an experiment - we'll see how it goes in work-a-day
 * use...
 *
 * Revision 1.19  1994/11/16  13:38:47  cmb
 * Fixed version string.
 *
 * Revision 1.18  1994/11/16 13:38:00  cmb
 * Added debugging information; tracked down bug when long filenames were
 * around that screwed up the output.
 *
 * Revision 1.17  1994/11/09 16:53:50  cmb
 * Changes to allow files to be read from the command line completed, including
 * facility to note files given on command line that don't exist.
 *
 * Revision 1.16  1994/11/09 16:19:43  cmb
 * Added the ability to take command line arguments - the help message hasn't
 * been updated yet.... and there hasn't been much checking yet!
 *
 * Revision 1.15  1994/11/02 13:18:13  cmb
 * Sorted some duff logic in 'tree_insert' so that infinite loops don't occur
 * if it doesn't know how to modify the class for a file with an entry in
 * the tree already. Detects case where there is a ,v file both in the RCS
 * sub-directory and in the current directory.
 * Finallly, closes RCS and working files after use - stupid omission,.
 *
 * Revision 1.14  1994/11/02 10:21:37  cmb
 * Some minor portability changes. Now compiles and runs on Linux, SunOS 4.1.3,
 * Solaris 2.3, Irix 5.2 and OSF 1/3.
 *
 * Revision 1.13  1994/11/01 14:46:07  cmb
 * Removed some comments at end.
 * Also, in last revision message, omitted to note that also added was code
 * to take account of the command line flags to include or omit certain
 * classes of files from the output.
 *
 * Revision 1.12  1994/11/01  14:39:51  cmb
 * Added some contact information.
 * Added stuff to read the comment leader, if present, from the RCS file.
 *
 * Revision 1.11  1994/10/30  17:26:51  cmb
 * Check-pointing revision.
 *
 * Revision 1.10  1994/10/30  13:46:06  cmb
 * Cleaned up code a bit so now compiles cleanly through 'gcc -Wall'.
 *
 * Revision 1.9  1994/10/29  16:02:11  cmb
 * Looking very good! Still doesn't read comment leaders from the RCS file,
 * so fails when the default type isn't right... Haven't found any other
 * bugs yet.
 *
 * Revision 1.8  1994/10/28  17:17:59  cmb
 * Progress! It now prints a list of files in current and RCS directory,
 * apparently correctly identifying RCS files and working files.
 *
 * Revision 1.7  1994/10/28  00:38:11  cmb
 * Checkpointing check-in. Got the reading of a directory into a data-
 * structure partially working. Don't do anything with it yet, and it needs
 * some tuning.
 *
 * Revision 1.6  1994/10/05  17:38:04  cmb
 * Formatting changes to make sure everything fits within 80 characters.
 *
 * Revision 1.5  1994/09/21  21:06:12  cmb
 * Added temporary routine to print out what a file is if it ain't an RCS
 * file.
 *
 * Revision 1.4  1994/02/14  21:36:57  cmb
 * Made it look for RCS file in RCS sub-dir...
 *
 * Revision 1.3  1994/02/11  23:07:44  cmb
 * Added signal handler code, though it is probably not necessary.
 *
 * Revision 1.2  1994/02/11  22:10:39  cmb
 * As far as I can tell, give it a file and the RCS file and it
 * successfully tells you whether they are identical. No file handling
 * more complicated than one file at a time.
 *
 * Revision 1.1  1994/02/10  00:13:27  cmb
 * Initial revision
 *
 *----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#include <limits.h>

#include <sys/stat.h>
#include <unistd.h>

#include <dirent.h>

#ifndef PATH_MAX
#define PATH_MAX _POSIX_PATH_MAX
#endif

#ifndef FILENAME_MAX
#define FILENAME_MAX 255	/* It needs to be reasonably long - this is
				 * really just a working guess...
				 */
#endif

/*----------------------------------------------------------------------
 *	Debugging macro
 *
 *	Note that the use of the __FUNCTION__ variable and the varargs
 *	macro is a GCC specific feature and is non-portable to other
 *	compilers. Note also can't use in an unprotected 'if' stmt.
 *----------------------------------------------------------------------*/

#ifdef __GNUC__
#define report(args...)\
if (g_debug) \
{ \
	fprintf(g_debug_file,"%20s : %5d : ", __FUNCTION__, __LINE__);\
	fprintf(g_debug_file, ## args);\
}
#else
#define report if (g_debug) printf
#endif

/*----------------------------------------------------------------------
 *	CPP defines and global variables - this should one day migrate
 *	to a header file. Most of this is verbatim copied from various
 *	of the original RCS source.
 *----------------------------------------------------------------------*/

#define KDELIM	'$'	/* delimiter for keywords			*/
#define VDELIM	':'	/* separates keywords from values		*/
#define SDELIM	'@'	/* actual character is needed for string handling */
#define COMMENT_MAX 80	/* Maximum length of a comment leader string	*/

#define true	 1
#define false	 0
#define nil	 0

#define AUTHOR		"Author"
#define DATE		"Date"
#define HEADER		"Header"
#define IDH		"Id"
#define LOCKER		"Locker"
#define LOG		"Log"
#define RCSFILE		"RCSfile"
#define REVISION	"Revision"
#define SOURCE		"Source"
#define STATE		"State"

#define keylength 8	/* max length of any of the above keywords */

enum markers {	Nomatch, Author, Date, Header, Id,
		Locker, Log, RCSfile, Revision, Source, State };

char const *const Keyword[] =
{
	/* This must be in the same order as markers type above. */
	nil,
	AUTHOR, DATE, HEADER, IDH,
	LOCKER, LOG, RCSFILE, REVISION, SOURCE, STATE
};

char const ciklog[] = "checked in with -k by ";

enum tokens
{
	DELIM,	DIGIT,	IDCHAR,	NEWLN,	LETTER,	Letter,
	PERIOD, SBEGIN, SPACE,	UNKN,
	COLON,	ID,	NUM,	SEMI,	STRING
};

/*----------------------------------------------------------------------
 * Now for some of my own additional datatypes.
 *
 * The first one is the enumeration covering the 'types' of files rcsi
 * deals with:
 *	RCSI_dir	Directories
 *	RCSI_RCS	An RCS ,v file
 *	RCSI_nonRCS	A normal file, not necessarily under the control of RCS
 *	RCSI_work	RCS working file - i.e. file and ,v file both exist
 *	RCSI_lnk	A link
 *	RCSI_other	A socket, or something
 *	RCSI_fail	No file exists.
 *
 * The second one is the structure/tree into which filenames are colllected.
 *----------------------------------------------------------------------*/

typedef enum FileClass
{
	RCSI_dir, RCSI_RCS, RCSI_nonRCS, RCSI_work, RCSI_lnk, RCSI_other,
        RCSI_fail
} FileClass;

typedef struct Tnode
{
	char *name;
	int length;
	FileClass class;
	struct Tnode *lower, *higher;
} Tnode;

/*----------------------------------------------------------------------
 * comtable pairs each pathname suffix with a comment leader. The comment
 * leader is placed before each line generated by the $Log keyword. This
 * table is used to guess the proper comment leader from the working file's
 * suffix during initial ci (see InitAdmin()). Comment leaders are needed
 * for languages without multiline comments; for others they are optional.
 *----------------------------------------------------------------------*/

struct compair {
	char const *suffix, *comlead;
};

static struct compair const comtable[] = {
	"a",   "-- ",   /* Ada         */
	"ada", "-- ",
	"asm", ";; ",	/* assembler (MS-DOS) */
	"bat", ":: ",	/* batch (MS-DOS) */
	"c",   " * ",   /* C           */
	"c++", "// ",	/* C++ in all its infinite guises */
	"cc",  "// ",
	"cpp", "// ",
	"cxx", "// ",
	"cl",  ";;; ",  /* Common Lisp */
	"cmd", ":: ",	/* command (OS/2) */
	"cmf", "c ",	/* CM Fortran  */
	"cs",  " * ",	/* C*          */
	"el",  "; ",    /* Emacs Lisp  */
	"f",   "c ",    /* Fortran     */
	"for", "c ",
	"h",   " * ",   /* C-header    */
	"hpp", "// ",	/* C++ header  */
	"hxx", "// ",
	"l",   " * ",   /* lex      NOTE: conflict between lex and franzlisp */
	"lisp",";;; ",	/* Lucid Lisp  */
	"lsp", ";; ",	/* Microsoft Lisp */
	"mac", ";; ",	/* macro (DEC-10, MS-DOS, PDP-11, VMS, etc) */
	"me",  ".\\\" ",/* me-macros   t/nroff*/
	"ml",  "; ",    /* mocklisp    */
	"mm",  ".\\\" ",/* mm-macros   t/nroff*/
	"ms",  ".\\\" ",/* ms-macros   t/nroff*/
	"p",   " * ",   /* Pascal      */
	"pas", " * ",
	"ps",  "% ",	/* PostScript  */
	"sty", "% ",	/* LaTeX style */
	"tex", "% ",	/* TeX	       */
	"y",   " * ",   /* yacc        */
	nil,   "# "     /* default for unknown suffix; must always be last */
};

/*----------------------------------------------------------------------
 * map of character types; ISO 8859/1 (Latin-1)
 *----------------------------------------------------------------------*/

enum tokens const ctab[] = {
	UNKN,	UNKN,	UNKN,	UNKN,	UNKN,	UNKN,	UNKN,	UNKN,
	SPACE,	SPACE,	NEWLN,	SPACE,	SPACE,	SPACE,	UNKN,	UNKN,
	UNKN,	UNKN,	UNKN,	UNKN,	UNKN,	UNKN,	UNKN,	UNKN,
	UNKN,	UNKN,	UNKN,	UNKN,	UNKN,	UNKN,	UNKN,	UNKN,
	SPACE,	IDCHAR,	IDCHAR,	IDCHAR,	DELIM,	IDCHAR,	IDCHAR,	IDCHAR,
	IDCHAR,	IDCHAR,	IDCHAR,	IDCHAR,	DELIM,	IDCHAR,	PERIOD,	IDCHAR,
	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,
	DIGIT,	DIGIT,	COLON,	SEMI,	IDCHAR,	IDCHAR,	IDCHAR,	IDCHAR,
	SBEGIN,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,
	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,
	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,
	LETTER,	LETTER,	LETTER,	IDCHAR,	IDCHAR,	IDCHAR,	IDCHAR,	IDCHAR,
	IDCHAR,	Letter,	Letter,	Letter,	Letter,	Letter,	Letter,	Letter,
	Letter,	Letter,	Letter,	Letter,	Letter,	Letter,	Letter,	Letter,
	Letter,	Letter,	Letter,	Letter,	Letter,	Letter,	Letter,	Letter,
	Letter,	Letter,	Letter,	IDCHAR,	IDCHAR,	IDCHAR,	IDCHAR,	UNKN,
	UNKN,	UNKN,	UNKN,	UNKN,	UNKN,	UNKN,	UNKN,	UNKN,
	UNKN,	UNKN,	UNKN,	UNKN,	UNKN,	UNKN,	UNKN,	UNKN,
	UNKN,	UNKN,	UNKN,	UNKN,	UNKN,	UNKN,	UNKN,	UNKN,
	UNKN,	UNKN,	UNKN,	UNKN,	UNKN,	UNKN,	UNKN,	UNKN,
	IDCHAR,	IDCHAR,	IDCHAR,	IDCHAR,	IDCHAR,	IDCHAR,	IDCHAR,	IDCHAR,
	IDCHAR,	IDCHAR,	IDCHAR,	IDCHAR,	IDCHAR,	IDCHAR,	IDCHAR,	IDCHAR,
	IDCHAR,	IDCHAR,	IDCHAR,	IDCHAR,	IDCHAR,	IDCHAR,	IDCHAR,	IDCHAR,
	IDCHAR,	IDCHAR,	IDCHAR,	IDCHAR,	IDCHAR,	IDCHAR,	IDCHAR,	IDCHAR,
	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,
	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,
	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	IDCHAR,
	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	Letter,
	Letter,	Letter,	Letter,	Letter,	Letter,	Letter,	Letter,	Letter,
	Letter,	Letter,	Letter,	Letter,	Letter,	Letter,	Letter,	Letter,
	Letter,	Letter,	Letter,	Letter,	Letter,	Letter,	Letter,	IDCHAR,
	Letter,	Letter,	Letter,	Letter,	Letter,	Letter,	Letter,	Letter
};

/*----------------------------------------------------------------------
 *	Newly introduce global variables. These replace dependencies in
 *	the original code on the delta structure. Since 'rcsi' doesn't
 *	need all that complexity of information, it doesn't bother to
 *	collect it.
 *----------------------------------------------------------------------*/

char	* Comment;
char	* LogMessage;
int	  LogSize = 100;	/* Same scheme as LockSize below...	*/
char	* Locks;
int	  LockSize = 100;	/* Reasonable figure - if more space is
				 * needed, it will be allocated dynamically
				 */
Tnode	* tree_root = NULL;	/* Root of the global tree of filenames	*/
int	  tree_longest = 0;	/* Longest filename in the tree		*/

				/* Some Booleans relating to the command
				 * line flags.
				 */
int	g_changed = false,	/* true => display only changed RCS files */
	g_rcsonly = false,	/* true => display only RCS related files */
	g_not_rcs = false,	/* true => display only non-RCS related files*/
	g_locked = false,	/* true => display only locked RCS files */
	g_unlocked = false,	/* true => display only unlocked RCS files */
	g_dot = false,		/* true => also include dot files	*/
	g_fileargs = false,	/* Are there filename arguments?	*/
	g_debug = false,	/* Do we want debugging information?	*/
	g_colour = true,	/* Do we want colour output?		*/
        g_RCSdir = true;	/* Normally an RCS dir is present...	*/

FILE	*g_debug_file;		/* File to which debugging information is
				 * dumped.
				 */

char	g_spaces[FILENAME_MAX];	/* Array of spaces... for printing	*/

				/* Version string - first bit is the main
				 * version number, last bit is generated
				 * by RCS.
				 */
char	g_version[] = "0.5  ($Id: rcsi.c,v 1.28 2003/04/19 22:54:57 cmb Exp $)";

/*----------------------------------------------------------------------
 *
 *	Function prototypes
 *
 *----------------------------------------------------------------------*/

int		is_rcs_filename (char *fname);
Tnode	       *node_insert (char *fname, int length, FileClass class);
void		tree_insert (char *fname, FileClass class);
int		process_full_case (Tnode *t);
void		process_rcs_case (Tnode *t);
void		p_name(Tnode *t);
void		p_node (Tnode *t);
void		print_tree (void);
FileClass	determine_class (char *fname);
void		read_files (void);
void		process_filearg (char *fname);
int		look_for (char *target, FILE *file_p);
void		skip_to (char *keyword, FILE *rcsfile_p);
enum markers	trymatch (char const *string);
static int	suffix_matches (register char const *suffix,
				register char const *pattern);
static char const *bindex (register char const *sp, register int c);
static int	discardkeyval (register int c, register FILE *f, int eofchar);
int 		match_log (FILE *file_p, int file_c);
int 		rcsfcmp (register FILE *file_p, register FILE *rcsfile_p);
void		parse_rcs_file (FILE *rcsfile_p, char *file_name);
void		sig_handle (int sig);

void	red(void);
void	bold_red(void);
void	green(void);
void	bold_green(void);
void	yellow(void);
void	blue(void);
void	bold_blue(void);
void	magenta(void);
void	cyan(void);
void	normal(void);

/*----------------------------------------------------------------------*/

void normal(void)
{
	if (! g_colour) return;
	fputc(27,  stdout);	/* This stuff resets foreground and	*/
	fputc('[', stdout);	/* background to black and white	*/
	fputc('0', stdout);	/* respectively - for use in xterms	*/
	fputc('0', stdout);	/* and on the console really....	*/
	fputc('m', stdout);
}

/*----------------------------------------------------------------------*/

void red(void)
{
	if (! g_colour) return;
	fputc(27,  stdout);
	fputc('[', stdout);
	fputc('0', stdout);
	fputc(';', stdout);
	fputc('3', stdout);
	fputc('1', stdout);
	fputc('m', stdout);
}

/*----------------------------------------------------------------------*/

void bold_red(void)
{
	if (! g_colour) return;
	fputc(27,  stdout);
	fputc('[', stdout);
	fputc('3', stdout);
	fputc('1', stdout);
	fputc(';', stdout);
	fputc('1', stdout);
	fputc('m', stdout);
}

/*----------------------------------------------------------------------*/

void green(void)
{
	if (! g_colour) return;
	fputc(27,  stdout);
	fputc('[', stdout);
	fputc('0', stdout);
	fputc(';', stdout);
	fputc('3', stdout);
	fputc('2', stdout);
	fputc('m', stdout);
}

/*----------------------------------------------------------------------*/

void bold_green(void)
{
	if (! g_colour) return;
	fputc(27,  stdout);
	fputc('[', stdout);
	fputc('3', stdout);
	fputc('2', stdout);
	fputc(';', stdout);
	fputc('1', stdout);
	fputc('m', stdout);
}

/*----------------------------------------------------------------------*/

void yellow(void)
{
	if (! g_colour) return;
	fputc(27,  stdout);
	fputc('[', stdout);
	fputc('0', stdout);
	fputc(';', stdout);
	fputc('3', stdout);
	fputc('3', stdout);
	fputc('m', stdout);
}

/*----------------------------------------------------------------------*/

void blue(void)
{
	if (! g_colour) return;
	fputc(27,  stdout);
	fputc('[', stdout);
	fputc('0', stdout);
	fputc(';', stdout);
	fputc('3', stdout);
	fputc('4', stdout);
	fputc('m', stdout);
}

/*----------------------------------------------------------------------*/

void bold_blue(void)
{
	if (! g_colour) return;
	fputc(27,  stdout);
	fputc('[', stdout);
	fputc('3', stdout);
	fputc('4', stdout);
	fputc(';', stdout);
	fputc('1', stdout);
	fputc('m', stdout);
}

/*----------------------------------------------------------------------*/

void magenta(void)
{
	if (! g_colour) return;
	fputc(27,  stdout);
	fputc('[', stdout);
	fputc('0', stdout);
	fputc(';', stdout);
	fputc('3', stdout);
	fputc('5', stdout);
	fputc('m', stdout);
}

/*----------------------------------------------------------------------*/

void cyan(void)
{
	if (! g_colour) return;
	fputc(27,  stdout);
	fputc('[', stdout);
	fputc('0', stdout);
	fputc(';', stdout);
	fputc('3', stdout);
	fputc('6', stdout);
	fputc('m', stdout);
}

/*----------------------------------------------------------------------
 * is_rcs_filename	Is the filename passed in an RCS filename?
 *			ASSUMES that RCS files all end in ',v'...
 *----------------------------------------------------------------------*/

int is_rcs_filename(char *fname)
{
	int l;

	if (fname == NULL) return(false);	/* No filename		*/
	l = strlen(fname);			/* If length < 3 then	*/
	if (l < 3) return(false);		/* can't have ',v' exten*/
	fname += (l - 2);			/* Look at end of fname	*/
	if (strcmp(fname, ",v") == 0) return(true);

	return(false);
}

/*----------------------------------------------------------------------
 * node_insert	This routine mallocs space for a new node in the tree,
 *		fills in the basic fields, and returns a pointer to the
 *		structure.
 *----------------------------------------------------------------------*/

Tnode *node_insert(char *fname, int length, FileClass class)
{
	Tnode *temp;
	temp = (Tnode *) malloc(sizeof(Tnode));
	if (temp == NULL)
	{
		printf("malloc failed in node_insert\n");
		exit(-1);
	}

	report("Inserting %s into tree, class %d, length %d\n", fname, class,
	       length);

	/* Store filename length. Keep track of the longest filename.	*/

	temp->name	= fname;
	temp->class	= class;
	temp->lower	= NULL;
	temp->higher	= NULL;
	temp->length	= length;
	if (length > tree_longest)
	{
		tree_longest = length;
	}
	return(temp);
}

/*----------------------------------------------------------------------
 * tree_insert	This routine inserts a filename of a specified class
 *		into the global tree. ASSUME that any ',v' has been
 *		stripped off and encoded in the class.
 *----------------------------------------------------------------------*/

void tree_insert(char *fname, FileClass class)
{
	Tnode *current = tree_root;
	int comp, len;
	char *tmp;

	len = strlen(fname);
	tmp = (char *) malloc(len+1);
	strcpy(tmp, fname);		/* Copy the filename to make	*/
	fname = tmp;			/* sure it actually appears in	*/
					/* the tree...			*/
	if (tree_root == NULL)
	{
		tree_root = node_insert(fname, len, class);
		return;
	}

	while (true)
	{
		comp = strcmp(current->name, fname);
		if (comp == 0)
		{
			/* There is already a file of this name in the
			 * data-structure. Modify the class field as
			 * appropriate.
			 */

			if (((class == RCSI_RCS) &&
			     (current->class == RCSI_nonRCS))
			    ||
			    ((class == RCSI_nonRCS) &&
			     (current->class == RCSI_RCS))
			   )
			{
				current->class = RCSI_work;
				return;
			}

			if ((class == RCSI_RCS) &&
			    (current->class == RCSI_work))
			{
				printf("Probably two RCS files!!! Ignoring "
				       "second one for %s\n", fname);
				return;
			}

			printf("Something funny going on!\n");
			exit(-1);
		}
		else if (comp > 0)	/* fname < current->name	*/
		{
			if (current->lower == NULL)
			{
				current->lower = node_insert(fname,len,class);
				return;
			}
			current = current->lower;
		}
		else	/* comp < 0, i.e. fname > current->name		*/
		{
			if (current->higher == NULL)
			{
				current->higher = node_insert(fname,len,class);
				return;
			}
			current = current->higher;
		}
	} /* This brace the end of the 'while (true)' loop	*/
}

/*----------------------------------------------------------------------
 * process_full_case	This routine is called by p_node when there is
 *			both an RCS file and a working file - i.e. the
 *			full-blown case where a file comparison has to
 *			be done.
 *----------------------------------------------------------------------*/

int process_full_case(Tnode *t)
{
	int result;
	char RCSname[PATH_MAX];
	FILE *work_p, *rcs_p;

	/*----------------------------------------------------------------
	 * Try and open the RCS file - first in the RCS directory, failing
	 * which in the current directory.
	 *----------------------------------------------------------------*/
	sprintf(RCSname, "RCS/%s,v", t->name);
	if ((rcs_p = fopen(RCSname, "r")) == NULL)
	{
		sprintf(RCSname, "%s,v", t->name);
		if ((rcs_p = fopen(RCSname, "r")) == NULL)
		{
			printf("Failed to open RCS file\n");
			exit(-1);
		}
	}

	/*----------------------------------------------------------------
	 * Next have a go at the working file.
	 *----------------------------------------------------------------*/
	if ((work_p = fopen(t->name, "r")) == NULL)
	{
		printf("Failed to open working file\n");
		exit(-1);
	}

	/*----------------------------------------------------------------
	 * Now onto the meat of the processing
	 *----------------------------------------------------------------*/

	parse_rcs_file(rcs_p, t->name);

	result = rcsfcmp(work_p, rcs_p);

	if (fclose(work_p) == EOF)
	{
		printf("Closing work file failed\n");
		exit(-1);
	}
	if (fclose(rcs_p) == EOF)
	{
		printf("Closing RCS file failed\n");
		exit(-1);
	}

	return(result);
}

/*----------------------------------------------------------------------
 * process_rcs_case	This routine is called by p_node when there is
 *			only an RCS file.
 *----------------------------------------------------------------------*/

void process_rcs_case(Tnode *t)
{
	char RCSname[PATH_MAX];
	FILE *rcs_p;

	/*----------------------------------------------------------------
	 * Try and open the RCS file - first in the RCS directory, failing
	 * which in the current directory.
	 *----------------------------------------------------------------*/
	sprintf(RCSname, "RCS/%s,v", t->name);
	if ((rcs_p = fopen(RCSname, "r")) == NULL)
	{
		sprintf(RCSname, "%s,v", t->name);
		if ((rcs_p = fopen(RCSname, "r")) == NULL)
		{
			printf("Failed to open RCS file\n");
			exit(-1);
		}
	}

	/*----------------------------------------------------------------
	 * Now onto the meat of the processing
	 *----------------------------------------------------------------*/

	parse_rcs_file(rcs_p, t->name);

	if (fclose(rcs_p) == EOF)
	{
		printf("Closing RCS file failed\n");
		exit(-1);
	}
}

/*----------------------------------------------------------------------
 * p_name	Routine to print a filename and pad to the right with
 *		the appropriate number of spaces.
 *		Only encapsulated in a single routine so it can be more
 *		neatly called from within the 'switch' statement in
 *		'p_node' below so that only relevant information is
 *		printed.
 *----------------------------------------------------------------------*/

void p_name(Tnode *t)
{
	g_spaces[tree_longest - t->length] = '\0';
	printf("%s%s ", t->name, g_spaces);
	g_spaces[tree_longest - t->length] = ' ';
}

/*----------------------------------------------------------------------
 * p_node	This routine is the recursive bit that really prints out
 *		the contents of the tree.
 *----------------------------------------------------------------------*/

void p_node(Tnode *t)
{
	int result, len;

	if (t == NULL) return;
	p_node(t->lower);

	report("Printing info on file %s\n", t->name);

	/* To get the spacing right we have a string of spaces. Once we know
	 * the length of the longest filename and the current one, we write
	 * an end of string character into the string of spaces to make up
	 * the difference, and then print the current filename plus the
	 * (now shortened) string of spaces. Finally, restore the string of
	 * spaces to its original state....
	 */

	normal();

	switch (t->class)
	{
	case RCSI_dir:
		if ( ! ( g_changed || g_rcsonly || g_locked || g_unlocked ))
		{
			p_name(t);
			bold_blue();
			printf("( Directory )\n");
		}
		break;

	case RCSI_nonRCS:
		if ( ! ( g_changed || g_rcsonly || g_locked || g_unlocked ))
		{
			p_name(t);
			printf("( Not under RCS )\n");
		}
		break;

        case RCSI_lnk:
                if ( ! ( g_changed || g_rcsonly || g_locked || g_unlocked ))
		{
			p_name(t);
                        magenta();
			printf("( Symbolic link )\n");
		}
		break;
                
	case RCSI_RCS:
				/* Faster to do this here => RCS file
				 * opened or parsed..., or not interested in
				 * RCS-related files.
				 */
		if ((g_changed) || (g_not_rcs))	
		{
			break;
		}

		process_rcs_case(t);
		len = strlen(Locks);

		/*--------------------------------------------------------
		 * Skip the printing if we don't want output for this case:
		 * - only interested in locked files and it isn't locked;
		 * - only interested in unlocked files and it is locked.
		 *--------------------------------------------------------*/
		if (((len == 0) && (g_locked))	||
		    ((len != 0) && (g_unlocked)))
		{
			break;
		}
		/*--------------------------------------------------------
		 * Now actually print out the relevant information
		 *--------------------------------------------------------*/
		p_name(t);
		if (len == 0)
		{
			green();
			printf("( in,  unchanged, unlocked )\n");
		}
		else
		{
			bold_green();
			printf("( in,  unchanged, %s )\n", Locks);
			free(Locks);
			Locks = nil;
		}
		break;

	case RCSI_work:
		if (g_not_rcs)	/* Not interested in RCS related files	*/
		{
			break;
		}
		
		result = process_full_case(t);
		len = strlen(Locks);

		/*--------------------------------------------------------
		 * Skip the printing if we don't want output for this case:
		 * - only interested in changed files and it hasn't changed;
		 * - only interested in locked files and it isn't locked;
		 * - only interested in unlocked files and it is locked.
		 *--------------------------------------------------------*/
		if ((g_changed && (result != 1)) ||
		    ((len == 0) && (g_locked)) ||
		    ((len != 0) && (g_unlocked)))
		{
			break;
		}

		/*--------------------------------------------------------
		 * Now actually print out the relevant information
		 *--------------------------------------------------------*/
		if (result == 1)
		{
			p_name(t);
			if (len == 0)
			{
				red();
				printf("( out, *revised*, unlocked )\n");
			}
			else
			{
				bold_red();
				printf("( out, *revised*, %s )\n", Locks);
				free(Locks);
				Locks = nil;
			}
		}
		else
		{
			p_name(t);
			if (len == 0)
			{
				green();
				printf("( out, unchanged, unlocked )\n");
			}
			else
			{
				bold_green();
				printf("( out, unchanged, %s )\n", Locks);
				free(Locks);
				Locks = nil;
			}
		}
		break;

	case RCSI_other:
		if ( ! ( g_changed || g_rcsonly || g_locked || g_unlocked ))
		{
			p_name(t);
			printf("( Link/socket/etc. )\n");
		}
		break;

	case RCSI_fail:
		if ( ! ( g_changed || g_rcsonly || g_locked || g_unlocked ))
		{
			p_name(t);
			cyan();
			printf("( ** non-existent ** )\n");
		}
		break;
	default:
		if ( ! ( g_changed || g_rcsonly || g_locked || g_unlocked ))
		{
			p_name(t);
			cyan();
			printf("( ** unknown ** )\n");
		}
		break;
	}

	p_node(t->higher);
}

/*----------------------------------------------------------------------
 * print_tree	This routine prints out a textual representation of the
 *		tree
 *----------------------------------------------------------------------*/

void print_tree(void)
{
	int i;

	report("Entering 'print_tree'. Longest filename is %d\n",
	       tree_longest);

	/* Make sure the spaces array is initialised. This could be
	 * more sensibly done just once in the program run, at startup.
	 */
	for (i = 0; i < (FILENAME_MAX-1); i++)
	{
		g_spaces[i] = ' ';
	}
	g_spaces[FILENAME_MAX -1] = '\0';

	p_node(tree_root);
	report("Leaving 'print_tree'.\n");
}

/*----------------------------------------------------------------------
 * determine_class	Routine to determine the class of a file by
 *			stat'ing the filename.
 *----------------------------------------------------------------------*/

FileClass determine_class(char *fname)
{
	FileClass class;
	struct stat buf;
	if (lstat(fname, &buf) == -1)
	{
		class = RCSI_fail;
	}
	else
	{
		if (S_ISREG(buf.st_mode))
		{
			/* Its a regular file */
			if (is_rcs_filename(fname))
			{
				class = RCSI_RCS;
			}
			else
			{
				class = RCSI_nonRCS;
			}
		}
		else if (S_ISDIR(buf.st_mode))
		{
			/* Its a directory */
			class = RCSI_dir;
		}
                else if (S_ISLNK(buf.st_mode))
                {
                        /* Its a symbolic link */
                        class = RCSI_lnk;
                }
		else if ((S_ISCHR(buf.st_mode))  ||
			 (S_ISBLK(buf.st_mode))  ||
			 (S_ISFIFO(buf.st_mode)) ||
			 (S_ISSOCK(buf.st_mode)))
		{
			/* Its a scoket */
			class = RCSI_other;
		}
		else
		{
			/* Dear only knows what it is! */
			printf("Unknown file type!\n");
			exit(-1);
		}
	}

	report("'determine_class' returns %d\n", class);
	return(class);
}

/*------------------------------------------------------------------------
 * process_filearg	Routine to process a file argument to rcsi and
 *			insert it into the tree data structure.
 *------------------------------------------------------------------------*/

void process_filearg(char *fname)
{
	FileClass class;
	struct stat buf;
	char *rcsfname;

	if ((rcsfname = (char *) malloc (strlen(fname) + 7)) == 0)
	{
		printf("malloc for constructing RCS file name failed\n");
		exit(-1);
	}

	g_fileargs = true;	/* We have got a filename argument	*/

	class = determine_class(fname);
	if (class == RCSI_RCS)
	{
		fname[strlen(fname)-2] = '\0';
	}
	else if (class == RCSI_nonRCS)
	{
		if (g_RCSdir)
		{
			sprintf(rcsfname, "RCS/%s,v", fname);
			if (stat(rcsfname, &buf) == 0)
			{
				if (S_ISREG(buf.st_mode))
				{
					class = RCSI_work;
				}
			}
		}
		if (class == RCSI_nonRCS)
		{
			sprintf(rcsfname, "%s,v", fname);
			if (stat(rcsfname, &buf) == 0)
			{
				if (S_ISREG(buf.st_mode))
				{
					class = RCSI_work;
				}
			}
		}
	}
	tree_insert(fname, class);
}

/*----------------------------------------------------------------------
 * read_files		Development only routine to read in the files in
 *			the current directory and print out their names
 *----------------------------------------------------------------------*/

void read_files(void)
{
	DIR *dir;
	struct dirent *dire;
	FileClass class;
	char pathname[PATH_MAX];

	report("Reading files from current directory\n");

	if ((dir = opendir(".")) == NULL)
	{
		printf("Failed to 'opendir' current directory\n");
		exit(-1);
	}

	while (true)
	{
		if ((dire = readdir(dir)) == NULL)
		{
			break;
		}
		/* If the first character of the filename is '.' then
		 * ignore it, as long as g_dot isn't true... Otherwise
		 * insert into the tree.
		 */
		if (((char) *(dire->d_name) != '.') || (g_dot))
		{
			if ((strcmp(dire->d_name, "RCS") == 0) && (!g_dot))
			{
				continue;
			}
			class = determine_class(dire->d_name);
			if (class == RCSI_RCS)
			{
				dire->d_name[strlen(dire->d_name)-2] = '\0';
			}
			tree_insert(dire->d_name, class);
		}
	}

	/*--------------------------------------------------------------
	 * Having done the current directory, do the same again for the
	 * RCS directory (if it exists). First store the current working
	 * directory  - so we are not relying on 'chdir("..")' to get us
	 * back here...
	 *--------------------------------------------------------------*/

	if (g_RCSdir)			/* If the RCS directory exists	*/
	{
		if ((getcwd(pathname,PATH_MAX)) == NULL)
		{
			printf("getcwd failed: %s\n", pathname);
			exit(-1);
		}
		if (chdir("RCS") != 0)
		{
			printf("RCS directory exists, but can't cd into it\n");
			exit(-1);
		}

		if ((dir = opendir(".")) == NULL)
		{
			printf("Failed to open RCS sub-dir\n");
			exit(-1);
		}
		while (true)
		{
			if ((dire = readdir(dir)) == NULL)
			{
				break;
			}
			/* If the first character of the filename is '.' then
			 * ignore it, as long as g_dot isn't true... Otherwise
			 * insert into the tree.
			 */
			if (((char) *(dire->d_name) != '.') || (g_dot))
			{
				if (is_rcs_filename(dire->d_name))
				{
					dire->d_name[strlen(dire->d_name)-2]
						= '\0';
					tree_insert(dire->d_name, RCSI_RCS);
				}
				/* N.B. Non-RCS files lying around in the
				 * RCS directory are ignored...
				 */
			}
		}
		if (chdir(pathname) == -1)
		{
			printf("Failed to return to current dir\n");
			exit(-1);
		}
	}
}

/*----------------------------------------------------------------------
 * look_for	Routine to look for a string in the RCS file, failing if
 *		a '@' is found. The current file position is stored and
 *		restored if the relevant search fails. If the search
 *		succeeds, the file pointer is left just after the last
 *		character of the search string. Intended to be used to
 *		search for the [optional] comment leader string.
 *----------------------------------------------------------------------*/

int look_for(char *target, FILE *file_p)
{
	long file_pos;
	int length, index = 0;
	char ch;

	/*----------------------------------------------------------------
	 * Save current file position
	 *----------------------------------------------------------------*/

	if ((file_pos = ftell(file_p)) == -1L)
	{
		printf("Error  finding current file position\n");
		exit(-1);
	}

	/*----------------------------------------------------------------
	 * Look for the 'target' string.
	 *----------------------------------------------------------------*/

	length = strlen(target);
	for (;;)
	{
		ch = fgetc(file_p);
		if (ch == target[index])
		{
			index++;
			if (index == length)
			{
				/* Found the 'target' string; skip to the
				 * next '@' symbol.
				 */
				while ((ch != SDELIM) || (ch == EOF))
				{
					ch = fgetc(file_p);
				}
				return(true);
			}
		}
		else
		{
			index = 0;
		}
		if ((ch == SDELIM) || (ch == EOF))
		{
			/*----------------------------------------------
			 * We haven't found the string, restore the file
			 * position to what it was at the start
			 *------------------------------------------------*/

			if (fseek(file_p, file_pos, SEEK_SET) != 0)
			{
				printf("fseek failed\n");
				exit(-1);
			}
			return(false);
		}
	}
}

/*----------------------------------------------------------------------
 * skip_to	Routine to skip to a keyword in the RCS file, jumping
 *		past any @....@ strings that lie between the current
 *		position and where you want to be. Leaves the file
 *		pointer at the first character after the '@' after the
 *		keyword, since will be used to skip past 'log' and 'text'
 *		keywords.
 *----------------------------------------------------------------------*/

void skip_to(char *keyword, FILE *rcsfile_p)
{
	int length, index = 0;
	char ch;

	length = strlen(keyword);
	for (;;)
	{
		ch = fgetc(rcsfile_p);
		if (ch == keyword[index])
		{
			index++;
			if (index == length)
			{
				/* Found the keyword - skip to the next
				 * @ symbol
				 */
				while (ch != SDELIM)
				{
					ch = fgetc(rcsfile_p);
				}
				return;
			}
		}
		else
		{
			index = 0;
		}

		if (ch == SDELIM)
		{
			for (;;)
			{
				ch = fgetc(rcsfile_p);
				if (ch == SDELIM)
				{
					ch = fgetc(rcsfile_p);
					if (ch != SDELIM)
					{
						ungetc(ch, rcsfile_p);
						break;
					}
				}
			}
		}
	}
}


/*----------------------------------------------------------------------
 * trymatch:	Checks whether string starts with a keyword followed
 *		by a KDELIM or a VDELIM.
 *		If successful, returns the appropriate marker, otherwise
 *		Nomatch.
 *		Copied from original RCS sources, and reformatted.
 *----------------------------------------------------------------------*/

enum markers trymatch(char const *string)
{
	register int j;
	register char const *p, *s;

	for (j = sizeof(Keyword)/sizeof(*Keyword); (--j); )
	{
		/* try next keyword */
		p = Keyword[j];
		s = string;
		while (*p++ == *s++)
		{
			if (!*p)
			{
				switch (*s)
				{
				case KDELIM:
				case VDELIM:
					return (enum markers)j;
				default:
					return Nomatch;
				}
			}
		}
	}
	return(Nomatch);
}

/*----------------------------------------------------------------------
 *	suffix_matches	Utility routine from RCS sources.
 *----------------------------------------------------------------------*/

static int suffix_matches(register char const *suffix,
			  register char const *pattern)
{
        register int c;
        if (!pattern)
	{
		return true;
	}

        for (;;)
	{
                switch (*suffix++ - (c = *pattern++))
		{
		case 0:
                        if (!c)
			{
				return true;
			}
			break;

		case 'A'-'a':
                        if (ctab[c] == Letter)
			{
                                break;
			}
                        /* fall into */
		default:
                        return false;
                }
	}
}




/*----------------------------------------------------------------------
 * bindex:	Finds the last occurrence of character c in string sp
 *		and returns a pointer to the character just beyond it.
 *		If the character doesn't occur in the string, sp is
 *		returned.
 *		Copied from RCS sources.
 *----------------------------------------------------------------------*/

static char const *bindex(register char const *sp, register int c)
{
        register char const *r;
        r = sp;
        while (*sp)
	{
                if (*sp++ == c)
		{
			r = sp;
		}
	}
        return(r);
}

/*----------------------------------------------------------------------
 *	Skips through the rest of an expanded keyword that differs in
 *	the RCS and 'real' file versions. This is based on a routine
 *	from the original RCS sources, but has been modified to cope
 *	with RCS files that 'end' with SDELIM. The extra parameter
 *	indicates whether the file is to be treated as an RCS file or as
 *	a normal file.
 *----------------------------------------------------------------------*/

static int discardkeyval(register int c, register FILE *f, int eofchar)
{
	for (;;)
	{
		switch (c) {
		case KDELIM:
		case '\n':
			return c;
		default:
			if (eofchar == EOF)		/* Normal file	*/
			{
				if ((c = fgetc(f)) == EOF)
				{
					return(EOF);
				}
			}
			else if (eofchar == SDELIM)	/* RCS file	*/
			{
				if ((c = fgetc(f)) == SDELIM)
				{
					int tmp = fgetc(f);
					if (tmp != SDELIM)
					{
						return(SDELIM);
					}
				}
			}
			else
			{
				printf("Illegal End of File character passed"
				       " to 'discardkeyval' (%x)\n", eofchar);
				exit(-1);
			}
			break;
		}
	}
}

/*----------------------------------------------------------------------
 * match_log	Function to check that the log message from the RCS file
 *		matches what appears in the real file.
 *		Returns true if they match, and false otherwise.
 *----------------------------------------------------------------------*/

int match_log(FILE *file_p, int file_c)
{
	int LogLength, CommentLength, i, j;

	CommentLength	= strlen(Comment);
	LogLength	= strlen(LogMessage);

	/*--------------------------------------------------------------
	 * First Check if it was a log entry created with 'ci -k'. If so
	 * then nothing will have been added to the real file, so we can
	 * just skip this bit. The 'ungetc' does some pointer adjusting
	 * to leave the file-pointer aiming in the right place.
	 *--------------------------------------------------------------*/

	if ( ! ((LogLength < sizeof(ciklog)-1) ||
		(memcmp(LogMessage, ciklog, sizeof(ciklog)-1))) )
	{
		ungetc(file_c, file_p);
		return(true);
	}

	/*--------------------------------------------------------------
	 * Log always leaves a newline after the trailing KDELIM...
	 *--------------------------------------------------------------*/
	if (file_c != '\n')
	{
		printf("I suspect that the ci operation malformed the "
		       "Log...\n");
		exit(-1);
	}

	/*--------------------------------------------------------------
	 * Skip past the first line of the Log Message, just by looking
	 * for a '\n'.
	 * (N.B. This works in most cases but fails when the log message
	 *  itself has been manually edited. Should really match the
	 *  contents of this line with that extracted from the RCS file.)
	 *--------------------------------------------------------------*/

	for (;;)
	{
		if ((file_c = fgetc(file_p)) == '\n')
		{
			break;
		}
		else if (file_c == EOF)
		{
			return(false);
		}
	}

	/*--------------------------------------------------------------
	 * Next get past the leading comment leader. Should always be
	 * the full leader - spaces won't be dropped off the end of the
	 * line, since there should always be something on this line
	 * (even when you enter a log with a blank first line...
	 *--------------------------------------------------------------*/

	for (i = 0; i < CommentLength; i++)
	{
		if ((file_c = fgetc(file_p)) != Comment[i])
		{
			return(false);
		}
	}

	/*--------------------------------------------------------------
	 * Spin through the LogMessage, matching it character by
	 * character. Every time we get a '\n' insert a comment leader
	 * in the matching process. When we do this, make sure we can
	 * cope with spaces being dropped off the end of comment leader
	 * lines that don't have anything else on them (doesn't just
	 * apply to the last of the comment leaders).
	 *--------------------------------------------------------------*/

	for (i = 0; i < LogLength; i++)
	{
		if ((file_c = fgetc(file_p)) != LogMessage[i])
		{
			return(false);
		}

		if (LogMessage[i] == '\n')
		{
			for (j = 0; j < CommentLength; j++)
			{
				if ((file_c = fgetc(file_p)) != Comment[j])
				{
					if (isspace(Comment[j]) &&
					    (file_c == '\n'))
					{
						ungetc(file_c, file_p);
					}
					else
					{
						return(false);
					}
				}
			}
		}
	}

	return(true);
}

/*----------------------------------------------------------------------
 *	rcsfcmp		Core of rcsi - compares two files, one the RCS
 *			file and the other the corresponding 'normal'
 *			file. Based on a routine from the original RCS
 *			sources, though extensively modified. Variable
 *			names have been made more meaningful, dependencies
 *			on the delta structure removed, and the double @@
 *			way or storing '@' symbols in the RCS file is dealt
 *			with. Comments below are from the original sources:
 *
 * Compare the files file_p and rcsfile_p. Return zero if file_p has the same
 * contents as rcsfile_p and neither has keywords, otherwise -1 if they are
 * the same ignoring keyword values, and 1 if they differ even ignoring
 * keyword values. For the LOG-keyword, rcsfcmp skips the log message
 * given by the LogMessage in file_p. Thus, rcsfcmp returns nonpositive
 * if file_p contains the same as rcsfile_p, with the keywords expanded.
 * Implementation: character-by-character comparison until $ is found.
 * If a $ is found, read in the marker keywords; if they are real keywords
 * and identical, read in keyword value. If value is terminated properly,
 * disregard it and optionally skip log message; otherwise, compare value.
 *----------------------------------------------------------------------*/

int rcsfcmp(register FILE *file_p, register FILE *rcsfile_p)
{
	register int		file_c, rcsfile_c;
	char			file_keyword[keylength + 2];
	int			eqkeyvals;
	register int		file_eof, rcsfile_eof;
	register char	      * tp;
	int			result;
	enum markers		match1;

	file_eof = rcsfile_eof = false;

	file_c		= 0;
	rcsfile_c	= 0;	/* Keep lint happy. */
	result		= 0;

	for (;;)
	{
		if (file_c != KDELIM)		/* get the next characters */
		{
			if ((file_c = fgetc(file_p)) == EOF)
			{
				file_eof = true;
			}
			if ((rcsfile_c = fgetc(rcsfile_p)) == SDELIM)
			{
				int tmp = fgetc(rcsfile_p);
				if (tmp != SDELIM)
				{
					rcsfile_eof = true;
				}
			}
			if (file_eof | rcsfile_eof)
			{
				goto eof;
			}
		}
		else				/* try to get both keywords */
		{
			tp = file_keyword;
			for (;;)
			{
				if ((file_c = fgetc(file_p)) == EOF)
				{
					file_eof = true;
				}
				if ((rcsfile_c = fgetc(rcsfile_p)) == SDELIM)
				{
					int tmp = fgetc(rcsfile_p);
					if (tmp != SDELIM)
					{
						rcsfile_eof = true;
					}
				}
				if (file_eof | rcsfile_eof)
				{
					goto eof;
				}
				if (file_c != rcsfile_c)
				{
					break;
				}
				switch (file_c)
				{
				default:
					if ((file_keyword + keylength) <= tp)
					{
						break;
					}
					*tp++ = file_c;
					continue;
				case '\n':
				case KDELIM:
				case VDELIM:
					break;
				}
				break;
			}
			if (((file_c    == KDELIM) || (file_c    == VDELIM)) &&
			    ((rcsfile_c == KDELIM) || (rcsfile_c == VDELIM)) &&
			    (*tp = file_c,
			     (match1 = trymatch(file_keyword)) != Nomatch))
			{
				result = -1;
				for (;;)
				{
					if (file_c != rcsfile_c)
					{
						file_c	  = discardkeyval(file_c,    file_p,	EOF);
						rcsfile_c = discardkeyval(rcsfile_c, rcsfile_p,	SDELIM);
						if ((file_eof	 = (file_c    == EOF))    |
						    (rcsfile_eof = (rcsfile_c == SDELIM)))
						{
							goto eof;
						}
						eqkeyvals = false;
						break;
					}
					switch (file_c)
					{
					default:
						if ((file_c =
						     fgetc(file_p)) == EOF)
						{
							file_eof = true;
						}
						if ((rcsfile_c =
						     fgetc(rcsfile_p))
						    == SDELIM)
						{
							int tmp =
							    fgetc(rcsfile_p);
							if (tmp != SDELIM)
							{
							    rcsfile_eof = true;
							}
						}
						if (file_eof | rcsfile_eof)
						{
							goto eof;
						}
						continue;

					case '\n':
					case KDELIM:
						eqkeyvals = true;
						break;
					}
					break;
				}
				if (file_c != rcsfile_c)
				{
					goto return1;
				}

				/* Skip closing KDELIM. */
				if (file_c == KDELIM)
				{

					if ((file_c = fgetc(file_p)) == EOF)
					{
						file_eof = true;
					}
					if ((rcsfile_c = fgetc(rcsfile_p))
					    == SDELIM)
					{
						int tmp = fgetc(rcsfile_p);
						if (tmp != SDELIM)
						{
							rcsfile_eof = true;
						}
					}
					if (file_eof | rcsfile_eof)
					{
						goto eof;
					}

					/* if the keyword is LOG, also skip
					 * the log message in file_p.
					 */

					if (match1 == Log)
					{
						if (!match_log(file_p, file_c))
						{
							result = 1;
							goto returnresult;
						}
						if ((file_c = fgetc(file_p))
						    == EOF)
						{
							goto eof;
						}
					}

				}
				else
				{
					/* both end in the same character,
					 * but not a KDELIM: must compare
					 * string values.
					 */
					if ( ! eqkeyvals )
					{
						goto return1;
					}
				}
			}
		}
		if (file_c != rcsfile_c)
		{
			goto return1;
		}
	}

 eof:
	if (file_eof == rcsfile_eof)
	{
		goto returnresult;
	}

 return1:
	result = 1;

 returnresult:
	return(result);
}



/*----------------------------------------------------------------------
 *	parse_rcs_file	Routine to do a simple parse of the RCS file,
 *			reading in lock information, comment leader,
 *			the latest log message, and leaving the file
 *			pointer at the start of the string containing
 *			the latest revision.
 *			File format information based on the rcsfile(5)
 *			manual page.
 *----------------------------------------------------------------------*/

void parse_rcs_file(FILE *rcsfile_p, char *file_name)
{
	const char *locks = "locks";
	char c, ch, last_c = '\0';
	register char const *Suffix;
        register int i, index, length;

	/*--------------------------------------------------------------
	 * Read lock information: read lines until we have the keyword
	 *			'locks', then
	 *--------------------------------------------------------------*/
	index = 0;
	length = strlen(locks);
	for (;;)
	{
		c = fgetc(rcsfile_p);
		if (c == locks[index])
		{
			index++;
			if (index == length)
			{
				break;
			}
		}
		else
		{
			index = 0;
		}
	}

	/* Allocate memory for the Locks	*/
	if ((Locks = (char *) malloc(LockSize)) == NULL)
	{
		printf("Malloc failed for lock string memory\n");
		exit(-1);
	}
	memset(Locks, '\0', LockSize);

	index = 0;

	for (;;)
	{
		/*------------------------------------------------------
		 * First get a character from the RCS file.
		 *------------------------------------------------------*/

		last_c = c;
		if ((c = fgetc(rcsfile_p)) == EOF)
		{
			printf("Malformed RCS file - unexpected EOF\n");
			exit(-1);
		}

		/*------------------------------------------------------
		 * Now copy it into the Locks array, or go on to the next
		 * stage once a ';' has been found.
		 *------------------------------------------------------*/

		if (c == ';')
		{
			break;
		}

		if (isspace(c))
		{
			c = ' ';
			/* Cut out duplicate whitespace	*/
			if (c == last_c)
			{
				continue;
			}
		}
		Locks[index++] = c;
		if (index == LockSize)
		{
			char *old_Locks = Locks;
			int old_LockSize = LockSize;

			/* Double the amount of space	*/
			LockSize += LockSize;
			if ((Locks = (char *) malloc(LockSize)) == NULL)
			{
			    printf("Malloc failed for lock string memory\n");
			    exit(-1);
			}
			memset(Locks, '\0', LockSize);
			memcpy(Locks, old_Locks, old_LockSize);
			free(old_Locks);
		}
	}

	/*--------------------------------------------------------------
	 * Read comment information: Just after the locks information
	 *			will be the comment leader info if present.
	 *			Check its there, and if so read it in. If
	 *			not, set comment leader according to the
	 *			comtable from rcsfnms.c.
	 *--------------------------------------------------------------*/

        /* Guess the comment leader from the suffix
	 *
	 * THIS SECTION IS NOT COMPLETED YET - STILL NEED TO CHECK TO
	 * SEE WHETHER THERE IS A COMMENT LEADER EXPLICITLY SPECIFIED
	 * IN THE RCSFILE...
	 */

	if ( ! look_for("comment", rcsfile_p) )
	{
		Suffix = bindex(file_name, '.');
		if (Suffix == file_name)
		{
			Suffix= ""; /* empty suffix; will get default*/
		}
		for (i = 0; !suffix_matches(Suffix, comtable[i].suffix); i++)
		{
			continue;
		}

		Comment = (char *) comtable[i].comlead;
	}
	else
	{
		/* We have a comment leader in the file - just got to
		 * extract it....
		 */
		int length = 0;

		Comment = (char *) malloc(COMMENT_MAX);
		for (;;)
		{
			ch = fgetc(rcsfile_p);
			if (ch == EOF)
			{
				printf("End of file found while reading "
				       "comment leader\n");
				exit(-1);
			}
			if (ch == SDELIM)
			{
				ch = fgetc(rcsfile_p);
				if (ch == EOF)
				{
					printf("End of file found while "
					       "reading comment leader\n");
					exit(-1);
				}
				if (ch != SDELIM)
				{
					ungetc(ch, rcsfile_p);
					Comment[length] = '\0';
					break;
				}
			}
			Comment[length] = ch;
			length++;
		}
	}

	/*--------------------------------------------------------------
	 * Read log information: read lines until we have the keyword
	 *			'log', then read in string and store.
	 *--------------------------------------------------------------*/

	skip_to("log", rcsfile_p);

	if ((LogMessage = (char *) malloc(LogSize)) == NULL)
	{
		printf("Failed to allocate memory for Log message\n");
		exit(-1);
	}
	memset(LogMessage, '\0', LogSize);

	index = 0;
	for (;;)
	{
		c = fgetc(rcsfile_p);
		if (c == SDELIM)
		{
			c = fgetc(rcsfile_p);
			if (c != SDELIM)
			{
				ungetc(c, rcsfile_p);
				break;
			}
		}
		LogMessage[index++] = c;
		if (index == LogSize)
		{
			char *old_Log = LogMessage;
			int old_LogSize = LogSize;

			/* Double the amount of space	*/
			LogSize += LogSize;
			if ((LogMessage = (char *) malloc(LogSize)) == NULL)
			{
			    printf("Malloc failed for Log message memory\n");
			    exit(-1);
			}
			memset(LogMessage, '\0', LogSize);
			memcpy(LogMessage, old_Log, old_LogSize);
			free(old_Log);
		}
	}

	/*--------------------------------------------------------------
	 * Position file pointer: Read to next '@'
	 *--------------------------------------------------------------*/

	skip_to("text", rcsfile_p);
}

/*----------------------------------------------------------------------
 * sig_handle	Crude signal handler, really just to cope sensibly with
 *		interrupts. Probably not necessary.
 *----------------------------------------------------------------------*/

void sig_handle(int sig)
{
	printf("Signal %d received\n", sig);
	exit(-1);
}

/*----------------------------------------------------------------------
 *	main	Main part of rcsi. This is development version, so will
 *		change rapidly. At the moment simply a harness for
 *		testing the different bits of functionality.
 *----------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
	int current_arg;
	struct stat buf;

	/*--------------------------------------------------------------
	 * Don't think this is necessary - maybe remove it later. Maybe
	 * only worth having one of these if you call subprograms using
	 * something like 'system'...
	 *--------------------------------------------------------------*/

	if (signal(SIGINT, sig_handle) == SIG_ERR)
	{
		printf("Failed to install signal handler\n");
		exit(-1);
	}

	/*----------------------------------------------------------------
	 * First stat the RCS directory to see if it is present.
	 *----------------------------------------------------------------*/

	if (stat("RCS", &buf) == 0)
	{
		if (S_ISDIR(buf.st_mode))
		{
			/* Its a directory */
			g_RCSdir = true;
		}
		else
		{
			g_RCSdir = false;
		}
	}
	else
	{
		g_RCSdir = false;
	}

	/*----------------------------------------------------------------
	 * OK, now process the command line arguments
	 *----------------------------------------------------------------*/

	current_arg = 1;
	while (current_arg < argc)
	{
		if ((strcmp(argv[current_arg], "-v") == 0) ||
		    (strcmp(argv[current_arg], "--version") == 0))
		{
			printf("rcsi version %s\n", g_version);
			normal();
			exit(0);
		}
		else if ((strcmp(argv[current_arg], "-c") == 0) ||
		    (strcmp(argv[current_arg], "--changed") == 0))
		{
			g_changed = true;
		}
		else if ((strcmp(argv[current_arg], "-r") == 0) ||
			 (strcmp(argv[current_arg], "--rcsonly") == 0))
		{
			if (g_not_rcs) 
			{
				printf("Can't use --rcsonly AND --not-rcs at "
				       "the same time!!\n");
				exit(-1);
			}
			g_rcsonly = true;
		}
		else if ((strcmp(argv[current_arg], "-n") == 0) ||
			 (strcmp(argv[current_arg], "--not-rcs") == 0))
		{
			if (g_rcsonly) 
			{
				printf("Can't use --rcsonly AND --not-rcs at "
				       "the same time!!\n");
				exit(-1);
			}
			g_not_rcs = true;
		}
		else if ((strcmp(argv[current_arg], "-l") == 0) ||
			 (strcmp(argv[current_arg], "--locked") == 0))
		{
			if (g_unlocked)
			{
				printf("Can't use both --locked and --unlocked"
				       " (-u/-l) together\n");
				exit(-1);
			}
			g_locked = true;
		}
		else if ((strcmp(argv[current_arg], "-u") == 0) ||
			 (strcmp(argv[current_arg], "--unlocked") == 0))
		{
			if (g_locked)
			{
				printf("Can't use both --locked and --unlocked"
				       " (-u/-l) together\n");
				exit(-1);
			}
			g_unlocked = true;
		}
		else if ((strcmp(argv[current_arg], "-d") == 0) ||
			 (strcmp(argv[current_arg], "-a") == 0) ||
			 (strcmp(argv[current_arg], "--dot") == 0))
		{
			g_dot = true;
		}
                else if ((strcmp(argv[current_arg], "--colour") == 0) ||
                         (strcmp(argv[current_arg], "--color") == 0))
                {
			g_colour = true;
                }
                else if ((strcmp(argv[current_arg], "--no-colour") == 0) ||
                         (strcmp(argv[current_arg], "--no-color") == 0))
		{
			g_colour = false;
                }
		else if (strcmp(argv[current_arg], "--DEBUG") == 0)
		{
			/*------------------------------------------------
			 * Turn on debugging information.
			 *------------------------------------------------*/
			g_debug = true;
			if ((g_debug_file = fopen("/tmp/rcsi.dump", "w"))
			    == NULL)
			{
				printf("Failed to open debugging log file\n");
				exit(-1);
			}
			fprintf(g_debug_file,"       Function Name : Line  : "
				"Message\n"
				"--------------------------------------------"
				"--------------------------\n");
		}
		else if ((strcmp(argv[current_arg], "-h") == 0) ||
			 (strcmp(argv[current_arg], "--help") == 0))
		{
			printf("rcsi: Check the status of RCS files. Usage: "
			       "rcsi [options] [filenames]\n"
			       "-h --help      This message\n"
			       "-v --version   Display the version number\n"
			       "-c --changed   Include only RCS files that "
			       "have changed\n"
			       "-l --locked    Include only RCS files that "
			       "are locked\n"
			       "-u --unlocked  Include only RCS files that "
			       "are unlocked\n"
			       "-d --dot -a    Include dot files\n"
			       "-r --rcsonly   Include only RCS related "
			       "files\n"
			       "-n --not-rcs   Include only non-RCS related "
			       "files\n"
                               "--colo[u]r     Use colour (default)\n"
                               "--no-colo[u]r  Don't use colour\n");
			normal();
			exit(0);
		}
		else
		{
			/*------------------------------------------------
			 * Must be a filename argument.
			 *------------------------------------------------*/
			process_filearg(argv[current_arg]);
		}
		current_arg++;
	}

	/*----------------------------------------------------------------
	 * Now for the real processing - read the files in (if necessary)
	 * and store them in a tree-like data-structure. This is where the
	 * correlation between RCS and working files is done. Then print
	 * the resulting tree.
	 *----------------------------------------------------------------*/

	if (!g_fileargs)
	{
		read_files();
	}
	else
	{
		report("Using files on command line\n");
	}

	print_tree();

	if (g_debug)
	{
		report("Closing debugging file\n");
		if (fclose(g_debug_file) == EOF)
		{
			printf("Failed to close debugging file\n");
			exit(-1);
		}
	}

	normal();
	exit(0);
}
