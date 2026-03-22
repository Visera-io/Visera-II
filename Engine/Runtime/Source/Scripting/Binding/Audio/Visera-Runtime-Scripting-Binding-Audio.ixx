/** Audio script bindings: visera.audio.post(eventName) posts a Wwise event on the default Player emitter (token 0). */
module;
#include <v8.h>
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Scripting.Binding.Audio;
#define VISERA_MODULE_NAME "Runtime.Scripting"
import Visera.Runtime.Scripting.Utils;
import Visera.Runtime.Audio;
import Visera.Core.Types.String;
import Visera.Core.Types.Optional;
import Visera.Core.Types.Name;
import Visera.Core.Log;

export namespace Visera
{
    /** Register visera.audio on IO_ViseraObject. I_Audio must be non-null. */
    void RegisterAudioBindings(FAudio* I_Audio, v8::Isolate* I_Isolate,
        v8::Local<v8::Context> I_Context, v8::Local<v8::Object> IO_ViseraObject);

namespace Private
{
    /** Default game object registered as "Player" in FAudio (Wwise); used for non-spatial script sounds. */
    inline constexpr FAudio::FObjectID DefaultEmitterToken{0};

    /** Optional second argument: "impact" (default), "ui", "ambient", "voice" (ASCII, case-insensitive). */
    FAudio::ECategory
    ParsePostEventCategory(v8::Isolate* I_Isolate, const v8::FunctionCallbackInfo<v8::Value>& I_Info)
    {
        if (I_Info.Length() < 2 || I_Info[1]->IsUndefined() || I_Info[1]->IsNull())
        { return FAudio::ECategory::Impact; }
        TOptional<FString> Label = FromV8String(I_Isolate, I_Info[1]);
        if (!Label.HasValue() || Label.GetValue().IsEmpty())
        { return FAudio::ECategory::Impact; }
        const FStringView Text{Label.GetValue()};
        auto LowerMatch = [Text](const char* I_AsciiLower) -> Bool
        {
            const auto Length = Text.GetSize();
            for (size_t Index = 0;; ++Index)
            {
                const char Expected = I_AsciiLower[Index];
                if (Expected == '\0')
                { return Index == Length; }
                if (Index >= Length)
                { return False; }
                char Byte = Text[static_cast<FStringView::SizeType>(Index)];
                if (Byte >= 'A' && Byte <= 'Z')
                { Byte = static_cast<char>(Byte + 32); }
                if (Byte != Expected)
                { return False; }
            }
        };
        if (LowerMatch("ui")) { return FAudio::ECategory::UI; }
        if (LowerMatch("ambient")) { return FAudio::ECategory::Ambient; }
        if (LowerMatch("voice")) { return FAudio::ECategory::Voice; }
        if (LowerMatch("impact")) { return FAudio::ECategory::Impact; }
        return FAudio::ECategory::Impact;
    }

    void V8_Audio_Load(const v8::FunctionCallbackInfo<v8::Value>& I_Info)
    {
        try
        {
            if (I_Info.Data().IsEmpty() || !I_Info.Data()->IsExternal())
            { return; }
            auto* Audio = static_cast<FAudio*>(v8::Local<v8::External>::Cast(I_Info.Data())->Value());
            if (!Audio) { return; }

            if (I_Info.Length() < 1)
            {
                LOG_WARN("(Binding) visera.audio.load(fileName) requires a bank file name string.");
                return;
            }

            v8::Isolate* Isolate = I_Info.GetIsolate();
            TOptional<FString> FileNameString = FromV8String(Isolate, I_Info[0]);
            if (!FileNameString.HasValue() || FileNameString.GetValue().IsEmpty())
            {
                LOG_WARN("(Binding) visera.audio.load requires a non-empty file name.");
                return;
            }

            const FName BankFile{FileNameString.GetValue()};
            if (!Audio->LoadSoundBank(BankFile))
            { LOG_WARN("(Binding) visera.audio.load failed for '{}'.", FileNameString.GetValue()); }
        }
        catch (...)
        {
            LOG_ERROR("(Binding) visera.audio.load threw.");
        }
    }

