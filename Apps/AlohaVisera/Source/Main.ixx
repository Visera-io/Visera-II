module;
#include <Visera.hpp>
export module AlohaVisera;
#define VISERA_MODULE_NAME "AlohaVisera"
import Visera.Core;
import Visera.Core.Media.Image.Wrappers;
import Visera.Runtime;
import Visera.Engine;
using namespace Visera;

void Foo(TMutable<TSpan<int, 2>> I_Val)
{
    auto& view = *I_Val;
    view[0] = 2232323;
}

export int main(int argc, char *argv[])
{
    FPNGImageWrapper PNG;
    //PNG.Parse("test_image.png");

    GWindow->Bootstrap();
    GRHI->Bootstrap();

    LOG_INFO("Hello {}", FName{"Visera"});

    // GWindow->Bootstrap();
    // while (!GWindow->ShouldClose())
    // {
    //     GWindow->PollEvents(); // You MUST call this function on MacOS.
    //
    //
    // }
    //GRHI->Terminate();
    //GWindow->Terminate();

    return EXIT_SUCCESS;
}