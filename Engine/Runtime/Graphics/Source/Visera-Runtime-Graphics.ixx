module;
#include <Visera-Graphics.hpp>
export module Visera.Runtime.Graphics;
#define VISERA_MODULE_NAME "Runtime.Graphic"
export import Visera.Runtime.Graphics.Debug;
export import Visera.Runtime.Graphics.Scene;
export import Visera.Runtime.Graphics.Material;
       import Visera.Runtime.Graphics.Texture;
       import Visera.Runtime.Global;
       import Visera.Runtime.RHI;
       import Visera.Core.Image;
       import Visera.Core.Containers.Map;
       import Visera.Core.Containers.Array;

export namespace Visera
{
   class VISERA_RUNTIME_API FGraphics : public IGlobalService
   {
   public:


   private:
      FRHI*           RHI;
      FRHICommandList CommandList;

   public:
      FGraphics(FName I_Name, FServiceRegistry* I_Registry, const FJSON& I_Config)
          : IGlobalService(I_Name, I_Registry, I_Config)
      {
         Dependencies =
         {
            EName::RHI,
         };

         if (!OnBootstrap.TryBind([this]
         {
            if (auto RHIWeak = GetService<FRHI>(EName::RHI); auto RHIShared = RHIWeak.Lock())
            {
               RHI = RHIShared.Get();
            }
            else
            {
               LOG_FATAL("Failed to get RHI service!");
               return False;
            }
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