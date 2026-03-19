/** Graphics script bindings: visera.draw(sprite). RegisterGraphicsBindings is called from main Binding. */
module;
#include <v8.h>
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Scripting.Binding.Graphics;
#define VISERA_MODULE_NAME "Runtime.Scripting"
import Visera.Runtime.Scripting.VM;
import Visera.Runtime.Graphics;
import Visera.Runtime.Graphics.Scene.Renderable;
import Visera.Core.Math.Geometry.Transform;
import Visera.Core.Math.Trigonometry;
import Visera.Core.Types.String;
import Visera.Core.Types.Optional;
import Visera.Runtime.AssetHub;
import Visera.Core.Log;

export namespace Visera
{
    /** Register Graphics API (e.g. visera.draw) on IO_ViseraObject. I_Graphics must be non-null. */
    void RegisterGraphicsBindings(FGraphics* I_Graphics, v8::Isolate* I_Isolate,
        v8::Local<v8::Context> I_Context, v8::Local<v8::Object> IO_ViseraObject);

namespace Private
{
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

            VPath MaterialPath{FStringView{MaterialOpt.GetValue()}};
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

    void RegisterGraphicsBindings(FGraphics* I_Graphics, v8::Isolate* I_Isolate,
        v8::Local<v8::Context> I_Context, v8::Local<v8::Object> IO_ViseraObject)
    {
        if (!I_Graphics || I_Isolate == nullptr || I_Context.IsEmpty() || IO_ViseraObject.IsEmpty())
            return;

        v8::Local<v8::FunctionTemplate> DrawTmpl = v8::FunctionTemplate::New(
            I_Isolate, Private::V8_Visera_Draw, v8::External::New(I_Isolate, I_Graphics));
        v8::Local<v8::Function> DrawFn = DrawTmpl->GetFunction(I_Context).ToLocalChecked();
        v8::MaybeLocal<v8::String> DrawKey = ToV8String(I_Isolate, "draw");
        if (!DrawKey.IsEmpty() && !IO_ViseraObject->Set(I_Context, DrawKey.ToLocalChecked(), DrawFn).IsNothing())
            { /* draw registered */ }
        else
            LOG_WARN("(Binding) Failed to set visera.draw.");
    }
}
