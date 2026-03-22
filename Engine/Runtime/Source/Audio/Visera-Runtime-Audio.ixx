module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Audio;
#define VISERA_MODULE_NAME "Runtime.Audio"
import Visera.Runtime.Audio.Interface;
import Visera.Runtime.Audio.Null;
import Visera.Runtime.Audio.Wwise;
import Visera.Core.Containers.Array;
import Visera.Core.Containers.Map;
import Visera.Core.Math.Hash.GoldenRatio;
import Visera.Core.OS.Memory;
import Visera.Core.OS.Thread.Sync.Atomic;
import Visera.Core.OS.Thread.Queue.MPSC;
import Visera.Core.Types.Name;
import Visera.Core.Types.String;
import Visera.Core.Types.Path;
import Visera.Core.Types.Pointer.Unique;
import Visera.Runtime.AssetHub;
import Visera.Core.Log;

export namespace Visera
{
    using EAudioEngine = IAudioEngine::EType;

    /** CreateInfo for FAudio. Presence in FEngineCreateInfo enables the service. */
    struct VISERA_RUNTIME_API FAudioCreateInfo
    {
        FString Engine = "Null";
        /** Resolved via asset hub; Wwise init.bnk and script loads use this root. */
        VPath   BankBasePath{FStringView{"@assets://soundbanks"}};
        UInt32  PumpCriticalMinPerTick = 64;
        UInt32  PumpNormalMaxPerTick   = 256;
        UInt32  PumpSpamMaxPerTick     = 128;
        UInt32  PumpSpamDropPerTick    = 128;
        UInt32  PumpMaxCommandsPerTick = 512;
        UInt64  QueueMaxPendingSpam    = 8192;
        UInt32  WwiseCommandQueueSizeBytes = 1024 * 1024;
    };

    class VISERA_RUNTIME_API FAudio
    {
    public:
        explicit FAudio(const FAudioCreateInfo& I_CreateInfo);
        ~FAudio();

    public:
        using FObjectID  = IAudioEngine::FObjectID;
        using FPlayingID = IAudioEngine::FPlayingID;
        enum class ECategory : UInt8
        {
            UI,
            Impact,
            Ambient,
            Voice,
        };
        enum class EPriority : UInt8
        {
            Critical,
            Normal,
            Spam,
        };

