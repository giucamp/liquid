
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "private_common.h"
#include "book.h"
#include <iostream>
#ifdef _WIN32
    #include <Windows.h>
#endif

namespace liquid
{
    namespace detail
    {
        thread_local bool g_silent_panic = false;

        void Expects(const char * i_topic, const Tensor & i_bool_tensor, const char * i_str_expr)
        {
            if(i_bool_tensor.GetScalarType() != ScalarType::Bool)
            {
                if(i_topic != nullptr)
                    Panic("Expects - ", i_topic, ": ", i_str_expr, " has type ", i_bool_tensor.GetScalarType());
                else
                    Panic("Expects: ", i_str_expr, " has type ", i_bool_tensor.GetScalarType());
            }

            if(!Always(i_bool_tensor))
            {
                if(i_topic != nullptr)
                    Panic("Expects - ", i_topic, ": ", i_str_expr, " is ", i_bool_tensor);
                else
                    Panic("Expects: ", i_str_expr, " is ", i_bool_tensor);
            }

            if(i_topic != nullptr)
                Book::Get().AddProposition(i_topic, i_bool_tensor, i_str_expr);
        }

        void ExpectsError(const char * i_topic, 
            const std::function<void()> & i_function,
            const char* i_expression_str, const char* i_expected_error)
        {
            if(g_silent_panic)
                Panic("ExpectsError - recursion is not allowed");

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
                Panic(i_topic, " - The expression ", i_expression_str, " was supposed to error");

            if(error != i_expected_error)
            {
                if(i_topic != nullptr)
                    Panic(i_topic, " - The expression ", i_expression_str, 
                        " was supposed to raise the error:\n", i_expected_error, 
                        "\nbut it raised:\n", error);
                else
                    Panic("The expression ", i_expression_str, 
                        " was supposed to raise the error:\n", i_expected_error, 
                        "\nbut it raised:\n", error);
            }

            if(i_topic != nullptr)
                Book::Get().AddPanicProposition(i_topic, i_expression_str, i_expected_error);
        }
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


    void Panic(const std::string & i_error)
    {
        if(!detail::g_silent_panic)
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
