module;
#include <Visera.hpp>
#include <bvh/v2/bvh.h>

#include "Visera-Runtime.hpp"
export module AlohaVisera;
#define VISERA_MODULE_NAME "AlohaVisera"
import Visera.Demos;
import Visera.Core;
import Visera.Runtime;
import Visera.Engine;
import Visera.Core.Compression;
import Visera.Runtime.Scene;
using namespace Visera;

class Foo : public VObject
{
public:
    void Bar() {
        auto k = shared_from_this();
    }
};

export int main(int argc, char *argv[])
{
    GLog->SetLevel(ELogLevel::Debug);

    FHiResClock Clock{};
    for (int i = 0; i < 100; ++i)
    {
        auto Instance = GScene->CreateObject<Foo>(FName(Format("Foo_{}", i)));
        LOG_DEBUG("[{}] {}", Instance->GetID(), Instance->GetName());
    }
    LOG_DEBUG("Test Time: {}ms", Clock.Elapsed().Milliseconds());

    //Demos
    { Demos::Compression Demo; }

    GEngine->Bootstrap();
    {

        GEngine->Run();
    }
    GEngine->Terminate();

    return EXIT_SUCCESS;
}