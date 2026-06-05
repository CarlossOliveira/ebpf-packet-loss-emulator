BPF_CC := clang
BPF_CFLAGS := -O2 -g -Wall -Wextra -Wno-unknown-pragmas -target bpf

CC := gcc

PKG_CONFIG_PATH := /usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig:$(PKG_CONFIG_PATH)

LIBBPF_CFLAGS := $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --cflags libbpf 2>/dev/null)
LIBBPF_LIBS := $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs libbpf 2>/dev/null)

CFLAGS := -Wall -Wextra -Wno-unknown-pragmas \
	-I/usr/local/include \
	$(LIBBPF_CFLAGS)

LDFLAGS := -L/usr/local/lib -L/usr/local/lib64 \
	-Wl,-rpath,/usr/local/lib \
	-Wl,-rpath,/usr/local/lib64

LDLIBS := $(LIBBPF_LIBS) -lbpf -lelf -lz -lreadline

PROJECT_ROOT := $(CURDIR)

CORE_DIR := $(PROJECT_ROOT)/src/core
CORE_INCLUDE_DIR := $(PROJECT_ROOT)/src/include
UTILS_DIR := $(PROJECT_ROOT)/src/core/utils
UTILS_INCLUDE_DIR := $(PROJECT_ROOT)/src/include/utils

BPF_DIR := $(PROJECT_ROOT)/src/bpf
BPF_UTILS_DIR := $(BPF_DIR)/utils
BPF_MODULES_DIR := $(BPF_DIR)/modules

OUTPUT_DIR := $(PROJECT_ROOT)/build
BIN_DIR := $(OUTPUT_DIR)/bin
BPF_OBJ_DIR := $(OUTPUT_DIR)/bpf/modules

APP_BIN := $(BIN_DIR)/bpf-packet-loss-emulator

BPF_SRCS := $(wildcard $(BPF_MODULES_DIR)/*.bpf.c)
BPF_OBJS := $(patsubst $(BPF_MODULES_DIR)/%.bpf.c,$(BPF_OBJ_DIR)/%.bpf.o,$(BPF_SRCS))

APP_SRCS := $(wildcard $(CORE_DIR)/*.c) $(wildcard $(UTILS_DIR)/*.c) $(PROJECT_ROOT)/src/main.c

all: check_dependencies app bpf

app:
	@echo "Compiling loader application..."
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -DAPP -DPROJECT_ROOT=\"$(PROJECT_ROOT)\" -DBPF_OBJECT_DIR=\"$(BPF_OBJ_DIR)\" \
		-I$(CORE_INCLUDE_DIR) \
		-I$(UTILS_INCLUDE_DIR) \
		$(APP_SRCS) \
		-o $(APP_BIN) \
		$(LDFLAGS) $(LDLIBS)
	@echo "Loader application compiled successfully."

bpf: $(BPF_OBJS)
	@echo "eBPF modules compiled successfully."

$(BPF_OBJ_DIR)/%.bpf.o: $(BPF_MODULES_DIR)/%.bpf.c
	@echo "Compiling BPF module $<..."
	@mkdir -p $(BPF_OBJ_DIR)
	$(BPF_CC) -DBPF -DPROJECT_ROOT=\"$(PROJECT_ROOT)\" -DBPF_OBJECT_DIR=\"$(BPF_OBJ_DIR)\" \
		-I$(BPF_DIR) \
		-I$(BPF_UTILS_DIR) \
		-I$(CORE_INCLUDE_DIR) \
		-I$(UTILS_INCLUDE_DIR) \
		$(BPF_CFLAGS) -c $< -o $@

check_dependencies:
	@echo "Checking dependencies..."
	@command -v clang >/dev/null || (echo "clang not found" && exit 1)
	@command -v gcc >/dev/null || (echo "gcc not found" && exit 1)
	@command -v tc >/dev/null || (echo "tc not found" && exit 1)
	@command -v clang-format >/dev/null || (echo "clang-format not found" && exit 1)
	@command -v pkg-config >/dev/null || (echo "pkg-config not found" && exit 1)
	@PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --exists libbpf || \
		(echo "libbpf pkg-config file not found" && exit 1)
	@echo "All dependencies are installed."

clean:
	rm -rf $(OUTPUT_DIR)

format:
	@find src -type f \( -name "*.c" -o -name "*.h" \) \
		! -name "vmlinux.h" \
		-exec clang-format -i {} \;
	@echo "Code formatted successfully."

.PHONY: all app bpf clean check_dependencies format