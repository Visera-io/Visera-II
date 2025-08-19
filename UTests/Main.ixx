module;
#include <Visera-Core.hpp>
#include <zlib.h>
export module Visera.UTests.Main;
#define VISERA_MODULE_NAME "UTests.Main"
import Visera.Core;
using namespace Visera;

export int main(int argc, char *argv[])
{
    FColor Color{1,2,3,4};
    auto LC = FLinearColor::FromPow22Color(Color);
    LOG_INFO("{}", Color.ToString());

    LOG_INFO("Zlib Version: {}", zlibVersion());
    LOG_WARN("{}", Float(Math::PI));

    return EXIT_SUCCESS;
}