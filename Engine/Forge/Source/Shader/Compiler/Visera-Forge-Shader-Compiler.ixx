module;
#include <Visera-Forge.hpp>
#include <Slang/slang.h>
#include <Slang/slang-com-ptr.h>
export module Visera.Forge.Shader.Compiler;
#define VISERA_MODULE_NAME "Forge.Shader"
import Visera.Core.Types.Path;
import Visera.Core.Types.Set;
import Visera.Core.Types.Array;
import Visera.Core.Types.String;
import Visera.Global;

export namespace Visera::Forge
{
    struct FRHIShaderLayout
    {
        struct FEntryPoint
        {
            FString Name;
            FString Stage; // "Vertex", "Fragment", etc.
        };
        struct FResource
        {
            FString Name;
            FString Type; // "Texture2D", "SamplerState", etc.
            UInt32  Binding;
            UInt32  Set = 0;
            UInt32  ArrayCount = 1;
            FString Access = "Read";
            TArray<FString> Stages;
        };
        struct FPushConstant
        {
            FString Name;
            UInt32  Size = 0;
            TArray<FString> Stages;
        };
        TArray<FEntryPoint> EntryPoints;
        TArray<FResource>   Resources;
        TArray<FPushConstant> PushConstants;
    };

    class FShaderCompiler
    {
    public:
    	[[nodiscard]] inline Bool
    	AddSearchPath(const FPath& I_Path);
    	[[nodiscard]] inline TArray<FByte>
    	Compile(const FPath& I_Path, FStringView I_EntryPoint, const FPath& I_SearchDirectory);
        [[nodiscard]] FRHIShaderLayout
    	ExtractReflection(const FPath& I_Path, FStringView I_EntryPoint, const FPath& I_SearchDirectory);

    private:
    	Slang::ComPtr<slang::IGlobalSession>
    	Context {nullptr}; //[Note] Currently, the global session type is not thread-safe. Applications that wish to compile on multiple threads will need to ensure that each concurrent thread compiles with a distinct global session.
    	TSet<FString>
    	SearchPaths{};

    	struct FSession
        {
            Slang::ComPtr<slang::ISession> Handle;
            slang::TargetDesc              Description;
    		Slang::ComPtr<slang::IBlob>    CompiledCode;
            Slang::ComPtr<slang::IComponentType> ShaderProgram;
        };
        FSession* Session {nullptr};

    public:
        FShaderCompiler();
    	~FShaderCompiler() { delete Session; slang::shutdown(); }

    private:
    	[[nodiscard]] Bool
    	CreateSession();
    	void inline
    	Process(FStringView I_File, FStringView I_EntryPoint);
    	[[nodiscard]] inline const char*
    	GetErrorMessage(const Slang::ComPtr<slang::IBlob>& I_Diagnostics) const { return static_cast<const char*>(I_Diagnostics->getBufferPointer()); }
    };

	Bool FShaderCompiler::
	AddSearchPath(const FPath& I_Path)
	{
		auto Path = I_Path.GetString();
		if (!SearchPaths.Contains(Path))
		{
			SearchPaths.Emplace(std::move(Path));
			LOG_DEBUG("Added a new shader path: {}", I_Path);
			return True;
		}
		LOG_ERROR("Failed to add shader path: {}", I_Path);
		return False;
	}

    TArray<FByte> FShaderCompiler::
    Compile(const FPath& I_Path, FStringView I_EntryPoint, const FPath& I_SearchDirectory)
    {
		const Bool HadPath = SearchPaths.Contains(I_SearchDirectory.GetString());
		if (!HadPath) { (void)AddSearchPath(I_SearchDirectory); }

		if (!Session || !HadPath)
		{
			delete Session;
			Session = nullptr;
			if (!CreateSession())
			{ LOG_FATAL("Failed to create the Slang Session!"); }
		}
		Process(*I_Path.GetFileName(), I_EntryPoint);
		const FByte* Buffer = static_cast<const FByte*>(Session->CompiledCode->getBufferPointer());

		auto ShaderCode = TArray<FByte>(
			Buffer,
			Buffer + Session->CompiledCode->getBufferSize());

		// Clean up
		Session->CompiledCode.setNull();

        return ShaderCode;
    }

    FShaderCompiler::
    FShaderCompiler()
    {
    	if (Context == nullptr)
    	{
    		LOG_DEBUG("Creating a Slang thread global context.");
    		if (slang::createGlobalSession(Context.writeRef()) != SLANG_OK)
    		{ LOG_FATAL("Failed to create the Slang Context (a.k.a, Global Session)!"); }
    	}
    	// Session and search paths are set by caller via AddSearchPath + Compile(..., I_SearchDirectory).
    }

