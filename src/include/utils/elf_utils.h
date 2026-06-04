#ifndef ELF_UTILS_H
#define ELF_UTILS_H

#include <libelf.h>

Elf *open_elf_file(const char *filename, Elf_Cmd cmd);
int close_elf_file(Elf *elf);
Elf_Scn *goto_elf_section(Elf *elf, const char *section_name);
Elf_Data *get_elf_section_data(Elf_Scn *scn);
void *read_elf_section(const char *filename, const char *section_name, size_t *size);

#endif // !ELF_UTILS_H