    private:
        struct FAudioCommand
        {
            enum class EType : UInt8
            {
                RegisterGameObject,
                UnregisterGameObject,
                PostEvent,
                SetRTPC,
                SetPosition,
            };
            EType      Type     {EType::PostEvent};
            ECategory  Category {ECategory::Impact};
            EPriority  Priority {EPriority::Normal};
            FObjectID  Token    {IAudioEngine::InvalidObjectID};
            IAudioEngine::FEventID ID {IAudioEngine::InvalidEventID}; // event id or rtpc id
            Float      Value {0.0f};              // rtpc value / impact intensity
            Float      X     {0.0f};
            Float      Y     {0.0f};
            Float      Z     {0.0f};
            UInt32     SpatialCell {0};
            FName      Name {};
        };
        PROFILING_ONLY_FIELD(
        struct FProfilingMetrics
        {
            UInt64 EnqueuedCritical         {0};
            UInt64 EnqueuedNormal           {0};
            UInt64 EnqueuedSpam             {0};
            UInt64 DroppedSpamDueToLimit    {0};
            UInt64 DroppedSpamByOverflow    {0};

            UInt64 PeakPendingCritical      {0};
            UInt64 PeakPendingNormal        {0};
            UInt64 PeakPendingSpam          {0};

            UInt64 PeakPumpBatchSize        {0};
            UInt64 PeakPumpBatchCapacity    {0};
            UInt64 PeakCoalescedRTPC        {0};
            UInt64 PeakCoalescedPosition    {0};
            UInt64 PeakAggregatedImpact     {0};
            UInt64 InlineOverflowEvents     {0};
            UInt64 PeakOverInlineBytes      {0};
            UInt64 RecommendedInlineBytes   {kAudioPumpInlineArenaBytes};
        } ProfilingMetrics {};
        );
    public:
        [[nodiscard]] inline FObjectID
        RegisterEmitter(FName I_Name, EPriority I_Priority = EPriority::Critical)
        {
            const auto Token = NextToken.FetchAdd(1, EMemoryOrder::Relaxed);
            FAudioCommand Command{};
            Command.Type     = FAudioCommand::EType::RegisterGameObject;
            Command.Priority = I_Priority;
            Command.Token    = Token;
            Command.Name     = I_Name;
            return EnqueueCommand(std::move(Command)) ? Token : IAudioEngine::InvalidObjectID;
        }
        inline void
        UnregisterEmitter(FObjectID I_Token, EPriority I_Priority = EPriority::Normal)
        {
            FAudioCommand Command{};
            Command.Type     = FAudioCommand::EType::UnregisterGameObject;
            Command.Priority = I_Priority;
            Command.Token    = I_Token;
            if (!EnqueueCommand(std::move(Command)))
            { LOG_WARN("Failed to enqueue UnregisterGameObject command!"); }
        }
        [[nodiscard]] inline Bool
        PostEvent(FName I_Event, FObjectID I_Token, ECategory I_Category, EPriority I_Priority = EPriority::Normal, Float I_Intensity = 1.0f, UInt32 I_SpatialCell = 0)
        {
            FAudioCommand Command{};
            Command.Type        = FAudioCommand::EType::PostEvent;
            Command.Category    = I_Category;
            Command.Priority    = I_Priority;
            Command.Token       = I_Token;
            Command.ID          = IAudioEngine::InvalidEventID;
            Command.Name        = I_Event;
            Command.Value       = I_Intensity;
            Command.SpatialCell = I_SpatialCell;
            return EnqueueCommand(std::move(Command));
        }
        [[nodiscard]] inline Bool
        PostEvent(IAudioEngine::FEventID I_EventID, FObjectID I_Token, ECategory I_Category, EPriority I_Priority = EPriority::Normal, Float I_Intensity = 1.0f, UInt32 I_SpatialCell = 0)
        {
            FAudioCommand Command{};
            Command.Type        = FAudioCommand::EType::PostEvent;
            Command.Category    = I_Category;
            Command.Priority    = I_Priority;
            Command.Token       = I_Token;
            Command.ID          = I_EventID;
            Command.Value       = I_Intensity;
            Command.SpatialCell = I_SpatialCell;
            return EnqueueCommand(std::move(Command));
        }
        [[nodiscard]] inline Bool
        SetRTPC(FName I_RTPC, FObjectID I_Token, Float I_Value, EPriority I_Priority = EPriority::Normal)
        {
            FAudioCommand Command{};
            Command.Type     = FAudioCommand::EType::SetRTPC;
            Command.Priority = I_Priority;
            Command.Token    = I_Token;
            Command.ID       = IAudioEngine::InvalidRTPCID;
            Command.Name     = I_RTPC;
            Command.Value    = I_Value;
            return EnqueueCommand(std::move(Command));
        }
        [[nodiscard]] inline Bool
        SetRTPC(IAudioEngine::FRTPCID I_RTPCID, FObjectID I_Token, Float I_Value, EPriority I_Priority = EPriority::Normal)
        {
            FAudioCommand Command{};
            Command.Type     = FAudioCommand::EType::SetRTPC;
            Command.Priority = I_Priority;
            Command.Token    = I_Token;
            Command.ID       = I_RTPCID;
            Command.Value    = I_Value;
            return EnqueueCommand(std::move(Command));
        }
        [[nodiscard]] inline Bool
        SetPosition(FObjectID I_Token, Float I_X, Float I_Y, Float I_Z, EPriority I_Priority = EPriority::Normal)
        {
            FAudioCommand Command{};
            Command.Type     = FAudioCommand::EType::SetPosition;
            Command.Priority = I_Priority;
            Command.Token    = I_Token;
            Command.X        = I_X;
            Command.Y        = I_Y;
            Command.Z        = I_Z;
            return EnqueueCommand(std::move(Command));
        }

        void inline
        Tick()
        {
            if (!Engine) { return; }
            Pump();
            Engine->RenderAudio();
        }

        /** Load an additional .bnk under BankBasePath (relative name, e.g. fire_meow.bnk). init.bnk is loaded automatically in the Wwise constructor path. */
        [[nodiscard]] inline Bool
        LoadSoundBank(FName I_RelativeBankFile)
        {
            if (!Engine || I_RelativeBankFile.IsNone()) { return False; }
            return Engine->LoadSoundBankFile(FName::FetchNameString(I_RelativeBankFile));
        }

