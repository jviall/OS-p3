#include "types.h"
#include "user.h"

int main(int argc, char *argv[]) {
  char *p = 0; // null
  printf(1, "%x\n", (unsigned int) *p);
  exit();
}