    void V8_Audio_Post(const v8::FunctionCallbackInfo<v8::Value>& I_Info)
    {
        try
        {
            if (I_Info.Data().IsEmpty() || !I_Info.Data()->IsExternal())
            { return; }
            auto* Audio = static_cast<FAudio*>(v8::Local<v8::External>::Cast(I_Info.Data())->Value());
            if (!Audio) { return; }

            if (I_Info.Length() < 1)
            {
                LOG_WARN("(Binding) visera.audio.post(eventName) requires an event name string.");
                return;
            }

            v8::Isolate* Isolate = I_Info.GetIsolate();
            TOptional<FString> EventNameString = FromV8String(Isolate, I_Info[0]);
            if (!EventNameString.HasValue() || EventNameString.GetValue().IsEmpty())
            {
                LOG_WARN("(Binding) visera.audio.post requires a non-empty event name string.");
                return;
            }

            const FName EventName{EventNameString.GetValue()};
            const FAudio::ECategory Category = ParsePostEventCategory(Isolate, I_Info);
            if (!Audio->PostEvent(EventName, DefaultEmitterToken, Category, FAudio::EPriority::Normal))
            { LOG_WARN("(Binding) visera.audio.post failed to enqueue '{}'.", EventNameString.GetValue()); }
        }
        catch (...)
        {
            LOG_ERROR("(Binding) visera.audio.post threw.");
        }
    }
}

    void RegisterAudioBindings(FAudio* I_Audio, v8::Isolate* I_Isolate,
        v8::Local<v8::Context> I_Context, v8::Local<v8::Object> IO_ViseraObject)
    {
        if (!I_Audio || I_Isolate == nullptr || I_Context.IsEmpty() || IO_ViseraObject.IsEmpty())
            return;

        v8::Local<v8::Object> AudioObject = v8::Object::New(I_Isolate);
        v8::Local<v8::External> AudioExternal = v8::External::New(I_Isolate, I_Audio);

        v8::Local<v8::FunctionTemplate> LoadTemplate = v8::FunctionTemplate::New(
            I_Isolate, Private::V8_Audio_Load, AudioExternal);
        v8::Local<v8::Function> LoadFunction = LoadTemplate->GetFunction(I_Context).ToLocalChecked();
        v8::MaybeLocal<v8::String> LoadKey = ToV8String(I_Isolate, "load");
        if (LoadKey.IsEmpty()
            || AudioObject->Set(I_Context, LoadKey.ToLocalChecked(), LoadFunction).IsNothing())
        {
            LOG_WARN("(Binding) Failed to set visera.audio.load.");
            return;
        }

        v8::Local<v8::FunctionTemplate> PostTemplate = v8::FunctionTemplate::New(
            I_Isolate, Private::V8_Audio_Post, AudioExternal);
        v8::Local<v8::Function> PostFunction = PostTemplate->GetFunction(I_Context).ToLocalChecked();
        v8::MaybeLocal<v8::String> PostKey = ToV8String(I_Isolate, "post");
        if (PostKey.IsEmpty()
            || AudioObject->Set(I_Context, PostKey.ToLocalChecked(), PostFunction).IsNothing())
        {
            LOG_WARN("(Binding) Failed to set visera.audio.post.");
            return;
        }

        v8::MaybeLocal<v8::String> AudioKey = ToV8String(I_Isolate, "audio");
        if (!AudioKey.IsEmpty()
            && !IO_ViseraObject->Set(I_Context, AudioKey.ToLocalChecked(), AudioObject).IsNothing())
        { /* visera.audio registered */ }
        else
            LOG_WARN("(Binding) Failed to set visera.audio.");
    }
}
