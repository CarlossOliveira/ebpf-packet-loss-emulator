#ifndef ELF_UTILS_H
#define ELF_UTILS_H

#include <libelf.h>
#include <stdbool.h>

int open_elf_file(const char *filename);

Elf *get_elf_handle(int elf_fd, Elf_Cmd cmd);

int close_elf_handle(Elf *elf);

int close_elf_file(int elf_fd);

bool elf_has_section(int elf_fd, const char *section_name);

void *read_elf_section(const char *filename, const char *section_name,
		       size_t *size);

#endif // !ELF_UTILS_H
