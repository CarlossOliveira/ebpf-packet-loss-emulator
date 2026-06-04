#include "elf_utils.h"

#include <libelf.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h> 
#include <unistd.h>
#include <gelf.h>

Elf *open_elf_file(const char *filename, Elf_Cmd cmd) {
    if (elf_version(EV_CURRENT) == EV_NONE) {
#ifndef SILENT_ELF
      printf("elf_utils: Failed to initialize Elf library: %s\n", elf_errmsg(-1));
#endif // !SILENT_ELF
      return NULL;
    }
    if (!filename) {
      return NULL;
    }

  int fd = open(filename, O_RDONLY);
  if (fd < 0) {
#ifndef SILENT_ELF
    printf("elf_utils: Failed to open Elf file: %s\n", filename);
#endif // !SILENT_ELF
    return NULL;
  }

    return elf_begin(fd, cmd, NULL);
}

int close_elf_file(Elf *elf) {
  if (!elf)
    return -1;

  int fd = elf_getbase(elf);
  if (elf_end(elf) != 0) {
#ifndef SILENT_ELF
    printf("elf_utils: Failed to close Elf file\n");
#endif // !SILENT_ELF
  }
  return close(fd);
}

Elf_Scn *goto_elf_section(Elf *elf, const char *section_name)
{
  Elf_Scn *scn = NULL;
  size_t shstrndx;

  if (elf_getshdrstrndx(elf, &shstrndx) != 0) {
#ifndef SILENT_ELF
    printf("elf_utils: Failed to get section header string table index\n");
#endif
    return NULL;
  }

  while ((scn = elf_nextscn(elf, scn)) != NULL) {
    GElf_Shdr shdr;

    if (gelf_getshdr(scn, &shdr) != &shdr) {
#ifndef SILENT_ELF
      printf("elf_utils: Failed to get section header\n");
#endif
      return NULL;
    }

    char *name = elf_strptr(elf, shstrndx, shdr.sh_name);
    if (!name) {
#ifndef SILENT_ELF
      printf("elf_utils: Failed to get section name\n");
#endif
      return NULL;
    }

    if (strcmp(name, section_name) == 0)
      return scn;
  }

#ifndef SILENT_ELF
  printf("elf_utils: Section '%s' not found in Elf file\n", section_name);
#endif
  return NULL;
}

Elf_Data *get_elf_section_data(Elf_Scn *scn) {
  Elf_Data *data = elf_getdata(scn, NULL);
  if (!data) {
#ifndef SILENT_ELF
    printf("elf_utils: Failed to get section data\n");
#endif // !SILENT_ELF
    return NULL;
  }
    return data;
}

void *read_elf_section(const char *filename, const char *section_name, size_t *size)
{
    Elf *elf = open_elf_file(filename, ELF_C_READ);
    if (!elf)
        return NULL;

    Elf_Scn *scn = goto_elf_section(elf, section_name);
    if (!scn) {
        elf_end(elf);
        return NULL;
    }

    Elf_Data *data = get_elf_section_data(scn);
    if (!data || !data->d_buf || data->d_size == 0) {
        elf_end(elf);
        return NULL;
    }

    void *copy = malloc(data->d_size);
    if (!copy) {
        elf_end(elf);
        return NULL;
    }

    memcpy(copy, data->d_buf, data->d_size);

    if (size)
        *size = data->d_size;

    elf_end(elf);
    return copy;
}