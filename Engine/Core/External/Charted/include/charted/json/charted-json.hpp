#pragma once

#include <cstdint>
#include <istream>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "../core/charted-core.hpp"
#include "nlohmann/json.hpp"

namespace charted
{
    class Json
    {
        using NativeJson = nlohmann::json;
        using Self = Json;

    public:
        Json() = default;
        explicit Json(NativeJson value) noexcept : Root(std::move(value)) {}

        [[nodiscard]] static std::optional<Self> Parse(std::string_view jsonText) noexcept
        {
            try { return Self(NativeJson::parse(jsonText)); }
            catch (...) { return std::nullopt; }
        }

        [[nodiscard]] static std::optional<Self> Parse(std::istream& input) noexcept
        {
            try { return Self(NativeJson::parse(input)); }
            catch (...) { return std::nullopt; }
        }

        [[nodiscard]] static std::optional<Self> Load(std::istream& input) noexcept
        {
            return Parse(input);
        }

        [[nodiscard]] bool IsNull() const noexcept { return Root.is_null(); }
        [[nodiscard]] bool IsDiscarded() const noexcept { return Root.is_discarded(); }
        [[nodiscard]] bool Contains(std::string_view key) const noexcept { return Root.contains(key); }
        template <concepts::Route TJSONRoute> [[nodiscard]] bool Contains(const TJSONRoute& routeValue) const noexcept
        { return Find(routeValue) != nullptr; }

        void Clear() noexcept { Root = NativeJson{}; }

        bool Erase(std::string_view key) noexcept
        {
            if (!Root.is_object()) { return false; }
            return Root.erase(std::string(key)) > 0;
        }

        template <concepts::Route TJSONRoute> bool Erase(const TJSONRoute& routeValue) noexcept
        {
            if (!IsRouteValid(routeValue)) { return false; }
            const Waypoint* last = nullptr;
            NativeJson* parent = FindParent(std::addressof(Root), routeValue, last, false);
            if (parent == nullptr || last == nullptr) { return false; }
            if (last->Type == WaypointType::Key)
            {
                if (!parent->is_object()) { return false; }
                return parent->erase(std::string(last->GetString())) > 0;
            }
            if (!parent->is_array() || last->Index >= parent->size()) { return false; }
            parent->erase(parent->begin() + static_cast<NativeJson::difference_type>(last->Index));
            return true;
        }

        [[nodiscard]] std::string Dump(bool pretty = true) const
        { return pretty ? Root.dump(4) : Root.dump(); }

        template <typename T> Self& Set(std::string_view key, T&& value)
        {
            Root[std::string(key)] = ToNative(std::forward<T>(value));
            return *this;
        }

        template <concepts::Route TJSONRoute, typename T> Self& Set(const TJSONRoute& routeValue, T&& value)
        {
            if (!IsRouteValid(routeValue)) { return *this; }
            NativeJson nativeValue = ToNative(std::forward<T>(value));
            const Waypoint* last = nullptr;
            NativeJson* parent = FindParent(std::addressof(Root), routeValue, last, true);
            if (parent == nullptr || last == nullptr) { return *this; }
            if (last->Type == WaypointType::Key)
            {
                if (!parent->is_object()) { *parent = NativeJson::object(); }
                (*parent)[std::string(last->GetString())] = std::move(nativeValue);
                return *this;
            }
            if (!parent->is_array()) { *parent = NativeJson::array(); }
            while (last->Index >= parent->size()) { parent->push_back(NativeJson{}); }
            (*parent)[last->Index] = std::move(nativeValue);
            return *this;
        }

        Self& EnsureObject(std::string_view key) { return Set(key, NativeJson::object()); }
        Self& EnsureArray(std::string_view key) { return Set(key, NativeJson::array()); }
        template <concepts::Route TJSONRoute> Self& EnsureObject(const TJSONRoute& routeValue) { return Set(routeValue, NativeJson::object()); }
        template <concepts::Route TJSONRoute> Self& EnsureArray(const TJSONRoute& routeValue) { return Set(routeValue, NativeJson::array()); }

        template <typename T> [[nodiscard]] std::optional<T> TryGet(std::string_view key) const noexcept
        {
            try
            {
                const auto it = Root.find(key);
                if (it == Root.end()) { return std::nullopt; }
                return FromNative<T>(*it);
            }
            catch (...) { return std::nullopt; }
        }

        template <typename T> [[nodiscard]] T Get(std::string_view key, T defaultValue = T{}) const
        {
            auto value = TryGet<T>(key);
            return value.has_value() ? std::move(value.value()) : std::move(defaultValue);
        }

        template <typename T, concepts::Route TJSONRoute> [[nodiscard]] std::optional<T> TryGet(const TJSONRoute& routeValue) const noexcept
        {
            if (!IsRouteValid(routeValue)) { return std::nullopt; }
            try
            {
                const NativeJson* found = FindPath(std::addressof(Root), routeValue);
                if (found == nullptr) { return std::nullopt; }
                return FromNative<T>(*found);
            }
            catch (...) { return std::nullopt; }
        }

        template <typename T, concepts::Route TJSONRoute> [[nodiscard]] T Get(const TJSONRoute& routeValue, T defaultValue = T{}) const
        {
            auto value = TryGet<T>(routeValue);
            return value.has_value() ? std::move(value.value()) : std::move(defaultValue);
        }

