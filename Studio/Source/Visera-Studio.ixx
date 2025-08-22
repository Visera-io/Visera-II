module;
#include <Visera-Studio.hpp>
#include <imgui.h>
export module Visera.Studio;
#define VISERA_MODULE_NAME "Studio"

namespace Visera
{
    class VISERA_STUDIO_API FStudio
    {
    private:
        ImGuiID A{};
    };

    export inline VISERA_STUDIO_API
    TUniquePtr<FStudio> GStudio = MakeUnique<FStudio>();
}
