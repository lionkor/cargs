#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    _CARGS_CALLBACK,
    _CARGS_TOGGLE,
    _CARGS_VALUE,
} cargs_type;

typedef struct {
    char* long_arg;
    char short_arg;
} cargs_common;

typedef struct {
    char* long_arg;
    char short_arg;
    void (*callback)();
} cargs_arg;

typedef struct {
    char* long_arg;
    char short_arg;
    bool* toggle;
} cargs_arg_toggle;

typedef struct {
    char* long_arg;
    char short_arg;
    void (*callback)(char*);
} cargs_arg_value;

typedef struct _cargs_all {
    cargs_type type;
    union _a {
        cargs_common common;
        cargs_arg callback_arg;
        cargs_arg_toggle toggle_arg;
        cargs_arg_value value_arg;
    } arg;
    char* desc;
} cargs_all;

#define _DEF_CARG(_cargs_d_type, _cargs_d_member, _cargs_d_first, _cargs_d_second, _cargs_d_third, _cargs_d_desc) \
    {                                                                                                             \
        .type = _cargs_d_type,                                                                                    \
        .arg = {                                                                                                  \
            ._cargs_d_member = { _cargs_d_first, _cargs_d_second, _cargs_d_third }                                \
        },                                                                                                        \
        .desc = _cargs_d_desc                                                                                     \
    }

// calls a callback if the flag is present, after all arguments have been parsed
// example: CARG("help", 'h', help_callback)
#define CARG(...) _DEF_CARG(_CARGS_CALLBACK, callback_arg, __VA_ARGS__)
// toggles a bool to true if the flag is present, false if not
// example: CARG_TOGGLE("quiet", 'q', &quiet_switch_bool)
#define CARG_TOGGLE(...) _DEF_CARG(_CARGS_TOGGLE, toggle_arg, __VA_ARGS__)
// matches arguments with single values, like
// --count=2
// or
// -c 2
// and calls the callback with that value
// example: CARG_VALUE("count", 'n', callback_with_arg)
#define CARG_VALUE(...) _DEF_CARG(_CARGS_VALUE, value_arg, __VA_ARGS__)

#define CARGS_ARGS static cargs_all _cargs_array[] =

bool _cargs_run(int argc, char** argv, cargs_all* map, size_t map_size);

#define cargs_run(argc, argv) _cargs_run(argc, argv, _cargs_array, sizeof(_cargs_array) / sizeof(cargs_all));
