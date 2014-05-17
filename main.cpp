
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>

std::string stprintf(const std::string& format)
{
    return format;
}
 
template<typename Type, typename... Args>
std::string stprintf(const std::string& format, const Type& value, Args... args)
{
    size_t i;
    std::stringstream strm;
    for (i=0; i<format.size(); i++)
    {
        if(format[i] == '%')
        {
           strm << value;
           strm << stprintf(format.substr(i+1), args...);
           return strm.str();
        }
        strm << format[i];
    }
    throw std::runtime_error("too many args");
 }


int main()
{
    std::cout <<
        stprintf("Hello, %! It's %:% o'clock in %.", "doc", 12, 4, "funkytown")
    << std::endl;
}
