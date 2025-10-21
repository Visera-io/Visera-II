module;
#include <Visera-Studio.hpp>
#include <freetype/freetype.h>
export module Visera.Studio.Baker.Font;
#define VISERA_MODULE_NAME "Studio.Baker"
import Visera.Core.Log;

namespace Visera
{
   export class VISERA_STUDIO_API FFontBaker
   {
   public:

   private:
      FT_Library Library {nullptr};

   public:
      FFontBaker();
      ~FFontBaker();
   };

   FFontBaker::
   FFontBaker()
   {
      LOG_TRACE("Initializing FreeType!");

      if (FT_Init_FreeType(&Library) != FT_Err_Ok)
      {
         LOG_FATAL("Failed to initialize FreeType!");
      }
   }

   FFontBaker::
   ~FFontBaker()
   {
      if (Library)
      {
         LOG_TRACE("Releasing FreeType.");
         FT_Done_FreeType(Library);
      }
   }
}