    private:
        [[nodiscard]] inline Bool
        EnqueueCommand(FAudioCommand&& I_Command)
        {
            switch (I_Command.Priority)
            {
                case EPriority::Critical:
                    CriticalQueue.Enqueue(std::move(I_Command));
                    {
                        const UInt64 NowPending = PendingCritical.FetchAdd(1, EMemoryOrder::Relaxed) + 1;
                        PROFILING_ONLY_FIELD(
                        ++ProfilingMetrics.EnqueuedCritical;
                        if (NowPending > ProfilingMetrics.PeakPendingCritical) { ProfilingMetrics.PeakPendingCritical = NowPending; }
                        );
                    }
                    return True;
                case EPriority::Normal:
                    NormalQueue.Enqueue(std::move(I_Command));
                    {
                        const UInt64 NowPending = PendingNormal.FetchAdd(1, EMemoryOrder::Relaxed) + 1;
                        PROFILING_ONLY_FIELD(
                        ++ProfilingMetrics.EnqueuedNormal;
                        if (NowPending > ProfilingMetrics.PeakPendingNormal) { ProfilingMetrics.PeakPendingNormal = NowPending; }
                        );
                    }
                    return True;
                case EPriority::Spam:
                    if (PendingSpam.Load(EMemoryOrder::Relaxed) >= MaxPendingSpam)
                    {
                        PROFILING_ONLY_FIELD(++ProfilingMetrics.DroppedSpamDueToLimit;);
                        return False;
                    }
                    SpamQueue.Enqueue(std::move(I_Command));
                    {
                        const UInt64 NowPending = PendingSpam.FetchAdd(1, EMemoryOrder::Relaxed) + 1;
                        PROFILING_ONLY_FIELD(
                        ++ProfilingMetrics.EnqueuedSpam;
                        if (NowPending > ProfilingMetrics.PeakPendingSpam) { ProfilingMetrics.PeakPendingSpam = NowPending; }
                        );
                    }
                    return True;
                default:
                    return False;
            }
        }
        inline void
        DrainQueue(TMPSCQueue<FAudioCommand>& I_Queue, TAtomic<UInt64>& I_Pending, UInt32 I_Budget, TPMRArray<FAudioCommand>& O_Batch)
        {
            for (UInt32 Index = 0; Index < I_Budget; ++Index)
            {
                auto OptionalCommand = I_Queue.Dequeue();
                if (!OptionalCommand.HasValue()) { break; }
                I_Pending.FetchSub(1, EMemoryOrder::Relaxed);
                O_Batch.PushBack(std::move(OptionalCommand.GetValue()));
            }
        }
        inline void
        DropSpamOverflow(UInt32 I_MaxDrop)
        {
            UInt64 DroppedCount = 0;
            for (UInt32 Index = 0; Index < I_MaxDrop; ++Index)
            {
                auto OptionalCommand = SpamQueue.Dequeue();
                if (!OptionalCommand.HasValue()) { break; }
                PendingSpam.FetchSub(1, EMemoryOrder::Relaxed);
                ++DroppedCount;
            }
            PROFILING_ONLY_FIELD(ProfilingMetrics.DroppedSpamByOverflow += DroppedCount;);
        }
        inline void
        SubmitCommand(const FAudioCommand& I_Command)
        {
            switch (I_Command.Type)
            {
                case FAudioCommand::EType::RegisterGameObject:
                {
                    if (!Engine->RegisterGameObject(I_Command.Token, FName::FetchNameString(I_Command.Name)))
                    { LOG_ERROR("Failed to register game object id:{} name:{}.", I_Command.Token, I_Command.Name.GetNameString()); }
                    else
                    { Playlist.InsertOrAssign(I_Command.Token, I_Command.Name); }
                    break;
                }
                case FAudioCommand::EType::UnregisterGameObject:
                {
                    if (!Engine->UnregisterGameObject(I_Command.Token))
                    { LOG_ERROR("Failed to unregister game object id:{}.", I_Command.Token); }
                    else
                    { Playlist.Erase(I_Command.Token); }
                    break;
                }
                case FAudioCommand::EType::PostEvent:
                {
                    auto EventID = I_Command.ID;
                    if (EventID == IAudioEngine::InvalidEventID && !I_Command.Name.IsNone())
                    { EventID = Engine->GetEventID(FName::FetchNameString(I_Command.Name)); }
                    const auto PlayingID = Engine->PostEvent(EventID, I_Command.Token);
                    if (PlayingID == IAudioEngine::InvalidPlayingID)
                    { LOG_ERROR("Failed to post event id:{} on token:{}.", EventID, I_Command.Token); }
                    break;
                }
                case FAudioCommand::EType::SetRTPC:
                {
                    auto RTPCID = I_Command.ID;
                    if (RTPCID == IAudioEngine::InvalidRTPCID && !I_Command.Name.IsNone())
                    { RTPCID = Engine->GetRTPCID(FName::FetchNameString(I_Command.Name)); }
                    if (!Engine->SetRTPC(RTPCID, I_Command.Token, I_Command.Value))
                    { LOG_ERROR("Failed to set RTPC id:{} on token:{}.", RTPCID, I_Command.Token); }
                    break;
                }
                case FAudioCommand::EType::SetPosition:
                {
                    if (!Engine->SetPosition(I_Command.Token, I_Command.X, I_Command.Y, I_Command.Z))
                    { LOG_ERROR("Failed to set position on token:{}.", I_Command.Token); }
                    break;
                }
                default: break;
            }
        }
        inline void
        Pump()
        {
            PumpBatch.Clear();
            DrainQueue(CriticalQueue, PendingCritical, CriticalQuotaPerTick, PumpBatch);
            if (PumpBatch.GetSize() < MaxCommandsPerTick)
            { DrainQueue(NormalQueue, PendingNormal, NormalQuotaPerTick, PumpBatch); }
            if (PumpBatch.GetSize() < MaxCommandsPerTick)
            { DrainQueue(SpamQueue, PendingSpam, SpamQuotaPerTick, PumpBatch); }
            if (PumpBatch.GetSize() >= MaxCommandsPerTick && SpamQueue.Peek() != nullptr)
            { DropSpamOverflow(SpamDropPerTick); }
            PROFILING_ONLY_FIELD(
            if (PumpBatch.GetSize() > ProfilingMetrics.PeakPumpBatchSize) { ProfilingMetrics.PeakPumpBatchSize = PumpBatch.GetSize(); }
            if (PumpBatch.GetCapacity() > ProfilingMetrics.PeakPumpBatchCapacity)
            {
                ProfilingMetrics.PeakPumpBatchCapacity = PumpBatch.GetCapacity();
                const UInt64 PeakCapacityBytes = ProfilingMetrics.PeakPumpBatchCapacity * sizeof(FAudioCommand);
                if (PeakCapacityBytes > kAudioPumpInlineArenaBytes)
                {
                    const UInt64 OverInlineBytes = PeakCapacityBytes - kAudioPumpInlineArenaBytes;
                    ++ProfilingMetrics.InlineOverflowEvents;
                    if (OverInlineBytes > ProfilingMetrics.PeakOverInlineBytes) { ProfilingMetrics.PeakOverInlineBytes = OverInlineBytes; }
                    ProfilingMetrics.RecommendedInlineBytes = PeakCapacityBytes;
                    LOG_WARN("[Profiling] Audio pump inline arena pressure: inline={} bytes, peak_capacity={} bytes, over_by={} bytes, recommended_inline={} bytes.",
                        kAudioPumpInlineArenaBytes,
                        PeakCapacityBytes,
                        OverInlineBytes,
                        ProfilingMetrics.RecommendedInlineBytes);
                }
            }
            );

            CoalescedRTPCByKey.Clear();
            CoalescedPositionByEmitter.Clear();
            AggregatedImpactEvents.Clear();

            for (const auto& Command : PumpBatch)
            {
                if (Command.Type == FAudioCommand::EType::SetRTPC)
                {
                    auto RTPCID = Command.ID;
                    if (RTPCID == IAudioEngine::InvalidRTPCID && !Command.Name.IsNone())
                    { RTPCID = Engine->GetRTPCID(FName::FetchNameString(Command.Name)); }
                    const auto Key = Math::GoldenRatioHashCombine(0, static_cast<UInt64>(Command.Token), static_cast<UInt64>(RTPCID));
                    FAudioCommand Resolved{Command};
                    Resolved.ID = RTPCID;
                    CoalescedRTPCByKey.InsertOrAssign(Key, Resolved);
                    continue;
                }
                if (Command.Type == FAudioCommand::EType::SetPosition)
                {
                    CoalescedPositionByEmitter.InsertOrAssign(static_cast<UInt64>(Command.Token), Command);
                    continue;
                }
                if (Command.Type == FAudioCommand::EType::PostEvent &&
                    Command.Category == ECategory::Impact &&
                    Command.Priority != EPriority::Critical)
                {
                    auto EventID = Command.ID;
                    if (EventID == IAudioEngine::InvalidEventID && !Command.Name.IsNone())
                    { EventID = Engine->GetEventID(FName::FetchNameString(Command.Name)); }
                    const auto Key = Math::GoldenRatioHashCombine(0, static_cast<UInt64>(EventID), static_cast<UInt64>(Command.SpatialCell));
                    FAudioCommand Resolved{Command};
                    Resolved.ID = EventID;
                    const auto Iter = AggregatedImpactEvents.Find(Key);
                    if (Iter == AggregatedImpactEvents.end() || Iter->second.Value < Command.Value)
                    {
                        AggregatedImpactEvents.InsertOrAssign(Key, Resolved);
                    }
                    continue;
                }
                SubmitCommand(Command);
            }
            PROFILING_ONLY_FIELD(
            if (CoalescedRTPCByKey.GetSize() > ProfilingMetrics.PeakCoalescedRTPC) { ProfilingMetrics.PeakCoalescedRTPC = CoalescedRTPCByKey.GetSize(); }
            if (CoalescedPositionByEmitter.GetSize() > ProfilingMetrics.PeakCoalescedPosition) { ProfilingMetrics.PeakCoalescedPosition = CoalescedPositionByEmitter.GetSize(); }
            if (AggregatedImpactEvents.GetSize() > ProfilingMetrics.PeakAggregatedImpact) { ProfilingMetrics.PeakAggregatedImpact = AggregatedImpactEvents.GetSize(); }
            );

            for (const auto& Pair : CoalescedRTPCByKey)
            { SubmitCommand(Pair.second); }
            for (const auto& Pair : CoalescedPositionByEmitter)
            { SubmitCommand(Pair.second); }
            for (const auto& Pair : AggregatedImpactEvents)
            { SubmitCommand(Pair.second); }
        }

