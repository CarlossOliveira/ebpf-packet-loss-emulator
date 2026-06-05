BPF_CC := clang
CC := gcc

PROJECT_ROOT := $(CURDIR)

SRC_DIR := $(PROJECT_ROOT)/src
CORE_DIR := $(SRC_DIR)/core
UTILS_DIR := $(CORE_DIR)/utils

INCLUDE_DIR := $(SRC_DIR)/include
UTILS_INCLUDE_DIR := $(INCLUDE_DIR)/utils

BPF_DIR := $(SRC_DIR)/bpf
BPF_UTILS_DIR := $(BPF_DIR)/utils
BPF_MODULES_DIR := $(BPF_DIR)/modules

BUILD_DIR := $(PROJECT_ROOT)/build
BIN_DIR := $(BUILD_DIR)/bin
BPF_OBJ_DIR := $(BUILD_DIR)/bpf/modules

APP_BIN := $(BIN_DIR)/bpf-packet-loss-emulator

APP_SRCS := $(wildcard $(CORE_DIR)/*.c) \
            $(wildcard $(UTILS_DIR)/*.c) \
            $(SRC_DIR)/main.c

BPF_SRCS := $(wildcard $(BPF_MODULES_DIR)/*.bpf.c)
BPF_OBJS := $(patsubst $(BPF_MODULES_DIR)/%.bpf.c,$(BPF_OBJ_DIR)/%.bpf.o,$(BPF_SRCS))

PKG_CONFIG_PATH := /usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig:$(PKG_CONFIG_PATH)
export PKG_CONFIG_PATH

LIBBPF_CFLAGS := $(shell PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig pkg-config --cflags libbpf 2>/dev/null)
LIBBPF_LIBS := $(shell PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig pkg-config --libs libbpf 2>/dev/null)

CFLAGS := -Wall -Wextra -Wno-unknown-pragmas \
          -I$(INCLUDE_DIR) \
          -I$(UTILS_INCLUDE_DIR) \
          $(LIBBPF_CFLAGS)

LDFLAGS := -L/usr/local/lib -L/usr/local/lib64 \
           -Wl,-rpath,/usr/local/lib \
           -Wl,-rpath,/usr/local/lib64

LDLIBS := $(LIBBPF_LIBS) -lelf -lz -lreadline

BPF_CFLAGS := -O2 -g -Wall -Wextra -Wno-unknown-pragmas -target bpf \
              -I$(BPF_DIR) \
              -I$(BPF_UTILS_DIR) \
              -I$(INCLUDE_DIR) \
              -I$(UTILS_INCLUDE_DIR)

DEFS := -DPROJECT_ROOT=\"$(PROJECT_ROOT)\" \
        -DBPF_OBJECT_DIR=\"$(BPF_OBJ_DIR)\"

all: check app bpf

app: $(APP_BIN)

$(APP_BIN): $(APP_SRCS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -DAPP $(DEFS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

bpf: $(BPF_OBJS)

$(BPF_OBJ_DIR)/%.bpf.o: $(BPF_MODULES_DIR)/%.bpf.c
	@mkdir -p $(BPF_OBJ_DIR)
	$(BPF_CC) $(BPF_CFLAGS) -DBPF $(DEFS) -c $< -o $@

check:
	@command -v clang >/dev/null || { echo "clang not found"; exit 1; }
	@command -v gcc >/dev/null || { echo "gcc not found"; exit 1; }
	@command -v tc >/dev/null || { echo "tc not found"; exit 1; }
	@command -v pkg-config >/dev/null || { echo "pkg-config not found"; exit 1; }
	@PKG_CONFIG_PATH="$(PKG_CONFIG_PATH)" pkg-config --exists libbpf || { echo "libbpf not found"; exit 1; }

clean:
	rm -rf $(BUILD_DIR)

format:
	@command -v clang-format >/dev/null || { echo "clang-format not found"; exit 1; }
	@find src -type f \( -name "*.c" -o -name "*.h" \) ! -name "vmlinux.h" -exec clang-format -i {} \;
	@echo "Code formatted."

.PHONY: all app bpf check clean format
