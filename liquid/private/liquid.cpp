
#include "liquid.h"
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