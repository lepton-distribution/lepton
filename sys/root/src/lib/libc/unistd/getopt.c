/*
The contents of this file are subject to the Mozilla Public License Version 1.1
(the "License"); you may not use this file except in compliance with the License.
You may obtain a copy of the License at http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the
specific language governing rights and limitations under the License.

The Original Code is Lepton.

The Initial Developer of the Original Code is Chauvin-Arnoux.
Portions created by Chauvin-Arnoux are Copyright (C) 2011. All Rights Reserved.

Alternatively, the contents of this file may be used under the terms of the eCos GPL license
(the  [eCos GPL] License), in which case the provisions of [eCos GPL] License are applicable
instead of those above. If you wish to allow use of your version of this file only under the
terms of the [eCos GPL] License and not to allow others to use your version of this file under
the MPL, indicate your decision by deleting  the provisions above and replace
them with the notice and other provisions required by the [eCos GPL] License.
If you do not delete the provisions above, a recipient may use your version of this file under
either the MPL or the [eCos GPL] License."
*/

/*
 * Modified for uClibc by Manuel Novoa III on 1/5/01.
 * Modified once again for uClibc by Erik Andersen 8/7/02
 */

#include "lib/libc/unistd/getopt.h"
#include "lib/libc/stdio/stdio.h"

#include <string.h>
#include <stdlib.h>

//#include <libintl.h>


/* Treat '-W foo' the same as the long option '--foo',
 * disabled for the moment since it costs about 2k... */
#undef SPECIAL_TREATMENT_FOR_W

/* This version of `getopt' appears to the caller like standard Unix `getopt'
   but it behaves differently for the user, since it allows the user
   to intersperse the options with the other arguments.

   As `getopt' works, it permutes the elements of ARGV so that,
   when it is done, all the options precede everything else.  Thus
   all application programs are extended to handle flexible argument order.

   Setting the environment variable POSIXLY_CORRECT disables permutation.
   Then the behavior is completely standard.

   GNU application programs can use a third alternative mode in which
   they can distinguish the relative order of options and other arguments.  */



/*extern int _getopt_internal (int argc, char *const *argv, const char *optstring,
        const struct option *longopts, int *longind, int long_only);*/



#define optarg    getopt_state->optarg
#define optind    getopt_state->optind
#define opterr    getopt_state->opterr
#define optopt    getopt_state->optopt
#define nextchar  getopt_state->nextchar

#define first_nonopt getopt_state->first_nonopt
#define last_nonopt  getopt_state->last_nonopt

#define __getopt_initialized  getopt_state->__getopt_initialized


/* Describe how to deal with options that follow non-option ARGV-elements.

   If the caller did not specify anything,
   the default is REQUIRE_ORDER if the environment variable
   POSIXLY_CORRECT is defined, PERMUTE otherwise.

   REQUIRE_ORDER means don't recognize them as options;
   stop option processing when the first non-option is seen.
   This is what Unix does.
   This mode of operation is selected by either setting the environment
   variable POSIXLY_CORRECT, or using `+' as the first character
   of the list of option characters.

   PERMUTE is the default.  We permute the contents of ARGV as we scan,
   so that eventually all the non-options are at the end.  This allows options
   to be given in any order, even with programs that were not written to
   expect this.

   RETURN_IN_ORDER is an option available to programs that were written
   to expect options and other ARGV-elements in any order and that care about
   the ordering of the two.  We describe each non-option ARGV-element
   as if it were the argument of an option with character code 1.
   Using `-' as the first character of the list of option characters
   selects this mode of operation.

   The special argument `--' forces an end of option-scanning regardless
   of the value of `ordering'.  In the case of RETURN_IN_ORDER, only
   `--' can cause `getopt' to return -1 with `optind' != ARGC.  */




static enum
{
   REQUIRE_ORDER, PERMUTE, RETURN_IN_ORDER
} ordering;

# include <string.h>
# define my_index       strchr

/* Handle permutation of arguments.  */

/* Exchange two adjacent subsequences of ARGV.
   One subsequence is elements [first_nonopt,last_nonopt)
   which contains all the non-options that have been skipped so far.
   The other is elements [last_nonopt,optind), which contains all
   the options processed since those non-options were skipped.

   `first_nonopt' and `last_nonopt' are relocated so that they describe
   the new indices of the non-options in ARGV after they are moved.  */


