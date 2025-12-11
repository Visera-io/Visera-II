module;
#include <Visera-Engine.hpp>
#include <Slang/slang.h>
#include <Slang/slang-com-ptr.h>
export module Visera.Engine.AssetHub.Shader.Slang;
#define VISERA_MODULE_NAME "Engine.AssetHub"
import Visera.Core.Types.Path;
import Visera.Core.Types.Set;
import Visera.Core.Types.Array;
import Visera.Core.Log;
import Visera.Runtime.Platform;
import Visera.Engine.Event;

namespace Visera
{
    export class VISERA_ENGINE_API FSlangCompiler
    {
    public:
    	[[nodiscard]] static inline Bool
    	AddSearchPath(const FPath& I_Path);
        [[nodiscard]] inline TArray<FByte>
    	Compile(const FPath& I_Path, FStringView I_EntryPoint);

    private:
    	static inline Slang::ComPtr<slang::IGlobalSession>
    	Context {nullptr}; //[Note] Currently, the global session type is not thread-safe. Applications that wish to compile on multiple threads will need to ensure that each concurrent thread compiles with a distinct global session.
    	static inline TSet<FString>
    	SearchPaths{};

    	struct FSession
        {
            Slang::ComPtr<slang::ISession> Handle;
            slang::TargetDesc              Description;
    		Slang::ComPtr<slang::IBlob>    CompiledCode;
        };
        TUniquePtr<FSession>               Session;

    public:
        FSlangCompiler();
    	~FSlangCompiler() = default;

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
		if (!SearchPaths.contains(Path))
		{
			SearchPaths.emplace(std::move(Path));
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

    		if (!AddSearchPath(GPlatform->GetResourceDirectory() / FPath{"Assets/App/Shader"}))
    		{ VISERA_ASSERT(False && "Failed to add app shader search path!"); }
    		if (!AddSearchPath(GPlatform->GetResourceDirectory() / FPath{"Assets/Engine/Shader"}))
    		{ VISERA_ASSERT(False && "Failed to add engine shader search path!"); }
    		if (!AddSearchPath(GPlatform->GetResourceDirectory() / FPath{"Assets/Studio/Shader"}))
    		{ VISERA_ASSERT(False && "Failed to add studio shader search path!"); }
    	}

    	if (!CreateSession())
    	{ LOG_FATAL("Failed to create the Slang Session!"); }

		GEvent->OnEngineTerminate.Subscribe([this](){ Session.reset(); slang::shutdown(); });
    }

	Bool FSlangCompiler::
	CreateSession()
    {
    	LOG_TRACE("Creating a new slang session.");
    	Session = MakeUnique<FSession>();

    	// Create Vulkan Compiler Session
    	Session->Description =
    	{
    		.format  = SLANG_SPIRV,
			.profile = Context->findProfile("glsl_450"),
		};

    	TArray<const char*> SlangSearchPaths;
		SlangSearchPaths.reserve(SearchPaths.size());
		for (const auto& Path : SearchPaths)
    	{ SlangSearchPaths.emplace_back(Path.data()); }

    	slang::SessionDesc SessionCreateInfo
		{
			.targets		 = &Session->Description,
			.targetCount	 = 1,
			.searchPaths	 = SlangSearchPaths.data(),
			.searchPathCount = static_cast<UInt32>(SearchPaths.size()),
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
    		Session->Handle->loadModule(I_File.GetUTF8Path().data(),
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
	 		I_EntryPoint.data(),
	 		ShaderEntryPoint.writeRef()) != SLANG_OK)
	 	{
	 		LOG_ERROR("Failed to find the EntryPoint({}) from Shader({})!",
	 		          I_EntryPoint.data(), I_File);
			return;
	 	}

	 	slang::IComponentType* const ShaderComponents[2]
    	{
	 		ShaderModule.get(),
	 		ShaderEntryPoint.get(),
	 	};

	 	Slang::ComPtr<slang::IComponentType> ShaderProgram {nullptr};
	 	if (Session->Handle->createCompositeComponentType(
	 		ShaderComponents, 2,
	 		ShaderProgram.writeRef(),
	 		Diagnostics.writeRef()) != SLANG_OK)
	 	{
	 		LOG_ERROR("Failed to create the Shader({}): {}!",
	 			      I_File, GetErrorMessage(Diagnostics));
	 		return;
	 	}

	 	if (ShaderProgram->getEntryPointCode(
	 		0,
	 		0,
	 		Session->CompiledCode.writeRef(),
	 		Diagnostics.writeRef()) != SLANG_OK)
	 	{
	 		LOG_ERROR("Failed to obtain compiled code from {}: {}!",
	 		          I_File, GetErrorMessage(Diagnostics));
	 		return;
	 	}

	 	// Reflect Shader
	 	slang::ProgramLayout* ShaderLayout = ShaderProgram->getLayout(0, Diagnostics.writeRef());
	 	if (Diagnostics)
		{
	 		LOG_ERROR("Failed to get reflection info from Shader({}): {}!",
	 			      I_File, GetErrorMessage(Diagnostics));
	 		return;
		}

	 	auto* EntryPointRef = ShaderLayout->findEntryPointByName(I_EntryPoint.data());
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


	 	// switch (EntryPointRef->getStage())
	 	// {
	 	// 	case SLANG_STAGE_VERTEX:	ShaderType = FRHIShader::EStage::Vertex;   break;
	 	// 	case SLANG_STAGE_FRAGMENT:	ShaderType = FRHIShader::EStage::Fragment; break;
	 	// 	default: LOG_ERROR("Unsupported Shader Stage!");
	 	// }
	 }
}
