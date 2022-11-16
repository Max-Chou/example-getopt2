#include <stdio.h>
#include <stdbool.h>
#include "parse-option.h"

static bool enable = false;
static const char *filename = "default";

static struct option options[] = {
    {OPTION_USAGE, 0, NULL, NULL, NULL, "example [<options>] ..."},
    {OPTION_GROUP, 0, NULL, NULL, NULL, "Options"},
    {OPTION_ENABLE, 'e', "enable", &enable, NULL, "Enable feature a"},
    {OPTION_STRING, 'f', "filename", &filename, "filename", "Filename"},
    {OPTION_END}
};


int main(int argc, const char **argv)
{
    reset_optind(2);

    int index = parse_options(argc, argv, options);
    printf("%d\n", index);

    printf("%d\n", enable);
    printf("%s\n", filename);

    for(int i =0; i < argc; i++)
    {
        printf("%s\n", argv[i]);
    }

    return 0;
}