    private:
        TUniquePtr<IAudioEngine>  Engine {};
        TMPSCQueue<FAudioCommand> CriticalQueue {};
        TMPSCQueue<FAudioCommand> NormalQueue {};
        TMPSCQueue<FAudioCommand> SpamQueue {};
        TMap<FObjectID, FName>    Playlist {};
        Memory::TMonotonicArena<kAudioPumpInlineArenaBytes> PumpArena {};
        TPMRArray<FAudioCommand>    PumpBatch;
        TMap<UInt64, FAudioCommand> CoalescedRTPCByKey {};
        TMap<UInt64, FAudioCommand> CoalescedPositionByEmitter {};
        TMap<UInt64, FAudioCommand> AggregatedImpactEvents {};
        TAtomic<UInt64>       NextToken {1};
        TAtomic<UInt64>       PendingCritical {0};
        TAtomic<UInt64>       PendingNormal {0};
        TAtomic<UInt64>       PendingSpam {0};

        UInt32 CriticalQuotaPerTick {64};
        UInt32 NormalQuotaPerTick {256};
        UInt32 SpamQuotaPerTick {128};
        UInt32 SpamDropPerTick {128};
        UInt32 MaxCommandsPerTick {512};
        UInt64 MaxPendingSpam {8192};
        UInt32 WwiseCommandQueueSizeBytes {1024 * 1024};
    };

