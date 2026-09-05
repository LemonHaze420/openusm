#pragma once

#include "variable.h"
#include "game_process.h"

struct hires_screenshot {
    struct params {
#if STANDALONE_SYSTEM
        static int &width()
        {
            static int value {};
            return value;
        }

        static int &height()
        {
            static int value {};
            return value;
        }
#else
        static inline Var<int> width{0x0095C740};
        static inline Var<int> height{0x0095C744};
#endif
    };

#if STANDALONE_SYSTEM
    static inline int flow[] {8, 9, 14};
    static inline game_process process{"hires_screenshot", flow, 3};
#else
    static inline Var<game_process> process{0x0092205C};
#endif
};
