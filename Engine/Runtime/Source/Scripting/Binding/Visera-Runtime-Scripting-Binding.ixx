/** Script bindings: exposes global `visera` (log, draw) to JS. Call RegisterAllBindings after context creation. */
module;
#include <v8.h>

#include <Visera-Runtime.hpp>
export module Visera.Runtime.Scripting.Binding;
#define VISERA_MODULE_NAME "Runtime.Scripting.Binding"
import Visera.Runtime.Scripting.VM;
import Visera.Runtime.Scripting.Context;
import Visera.Runtime.Graphics;
import Visera.Runtime.Graphics.Scene.Renderable;
import Visera.Core.Math.Geometry.Transform;
import Visera.Core.Math.Trigonometry;
import Visera.Core.Types.String;
import Visera.Core.Types.Optional;
import Visera.Core.Types.Path;
import Visera.Platform;
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

    /** Read a number from I_Obj[I_Key]; returns NullOpt if missing or not a number. */
    TOptional<Double> GetObjectNumber(v8::Isolate* I_Isolate, v8::Local<v8::Context> I_Context,
        v8::Local<v8::Object> I_Obj, const char* I_Key)
    {
        v8::MaybeLocal<v8::String> Key = ToV8String(I_Isolate, I_Key);
        if (Key.IsEmpty()) return NullOpt;
        v8::Local<v8::Value> Val;
        if (!I_Obj->Get(I_Context, Key.ToLocalChecked()).ToLocal(&Val) || Val.IsEmpty()) return NullOpt;
        return FromV8Double(I_Context, Val);
    }

    /** Read a string from I_Obj[I_Key]; returns NullOpt if missing or not a string. */
    TOptional<FString> GetObjectString(v8::Isolate* I_Isolate, v8::Local<v8::Context> I_Context,
        v8::Local<v8::Object> I_Obj, const char* I_Key)
    {
        v8::MaybeLocal<v8::String> Key = ToV8String(I_Isolate, I_Key);
        if (Key.IsEmpty()) return NullOpt;
        v8::Local<v8::Value> Val;
        if (!I_Obj->Get(I_Context, Key.ToLocalChecked()).ToLocal(&Val) || Val.IsEmpty()) return NullOpt;
        return FromV8String(I_Isolate, Val);
    }

    /** C++ callback for visera.draw(sprite): reads sprite object (x, y, width, height, material), loads material, submits draw. */
    void V8_Visera_Draw(const v8::FunctionCallbackInfo<v8::Value>& I_Info)
    {
        try
        {
            if (I_Info.Length() < 1 || !I_Info[0]->IsObject())
            {
                LOG_WARN("(Binding) visera.draw(sprite) expects a sprite object.");
                return;
            }
            FGraphics* Graphics = static_cast<FGraphics*>(I_Info.Data().As<v8::External>()->Value());
            if (!Graphics) return;

            v8::Isolate* Isolate = I_Info.GetIsolate();
            v8::Local<v8::Context> Context = Isolate->GetCurrentContext();
            v8::Local<v8::Object> Sprite = I_Info[0].As<v8::Object>();

            Double X = GetObjectNumber(Isolate, Context, Sprite, "x").Get(0.0);
            Double Y = GetObjectNumber(Isolate, Context, Sprite, "y").Get(0.0);
            Double W = GetObjectNumber(Isolate, Context, Sprite, "width").Get(0.0);
            Double H = GetObjectNumber(Isolate, Context, Sprite, "height").Get(0.0);
            TOptional<FString> MaterialOpt = GetObjectString(Isolate, Context, Sprite, "material");
            if (!MaterialOpt.HasValue())
                MaterialOpt = GetObjectString(Isolate, Context, Sprite, "materialPath");
            if (!MaterialOpt.HasValue() || MaterialOpt.GetValue().IsEmpty())
            {
                LOG_WARN("(Binding) visera.draw(sprite) requires sprite.material or sprite.materialPath.");
                return;
            }

            FPath MaterialPath = FPlatform::GetResourceDirectory() / FPath(MaterialOpt.GetValue());
            TSharedPtr<FMaterial> Material = Graphics->LoadMaterial(MaterialPath);
            if (!Material)
            {
                LOG_WARN("(Binding) visera.draw: LoadMaterial failed for '{}'.", MaterialOpt.GetValue());
                return;
            }

            FInstanceData InstanceData{
                .Transform  = FTransform3x4F::MakeTransform2D(
                    {static_cast<Float>(X), static_cast<Float>(Y)},
                    {static_cast<Float>(W), static_cast<Float>(H)},
                    FDegree{0.f}, 0.f),
                .Color      = {1.f, 1.f, 1.f, 1.f},
                .CustomData = {0.f, 0.f, 1.f, 1.f}
            };
            FRenderableMeta Meta{
                .InstanceData = InstanceData,
                .Material     = std::move(Material),
                .Mesh         = nullptr
            };
            Graphics->Draw(Meta);
        }
        catch (...)
        {
            LOG_ERROR("(Binding) visera.draw threw.");
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
        {
            v8::Local<v8::FunctionTemplate> DrawTmpl = v8::FunctionTemplate::New(
                Isolate, Private::V8_Visera_Draw, v8::External::New(Isolate, I_Graphics));
            v8::Local<v8::Function> DrawFn = DrawTmpl->GetFunction(Context).ToLocalChecked();
            v8::MaybeLocal<v8::String> DrawKey = ToV8String(Isolate, "draw");
            if (!DrawKey.IsEmpty() && !ViseraObject->Set(Context, DrawKey.ToLocalChecked(), DrawFn).IsNothing())
                { /* draw registered */ }
            else
                LOG_WARN("(Binding) Failed to set visera.draw.");
        }

        v8::MaybeLocal<v8::String> GlobalName = ToV8String(Isolate, "visera");
        if (GlobalName.IsEmpty()) return;
        if (Context->Global()->Set(Context, GlobalName.ToLocalChecked(), ViseraObject).IsNothing())
            LOG_WARN("(Binding) Failed to set global visera.");
    }
}
