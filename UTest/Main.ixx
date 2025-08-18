module;
#include <Visera-Core.hpp>
export module Visera.Core.UTest.Main;

import Visera.Core;
using namespace Visera;

export int main(int argc, char *argv[])
{
    FColor Color{1,2,3,4};
    auto LC = FLinearColor::FromPow22Color(Color);

    return EXIT_SUCCESS;
}