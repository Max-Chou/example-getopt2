#ifndef _GETOPT2_H
#define _GETOPT2_H

extern char *optarg;

extern int optind;

extern int opterr;

extern int optopt;

struct option
{
    const char *name;
    int has_arg;
    int *flag;
    int val;
};

#define no_argument             0
#define required_argument       1
#define optional_argument       2

extern int getopt(int argc, char *const *argv, const char *shortopts);
extern int getopt_long(int argc, char *const *argv, const char *shortopts, const struct option *longopts, int *longind);

/* Internal only. Users should not call this directly. */
extern int _getopt_internal(
    int argc, char *const *argv,
    const char *shortopts,
    const struct option *longopts, int *longind,
    int long_only
);

#define NONOPTION_P (argv[optind][0] != '-' || argv[optind][1] == '\0')

#endif