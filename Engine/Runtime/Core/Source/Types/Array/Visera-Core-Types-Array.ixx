module;
#include <Visera-Core.hpp>
#include <vector>
export module Visera.Core.Types.Array;
#define VISERA_MODULE_NAME "Core.Types"
import Visera.Core.OS.Memory;

export namespace Visera
{
    template<typename T>
    using TArray    = std::vector<T>;
    template<typename T>
    using TPMRArray = std::pmr::vector<T>;
}