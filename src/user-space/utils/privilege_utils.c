#include "privilege_utils.h"

#include <unistd.h>

int require_root(void)
{
	if (geteuid() != 0) {
		return 1;
	}
	return 0;
}
