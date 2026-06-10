#include "elf_utils.h"

#include <fcntl.h>
#include <gelf.h>
#include <libelf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int open_elf_file(const char *filename)
{
	if (elf_version(EV_CURRENT) == EV_NONE) {
#ifdef DEBUG
		printf("elf_utils: Failed to initialize Elf library: %s\n", elf_errmsg(-1));
#endif // !DEBUG
		return -1;
	}
	if (!filename) {
		return -1;
	}

	int elf_fd = open(filename, O_RDONLY);
	if (elf_fd < 0) {
#ifdef DEBUG
		printf("elf_utils: Failed to open Elf file: %s\n", filename);
#endif // !DEBUG
		return -1;
	}

	return elf_fd;
}

int close_elf_file(int elf_fd)
{
	if (elf_fd < 0)
		return -1;

	return close(elf_fd);
}

Elf *get_elf_handle(int elf_fd, Elf_Cmd cmd)
{
	if (elf_fd < 0)
		return NULL;

	Elf *elf = elf_begin(elf_fd, cmd, NULL);
	if (!elf) {
#ifdef DEBUG
		printf("elf_utils: Failed to get Elf handle: %s\n", elf_errmsg(-1));
#endif // !DEBUG
		return NULL;
	}
	return elf;
}

int close_elf_handle(Elf *elf)
{
	if (!elf)
		return -1;

	if (elf_end(elf) != 0) {
#ifdef DEBUG
		printf("elf_utils: Failed to close Elf handle\n");
#endif // !DEBUG
		return -1;
	}
	return 0;
}

static Elf_Scn *goto_elf_section(Elf *elf, const char *section_name)
{
	Elf_Scn *scn = NULL;
	size_t shstrndx;

	if (!elf || !section_name)
		return NULL;

	if (elf_getshdrstrndx(elf, &shstrndx) != 0) {
#ifdef DEBUG
		printf("elf_utils: Failed to get section header string table "
		       "index\n");
#endif
		return NULL;
	}

	while ((scn = elf_nextscn(elf, scn)) != NULL) {
		GElf_Shdr shdr;

		if (gelf_getshdr(scn, &shdr) != &shdr) {
#ifdef DEBUG
			printf("elf_utils: Failed to get section header\n");
#endif
			return NULL;
		}

		char *name = elf_strptr(elf, shstrndx, shdr.sh_name);
		if (!name) {
#ifdef DEBUG
			printf("elf_utils: Failed to get section name\n");
#endif
			return NULL;
		}

		if (strcmp(name, section_name) == 0)
			return scn;
	}

#ifdef DEBUG
	printf("elf_utils: Section '%s' not found in Elf file\n", section_name);
#endif
	return NULL;
}

static Elf_Data *get_elf_section_data(Elf_Scn *scn)
{
	Elf_Data *data = elf_getdata(scn, NULL);
	if (!data) {
#ifdef DEBUG
		printf("elf_utils: Failed to get section data\n");
#endif // !DEBUG
		return NULL;
	}
	return data;
}

bool elf_has_section(int elf_fd, const char *section_name)
{
	Elf *elf = get_elf_handle(elf_fd, ELF_C_READ);
	if (!elf)
		return false;

	bool found = goto_elf_section(elf, section_name) != NULL;
	close_elf_handle(elf);
	return found;
}

void *read_elf_section(const char *filename, const char *section_name, size_t *section_size)
{
	if (section_size)
		*section_size = 0;

	int elf_fd = open_elf_file(filename);
	if (elf_fd < 0)
		return NULL;

	Elf *elf = get_elf_handle(elf_fd, ELF_C_READ);
	if (!elf) {
		close_elf_file(elf_fd);
		return NULL;
	}

	Elf_Scn *scn = goto_elf_section(elf, section_name);
	if (!scn) {
		close_elf_handle(elf);
		close_elf_file(elf_fd);
		return NULL;
	}

	Elf_Data *data = get_elf_section_data(scn);
	if (!data || !data->d_buf || data->d_size == 0) {
		close_elf_handle(elf);
		close_elf_file(elf_fd);
		return NULL;
	}

	void *copy = malloc(data->d_size);
	if (!copy) {
		close_elf_handle(elf);
		close_elf_file(elf_fd);
		return NULL;
	}

	memcpy(copy, data->d_buf, data->d_size);

	if (section_size)
		*section_size = data->d_size;

	close_elf_handle(elf);
	close_elf_file(elf_fd);
	return copy;
}
