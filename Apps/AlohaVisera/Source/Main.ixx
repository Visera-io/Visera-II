module;
#include "../Global/Visera.hpp"
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

    TArray<int> arr{1,2,3,4};
    TArrayView<int> arrView{arr};

    TArray<int> nums { 1,2};
    TSpan<int, 2> numsSpan{nums};
    Foo(&numsSpan);

    LOG_INFO("{}", numsSpan[0]);

    return EXIT_SUCCESS;
}