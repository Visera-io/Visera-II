/** Script bindings: exposes global `visera` (log, draw) to JS. Call RegisterAllBindings after context creation. */
module;
#include <v8.h>

#include <Visera-Runtime.hpp>
export module Visera.Runtime.Scripting.Binding;
#define VISERA_MODULE_NAME "Runtime.Scripting.Binding"
import Visera.Runtime.Scripting.VM;
import Visera.Runtime.Scripting.Context;
import Visera.Runtime.Scripting.Binding.Graphics;
import Visera.Runtime.Graphics;
import Visera.Core.Types.String;
import Visera.Core.Log;

export namespace Visera
{
    /** Register all script bindings on the given context. I_Graphics may be null (e.g. when only log API is used). */
    void RegisterAllBindings(FGraphics* I_Graphics, FJavaScriptContext& I_Context);

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

}

    void RegisterAllBindings(FGraphics* I_Graphics, FJavaScriptContext& I_Context)
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

        v8::MaybeLocal<v8::String> GlobalName = ToV8String(Isolate, "visera");
        if (GlobalName.IsEmpty()) return;
        if (Context->Global()->Set(Context, GlobalName.ToLocalChecked(), ViseraObject).IsNothing())
            LOG_WARN("(Binding) Failed to set global visera.");
    }
}
