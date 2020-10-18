
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "miu6/lexer.h"
#include <cctype>
#include <cstdlib>
#include <cinttypes>

namespace liquid
{
    namespace miu6
    {
        namespace
        {
            template <typename PREDICATE>
                size_t CountIf(std::string_view i_source, const PREDICATE & i_predicate)
            {
                for(size_t i = 0; i < i_source.length(); i++)
                    if(!i_predicate(i_source[i]))
                        return i;

                return i_source.length();
            }

            bool IsUtf8CodeUnit(char i_char)
            {
                return static_cast<unsigned char>(i_char) >= 0x80;
            }

            bool IsSpace(char i_char)
            {
                return !IsUtf8CodeUnit(i_char) && std::isspace(i_char);
            }

            bool IsDigit(char i_char)
            {
                return !IsUtf8CodeUnit(i_char) && std::isdigit(i_char);
            }

            bool IsAlpha(char i_char)
            {
                return IsUtf8CodeUnit(i_char) || std::isalpha(i_char);
            }

            bool IsAlphaNum(char i_char)
            {
                return IsUtf8CodeUnit(i_char) || std::isalnum(i_char);
            }
        }

        std::string_view Lexer::TryParseSpaces(std::string_view & io_source)
        {
            const char * beginning = io_source.data();
            while(!io_source.empty() && IsSpace(io_source[0]))
                io_source = io_source.substr(1);
            return std::string_view(beginning, io_source.data() - beginning);
        }

        /* returns true if two sequences of spaces are simmetrical, that is 
            if i_prefix == reverse(i_postfix). The sequence \r\n is considered
            a single space, and must not be reversed in the postix. */
        bool Lexer::WhiteSimmetry(std::string_view i_prefix, std::string_view i_postfix)
        {
            if(i_prefix.length() != i_postfix.length())
                return false;

            size_t const length = i_prefix.length();
            for(size_t i = 0; i < length; i++)
            {
                if(i_prefix[i] == '\r' && i + 1 < length &&
                    i_prefix[i] == '\n')
                {
                    if(i_postfix[length - 2 - i] != '\r' ||
                       i_postfix[length - 1 - i] != '\n')
                        return false;
                    else
                        i++;
                }
                else if(i_prefix[i] != i_postfix[length - 1 - i])
                    return false;
            }
            return true;
        }

        bool Lexer::TryParseString(std::string_view & io_source, std::string_view i_what)
        {
            if (StartsWith(io_source, i_what))
            {
                io_source = io_source.substr(i_what.length());
                return true;
            }

            return false;
        }

        std::optional<Real> Lexer::TryParseReal(std::string_view & io_source)
        {
            static_assert(std::is_same_v<Real, double>, "std::strtod works with doubles");
            char * end_ptr = nullptr;
            auto const value = std::strtod(io_source.data(), &end_ptr);
            if(end_ptr == io_source.data())
                return {};
        
            io_source = io_source.substr(end_ptr - io_source.data());
            return value;
        }

        std::optional<Integer> Lexer::TryParseInteger(std::string_view & io_source)
        {
            size_t const digits = CountIf(io_source, IsDigit);
            if(digits == 0)
                return {};

            if(digits < io_source.length() &&
                (io_source[digits] == '.' || isalpha(io_source[digits])))
                    return {};

            auto const value = std::strtoimax(std::string(io_source.substr(0, digits)).c_str(), nullptr, 10);
            io_source = io_source.substr(digits);
            return NumericCast<Integer>(value);
        }

        std::optional<Bool> Lexer::TryParseBool(std::string_view & io_source)
        {
            if(TryParseString(io_source, "true"))
                return true;
            else if(TryParseString(io_source, "false"))
                return false;
            else
                return {};
        }

        std::optional<std::string_view> Lexer::TryParseName(std::string_view & io_source)
        {
            size_t const length = CountIf(io_source, IsAlphaNum);
            if(length > 0 && IsAlpha(io_source[0]))
            {
                auto const name = io_source.substr(0, length);
                io_source = io_source.substr(length);
                return name;
            }
            else
                return {};
        }

        Token Lexer::TryParseToken(std::string_view & io_source)
        {
            auto const spaces = TryParseSpaces(io_source);

            const char * first_char = io_source.data();
            Token token = TryParseTokenImpl(spaces, io_source);
            token.m_chars = { first_char, static_cast<size_t>(io_source.data() - first_char) };
            return token;
        }

