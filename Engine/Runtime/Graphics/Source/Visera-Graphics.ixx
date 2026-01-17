module;
#include <Visera-Graphics.hpp>
export module Visera.Graphics;
#define VISERA_MODULE_NAME "Graphics"
export import Visera.Graphics.Debug;
export import Visera.Graphics.Scene;
       import Visera.Graphics.RenderGraph;
       import Visera.Global;

export namespace Visera
{
   class VISERA_GRAPHICS_API FGraphics : public IGlobalService
   {
   public:
      void Tick(Float I_Time)
      {
         RenderGraph.Execute(nullptr);
      }

   private:
      FRenderGraph RenderGraph;

   public:
      FGraphics() : IGlobalService(EName::Graphics)
      {
         Dependencies =
         {
            EName::RHI,
         };

         if (!OnBootstrap.TryBind([this]
         {
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