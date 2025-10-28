module;
#include <Visera.hpp>
export module Visera.Demos;
#define VISERA_MODULE_NAME "Visera.Demos"
//import Visera.Core;
//import Visera.Engine.AssetHub;

export namespace Visera::Demos
{
    // struct Compression
    // {
    //     Compression()
    //     {
    //         TArray<FByte> Buffer;
    //         FStringView Source = "aaaaaaaa";
    //         if (auto Result = Compress(Source, &Buffer) != ECompressionStatue::Success)
    //         {
    //             LOG_ERROR("Failed to compress! ({})", Result);
    //         }
    //         else LOG_WARN("Source Size:{} Compressed Size:{}", Source.size(), Buffer.size());
    //     }
    // };
    //
    // struct FileSystem
    // {
    //     FileSystem()
    //     {
    //
    //          LOG_INFO("{}", TEXT("❌"));
    //          FPath PathA{TEXT("Assets")};
    //          FPath PathB{TEXT("Assets")};
    //
    //          FFileSystem VFS{ FPath::CurrentPath() };
    //          LOG_INFO("{}", GAssetHub->GetDebugName());
    //
    //          if (!VFS.SearchFile(TEXT("Hello")).IsEmpty())
    //          {
    //              LOG_INFO("Found");
    //          }
    //          else LOG_ERROR("Not found");
    //          if (VFS.CreateDirectory(PathA))
    //          {
    //
    //          }
    //          if (VFS.DeleteDirectory(PathB))
    //          {
    //
    //          }
    //         LOG_INFO("{}", VFS.GetRoot());
    //     }
    //
    // };
}