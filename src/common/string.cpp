#include "common/string.hpp"

#include <cstdint>

namespace Lexvi {
    bool cstring_view::operator==(const cstring_view& other) const {
        if (m_str == other.m_str) return true;
    
        if (!m_str || !other.m_str) return false;
    
        uint64_t idx = 0;
    
        while (m_str[idx] != '\0' && other.m_str[idx] != '\0') {
            if (m_str[idx] != other.m_str[idx])
                return false;
    
            ++idx;
        }
    
        return m_str[idx] == other.m_str[idx];
    }
}
