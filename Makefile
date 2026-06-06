BPF_CC := clang
CC := gcc

ROOT := $(CURDIR)
SRC := $(ROOT)/src
BUILD := $(ROOT)/build

BIN := $(BUILD)/bin/bpf-packet-loss-emulator
BPF_OBJ_DIR := $(BUILD)/bpf/modules

CORE := $(SRC)/core
BPF_MOD := $(SRC)/bpf/modules

PKG_CONFIG_PATH := /usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig$(if $(PKG_CONFIG_PATH),:$(PKG_CONFIG_PATH))
export PKG_CONFIG_PATH

LIBBPF_CFLAGS := $(shell pkg-config --cflags libbpf 2>/dev/null)
LIBBPF_LIBS := $(shell pkg-config --libs libbpf 2>/dev/null)

INCLUDES := \
	-I$(SRC)/include \
	-I$(SRC)/include/utils

APP_SRCS := \
	$(wildcard $(CORE)/*.c) \
	$(wildcard $(CORE)/utils/*.c) \
	$(SRC)/main.c

BPF_SRCS := $(wildcard $(BPF_MOD)/*.bpf.c)
BPF_OBJS := $(patsubst $(BPF_MOD)/%.bpf.c,$(BPF_OBJ_DIR)/%.bpf.o,$(BPF_SRCS))

COMMON_FLAGS := -Wall -Wextra -Wno-unknown-pragmas

CFLAGS := $(COMMON_FLAGS) $(INCLUDES) $(LIBBPF_CFLAGS)
BPF_CFLAGS := -O2 -g $(COMMON_FLAGS) -target bpf \
	-I$(SRC)/bpf \
	-I$(SRC)/bpf/utils \
	$(INCLUDES)

DEFS := \
	-DPROJECT_ROOT=\"$(ROOT)\" \
	-DBPF_OBJECT_DIR=\"$(BPF_OBJ_DIR)\"

LDFLAGS := \
	-L/usr/local/lib \
	-L/usr/local/lib64 \
	-Wl,-rpath,/usr/local/lib \
	-Wl,-rpath,/usr/local/lib64

LDLIBS := $(LIBBPF_LIBS) -lelf -lz -lreadline

DEPS_SCRIPT := $(ROOT)/scripts/dependencie_manager.sh

all: check app bpf

app: $(BIN)

$(BIN): $(APP_SRCS)
	@echo "Building application..."
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -DAPP $(DEFS) $^ -o $@ $(LDFLAGS) $(LDLIBS)
	@echo "Application compiled successfully."

bpf: $(BPF_OBJS)
	@echo "All BPF modules compiled successfully."

$(BPF_OBJ_DIR)/%.bpf.o: $(BPF_MOD)/%.bpf.c
	@echo "Compiling BPF module: $<"
	@mkdir -p $(@D)
	$(BPF_CC) $(BPF_CFLAGS) -DBPF $(DEFS) -c $< -o $@

check:
	@echo "Checking dependencies..."
	@for tool in clang gcc tc pkg-config; do \
		command -v $$tool >/dev/null || { echo "$$tool not found"; exit 1; }; \
	done
	@pkg-config --exists libbpf || { echo "libbpf not found. Run 'make install' before building."; exit 1; }
	@echo "All dependencies available."

install uninstall:
	@echo "$@ing dependencies..."
	@chmod +x $(DEPS_SCRIPT)
	$(DEPS_SCRIPT) $@
	@echo "All dependencies $@ed successfully."

clean:
	rm -rf $(BUILD)

format:
	@echo "Formatting code..."
	@command -v clang-format >/dev/null || { echo "clang-format not found"; exit 1; }
	@find src -type f \( -name "*.c" -o -name "*.h" \) ! -name "vmlinux.h" -exec clang-format -i {} \;
	@echo "Code formatted successfully."

.PHONY: all app bpf check install uninstall clean format