static void exchange (getopt_state_t* getopt_state, char **argv)
{
   int bottom = first_nonopt;
   int middle = last_nonopt;
   int top = optind;
   char *tem;

   /* Exchange the shorter segment with the far end of the longer segment.
      That puts the shorter segment into the right place.
      It leaves the longer segment in the right place overall,
      but it consists of two parts that need to be swapped next.  */

   while (top > middle && middle > bottom)
   {
      if (top - middle > middle - bottom)
      {
         /* Bottom segment is the short one.  */
         int len = middle - bottom;
         register int i;

         /* Swap it with the top part of the top segment.  */
         for (i = 0; i < len; i++)
         {
            tem = argv[bottom + i];
            argv[bottom + i] = argv[top - (middle - bottom) + i];
            argv[top - (middle - bottom) + i] = tem;
         }
         /* Exclude the moved bottom segment from further swapping.  */
         top -= len;
      }
      else
      {
         /* Top segment is the short one.  */
         int len = top - middle;
         register int i;

         /* Swap it with the bottom part of the bottom segment.  */
         for (i = 0; i < len; i++)
         {
            tem = argv[bottom + i];
            argv[bottom + i] = argv[middle + i];
            argv[middle + i] = tem;
         }
         /* Exclude the moved top segment from further swapping.  */
         bottom += len;
      }
   }

   /* Update records for the slots the non-options now occupy.  */

   first_nonopt += (optind - last_nonopt);
   last_nonopt = optind;
}

/* Initialize the internal data when the first call is made.  */

static const char *_getopt_initialize (getopt_state_t* getopt_state, int argc, char *const * argv,
                                       const char *optstring)
{
   /* Start processing options with ARGV-element 1 (since ARGV-element 0
      is the program name); the sequence of previously skipped
      non-option ARGV-elements is empty.  */

   first_nonopt = last_nonopt = optind;

   nextchar = NULL;

   /* Determine how to handle the ordering of options and nonoptions.  */

   if (optstring[0] == '-')
   {
      ordering = RETURN_IN_ORDER;
      ++optstring;
   }
   else if (optstring[0] == '+')
   {
      ordering = REQUIRE_ORDER;
      ++optstring;
   }
   /*else if (getenv ("POSIXLY_CORRECT") != NULL)
          ordering = REQUIRE_ORDER;*/
   else
      ordering = PERMUTE;

   return optstring;
}

/* Scan elements of ARGV (whose length is ARGC) for option characters
   given in OPTSTRING.

   If an element of ARGV starts with '-', and is not exactly "-" or "--",
   then it is an option element.  The characters of this element
   (aside from the initial '-') are option characters.  If `getopt'
   is called repeatedly, it returns successively each of the option characters
   from each of the option elements.

   If `getopt' finds another option character, it returns that character,
   updating `optind' and `nextchar' so that the next call to `getopt' can
   resume the scan with the following option character or ARGV-element.

   If there are no more option characters, `getopt' returns -1.
   Then `optind' is the index in ARGV of the first ARGV-element
   that is not an option.  (The ARGV-elements have been permuted
   so that those that are not options now come last.)

   OPTSTRING is a string containing the legitimate option characters.
   If an option character is seen that is not listed in OPTSTRING,
   return '?' after printing an error message.  If you set `opterr' to
   zero, the error message is suppressed but we still return '?'.

   If a char in OPTSTRING is followed by a colon, that means it wants an arg,
   so the following text in the same ARGV-element, or the text of the following
   ARGV-element, is returned in `optarg'.  Two colons mean an option that
   wants an optional arg; if there is text in the current ARGV-element,
   it is returned in `optarg', otherwise `optarg' is set to zero.

   If OPTSTRING starts with `-' or `+', it requests different methods of
   handling the non-option ARGV-elements.
   See the comments about RETURN_IN_ORDER and REQUIRE_ORDER, above.

   Long-named options begin with `--' instead of `-'.
   Their names may be abbreviated as long as the abbreviation is unique
   or is an exact match for some defined option.  If they have an
   argument, it follows the option name in the same ARGV-element, separated
   from the option name by a `=', or else the in next ARGV-element.
   When `getopt' finds a long-named option, it returns 0 if that option's
   `flag' field is nonzero, the value of the option's `val' field
   if the `flag' field is zero.

   The elements of ARGV aren't really const, because we permute them.
   But we pretend they're const in the prototype to be compatible
   with other systems.

   LONGOPTS is a vector of `struct option' terminated by an
   element containing a name which is zero.

   LONGIND returns the index in LONGOPT of the long-named option found.
   It is only valid when a long-named option has been found by the most
   recent call.

   If LONG_ONLY is nonzero, '-' as well as '--' can introduce
   long-named options.  */

