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
    //auto k = Memory::MallocNow(2, 64);
    //Memory::Free(k, 0);
    FName Name{EName::None};
    Memory::Prefetch(nullptr);

    std::cout << FName::FetchNameString(Name);
    FJSON Json;
    Json.Set("Name", "LJYC");
    std::cout << Json.Get("Name");
    return EXIT_SUCCESS;
}