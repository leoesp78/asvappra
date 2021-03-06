#ifndef GMOCK_GMOCK_MORE_MATCHERS_H_
#define GMOCK_GMOCK_MORE_MATCHERS_H_

#include "gmock-generated-matchers.h"

namespace testing {
    MATCHER(IsEmpty, negation ? "isn't empty" : "is empty") {
        if (arg.empty()) return true;
        *result_listener << "whose size is " << arg.size();
        return false;
    }
}
#endif