	Bool FShaderCompiler::
	CreateSession()
    {
    	LOG_TRACE("Creating a new slang session.");
    	Session = new FSession(); VISERA_ASSERT(Session);

    	// Create Vulkan Compiler Session
    	Session->Description =
    	{
    		.format  = SLANG_SPIRV,
			.profile = Context->findProfile("glsl_450"),
		};

    	TArray<const char*> SlangSearchPaths;
		SlangSearchPaths.Reserve(SearchPaths.GetSize());
		for (const auto& Path : SearchPaths)
    	{ SlangSearchPaths.EmplaceBack(Path.Data()); }

    	slang::SessionDesc SessionCreateInfo
		{
			.targets		 = &Session->Description,
			.targetCount	 = 1,
			.searchPaths	 = SlangSearchPaths.Data(),
			.searchPathCount = static_cast<UInt32>(SearchPaths.GetSize()),
		};

    	if (Context->createSession(SessionCreateInfo, Session->Handle.writeRef()) != SLANG_OK)
    	{
    		LOG_ERROR("Failed to create the Slang Session!");
    		return False;
    	}

    	return True;
    }

     void FShaderCompiler::
	 Process(FStringView I_File, FStringView I_EntryPoint)
	 {
    	VISERA_ASSERT(Context && Session);

    	LOG_TRACE("Compiling the {} (entry_point: {}).", I_File, I_EntryPoint);
	 	Slang::ComPtr<slang::IBlob>  Diagnostics;

	 	// Create Shader Module
	 	Slang::ComPtr<slang::IModule> ShaderModule
    	{
    		Session->Handle->loadModule(I_File.Data(),
    		Diagnostics.writeRef())
    	};
	 	if (Diagnostics)
	 	{
	 		LOG_ERROR("Failed to create the Shader Module: {}!",
	 			      GetErrorMessage(Diagnostics));
			return;
	 	}

	 	// Create Shader Program
	 	Slang::ComPtr<slang::IEntryPoint> ShaderEntryPoint;
	 	if (ShaderModule->findEntryPointByName(
	 		I_EntryPoint.Data(),
	 		ShaderEntryPoint.writeRef()) != SLANG_OK)
	 	{
	 		// Not an error: caller may try multiple names (e.g. VertMain, FragMain, main).
	 		LOG_DEBUG("EntryPoint({}) not found in Shader({}), skipping.", I_EntryPoint.Data(), I_File);
			return;
	 	}

	 	slang::IComponentType* const ShaderComponents[2]
    	{
	 		ShaderModule.get(),
	 		ShaderEntryPoint.get(),
	 	};

	 	if (Session->Handle->createCompositeComponentType(
	 		ShaderComponents, 2,
	 		Session->ShaderProgram.writeRef(),
	 		Diagnostics.writeRef()) != SLANG_OK)
	 	{
	 		LOG_ERROR("Failed to create the Shader({}): {}!",
	 			      I_File, GetErrorMessage(Diagnostics));
	 		return;
	 	}

	 	if (Session->ShaderProgram->getEntryPointCode(
	 		0,
	 		0,
	 		Session->CompiledCode.writeRef(),
	 		Diagnostics.writeRef()) != SLANG_OK)
	 	{
	 		LOG_ERROR("Failed to obtain compiled code from {}: {}!",
	 		          I_File, GetErrorMessage(Diagnostics));
	 		return;
	 	}
	 }

