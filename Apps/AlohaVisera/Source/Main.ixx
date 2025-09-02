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
    // FPNGImageWrapper PNG;
    // //PNG.Parse("test_image.png");
    FHiResClock Clock{};

    GEngine->Bootstrap();
    {
        auto& V = GRHI->GetDriver();
        if (V->GetType() == RHI::EDriverType::Vulkan)
        {
            LOG_INFO("Vulkan");
        }
        else LOG_WARN("??");

        GEngine->Run();
    }
    GEngine->Terminate();

    return EXIT_SUCCESS;
}