int _getopt_internal (getopt_state_t* getopt_state, int argc, char *const *argv,
                      const char *optstring,
                      const struct option *longopts, int *longind,
                      int long_only)
{
   int print_errors = opterr;
   if (optstring[0] == ':')
      print_errors = 0;

   if (argc < 1)
      return -1;

   optarg = NULL;

   if (optind == 0 || !__getopt_initialized)
   {
      if (optind == 0)
         optind = 1;    /* Don't scan ARGV[0], the program name.  */
      optstring = _getopt_initialize (getopt_state,argc, argv, optstring);
      __getopt_initialized = 1;
   }

   /* Test whether ARGV[optind] points to a non-option argument.
      Either it does not have option syntax, or there is an environment flag
      from the shell indicating it is not an option.  The later information
      is only used when the used in the GNU libc.  */
#define NONOPTION_P (argv[optind][0] != '-' || argv[optind][1] == '\0')

   if (nextchar == NULL || *nextchar == '\0')
   {
      /* Advance to the next ARGV-element.  */

      /* Give FIRST_NONOPT & LAST_NONOPT rational values if OPTIND has been
         moved back by the user (who may also have changed the arguments).  */
      if (last_nonopt > optind)
         last_nonopt = optind;
      if (first_nonopt > optind)
         first_nonopt = optind;

      if (ordering == PERMUTE)
      {
         /* If we have just processed some options following some non-options,
            exchange them so that the options come first.  */

         if (first_nonopt != last_nonopt && last_nonopt != optind)
            exchange (getopt_state,(char **) argv);
         else if (last_nonopt != optind)
            first_nonopt = optind;

         /* Skip any additional non-options
            and extend the range of non-options previously skipped.  */

         while (optind < argc && NONOPTION_P)
            optind++;
         last_nonopt = optind;
      }

      /* The special ARGV-element `--' means premature end of options.
         Skip it like a null option,
         then exchange with previous non-options as if it were an option,
         then skip everything else like a non-option.  */

      if (optind != argc && !strcmp (argv[optind], "--"))
      {
         optind++;

         if (first_nonopt != last_nonopt && last_nonopt != optind)
            exchange (getopt_state,(char **) argv);
         else if (first_nonopt == last_nonopt)
            first_nonopt = optind;
         last_nonopt = argc;

         optind = argc;
      }

      /* If we have done all the ARGV-elements, stop the scan
         and back over any non-options that we skipped and permuted.  */

      if (optind == argc)
      {
         /* Set the next-arg-index to point at the non-options
            that we previously skipped, so the caller will digest them.  */
         if (first_nonopt != last_nonopt)
            optind = first_nonopt;
         return -1;
      }

      /* If we have come to a non-option and did not permute it,
         either stop the scan or describe it to the caller and pass it by.  */

      if (NONOPTION_P)
      {
         if (ordering == REQUIRE_ORDER)
            return -1;
         optarg = argv[optind++];
         return 1;
      }

      /* We have found another option-ARGV-element.
         Skip the initial punctuation.  */

      nextchar = (argv[optind] + 1
                  + (longopts != NULL && argv[optind][1] == '-'));
   }

   /* Decode the current option-ARGV-element.  */

   /* Check whether the ARGV-element is a long option.

      If long_only and the ARGV-element has the form "-f", where f is
      a valid short option, don't consider it an abbreviated form of
      a long option that starts with f.  Otherwise there would be no
      way to give the -f short option.

      On the other hand, if there's a long option "fubar" and
      the ARGV-element is "-fu", do consider that an abbreviation of
      the long option, just like "--fu", and not "-f" with arg "u".

      This distinction seems to be the most useful approach.  */

   if (longopts != NULL
       && (argv[optind][1] == '-'
           || (long_only && (argv[optind][2] || !my_index (optstring, argv[optind][1])))))
   {
      char *nameend;
      const struct option *p;
      const struct option *pfound = NULL;
      int exact = 0;
      int ambig = 0;
      int indfound = -1;
      int option_index;

      for (nameend = nextchar; *nameend && *nameend != '='; nameend++)
         /* Do nothing.  */;

      /* Test all long options for either exact match
         or abbreviated matches.  */
      for (p = longopts, option_index = 0; p->name; p++, option_index++)
         if (!strncmp (p->name, nextchar, nameend - nextchar))
         {
            if ((unsigned int) (nameend - nextchar)
                == (unsigned int) strlen (p->name))
            {
               /* Exact match found.  */
               pfound = p;
               indfound = option_index;
               exact = 1;
               break;
            }
            else if (pfound == NULL)
            {
               /* First nonexact match found.  */
               pfound = p;
               indfound = option_index;
            }
            else if (long_only
                     || pfound->has_arg != p->has_arg
                     || pfound->flag != p->flag
                     || pfound->val != p->val)
               /* Second or later nonexact match found.  */
               ambig = 1;
         }

      if (ambig && !exact)
      {
         if (print_errors)
         {
            fprintf (stderr, "%s: option `%s' is ambiguous\n",
                     argv[0], argv[optind]);
         }
         nextchar += strlen (nextchar);
         optind++;
         optopt = 0;
         return '?';
      }

      if (pfound != NULL)
      {
         option_index = indfound;
         optind++;
         if (*nameend)
         {
            /* Don't test has_arg with >, because some C compilers don't
               allow it to be used on enums.  */
            if (pfound->has_arg)
               optarg = nameend + 1;
            else
            {
               if (print_errors)
               {

                  if (argv[optind - 1][1] == '-')
                  {
                     /* --option */
                     fprintf (stderr, "\
					%s: option `--%s' doesn't allow an argument\n",
                              argv[0], pfound->name);
                  }
                  else
                  {
                     /* +option or -option */
                     fprintf (stderr, "\
					%s: option `%c%s' doesn't allow an argument\n",
                              argv[0], argv[optind - 1][0], pfound->name);
                  }

               }

               nextchar += strlen (nextchar);

               optopt = pfound->val;
               return '?';
            }
         }
         else if (pfound->has_arg == 1)
         {
            if (optind < argc)
               optarg = argv[optind++];
            else
            {
               if (print_errors)
               {
                  fprintf (stderr,
                           "%s: option `%s' requires an argument\n",
                           argv[0], argv[optind - 1]);
               }
               nextchar += strlen (nextchar);
               optopt = pfound->val;
               return optstring[0] == ':' ? ':' : '?';
            }
         }
         nextchar += strlen (nextchar);
         if (longind != NULL)
            *longind = option_index;
         if (pfound->flag)
         {
            *(pfound->flag) = pfound->val;
            return 0;
         }
         return pfound->val;
      }

      /* Can't find it as a long option.  If this is not getopt_long_only,
         or the option starts with '--' or is not a valid short
         option, then it's an error.
         Otherwise interpret it as a short option.  */
      if (!long_only || argv[optind][1] == '-'
          || my_index (optstring, *nextchar) == NULL)
      {
         if (print_errors)
         {

            if (argv[optind][1] == '-')
            {
               /* --option */
               fprintf (stderr, "%s: unrecognized option `--%s'\n",
                        argv[0], nextchar);
            }
            else
            {
               /* +option or -option */
               fprintf (stderr, "%s: unrecognized option `%c%s'\n",
                        argv[0], argv[optind][0], nextchar);
            }

         }
         nextchar = (char *) "";
         optind++;
         optopt = 0;
         return '?';
      }
   }

   /* Look at and handle the next short option-character.  */

   {
      char c = *nextchar++;
      char *temp = my_index (optstring, c);

      /* Increment `optind' when we start to process its last character.  */
      if (*nextchar == '\0')
         ++optind;

      if (temp == NULL || c == ':')
      {
         if (print_errors)
         {
            /* 1003.2 specifies the format of this message.  */
            fprintf (stderr, "%s: illegal option -- %c\n", argv[0], c);
         }
         optopt = c;
         return '?';
      }
#ifdef SPECIAL_TREATMENT_FOR_W
      /* Convenience. Treat POSIX -W foo same as long option --foo */
      if (temp[0] == 'W' && temp[1] == ';')
      {
         char *nameend;
         const struct option *p;
         const struct option *pfound = NULL;
         int exact = 0;
         int ambig = 0;
         int indfound = 0;
         int option_index;

         /* This is an option that requires an argument.  */
         if (*nextchar != '\0')
         {
            optarg = nextchar;
            /* If we end this ARGV-element by taking the rest as an arg,
               we must advance to the next element now.  */
            optind++;
         }
         else if (optind == argc)
         {
            if (print_errors)
            {
               /* 1003.2 specifies the format of this message.  */
               fprintf (stderr, _("%s: option requires an argument -- %c\n"),
                        argv[0], c);
            }
            optopt = c;
            if (optstring[0] == ':')
               c = ':';
            else
               c = '?';
            return c;
         }
         else
            /* We already incremented `optind' once;
               increment it again when taking next ARGV-elt as argument.  */
            optarg = argv[optind++];

         /* optarg is now the argument, see if it's in the
            table of longopts.  */

         for (nextchar = nameend = optarg; *nameend && *nameend != '='; nameend++)
            /* Do nothing.  */;

         /* Test all long options for either exact match
            or abbreviated matches.  */
         for (p = longopts, option_index = 0; p->name; p++, option_index++)
            if (!strncmp (p->name, nextchar, nameend - nextchar))
            {
               if ((unsigned int) (nameend - nextchar) == strlen (p->name))
               {
                  /* Exact match found.  */
                  pfound = p;
                  indfound = option_index;
                  exact = 1;
                  break;
               }
               else if (pfound == NULL)
               {
                  /* First nonexact match found.  */
                  pfound = p;
                  indfound = option_index;
               }
               else
                  /* Second or later nonexact match found.  */
                  ambig = 1;
            }
         if (ambig && !exact)
         {
            if (print_errors)
            {
               fprintf (stderr, _("%s: option `-W %s' is ambiguous\n"),
                        argv[0], argv[optind]);
            }
            nextchar += strlen (nextchar);
            optind++;
            return '?';
         }
         if (pfound != NULL)
         {
            option_index = indfound;
            if (*nameend)
            {
               /* Don't test has_arg with >, because some C compilers don't
                  allow it to be used on enums.  */
               if (pfound->has_arg)
                  optarg = nameend + 1;
               else
               {
                  if (print_errors)
                  {
                     fprintf (stderr, _("\
					%s: option `-W %s' doesn't allow an argument\n"),
                              argv[0], pfound->name);
                  }

                  nextchar += strlen (nextchar);
                  return '?';
               }
            }
            else if (pfound->has_arg == 1)
            {
               if (optind < argc)
                  optarg = argv[optind++];
               else
               {
                  if (print_errors)
                  {
                     fprintf (stderr,
                              _("%s: option `%s' requires an argument\n"),
                              argv[0], argv[optind - 1]);
                  }
                  nextchar += strlen (nextchar);
                  return optstring[0] == ':' ? ':' : '?';
               }
            }
            nextchar += strlen (nextchar);
            if (longind != NULL)
               *longind = option_index;
            if (pfound->flag)
            {
               *(pfound->flag) = pfound->val;
               return 0;
            }
            return pfound->val;
         }
         nextchar = NULL;
         return 'W';    /* Let the application handle it.   */
      }
#endif
      if (temp[1] == ':')
      {
         if (temp[2] == ':')
         {
            /* This is an option that accepts an argument optionally.  */
            if (*nextchar != '\0')
            {
               optarg = nextchar;
               optind++;
            }
            else
               optarg = NULL;
            nextchar = NULL;
         }
         else
         {
            /* This is an option that requires an argument.  */
            if (*nextchar != '\0')
            {
               optarg = nextchar;
               /* If we end this ARGV-element by taking the rest as an arg,
                  we must advance to the next element now.  */
               optind++;
            }
            else if (optind == argc)
            {
               if (print_errors)
               {
                  /* 1003.2 specifies the format of this message.  */
                  fprintf (stderr,
                           "%s: option requires an argument -- %c\n",
                           argv[0], c);
               }
               optopt = c;
               if (optstring[0] == ':')
                  c = ':';
               else
                  c = '?';
            }
            else
               /* We already incremented `optind' once;
                  increment it again when taking next ARGV-elt as argument.  */
               optarg = argv[optind++];
            nextchar = NULL;
         }
      }
      return c;
   }
}

int getopt_init(getopt_state_t* getopt_state){

   optarg = NULL;
   optind = 1;
   opterr = 1;
   optopt = '?';

   __getopt_initialized = 0;

   return 0;
}

int getopt (getopt_state_t* getopt_state, int argc, char *const *argv, const char *optstring)
{
   return _getopt_internal (getopt_state,argc, argv, optstring,
                            (const struct option *) 0, (int *) 0, 0);
}

int getopt_long (getopt_state_t* getopt_state, int argc, char *const *argv, const char *options,
                 const struct option *long_options, int *opt_index)
{
   return _getopt_internal (getopt_state,argc, argv, options, long_options, opt_index, 0);
}

/* Like getopt_long, but '-' as well as '--' can indicate a long option.
   If an option that starts with '-' (not '--') doesn't match a long option,
   but does match a short option, it is parsed as a short option
   instead.  */

int getopt_long_only (getopt_state_t* getopt_state, int argc, char *const *argv,
                      const char *options,
                      const struct option *long_options,
                      int *opt_index)
{
   return _getopt_internal (getopt_state,argc, argv, options, long_options, opt_index, 1);
}

