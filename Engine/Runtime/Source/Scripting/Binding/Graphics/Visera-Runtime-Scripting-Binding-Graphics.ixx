/** Graphics script bindings: visera.drawSprite(descriptor). RegisterGraphicsBindings is called from main Binding. */
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
    /** Register Graphics API (e.g. visera.drawSprite) on IO_ViseraObject. I_Graphics must be non-null. */
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

    /** If I_Parent[I_Key] is a non-null object, sets IO_Child and returns true. */
    bool TryGetObjectChild(v8::Isolate* I_Isolate, v8::Local<v8::Context> I_Context,
        v8::Local<v8::Object> I_Parent, const char* I_Key, v8::Local<v8::Object>& IO_Child)
    {
        v8::MaybeLocal<v8::String> Key = ToV8String(I_Isolate, I_Key);
        if (Key.IsEmpty()) return false;
        v8::Local<v8::Value> Val;
        if (!I_Parent->Get(I_Context, Key.ToLocalChecked()).ToLocal(&Val) || Val.IsEmpty()) return false;
        if (!Val->IsObject()) return false;
        IO_Child = Val.As<v8::Object>();
        return true;
    }

    /** C++ callback for visera.drawSprite(descriptor): nested transform, extent, frame, material. */
    void V8_Visera_DrawSprite(const v8::FunctionCallbackInfo<v8::Value>& I_Info)
    {
        try
        {
            if (I_Info.Length() < 1 || !I_Info[0]->IsObject())
            {
                LOG_WARN("(Binding) visera.drawSprite(descriptor) expects an object.");
                return;
            }
            FGraphics* Graphics = static_cast<FGraphics*>(I_Info.Data().As<v8::External>()->Value());
            if (!Graphics) return;

            v8::Isolate* Isolate = I_Info.GetIsolate();
            v8::Local<v8::Context> Context = Isolate->GetCurrentContext();
            v8::Local<v8::Object> Descriptor = I_Info[0].As<v8::Object>();

            Double PositionX = 0.0;
            Double PositionY = 0.0;
            Double PositionZ = 0.0;
            Double RotationDegrees = 0.0;
            v8::Local<v8::Object> TransformObject;
            if (TryGetObjectChild(Isolate, Context, Descriptor, "transform", TransformObject))
            {
                v8::Local<v8::Object> PositionObject;
                if (TryGetObjectChild(Isolate, Context, TransformObject, "position", PositionObject))
                {
                    PositionX = GetObjectNumber(Isolate, Context, PositionObject, "x").Get(0.0);
                    PositionY = GetObjectNumber(Isolate, Context, PositionObject, "y").Get(0.0);
                    PositionZ = GetObjectNumber(Isolate, Context, PositionObject, "z").Get(0.0);
                }
                RotationDegrees = GetObjectNumber(Isolate, Context, TransformObject, "rotation").Get(0.0);
            }

            Double ExtentWidth = 0.0;
            Double ExtentHeight = 0.0;
            v8::Local<v8::Object> ExtentObject;
            if (TryGetObjectChild(Isolate, Context, Descriptor, "extent", ExtentObject))
            {
                ExtentWidth  = GetObjectNumber(Isolate, Context, ExtentObject, "width").Get(0.0);
                ExtentHeight = GetObjectNumber(Isolate, Context, ExtentObject, "height").Get(0.0);
            }

            const Double Frame = GetObjectNumber(Isolate, Context, Descriptor, "frame").Get(0.0);

            Float ColorR = 1.f;
            Float ColorG = 1.f;
            Float ColorB = 1.f;
            Float ColorA = 1.f;
            v8::Local<v8::Object> ColorObject;
            if (TryGetObjectChild(Isolate, Context, Descriptor, "color", ColorObject))
            {
                ColorR = static_cast<Float>(GetObjectNumber(Isolate, Context, ColorObject, "r").Get(1.0));
                ColorG = static_cast<Float>(GetObjectNumber(Isolate, Context, ColorObject, "g").Get(1.0));
                ColorB = static_cast<Float>(GetObjectNumber(Isolate, Context, ColorObject, "b").Get(1.0));
                ColorA = static_cast<Float>(GetObjectNumber(Isolate, Context, ColorObject, "a").Get(1.0));
            }

            Float CustomDataX = static_cast<Float>(Frame);
            Float CustomDataY = 0.f;
            Float CustomDataZ = 1.f;
            Float CustomDataW = 1.f;
            v8::Local<v8::Object> CustomDataObject;
            if (TryGetObjectChild(Isolate, Context, Descriptor, "customData", CustomDataObject))
            {
                CustomDataX = static_cast<Float>(GetObjectNumber(Isolate, Context, CustomDataObject, "x").Get(static_cast<Double>(CustomDataX)));
                CustomDataY = static_cast<Float>(GetObjectNumber(Isolate, Context, CustomDataObject, "y").Get(static_cast<Double>(CustomDataY)));
                CustomDataZ = static_cast<Float>(GetObjectNumber(Isolate, Context, CustomDataObject, "z").Get(static_cast<Double>(CustomDataZ)));
                CustomDataW = static_cast<Float>(GetObjectNumber(Isolate, Context, CustomDataObject, "w").Get(static_cast<Double>(CustomDataW)));
            }

            TOptional<FString> MaterialOpt = GetObjectString(Isolate, Context, Descriptor, "material");
            if (!MaterialOpt.HasValue())
                MaterialOpt = GetObjectString(Isolate, Context, Descriptor, "materialPath");
            if (!MaterialOpt.HasValue() || MaterialOpt.GetValue().IsEmpty())
            {
                LOG_WARN("(Binding) visera.drawSprite requires descriptor.material or descriptor.materialPath.");
                return;
            }

            VPath MaterialPath{MaterialOpt.GetValue()};
            auto Material = Graphics->LoadMaterial(MaterialPath);
            if (!Material)
            {
                LOG_WARN("(Binding) visera.drawSprite: LoadMaterial failed for '{}'.", MaterialOpt.GetValue());
                return;
            }

            FInstanceData InstanceData{
                .Transform  = FTransform3x4F::MakeTransform2D(
                    {static_cast<Float>(PositionX), static_cast<Float>(PositionY)},
                    {static_cast<Float>(ExtentWidth), static_cast<Float>(ExtentHeight)},
                    FDegree{static_cast<Float>(RotationDegrees)},
                    static_cast<Float>(PositionZ)),
                .Color      = {ColorR, ColorG, ColorB, ColorA},
                .CustomData = {CustomDataX, CustomDataY, CustomDataZ, CustomDataW}
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
            LOG_ERROR("(Binding) visera.drawSprite threw.");
        }
    }
}

    void RegisterGraphicsBindings(FGraphics* I_Graphics, v8::Isolate* I_Isolate,
        v8::Local<v8::Context> I_Context, v8::Local<v8::Object> IO_ViseraObject)
    {
        if (!I_Graphics || I_Isolate == nullptr || I_Context.IsEmpty() || IO_ViseraObject.IsEmpty())
            return;

        v8::Local<v8::FunctionTemplate> DrawSpriteTmpl = v8::FunctionTemplate::New(
            I_Isolate, Private::V8_Visera_DrawSprite, v8::External::New(I_Isolate, I_Graphics));
        v8::Local<v8::Function> DrawSpriteFn = DrawSpriteTmpl->GetFunction(I_Context).ToLocalChecked();
        v8::MaybeLocal<v8::String> DrawSpriteKey = ToV8String(I_Isolate, "drawSprite");
        if (!DrawSpriteKey.IsEmpty()
            && !IO_ViseraObject->Set(I_Context, DrawSpriteKey.ToLocalChecked(), DrawSpriteFn).IsNothing())
        { /* drawSprite registered */ }
        else
            LOG_WARN("(Binding) Failed to set visera.drawSprite.");
    }
}
