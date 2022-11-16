#ifndef PARSE_OPTIONS_H
#define PARSE_OPTIONS_H

enum parse_opt_type {
    OPTION_ENABLE,
    OPTION_DISABLE,
    OPTION_BOOL,
    OPTION_INT32,
    OPTION_UINT32,
    OPTION_INT64,
    OPTION_UINT64,
    OPTION_DOUBLE,
    OPTION_STRING,
    OPTION_USAGE,
    OPTION_GROUP,
    OPTION_END
};

enum parse_opt_result {
    PARSE_OPT_HELP = -2,
    PARSE_OPT_ERROR = -1,
    PARSE_OPT_DONE = 0,
    PARSE_OPT_NON_OPTION,
    PARSE_OPT_UNKNOWN
};

struct option {
    enum parse_opt_type type;
    int short_name;
    const char *long_name;
    void *val;
    const char *usage;
    const char *help;
};

extern void reset_optind(int index);
extern int parse_options(int argc, const char **argv, const struct option *options);



#endif