module;
#include <Visera-Core.hpp>
#include <random>
export module Visera.Core.Math.Random.Seed;
#define VISERA_MODULE_NAME "Core.Math"

namespace Visera
{
    export class VISERA_CORE_API FSeedPool
    {
    public:
        template<Concepts::UnsingedIntegral T = UInt32> [[nodiscard]] inline T
        Get() noexcept { return static_cast<T>(Generator()); }

    private:
        std::random_device Generator{}; // [TODO]: Policy
    };
} // namespace Visera