/** Input script bindings: visera.input.addMapping, addAction, mouse.cursor, etc. */
module;
#include <v8.h>
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Scripting.Binding.Input;
#define VISERA_MODULE_NAME "Runtime.Scripting"
import Visera.Runtime.Scripting.VM;
import Visera.Runtime.Scripting.Utils;
import Visera.Runtime.Input;
import Visera.Runtime.Input.Mapping;
import Visera.Core.Types.String;
import Visera.Core.Types.Optional;
import Visera.Core.Types.JSON;
import Visera.Core.Types.Name;
import Visera.Core.Containers.Map;
import Visera.Core.Log;

export namespace Visera
{
    /**
     * Owns JS input callbacks and engine-side OnTriggered subscriptions. Destructor unsubscribes and resets v8::Globals.
     */
    class VISERA_RUNTIME_API FJavaScriptInputBindingState
    {
    public:
        explicit FJavaScriptInputBindingState(FInput* I_Input) : Input(I_Input) {}

        FJavaScriptInputBindingState(const FJavaScriptInputBindingState&)            = delete;
        FJavaScriptInputBindingState& operator=(const FJavaScriptInputBindingState&) = delete;

        ~FJavaScriptInputBindingState();

        [[nodiscard]] FInput*
        GetInput() const { return Input; }

        /** Adds visera.input. I_Input must outlive this state. */
        void RegisterOnViseraObject(v8::Isolate* I_Isolate, v8::Local<v8::Context> I_Context, v8::Local<v8::Object> IO_Visera);

        /** Invoked from the engine OnTriggered bridge (script layer). */
        void FireActionScript(FName I_ActionName, FInputAction* I_Action);

        /** Register or replace the JS function for an action; wires OnTriggered once per action. */
        void BindScriptAction(v8::Isolate* I_Isolate, FName I_ActionName, v8::Local<v8::Function> I_Function);

    private:

        FInput*                               Input {nullptr};
        v8::Isolate*                          Isolate {nullptr};
        v8::Global<v8::Context>               StoredContext;
        TMap<FName, UInt64>                   OnTriggeredHandles;
        TMap<FName, v8::Global<v8::Function>> ScriptCallbacks;
    };

    void RegisterInputBindings(FInput* I_Input, FJavaScriptInputBindingState& I_State, v8::Isolate* I_Isolate,
        v8::Local<v8::Context> I_Context, v8::Local<v8::Object> IO_Visera);
}

namespace Visera::PrivateInputBinding
{
    static FJavaScriptInputBindingState*
    StateFromData(v8::Local<v8::Value> I_Data)
    {
        if (I_Data.IsEmpty() || !I_Data->IsExternal())
        { return nullptr; }
        return static_cast<FJavaScriptInputBindingState*>(I_Data.As<v8::External>()->Value());
    }

    static FInput*
    InputFromData(v8::Local<v8::Value> I_Data)
    {
        if (I_Data.IsEmpty() || !I_Data->IsExternal())
        { return nullptr; }
        return static_cast<FInput*>(I_Data.As<v8::External>()->Value());
    }

    void V8_AddMapping(const v8::FunctionCallbackInfo<v8::Value>& I_Info)
    {
        try
        {
            FJavaScriptInputBindingState* State = StateFromData(I_Info.Data());
            FInput*                       Input = State ? State->GetInput() : nullptr;
            if (!Input || I_Info.Length() < 1 || !I_Info[0]->IsObject())
            {
                LOG_WARN("(Binding) visera.input.addMapping expects a descriptor object.");
                return;
            }
            v8::Isolate*           Isolate = I_Info.GetIsolate();
            v8::Local<v8::Context> Context = Isolate->GetCurrentContext();
            v8::MaybeLocal<v8::String> JsonStr = v8::JSON::Stringify(Context, I_Info[0]);
            if (JsonStr.IsEmpty())
            {
                LOG_WARN("(Binding) visera.input.addMapping: JSON.stringify failed.");
                return;
            }
            v8::String::Utf8Value Utf8(Isolate, JsonStr.ToLocalChecked());
            if (*Utf8 == nullptr)
            { return; }
            const FStringView              Sv(*Utf8, static_cast<FString::SizeType>(Utf8.length()));
            const TOptional<FJSON>         JsonOpt = FJSON::Parse(Sv);
            if (!JsonOpt.HasValue())
            {
                LOG_WARN("(Binding) visera.input.addMapping: JSON parse failed.");
                return;
            }
            const TOptional<FInputMapping> MappingOpt = ParseInputMappingDescriptor(JsonOpt.GetValue());
            if (!MappingOpt.HasValue())
            {
                LOG_WARN("(Binding) visera.input.addMapping: invalid descriptor.");
                return;
            }
            Input->AddMapping(MappingOpt.GetValue());
        }
        catch (...)
        {
            LOG_ERROR("(Binding) visera.input.addMapping threw.");
        }
    }

