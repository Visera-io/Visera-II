module;
#include <Visera-Runtime.hpp>
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>
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
        AKRESULT
        Close(AkFileDesc* I_FileDesc) override { VISERA_UNIMPLEMENTED_API; return AK_Success; };
        AkUInt32
        GetBlockSize(AkFileDesc& I_FileDesc) override { VISERA_UNIMPLEMENTED_API; return AK_Success; };
        void
        GetDeviceDesc(AkDeviceDesc& O_DeviceDesc) override { VISERA_UNIMPLEMENTED_API; };
        AkUInt32
        GetDeviceData() override {VISERA_UNIMPLEMENTED_API; return 0;};
        void
        BatchOpen(AkUInt32			    I_NumFiles,
                  AkAsyncFileOpenData** I_Items) override {VISERA_UNIMPLEMENTED_API;};
        void
        BatchRead(AkUInt32				I_NumTransfers,
                  BatchIoTransferItem*	I_TransferItems) override {VISERA_UNIMPLEMENTED_API;};
        void
        BatchWrite(AkUInt32				I_NumTransfers,
                   BatchIoTransferItem*	I_TransferItems) override {VISERA_UNIMPLEMENTED_API;};
    public:
        explicit FWwiseIO()
        {

        }

        ~FWwiseIO() override
        {

        }
    };
}
