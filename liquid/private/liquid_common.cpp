
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
    struct PanicException : public std::exception
    {
        using exception::exception;
    };

    namespace detail
    {
        thread_local int64_t g_silent_panic_count;
    }

    SilentPanicContext::SilentPanicContext()
    {
        detail::g_silent_panic_count++;
    }

    SilentPanicContext::~SilentPanicContext()
    {
        --detail::g_silent_panic_count;
        if(detail::g_silent_panic_count < 0)
            Panic("Internak error: g_silent_panic_count is ", detail::g_silent_panic_count);
    }

    void Expects(const char * i_topic, const Tensor & i_bool_tensor, const char * i_cpp_source_code)
    {
        auto GetMessageHeader = [i_topic, i_cpp_source_code]{
            return i_topic != nullptr ? 
                std::string("Expects - ") + i_topic + ": " + i_cpp_source_code :
                std::string("Expects - ") + i_cpp_source_code;
        };

        if(i_bool_tensor.GetScalarType() != ScalarType::Bool)
            Panic(GetMessageHeader(), " has type ", i_bool_tensor.GetScalarType(), " (should be bool)");

        if(!Always(i_bool_tensor))
            Panic(GetMessageHeader(), " is ", i_bool_tensor);

        if(i_topic != nullptr)
            Book::Get().AddProposition(i_topic, i_bool_tensor, i_cpp_source_code, nullptr);
    }

    void Expects(const char * i_topic, const char * i_miu6_source_code)
    {
        auto GetMessageHeader = [i_topic, i_miu6_source_code]{
            return i_topic != nullptr ? 
                std::string("Expects - ") + i_topic + ": " + i_miu6_source_code :
                std::string("Expects - ") + i_miu6_source_code;
        };

        Tensor bool_tensor(i_miu6_source_code);
        if(bool_tensor.GetScalarType() != ScalarType::Bool)
            Panic(GetMessageHeader(), " has type ", bool_tensor.GetScalarType(), " (should be bool)");

        if(!Always(bool_tensor))
            Panic(GetMessageHeader(), " is ", bool_tensor);

        if(i_topic != nullptr)
            Book::Get().AddProposition(i_topic, bool_tensor, nullptr, i_miu6_source_code);
    }

    void Expects(const char * i_miu6_source_code)
    {
        Expects(nullptr, i_miu6_source_code);
    }

    void ExpectsPanic(const char * i_topic, 
        const std::function<void()> & i_function,
        const char * i_cpp_source_code, const char * i_expected_message)
    {
        auto GetMessageHeader = [i_topic, i_cpp_source_code]{
            return i_topic != nullptr ? 
                std::string("ExpectsPanic - ") + i_topic + ": " + i_cpp_source_code :
                std::string("ExpectsPanic - ") + i_cpp_source_code;
        };

        std::string panic_message;
        bool got_error = false;
        try
        {
            SilentPanicContext slient_panic;
            i_function();
        }
        catch (const PanicException & i_exc)
        {
            panic_message = i_exc.what();
            got_error = true;
        }

        if(!got_error)
            Panic(GetMessageHeader(), " was supposed to panic");

        if(panic_message.find(i_expected_message) == std::string::npos)
            Panic(GetMessageHeader(), " was supposed to panic with the message:\n",
                i_expected_message, ", but it gave:\n", panic_message);

        if(i_topic != nullptr)
            Book::Get().AddPanicProposition(i_topic, i_expected_message, i_cpp_source_code, nullptr);
    }

    void ExpectsPanic(const char * i_topic, const char * i_miu6_source_code,
        const char * i_expected_message)
    {
        auto GetMessageHeader = [i_topic, i_miu6_source_code]{
            return i_topic != nullptr ? 
                std::string("ExpectsPanic - ") + i_topic + ": " + i_miu6_source_code :
                std::string("ExpectsPanic - ") + i_miu6_source_code;
        };

        std::string panic_message;
        bool got_error = false;
        try
        {
            SilentPanicContext slient_panic;
            Tensor tensor(i_miu6_source_code);
        }
        catch (const PanicException & i_exc)
        {
            panic_message = i_exc.what();
            got_error = true;
        }

        if(!got_error)
            Panic(GetMessageHeader(), " was supposed to panic");

        if(panic_message.find(i_expected_message) == std::string::npos)
            Panic(GetMessageHeader(), " was supposed to panic with the message:\n",
                i_expected_message, ", but is gave:\n", panic_message);

        if(i_topic != nullptr)
            Book::Get().AddPanicProposition(i_topic, i_expected_message, nullptr, i_miu6_source_code);
    }

    void ExpectsPanic(const char * i_miu6_source_code, const char * i_expected_message)
    {
        ExpectsPanic(nullptr, i_miu6_source_code, i_expected_message);
    }

    std::ostream & operator << (std::ostream & i_ostream, ScalarType i_scalar_type)
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
        if(detail::g_silent_panic_count <= 0)
        {
            std::cerr << i_error << std::endl;

            #ifdef _WIN32
                if(IsDebuggerPresent())
                    DebugBreak();
            #endif
        }
        throw PanicException(i_error.c_str());
    }
}