    FAudio::FAudio(const FAudioCreateInfo& I_CreateInfo)
        : PumpArena()
        , PumpBatch(&PumpArena.Get())
    {
        CriticalQuotaPerTick   = I_CreateInfo.PumpCriticalMinPerTick;
        NormalQuotaPerTick     = I_CreateInfo.PumpNormalMaxPerTick;
        SpamQuotaPerTick       = I_CreateInfo.PumpSpamMaxPerTick;
        SpamDropPerTick        = I_CreateInfo.PumpSpamDropPerTick;
        MaxCommandsPerTick     = I_CreateInfo.PumpMaxCommandsPerTick;
        MaxPendingSpam         = I_CreateInfo.QueueMaxPendingSpam;
        WwiseCommandQueueSizeBytes = I_CreateInfo.WwiseCommandQueueSizeBytes;
        PumpBatch.Reserve(MaxCommandsPerTick);
        CoalescedRTPCByKey.Reserve(MaxCommandsPerTick);
        CoalescedPositionByEmitter.Reserve(MaxCommandsPerTick);
        AggregatedImpactEvents.Reserve(MaxCommandsPerTick);

        const Bool bWwise = (I_CreateInfo.Engine == "Wwise" || I_CreateInfo.Engine == "wwise");
        if (bWwise)
        {
            Engine = MakeUnique<FWwiseAudioEngine>(WwiseCommandQueueSizeBytes);
            const FObjectID MainID{0};
            if (!Engine->RegisterGameObject(MainID, "Player"))
            { LOG_FATAL("Failed to register Main Listener"); }
            if (!Engine->SetDefaultListeners(MainID))
            { LOG_FATAL("Failed to set default listeners"); }
            const FPath ResolvedBankPath = I_CreateInfo.BankBasePath.GetRealPath();
            if (ResolvedBankPath.IsEmpty() || !Engine->MountSoundBankBase(ResolvedBankPath))
            { LOG_WARN("Failed to mount sound bank base path. Banks may not load."); }
            else if (!Engine->LoadSoundBankFile(FStringView{"init.bnk"}))
            { LOG_WARN("Failed to load init.bnk. Wwise events may fail."); }
        }
        else
        { Engine = MakeUnique<FNullAudioEngine>(); }

        DEBUG_ONLY_FIELD(
        switch (Engine->GetType())
        {
            case IAudioEngine::EType::Null : LOG_DEBUG("Audio Engine: Null.");  break;
            case IAudioEngine::EType::Wwise: LOG_DEBUG("Audio Engine: Wwise."); break;
            default: LOG_FATAL("Unknown Audio Engine!");  break;
        }
        );
    }