    FRHIShaderLayout FShaderCompiler::
    ExtractReflection(const FPath& I_Path, FStringView I_EntryPoint, const FPath& I_SearchDirectory)
    {
        FRHIShaderLayout Reflection;
        (void)I_SearchDirectory; // Session already configured by Compile() before ExtractReflection is called.
        VISERA_ASSERT(Session && Session->ShaderProgram);

        Slang::ComPtr<slang::IBlob> Diagnostics;
        slang::ProgramLayout* ShaderLayout = Session->ShaderProgram->getLayout(0, Diagnostics.writeRef());
        if (Diagnostics || !ShaderLayout)
        {
            LOG_ERROR("Failed to get reflection info from Shader({})!", I_Path);
            return Reflection;
        }

        // Resolve current entry point only (this .vshader is built for a single entry point).
        slang::EntryPointLayout* CurrentEP = nullptr;
        const auto EntryPointCount = ShaderLayout->getEntryPointCount();
        for (SlangUInt i = 0; i < EntryPointCount; ++i)
        {
            auto* EP = ShaderLayout->getEntryPointByIndex(i);
            if (EP && FStringView(EP->getName()) == I_EntryPoint)
            {
                CurrentEP = EP;
                break;
            }
        }
        if (!CurrentEP && EntryPointCount > 0)
            CurrentEP = ShaderLayout->getEntryPointByIndex(0);

        if (!CurrentEP)
        {
            LOG_ERROR("No entry point found for Shader({}).", I_Path);
            return Reflection;
        }

        // Single entry point stage (no union, no All fallback).
        FStringView StageName;
        switch (CurrentEP->getStage())
        {
        case SLANG_STAGE_VERTEX:   StageName = "Vertex";   break;
        case SLANG_STAGE_FRAGMENT: StageName = "Fragment"; break;
        case SLANG_STAGE_COMPUTE:  StageName = "Compute";  break;
        case SLANG_STAGE_GEOMETRY:
        case SLANG_STAGE_HULL:
        case SLANG_STAGE_DOMAIN:
        default:
            LOG_ERROR("Unsupported stage for entry point {} in Shader({}).", I_EntryPoint, I_Path);
            return Reflection;
        }

        // EntryPoints[0] only.
        Reflection.EntryPoints.PushBack({ FString(CurrentEP->getName()), FString(StageName) });

        auto ArrayUnwrap = [](slang::TypeReflection* I_Type, UInt32& O_ArrayCount) -> slang::TypeReflection*
        {
            O_ArrayCount = 1;
            auto* T = I_Type;
            while (T && T->isArray())
            {
                const size_t N = T->getElementCount();
                if (N != 0) O_ArrayCount *= static_cast<UInt32>(N);
                T = T->getElementType();
            }
            return T;
        };

        auto ExtractResourceLike = [&](slang::VariableLayoutReflection* VarLayout)
        {
            if (!VarLayout) return;
            auto* Var = VarLayout->getVariable();
            if (!Var) return;
            auto* Type = Var->getType();
            if (!Type) return;

            if (VarLayout->getCategory() == slang::ParameterCategory::PushConstantBuffer)
            {
                const char* Name = Var->getName();
                const FStringView PCName = (Name && Name[0] != '\0') ? FStringView(Name) : FStringView("PushConstants");
                const UInt32 Size = static_cast<UInt32>(VarLayout->getTypeLayout()->getSize(slang::ParameterCategory::PushConstantBuffer));
                FRHIShaderLayout::FPushConstant PC;
                PC.Name = FString(PCName);
                PC.Size = Size;
                PC.Stages.PushBack(FString(StageName));
                Reflection.PushConstants.PushBack(std::move(PC));
                return;
            }

            UInt32 ArrayCount = 1;
            auto* BaseType = ArrayUnwrap(Type, ArrayCount);
            if (!BaseType) return;

            const auto TypeKind = BaseType->getKind();
            if (TypeKind != slang::TypeReflection::Kind::Resource &&
                TypeKind != slang::TypeReflection::Kind::SamplerState &&
                TypeKind != slang::TypeReflection::Kind::ConstantBuffer &&
                TypeKind != slang::TypeReflection::Kind::TextureBuffer &&
                TypeKind != slang::TypeReflection::Kind::ShaderStorageBuffer)
            { return; }

            FRHIShaderLayout::FResource Res;
            Res.Name = FString(Var->getName());
            Res.Binding = VarLayout->getBindingIndex();
            Res.Set = static_cast<UInt32>(VarLayout->getBindingSpace());
            Res.ArrayCount = ArrayCount == 0 ? 1u : ArrayCount;
            Res.Stages.PushBack(FString(StageName));

            switch (TypeKind)
            {
            case slang::TypeReflection::Kind::Resource:
                {
                    const auto Shape = BaseType->getResourceShape();
                    const auto BaseShape = Shape & SLANG_RESOURCE_BASE_SHAPE_MASK;
                    switch (BaseShape)
                    {
                    case SLANG_TEXTURE_2D: Res.Type = "Texture2D"; break;
                    case SLANG_TEXTURE_CUBE: Res.Type = "TextureCube"; break;
                    case SLANG_TEXTURE_3D: Res.Type = "Texture3D"; break;
                    case SLANG_TEXTURE_1D: Res.Type = "Texture1D"; break;
                    default: return;
                    }
                    const auto Access = BaseType->getResourceAccess();
                    switch (Access)
                    {
                    case SLANG_RESOURCE_ACCESS_READ: Res.Access = "Read"; break;
                    case SLANG_RESOURCE_ACCESS_WRITE: Res.Access = "Write"; break;
                    case SLANG_RESOURCE_ACCESS_READ_WRITE: Res.Access = "ReadWrite"; break;
                    default: Res.Access = "Read"; break;
                    }
                }
                break;
            case slang::TypeReflection::Kind::SamplerState:
                Res.Type = "SamplerState";
                Res.Access = "Read";
                break;
            case slang::TypeReflection::Kind::ConstantBuffer:
                Res.Type = "ConstantBuffer";
                Res.Access = "Read";
                break;
            case slang::TypeReflection::Kind::TextureBuffer:
                Res.Type = "TextureBuffer";
                Res.Access = "Read";
                break;
            case slang::TypeReflection::Kind::ShaderStorageBuffer:
                Res.Type = "StructuredBuffer";
                Res.Access = "ReadWrite";
                break;
            default:
                return;
            }

            Reflection.Resources.PushBack(std::move(Res));
        };

        // Only current entry point's parameters (program layout has one entry point when built for one).
        const auto ParamCount = ShaderLayout->getParameterCount();
        for (unsigned i = 0; i < ParamCount; ++i)
        {
            ExtractResourceLike(ShaderLayout->getParameterByIndex(i));
        }

        return Reflection;
    }
}
