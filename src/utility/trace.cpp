#include "trace.h"

trace_obj::trace_obj(std::string func_name) : m_func_name(func_name)
{
    std::cout << std::string(m_indent, '\t') << "--> " << m_func_name;
    ++m_indent;
}

trace_obj::~trace_obj()
{
    --m_indent;
    std::cout << std::string(m_indent, '\t') << "<-- " << m_func_name << '\n';
}
