#pragma once

#include <tuple>
#include <concepts>

#include "common/types/result.hpp"

namespace Lexvi::Types {
    template<typename T>
    concept HasInit = requires(T& obj) {
        { obj.Init() } -> std::same_as<Result>;
    };

    template<typename T>
    concept HasShutdown = requires(T& obj) {
        { obj.Shutdown() } -> std::same_as<void>;
    };

    // A service is any class with an Init and Cleanup function
    template<typename T>
    concept Service_T = HasInit<T> && HasShutdown<T>;

    template<Service_T... Services_T>
    class ServiceList {
        private:
            std::tuple<Services_T...> m_services{};

        public:
            ServiceList() = default;

        public:
            template<Service_T Service>
            Result Init() {
                return std::get<Service>(m_services).Init();
            }


            template<Service_T Service>
            Service& get() {
                return std::get<Service>(m_services);
            }
            
            template<Service_T Service>
            const Service& get() const {
                return std::get<Service>(m_services);
            }

        private:
            template<std::size_t... I>
            Result InitAllImpl(std::index_sequence<I...>) {
                Result result{};
        
                (
                    [&] {
                        if (result.IsError())
                            return;
        
                        result = std::get<I>(m_services).Init();
                    }(),
                    ...
                );
        
                return result;
            }
        
        public:
            Result InitAll() {
                return InitAllImpl(std::index_sequence_for<Services_T...>{});
            }
    };
}
