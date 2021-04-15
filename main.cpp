
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <exception>
#include <stdexcept>
#include <iomanip>

/*
* todo:
*   - format specifiers
*       + compile time checking?
            ~ how does __attribute__((format(printf, ...))) do it?
            ~ possible, but ugly implementation: https://codereview.stackexchange.com/q/84768
            ~ just how far can this be driven?
*
*
*   - indexed arguments: - i.e., `format(..., "%{1}.4s %{0}s", "world", "hello");` would
*     result in "hell world" (use of %.4s turns "hello" into "hell" :-))[1]
*       + Qt does something like this, but checking is runtime-bound. boo! awful.
*       + C# does this too, but like Qt, checking happens during runtime. however, C# also
*         has string interpolation.
*       + there are various implementations of type-safe *printf libraries for C++, but:
*           1) they're either unreasonably bloated, or
*           2) are either not idiomatic C++, or invent their own stupid style (like using PascalCase. yuck!)
*           3) use unnecessary buildsystem cruft, rendering them difficult to integrate
*
*   - use standard headers ONLY. no special machine-level floating point shit, or such.
*
*
*
* [1]:
* int main()
* {
*     const char* a = "hello";
*     const char* b = "world";
*     printf("%.4s %s\n", a, b);
*     return 0;
* }
*
*/

namespace sfprintf
{
    template<typename CharT>
    inline void quoteString(std::basic_ostream<CharT>& out, const std::basic_string<CharT>& str)
    {
        int cd;
        size_t i;
        out << CharT('"');
        for(i=0; i<str.length(); i++)
        {
            cd = str[i];
            switch(cd)
            {
                case '\n': out << CharT('\\') << CharT('n'); break;
                case '\r': out << CharT('\\') << CharT('r'); break;
                case '\t': out << CharT('\\') << CharT('t'); break;
                case '\0': out << CharT('\\') << CharT('0'); break;
                default:
                    {
                        if((cd > 31) && (cd < 127))
                        {
                            out << CharT(cd);
                        }
                        else
                        {
                            out << CharT('0') << CharT('x') << std::hex << int(cd);
                        }
                    }
                    break;
            }
        }
        out << CharT('"');
    }

    template<typename CharT>
    inline std::basic_string<CharT> quoteString(const std::basic_string<CharT>& s)
    {
        std::basic_string<CharT> os;
        quoteString(os, s);
        return os;
    }


    template<typename CharT>
    inline constexpr std::basic_ostream<CharT>& format(std::basic_ostream<CharT>& out, std::basic_string_view<CharT> format)
    {
        out << format;
        return out;
    }
     
    template<typename CharT, typename Type, typename... Targs>
    constexpr std::basic_ostream<CharT>& format(
        /* output stream */
        std::basic_ostream<CharT>& out,
        /* format string */
        std::basic_string_view<CharT> fmt,
        /* the first typed argumnt */
        Type value,
        /* the rest of the arguments*/
        Targs... Fargs
    )
    {
        size_t i;
        int cd;
        for (i=0; i<fmt.length(); i++)
        {
            if(fmt[i] == '%')
            {
                cd = fmt[i + 1];
                switch(cd)
                {
                    case '%':
                        out << CharT('%');
                        break;
                    case 's':
                    case 'd':
                    case 'l':
                    case 'c':
                        {
                            out << value;
                        }
                        break;
                    case 'q':
                        {
                            std::basic_stringstream<CharT> tmp;
                            tmp << value;
                            quoteString(out, tmp.str());
                        }
                        break;
                    default:
                        {
                            throw std::runtime_error("invalid format character");
                        }
                        break;
                }
                auto subst = fmt.substr(i+2, fmt.length());
                return format(out, subst, Fargs...);
            }
            out << fmt[i];
        }
        return out;
    }

    template<typename CharT, size_t len, typename... ArgsT>
    inline std::basic_ostream<CharT>& format(
        std::basic_ostream<CharT>& out, const char(&fmt)[len], ArgsT&&... args)
    {
        return format(out, std::basic_string_view<CharT>(fmt, len), args...);
    }

}
 
int main()
{
    std::stringstream buf;
    try
    {
        sfprintf::mysprintf(buf, "%s world%c %d\n", "Hello", '!', 123);
        auto s = buf.str();
        std::cout << s << "(len=" << s.length() << ")" << std::endl;
    }
    catch(std::runtime_error& e)
    {
        std::cerr << "runtime_error: " << e.what() << std::endl;
    }
    return 0;
}
