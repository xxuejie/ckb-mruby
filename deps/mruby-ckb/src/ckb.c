#include "mruby.h"
#include "mruby/array.h"
#include "mruby/hash.h"
#include "mruby/string.h"
#include "mruby/variable.h"

#include "ckb_syscalls.h"

static void check_ret(mrb_state *mrb, int ret) {
  if (ret == CKB_INDEX_OUT_OF_BOUND) {
    struct RClass *mrb_ckb = mrb_module_get(mrb, "CKB");
    struct RClass *error_class =
        mrb_class_get_under(mrb, mrb_ckb, "IndexOutOfBound");
    mrb_raise(mrb, error_class, "Index out of bound!");
  } else if (ret == CKB_ITEM_MISSING) {
    struct RClass *mrb_ckb = mrb_module_get(mrb, "CKB");
    struct RClass *error_class =
        mrb_class_get_under(mrb, mrb_ckb, "ItemMissing");
    mrb_raise(mrb, error_class, "Item Missing!");
  }
}

static mrb_value ckb_mrb_load_tx_hash(mrb_state *mrb, mrb_value obj) {
  uint64_t len;
  mrb_value s;
  int ret;

  len = 32;
  s = mrb_str_new_capa(mrb, len);
  ret = ckb_load_tx_hash(RSTRING_PTR(s), &len, 0);
  check_ret(mrb, ret);
  if (len != 32) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "cannot load transaction hash!");
  }
  RSTR_SET_LEN(mrb_str_ptr(s), len);
  return s;
}

static mrb_value ckb_mrb_load_script_hash(mrb_state *mrb, mrb_value obj) {
  uint64_t len;
  mrb_value s;
  int ret;

  len = 32;
  s = mrb_str_new_capa(mrb, len);
  ret = ckb_load_script_hash(RSTRING_PTR(s), &len, 0);
  check_ret(mrb, ret);
  if (len != 32) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "cannot load script hash!");
  }
  RSTR_SET_LEN(mrb_str_ptr(s), len);
  return s;
}

static mrb_value ckb_mrb_debug(mrb_state *mrb, mrb_value obj) {
  mrb_value s;

  mrb_get_args(mrb, "S", &s);

  ckb_debug(mrb_string_value_cstr(mrb, &s));

  return obj;
}

static mrb_value ckb_mrb_cell_internal_read(mrb_state *mrb, mrb_value self) {
  mrb_int source, index, len, offset;
  mrb_value buf;
  uint64_t buf_len;
  int ret;

  source = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@source")));
  index = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@index")));

  mrb_get_args(mrb, "i|i", &len, &offset);

  buf_len = len;
  if (buf_len == 0) {
    ret = ckb_load_cell(NULL, &buf_len, 0, index, source);
    check_ret(mrb, ret);

    return mrb_fixnum_value(buf_len);
  } else {
    buf = mrb_str_new_capa(mrb, buf_len);
    ret = ckb_load_cell(RSTRING_PTR(buf), &buf_len, offset, index, source);
    check_ret(mrb, ret);

    RSTR_SET_LEN(mrb_str_ptr(buf), len);
    return buf;
  }
}

static mrb_value ckb_mrb_cell_data_internal_read(mrb_state *mrb,
                                                 mrb_value self) {
  mrb_int source, index, len, offset;
  mrb_value buf;
  uint64_t buf_len;
  int ret;

  source = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@source")));
  index = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@index")));

  mrb_get_args(mrb, "i|i", &len, &offset);

  buf_len = len;
  if (buf_len == 0) {
    ret = ckb_load_cell_data(NULL, &buf_len, 0, index, source);
    check_ret(mrb, ret);

    return mrb_fixnum_value(buf_len);
  } else {
    buf = mrb_str_new_capa(mrb, buf_len);
    ret = ckb_load_cell_data(RSTRING_PTR(buf), &buf_len, offset, index, source);
    check_ret(mrb, ret);

    RSTR_SET_LEN(mrb_str_ptr(buf), len);
    return buf;
  }
}

static mrb_value ckb_mrb_input_internal_read(mrb_state *mrb, mrb_value self) {
  mrb_int source, index, len, offset;
  mrb_value buf;
  uint64_t buf_len;
  int ret;

  source = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@source")));
  index = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@index")));

  mrb_get_args(mrb, "i|i", &len, &offset);

  buf_len = len;
  if (buf_len == 0) {
    ret = ckb_load_input(NULL, &buf_len, 0, index, source);
    check_ret(mrb, ret);

    return mrb_fixnum_value(buf_len);
  } else {
    buf = mrb_str_new_capa(mrb, buf_len);
    ret = ckb_load_input(RSTRING_PTR(buf), &buf_len, offset, index, source);
    check_ret(mrb, ret);

    RSTR_SET_LEN(mrb_str_ptr(buf), len);
    return buf;
  }
}

