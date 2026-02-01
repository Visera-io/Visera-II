module;
#include <Visera-Core.hpp>
#include <future>
#include <chrono>
export module Visera.Core.OS.Thread.Future;
#define VISERA_MODULE_NAME "Core.OS"

export namespace Visera
{
	enum class EFutureStatus : Int32
	{
		Ready    = static_cast<Int32>(std::future_status::ready),
		Timeout  = static_cast<Int32>(std::future_status::timeout),
		Deferred = static_cast<Int32>(std::future_status::deferred),
	};

	template<typename T>
	class VISERA_CORE_API FFuture
	{
	public:
		[[nodiscard]] Bool
		Valid() const { return Future.valid(); }
		[[nodiscard]] Bool
		IsReady() const { return Future.valid() ? Future.wait_for(std::chrono::seconds(0)) == std::future_status::ready : False; }
		void
		Wait() const { Future.wait(); }
		[[nodiscard]] EFutureStatus
		WaitFor(Float I_Seconds) const { if (!(I_Seconds >= 0)) { return EFutureStatus::Timeout; } return static_cast<EFutureStatus>(Future.wait_for(std::chrono::duration<Float, std::ratio<1>>(I_Seconds))); }
		T
		Get() const { return Future.get(); }

	private:
		std::shared_future<T> Future;

	public:
		FFuture() = default;
		explicit FFuture(std::future<T> I_Future) : Future(I_Future.share()) {}
		explicit FFuture(std::shared_future<T> I_Future) : Future(std::move(I_Future)) {}
		FFuture(FFuture&&)					= default;
		FFuture& operator=(FFuture&&)		= default;
		FFuture(const FFuture&)				= default;
		FFuture& operator=(const FFuture&)	= default;
	};

	template<>
	class VISERA_CORE_API FFuture<void>
	{
	public:
		[[nodiscard]] Bool
		Valid() const { return Future.valid(); }
		[[nodiscard]] Bool
		IsReady() const { return Future.valid() ? Future.wait_for(std::chrono::seconds(0)) == std::future_status::ready : False; }
		void
		Wait() const { Future.wait(); }
		[[nodiscard]] EFutureStatus
		WaitFor(Float I_Seconds) const { if (!(I_Seconds >= 0)) { return EFutureStatus::Timeout; } return static_cast<EFutureStatus>(Future.wait_for(std::chrono::duration<Float, std::ratio<1>>(I_Seconds))); }
		void
		Get() const { Future.get(); }

	private:
		std::shared_future<void> Future;

	public:
		FFuture() = default;
		explicit FFuture(std::future<void> I_Future) : Future(I_Future.share()) {}
		explicit FFuture(std::shared_future<void> I_Future) : Future(std::move(I_Future)) {}
		FFuture(FFuture&&)					= default;
		FFuture& operator=(FFuture&&)		= default;
		FFuture(const FFuture&)				= default;
		FFuture& operator=(const FFuture&)	= default;
	};
}
