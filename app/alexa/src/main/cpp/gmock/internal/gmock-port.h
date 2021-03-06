#ifndef GMOCK_INCLUDE_GMOCK_INTERNAL_GMOCK_PORT_H_
#define GMOCK_INCLUDE_GMOCK_INTERNAL_GMOCK_PORT_H_

#include <assert.h>
#include <stdlib.h>
#include <iostream>
#include "../../gtest/internal/gtest-linked_ptr.h"
#include "../../gtest/internal/gtest-port.h"
#include "custom/gmock-port.h"

#if defined(_MSC_VER) && _MSC_VER < 1310
#error "At least Visual C++ 2003 (7.1) is required to compile Google Mock."
#endif
#define GMOCK_FLAG(name) FLAGS_gmock_##name
#if !defined(GMOCK_DECLARE_bool_)
#define GMOCK_DECLARE_bool_(name) extern GTEST_API_ bool GMOCK_FLAG(name)
#define GMOCK_DECLARE_int32_(name)  extern GTEST_API_ ::testing::internal::Int32 GMOCK_FLAG(name)
#define GMOCK_DECLARE_string_(name)  extern GTEST_API_ ::std::string GMOCK_FLAG(name)
#define GMOCK_DEFINE_bool_(name, default_val, doc)  GTEST_API_ bool GMOCK_FLAG(name) = (default_val)
#define GMOCK_DEFINE_int32_(name, default_val, doc)  GTEST_API_ ::testing::internal::Int32 GMOCK_FLAG(name) = (default_val)
#define GMOCK_DEFINE_string_(name, default_val, doc)  GTEST_API_ ::std::string GMOCK_FLAG(name) = (default_val)
#endif
#endif