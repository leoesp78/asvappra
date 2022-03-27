#ifndef __GOBJECTTRACE_H__
#define __GOBJECTTRACE_H__

#include "../gio/config.h"

#ifndef SIZEOF_CHAR
#error "config.h must be included prior to gobject_trace.h"
#endif

#ifdef HAVE_DTRACE
#include "gobject_probes.h"
#define TRACE(probe) probe
#else
#define TRACE(probe)
#endif

#endif