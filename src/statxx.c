/*
 * Copyright (c) 2024 Frederick H. G. Wright II <fw@fwright.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* MP support header */
#include "MacportsLegacySupport.h"

#if __MPLS_LIB_SUPPORT_STAT64__ || __MPLS_LIB_SUPPORT_ATCALLS__

/* Common setup for both kinds of *stat*() calls provided here */

/*
 * Cause our own refs to always use the 32-bit-inode variants.  This
 * wouldn't work on arm64, but is known to work on all platforms where
 * our implementations are needed.  It means that the referenced function
 * names are always the "unadorned" versions, except when we explicitly
 * add a suffix.
 */

#define _DARWIN_NO_64_BIT_INODE 1

#include <sys/stat.h>

/* Make sure we have "struct sta64" */
#if !__MPLS_HAVE_STAT64
struct stat64 __DARWIN_STRUCT_STAT64;
#endif /* !__MPLS_HAVE_STAT64 */

#endif /*__MPLS_LIB_SUPPORT_STAT64__ || __MPLS_LIB_SUPPORT_ATCALLS__*/

#if __MPLS_LIB_SUPPORT_STAT64__

/*
 * This provides definitions for some 64-bit-inode function variants on 10.4.
 *
 * For the *stat() functions, this simply involves translating the result of
 * the 32-bit-inode variant.
 *
 * Providing a similar capability for directory-related functions would be
 * much more difficult, since the differently-formatted dirent struct is
 * provided via a pointer to an internal buffer, rather than one provided
 * by the caller.  Hence we don't do anything about those, for now.
 */

/* Do a field-by-field copy from ino32 stat to ino64 stat. */
/* Also provide passthrough for return value. */
static int
convert_stat(const struct stat *in, struct stat64 *out, int status)
{
  out->st_dev = in->st_dev;
  out->st_mode = in->st_mode;
  out->st_nlink = in->st_nlink;
  out->st_ino = in->st_ino;  /* Possible but unlikely overflow here */
  out->st_uid = in->st_uid;
  out->st_gid = in->st_gid;
  out->st_rdev = in->st_rdev;
  out->st_atimespec = in->st_atimespec;
  out->st_mtimespec = in->st_mtimespec;
  out->st_ctimespec = in->st_ctimespec;
  /* The ino32 stat doesn't have birthtime, so use MIN(ctime, mtime) */
  if (in->st_ctimespec.tv_sec < in->st_mtimespec.tv_sec
      || (in->st_ctimespec.tv_sec == in->st_mtimespec.tv_sec
          && in->st_ctimespec.tv_nsec < in->st_mtimespec.tv_nsec)) {
    out->st_birthtimespec = in->st_ctimespec;
  } else {
    out->st_birthtimespec = in->st_mtimespec;
  }
  out->st_size = in->st_size;
  out->st_blocks = in->st_blocks;
  out->st_blksize = in->st_blksize;
  out->st_flags = in->st_flags;
  out->st_gen = in->st_gen;
  out->st_lspare = 0;
  out->st_qspare[0] = 0;
  out->st_qspare[1] = 0;
  return status;
}

int
stat$INODE64(const char *__restrict path, struct stat64 *buf)
{
  struct stat stbuf;
  return convert_stat(&stbuf, buf, stat(path, &stbuf));
}

int
lstat$INODE64(const char *__restrict path, struct stat64 *buf)
{
  struct stat stbuf;
  return convert_stat(&stbuf, buf, lstat(path, &stbuf));
}

int
fstat$INODE64(int fildes, struct stat64 *buf)
{
  struct stat stbuf;
  return convert_stat(&stbuf, buf, fstat(fildes, &stbuf));
}

int
statx_np$INODE64(const char *__restrict path, struct stat64 *buf,
                 filesec_t fsec)
{
  struct stat stbuf;
  return convert_stat(&stbuf, buf, statx_np(path, &stbuf, fsec));
}

