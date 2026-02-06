module;
#include <Visera-Forge.hpp>
#include <absl/strings/escaping.h>
export module Visera.Forge.Utils.Escaping;
#define VISERA_MODULE_NAME "Forge.Utils.Escaping"
import Visera.Core.Types.String;
import Visera.Core.Types.Array;
import Visera.Core.Types.Optional;

/**
 * Forge base64 encoding/decoding utilities.
 * Wraps Abseil's Base64Escape/Base64Unescape functions.
 */
export namespace Visera::Forge
{
    /**
     * Encode binary data to base64 string.
     * @param I_Data Binary data to encode
     * @return Base64-encoded string
     */
    [[nodiscard]] inline FString
    Base64Encode(const TArray<FByte>& I_Data)
    {
        std::string Encoded;
        absl::Base64Escape(
            absl::string_view(
                reinterpret_cast<const char*>(I_Data.Data()),
                I_Data.GetSize()),
            &Encoded);
        return FString(Encoded);
    }

    /**
     * Encode binary data to base64 string (from raw pointer and size).
     * @param I_Data Pointer to binary data
     * @param I_Size Size of data in bytes
     * @return Base64-encoded string
     */
    [[nodiscard]] inline FString
    Base64Encode(const FByte* I_Data, size_t I_Size)
    {
        std::string Encoded;
        absl::Base64Escape(
            absl::string_view(
                reinterpret_cast<const char*>(I_Data),
                I_Size),
            &Encoded);
        return FString(Encoded);
    }

    /**
     * Decode base64 string to binary data.
     * @param I_Base64 Base64-encoded string
     * @return Optional containing decoded binary data, or NullOpt if decoding fails
     */
    [[nodiscard]] inline TOptional<TArray<FByte>>
    Base64Decode(FStringView I_Base64)
    {
        std::string Decoded;
        const std::string_view Native = I_Base64.GetNative();
        if (!absl::Base64Unescape(
                absl::string_view(Native.data(), Native.size()),
                &Decoded))
        {
            return NullOpt;
        }

        return TOptional<TArray<FByte>>(
            TArray<FByte>(
                reinterpret_cast<const FByte*>(Decoded.data()),
                reinterpret_cast<const FByte*>(Decoded.data() + Decoded.size())));
    }
}
