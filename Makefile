TARGET := riscv64-unknown-elf
CC := $(TARGET)-gcc
LD := $(TARGET)-gcc
OBJCOPY := $(TARGET)-objcopy
CFLAGS := -O3 -D__riscv_soft_float -D__riscv_float_abi_soft -DMRB_WITHOUT_FLOAT -I deps/mruby/include -I deps/ckb-c-stdlib
LDFLAGS := -Wl,-static -fdata-sections -ffunction-sections -Wl,--gc-sections

all: build/entry build/repl

build/entry: c/entry.c deps/mruby/build/riscv-without-io/lib/libmruby.a
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@
	$(OBJCOPY) --only-keep-debug $@ $@.debug
	$(OBJCOPY) --strip-debug --strip-all $@

build/repl: c/repl.c deps/mruby/build/riscv-with-io/lib/libmruby.a
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@
	$(OBJCOPY) --only-keep-debug $@ $@.debug
	$(OBJCOPY) --strip-debug --strip-all $@

deps/mruby/build/riscv-without-io/lib/libmruby.a: mruby
deps/mruby/build/riscv-with-io/lib/libmruby.a: mruby

mruby:
	cd deps/mruby && \
		MRUBY_CONFIG=../../build_config.rb make

clean-mruby:
	cd deps/mruby && \
		MRUBY_CONFIG=../../build_config.rb make clean

clean: clean-mruby
	rm -f build/entry build/entry.debug build/repl build/repl.debug

dist: clean all

.PHONY: all clean clean-mruby dist mruby
