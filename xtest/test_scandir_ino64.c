/*
 * Version of test_scandir with 64-bit inodes (if possible).
 *
 * This is primarily for 10.5, where 64-bit inodes are supported
 * but not the default.
 *
 * Currently, 64-bit inodes don't work on 10.4.  Although requesting
 * 64-bit inode support is ignored by the 10.4 SDK, it can work when
 * building with a 10.5+ SDK, so we explicitly disable it here.
 */

#if !defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) \
    || __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1050

#include <libgen.h>
#include <stdio.h>

int
main(int argc, char *argv[])
{
  (void) argc;
  printf("%s doesn't work on 10.4.\n", basename(argv[0]));
  return 0;
}

#else /* >= 10.5 */

#define _DARWIN_USE_64_BIT_INODE 1

#include "test_scandir.c"

#endif /* >= 10.5 */