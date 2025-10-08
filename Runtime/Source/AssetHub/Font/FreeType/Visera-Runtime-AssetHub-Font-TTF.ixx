module;
#include <Visera-Runtime.hpp>
#include <freetype/freetype.h>
export module Visera.Runtime.AssetHub.Font.FreeType;
#define VISERA_MODULE_NAME "Runtime.AssetHub"
import Visera.Runtime.AssetHub.Asset;
import Visera.Core.Log;

namespace Visera
{
   export class VISERA_RUNTIME_API FFreeType
   {
   public:
      FFreeType()
      {
         LOG_TRACE("Initializing FreeType!");

         if (FT_Init_FreeType(&Library) != FT_Err_Ok)
         {
            LOG_FATAL("Failed to initialize FreeType!");
         }
      }
      ~FFreeType()
      {
         if (Library)
         {
            LOG_TRACE("Releasing FreeType.");
            FT_Done_FreeType(Library);
         }
      }
   private:
      FT_Library Library {nullptr};
   };
}