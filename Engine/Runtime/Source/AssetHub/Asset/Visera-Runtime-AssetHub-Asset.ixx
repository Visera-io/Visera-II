module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.AssetHub.Asset;
#define VISERA_MODULE_NAME "Runtime.AssetHub"

export namespace Visera
{
    enum class ELoadMode : UInt8
    {
        Eager,
    };

    enum class ESaveMode : UInt8
    {
        AtomicReplace,
        Direct,
    };

	/**
	 * Read-only asset interface for AssetHub.
	 * All loaded assets (FImageAsset, FFontAsset, FShaderAsset, etc.) implement this.
	 * Only read API is exposed; in-place modification by users is not supported.
	 * To write new asset data, use FAssetHub::Save(const DataType&, const FPath&) with pure data (e.g. FImage) to avoid multi-thread write issues.
	 */
	class VISERA_RUNTIME_API IAsset
	{
	public:
		/** Size in bytes of the asset data (e.g. image buffer, font file buffer, SPIR-V). */
		[[nodiscard]] virtual UInt64
		GetByteSize() const = 0;

	public:
		virtual ~IAsset() = default;
	};

    namespace Concepts
    {
        template<typename T> concept
        Asset = std::derived_from<T, IAsset>;
    }
}
