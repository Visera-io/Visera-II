module;
#include <Visera-Core.hpp>
export module Visera.UTests.Main;
#define VISERA_MODULE_NAME "UTests.Main"
import Visera.Core;
import Visera.Core.Media.Image.Wrappers;
using namespace Visera;

export int main(int argc, char *argv[])
{
    FPNGImageWrapper PNG;
    //PNG.Parse("test_image.png");
    LOG_INFO("{}", FName(TEXT("時雨の町")));

    return EXIT_SUCCESS;
}