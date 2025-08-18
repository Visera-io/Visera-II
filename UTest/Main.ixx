module;
#include <Visera-Core.hpp>
export module Visera.UTest.Main;
#define VISERA_MODULE_NAME "UTest.Main"
import Visera.Core;
using namespace Visera;

export int main(int argc, char *argv[])
{
    FColor Color{1,2,3,4};
    auto LC = FLinearColor::FromPow22Color(Color);
    LOG_INFO("{}", Color.ToString());

    LOG_WARN("{}", Float(Math::PI));

    return EXIT_SUCCESS;
}