#ifndef ELF_UTILS_H
#define ELF_UTILS_H

#include <libelf.h>
#include <stdbool.h>

/**
 * @brief Opens an ELF file and returns its file descriptor.
 *
 * @param filename The path to the ELF file to open.
 * @return The file descriptor of the opened ELF file, or -1 on failure.
 */
int open_elf_file(const char *filename);

/**
 * @brief Closes the file descriptor of an ELF file.
 *
 * @param elf_fd The file descriptor of the ELF file to close.
 * @return 0 on success, -1 on failure.
 */
int close_elf_file(int elf_fd);

/**
 * @brief Gets the ELF handle for a given file descriptor.
 *
 * @param elf_fd The file descriptor of the ELF file.
 * @param cmd The command to pass to elf_begin.
 * @return The ELF handle, or NULL on failure.
 */
Elf *get_elf_handle(int elf_fd, Elf_Cmd cmd);

/**
 * @brief Closes the ELF handle.
 *
 * @param elf The ELF handle to close.
 * @return 0 on success, -1 on failure.
 */
int close_elf_handle(Elf *elf);

/**
 * @brief Checks if an ELF file has a specific section.
 *
 * @param elf_fd The file descriptor of the ELF file.
 * @param section_name The name of the section to check for.
 * @return true if the section exists, false otherwise.
 */
bool elf_has_section(int elf_fd, const char *section_name);

/**
 * @brief Reads the data of a specific section from an ELF file.
 *
 * @param filename The path to the ELF file.
 * @param section_name The name of the section to read.
 * @param section_size A pointer to store the size of the read section.
 * @return A pointer to the allocated memory containing the section data, or NULL on failure.
 */
void *read_elf_section(const char *filename, const char *section_name, size_t *section_size);

#endif // !ELF_UTILS_H