    void V8_AddAction(const v8::FunctionCallbackInfo<v8::Value>& I_Info)
    {
        try
        {
            FJavaScriptInputBindingState* State = StateFromData(I_Info.Data());
            FInput*                       Input = State ? State->GetInput() : nullptr;
            if (!Input || I_Info.Length() < 2 || !I_Info[0]->IsString() || !I_Info[1]->IsFunction())
            {
                LOG_WARN("(Binding) visera.input.addAction(name, fn) expects a string and a function.");
                return;
            }
            v8::Isolate* Isolate = I_Info.GetIsolate();
            TOptional<FString> NameOpt = FromV8String(Isolate, I_Info[0]);
            if (!NameOpt.HasValue() || NameOpt.GetValue().IsEmpty())
            { return; }
            const FName ActionName(NameOpt.GetValue().GetNative());

            if (!State)
            { return; }
            v8::Local<v8::Function> Fn = I_Info[1].As<v8::Function>();
            State->BindScriptAction(Isolate, ActionName, Fn);
        }
        catch (...)
        {
            LOG_ERROR("(Binding) visera.input.addAction threw.");
        }
    }

    void V8_RemoveMappings(const v8::FunctionCallbackInfo<v8::Value>& I_Info)
    {
        try
        {
            FJavaScriptInputBindingState* State = StateFromData(I_Info.Data());
            FInput*                       Input = State ? State->GetInput() : nullptr;
            if (!Input || I_Info.Length() < 1 || !I_Info[0]->IsString())
            {
                LOG_WARN("(Binding) visera.input.removeMappings(name) expects a string.");
                return;
            }
            v8::Isolate* Isolate = I_Info.GetIsolate();
            TOptional<FString> NameOpt = FromV8String(Isolate, I_Info[0]);
            if (!NameOpt.HasValue() || NameOpt.GetValue().IsEmpty())
            { return; }
            Input->RemoveMappings(FName(NameOpt.GetValue().GetNative()));
        }
        catch (...)
        {
            LOG_ERROR("(Binding) visera.input.removeMappings threw.");
        }
    }

    void V8_IsActionActive(const v8::FunctionCallbackInfo<v8::Value>& I_Info)
    {
        try
        {
            FJavaScriptInputBindingState* State = StateFromData(I_Info.Data());
            FInput*                       Input = State ? State->GetInput() : nullptr;
            if (!Input || I_Info.Length() < 1 || !I_Info[0]->IsString())
            {
                LOG_WARN("(Binding) visera.input.isActionActive(name) expects a string.");
                return;
            }
            v8::Isolate* Isolate = I_Info.GetIsolate();
            TOptional<FString> NameOpt = FromV8String(Isolate, I_Info[0]);
            if (!NameOpt.HasValue() || NameOpt.GetValue().IsEmpty())
            { return; }
            const Bool Active = Input->IsActionActive(FName(NameOpt.GetValue().GetNative()));
            I_Info.GetReturnValue().Set(v8::Boolean::New(Isolate, Active));
        }
        catch (...)
        {
            LOG_ERROR("(Binding) visera.input.isActionActive threw.");
        }
    }

    void V8_CursorPositionGetter(v8::Local<v8::Name>, const v8::PropertyCallbackInfo<v8::Value>& I_Info)
    {
        try
        {
            FInput* Input = InputFromData(I_Info.Data());
            if (!Input)
            { return; }
            v8::Isolate*           Isolate = I_Info.GetIsolate();
            v8::Local<v8::Context> Context = Isolate->GetCurrentContext();
            const auto             P       = Input->GetMouse()->GetCursor().Position;
            v8::Local<v8::Object>  Out     = v8::Object::New(Isolate);
            (void)Out->Set(Context, ToV8String(Isolate, "x").ToLocalChecked(), v8::Number::New(Isolate, static_cast<double>(P.X)));
            (void)Out->Set(Context, ToV8String(Isolate, "y").ToLocalChecked(), v8::Number::New(Isolate, static_cast<double>(P.Y)));
            I_Info.GetReturnValue().Set(Out);
        }
        catch (...)
        {
            LOG_ERROR("(Binding) cursor.position getter threw.");
        }
    }

