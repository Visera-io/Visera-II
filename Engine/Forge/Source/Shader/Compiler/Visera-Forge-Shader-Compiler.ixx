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
        [[nodiscard]] FShaderReflection
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
    	Process(const FPath&  I_File, FStringView   I_EntryPoint);
    	[[nodiscard]] inline const char*
    	GetErrorMessage(const Slang::ComPtr<slang::IBlob>& I_Diagnostics) const { return static_cast<const char*>(I_Diagnostics->getBufferPointer()); }
    };

	Bool FShaderCompiler::
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

    TArray<FByte> FShaderCompiler::
    Compile(const FPath& I_Path, FStringView I_EntryPoint, const FPath& I_SearchDirectory)
    {
		const Bool HadPath = SearchPaths.Contains(I_SearchDirectory.GetUTF8Path());
		if (!HadPath) { (void)AddSearchPath(I_SearchDirectory); }

		if (!Session || !HadPath)
		{
			delete Session;
			Session = nullptr;
			if (!CreateSession())
			{ LOG_FATAL("Failed to create the Slang Session!"); }
		}
		Process(I_Path.GetFileName(), I_EntryPoint);
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

    FShaderReflection FShaderCompiler::
    ExtractReflection(const FPath& I_Path, FStringView I_EntryPoint, const FPath& I_SearchDirectory)
    {
        FShaderReflection Reflection;
        (void)I_SearchDirectory; // Session already configured by Compile() before ExtractReflection is called.
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
            case SLANG_STAGE_GEOMETRY:
            case SLANG_STAGE_HULL:
            case SLANG_STAGE_DOMAIN:
            default: continue; // Skip unsupported stages (Geometry/Tessellation removed for now)
            }

            Reflection.EntryPoints.PushBack(std::move(EP));
        }

        auto AddStage = [](TArray<FString>& IO_Stages, FStringView I_Stage)
        {
            if (I_Stage.IsEmpty()) return;
            for (const auto& S : IO_Stages) { if (S == I_Stage) return; }
            IO_Stages.PushBack(FString(I_Stage));
        };

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

        auto FindOrAddResource = [&Reflection](const FShaderReflection::FResource& I_Res) -> FShaderReflection::FResource&
        {
            for (auto& R : Reflection.Resources)
            {
                if (R.Set == I_Res.Set && R.Binding == I_Res.Binding) return R;
            }
            Reflection.Resources.PushBack(I_Res);
            return Reflection.Resources.Back();
        };

        auto FindOrAddPushConstant = [&Reflection](FStringView I_Name, UInt32 I_Size) -> FShaderReflection::FPushConstant&
        {
            for (auto& PC : Reflection.PushConstants)
            {
                if (PC.Name == I_Name) return PC;
            }
            FShaderReflection::FPushConstant PC;
            PC.Name = FString(I_Name);
            PC.Size = I_Size;
            Reflection.PushConstants.PushBack(std::move(PC));
            return Reflection.PushConstants.Back();
        };

        auto ExtractResourceLike = [&](slang::VariableLayoutReflection* VarLayout, FStringView I_StageForUsage)
        {
            if (!VarLayout) return;
            auto* Var = VarLayout->getVariable();
            if (!Var) return;
            auto* Type = Var->getType();
            if (!Type) return;

            // Push-constant blocks: reported as a distinct parameter category.
            if (VarLayout->getCategory() == slang::ParameterCategory::PushConstantBuffer)
            {
                const char* Name = Var->getName();
                const FStringView PCName = (Name && Name[0] != '\0') ? FStringView(Name) : FStringView("PushConstants");
                const UInt32 Size = static_cast<UInt32>(VarLayout->getTypeLayout()->getSize(slang::ParameterCategory::PushConstantBuffer));
                auto& PC = FindOrAddPushConstant(PCName, Size);
                AddStage(PC.Stages, I_StageForUsage);
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

            FShaderReflection::FResource Res;
            Res.Name = FString(Var->getName());
            Res.Binding = VarLayout->getBindingIndex();
            Res.Set = static_cast<UInt32>(VarLayout->getBindingSpace());
            Res.ArrayCount = ArrayCount == 0 ? 1u : ArrayCount;

            // Determine resource type
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
                    default: return; // Skip unsupported texture types
                    }

                    // Determine access mode
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

            auto& R = FindOrAddResource(Res);
            // Keep first-seen Name/Type/Access/ArrayCount, but always add stages.
            AddStage(R.Stages, I_StageForUsage);
        };

        // Extract Resources + PushConstants (global parameters) — used for basic shape info (no stage usage here).
        const auto ParamCount = ShaderLayout->getParameterCount();
        for (unsigned i = 0; i < ParamCount; ++i)
        {
            ExtractResourceLike(ShaderLayout->getParameterByIndex(i), "");
        }

        // Refine stage usage: for each entry point, record which resources/PCs are used by that stage.
        for (SlangUInt epi = 0; epi < EntryPointCount; ++epi)
        {
            auto* EntryPointRef = ShaderLayout->getEntryPointByIndex(epi);
            if (!EntryPointRef) continue;
            FStringView StageName = "All";
            switch (EntryPointRef->getStage())
            {
            case SLANG_STAGE_VERTEX: StageName = "Vertex"; break;
            case SLANG_STAGE_FRAGMENT: StageName = "Fragment"; break;
            case SLANG_STAGE_COMPUTE: StageName = "Compute"; break;
            case SLANG_STAGE_GEOMETRY:
            case SLANG_STAGE_HULL:
            case SLANG_STAGE_DOMAIN:
            default: continue; // Skip unsupported stages (Geometry/Tessellation removed for now)
            }

            // Prefer binding-range walk for accurate stage usage + descriptor array count.
            if (auto* TL = EntryPointRef->getTypeLayout())
            {
                const SlangInt RC = TL->getBindingRangeCount();
                for (SlangInt ri = 0; ri < RC; ++ri)
                {
                    const auto BT = TL->getBindingRangeType(ri);
                    // Push constants are handled via ExtractResourceLike (category), skip here.
                    if (BT == slang::BindingType::PushConstant) continue;

                    const UInt32 Set = static_cast<UInt32>(TL->getBindingRangeDescriptorSetIndex(ri));
                    const UInt32 Binding = static_cast<UInt32>(TL->getDescriptorSetDescriptorRangeIndexOffset(Set, ri));
                    const UInt32 Count = static_cast<UInt32>(TL->getBindingRangeBindingCount(ri));

                    for (auto& R : Reflection.Resources)
                    {
                        if (R.Set == Set && R.Binding == Binding)
                        {
                            AddStage(R.Stages, StageName);
                            if (Count > R.ArrayCount) R.ArrayCount = Count;
                            break;
                        }
                    }
                }
            }

            const unsigned EPC = EntryPointRef->getParameterCount();
            for (unsigned pi = 0; pi < EPC; ++pi)
            {
                ExtractResourceLike(EntryPointRef->getParameterByIndex(pi), StageName);
            }
        }

        // Ensure every resource has at least one stage (fallback to All).
        for (auto& R : Reflection.Resources)
        {
            if (R.Stages.IsEmpty()) { R.Stages.PushBack("All"); }
        }
        for (auto& PC : Reflection.PushConstants)
        {
            if (PC.Stages.IsEmpty()) { PC.Stages.PushBack("All"); }
        }

        return Reflection;
    }
}
