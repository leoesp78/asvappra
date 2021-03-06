#ifndef WINCONFIG_H
#define WINCONFIG_H

#define WIN32_LEAN_AND_MEAN
//#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#include <memory.h>
#include <string.h>

#if defined(HAVE_EXPAT_CONFIG_H)
#include <expat_config.h>
#else
#define XML_NS 1
#define XML_DTD 1
#define BYTEORDER 1234
#endif
#endif