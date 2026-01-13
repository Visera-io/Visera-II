module;
#include <Visera-Core.hpp>
export module Visera.Core.Archive.JSON;
#define VISERA_MODULE_NAME "Core.Archive"
       import Visera.Core.Archive.Interface;
       import Visera.Core.Types.JSON;
       import Visera.Core.Types.Array;

export namespace Visera
{
    // Scope entry for tracking nested object/array paths
    struct FScopeEntry
    {
        FString Path;  // Current path prefix (e.g., "parent.child")
        Bool    IsArray = False;
        UInt32  ArrayIndex = 0;
    };

    /**
     * @brief JSON implementation of IArchive using Visera.Core.Types.JSON.
     * 
     * Serializes data to/from JSON format. Supports nested objects, arrays, and all basic types.
     * 
     * @example Basic saving (create JSON from data)
     * @code
     * FArchiveJSON Archive(EMode::Saving);
     * Archive << "name" << "Bob"
     *        << "id" << 12345u
     *        << "temperature" << 98.6;
     * FString jsonString = Archive.Dump(true);  // Pretty print
     * // jsonString = {
     * //     "name": "Bob",
     * //     "id": 12345,
     * //     "temperature": 98.6
     * // }
     * @endcode
     * 
     * @example Basic loading (read data from JSON string)
     * @code
     * FString jsonInput = R"({"name":"Carol","id":67890,"temperature":36.5})";
     * FArchiveJSON Archive(jsonInput, EMode::Loading);
     * FString name;
     * UInt32 id;
     * Double temp;
     * Archive << "name" << name << "id" << id << "temperature" << temp;
     * // name = "Carol", id = 67890, temp = 36.5
     * @endcode
     * 
     * @example Loading from existing FJSON object
     * @code
     * FJSON json;
     * json.Parse(R"({"value":42})");
     * FArchiveJSON Archive(json, EMode::Loading);
     * Int32 value = 0;
     * Archive << "value" << value;
     * // value = 42
     * @endcode
     * 
     * @example Nested object serialization
     * @code
     * FArchiveJSON Archive(EMode::Saving);
     * Archive.PushScope("config");
     * Archive << "width" << 1920 << "height" << 1080;
     * Archive.PushScope("display");
     * Archive << "fullscreen" << true << "vsync" << false;
     * Archive.PopScope();
     * Archive << "version" << 1;
     * Archive.PopScope();
     * // Result: {"config":{"width":1920,"height":1080,"display":{"fullscreen":true,"vsync":false},"version":1}}
     * @endcode
     * 
     * @example Direct JSON access
     * @code
     * FArchiveJSON Archive(EMode::Saving);
     * Archive << "key" << "value";
     * FJSON& json = Archive.GetJSON();
     * // Can manipulate json directly if needed
     * json.Set("additional", "data");
     * @endcode
     * 
     * @example Save mode with parsing existing JSON
     * @code
     * FString existingJson = R"({"old":"data"})";
     * FArchiveJSON Archive(existingJson, EMode::Saving);
     * // Can modify existing JSON
     * Archive << "new" << "field";
     * Archive << "old" << "updated";
     * @endcode
     */
    // class VISERA_CORE_API FArchiveJSON : public IArchive
    // {
    // public:
    //     explicit FArchiveJSON(EMode I_Mode)
    //         : Mode(I_Mode)
    //     {
    //         if (I_Mode == EMode::Saving)
    //         {
    //             // Initialize as empty JSON object for saving
    //             JSON.GetNative() = {};
    //         }
    //     }
    //
    //     explicit FArchiveJSON(FStringView I_JSONData, EMode I_Mode = EMode::Loading)
    //         : Mode(I_Mode)
    //     {
    //         if (I_Mode == EMode::Loading)
    //         {
    //             JSON.Parse(I_JSONData);
    //         }
    //     }
    //
    //     explicit FArchiveJSON(const FJSON& I_JSON, EMode I_Mode = EMode::Loading)
    //         : Mode(I_Mode)
    //         , JSON(I_JSON)
    //     {
    //     }
    //
    //     ~FArchiveJSON() override = default;
    //
    //     [[nodiscard]] Bool IsLoading() const override
    //     {
    //         return Mode == EMode::Loading;
    //     }
    //
    //     [[nodiscard]] Bool IsSaving() const override
    //     {
    //         return Mode == EMode::Saving;
    //     }
    //
    //     void Serialize(FStringView I_Key, Bool& IO_Value) override
    //     {
    //         FString FullPath = BuildFullPath(I_Key);
    //         if (IsSaving())
    //         {
    //             JSON.SetPath(FullPath, IO_Value);
    //         }
    //         else
    //         {
    //             IO_Value = JSON.GetBoolPath(FullPath, IO_Value);
    //         }
    //     }
    //
    //     void Serialize(FStringView I_Key, Int32& IO_Value) override
    //     {
    //         FString FullPath = BuildFullPath(I_Key);
    //         if (IsSaving())
    //         {
    //             JSON.SetPath(FullPath, static_cast<Double>(IO_Value));
    //         }
    //         else
    //         {
    //             Double Value = static_cast<Double>(IO_Value);
    //             JSON.TryGetNumberPath(FullPath, &Value);
    //             IO_Value = static_cast<Int32>(Value);
    //         }
    //     }
    //
    //     void Serialize(FStringView I_Key, UInt32& IO_Value) override
    //     {
    //         FString FullPath = BuildFullPath(I_Key);
    //         if (IsSaving())
    //         {
    //             JSON.SetPath(FullPath, static_cast<Double>(IO_Value));
    //         }
    //         else
    //         {
    //             Double Value = static_cast<Double>(IO_Value);
    //             JSON.TryGetNumberPath(FullPath, &Value);
    //             IO_Value = static_cast<UInt32>(Value);
    //         }
    //     }
    //
    //     void Serialize(FStringView I_Key, Double& IO_Value) override
    //     {
    //         FString FullPath = BuildFullPath(I_Key);
    //         if (IsSaving())
    //         {
    //             JSON.SetPath(FullPath, IO_Value);
    //         }
    //         else
    //         {
    //             JSON.TryGetNumberPath(FullPath, &IO_Value);
    //         }
    //     }
    //
    //     void Serialize(FStringView I_Key, FString& IO_Value) override
    //     {
    //         FString FullPath = BuildFullPath(I_Key);
    //         if (IsSaving())
    //         {
    //             JSON.SetPath(FullPath, IO_Value);
    //         }
    //         else
    //         {
    //             IO_Value = JSON.GetStringPath(FullPath, IO_Value);
    //         }
    //     }
    //
    //     Bool PushScope(FStringView I_Key) override
    //     {
    //         FScopeEntry Entry;
    //         if (ScopeStack.IsEmpty())
    //         {
    //             Entry.Path = FString(I_Key);
    //         }
    //         else
    //         {
    //             const FScopeEntry& Top = ScopeStack.Back();
    //             if (Top.IsArray)
    //             {
    //                 Entry.Path = Top.Path + "[" + std::to_string(Top.ArrayIndex) + "]." + FString(I_Key);
    //             }
    //             else
    //             {
    //                 if (Top.Path.empty())
    //                 {
    //                     Entry.Path = FString(I_Key);
    //                 }
    //                 else
    //                 {
    //                     Entry.Path = Top.Path + "." + FString(I_Key);
    //                 }
    //             }
    //         }
    //
    //         // Check if this is an array by attempting to detect array notation
    //         // For now, assume object scope unless specified otherwise
    //         Entry.IsArray = False;
    //         Entry.ArrayIndex = 0;
    //
    //         ScopeStack.PushBack(Entry);
    //         return True;
    //     }
    //
    //     void PopScope() override
    //     {
    //         if (!ScopeStack.IsEmpty())
    //         {
    //             ScopeStack.PopBack();
    //         }
    //     }
    //
    //     // Additional methods for JSON-specific operations
    //     [[nodiscard]] FString Dump(Bool bPretty = True) const
    //     {
    //         return JSON.Dump(bPretty);
    //     }
    //
    //     [[nodiscard]] Bool Parse(FStringView I_JSONData)
    //     {
    //         return JSON.Parse(I_JSONData);
    //     }
    //
    //     [[nodiscard]] FJSON& GetJSON() { return JSON; }
    //     [[nodiscard]] const FJSON& GetJSON() const { return JSON; }
    //
    //     // Array scope management helpers
    //     Bool PushArrayScope(FStringView I_Key, UInt32 I_ArrayIndex)
    //     {
    //         FScopeEntry Entry;
    //         if (ScopeStack.IsEmpty())
    //         {
    //             Entry.Path = FString(I_Key);
    //         }
    //         else
    //         {
    //             const FScopeEntry& Top = ScopeStack.Back();
    //             if (Top.IsArray)
    //             {
    //                 Entry.Path = Top.Path + "[" + std::to_string(Top.ArrayIndex) + "]." + FString(I_Key);
    //             }
    //             else
    //             {
    //                 if (Top.Path.empty())
    //                 {
    //                     Entry.Path = FString(I_Key);
    //                 }
    //                 else
    //                 {
    //                     Entry.Path = Top.Path + "." + FString(I_Key);
    //                 }
    //             }
    //         }
    //
    //         Entry.IsArray = True;
    //         Entry.ArrayIndex = I_ArrayIndex;
    //         ScopeStack.PushBack(Entry);
    //         return True;
    //     }
    //
    // private:
    //     EMode Mode;
    //     FJSON JSON;
    //     TArray<FScopeEntry> ScopeStack;
    //
    //     [[nodiscard]] FString BuildFullPath(FStringView I_Key) const
    //     {
    //         if (ScopeStack.IsEmpty())
    //         {
    //             return FString(I_Key);
    //         }
    //
    //         const FScopeEntry& Top = ScopeStack.Back();
    //         if (Top.IsArray)
    //         {
    //             return Top.Path + "[" + std::to_string(Top.ArrayIndex) + "]." + FString(I_Key);
    //         }
    //         else
    //         {
    //             if (Top.Path.empty())
    //             {
    //                 return FString(I_Key);
    //             }
    //             return Top.Path + "." + FString(I_Key);
    //         }
    //     }
    // };
}
