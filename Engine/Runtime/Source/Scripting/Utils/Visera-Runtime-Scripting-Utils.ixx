/** V8 value conversion helpers for the scripting layer. Used by VM, Context and Binding. */
module;
#include <v8.h>
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Scripting.Utils;
#define VISERA_MODULE_NAME "Runtime.Scripting.Utils"
import Visera.Core.Types.String;
import Visera.Core.Types.Optional;

export namespace Visera
{
    /** Convert C++ string to v8::Local<v8::String>. Returns empty on failure. */
    [[nodiscard]] v8::MaybeLocal<v8::String>
    ToV8String(v8::Isolate* I_Isolate, FStringView I_String);

    /** Convert v8::Value to FString (UTF-8). Returns NullOpt if not a string or conversion fails. */
    [[nodiscard]] TOptional<FString>
    FromV8String(v8::Isolate* I_Isolate, v8::Local<v8::Value> I_Value);

    /** Safe Int32 extraction from v8::Value. */
    [[nodiscard]] TOptional<Int32>
    FromV8Int32(v8::Local<v8::Context> I_Context, v8::Local<v8::Value> I_Value);

    /** Safe Double extraction from v8::Value. */
    [[nodiscard]] TOptional<Double>
    FromV8Double(v8::Local<v8::Context> I_Context, v8::Local<v8::Value> I_Value);

    // --- Implementation ---

    v8::MaybeLocal<v8::String> ToV8String(v8::Isolate* I_Isolate, FStringView I_String)
    {
        if (!I_Isolate || I_String.IsEmpty())
            return v8::MaybeLocal<v8::String>();
        const std::string_view View = I_String.GetNative();
        return v8::String::NewFromUtf8(I_Isolate, View.data(), v8::NewStringType::kNormal,
            static_cast<int>(View.size()));
    }

    TOptional<FString> FromV8String(v8::Isolate* I_Isolate, v8::Local<v8::Value> I_Value)
    {
        if (!I_Value->IsString())
            return NullOpt;
        v8::String::Utf8Value Utf8(I_Isolate, I_Value);
        if (*Utf8 == nullptr)
            return NullOpt;
        return FString(*Utf8, static_cast<FString::SizeType>(Utf8.length()));
    }

    TOptional<Int32> FromV8Int32(v8::Local<v8::Context> I_Context, v8::Local<v8::Value> I_Value)
    {
        if (I_Context.IsEmpty() || I_Value.IsEmpty())
            return NullOpt;
        v8::Maybe<int32_t> Maybe = I_Value->Int32Value(I_Context);
        if (!Maybe.IsJust())
            return NullOpt;
        return Maybe.FromJust();
    }

    TOptional<Double> FromV8Double(v8::Local<v8::Context> I_Context, v8::Local<v8::Value> I_Value)
    {
        if (I_Context.IsEmpty() || I_Value.IsEmpty())
            return NullOpt;
        v8::Maybe<double> Maybe = I_Value->NumberValue(I_Context);
        if (!Maybe.IsJust())
            return NullOpt;
        return Maybe.FromJust();
    }
}
