#include "cargs.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* str;
    size_t len;
} carg_string;

typedef struct {
    int argc;
    char** argv;
    int* i;
    cargs_all* map;
    size_t map_size;
} carg_context;

static inline carg_string carg_string_create(char* s) {
    return (carg_string) { s, strlen(s) };
}

static inline bool is_long_arg(carg_string msg) {
    return msg.len > 2 && msg.str[0] == '-' && msg.str[1] == '-' && isalpha(msg.str[2]);
}

static inline bool is_short_arg(carg_string msg) {
    return msg.len == 2 && msg.str[0] == '-' && isalpha(msg.str[1]);
}

static carg_context _ctx;
void internal_help() {
    printf("Usage: %s [options]\n"
           "Options:\n",
        _ctx.argv[0]);
    for (size_t i = 0; i < _ctx.map_size; ++i) {
        switch (_ctx.map[i].type) {
        case _CARGS_CALLBACK:
            printf("  --%-*s -%c         %s\n", 15, _ctx.map[i].arg.common.long_arg, _ctx.map[i].arg.common.short_arg, _ctx.map[i].desc);
            break;
        case _CARGS_TOGGLE:
            printf("  --%-*s -%c         %s\n", 15, _ctx.map[i].arg.common.long_arg, _ctx.map[i].arg.common.short_arg, _ctx.map[i].desc);
            break;
        case _CARGS_VALUE: {
            char str[64] = "";
            strcat(str, _ctx.map[i].arg.common.long_arg);
            strcat(str, " <value>");
            printf("  --%-*s -%c <value> %s\n", 15, str, _ctx.map[i].arg.common.short_arg, _ctx.map[i].desc);
            break;
        }
        }
    }
}

static inline bool find_in_map(carg_string* arg, carg_context* ctx, cargs_all* o_entry) {
    bool islong = is_long_arg(*arg);
    if (!islong && !is_short_arg(*arg)) {
        return false;
    }
    for (size_t i = 0; i < ctx->map_size; ++i) {
        if (islong && strcmp(ctx->map[i].arg.common.long_arg, &arg->str[2]) == 0) {
            *o_entry = ctx->map[i];
            return true;
        } else if (ctx->map[i].arg.common.short_arg == arg->str[1]) {
            *o_entry = ctx->map[i];
            return true;
        } else {
            // printf("%c != %c\n", ctx->map[i].arg.common.short_arg, arg->str[1]);
        }
    }
    // not found, is it --help or -h?
    // if so, invoke builtin help print
    if ((islong && strcmp("help", &arg->str[2]) == 0)
        || arg->str[1] == 'h') {
        cargs_all help;
        help.type = _CARGS_CALLBACK;
        help.arg.callback_arg.callback = internal_help;
        help.arg.callback_arg.long_arg = "help";
        help.arg.callback_arg.short_arg = 'h';
        *o_entry = help;
        _ctx = *ctx; // copy context for help print later
        return true;
    }
    return false;
}

typedef void (*callback_type)();
typedef void (*callback_value)(char*);
// expect no more than 20 normal callbacks to be needed
static callback_type callbacks[20];
static size_t callbacks_index = 0;

typedef struct {
    callback_value cb;
    char* arg;
} value_callback_container;

static value_callback_container value_callback_containers[20];
static size_t value_callback_containers_index = 0;

static inline bool handle_arg(cargs_all* arg, carg_context* ctx) {
    switch (arg->type) {
    case _CARGS_CALLBACK:
        callbacks[callbacks_index++] = arg->arg.callback_arg.callback;
        break;
    case _CARGS_TOGGLE:
        // is present, so set to true
        if (arg->arg.toggle_arg.toggle) {
            *arg->arg.toggle_arg.toggle = true;
        }
        break;
    case _CARGS_VALUE: {
        value_callback_container cont;
        cont.cb = arg->arg.value_arg.callback;
        if (*ctx->i + 1 < ctx->argc) {
            ++(*ctx->i); // advance i
            cont.arg = ctx->argv[*ctx->i]; // use advanced i to get the next arg, which is a value
        } else {
            printf("expected a value for argument \"%s\"\n", ctx->argv[*ctx->i]);
            return false;
        }
        value_callback_containers[value_callback_containers_index++] = cont;
        break;
    }
    default:
        printf("broken arg type, aborting\n");
        return false;
    }
    return true;
}

bool _cargs_run(int argc, char** argv, cargs_all* map, size_t map_size) {
    // set all toggles to false by default
    for (size_t i = 0; i < map_size; ++i) {
        if (map[i].type == _CARGS_TOGGLE
            && map[i].arg.toggle_arg.toggle) {
            *map[i].arg.toggle_arg.toggle = false;
        }
    }
    int i;
    carg_context ctx = { argc, argv, &i, map, map_size };
    for (i = 1; i < argc; ++i) {
        carg_string arg = carg_string_create(argv[i]);
        cargs_all result;
        bool found = find_in_map(&arg, &ctx, &result);
        if (found) {
            bool rc = handle_arg(&result, &ctx);
            if (!rc) {
                printf("requirements for argument \"%s\" not satisfied\n", arg.str);
                return false;
            }
        } else {
            printf("invalid argument \"%s\"\n", arg.str);
            return false;
        }
    }

    // call all callbacks in order
    for (size_t i = 0; i < callbacks_index; ++i) {
        callbacks[i]();
    }
    for (size_t i = 0; i < value_callback_containers_index; ++i) {
        value_callback_container cont = value_callback_containers[i];
        cont.cb(cont.arg);
    }
    return true;
}

