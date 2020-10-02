
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
    namespace
    {
        thread_local bool g_silent_panic = false;
    }

    std::ostream& operator << (std::ostream& i_ostream, ScalarType i_scalar_type)
    {
        switch (i_scalar_type)
        {
            case ScalarType::Any: i_ostream << "any"; break;
            case ScalarType::Real: i_ostream << "real"; break;
            case ScalarType::Integer: i_ostream << "int"; break;
            case ScalarType::Bool: i_ostream << "bool"; break;
            default: Panic("Unrecognized scalar type ", static_cast<int>(i_scalar_type));
        }
        return i_ostream;
    }

    void ExpectsError(const std::function<void()> & i_function,
        const char * i_expression_str, const char* i_expected_error)
    {
        std::string error;
        bool got_error = false;
        try
        {
            g_silent_panic = true;
            i_function();
            g_silent_panic = false;
        }
        catch (const std::exception & i_exc)
        {
            g_silent_panic = false;
            error = i_exc.what();
            got_error = true;
        }
        catch (...)
        {
            g_silent_panic = false;
        }

        if(!got_error)
            Panic("The expression ", i_expression_str, " was supponed to error");

        if(error != i_expected_error)
            Panic("The expression ", i_expression_str, " was supponed to raise the error:\n", 
                i_expected_error, "\nbug it raised:\n", error);
    }

    void Panic(const std::string & i_error)
    {
        if(!g_silent_panic)
        {
            std::cerr << i_error << std::endl;

            #ifdef _WIN32
                if(IsDebuggerPresent())
                    DebugBreak();
            #endif
        }
        throw std::exception(i_error.c_str());
    }
}
