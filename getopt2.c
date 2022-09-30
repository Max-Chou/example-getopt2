#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "getopt2.h"

/* When `getopt' finds an option that takes an argument, the argument value is returned here.*/
char *optarg = NULL;

/* index in argv of the next element to be scanned */
int optind = 1;

/* error message for unrecognized options. */
int opterr = 1;

/* option character which was unreconginzed. */
int optopt = '?';

/* the next char to be scanned in the option-element in which the last option character we returned was found.*/
static char *nextchar = NULL;
static char *
my_index (const char *str, int chr)
{
    while (*str)
    {
        if(*str == chr)
            return (char *) str;
        str++;
    }
    return 0;
}
static int first_nonopt = 1;
static int last_nonopt = 1;

/* Exchange two adjacent subsequences of ARGV. */
/* One subsequence is elements [first_nonopt, last_nonopt) which contains all
the non-options that have been skipped so far.*/
/* The other is elements [last_nonopt, optind), which contains all the options processed 
since those non-options were skipped.
*/
static void
exchange (char **argv)
{
    int bottom = first_nonopt;
    int middle = last_nonopt;
    int top = optind;
    char *tem;

    while (top > middle && middle > bottom)
    {
        if (top - middle > middle - bottom)
        {
            int len = middle - bottom;
            int i;

            for(i=0;i<len;i++)
            {
                tem = argv[bottom+i];
                argv[bottom+i] = argv[top-(middle-bottom)+i];
                argv[top-(middle-bottom)+i] = tem;
            }
            top -= len;
        }
        else
        {
            int len = top - middle;
            int i;
            for(i=0;i<len;i++)
            {
                tem = argv[bottom+i];
                argv[bottom+i] = argv[middle+i];
                argv[middle+i] = tem;
            }
            bottom += len;
        }
    }
    first_nonopt += (optind - last_nonopt);
    last_nonopt = optind;
}



