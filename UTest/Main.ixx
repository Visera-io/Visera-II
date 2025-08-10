module;
#include <Visera-Core.hpp>
export module Visera.Core.UTest.Main;

import Visera.Core;
using namespace Visera;

export int main(int argc, char *argv[])
{
    std::cout << "Hello World!\n";

    UInt64 Value = CityHash64("Hello");
    std::cout << Value << "\n";
    auto k = Memory::Malloc(2, 0);
    Memory::Free(k, 0);

    OS::Sleep(100);
    return EXIT_SUCCESS;
}