    void V8_CursorOffsetGetter(v8::Local<v8::Name>, const v8::PropertyCallbackInfo<v8::Value>& I_Info)
    {
        try
        {
            FInput* Input = InputFromData(I_Info.Data());
            if (!Input)
            { return; }
            v8::Isolate*           Isolate = I_Info.GetIsolate();
            v8::Local<v8::Context> Context = Isolate->GetCurrentContext();
            const auto             O       = Input->GetMouse()->GetCursor().Offset;
            v8::Local<v8::Object>  Out     = v8::Object::New(Isolate);
            (void)Out->Set(Context, ToV8String(Isolate, "x").ToLocalChecked(), v8::Number::New(Isolate, static_cast<double>(O.X)));
            (void)Out->Set(Context, ToV8String(Isolate, "y").ToLocalChecked(), v8::Number::New(Isolate, static_cast<double>(O.Y)));
            I_Info.GetReturnValue().Set(Out);
        }
        catch (...)
        {
            LOG_ERROR("(Binding) cursor.offset getter threw.");
        }
    }
}

void Visera::FJavaScriptInputBindingState::BindScriptAction(v8::Isolate* I_Isolate, FName I_ActionName, v8::Local<v8::Function> I_Function)
{
    if (!Input || !I_Isolate)
    { return; }
    FInputAction* Action = Input->AddAction(I_ActionName);
    if (!Action)
    { return; }
    if (!OnTriggeredHandles.Contains(I_ActionName))
    {
        const UInt64 Handle = Action->OnTriggered.Subscribe(
            [this, I_ActionName](FInputAction* I_Action) { FireActionScript(I_ActionName, I_Action); });
        OnTriggeredHandles.InsertOrAssign(I_ActionName, Handle);
    }
    ScriptCallbacks.InsertOrAssign(I_ActionName, v8::Global<v8::Function>(I_Isolate, I_Function));
}

Visera::FJavaScriptInputBindingState::~FJavaScriptInputBindingState()
{
    if (!Input)
    { return; }

    for (auto Iterator = OnTriggeredHandles.begin(); Iterator != OnTriggeredHandles.end(); ++Iterator)
    {
        if (FInputAction* Action = Input->GetAction(Iterator->first))
        { Action->OnTriggered.Unsubscribe(Iterator->second); }
    }
    OnTriggeredHandles.Clear();

    if (Isolate)
    {
        v8::Isolate::Scope IsolateScope(Isolate);
        v8::HandleScope    HandleScope(Isolate);
        for (auto Iterator = ScriptCallbacks.begin(); Iterator != ScriptCallbacks.end(); ++Iterator)
        { Iterator->second.Reset(); }
    }
    ScriptCallbacks.Clear();
    StoredContext.Reset();
    Isolate = nullptr;
    Input   = nullptr;
}

void Visera::FJavaScriptInputBindingState::FireActionScript(FName I_ActionName, FInputAction* I_Action)
{
    if (!Isolate || StoredContext.IsEmpty())
    { return; }
    auto CallbackIterator = ScriptCallbacks.Find(I_ActionName);
    if (CallbackIterator == ScriptCallbacks.end())
    { return; }

    v8::Isolate::Scope IsolateScope(Isolate);
    v8::HandleScope    HandleScope(Isolate);
    v8::Local<v8::Context> Context = StoredContext.Get(Isolate);
    if (Context.IsEmpty())
    { return; }
    v8::Context::Scope ContextScope(Context);

    v8::Local<v8::Function> Fn = CallbackIterator->second.Get(Isolate);
    if (Fn.IsEmpty())
    { return; }

    v8::Local<v8::Object> InfoObj = v8::Object::New(Isolate);
    (void)InfoObj->Set(Context, ToV8String(Isolate, "name").ToLocalChecked(),
        ToV8String(Isolate, I_Action->GetName().GetNameString()).ToLocalChecked());

    v8::Local<v8::Value> Arg = InfoObj;
    v8::TryCatch         TryCatch(Isolate);
    v8::Local<v8::Value> Recv = Context->Global();
    (void)Fn->Call(Context, Recv, 1, &Arg);
    if (TryCatch.HasCaught())
    { LOG_ERROR("(Binding) input action script callback threw."); }
}

