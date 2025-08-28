module;
#include <Visera-Engine.hpp>
export module Visera.Engine.Object;
#define VISERA_MODULE_NAME "Engine.Object"
import Visera.Core.Log;
import Visera.Core.Types.Name;

export namespace Visera
{
    /**
     * @brief Base class for all engine objects
     * 
     * Uses FName for high-performance object identification.
     * FName provides O(1) comparisons and minimal memory footprint
     * through string interning, crucial for engine performance.
     */
    class VISERA_ENGINE_API VObject
    {
    public:
        /**
         * @brief Get debug name for this object type
         * @return Human-readable name for debugging
         */
        [[nodiscard]] virtual FStringView
        GetDebugName() const { return "VObject"; }

        /**
         * @brief Get the object's FName identifier
         * @return High-performance FName for object identification
         */
        [[nodiscard]] const FName& GetObjectName() const { return ObjectName; }

        /**
         * @brief Set the object's FName identifier
         * @param NewName The new name for this object
         */
        void SetObjectName(const FName& NewName) { ObjectName = NewName; }

        /**
         * @brief Get unique instance identifier for event system
         * @return Unique 64-bit identifier (Handle + Instance)
         */
        [[nodiscard]] UInt64 GetInstanceID() const 
        { 
            return ObjectName.GetIdentifier();
        }

        /**
         * @brief Constructor with optional object name
         * @param InObjectName Name for this object (defaults to generic "Object")
         */
        explicit VObject(const FName& InObjectName = FName(EName::Object))
            : ObjectName(InObjectName)
        {
        }
        
        virtual ~VObject() = default;

        // Prevent copying to avoid object identity issues
        VObject(const VObject&) = delete;
        VObject& operator=(const VObject&) = delete;
        
        // Allow moving
        VObject(VObject&&) = default;
        VObject& operator=(VObject&&) = default;

        /**
         * @brief Fast equality comparison using FName performance
         */
        bool operator==(const VObject& Other) const
        {
            return ObjectName == Other.ObjectName;
        }

    private:
        FName ObjectName;  ///< High-performance object identifier
    };
}