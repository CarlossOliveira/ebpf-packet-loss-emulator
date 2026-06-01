EBPF_CC := clang
EBPF_CFLAGS := -O2 -g -Wall -Wextra -Wno-unknown-pragmas -target bpf

CC := gcc
CFLAGS := -Wall -Wextra -Wno-unknown-pragmas

SRC_DIR := src
INCLUDE_DIR := $(SRC_DIR)/include
CORE_DIR := $(SRC_DIR)/core
UTILS_DIR := $(SRC_DIR)/utils
EBPF_SRC := $(SRC_DIR)/bpf

OUT_DIR := bin
APP_BIN := $(OUT_DIR)/ebpf-packet-loss-emulator
BPF_MODULES_DIR := modules

EBPF_SRCS := $(wildcard $(EBPF_SRC)/*.bpf.c)
EBPF_OBJS := $(patsubst $(EBPF_SRC)/%.bpf.c,$(BPF_MODULES_DIR)/%.bpf.o,$(EBPF_SRCS))

APP_SRCS := \
	$(SRC_DIR)/main.c \
	$(CORE_DIR)/setup.c \
	$(CORE_DIR)/cleanup.c \
	$(UTILS_DIR)/utils.c

all: check_dependencies app ebpf

app:
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -DAPP -I$(INCLUDE_DIR) $(APP_SRCS) -o $(APP_BIN) -lpthread

ebpf: $(EBPF_OBJS)

$(BPF_MODULES_DIR)/%.bpf.o: $(EBPF_SRC)/%.bpf.c
	@mkdir -p $(BPF_MODULES_DIR)
	$(EBPF_CC) -DBPF -I$(INCLUDE_DIR) $(EBPF_CFLAGS) -c $< -o $@

check_dependencies:
	@echo "Checking dependencies..."
	@command -v clang >/dev/null || (echo "clang not found" && exit 1)
	@command -v gcc >/dev/null || (echo "gcc not found" && exit 1)
	@command -v tc >/dev/null || (echo "tc not found" && exit 1)
	@echo "All dependencies are installed."

clean:
	rm -rf $(OUT_DIR) $(BPF_MODULES_DIR)

.PHONY: all app ebpf clean check_dependencies