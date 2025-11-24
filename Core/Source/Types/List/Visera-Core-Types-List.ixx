module;
#include <Visera-Core.hpp>
#include <ankerl/unordered_dense.h>
export module Visera.Core.Types.List;
#define VISERA_MODULE_NAME "Core.Types"
import Visera.Core.OS.Thread;

export namespace Visera
{
    template<typename T>
    using TList     = std::list<T>;
}