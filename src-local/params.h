/**
# params.h

Typed runtime parameter accessors for oscillating-film simulation cases.

`params_init_from_argv(argc, argv)` loads `argv[1]` when provided, otherwise
falls back to `case.params`.

## API

- `params_load(file)`: explicit load
- `params_init_from_argv(argc, argv)`: argv-aware load
- `param_string(key, default)`
- `param_double(key, default)`
- `param_int(key, default)`
- `param_bool(key, default)`
*/

#ifndef OSCILLATING_FILM_PARAMS_H
#define OSCILLATING_FILM_PARAMS_H

#include "parse_params.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

/**
## Loader Wrappers

Thin wrappers around `parse_params.h` to keep call sites concise.
*/
static inline int params_load(const char *filename)
{
  return parse_params_load(filename);
}

static inline void params_init_from_argv(int argc, const char *argv[])
{
  parse_params_init_from_argv(argc, argv);
}

/**
### param_string()

Returns a raw string value or fallback when key is missing.
*/
static inline const char *param_string(const char *key, const char *default_value)
{
  return parse_params_get_string(key, default_value);
}

/**
### param_double()

Parses a `double` with fallback and warning on malformed values.
*/
static inline double param_double(const char *key, double default_value)
{
  const char *s = param_string(key, NULL);
  if (!s)
    return default_value;

  errno = 0;
  char *end = NULL;
  double v = strtod(s, &end);
  while (end && *end && isspace((unsigned char)*end))
    end++;

  if (errno != 0 || end == s || (end && *end != '\0')) {
    fprintf(stderr,
            "WARNING: Invalid double for '%s' ('%s') in %s, using default %g\n",
            key, s, parse_params_current_file(), default_value);
    return default_value;
  }

  return v;
}

/**
### param_int()

Parses an `int` with range checks, fallback, and warning on malformed values.
*/
static inline int param_int(const char *key, int default_value)
{
  const char *s = param_string(key, NULL);
  if (!s)
    return default_value;

  errno = 0;
  char *end = NULL;
  long v = strtol(s, &end, 10);
  while (end && *end && isspace((unsigned char)*end))
    end++;

  if (errno != 0 || end == s || (end && *end != '\0') ||
      v < INT_MIN || v > INT_MAX) {
    fprintf(stderr,
            "WARNING: Invalid int for '%s' ('%s') in %s, using default %d\n",
            key, s, parse_params_current_file(), default_value);
    return default_value;
  }

  return (int) v;
}

/**
### param_bool()

Accepts common boolean spellings (`1/0`, `true/false`, `yes/no`, `on/off`)
and falls back to default on invalid strings.
*/
static inline bool param_bool(const char *key, bool default_value)
{
  const char *s = param_string(key, NULL);
  if (!s)
    return default_value;

  if (!strcasecmp(s, "1") || !strcasecmp(s, "true") ||
      !strcasecmp(s, "yes") || !strcasecmp(s, "on"))
    return true;
  if (!strcasecmp(s, "0") || !strcasecmp(s, "false") ||
      !strcasecmp(s, "no") || !strcasecmp(s, "off"))
    return false;

  fprintf(stderr,
          "WARNING: Invalid bool for '%s' ('%s') in %s, using default %d\n",
          key, s, parse_params_current_file(), default_value ? 1 : 0);
  return default_value;
}

#endif // OSCILLATING_FILM_PARAMS_H
