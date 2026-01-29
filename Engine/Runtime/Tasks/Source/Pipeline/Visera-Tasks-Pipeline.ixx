module;
#include <Visera-Tasks.hpp>
#include <taskflow/taskflow.hpp>
export module Visera.Tasks.Pipeline;
#define VISERA_MODULE_NAME "Tasks.Pipeline"
import Visera.Tasks.Interface;

export namespace Visera
{
    // Forward declaration
    class FTaskNode;

    /**
     * Wrapper around taskflow::Taskflow for creating and managing task pipelines.
     * Provides a Visera-style interface for building task dependency graphs.
     */
    class VISERA_TASKS_API FTaskPipeline
    {
    public:
        template<Concepts::Task TaskType> [[nodiscard]] FTaskNode
        Emplace(TaskType&& I_Task);
        [[nodiscard]] UInt64
        GetTaskCount() const;
        [[nodiscard]] Bool
        IsEmpty() const;
        void
        Clear();
        [[nodiscard]] FString
        GetName() const;
        [[nodiscard]] FString
        Dump() const;
        [[nodiscard]] tf::Taskflow&
        GetTaskflow();

        [[nodiscard]] const tf::Taskflow&
        GetTaskflow() const;

    private:
        tf::Taskflow Pipeline;

    public:
        explicit FTaskPipeline(const FString& I_Name = "");
    };

    /**
     * Task node handle for managing task dependencies.
     */
    class VISERA_TASKS_API FTaskNode
    {
    public:
        [[nodiscard]] Bool
        IsValid() const;
        void
        Precede(const FTaskNode& I_Other);
        void
        Succeed(const FTaskNode& I_Other);

    private:
        explicit FTaskNode(tf::Task I_Task);

        [[nodiscard]] tf::Task&
        GetTask();

        [[nodiscard]] const tf::Task&
        GetTask() const;

        template<Concepts::Task TaskType>
        friend FTaskNode FTaskPipeline::Emplace(TaskType&&);
        friend class FTaskNode;

        tf::Task Task;

    public:
        FTaskNode() = default;
        FTaskNode(const FTaskNode& I_Other);
        FTaskNode(FTaskNode&& I_Other) noexcept;
        FTaskNode& operator=(const FTaskNode& I_Other);
        FTaskNode& operator=(FTaskNode&& I_Other) noexcept;
        ~FTaskNode();
    };

    // Implementation

    FTaskPipeline::FTaskPipeline(const FString& I_Name)
        : Pipeline(I_Name.empty() ? "ViseraTaskPipeline" : I_Name)
    {
    }

    template<Concepts::Task TaskType>
    FTaskNode FTaskPipeline::
    Emplace(TaskType&& I_Task)
    {
        return FTaskNode(Pipeline.emplace(std::forward<TaskType>(I_Task)));
    }

    UInt64 FTaskPipeline::
    GetTaskCount() const
    {
        return static_cast<UInt64>(Pipeline.num_tasks());
    }

    Bool FTaskPipeline::
    IsEmpty() const
    {
        return Pipeline.empty();
    }

    void FTaskPipeline::
    Clear()
    {
        Pipeline.clear();
    }

    FString FTaskPipeline::
    GetName() const
    {
        return Pipeline.name();
    }

    FString FTaskPipeline::
    Dump() const
    {
        return Pipeline.dump();
    }

    tf::Taskflow& FTaskPipeline::
    GetTaskflow()
    {
        return Pipeline;
    }

    const tf::Taskflow& FTaskPipeline::
    GetTaskflow() const
    {
        return Pipeline;
    }

    FTaskNode::FTaskNode(tf::Task I_Task)
        : Task(std::move(I_Task))
    {
    }

    FTaskNode::FTaskNode(const FTaskNode& I_Other)
        : Task(I_Other.Task)
    {
    }

    FTaskNode::FTaskNode(FTaskNode&& I_Other) noexcept
        : Task(std::move(I_Other.Task))
    {
    }

    FTaskNode& FTaskNode::
    operator=(const FTaskNode& I_Other)
    {
        if (this != &I_Other)
        {
            Task = I_Other.Task;
        }
        return *this;
    }

    FTaskNode& FTaskNode::
    operator=(FTaskNode&& I_Other) noexcept
    {
        if (this != &I_Other)
        {
            Task = std::move(I_Other.Task);
        }
        return *this;
    }

    FTaskNode::~FTaskNode() = default;

    Bool FTaskNode::
    IsValid() const
    {
        return !Task.empty();
    }

    void FTaskNode::
    Precede(const FTaskNode& I_Other)
    {
        Task.precede(I_Other.Task);
    }

    void FTaskNode::
    Succeed(const FTaskNode& I_Other)
    {
        Task.succeed(I_Other.Task);
    }

    tf::Task& FTaskNode::
    GetTask()
    {
        return Task;
    }

    const tf::Task& FTaskNode::
    GetTask() const
    {
        return Task;
    }
}
