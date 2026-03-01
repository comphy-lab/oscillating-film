/**
# parse_params.h

Low-level runtime key/value parameter parser for oscillating-film cases.

Parses plain `key=value` files, strips whitespace, ignores `#` comments, and
stores the most recent value for each key.

## Usage

Include this header and call:

1. `parse_params_init_from_argv(argc, argv)` or `parse_params_load(file)`
2. `parse_params_get_string(key, default_value)` for retrieval

This header is used by [params.h](params.h) to provide typed accessors.
*/

#ifndef OSCILLATING_FILM_PARSE_PARAMS_H
#define OSCILLATING_FILM_PARSE_PARAMS_H

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/**
## Compile-Time Limits

Upper bounds for in-memory parameter table size and key/value string lengths.
*/
#ifndef PARSE_PARAMS_MAX_ENTRIES
#define PARSE_PARAMS_MAX_ENTRIES 256
#endif

#ifndef PARSE_PARAMS_KEY_LEN
#define PARSE_PARAMS_KEY_LEN 128
#endif

#ifndef PARSE_PARAMS_VALUE_LEN
#define PARSE_PARAMS_VALUE_LEN 256
#endif

/**
## Data Structures
*/
typedef struct {
  char key[PARSE_PARAMS_KEY_LEN];
  char value[PARSE_PARAMS_VALUE_LEN];
} ParseParamEntry;

typedef struct {
  ParseParamEntry entries[PARSE_PARAMS_MAX_ENTRIES];
  int count;
  bool loaded;
  bool warned_missing;
  char file[PARSE_PARAMS_VALUE_LEN];
} ParseParamsState;

static ParseParamsState _parse_params_state = {
  .count = 0,
  .loaded = false,
  .warned_missing = false,
  .file = "case.params",
};

/**
### parse_params_trim()

Trims leading and trailing whitespace in-place.
*/
static inline char *parse_params_trim(char *s)
{
  if (!s)
    return s;

  while (*s && isspace((unsigned char)*s))
    s++;

  size_t n = strlen(s);
  while (n > 0 && isspace((unsigned char)s[n - 1]))
    s[--n] = '\0';

  return s;
}

/**
### parse_params_find_key()

Returns the entry index for `key`, or `-1` when the key is not loaded.
*/
static inline int parse_params_find_key(const char *key)
{
  for (int i = 0; i < _parse_params_state.count; i++)
    if (!strcmp(_parse_params_state.entries[i].key, key))
      return i;
  return -1;
}

/**
### parse_params_set_value()

Inserts or updates a key in the internal table.
*/
static inline void parse_params_set_value(const char *key, const char *value)
{
  int idx = parse_params_find_key(key);

  if (idx < 0) {
    if (_parse_params_state.count >= PARSE_PARAMS_MAX_ENTRIES) {
      fprintf(stderr,
              "WARNING: parse_params.h entry limit reached (%d), skipping '%s'\n",
              PARSE_PARAMS_MAX_ENTRIES, key);
      return;
    }

    idx = _parse_params_state.count++;
    strncpy(_parse_params_state.entries[idx].key, key, PARSE_PARAMS_KEY_LEN - 1);
    _parse_params_state.entries[idx].key[PARSE_PARAMS_KEY_LEN - 1] = '\0';
  }

  strncpy(_parse_params_state.entries[idx].value, value, PARSE_PARAMS_VALUE_LEN - 1);
  _parse_params_state.entries[idx].value[PARSE_PARAMS_VALUE_LEN - 1] = '\0';
}

/**
### parse_params_load()

Loads key/value pairs from `filename` into the internal table.

#### Notes

- Lines without `=` are ignored.
- `#` starts an inline comment.
- Empty keys/values are ignored.
*/
static inline int parse_params_load(const char *filename)
{
  _parse_params_state.count = 0;
  _parse_params_state.loaded = true;

  if (filename && filename[0]) {
    strncpy(_parse_params_state.file, filename, PARSE_PARAMS_VALUE_LEN - 1);
    _parse_params_state.file[PARSE_PARAMS_VALUE_LEN - 1] = '\0';
  }

  FILE *fp = fopen(_parse_params_state.file, "r");
  if (!fp) {
    if (!_parse_params_state.warned_missing) {
      fprintf(stderr,
              "WARNING: Parameter file '%s' not found. Using defaults.\n",
              _parse_params_state.file);
      _parse_params_state.warned_missing = true;
    }
    return -1;
  }

  char line[PARSE_PARAMS_KEY_LEN + PARSE_PARAMS_VALUE_LEN + 64];
  while (fgets(line, sizeof(line), fp)) {
    char *comment = strchr(line, '#');
    if (comment)
      *comment = '\0';

    char *eq = strchr(line, '=');
    if (!eq)
      continue;

    *eq = '\0';
    char *key = parse_params_trim(line);
    char *value = parse_params_trim(eq + 1);

    if (!key || !value || !*key || !*value)
      continue;

    parse_params_set_value(key, value);
  }

  fclose(fp);
  return 0;
}

/**
### parse_params_init_from_argv()

Selects parameter file from `argv[1]` when present, otherwise uses
`case.params`, then loads it.
*/
static inline void parse_params_init_from_argv(int argc, const char *argv[])
{
  if (argc > 1 && argv[1] && argv[1][0]) {
    strncpy(_parse_params_state.file, argv[1], PARSE_PARAMS_VALUE_LEN - 1);
    _parse_params_state.file[PARSE_PARAMS_VALUE_LEN - 1] = '\0';
  } else {
    strncpy(_parse_params_state.file, "case.params", PARSE_PARAMS_VALUE_LEN - 1);
    _parse_params_state.file[PARSE_PARAMS_VALUE_LEN - 1] = '\0';
  }

  (void) parse_params_load(_parse_params_state.file);
}

/**
### parse_params_current_file()

Returns the current parameter-file path used by the parser state.
*/
static inline const char *parse_params_current_file(void)
{
  return _parse_params_state.file;
}

/**
### parse_params_ensure_loaded()

Loads the current parameter file lazily if the table is not loaded yet.
*/
static inline void parse_params_ensure_loaded(void)
{
  if (!_parse_params_state.loaded)
    (void) parse_params_load(_parse_params_state.file);
}

/**
### parse_params_get_string()

Returns the string value for `key`, or `default_value` when absent.
*/
static inline const char *parse_params_get_string(const char *key,
                                                  const char *default_value)
{
  parse_params_ensure_loaded();
  int idx = parse_params_find_key(key);
  return idx >= 0 ? _parse_params_state.entries[idx].value : default_value;
}

#endif // OSCILLATING_FILM_PARSE_PARAMS_H
