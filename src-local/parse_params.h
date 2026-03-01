/**
# parse_params.h

Low-level runtime key/value parameter parser for oscillating-film cases.

Parses plain `key=value` files, strips whitespace, ignores `#` comments, and
stores the most recent value for each key.
*/

#ifndef OSCILLATING_FILM_PARSE_PARAMS_H
#define OSCILLATING_FILM_PARSE_PARAMS_H

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifndef PARSE_PARAMS_MAX_ENTRIES
#define PARSE_PARAMS_MAX_ENTRIES 256
#endif

#ifndef PARSE_PARAMS_KEY_LEN
#define PARSE_PARAMS_KEY_LEN 128
#endif

#ifndef PARSE_PARAMS_VALUE_LEN
#define PARSE_PARAMS_VALUE_LEN 256
#endif

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

static inline int parse_params_find_key(const char *key)
{
  for (int i = 0; i < _parse_params_state.count; i++)
    if (!strcmp(_parse_params_state.entries[i].key, key))
      return i;
  return -1;
}

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

static inline const char *parse_params_current_file(void)
{
  return _parse_params_state.file;
}

static inline void parse_params_ensure_loaded(void)
{
  if (!_parse_params_state.loaded)
    (void) parse_params_load(_parse_params_state.file);
}

static inline const char *parse_params_get_string(const char *key,
                                                  const char *default_value)
{
  parse_params_ensure_loaded();
  int idx = parse_params_find_key(key);
  return idx >= 0 ? _parse_params_state.entries[idx].value : default_value;
}

#endif // OSCILLATING_FILM_PARSE_PARAMS_H