        Token Lexer::TryParseTokenImpl(std::string_view i_prefix_spaces, std::string_view & io_source)
        {
            for(const Symbol & symbol : g_alphabet)
            {
                if(HasFlags(symbol.m_flags, SymbolFlags::BinaryOperator))
                {
                    // binary operator - enforce white space symmetry
                    auto new_source = io_source;
                    if(TryParseString(new_source, symbol.m_chars))
                    {
                        std::string_view postfix_spaces = TryParseSpaces(new_source);
                        if(WhiteSimmetry(i_prefix_spaces, postfix_spaces))
                        {
                            io_source = new_source;
                            return { symbol.m_id, {}, symbol.m_flags, symbol.m_precedence };
                        }
                    }                    
                }
                else
                {
                    // non-binary operator
                    if(TryParseString(io_source, symbol.m_chars))
                        return { symbol.m_id, {}, symbol.m_flags, symbol.m_precedence };
                }
            }

            // literals
            if(auto const literal = TryParseBool(io_source))
                return { SymbolId::Literal, *literal };
            else if(auto const literal = TryParseInteger(io_source))
                return { SymbolId::Literal, *literal };
            else if(auto const literal = TryParseReal(io_source))
                return { SymbolId::Literal, *literal };

            // name
            else if(auto const name = TryParseName(io_source))
                return { SymbolId::Name };

            // end
            else if(io_source.empty())
                return { SymbolId::EndOfSource };

            // unrecognized
            else
                return { SymbolId::Unrecognized };
        }

        Lexer::Lexer(std::string_view i_source)
            : m_whole_source(i_source), m_remaining_source(i_source)
        {
            m_curr_token = TryParseToken(m_remaining_source);
        }

        void Lexer::Advance()
        {
            m_curr_token = TryParseToken(m_remaining_source);
        }

        std::optional<Token> Lexer::TryAccept(SymbolId i_symbol_id)
        {
            if (m_curr_token.m_symbol_id == i_symbol_id)
            {
                auto const result = m_curr_token;
                Advance();
                return result;
            }
            else
                return {};
        }

        Token Lexer::Accept(SymbolId i_symbol_id)
        {
            auto result = TryAccept(i_symbol_id);
            if(result)
                return *result;
            Panic(*this, " Unexpected token");
        }

        namespace 
        {
            // range of source chars delimited by line-enders or source bounds
            struct Line { std::string_view m_chars; size_t m_number; };

            // finds the line containing the character pointed by i_at
            Line GetLineAt(std::string_view i_source, const char * i_at)
            {
                const char * curr_char = i_source.data();
                const char * const end_of_source = curr_char + i_source.size();

                if(i_at < curr_char || i_at > end_of_source)
                    Panic("GetLineAt - i_at is outside the source code");

                const char * beginning_of_line = curr_char;
                size_t line_number = 1;
                while(curr_char < i_at)
                {
                    if(*curr_char == '\n')
                    {
                        line_number++;
                        curr_char++;
                        beginning_of_line = curr_char;
                    }
                    else if(*curr_char == '\r' &&
                        curr_char + 1 < end_of_source &&
                        curr_char[1] == '\n')
                    {
                        line_number++;
                        curr_char += 2;
                        beginning_of_line = curr_char;
                    }
                    else
                        curr_char++;
                }

                const char * end_of_line = curr_char;
                while(end_of_line < end_of_source && *end_of_line != '\n')
                    end_of_line++;
                if(end_of_line > i_source.data() && end_of_line[-1] == '\r')
                    end_of_line--;

                return {std::string_view(beginning_of_line, end_of_line - beginning_of_line), line_number};
            }
        }

        std::ostream & operator << (std::ostream & i_dest, const Lexer & i_lexer)
        {
            const char * at = i_lexer.m_curr_token.m_chars.data();
            Line const line = GetLineAt(i_lexer.m_whole_source, at);
            
            std::string const prefix = "(" + std::to_string(line.m_number) + "): ";
            i_dest << "\n" << prefix << line.m_chars;
            i_dest << "\n" << std::string(prefix.size() + (at - line.m_chars.data()), ' ') << '^';

            return i_dest;
        }

    } // namespace miu6
}
