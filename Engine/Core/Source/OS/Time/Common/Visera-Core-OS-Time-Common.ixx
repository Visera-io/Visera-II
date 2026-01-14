module;
#include <Visera-Core.hpp>
export module Visera.Core.OS.Time.Common;
#define VISERA_MODULE_NAME "Core.OS"

export namespace Visera
{
    namespace Concepts
    {
        template<typename T> concept
        Clock = requires
        {
            requires std::is_class_v<std::chrono::system_clock>;
            requires std::is_class_v<std::chrono::steady_clock>;
            requires std::is_class_v<std::chrono::high_resolution_clock>;
        };
    }
}