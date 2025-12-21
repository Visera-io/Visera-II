module;
#include <Visera-Core.hpp>
export module Visera.Core.Types.UUID;
#define VISERA_MODULE_NAME "Core.Types"
import Visera.Core.OS.Memory;

export namespace Visera
{
    /*
     * You can generate this via Visera.Platform (OS APIs).
     * Ref: RFC 4122 (UUID)
     *
     * Layout note:
     * - Data[16] stores the UUID as a 16-octet sequence (canonical byte order as used by the RFC text form).
     * - A/B/C/D are a convenience view for fast nil checks and simple comparisons.
     * - Because Data <-> A/B/C/D depends on the host endianness, Platform code should prefer writing Data[16]
     *   (canonical bytes), then call NormalizeWordsFromData() if it needs A/B/C/D.
     */
    class VISERA_CORE_API FUUID
    {
    public:
#if defined(VISERA_ON_LITTLE_ENDIAN_PLATFORM)
        union { struct { UInt32 A, B, C, D; }; FByte Data[16]{0}; };
#else
        union { struct { UInt32 D, C, B, A; }; FByte Data[16]{0}; };
#endif

        [[nodiscard]] constexpr Bool
        IsNil() const { return A == 0 && B == 0 && C == 0 && D == 0; }

        [[nodiscard]] friend inline auto
        operator<=>(const FUUID& I_Lhs, const FUUID& I_Rhs) noexcept
        {
            const int Cmp = Memory::Memcmp(I_Lhs.Data, I_Rhs.Data, 16);
            if (Cmp < 0) { return std::strong_ordering::less;   }
            if (Cmp > 0) { return std::strong_ordering::greater;}
            return std::strong_ordering::equal;
        }

        [[nodiscard]] friend inline Bool
        operator==(const FUUID& I_Lhs, const FUUID& I_Rhs) noexcept
        {  return Memory::Memcmp(I_Lhs.Data, I_Rhs.Data, 16) == 0; }
    };
};

VISERA_MAKE_FORMATTER(Visera::FUUID,
    // BODY
    const char* Hex = "0123456789abcdef";

    char S[37];
    int  I = 0;

    auto WriteByte = [&](Visera::FByte B)
    {
        S[I++] = Hex[(B >> 4) & 0x0F];
        S[I++] = Hex[(B >> 0) & 0x0F];
    };

    // 8-4-4-4-12 (bytes: 4-2-2-2-6), Data is canonical octet sequence
    WriteByte(I_Formatee.Data[0]);  WriteByte(I_Formatee.Data[1]);
    WriteByte(I_Formatee.Data[2]);  WriteByte(I_Formatee.Data[3]);
    S[I++] = '-';

    WriteByte(I_Formatee.Data[4]);  WriteByte(I_Formatee.Data[5]);
    S[I++] = '-';

    WriteByte(I_Formatee.Data[6]);  WriteByte(I_Formatee.Data[7]);
    S[I++] = '-';

    WriteByte(I_Formatee.Data[8]);  WriteByte(I_Formatee.Data[9]);
    S[I++] = '-';

    WriteByte(I_Formatee.Data[10]); WriteByte(I_Formatee.Data[11]);
    WriteByte(I_Formatee.Data[12]); WriteByte(I_Formatee.Data[13]);
    WriteByte(I_Formatee.Data[14]); WriteByte(I_Formatee.Data[15]);

    S[I] = '\0';
,
    // FormatString
    "{}",
    // __VA_ARGS__
    S
);