        template <concepts::Route TJSONRoute> [[nodiscard]] NativeJson* Find(const TJSONRoute& routeValue) noexcept
        {
            if (!IsRouteValid(routeValue)) { return nullptr; }
            return FindPath(std::addressof(Root), routeValue);
        }
        template <concepts::Route TJSONRoute> [[nodiscard]] const NativeJson* Find(const TJSONRoute& routeValue) const noexcept
        {
            if (!IsRouteValid(routeValue)) { return nullptr; }
            return FindPath(std::addressof(Root), routeValue);
        }

        [[nodiscard]] NativeJson& GetNative() noexcept { return Root; }
        [[nodiscard]] const NativeJson& GetNative() const noexcept { return Root; }

    private:
        template <typename T> [[nodiscard]] static NativeJson ToNative(T&& value)
        {
            if constexpr (std::same_as<std::remove_cvref_t<T>, Self>) { return value.GetNative(); }
            else if constexpr (std::same_as<std::remove_cvref_t<T>, std::string_view>) { return NativeJson(std::string(value)); }
            else { return NativeJson(std::forward<T>(value)); }
        }

        template <typename T> [[nodiscard]] static std::optional<T> FromNative(const NativeJson& value)
        {
            if constexpr (std::same_as<T, Self>) { return T(value); }
            else { return value.template get<T>(); }
        }

        template <concepts::Route TJSONRoute> static bool IsRouteValid(const TJSONRoute& routeValue)
        {
            if constexpr (concepts::IsStaticRoute<std::remove_cvref_t<TJSONRoute>>::value) { return std::remove_cvref_t<TJSONRoute>::Valid; }
            else if constexpr (requires { { routeValue.IsValid() } -> std::convertible_to<bool>; }) { return static_cast<bool>(routeValue.IsValid()); }
            else { return true; }
        }

        template <typename TNativeJson, concepts::Route TJSONRoute>
        [[nodiscard]] static TNativeJson* FindPath(TNativeJson* root, const TJSONRoute& routeValue) noexcept
        {
            TNativeJson* current = root;
            Cursor cursor(routeValue);
            while (cursor.HasNext())
            {
                if (std::optional<std::string_view> key = cursor.NextKey(); key.has_value())
                {
                    if (!current->is_object()) { return nullptr; }
                    const auto it = current->find(std::string(key.value()));
                    if (it == current->end()) { return nullptr; }
                    current = std::addressof(*it);
                }
                else if (std::optional<std::uint32_t> index = cursor.NextIndex(); index.has_value())
                {
                    if (!current->is_array()) { return nullptr; }
                    if (index.value() >= current->size()) { return nullptr; }
                    current = std::addressof((*current)[index.value()]);
                }
                else { return nullptr; }
            }
            return current;
        }

        template <concepts::Route TJSONRoute>
        [[nodiscard]] static NativeJson* FindParent(
            NativeJson* root,
            const TJSONRoute& routeValue,
            const Waypoint*& outLast,
            bool createMissing) noexcept
        {
            outLast = nullptr;
            if (root == nullptr) { return nullptr; }
            Cursor cursor(routeValue);
            const Waypoint* token = cursor.Next();
            if (token == nullptr) { return nullptr; }

            NativeJson* current = root;
            while (cursor.HasNext())
            {
                const Waypoint* next = cursor.Peek();
                if (next == nullptr) { return nullptr; }

                if (token->Type == WaypointType::Key)
                {
                    if (!current->is_object())
                    {
                        if (!createMissing) { return nullptr; }
                        *current = NativeJson::object();
                    }
                    const std::string key(token->GetString());
                    auto it = current->find(key);
                    if (it == current->end())
                    {
                        if (!createMissing) { return nullptr; }
                        (*current)[key] = (next->Type == WaypointType::Key) ? NativeJson::object() : NativeJson::array();
                        it = current->find(key);
                    }

                    NativeJson& child = *it;
                    if (next->Type == WaypointType::Key)
                    {
                        if (!child.is_object())
                        {
                            if (!createMissing) { return nullptr; }
                            child = NativeJson::object();
                        }
                    }
                    else if (!child.is_array())
                    {
                        if (!createMissing) { return nullptr; }
                        child = NativeJson::array();
                    }
                    current = std::addressof(child);
                }
                else
                {
                    if (!current->is_array())
                    {
                        if (!createMissing) { return nullptr; }
                        *current = NativeJson::array();
                    }
                    while (token->Index >= current->size())
                    {
                        if (!createMissing) { return nullptr; }
                        current->push_back(NativeJson{});
                    }

                    NativeJson& child = (*current)[token->Index];
                    if (next->Type == WaypointType::Key)
                    {
                        if (!child.is_object())
                        {
                            if (!createMissing) { return nullptr; }
                            child = NativeJson::object();
                        }
                    }
                    else if (!child.is_array())
                    {
                        if (!createMissing) { return nullptr; }
                        child = NativeJson::array();
                    }
                    current = std::addressof(child);
                }

                token = cursor.Next();
                if (token == nullptr) { return nullptr; }
            }

            outLast = token;
            return current;
        }

    private:
        NativeJson Root;
    };
}