static mrb_value ckb_mrb_header_internal_read(mrb_state *mrb, mrb_value self) {
  mrb_int source, index, len, offset;
  mrb_value buf;
  uint64_t buf_len;
  int ret;

  source = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@source")));
  index = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@index")));

  mrb_get_args(mrb, "i|i", &len, &offset);

  buf_len = len;
  if (buf_len == 0) {
    ret = ckb_load_header(NULL, &buf_len, 0, index, source);
    check_ret(mrb, ret);

    return mrb_fixnum_value(buf_len);
  } else {
    buf = mrb_str_new_capa(mrb, buf_len);
    ret = ckb_load_header(RSTRING_PTR(buf), &buf_len, offset, index, source);
    check_ret(mrb, ret);

    RSTR_SET_LEN(mrb_str_ptr(buf), len);
    return buf;
  }
}

static mrb_value ckb_mrb_cell_field_internal_read(mrb_state *mrb,
                                                  mrb_value self) {
  mrb_int source, index, len, offset, field;
  mrb_value buf;
  uint64_t buf_len;
  int ret;

  source = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@source")));
  index = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@index")));
  field = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@field")));

  mrb_get_args(mrb, "i|i", &len, &offset);

  buf_len = len;
  if (buf_len == 0) {
    ret = ckb_load_cell_by_field(NULL, &buf_len, 0, index, source, field);
    check_ret(mrb, ret);

    return mrb_fixnum_value(buf_len);
  } else {
    buf = mrb_str_new_capa(mrb, buf_len);
    ret = ckb_load_cell_by_field(RSTRING_PTR(buf), &buf_len, offset, index,
                                 source, field);
    check_ret(mrb, ret);

    RSTR_SET_LEN(mrb_str_ptr(buf), len);
    return buf;
  }
}

static mrb_value ckb_mrb_input_field_internal_read(mrb_state *mrb,
                                                   mrb_value self) {
  mrb_int source, index, len, offset, field;
  mrb_value buf;
  uint64_t buf_len;
  int ret;

  source = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@source")));
  index = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@index")));
  field = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@field")));

  mrb_get_args(mrb, "i|i", &len, &offset);

  buf_len = len;
  if (buf_len == 0) {
    ret = ckb_load_input_by_field(NULL, &buf_len, 0, index, source, field);
    check_ret(mrb, ret);

    return mrb_fixnum_value(buf_len);
  } else {
    buf = mrb_str_new_capa(mrb, buf_len);
    ret = ckb_load_input_by_field(RSTRING_PTR(buf), &buf_len, offset, index,
                                  source, field);
    check_ret(mrb, ret);

    RSTR_SET_LEN(mrb_str_ptr(buf), len);
    return buf;
  }
}

void mrb_mruby_ckb_gem_init(mrb_state *mrb) {
  struct RClass *mrb_ckb, *reader, *cell, *input, *header, *cell_field,
      *input_field, *cell_data;
  mrb_ckb = mrb_define_module(mrb, "CKB");
  mrb_define_module_function(mrb, mrb_ckb, "load_tx_hash", ckb_mrb_load_tx_hash,
                             MRB_ARGS_NONE());
  mrb_define_module_function(mrb, mrb_ckb, "load_script_hash",
                             ckb_mrb_load_script_hash, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, mrb_ckb, "debug", ckb_mrb_debug,
                             MRB_ARGS_REQ(1));
  reader = mrb_define_class_under(mrb, mrb_ckb, "Reader", mrb->object_class);
  cell = mrb_define_class_under(mrb, mrb_ckb, "Cell", reader);
  mrb_define_method(mrb, cell, "internal_read", ckb_mrb_cell_internal_read,
                    MRB_ARGS_REQ(1));
  cell_data = mrb_define_class_under(mrb, mrb_ckb, "CellData", reader);
  mrb_define_method(mrb, cell_data, "internal_read",
                    ckb_mrb_cell_data_internal_read, MRB_ARGS_REQ(1));
  input = mrb_define_class_under(mrb, mrb_ckb, "Input", reader);
  mrb_define_method(mrb, input, "internal_read", ckb_mrb_input_internal_read,
                    MRB_ARGS_REQ(1));
  header = mrb_define_class_under(mrb, mrb_ckb, "Header", reader);
  mrb_define_method(mrb, header, "internal_read", ckb_mrb_header_internal_read,
                    MRB_ARGS_REQ(1));
  cell_field = mrb_define_class_under(mrb, mrb_ckb, "CellField", reader);
  mrb_define_method(mrb, cell_field, "internal_read",
                    ckb_mrb_cell_field_internal_read, MRB_ARGS_REQ(1));
  input_field = mrb_define_class_under(mrb, mrb_ckb, "InputField", reader);
  mrb_define_method(mrb, input_field, "internal_read",
                    ckb_mrb_input_field_internal_read, MRB_ARGS_REQ(1));
}

void mrb_mruby_ckb_gem_final(mrb_state *mrb) {}
