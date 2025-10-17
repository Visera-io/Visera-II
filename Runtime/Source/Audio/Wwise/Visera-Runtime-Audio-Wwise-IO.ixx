module;
#include <Visera-Runtime.hpp>
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>
#if defined(VISERA_ON_WINDOWS_SYSTEM)
#include <AKSamples/SoundEngine/Win32/AkDefaultIOHookDeferred.h>
#else
#include <AKSamples/SoundEngine/POSIX/AkDefaultIOHookDeferred.h>
#endif
export module Visera.Runtime.Audio.Wwise.IO;
#define VISERA_MODULE_NAME "Runtime.Audio"
import Visera.Core.Log;

namespace Visera
{
    export VISERA_RUNTIME_API class FWwiseIO
        : public AK::StreamMgr::IAkLowLevelIOHook,
          public AK::StreamMgr::IAkFileLocationResolver
    {
    public:
        void inline
        Initialize(const AkDeviceSettings& I_DeviceSettings)
        {
            DefaultIO.Init(I_DeviceSettings);
        }
    private:
        CAkDefaultIOHookDeferred DefaultIO; // Forward to Wwise's default blocking IO hook
    public:
        AKRESULT
        Close(AkFileDesc* I_FileDesc) override { return DefaultIO.Close(I_FileDesc); }
        AkUInt32
        GetBlockSize(AkFileDesc& I_FileDesc) override { return DefaultIO.GetBlockSize(I_FileDesc); }
        void
        GetDeviceDesc(AkDeviceDesc& O_DeviceDesc) override { return DefaultIO.GetDeviceDesc(O_DeviceDesc); }
        AkUInt32
        GetDeviceData() override { return DefaultIO.GetDeviceData(); }
        void
        BatchOpen(AkUInt32 I_NumFiles, AkAsyncFileOpenData** I_Items) override { return DefaultIO.BatchOpen(I_NumFiles, I_Items); }
        void
        BatchRead(AkUInt32 I_NumTransfers, BatchIoTransferItem* I_TransferItems) override { return DefaultIO.BatchRead(I_NumTransfers, I_TransferItems); }
        void
        BatchWrite(AkUInt32 I_NumTransfers, BatchIoTransferItem* I_TransferItems) override { return DefaultIO.BatchWrite(I_NumTransfers, I_TransferItems); }
    public:
        FWwiseIO() = default;

        ~FWwiseIO() override = default;
    };
}
