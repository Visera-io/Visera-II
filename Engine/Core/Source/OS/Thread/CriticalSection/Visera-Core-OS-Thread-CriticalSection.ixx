module;
#include <Visera-Core.hpp>
#include <mutex>
export module Visera.Core.OS.Thread.CriticalSection;
#define VISERA_MODULE_NAME "Core.OS"

export namespace Visera
{
    class VISERA_CORE_API FCriticalSection
    {
    public:
        Bool TryToLock() { return Mutex.try_lock(); }
        void Lock()		 { Mutex.lock(); }
        void Unlock()	 { Mutex.unlock(); }

    private:
        std::mutex Mutex;
    };

    class VISERA_CORE_API FScopeLock
    {
    public:
        explicit FScopeLock(TMutable<FCriticalSection> I_CriticalSection) : CriticalSection(I_CriticalSection)
        {
            VISERA_ASSERT(CriticalSection != nullptr);
            CriticalSection->Lock();
        }

        ~FScopeLock()
        {
            CriticalSection->Unlock();
        }

        FScopeLock(const FScopeLock&)              = delete;
        FScopeLock& operator=(const FScopeLock&)   = delete;
        FScopeLock(FScopeLock&&)                   = delete;
        FScopeLock& operator=(FScopeLock&&)        = delete;

    private:
        TMutable<FCriticalSection> CriticalSection;
    };
}