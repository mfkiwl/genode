LXIP_DIR       := $(REP_DIR)/src/lib/lxip
LIBS           := jitterentropy virt_lx_emul virt_linux_generated
SHARED_LIB     := yes

TARGET_LIB_DIR := $(LXIP_DIR)
INC_DIR        += $(LXIP_DIR)/shadow $(LXIP_DIR)

SRC_CC += lx_emul/random.cc

SRC_CC += init.cc \
          socket.cc \

SRC_C += dummies.c \
         generated_dummies.c \
         lx_emul.c \
         lx_socket.c \
         lx_user.c \
         net_driver.c \

SRC_CC += genode_c_api/nic_client.cc

LD_OPT += --version-script=$(LXIP_DIR)/symbol.map

#
# Don't create jump slots for everything compiled in here since there are
# function calls from inline assembly (i.e., __sw_hweight64) with unaligned sp
# leading to GPs during fxsave on x86).
#
LD_OPT += -Bsymbolic-functions

vpath %.cc $(REP_DIR)/src/lib
vpath %.c  $(LXIP_DIR)/spec/$(SPEC_ARCH)
vpath %    $(LXIP_DIR)

C_API = $(dir $(call select_from_repositories,src/lib/genode_c_api))

vpath genode_c_api/nic_client.cc $(C_API)

# vi: set ft=make :
