module;
#include <Visera.hpp>
#include <bvh/v2/bvh.h>
export module AlohaVisera;
#define VISERA_MODULE_NAME "AlohaVisera"
import Visera.Core;
import Visera.Runtime;
import Visera.Engine;
import Visera.Core.Compression;
import Visera.Runtime.Object;
using namespace Visera;

export int main(int argc, char *argv[])
{
    // FPNGImageWrapper PNG;
    // //PNG.Parse("test_image.png");
    //bvh::v2::BBox<Int32, 2> TestBB;

    FHiResClock Clock{};

    auto Obj = VObject::Create();
    LOG_INFO("{}", Obj->GetID());
    
    TArray<FByte> Buffer;
    FStringView Source = "aaaaaaaa";
    if (auto Result = Compress(Source, &Buffer) != ECompressionStatue::Success)
    {
        LOG_ERROR("Failed to compress! ({})", Result);
    }
    else LOG_WARN("Src Size:{} Com Size:{}", Source.size(), Buffer.size());

    auto Lib = GPlatform->LoadLibrary(
        PATH("libhostfxr.dylib"));
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

    // LOG_INFO("{}", TEXT("❌"));
    // FPath PathA{TEXT("Assets")};
    // FPath PathB{TEXT("Assets")};
    //
    // FFileSystem VFS{ FPath::CurrentPath() };
    // LOG_INFO("{}", GAssetHub->GetDebugName());
    //
    // if (!VFS.SearchFile(TEXT("Hello")).IsEmpty())
    // {
    //     LOG_INFO("Found");
    // }
    // else LOG_ERROR("Not found");
    // if (VFS.CreateDirectory(PathA))
    // {
    //
    // }
    // if (VFS.DeleteDirectory(PathB))
    // {
    //
    // }
    //LOG_INFO("{}", VFS.GetRoot());

    GEngine->Bootstrap();
    {
        GEngine->Run();
    }
    GEngine->Terminate();

    return EXIT_SUCCESS;
}