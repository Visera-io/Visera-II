module;
#include <Visera-RHI.hpp>
export module Visera.Runtime.RHI.Registry.Item;
#define VISERA_MODULE_NAME "Runtime.RHI"

export namespace Visera
{
    class VISERA_RUNTIME_API IRHIRegistryItem
    {

    };

    namespace Concepts
    {
        template<typename T> concept
        RHIRegistryItem =
            std::derived_from<std::remove_cvref_t<T>, IRHIRegistryItem> &&

            requires { typename std::remove_cvref_t<T>::FCreateInfo; } &&

            requires(const typename std::remove_cvref_t<T>::FCreateInfo& A,
                     const typename std::remove_cvref_t<T>::FCreateInfo& B)
            {
                { A == B } -> std::convertible_to<Bool>;
            } &&

            requires(const typename std::remove_cvref_t<T>::FCreateInfo& A,
                     const typename std::remove_cvref_t<T>::FCreateInfo& B)
            {
                { A != B } -> std::convertible_to<Bool>;
            } &&

            requires(const typename std::remove_cvref_t<T>::FCreateInfo& A,
                     const typename std::remove_cvref_t<T>::FCreateInfo& B)
            {
                { A.IsCompatibleWith(B) } -> std::convertible_to<Bool>;
            };
    }
}