int 
_getopt_internal(int argc, char *const *argv, const char *optstring,
                 const struct option *longopts,
                 int *longind, int long_only)
{
    optarg = NULL;
    if (nextchar == NULL || *nextchar == '\0')
    {
        if (last_nonopt > optind)
            last_nonopt = optind;
        if (first_nonopt > optind)
            first_nonopt = optind;
        
        if (first_nonopt != last_nonopt && last_nonopt != optind)
            exchange((char **) argv);
        else if (last_nonopt != optind)
            first_nonopt = optind;

        /* skip any additional non-options */
        while (optind < argc && NONOPTION_P)
            optind++;
        last_nonopt = optind;
    
        /* the special ARGV-element `--' means premature end of options. */
        if (optind != argc && !strcmp(argv[optind], "--"))
        {
            optind++;
            if (first_nonopt != last_nonopt && last_nonopt != optind)
                exchange ((char **) argv);
            else if (first_nonopt == last_nonopt)
                first_nonopt = optind;
            last_nonopt = argc;

            optind = argc;
        }

        if (optind == argc)
        {
            if (first_nonopt != last_nonopt)
                optind = first_nonopt;
            return -1;
        }

        if (NONOPTION_P)
        {
            optarg = argv[optind++];
            return 1;
        }

        nextchar = (argv[optind] + 1
            + (longopts != NULL && argv[optind][1] == '-'));
    }

    if (longopts != NULL
        && (argv[optind][1] == '-'
            || (long_only && (argv[optind][2] || !my_index(optstring, argv[optind][1])))))
    {
        char *nameend;
        const struct option *p;
        const struct option *pfound = NULL;
        int exact = 0;
        int ambig = 0;
        int indfound = -1;
        int option_index;

        for (nameend = nextchar; *nameend && *nameend != '='; nameend++)
            ;

        /* Test all long options for either exact match or abreviated matches. */
        for (p = longopts, option_index=0; p->name; p++, option_index++)
        {
            if (!strncmp(p->name, nextchar, nameend - nextchar))
            {
                /* exact match */
                if ((unsigned int) (nameend - nextchar) == (unsigned int) strlen(p->name))
                {
                    pfound = p;
                    indfound = option_index;
                    exact = 1;
                    break;
                }
                else if (pfound == NULL)
                {
                    pfound = NULL;
                    indfound = option_index;
                }
                else
                {
                    ambig = 1;
                }
            }
        }

        if(ambig && !exact)
        {
            if (opterr)
            {
                fprintf(stderr, "%s: option `%s' is ambiguous\n", argv[0], argv[optind]);
            }
            nextchar += strlen(nextchar);
            optind++;
            optopt = 0;
            return '?';
        }

        if(pfound != NULL)
        {
            option_index = indfound;
            optind++;
            if (*nameend)
            {
                if (pfound->has_arg)
                {
                    optarg = nameend + 1;
                }
                else 
                {
                    if (opterr)
                    {
                        if (argv[optind-1][1] == '-')
                        {
                            fprintf(stderr, "%s: option `--%s' doesn't allow an argument\n", argv[0], pfound->name);
                        }
                        else 
                        {
                            fprintf(stderr, "%s: option `%c%s' doesn't allow an argument\n", argv[0], argv[optind-1][0], pfound->name);
                        }
                    }
                    nextchar += strlen(nextchar);
                    optopt = pfound->val;
                    return '?';
                }
            }
            else if (pfound->has_arg == 1)
            {
                if (optind < argc)
                {
                    optarg = argv[optind++];
                }
                else 
                {
                    if (opterr)
                    {
                        fprintf(stderr, "%s: option `%s' requires an argument\n", argv[0], argv[optind-1]);
                    }
                    nextchar += strlen(nextchar);
                    optopt = pfound->val;
                    return optstring[0] == ':' ? ':' : '?';
                }
            }
            nextchar += strlen(nextchar);
            if (longind != NULL)
            {
                *longind = option_index;
            }
            
            if (pfound->flag)
            {
                *(pfound->flag) = pfound->val;
                return 0;
            }
            return pfound->val;
        }
        if (!long_only || argv[optind][1] == '-' || my_index(optstring, *nextchar) == NULL)
        {
            if (opterr)
            {
                if (argv[optind][1] == '-')
                {
                    fprintf(stderr, "%s: unrecognized option `--%s'\n", argv[0], nextchar);
                }
                else 
                {
                    fprintf(stderr, "%s: unrecognized option `%c%s'\n", argv[0], argv[optind][0], nextchar);
                }
            }
            nextchar = (char *) "";
            optind++;
            optopt = 0;
            return '?';
        }
    }

    {
        char c = *nextchar++;
        char *temp = my_index(optstring, c);

        if (*nextchar == '\0')
        {
            ++optind;
        }

        if (temp == NULL || c == ':')
        {
            if (opterr)
            {
                fprintf(stderr, "%s: invalid option -- %c\n", argv[0], c);
            }
            optopt = c;
            return '?';
        }

        if (temp[0] == 'W' && temp[1] == ';')
        {
            char *nameend;
            const struct option *p;
            const struct option *pfound = NULL;
            int exact = 0;
            int ambig = 0;
            int indfound = 0;
            int option_index;

            if (*nextchar != '\0')
            {
                optarg = nextchar;
                optind++;
            }
            else if (optind == argc)
            {
                if (opterr)
                {
                    fprintf(stderr, "%s: option requires an argument -- %c\n", argv[0], c);
                }
                optopt = c;
                if (optstring[0] == ':')
                {
                    c = ':';
                }
                else 
                {
                    c = '?';
                }
                return c;
            }
            else 
            {
                optarg = argv[optind++];
            }
            for (nextchar = nameend = optarg; *nameend && *nameend != '='; nameend++)
                ;
            

            for(p = longopts, option_index = 0; p->name; p++, option_index++)
            {
                if (!strncmp(p->name, nextchar, nameend - nextchar))
                {
                    if ((unsigned int) (nameend - nextchar) == strlen(p->name))
                    {
                        pfound = p;
                        indfound = option_index;
                        exact = 1;
                        break;
                    }
                    else if (pfound == NULL)
                    {
                        pfound = p;
                        indfound = option_index;
                    }
                    else 
                    {
                        ambig = 1;
                    }
                }
            }


            if (ambig && !exact )
            {
                if(opterr)
                {
                    fprintf(stderr, "%s: option `-W %s' is ambiguous\n", argv[0], argv[optind]);
                    nextchar += strlen(nextchar);
                    optind++;
                    return '?';
                }
            }

            if (pfound != NULL)
            {
                option_index = indfound;
                if (*nameend)
                {
                    if (pfound->has_arg)
                    {
                        optarg = nameend + 1;
                    }
                    else 
                    {
                        if (opterr)
                        {
                            fprintf(stderr,"%s: option `-W %s' doesn't allow an argument\n", argv[0], pfound->name);
                        }
                        nextchar += strlen(nextchar);
                        return '?';
                    }
                }
                else if (pfound->has_arg == 1)
                {
                    if(optind < argc)
                    {
                        optarg = argv[optind++];
                    }
                    else 
                    {
                        if (opterr)
                        {
                            fprintf(stderr, "%s: option `%s' requires an argument\n", argv[0], argv[optind-1]);
                        }
                        nextchar += strlen(nextchar);
                        return optstring[0] == ':' ? ':' : '?';
                    }
                }
                nextchar += strlen(nextchar);
                if (longind != NULL)
                {
                    *longind = option_index;
                }

                if (pfound->flag)
                {
                    *(pfound->flag) = pfound->val;
                    return 0;
                }
                return pfound->val;
            }
            nextchar = NULL;
            return 'W';
        }

        if (temp[1] == ':')
        {
            if(temp[2] == ':')
            {
                if (*nextchar != '\0')
                {
                    optarg = nextchar;
                    optind++;
                }
                else
                {
                    optarg = NULL;
                }
                nextchar = NULL;
            }
            else 
            {
                if (*nextchar != '\0')
                {
                    optarg = nextchar;
                    optind++;
                }
                else if (optind == argc)
                {
                    if (opterr)
                    {
                        fprintf(stderr, "%s: option requires an argument -- %c\n", argv[0], c);
                    }
                    optopt = c;
                    if (optstring[0] == ':')
                    {
                        c = ':';
                    }
                    else
                    {
                        c = '?';
                    }
                }
                else 
                {
                    optarg = argv[optind++];
                }
                nextchar = NULL;
            }
        }
        return c;
    }
}

