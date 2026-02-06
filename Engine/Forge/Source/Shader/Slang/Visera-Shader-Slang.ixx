module;
#include <Visera-Forge.hpp>
#include <Slang/slang.h>
#include <Slang/slang-com-ptr.h>
export module Visera.Shader.Slang;
#define VISERA_MODULE_NAME "Shader.Slang"
import Visera.Core.Types.Path;
import Visera.Core.Types.Set;
import Visera.Core.Types.Array;
import Visera.Core.Types.String;
import Visera.Global;
import Visera.Platform;

export namespace Visera::Forge
{
    struct FShaderReflection
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
            FString Access = "Read";
            FString Stage = "All";
        };
        TArray<FEntryPoint> EntryPoints;
        TArray<FResource>   Resources;
    };

    class VISERA_FORGE_API FSlangCompiler
    {
    public:
    	[[nodiscard]] inline Bool
    	AddSearchPath(const FPath& I_Path);
        [[nodiscard]] inline TArray<FByte>
    	Compile(const FPath& I_Path, FStringView I_EntryPoint);
        [[nodiscard]] FShaderReflection
    	ExtractReflection(const FPath& I_Path, FStringView I_EntryPoint);

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
        FSlangCompiler();
    	~FSlangCompiler() { delete Session; slang::shutdown(); }

    private:
    	[[nodiscard]] Bool
    	CreateSession();
    	void inline
    	Process(const FPath&  I_File, FStringView   I_EntryPoint);
    	[[nodiscard]] inline const char*
    	GetErrorMessage(const Slang::ComPtr<slang::IBlob>& I_Diagnostics) const { return static_cast<const char*>(I_Diagnostics->getBufferPointer()); }
    };

	Bool FSlangCompiler::
	AddSearchPath(const FPath& I_Path)
	{
		auto Path = I_Path.GetUTF8Path();
		if (!SearchPaths.Contains(Path))
		{
			SearchPaths.Emplace(std::move(Path));
			LOG_DEBUG("Added a new shader path: {}", I_Path);
			return True;
		}
		LOG_ERROR("Failed to add shader path: {}", I_Path);
		return False;
	}

    TArray<FByte> FSlangCompiler::
    Compile(const FPath& I_Path, FStringView I_EntryPoint)
    {
		Process(I_Path.GetFileName(), I_EntryPoint);
		const FByte* Buffer = static_cast<const FByte*>(Session->CompiledCode->getBufferPointer());

		auto ShaderCode = TArray<FByte>(
			Buffer,
			Buffer + Session->CompiledCode->getBufferSize());

		// Clean up
		Session->CompiledCode.setNull();

        return ShaderCode;
    }

    FSlangCompiler::
    FSlangCompiler()
    {
    	if (Context == nullptr)
    	{
    		LOG_DEBUG("Creating a Slang thread global context.");

    		if (slang::createGlobalSession(Context.writeRef()) != SLANG_OK)
    		{ LOG_FATAL("Failed to create the Slang Context (a.k.a, Global Session)!"); }

    		if (!AddSearchPath(FPlatform::GetResourceDirectory() / FPath{"Assets/App/Shader"}))
    		{ VISERA_ASSERT(False && "Failed to add app shader search path!"); }
    		if (!AddSearchPath(FPlatform::GetResourceDirectory() / FPath{"Assets/Engine/Shader"}))
    		{ VISERA_ASSERT(False && "Failed to add engine shader search path!"); }
    		if (!AddSearchPath(FPlatform::GetResourceDirectory() / FPath{"Assets/Studio/Shader"}))
    		{ VISERA_ASSERT(False && "Failed to add studio shader search path!"); }
    	}

    	if (!CreateSession())
    	{ LOG_FATAL("Failed to create the Slang Session!"); }
    }

	Bool FSlangCompiler::
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

     void FSlangCompiler::
	 Process(const FPath& I_File, FStringView  I_EntryPoint)
	 {
    	VISERA_ASSERT(Context && Session);

    	LOG_TRACE("Compiling the {} (entry_point: {}).", I_File, I_EntryPoint);
	 	Slang::ComPtr<slang::IBlob>  Diagnostics;

	 	// Create Shader Module
	 	Slang::ComPtr<slang::IModule> ShaderModule
    	{
    		Session->Handle->loadModule(I_File.GetUTF8Path().Data(),
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
	 		LOG_ERROR("Failed to find the EntryPoint({}) from Shader({})!",
	 		          I_EntryPoint.Data(), I_File);
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
		// slang::TypeReflection Type;
		// switch (Type.getKind())
		// {
		// case slang::TypeReflection::Kind::Resource: // https://docs.shader-slang.org/en/latest/external/slang/docs/user-guide/09-reflection.html#resources
		// 	{
		// 		auto Shape		= Type.getResourceShape();
		// 		auto Access		= Type.getResourceAccess();
		// 		auto ResultType = Type.getResourceResultType();
		// 		UInt32 BaseShape = Shape & SLANG_RESOURCE_BASE_SHAPE_MASK;
		// 		switch (BaseShape)
		// 		{
		// 		case SLANG_TEXTURE_2D: LOG_INFO("Texture2D"); break;
		// 		case SLANG_TEXTURE_CUBE: LOG_INFO("Cube"); break;
		// 		default : LOG_WARN("Unsupported slang base shape {}!", BaseShape); break;
		// 		}
		// 		break;
		// 	}
		// case slang::TypeReflection::Kind::ConstantBuffer:
		// 	{
		// 		LOG_INFO("ConstantBuffer");
		// 		break;
		// 	}
		// case slang::TypeReflection::Kind::SamplerState:
		// 	{
		// 		LOG_INFO("SamplerState");
		// 		break;
		// 	}
		// default:
		// 	LOG_WARN("Unsupported slang type reflection {}!", static_cast<UInt32>(Type.getKind()));
		// }


	 }

    FShaderReflection FSlangCompiler::
    ExtractReflection(const FPath& I_Path, FStringView I_EntryPoint)
    {
        FShaderReflection Reflection;
        VISERA_ASSERT(Session && Session->ShaderProgram);

        Slang::ComPtr<slang::IBlob> Diagnostics;
        slang::ProgramLayout* ShaderLayout = Session->ShaderProgram->getLayout(0, Diagnostics.writeRef());
        if (Diagnostics || !ShaderLayout)
        {
            LOG_ERROR("Failed to get reflection info from Shader({})!", I_Path);
            return Reflection;
        }

        // Extract EntryPoints
        const auto EntryPointCount = ShaderLayout->getEntryPointCount();
        for (SlangUInt i = 0; i < EntryPointCount; ++i)
        {
            auto* EntryPointRef = ShaderLayout->getEntryPointByIndex(i);
            if (!EntryPointRef) continue;

            FShaderReflection::FEntryPoint EP;
            EP.Name = FString(EntryPointRef->getName());

            // Convert Slang stage to string
            const auto Stage = EntryPointRef->getStage();
            switch (Stage)
            {
            case SLANG_STAGE_VERTEX: EP.Stage = "Vertex"; break;
            case SLANG_STAGE_FRAGMENT: EP.Stage = "Fragment"; break;
            case SLANG_STAGE_COMPUTE: EP.Stage = "Compute"; break;
            case SLANG_STAGE_GEOMETRY: EP.Stage = "Geometry"; break;
            case SLANG_STAGE_HULL: EP.Stage = "TessellationControl"; break;
            case SLANG_STAGE_DOMAIN: EP.Stage = "TessellationEvaluation"; break;
            default: continue; // Skip unsupported stages
            }

            Reflection.EntryPoints.PushBack(std::move(EP));
        }

        // Extract Resources (global parameters)
        const auto ParamCount = ShaderLayout->getParameterCount();
        for (unsigned i = 0; i < ParamCount; ++i)
        {
            auto* VarLayout = ShaderLayout->getParameterByIndex(i);
            if (!VarLayout) continue;

            auto* Var = VarLayout->getVariable();
            if (!Var) continue;

            auto* Type = Var->getType();
            if (!Type) continue;

            const auto TypeKind = Type->getKind();
            if (TypeKind != slang::TypeReflection::Kind::Resource &&
                TypeKind != slang::TypeReflection::Kind::SamplerState &&
                TypeKind != slang::TypeReflection::Kind::ConstantBuffer &&
                TypeKind != slang::TypeReflection::Kind::TextureBuffer &&
                TypeKind != slang::TypeReflection::Kind::ShaderStorageBuffer)
            { continue; }

            FShaderReflection::FResource Res;
            Res.Name = FString(Var->getName());
            Res.Binding = VarLayout->getBindingIndex();
            Res.Set = static_cast<UInt32>(VarLayout->getBindingSpace());

            // Determine resource type
            switch (TypeKind)
            {
            case slang::TypeReflection::Kind::Resource:
                {
                    const auto Shape = Type->getResourceShape();
                    const auto BaseShape = Shape & SLANG_RESOURCE_BASE_SHAPE_MASK;
                    switch (BaseShape)
                    {
                    case SLANG_TEXTURE_2D: Res.Type = "Texture2D"; break;
                    case SLANG_TEXTURE_CUBE: Res.Type = "TextureCube"; break;
                    case SLANG_TEXTURE_3D: Res.Type = "Texture3D"; break;
                    case SLANG_TEXTURE_1D: Res.Type = "Texture1D"; break;
                    default: continue; // Skip unsupported texture types
                    }

                    // Determine access mode
                    const auto Access = Type->getResourceAccess();
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
                continue; // Skip unsupported types
            }

            Reflection.Resources.PushBack(std::move(Res));
        }

        return Reflection;
    }
}
