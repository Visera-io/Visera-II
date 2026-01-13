module;
#include <Visera-Game.hpp>
#include <entt/entt.hpp>
export module Visera.Game.World.Entity;
#define VISERA_MODULE_NAME "Game.World"

namespace Visera
{
    export class VISERA_ENGINE_API FEntity
    {
    public:
        [[nodiscard]] Bool
        IsValid() const noexcept { return Handle && Handle.registry() && Handle.entity() != entt::null && Handle.registry()->valid(Handle.entity()); }
        [[nodiscard]] entt::entity
        GetID() const noexcept { return Handle.entity(); }
        template<class T> [[nodiscard]] Bool
        Has() const {return Handle.all_of<T>(); }
        template<class T, class... Args> T&
        Add(Args&&... args) {return Handle.emplace<T>(std::forward<Args>(args)...); }
        template<class T> [[nodiscard]] T&
        Get() {return Handle.get<T>(); }
        template<class T> [[nodiscard]] const T&
        Get() const {return Handle.get<T>(); }
        template<class T> [[nodiscard]] T*
        TryGet() {return Handle.try_get<T>(); }
        template<class T> [[nodiscard]] const T*
        TryGet() const {return Handle.try_get<T>(); }
        template<class T> void
        Remove() {Handle.remove<T>(); }

    private:
        entt::handle Handle{};

    public:
        FEntity() = default;
        FEntity(entt::registry& I_Registry, entt::entity I_ID) noexcept
        : Handle{ I_Registry, I_ID }
        {

        }
    };
}
