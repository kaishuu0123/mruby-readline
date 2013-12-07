#include "mruby.h"
#include "mruby/value.h"
#include "mruby/class.h"
#include "mruby/string.h"
#include "mruby/array.h"
#include "mruby/variable.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

mrb_state *readline_mrb_state; /* XXX */
static char **readline_attempted_completion_function(const char *text, int start, int end);

static char **readline_attempted_completion_function(const char *text, int start, int end)
{
  char **result;
  int matches = 0;
  mrb_value proc, ary, append_character;
  mrb_state *mrb = readline_mrb_state;
  mrb_value *self = mrb->c->stack;
 
  /* TODO 
  append_character = mrb_iv_get(mrb, *self,
      mrb_intern_lit(mrb, "@completion_append_character"));
  if (mrb_nil_p(append_character)) {
    return NULL;
  }
  fprintf(stderr, "append '%s'\n", RSTRING_PTR(append_character));
  rl_completion_append_character = RSTRING_PTR(append_character);
  */

#ifdef HAVE_RL_ATTEMPTED_COMPLETION_OVER
      rl_attempted_completion_over = 1; 
#endif

  proc = mrb_iv_get(mrb, *self, mrb_intern_lit(mrb, "@completion_proc"));
  if (mrb_nil_p(proc)) {
    return NULL;
  }

  ary = mrb_funcall(mrb, proc, "call", 1, mrb_str_new_cstr(mrb, text));
  if (!mrb_array_p(ary)) {
    /* XXX: convert to array? */
    return NULL;
  }

  matches = RARRAY_LEN(ary);
  if (matches == 0) {
    return NULL;
  }
  result = (char**)malloc((matches + 2) * sizeof(char*));
  for (int i = 0; i < matches; i++) {
    mrb_value tmp;
    tmp = RARRAY_PTR(ary)[i];
    result[i + 1] = (char *)malloc(RSTRING_LEN(tmp) + 1);
    strcpy(result[i + 1], RSTRING_PTR(tmp));
  }
  result[matches + 1] = NULL;

  const char *result1 = result[1];
  long low = strlen(result1);

  if (matches == 1) {
    result[0] = strdup(result[1]);
  } else {
    for (int i = 1; i < matches; i++) {
      int c1, c2;
      long i1, i2, l2;
      int n1, n2;
      const char *p2 = result[i + 1];

      l2 = strlen(p2);
      for (i1 = i2 = 0; i1 < low && i2 < l2; i1 += n1, i2 += n2) {
        c1 = result1 + i1;
        c2 = p2 + i2;
        if (c1 != c2)
          break;
      }

      low = i1;
      result[0] = (char *)malloc(low + 1);
      strncpy(result[0], result[1], low);
      result[0][low] = '\0';
    }
  }

  return result;
}

static mrb_value
mrb_c_readline(mrb_state *mrb, mrb_value self)
{
  mrb_value str, result_str, add_hist;
  char *s, *line;

  mrb_get_args(mrb, "Sb", &str, &add_hist);
  s = RSTRING_PTR(str);
  line = readline(s);
 
  if (line) {
    result_str = mrb_str_new(mrb, line, strlen(line));
    free(line);

    return result_str;
  }

  return mrb_nil_value();
}

void
mrb_mruby_readline_gem_init(mrb_state* mrb) {
  struct RClass *mod_readline;
  mod_readline = mrb_define_module(mrb, "Readline");
  readline_mrb_state = mrb;

  rl_attempted_completion_function = readline_attempted_completion_function;
  rl_bind_key('\t',rl_complete);

  mrb_define_module_function(mrb, mod_readline, "_c_readline", mrb_c_readline, MRB_ARGS_NONE());
}

void
mrb_mruby_readline_gem_final(mrb_state* mrb) {
  // finalizer
}

void
mrb_mruby_readline_gem_test(mrb_state* mrb) {
  // finalizer
}
