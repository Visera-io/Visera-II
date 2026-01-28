module;
#include <Visera-Tasks.hpp>
#include <taskflow/taskflow.hpp>
export module Visera.Tasks;
#define VISERA_MODULE_NAME "Tasks"
import Visera.Global;

export namespace Visera
{
    class VISERA_TASKS_API FTasks : public IGlobalService
    {
    public:

    private:

    public:
        FTasks() : IGlobalService(EName::Tasks)
        {
            Dependencies =
            {

            };

            if (!OnBootstrap.TryBind([this]
            {
                return True;
            }))
            { LOG_FATAL("Failed to bind bootstrap function!"); }

            if (!OnTerminate.TryBind([this]
            {
                return True;
            }))
            { LOG_FATAL("Failed to bind terminate function!"); }
        }
    };
}