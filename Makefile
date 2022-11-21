# SPDX-License-Identifier: MIT

GCC		= gcc

#SHELL		:= /bin/bash

A_FLAGS		:= -D__ASSEMBLY__

C_FLAGS		:= -g -O2
C_FLAGS		+= -m64 -march=x86-64 -mno-sse2
C_FLAGS		+= -fno-stack-protector
C_FLAGS		+= -ffreestanding
C_FLAGS		+= -Wall -Wstrict-prototypes -Wno-address-of-packed-member

LD_FLAGS	:= -m64
LD_FLAGS	+= -nostdlib
LD_FLAGS	+= -Wl,-Tsrc/start/svsm.lds -Wl,--build-id=none

TARGET_DIR	:= target
TARGET		:= $(TARGET_DIR)/x86_64-unknown-none/debug

#STATIC_LIBS	:= $(addprefix external/build/lib/, libtpm.a libplatform.a libwolfssl.a libcrt.a libm.a)
STATIC_LIBS	:= $(addprefix external/build/lib/, libcrt.a libwolfssl.a libm.a )

OBJS		:= src/start/start.o
#OBJS		+= $(TARGET)/liblinux_svsm.a


FEATURES	:= ""

## Memory layout

SVSM_GPA	:= 0x8000000000
SVSM_MEM	:= 0x20000000
LDS_FLAGS	:= -DSVSM_GPA_LDS="$(SVSM_GPA)"
LDS_FLAGS	+= -DSVSM_GPA="$(SVSM_GPA)ULL"
LDS_FLAGS	+= -DSVSM_MEM="$(SVSM_MEM)ULL"

.PHONY: all doc prereq clean superclean

all: .prereq .WAIT svsm.bin

doc: .prereq
	cargo doc --open

svsm.bin: svsm.bin.elf
	objcopy -g -O binary $< $@

svsm.bin.elf: $(OBJS) src/start/svsm.lds
	@xargo build
	$(GCC) $(LD_FLAGS) -o $@ $(OBJS) -Wl,--start-group $(TARGET)/liblinux_svsm.a $(STATIC_LIBS) -Wl,--end-group

%.a: src/*.rs src/cpu/*.rs src/mem/*.rs src/util/*.rs
	@xargo build --features $(FEATURES)

%.o: %.S src/start/svsm.h
	$(GCC) $(C_FLAGS) $(LDS_FLAGS) $(A_FLAGS) -c -o $@ $<

%.lds: %.lds.S src/start/svsm.h
	$(GCC) $(A_FLAGS) $(LDS_FLAGS) -E -P -o $@ $<

prereq: .prereq

.prereq:
	curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
	source $(HOME)/.cargo/env
	echo "source $(HOME)/.cargo/env" >> ~/.bashrc
	rustup +nightly target add x86_64-unknown-none
	rustup component add rust-src
	rustup component add llvm-tools-preview
	rustup override set nightly
	cargo install xargo
	cargo install bootimage
	touch .prereq

clean:
	@xargo clean 
	rm -f svsm.bin svsm.bin.elf $(OBJS)
	rm -rf $(TARGET_DIR)
	rm -f src/start/svsm.lds

superclean: clean
	rm -f .prereq