void Visera::FJavaScriptInputBindingState::RegisterOnViseraObject(v8::Isolate* I_Isolate, v8::Local<v8::Context> I_Context,
    v8::Local<v8::Object> IO_Visera)
{
    if (!Input || !I_Isolate || I_Context.IsEmpty() || IO_Visera.IsEmpty())
    { return; }

    Isolate = I_Isolate;
    StoredContext.Reset(I_Isolate, I_Context);

    v8::Local<v8::External> StateExt = v8::External::New(I_Isolate, this);

    v8::Local<v8::Object> InputObj = v8::Object::New(I_Isolate);
    auto AddMethod = [&](const char* I_Name, void (*I_Cb)(const v8::FunctionCallbackInfo<v8::Value>&))
    {
        v8::MaybeLocal<v8::String> Key = ToV8String(I_Isolate, I_Name);
        if (Key.IsEmpty())
        { return; }
        v8::Local<v8::FunctionTemplate> Tmpl = v8::FunctionTemplate::New(I_Isolate, I_Cb, StateExt);
        v8::Local<v8::Function>          Fn  = Tmpl->GetFunction(I_Context).ToLocalChecked();
        (void)InputObj->Set(I_Context, Key.ToLocalChecked(), Fn);
    };

    AddMethod("addMapping", Visera::PrivateInputBinding::V8_AddMapping);
    AddMethod("addAction", Visera::PrivateInputBinding::V8_AddAction);
    AddMethod("removeMappings", Visera::PrivateInputBinding::V8_RemoveMappings);
    AddMethod("isActionActive", Visera::PrivateInputBinding::V8_IsActionActive);

    v8::Local<v8::Object> MouseObj = v8::Object::New(I_Isolate);

    v8::Local<v8::External> InputExt = v8::External::New(I_Isolate, Input);
    v8::Local<v8::ObjectTemplate> CursorTemplate = v8::ObjectTemplate::New(I_Isolate);
    v8::MaybeLocal<v8::String>  PosName         = ToV8String(I_Isolate, "position");
    v8::MaybeLocal<v8::String>  OffName         = ToV8String(I_Isolate, "offset");
    if (!PosName.IsEmpty())
    {
        CursorTemplate->SetNativeDataProperty(PosName.ToLocalChecked(), Visera::PrivateInputBinding::V8_CursorPositionGetter, nullptr, InputExt);
    }
    if (!OffName.IsEmpty())
    {
        CursorTemplate->SetNativeDataProperty(OffName.ToLocalChecked(), Visera::PrivateInputBinding::V8_CursorOffsetGetter, nullptr, InputExt);
    }
    v8::Local<v8::Object> CursorObj;
    if (!CursorTemplate->NewInstance(I_Context).ToLocal(&CursorObj))
    { CursorObj = v8::Object::New(I_Isolate); }

    v8::MaybeLocal<v8::String> CursorKey = ToV8String(I_Isolate, "cursor");
    if (!CursorKey.IsEmpty())
    { (void)MouseObj->Set(I_Context, CursorKey.ToLocalChecked(), CursorObj); }

    v8::MaybeLocal<v8::String> MouseKey = ToV8String(I_Isolate, "mouse");
    if (!MouseKey.IsEmpty())
    { (void)InputObj->Set(I_Context, MouseKey.ToLocalChecked(), MouseObj); }

    v8::MaybeLocal<v8::String> InputKey = ToV8String(I_Isolate, "input");
    if (!InputKey.IsEmpty())
    { (void)IO_Visera->Set(I_Context, InputKey.ToLocalChecked(), InputObj); }
}

void Visera::RegisterInputBindings(FInput* I_Input, FJavaScriptInputBindingState& I_State, v8::Isolate* I_Isolate,
    v8::Local<v8::Context> I_Context, v8::Local<v8::Object> IO_Visera)
{
    if (!I_Input)
    { return; }
    I_State.RegisterOnViseraObject(I_Isolate, I_Context, IO_Visera);
}
