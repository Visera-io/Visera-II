module;
#include <v8.h>
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Scripting.Binding;
#define VISERA_MODULE_NAME "Runtime.Scripting.Binding"
export import Visera.Runtime.Scripting.Binding.Graphics;
export import Visera.Runtime.Scripting.Binding.Audio;
export import Visera.Runtime.Scripting.Binding.Input;
       import Visera.Runtime.Scripting.VM;
       import Visera.Runtime.Scripting.Context;
       import Visera.Runtime.Scripting.Utils;
       import Visera.Runtime.Graphics;
       import Visera.Runtime.Audio;
       import Visera.Runtime.Input;
       import Visera.Runtime.Window;
       import Visera.Core.Types.String;
       import Visera.Core.Types.Optional;
       import Visera.Core.Log;

export namespace Visera
{
    /**
     * Register all script bindings on the given context. I_Graphics may be null (e.g. when only log API is used).
     * I_Audio may be null when audio is disabled; when non-null, registers visera.audio.load / visera.audio.post for Wwise banks and events.
     * I_InputState must be non-null when I_Input is non-null (owns visera.input); otherwise pass nullptr for both.
     * When I_ScriptMainWindowSink is non-null, registers visera.setMainWindow (OnInit); writes FWindowCreateInfo for the engine to consume.
     */
    void RegisterAllBindings(FGraphics* I_Graphics, FAudio* I_Audio, FInput* I_Input, FJavaScriptInputBindingState* I_InputState, TOptional<FWindowCreateInfo>* I_ScriptMainWindowSink, FJavaScriptContext& I_Context);

namespace Private
{
    void V8_Log_Info(const v8::FunctionCallbackInfo<v8::Value>& I_Info)
    {
        try
        {
            v8::Isolate* Isolate = I_Info.GetIsolate();
            if (!Isolate) return;
            FString Message = I_Info.Length() > 0
                ? FromV8String(Isolate, I_Info[0]).Get(FString(""))
                : FString("");
            LOG_INFO("(Script) {}", Message);
        }
        catch (...)
        {
            LOG_ERROR("(Binding) log.info threw.");
        }
    }

    void V8_Log_Warn(const v8::FunctionCallbackInfo<v8::Value>& I_Info)
    {
        try
        {
            v8::Isolate* Isolate = I_Info.GetIsolate();
            if (!Isolate) return;
            FString Message = I_Info.Length() > 0
                ? FromV8String(Isolate, I_Info[0]).Get(FString(""))
                : FString("");
            LOG_WARN("(Script) {}", Message);
        }
        catch (...)
        {
            LOG_ERROR("(Binding) log.warn threw.");
        }
    }

    void V8_Log_Error(const v8::FunctionCallbackInfo<v8::Value>& I_Info)
    {
        try
        {
            v8::Isolate* Isolate = I_Info.GetIsolate();
            if (!Isolate) return;
            FString Message = I_Info.Length() > 0
                ? FromV8String(Isolate, I_Info[0]).Get(FString(""))
                : FString("");
            LOG_ERROR("(Script) {}", Message);
        }
        catch (...)
        {
            LOG_ERROR("(Binding) log.error threw.");
        }
    }

    UInt32
    ClampWindowDimension(Double I_Value)
    {
        if (I_Value < 1.0) return 1u;
        if (I_Value > 16384.0) return 16384u;
        return static_cast<UInt32>(I_Value);
    }

