module;
#include <Visera.hpp>
#include <bvh/v2/bvh.h>
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
    bvh::v2::BBox<Int32, 2> TestBB;

    TS::TMap<FString, Int32> Map;
    Map["Hello"] = 10;
    LOG_INFO("{}", Map["Hello"]);

    auto Lib = GPlatform->LoadLibrary(
        PATH("hostfxr.dll"));
    if (!Lib->IsLoaded())
    {

    }
    Lib = GPlatform->LoadLibrary(PATH("kernel32.dll"));
    if (Lib->IsLoaded())
    {
        LOG_INFO("Loaded Kernel32.dll");
        if (Lib->LoadFunction("SetThreadDescription"))
        {
            LOG_INFO("Loaded SetThreadDescription");
        }

    }

    LOG_INFO("{}", TEXT("❌"));
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