module;
#include <Visera.hpp>
export module AlohaVisera;
#define VISERA_MODULE_NAME "AlohaVisera"
import Visera.Core;
import Visera.Core.Media.Image.Wrappers;
import Visera.Runtime;
import Visera.Engine;
using namespace Visera;

export int main(int argc, char *argv[])
{
    FPNGImageWrapper PNG;
    //PNG.Parse("test_image.png");
    FHiResClock Clock{};
    GWindow->Bootstrap();
    GRHI->Bootstrap();
    LOG_INFO("{}ms", Clock.Elapsed().Milliseconds());

    while (!GWindow->ShouldClose())
    {

        static UInt64 CFrame{ 0 };
        //LOG_INFO("Frame {}.", ++CFrame);
        GWindow->PollEvents(); // You MUST call this function on MacOS.

    }
    GRHI->Terminate();
    //GWindow->Terminate();

    return EXIT_SUCCESS;
}