module;
#include <Visera-Core.hpp>
#include <list>
export module Visera.Core.Types.List;
#define VISERA_MODULE_NAME "Core.Types"

export namespace Visera
{
    template<typename T>
    using TList     = std::list<T>;
}