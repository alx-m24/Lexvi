#pragma once

namespace Lexvi {
    // Non-owning view on const char* strings
    // Should be preferred over raw const char*
    class cstring_view {
        private:
            const char* m_str{};

        public:
            cstring_view() = default;
            explicit cstring_view(const char* str) : m_str(str) {}

        public:
            const char* data() const {
                return m_str;
            }

        public:
            explicit operator bool() const {
                return m_str != nullptr;
            }

            bool operator==(const cstring_view& other) const;
    };
}
