module;
#include "../Global/Visera.hpp"
export module AlohaVisera;
#define VISERA_MODULE_NAME "AlohaVisera"
import Visera.Core;
import Visera.Core.Media.Image.Wrappers;
import Visera.Runtime;
import Visera.Engine;
using namespace Visera;

void Foo(TMutable<Float> I_Val)
{
    I_Val = 0;
}

export int main(int argc, char *argv[])
{
    FPNGImageWrapper PNG;
    //PNG.Parse("test_image.png");

    TArray<UInt32> arr{1,2,3,4};
    TArrayView<UInt32> arrView{arr};

    LOG_INFO("{}", arrView[1]);

    Float Val = 2.0;
    Foo(&Val);
    Float* const pV = &Val;
    *pV = 3;
    LOG_INFO("{}", Val);

    return EXIT_SUCCESS;
}