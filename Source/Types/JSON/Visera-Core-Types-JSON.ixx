module;
#include <Visera-Core.hpp>
#include <nlohmann/json.hpp>
export module Visera.Core.Types.JSON;
#define VISERA_MODULE_NAME "Core.Types.JSON"

export namespace Visera
{
    class FJSON
    {
    public:
        using Json = nlohmann::json;

        FJSON() = default;
        explicit FJSON(FStringView I_JSONData)
        {
            Parse(I_JSONData);
        }

        bool Parse(FStringView I_JSONData)
        {
            try
            {
                Data = Json::parse(I_JSONData);
                return true;
            }
            catch (const std::exception& e)
            {
                VISERA_WIP
                return false;
            }
        }

        void Set(FStringView I_Key, FStringView I_Value)
        {
            Data[I_Key] = I_Value;
        }

        FStringView
        Get(FStringView I_Key, FStringView I_DefaultValue = "") const
        {
            if (Data.contains(I_Key))
            {
                try
                {
                    return Data.at(I_Key).get<FStringView>();
                }
                catch (...)
                {
                    return I_DefaultValue;
                }
            }
            return I_DefaultValue;
        }

        bool Contains(FStringView I_Key) const
        {
            return Data.contains(I_Key);
        }

        FString
        Dump(bool bPretty = True) const
        {
            return bPretty ? Data.dump(4) : Data.dump();
        }

    private:
        Json Data;
    };

}