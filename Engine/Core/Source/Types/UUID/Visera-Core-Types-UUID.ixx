module;
#include <Visera-Core.hpp>
export module Visera.Core.Types.UUID;
#define VISERA_MODULE_NAME "Core.Types"
import Visera.Core.Math.Random;
import Visera.Core.OS.Memory;

export namespace Visera
{
    /*
     * Ref: RFC 4122 (UUID).
     * Generate in Core: static Generate() returns RFC 4122 version 4 (random) UUID; no Platform dependency.
     * Alternatively generate via Visera.Platform (OS APIs).
     *
     * Layout: Data[16] stores the UUID as a 16-octet sequence (canonical byte order as used by the RFC text form).
     */
    class VISERA_CORE_API FUUID
    {
    public:
        FByte Data[16]{0};

        /** Returns a new RFC 4122 version 4 (random) UUID using Core.Math.Random. */
        [[nodiscard]] static FUUID Generate();

        [[nodiscard]] constexpr Bool IsNil() const
        {
            for (int Index = 0; Index < 16; ++Index)
            {
                if (Data[Index] != 0) return False;
            }
            return True;
        }

        [[nodiscard]] friend auto operator<=>(const FUUID&, const FUUID&) = default;
    };

    inline FUUID FUUID::Generate()
    {
        thread_local FRandomSeed RandomSeed;
        thread_local FPCG32 RandomGenerator(RandomSeed.Get<UInt64>(), RandomSeed.Get<UInt32>());

        FUUID Result;
        const UInt32 Randoms[4] = {
            RandomGenerator.Uniform<UInt32>(),
            RandomGenerator.Uniform<UInt32>(),
            RandomGenerator.Uniform<UInt32>(),
            RandomGenerator.Uniform<UInt32>()
        };
        Memory::Memcpy(Result.Data, Randoms, 16);

        Result.Data[6] = static_cast<FByte>((Result.Data[6] & 0x0Fu) | 0x40u);
        Result.Data[8] = static_cast<FByte>((Result.Data[8] & 0x3Fu) | 0x80u);

        return Result;
    }
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
