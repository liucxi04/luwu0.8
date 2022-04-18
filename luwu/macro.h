//
// Created by liucxi on 2022/4/18.
//

#ifndef LUWU_MACRO_H
#define LUWU_MACRO_H

#include "log.h"
#include "utils.h"
#include <cstring>
#include <cassert>

#define LUWU_ASSERT(x) \
    if (!(x)) {        \
        LUWU_LOG_ERROR(LUWU_LOG_ROOT()) << "ASSERTION: " #x \
                                        << "\nbacktrace:\n" \
                                        << liucxi::BacktraceToString(64, 2, "    "); \
        assert(x);                                \
    }

#define LUWU_ASSERT2(x, w) \
    if (!(x)) {        \
        LUWU_LOG_ERROR(LUWU_LOG_ROOT()) << "ASSERTION: " #x \
                                        << "\n" \
                                        << w \
                                        << "\nbacktrace:\n" \
                                        << liucxi::BacktraceToString(64, 2, "    "); \
        assert(x);                                \
    }
#endif //LUWU_MACRO_H
