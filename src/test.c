#include "cargs.h"
#include <stdio.h>

void cmd_noop() {
    printf("noop!");
}

void cmd_echo(char* s) {
    if (s) {
        puts(s);
    } else {
        puts("(null)");
    }
}

bool quiet;

CARGS_ARGS {
    //CARG("help", 'h', cmd_help),
    CARG_TOGGLE("quiet", 'q', &quiet, "disable logging"),
    CARG("noop", 'n', cmd_noop, "does nothing"),
    CARG_VALUE("echo", 'e', cmd_echo, "echoes the value"),
};

int main(int argc, char** argv) {
    bool ret = cargs_run(argc, argv);
    printf("quiet: %s\n", quiet ? "true" : "false");
    if (!ret) {
        printf("something went wrong! oh no!\n");
        return 1;
    }
}
