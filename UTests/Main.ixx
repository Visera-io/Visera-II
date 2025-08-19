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
    PNG.Parse("D:\\VSpace\\Visera-Core\\cmake-build-debug-visual-studio\\Debug\\"
              "test_image.png");

    return EXIT_SUCCESS;
}