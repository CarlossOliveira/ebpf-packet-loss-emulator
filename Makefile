BPF_CC := clang
BPF_CFLAGS := -O2 -g -Wall -Wextra -Wno-unknown-pragmas -target bpf

CC := gcc
CFLAGS := -Wall -Wextra -Wno-unknown-pragmas
LIBS := -lbpf -lelf -lreadline

SRC_DIR := src
INCLUDE_DIR := $(SRC_DIR)/include
CORE_DIR := $(SRC_DIR)/core
UTILS_DIR := $(SRC_DIR)/utils
BPF_SRC := $(SRC_DIR)/bpf_modules

OUT_DIR := build
BIN_DIR := $(OUT_DIR)/bin
BPF_MODULES_DIR := $(OUT_DIR)/bpf/modules
APP_BIN := $(BIN_DIR)/bpf-packet-loss-emulator

BPF_SRCS := $(wildcard $(BPF_SRC)/*.bpf.c)
BPF_OBJS := $(patsubst $(BPF_SRC)/%.bpf.c,$(BPF_MODULES_DIR)/%.bpf.o,$(BPF_SRCS))

APP_SRCS := \
	$(SRC_DIR)/main.c \
	$(CORE_DIR)/globals.c \
	$(CORE_DIR)/setup.c \
	$(CORE_DIR)/cleanup.c \
	$(UTILS_DIR)/utils.c

all: check_dependencies app bpf

app:
	@echo "Compiling loader application..."
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -DAPP -I$(INCLUDE_DIR) $(APP_SRCS) -o $(APP_BIN) $(LIBS)
	@echo "Loader application compiled successfully."

bpf: $(BPF_OBJS)
	@echo "eBPF modules compiled successfully."

$(BPF_MODULES_DIR)/%.bpf.o: $(BPF_SRC)/%.bpf.c
	@echo "Compiling BPF module $<..."
	@mkdir -p $(BPF_MODULES_DIR)
	$(BPF_CC) -DBPF -I$(INCLUDE_DIR) $(BPF_CFLAGS) -c $< -o $@

check_dependencies:
	@echo "Checking dependencies..."
	@command -v clang >/dev/null || (echo "clang not found" && exit 1)
	@command -v gcc >/dev/null || (echo "gcc not found" && exit 1)
	@command -v tc >/dev/null || (echo "tc not found" && exit 1)
	@echo "All dependencies are installed."

clean:
	rm -rf $(OUT_DIR)

.PHONY: all app bpf clean check_dependencies