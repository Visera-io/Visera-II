module;
#include <Visera-Core.hpp>
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

    LOG_INFO("{}", FName(TEXT("sdwdw_0").GetData()));
    TMap<FName, int> Map;
    auto TestName= FName{"s_0"};
    Map[TestName] = 1;
    TSet<FName> TestSet;
    TestSet.insert(TestName);

    LOG_INFO("{}", Map[TestName]);

    LOG_INFO("?");
    VWindow Window{};

    return EXIT_SUCCESS;
}