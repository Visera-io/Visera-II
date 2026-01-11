module;
#include <Visera-Core.hpp>
#include <ankerl/unordered_dense.h>
export module Visera.Core.Types.Set;
#define VISERA_MODULE_NAME "Core.Types"

export namespace Visera
{
    template<typename T>
    using TSet      = ankerl::unordered_dense::set<T>;
}