BPF_CC := clang
CC := gcc

ROOT := $(CURDIR)

SRC := $(ROOT)/src

BUILD := $(ROOT)/build
BUILD_SRC := $(BUILD)/src
BUILD_DOCS := $(BUILD)/docs

USERSPACE := $(SRC)/user-space
KERNELSPACE := $(SRC)/kernel-space

BIN_DIR := $(BUILD_SRC)/bin
OBJ_DIR := $(BUILD_SRC)/user-space
BPF_OBJ_DIR := $(BUILD_SRC)/kernel-space/bpf/modules

BIN := $(BIN_DIR)/ebpf-packet-loss-emulator

PKG_CONFIG_PATH := /usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig$(if $(PKG_CONFIG_PATH),:$(PKG_CONFIG_PATH))
export PKG_CONFIG_PATH

LIBBPF_CFLAGS := $(shell pkg-config --cflags libbpf 2>/dev/null)
LIBBPF_LIBS := $(shell pkg-config --libs libbpf 2>/dev/null)

APP_SRCS := $(shell find $(USERSPACE) -type f -name "*.c")
BPF_SRCS := $(shell find $(KERNELSPACE)/bpf/modules -type f -name "*.bpf.c")

APP_OBJS := $(patsubst $(USERSPACE)/%.c,$(OBJ_DIR)/%.o,$(APP_SRCS))
BPF_OBJS := $(patsubst $(KERNELSPACE)/bpf/modules/%.bpf.c,$(BPF_OBJ_DIR)/%.bpf.o,$(BPF_SRCS))

USER_INCLUDES := \
	-I$(SRC)/shared/include \
	-I$(USERSPACE)/include \
	-I$(USERSPACE)/utils \
	-I$(USERSPACE)/core \
	-I$(USERSPACE)/cli \
	-I$(USERSPACE)/cli/commands

BPF_INCLUDES := \
	-I$(SRC)/shared/include \
	-I$(KERNELSPACE)/bpf \
	-I$(KERNELSPACE)/include \
	-I$(KERNELSPACE)/bpf/utils

APP_FLAGS := \
	-Wall \
	-Wextra \
	-Wpedantic \
	-Werror \
	-fsanitize=address \
	-fsanitize=undefined

BPF_FLAGS := \
	-Wall \
	-Wextra

CFLAGS := \
	-O2 \
	-g \
	-std=gnu17 \
	-MMD \
	-MP \
	$(APP_FLAGS) \
	$(USER_INCLUDES) \
	$(LIBBPF_CFLAGS)

BPF_CFLAGS := \
	-O2 \
	-g \
	-target bpf \
	$(BPF_FLAGS) \
	$(BPF_INCLUDES)

DEFS := \
	-DPROJECT_ROOT=\"$(ROOT)\" \
	-DBPF_OBJECT_DIR=\"$(BPF_OBJ_DIR)\"

LDFLAGS := \
	-L/usr/local/lib \
	-L/usr/local/lib64 \
	-Wl,-rpath,/usr/local/lib \
	-Wl,-rpath,/usr/local/lib64 \
	-fsanitize=address \
	-fsanitize=undefined

LDLIBS := $(LIBBPF_LIBS) -lelf -lz -lreadline

# ---------------- RULES ----------------

all: build

build: app bpf

app: $(BIN)

bpf: $(BPF_OBJS)
	@echo "All BPF modules compiled successfully."

$(BIN): $(APP_OBJS)
	@echo "Linking application..."
	@mkdir -p $(@D)
	$(CC) $^ -o $@ $(LDFLAGS) $(LDLIBS)
	@echo "Application compiled successfully."

$(OBJ_DIR)/%.o: $(USERSPACE)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(DEFS) -DAPP -c $< -o $@

$(BPF_OBJ_DIR)/%.bpf.o: $(KERNELSPACE)/bpf/modules/%.bpf.c
	@echo "Compiling BPF module: $<"
	@mkdir -p $(@D)
	$(BPF_CC) $(BPF_CFLAGS) $(DEFS) -DBPF -c $< -o $@

docs:
	@echo "Generating documentation..."
	@chmod +x $(DEPS_SCRIPT)
	$(DEPS_SCRIPT) $@
	@echo "Documentation generated successfully."

# ---------------- CLEAN ----------------

clean: clean-build clean-docs
	@echo "Cleaning project..."
	rm -rf $(BUILD)
	@echo "Project cleaned successfully."

clean-docs:
	@echo "Cleaning documentation..."
	rm -rf $(BUILD_DOCS)
	rm -rf $(ROOT)/docs/source/api
	rm -rf $(ROOT)/.docs-venv
	rm -rf $(ROOT)/docs/plugins/mkdocs/__pycache__

	@echo "Documentation cleaned successfully."

clean-build:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_SRC)
	@echo "Build artifacts cleaned successfully."

# ---------------- OTHER ----------------

install uninstall:
	@echo "$@ing dependencies..."
	@chmod +x $(DEPS_SCRIPT)
	$(DEPS_SCRIPT) $@
	@echo "Done."

format:
	@echo "Formatting code..."
	command -v clang-format >/dev/null || exit 1
	find $(SRC) -type f \( -name "*.c" -o -name "*.h" \) ! -name "vmlinux.h" -exec clang-format -i {} \;
	@echo "Code formatted successfully."

compile-commands:
	bear -- make clean all

-include $(APP_OBJS:.o=.d)

.PHONY: all build app bpf docs install uninstall clean clean-docs clean-build format compile-commands
