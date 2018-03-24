/* TREELANG Compiler definitions for interfacing to treetree.c
   (compiler back end interface).
   
   Copyright (C) 1986, 87, 89, 92-96, 1997, 1999, 2000, 2001, 2002, 2003,
   2004, 2005, 2007 Free Software Foundation, Inc.
   
   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 3, or (at your option) any
   later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING3.  If not see
   <http://www.gnu.org/licenses/>.
   
   In other words, you are welcome to use, share and improve this program.
   You are forbidden to forbid anyone else to use, share and improve
   what you give them. Help stamp out software-hoarding!  
   
   ---------------------------------------------------------------------------
   
   Written by Tim Josling 1999, 2000, 2001, based in part on other
   parts of the GCC compiler.  */

bool joke_init (void);

void joke_finish (void);

void joke_init_options (unsigned int, struct cl_decoded_option *);

bool joke_handle_option (size_t scode, const char *arg,
		   int value ATTRIBUTE_UNUSED, int kind ATTRIBUTE_UNUSED,
		   location_t loc ATTRIBUTE_UNUSED,
		   const struct cl_option_handlers *handlers ATTRIBUTE_UNUSED);

bool joke_post_options (const char **);

unsigned int joke_option_lang_mask(void);

void joke_parse_file ();

static bool joke_mark_addressable (tree exp);

static tree joke_type_for_size (unsigned precision, int unsignedp);

static tree joke_type_for_mode (enum machine_mode mode, int unsignedp);

/* Functions to keep track of the current scope */
static tree pushdecl (tree decl);

/* Langhooks.  */
const struct attribute_spec joke_attribute_table[] = {
  { NULL,                     0, 0, false, false, false, NULL, false }
};

static void joke_write_globals (void);

static tree getdecls (void);

static bool global_bindings_p (void);

/* The front end language hooks (addresses of code for this front
   end).  These are not really very language-dependent, i.e.
   treelang, C, Mercury, etc. can all use almost the same definitions.  */

#undef LANG_HOOKS_MARK_ADDRESSABLE
#define LANG_HOOKS_MARK_ADDRESSABLE joke_mark_addressable
#undef LANG_HOOKS_TYPE_FOR_MODE
#define LANG_HOOKS_TYPE_FOR_MODE joke_type_for_mode
#undef LANG_HOOKS_TYPE_FOR_SIZE
#define LANG_HOOKS_TYPE_FOR_SIZE joke_type_for_size
#undef LANG_HOOKS_PARSE_FILE
#define LANG_HOOKS_PARSE_FILE joke_parse_file
#undef LANG_HOOKS_ATTRIBUTE_TABLE
#define LANG_HOOKS_ATTRIBUTE_TABLE joke_attribute_table
#undef LANG_HOOKS_WRITE_GLOBALS
#define LANG_HOOKS_WRITE_GLOBALS joke_write_globals

/* Hook routines and data unique to treelang.  */

#undef LANG_HOOKS_INIT
#define LANG_HOOKS_INIT joke_init
#undef LANG_HOOKS_NAME
#define LANG_HOOKS_NAME "GNU Sample Frontend"
#undef LANG_HOOKS_FINISH
#define LANG_HOOKS_FINISH joke_finish
#undef LANG_HOOKS_INIT_OPTIONS
#define LANG_HOOKS_INIT_OPTIONS joke_init_options
#undef LANG_HOOKS_HANDLE_OPTION
#define LANG_HOOKS_HANDLE_OPTION joke_handle_option
#undef LANG_HOOKS_POST_OPTIONS
#define LANG_HOOKS_POST_OPTIONS joke_post_options
#undef LANG_HOOKS_OPTION_LANG_MASK
#define LANG_HOOKS_OPTION_LANG_MASK joke_option_lang_mask
