#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include "parse-option.h"


static int option_index = 1;

static int first_nonopt = 1;
static int last_nonopt = 1;


static void
exchange (char **argv)
{
    int bottom = first_nonopt;
    int middle = last_nonopt;
    int top = option_index;
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
    first_nonopt += (option_index - last_nonopt);
    last_nonopt = option_index;
}

static enum parse_opt_result get_value(const char *optarg, const struct option *opt)
{
    const char *kTrue[] = {"1", "t", "true", "y", "yes"};
    const char *kFalse[] = {"0", "f", "false", "n", "no"};
    long r;
    switch (opt->type)
    {
    case OPTION_ENABLE:
        *(bool *) opt->val = true;
        return PARSE_OPT_DONE;
    case OPTION_DISABLE:
        *(bool *) opt->val = false;
        return PARSE_OPT_DONE;
    case OPTION_BOOL:
        for(size_t i = 0; i < sizeof(kTrue)/sizeof(*kTrue); ++i)
        {
            if (strcmp(optarg, kTrue[i]) == 0)
            {
                *(bool *) opt->val = true;
                return PARSE_OPT_DONE;
            }
        }

        for(size_t i = 0; i < sizeof(kFalse)/sizeof(*kFalse); ++i)
        {
            if (strcmp(optarg, kFalse[i]) == 0)
            {
                *(bool *) opt->val = false;
                return PARSE_OPT_DONE;
            }
        }
        return PARSE_OPT_ERROR;
    case OPTION_INT32:
        r = strtol(optarg, NULL, 0);
        *(int *) opt->val = (int) r;
        return PARSE_OPT_DONE;
    case OPTION_INT64:
        r = strtol(optarg, NULL, 0);
        *(long *) opt->val = r;
        return PARSE_OPT_DONE;
    case OPTION_STRING:
        *(const char **) opt->val = (const char *)optarg;
        return PARSE_OPT_DONE;

    default:
        return PARSE_OPT_ERROR;
    }
}



static enum parse_opt_result parse_short_opt(const char *arg, const struct option *options)
{
    for(; options->type != OPTION_END; options++)
    {
        if (options->short_name == *arg)
        {
            const char *optarg = (*arg+1) ? arg + 2 : NULL;
            return get_value(optarg, options);
        }
    }
    return PARSE_OPT_UNKNOWN;
}


static enum parse_opt_result parse_long_opt(const char *arg, const struct option *options)
{

    char *nameend;
    for (nameend = (char *)arg; *nameend && *nameend != '='; nameend++)
        ;

    for(; options->type != OPTION_END; options++)
    {
        if (options->long_name && !strncmp(options->long_name, arg, nameend-arg))
        {
            if ((unsigned int) (nameend - arg) == (unsigned int) strlen(options->long_name))
            {
                const char *optarg = (*nameend) ? nameend + 1 : NULL;
                return get_value(optarg, options);
            }
        }
    }
    return PARSE_OPT_UNKNOWN;
}

#define USAGE_OPTS_WIDTH 24
#define USAGE_GAP         2

static enum parse_opt_result usage_with_options(const struct option *opts)
{
    FILE *outfile = stderr;

    // display options
    for(; opts->type != OPTION_END; opts++)
    {
        size_t pos;
        int pad;

        if (opts->type == OPTION_USAGE)
        {
            fprintf(outfile, "usage: %s\n", opts->help);
            fputc('\n', outfile);
            continue;
        }

        if (opts->type == OPTION_GROUP)
        {
            fprintf(outfile, "%s\n", opts->help);
            continue;
        }

        pos = fprintf(outfile, "    ");
        if (opts->short_name)
        {
            pos += fprintf(outfile, "-%c", opts->short_name);
        }
        if (opts->long_name && opts->short_name)
        {
            pos += fprintf(outfile, ", ");
        }
        if (opts->long_name)
        {
            pos += fprintf(outfile, "--%s", opts->long_name);
        }

        if (opts->usage)
        {
            pos += fprintf(outfile, "[=<%s>]", opts->usage);
        }

        if(pos <= USAGE_OPTS_WIDTH)
        {
            pad = USAGE_OPTS_WIDTH - pos;
        }
        else
        {
            fputc('\n', outfile);
            pad = USAGE_OPTS_WIDTH;
        }
        fprintf(outfile, "%*s%s\n", pad + USAGE_GAP, "", opts->help);
    }
    fputc('\n', outfile);

    return PARSE_OPT_HELP;
}

void reset_optind(int index)
{
    option_index = index;
    last_nonopt = index;
    first_nonopt = index;
}

int parse_options(int argc, const char **argv, const struct option *options)
{

    while(option_index <= argc)
    {
        if (last_nonopt > option_index)
        {
            last_nonopt = option_index;
        }
        if (first_nonopt > option_index)
        {
            first_nonopt = option_index;
        }
        if (first_nonopt != last_nonopt && last_nonopt != option_index)
        {
            exchange((char **) argv);
        }
        else if (last_nonopt != option_index)
        {
            first_nonopt = option_index;
        }
        // skip non-options
        while(option_index < argc && (argv[option_index][0] != '-' || argv[option_index][1] == '\0'))
        {
            option_index++;
        }
        last_nonopt = option_index;

        if(option_index == argc)
        {
            if(first_nonopt != last_nonopt)
            {
                option_index = first_nonopt;
            }
            return option_index;
        }

        const char *arg = argv[option_index];
        // parse short options
        if (arg[1] != '-')
        {
            if (arg[1] == 'h')
            {
                return usage_with_options(options);
            }

            switch (parse_short_opt(arg+1, options))
            {
            case PARSE_OPT_ERROR:
                return PARSE_OPT_ERROR;
            default:
                break;
            }
        }
        // parse long options
        if (!strcmp(arg+2, "help"))
        {
            return usage_with_options(options);
        }

        switch (parse_long_opt(arg+2, options))
        {
        case PARSE_OPT_ERROR:
            return PARSE_OPT_ERROR;
        default:
            break;
        }
        option_index++;
    }

    return PARSE_OPT_ERROR;
}