    void V8_SetMainWindow(const v8::FunctionCallbackInfo<v8::Value>& I_Info)
    {
        try
        {
            if (I_Info.Length() < 1 || !I_Info[0]->IsObject())
            {
                LOG_WARN("(Binding) visera.setMainWindow(obj) requires an object.");
                return;
            }
            if (I_Info.Data().IsEmpty() || !I_Info.Data()->IsExternal())
            { return; }
            auto* Sink = static_cast<TOptional<FWindowCreateInfo>*>(v8::Local<v8::External>::Cast(I_Info.Data())->Value());
            if (!Sink)
            { return; }

            v8::Isolate*           Isolate = I_Info.GetIsolate();
            v8::Local<v8::Context> Context = Isolate->GetCurrentContext();
            v8::Local<v8::Object>  Obj     = I_Info[0].As<v8::Object>();

            auto TryGet = [&](const char* I_Key) -> v8::MaybeLocal<v8::Value>
            {
                v8::MaybeLocal<v8::String> K = ToV8String(Isolate, I_Key);
                if (K.IsEmpty()) return {};
                return Obj->Get(Context, K.ToLocalChecked());
            };

            FWindowCreateInfo Out{};

            if (v8::MaybeLocal<v8::Value> MV = TryGet("title"); !MV.IsEmpty())
            {
                if (TOptional<FString> S = FromV8String(Isolate, MV.ToLocalChecked()); S.HasValue() && !S.GetValue().IsEmpty())
                { Out.Title = std::move(S.GetValue()); }
            }
            else if (v8::MaybeLocal<v8::Value> MV2 = TryGet("Title"); !MV2.IsEmpty())
            {
                if (TOptional<FString> S = FromV8String(Isolate, MV2.ToLocalChecked()); S.HasValue() && !S.GetValue().IsEmpty())
                { Out.Title = std::move(S.GetValue()); }
            }

            if (v8::MaybeLocal<v8::Value> MV = TryGet("width"); !MV.IsEmpty())
            {
                if (TOptional<Double> N = FromV8Double(Context, MV.ToLocalChecked()))
                { Out.Width = ClampWindowDimension(N.GetValue()); }
            }
            else if (v8::MaybeLocal<v8::Value> MV2 = TryGet("Width"); !MV2.IsEmpty())
            {
                if (TOptional<Double> N = FromV8Double(Context, MV2.ToLocalChecked()))
                { Out.Width = ClampWindowDimension(N.GetValue()); }
            }

            if (v8::MaybeLocal<v8::Value> MV = TryGet("height"); !MV.IsEmpty())
            {
                if (TOptional<Double> N = FromV8Double(Context, MV.ToLocalChecked()))
                { Out.Height = ClampWindowDimension(N.GetValue()); }
            }
            else if (v8::MaybeLocal<v8::Value> MV2 = TryGet("Height"); !MV2.IsEmpty())
            {
                if (TOptional<Double> N = FromV8Double(Context, MV2.ToLocalChecked()))
                { Out.Height = ClampWindowDimension(N.GetValue()); }
            }

            auto ApplyBool = [&](const char* Camel, const char* Pascal, Bool& O_Field)
            {
                for (const char* Key : {Camel, Pascal})
                {
                    v8::MaybeLocal<v8::String> KS = ToV8String(Isolate, Key);
                    if (KS.IsEmpty()) { continue; }
                    v8::Local<v8::Value> V;
                    if (!Obj->Get(Context, KS.ToLocalChecked()).ToLocal(&V) || V.IsEmpty() || !V->IsBoolean())
                    { continue; }
                    O_Field = v8::Local<v8::Boolean>::Cast(V)->Value() ? True : False;
                    return;
                }
            };
            ApplyBool("resizable", "Resizable", Out.Resizable);
            ApplyBool("center", "Center", Out.Center);
            ApplyBool("fullscreen", "Fullscreen", Out.Fullscreen);

            *Sink = Out;
        }
        catch (...)
        {
            LOG_ERROR("(Binding) setMainWindow threw.");
        }
    }

}

    void RegisterAllBindings(FGraphics* I_Graphics, FAudio* I_Audio, FInput* I_Input, FJavaScriptInputBindingState* I_InputState, TOptional<FWindowCreateInfo>* I_ScriptMainWindowSink, FJavaScriptContext& I_Context)
    {
        v8::Isolate* Isolate = I_Context.GetIsolate();
        if (!Isolate) return;
        v8::Isolate::Scope IsolateScope(Isolate);
        v8::HandleScope HandleScope(Isolate);
        v8::Local<v8::Context> Context = I_Context.GetContext();
        if (Context.IsEmpty()) return;
        v8::Context::Scope ContextScope(Context);

        v8::Local<v8::Object> LogObject = v8::Object::New(Isolate);
        auto SetLogMethod = [&](const char* Name, void (*Callback)(const v8::FunctionCallbackInfo<v8::Value>&))
        {
            v8::MaybeLocal<v8::String> Key = ToV8String(Isolate, Name);
            if (Key.IsEmpty()) return;
            v8::Local<v8::FunctionTemplate> Tmpl = v8::FunctionTemplate::New(Isolate, Callback);
            v8::Local<v8::Function> Fn = Tmpl->GetFunction(Context).ToLocalChecked();
            LogObject->Set(Context, Key.ToLocalChecked(), Fn);
        };
        SetLogMethod("info", Private::V8_Log_Info);
        SetLogMethod("warn", Private::V8_Log_Warn);
        SetLogMethod("error", Private::V8_Log_Error);

        v8::Local<v8::Object> ViseraObject = v8::Object::New(Isolate);
        v8::MaybeLocal<v8::String> LogKey = ToV8String(Isolate, "log");
        if (LogKey.IsEmpty()) return;
        if (ViseraObject->Set(Context, LogKey.ToLocalChecked(), LogObject).IsNothing())
            return;

        if (I_Graphics)
            RegisterGraphicsBindings(I_Graphics, Isolate, Context, ViseraObject);

        if (I_Audio)
            RegisterAudioBindings(I_Audio, Isolate, Context, ViseraObject);

        if (I_Input && I_InputState)
            RegisterInputBindings(I_Input, *I_InputState, Isolate, Context, ViseraObject);

        if (I_ScriptMainWindowSink)
        {
            v8::Local<v8::External> Ext = v8::External::New(Isolate, I_ScriptMainWindowSink);
            v8::Local<v8::FunctionTemplate> Tmpl = v8::FunctionTemplate::New(Isolate, Private::V8_SetMainWindow, Ext);
            v8::Local<v8::Function>         Fn   = Tmpl->GetFunction(Context).ToLocalChecked();
            v8::MaybeLocal<v8::String>      Key  = ToV8String(Isolate, "setMainWindow");
            if (!Key.IsEmpty())
            { (void)ViseraObject->Set(Context, Key.ToLocalChecked(), Fn); }
        }

        v8::MaybeLocal<v8::String> GlobalName = ToV8String(Isolate, "visera");
        if (GlobalName.IsEmpty()) return;
        if (Context->Global()->Set(Context, GlobalName.ToLocalChecked(), ViseraObject).IsNothing())
            LOG_WARN("(Binding) Failed to set global visera.");
    }
}