int
lstatx_np$INODE64(const char *__restrict path, struct stat64 *buf,
                  filesec_t fsec)
{
  struct stat stbuf;
  return convert_stat(&stbuf, buf, lstatx_np(path, &stbuf, fsec));
}

int
fstatx_np$INODE64(int fildes, struct stat64 *buf, filesec_t fsec)
{
  struct stat stbuf;
  return convert_stat(&stbuf, buf, fstatx_np(fildes, &stbuf, fsec));
}

#if __MPLS_HAVE_STAT64

int
stat64(const char *__restrict path, struct stat64 *buf)
{
  struct stat stbuf;
  return convert_stat(&stbuf, buf, stat(path, &stbuf));
}

int
lstat64(const char *__restrict path, struct stat64 *buf)
{
  struct stat stbuf;
  return convert_stat(&stbuf, buf, lstat(path, &stbuf));
}

int
fstat64(int fildes, struct stat64 *buf)
{
  struct stat stbuf;
  return convert_stat(&stbuf, buf, fstat(fildes, &stbuf));
}

int
statx64_np(const char *__restrict path, struct stat64 *buf, filesec_t fsec)
{
  struct stat stbuf;
  return convert_stat(&stbuf, buf, statx_np(path, &stbuf, fsec));
}

int
lstatx64_np(const char *__restrict path, struct stat64 *buf, filesec_t fsec)
{
  struct stat stbuf;
  return convert_stat(&stbuf, buf, lstatx_np(path, &stbuf, fsec));
}

int
fstatx64_np(int fildes, struct stat64 *buf, filesec_t fsec)
{
  struct stat stbuf;
  return convert_stat(&stbuf, buf, fstatx_np(fildes, &stbuf, fsec));
}

#endif /* __MPLS_HAVE_STAT64 */

#endif /* __MPLS_LIB_SUPPORT_STAT64__*/

#if __MPLS_LIB_SUPPORT_ATCALLS__

/*
 * Provide "at" versions of the *stat*() calls, on OS versions that don't
 * provide them natively.
 */

int stat$INODE64(const char *__restrict path, struct stat64 *buf);
int lstat$INODE64(const char *__restrict path, struct stat64 *buf);

#include "atcalls.h"

int fstatat(int fd, const char *__restrict path, struct stat *buf, int flag)
{
    ERR_ON(EINVAL, flag & ~AT_SYMLINK_NOFOLLOW);
    if (flag & AT_SYMLINK_NOFOLLOW) {
        return ATCALL(fd, path, lstat(path, buf));
    } else {
        return ATCALL(fd, path, stat(path, buf));
    }
}

int fstatat$INODE64(int fd, const char *__restrict path, struct stat64 *buf,
                    int flag)
{
    ERR_ON(EINVAL, flag & ~AT_SYMLINK_NOFOLLOW);
    if (flag & AT_SYMLINK_NOFOLLOW) {
        return ATCALL(fd, path, lstat$INODE64(path, buf));
    } else {
        return ATCALL(fd, path, stat$INODE64(path, buf));
    }
}

#if __MPLS_HAVE_STAT64

/*
 * The fstatat64 function is not expected to be accessed directly (though many
 * system libraries provide it as a convenience synonym for fstatat$INODE64),
 * so no SDK provides a prototype for it.  We do so here.
 */

extern int fstatat64(int fd, const char *__restrict path,
                     struct stat64 *buf, int flag);

int fstatat64(int fd, const char *path, struct stat64 *buf, int flag)
{
    ERR_ON(EINVAL, flag & ~AT_SYMLINK_NOFOLLOW);
    if (flag & AT_SYMLINK_NOFOLLOW) {
        return ATCALL(fd, path, lstat64(path, buf));
    } else {
        return ATCALL(fd, path, stat64(path, buf));
    }
}

#endif /* __MPLS_HAVE_STAT64 */

#endif  /* __MPLS_LIB_SUPPORT_ATCALLS__ */
