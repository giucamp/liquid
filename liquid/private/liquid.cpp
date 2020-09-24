
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "liquid/liquid_common.h"
#ifdef _WIN32
    #include <Windows.h>
#endif

namespace liquid
{
    void DbgBreak()
    {
        #ifdef _WIN32
            if(IsDebuggerPresent())
                DebugBreak();
        #endif
    }
}
