#include <vector>
#include "gtest-typed-test_test.h"
#include "gtest.h"

#if GTEST_HAS_TYPED_TEST_P
INSTANTIATE_TYPED_TEST_CASE_P(Vector, ContainerTest,testing::Types<std::vector<int> >);
#endif
