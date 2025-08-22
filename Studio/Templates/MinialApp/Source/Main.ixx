module;
#include <Visera.hpp>
export module MinialApp;
#define VISERA_MODULE_NAME "MinialApp"
import Visera.Core;
import Visera.Engine;
using namespace Visera;

export int main(int argc, char *argv[])
{
    LOG_INFO("Hello, World!");

    return EXIT_SUCCESS;
}