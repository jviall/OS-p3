#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int
main(int argc, char *argv[])
{
  mprotect((void*)0x1001, 1) ;
  munprotect((void*)0x1000, 1);
  printf(1, "mtest ran\n");
  exit();
}
