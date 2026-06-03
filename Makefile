BPF_CC := clang
BPF_CFLAGS := -O2 -g -Wall -Wextra -Wno-unknown-pragmas -target bpf

CC := gcc
CFLAGS := -Wall -Wextra -Wno-unknown-pragmas
LIBS := -lbpf -lelf -lreadline

SRC_DIR := src
INCLUDE_DIR := $(SRC_DIR)/include
CORE_DIR := $(SRC_DIR)/core
UTILS_DIR := $(SRC_DIR)/utils
BPF_SRC := $(SRC_DIR)/bpf

OUT_DIR := bin
APP_BIN := $(OUT_DIR)/bpf-packet-loss-emulator
BPF_MODULES_DIR := modules

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
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -DAPP -I$(INCLUDE_DIR) $(APP_SRCS) -o $(APP_BIN) $(LIBS)

bpf: $(BPF_OBJS)

$(BPF_MODULES_DIR)/%.bpf.o: $(BPF_SRC)/%.bpf.c
	@mkdir -p $(BPF_MODULES_DIR)
	$(BPF_CC) -DBPF -I$(INCLUDE_DIR) $(BPF_CFLAGS) -c $< -o $@

check_dependencies:
	@echo "Checking dependencies..."
	@command -v clang >/dev/null || (echo "clang not found" && exit 1)
	@command -v gcc >/dev/null || (echo "gcc not found" && exit 1)
	@command -v tc >/dev/null || (echo "tc not found" && exit 1)
	@echo "All dependencies are installed."

clean:
	rm -rf $(OUT_DIR) $(BPF_MODULES_DIR)

.PHONY: all app bpf clean check_dependencies