#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int
main(int argc, char *argv[])
{
  mprotect((void*)0, 1) ;
  munprotect((void*)0, 1);
  printf(1, "mtest ran\n");
  exit();
}
