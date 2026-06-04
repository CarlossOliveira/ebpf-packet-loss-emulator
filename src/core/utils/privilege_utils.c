#include "privilege_utils.h"

#include "io_utils.h" // print()

#include <unistd.h> // geteuid

int require_root(void) {
  if (geteuid() != 0) {
    print(ERROR, "This program must be run as root.");
    return 1;
  }
  return 0;
}