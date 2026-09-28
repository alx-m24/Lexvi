#pragma once

#include <cstdint>

#include "common/types/string.hpp"

namespace Lexvi::Types {
    enum class Status : uint8_t {
        OK = 0,
        ERROR
    };

    class Result {
        private:
            Status status{};
            cstring_view error{}; 

        public:
            Result() : status(Status::OK) {}
            Result(const cstring_view& err) : status(Status::ERROR), error(err) {}

            cstring_view getError() const {
                return error;
            }

            bool IsError() const {
                return status != Status::OK;
            }
    };
}