    FAudio::~FAudio()
    {
        if (!Engine) { return; }
        PROFILING_ONLY_FIELD(
        const Bool InlineArenaMayOverflow = ProfilingMetrics.RecommendedInlineBytes > kAudioPumpInlineArenaBytes;
        LOG_INFO("[Profiling] Enqueued counts: critical={}, normal={}, spam={}.",
            ProfilingMetrics.EnqueuedCritical,
            ProfilingMetrics.EnqueuedNormal,
            ProfilingMetrics.EnqueuedSpam);
        LOG_INFO("[Profiling] Queue peaks: critical={}, normal={}, spam={}, spam_limit={}.",
            ProfilingMetrics.PeakPendingCritical,
            ProfilingMetrics.PeakPendingNormal,
            ProfilingMetrics.PeakPendingSpam,
            MaxPendingSpam);
        LOG_INFO("[Profiling] Spam drops: limit_rejects={}, overflow_drops={}, enqueued_spam={}.",
            ProfilingMetrics.DroppedSpamDueToLimit,
            ProfilingMetrics.DroppedSpamByOverflow,
            ProfilingMetrics.EnqueuedSpam);
        LOG_INFO("[Profiling] Pump batch: peak_size={}, peak_capacity={}, inline_arena={} bytes, estimated_needed_inline={} bytes.",
            ProfilingMetrics.PeakPumpBatchSize,
            ProfilingMetrics.PeakPumpBatchCapacity,
            kAudioPumpInlineArenaBytes,
            ProfilingMetrics.PeakPumpBatchSize * sizeof(FAudioCommand));
        LOG_INFO("[Profiling] Pump inline overflow: events={}, peak_over_by={} bytes.",
            ProfilingMetrics.InlineOverflowEvents,
            ProfilingMetrics.PeakOverInlineBytes);
        if (InlineArenaMayOverflow)
        {
            LOG_WARN("[Profiling] Pump inline arena may be too small. Consider >= {} bytes.",
                ProfilingMetrics.RecommendedInlineBytes);
        }
        LOG_INFO("[Profiling] Coalescing peaks: rtpc={}, position={}, impact={}.",
            ProfilingMetrics.PeakCoalescedRTPC,
            ProfilingMetrics.PeakCoalescedPosition,
            ProfilingMetrics.PeakAggregatedImpact);
        );
        if (!Engine->UnregisterAllGameObjects())
        { LOG_WARN("Failed to unregister all game objects during shutdown."); }
        Playlist.Clear();
        Engine.Reset();
    }
}
