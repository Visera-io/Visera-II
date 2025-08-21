module;
#include <Visera-Core.hpp>
export module Visera.Core.Media.Image;
#define VISERA_MODULE_NAME "Core.Media"
export import Visera.Core.Media.Image.Common;
export import Visera.Core.Media.Image.UTils;
       import Visera.Core.Media.Image.Wrappers;

export namespace Visera
{
    class FImage
    {
    public:

    protected:
        TArray<FByte> Data;
    };

}
