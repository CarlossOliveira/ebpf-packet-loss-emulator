#ifndef PRIVILEGE_UTILS_H
#define PRIVILEGE_UTILS_H

/**
 * @brief Checks if the current process has root privileges.
 *
 * @return 0 if the process has root privileges, -1 otherwise.
 */
int require_root(void);

#endif // !PRIVILEGE_UTILS_H