int
getopt (int argc, char *const *argv, const char *optstring)
{
  return _getopt_internal (argc, argv, optstring,
			   (const struct option *) 0,
			   (int *) 0,
			   0);
}

int
getopt_long (int argc,  char *const *argv,  const char *options,
             const struct option *long_options, int *opt_index)
{
  return _getopt_internal (argc, argv, options, long_options, opt_index, 0);
}

int
getopt_long_only (int argc, char *const *argv, const char *options,
                  const struct option *long_options, int *opt_index)
{
  return _getopt_internal (argc, argv, options, long_options, opt_index, 1);
}

#ifdef TEST

/* Compile with -DTEST to make an executable for use in testing
   the above definition of `getopt'.  */

int
main (int argc, char **argv)
{
  int c;
  int digit_optind = 0;

  while (1)
    {
      int this_option_optind = optind ? optind : 1;

      c = getopt (argc, argv, "abc:d:0123456789");
      if (c == -1)
	break;

      switch (c)
	{
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	  if (digit_optind != 0 && digit_optind != this_option_optind)
	    printf ("digits occur in two different argv-elements.\n");
	  digit_optind = this_option_optind;
	  printf ("option %c\n", c);
	  break;

	case 'a':
	  printf ("option a\n");
	  break;

	case 'b':
	  printf ("option b\n");
	  break;

	case 'c':
	  printf ("option c with value `%s'\n", optarg);
	  break;

	case '?':
	  break;

	default:
	  printf ("?? getopt returned character code 0%o ??\n", c);
	}
    }

  if (optind < argc)
    {
      printf ("non-option ARGV-elements: ");
      while (optind < argc)
	printf ("%s ", argv[optind++]);
      printf ("\n");
    }

  exit (0);
}

#endif /* TEST */