module;
#include <Visera-Graphics.hpp>
export module Visera.Graphics;
#define VISERA_MODULE_NAME "Graphics"
export import Visera.Graphics.Debug;
export import Visera.Graphics.Scene;
export import Visera.Graphics.Material;
       import Visera.Graphics.Texture;
       import Visera.Global;
       import Visera.RHI;
       import Visera.Core.Image;
       import Visera.Core.Types.Map;
       import Visera.Core.Types.Array;

export namespace Visera
{
   class VISERA_GRAPHICS_API FGraphics : public IGlobalService
   {
   public:
      void Tick(Float I_Time)
      {
         RHI->Submit(CommandList);
         CommandList.Reset();
      }

   private:
      FRHI*           RHI;
      FRHICommandList CommandList;

   public:
      FGraphics() : IGlobalService(EName::Graphics)
      {
         Dependencies =
         {
            EName::RHI,
         };

         if (!OnBootstrap.TryBind([this]
         {
            RHI = Get<FRHI>(EName::RHI);
            return True;
         }))
         { LOG_FATAL("Failed to bind bootstrap function!"); }

         if (!OnTerminate.TryBind([this]
         {
            return True;
         }))
         { LOG_FATAL("Failed to bind terminate function!"); }
      }
   };
}