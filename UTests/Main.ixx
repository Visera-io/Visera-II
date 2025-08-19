module;
#include <Visera-Core.hpp>
export module Visera.UTests.Main;
#define VISERA_MODULE_NAME "UTests.Main"
import Visera.Core;
using namespace Visera;


export int main(int argc, char *argv[])
{
    FColor Color{1,2,3,4};
    auto LC = FLinearColor::FromPow22Color(Color);
    LOG_INFO("{}", Color);

    LOG_WARN("{}", FRadian{3.14});

    auto Text = TEXT("你好世界");
    std::cout << (const char*)(Text);
    //LOG_INFO("{}", MyType{42, 100, "test"});
    FName name {"marry_0"};
    FMatrix4x4F Mat_I{FMatrix4x4F::Identity()};
    LOG_INFO("Vec = {}", FVector4F{1,2,3,4});
    LOG_INFO("Mat = {}", Mat_I);

    return EXIT_SUCCESS;
}