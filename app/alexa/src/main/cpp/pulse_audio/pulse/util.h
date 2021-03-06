#ifndef fooutilhfoo
#define fooutilhfoo

/***
  This file is part of PulseAudio.

  Copyright 2004-2006 Lennart Poettering
  Copyright 2006 Pierre Ossman <ossman@cendio.se> for Cendio AB

  PulseAudio is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  PulseAudio is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with PulseAudio; if not, see <http://www.gnu.org/licenses/>.
***/

#include "../../../../../../../../AndroidSDK/ndk/21.3.6528147/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/c++/v1/stddef.h"
#include "cdecl.h"
#include "version.h"

/** \file
 * Assorted utility functions */

PA_C_DECL_BEGIN
typedef unsigned int size_t;
char *pa_get_user_name(char *s, size_t l);

/** Return the current hostname in the specified buffer. */
char *pa_get_host_name(char *s, size_t l);

/** Return the fully qualified domain name in s */
char *pa_get_fqdn(char *s, size_t l);

/** Return the home directory of the current user */
char *pa_get_home_dir(char *s, size_t l);

/** Return the binary file name of the current process. This is not
 * supported on all architectures, in which case NULL is returned. */
char *pa_get_binary_name(char *s, size_t l);

/** Return a pointer to the filename inside a path (which is the last
 * component). If passed NULL will return NULL. */
char *pa_path_get_filename(const char *p);

/** Wait t milliseconds */
int pa_msleep(unsigned long t);

/** Make the calling thread realtime if we can. On Linux, this uses RealTimeKit
 * if available and POSIX APIs otherwise (the latter applies to other UNIX
 * variants as well). This is also implemented for macOS and Windows.
 * \since 13.0 */
int pa_thread_make_realtime(int rtprio);

PA_C_DECL_END

#endif
