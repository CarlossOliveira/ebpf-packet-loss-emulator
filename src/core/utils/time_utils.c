#include "time_utils.h"

#include <stddef.h> // size_t
#include <time.h>   // time_t, time, struct tm, localtime_r, strftime

char *get_current_time(char *buffer, size_t size) {
  if (!buffer || size == 0)
    return NULL;

  time_t now = time(NULL);
  struct tm t;
  localtime_r(&now, &t);
  strftime(buffer, size, "%Y-%m-%d %H:%M:%S", &t);
  return buffer;
}