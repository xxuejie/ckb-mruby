/* Modified from
 * https://github.com/mruby/mruby/blob/ec957ec5eee6c0a2874948c1b063e89d9fbd4eab/mrbgems/mruby-bin-mirb/tools/mirb/mirb.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mruby.h>
#include <mruby/compile.h>
#include <mruby/data.h>
#include <mruby/proc.h>
#include "mruby/array.h"

#include <mruby/string.h>

static void p(mrb_state *mrb, mrb_value obj, int prompt) {
  mrb_value val;
  char *msg;

  val = mrb_funcall(mrb, obj, "inspect", 0);
  if (prompt) {
    if (!mrb->exc) {
      fputs(" => ", stdout);
    } else {
      val = mrb_funcall(mrb, mrb_obj_value(mrb->exc), "inspect", 0);
    }
  }
  if (!mrb_string_p(val)) {
    val = mrb_obj_as_string(mrb, obj);
  }
  msg = mrb_locale_from_utf8(RSTRING_PTR(val), (int)RSTRING_LEN(val));
  fwrite(msg, strlen(msg), 1, stdout);
  mrb_locale_free(msg);
  putc('\n', stdout);
}

/* Guess if the user might want to enter more
 * or if he wants an evaluation of his code now */
int is_code_block_open(struct mrb_parser_state *parser) {
  int code_block_open = FALSE;

  /* check for heredoc */
  if (parser->parsing_heredoc != NULL) return TRUE;

  if (parser->lex_strterm) return TRUE;

  /* check if parser error are available */
  if (0 < parser->nerr) {
    const char *unexpected_end = "syntax error, unexpected $end";
    const char *message = parser->error_buffer[0].message;

    /* a parser error occur, we have to check if */
    /* we need to read one more line or if there is */
    /* a different issue which we have to show to */
    /* the user */

    if (strncmp(message, unexpected_end, strlen(unexpected_end)) == 0) {
      code_block_open = TRUE;
    } else if (strcmp(message, "syntax error, unexpected keyword_end") == 0) {
      code_block_open = FALSE;
    } else if (strcmp(message, "syntax error, unexpected tREGEXP_BEG") == 0) {
      code_block_open = FALSE;
    }
    return code_block_open;
  }

  /* check for unterminated string */
  if (parser->lex_strterm) return TRUE;

  switch (parser->lstate) {
      /* all states which need more code */

    case EXPR_BEG:
      /* an expression was just started, */
      /* we can't end it like this */
      code_block_open = TRUE;
      break;
    case EXPR_DOT:
      /* a message dot was the last token, */
      /* there has to come more */
      code_block_open = TRUE;
      break;
    case EXPR_CLASS:
      /* a class keyword is not enough! */
      /* we need also a name of the class */
      code_block_open = TRUE;
      break;
    case EXPR_FNAME:
      /* a method name is necessary */
      code_block_open = TRUE;
      break;
    case EXPR_VALUE:
      /* if, elsif, etc. without condition */
      code_block_open = TRUE;
      break;

      /* now all the states which are closed */

    case EXPR_ARG:
      /* an argument is the last token */
      code_block_open = FALSE;
      break;

      /* all states which are unsure */

    case EXPR_CMDARG:
      break;
    case EXPR_END:
      /* an expression was ended */
      break;
    case EXPR_ENDARG:
      /* closing parenthese */
      break;
    case EXPR_ENDFN:
      /* definition end */
      break;
    case EXPR_MID:
      /* jump keyword like break, return, ... */
      break;
    case EXPR_MAX_STATE:
      /* don't know what to do with this token */
      break;
    default:
      /* this state is unexpected! */
      break;
  }

  return code_block_open;
}

static void cleanup(mrb_state *mrb) { mrb_close(mrb); }

int main() {
  int last_char;
  char ruby_code[1024] = {0};
  char last_code_line[1024] = {0};
  int char_index;
  mrbc_context *cxt;
  struct mrb_parser_state *parser;
  mrb_state *mrb;
  mrb_value result;
  int n;
  int code_block_open = FALSE;
  int ai;
  unsigned int stack_keep = 0;

  /* new interpreter instance */
  mrb = mrb_open();
  if (mrb == NULL) {
    fputs("Invalid mrb interpreter, exiting mirb\n", stderr);
    return EXIT_FAILURE;
  }
  mrb_define_global_const(mrb, "ARGV", mrb_ary_new_capa(mrb, 0));

  cxt = mrbc_context_new(mrb);
  cxt->capture_errors = 1;

  ai = mrb_gc_arena_save(mrb);
  while (TRUE) {
    if (code_block_open) {
      printf("* ");
    } else {
      printf("> ");
    }
    char_index = 0;
    while ((last_char = getchar()) != '\n') {
      if (last_char == EOF) break;
      last_code_line[char_index++] = last_char;
    }
    if (last_char == EOF) {
      printf("\n");
      break;
    }
    last_code_line[char_index] = '\0';

    if ((strcmp(last_code_line, "quit") == 0) ||
        (strcmp(last_code_line, "exit") == 0)) {
      if (!code_block_open) {
        break;
      } else {
        /* count the quit/exit commands as strings if in a quote block */
        strcat(ruby_code, "\n");
        strcat(ruby_code, last_code_line);
      }
    } else {
      if (code_block_open) {
        strcat(ruby_code, "\n");
        strcat(ruby_code, last_code_line);
      } else {
        strcpy(ruby_code, last_code_line);
      }
    }

    /* parse code */
    parser = mrb_parser_new(mrb);
    parser->s = ruby_code;
    parser->send = ruby_code + strlen(ruby_code);
    parser->lineno = 1;
    mrb_parser_parse(parser, cxt);
    code_block_open = is_code_block_open(parser);

    if (code_block_open) {
      /* no evaluation of code */
    } else {
      if (0 < parser->nerr) {
        /* syntax error */
        printf("line %d: %s\n", parser->error_buffer[0].lineno,
               parser->error_buffer[0].message);
      } else {
        /* generate bytecode */
        struct RProc *proc = mrb_generate_code(mrb, parser);
        if (proc == NULL) {
          fputs("codegen error\n", stderr);
          mrb_parser_free(parser);
          break;
        }

        /* adjust stack length of toplevel environment */
        if (mrb->c->cibase->env) {
          struct REnv *e = mrb->c->cibase->env;
          if (e && MRB_ENV_STACK_LEN(e) < proc->body.irep->nlocals) {
            MRB_ENV_SET_STACK_LEN(e, proc->body.irep->nlocals);
          }
        }
        /* pass a proc for evaluation */
        /* evaluate the bytecode */
        result = mrb_vm_run(mrb, proc, mrb_top_self(mrb), stack_keep);
        stack_keep = proc->body.irep->nlocals;
        /* did an exception occur? */
        if (mrb->exc) {
          p(mrb, mrb_obj_value(mrb->exc), 0);
          mrb->exc = 0;
        } else {
          /* no */
          if (!mrb_respond_to(mrb, result, mrb_intern_lit(mrb, "inspect"))) {
            result = mrb_any_to_s(mrb, result);
          }
          p(mrb, result, 1);
        }
      }
      ruby_code[0] = '\0';
      last_code_line[0] = '\0';
      mrb_gc_arena_restore(mrb, ai);
    }
    mrb_parser_free(parser);
  }
  mrbc_context_free(mrb, cxt);
  mrb_close(mrb);

  return 0;
}
