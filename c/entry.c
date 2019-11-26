#include <mruby.h>
#include <mruby/compile.h>

#include "ckb_consts.h"
#include "molecule/blockchain.h"

extern int ckb_load_script(void *addr, uint64_t *len, size_t offset);

#define ERROR_LOADING_SCRIPT -1

#define MAX_WITNESS_SIZE 32768
#define SCRIPT_SIZE 32768

int main(int argc, char *argv[]) {
  unsigned char script[SCRIPT_SIZE];
  uint64_t len = SCRIPT_SIZE;
  int ret = ckb_load_script(script, &len, 0);
  if (ret != CKB_SUCCESS) {
    return ERROR_LOADING_SCRIPT;
  }
  if (len > SCRIPT_SIZE) {
    return ERROR_LOADING_SCRIPT;
  }
  mol_seg_t script_seg;
  script_seg.ptr = (uint8_t *)script;
  script_seg.size = len;
  if (MolReader_Script_verify(&script_seg, false) != MOL_OK) {
    return ERROR_LOADING_SCRIPT;
  }
  mol_seg_t args_seg = MolReader_Script_get_args(&script_seg);
  mol_seg_t args_bytes_seg = MolReader_Bytes_raw_bytes(&args_seg);
  if (args_bytes_seg.size > SCRIPT_SIZE) {
    return ERROR_LOADING_SCRIPT;
  }

  mrb_state *mrb = mrb_open();
  mrb_value v = mrb_load_nstring(mrb, (const char *)args_bytes_seg.ptr,
                                 args_bytes_seg.size);

  if (mrb->exc) {
#ifndef MRB_DISABLE_STDIO
    if (mrb_undef_p(v)) {
      mrb_p(mrb, mrb_obj_value(mrb->exc));
    } else {
      mrb_print_error(mrb);
    }
#else
    mrb_value str = mrb_inspect(mrb, mrb_obj_value(mrb->exc));
    ckb_debug(mrb_string_value_cstr(mrb, &str));
#endif
    return -2;
  }

  return 0;
}
