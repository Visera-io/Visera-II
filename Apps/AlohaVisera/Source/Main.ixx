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

    FPath PathA{TEXT("Assets")};
    FPath PathB{TEXT("Assets")};

    FFileSystem VFS{ FPath::CurrentPath() };
    LOG_INFO("{}", GAssetHub->GetDebugName());

    if (!VFS.SearchFile(TEXT("Hello")).IsEmpty())
    {
        LOG_INFO("Found");
    }
    else LOG_ERROR("Not found");
    if (VFS.CreateDirectory(PathA))
    {

    }
    if (VFS.DeleteDirectory(PathB))
    {

    }

    LOG_INFO("{}", VFS.GetRoot());

    GEngine->Bootstrap();
    {


        GEngine->Run();
    }
    GEngine->Terminate();

    return EXIT_SUCCESS;
}