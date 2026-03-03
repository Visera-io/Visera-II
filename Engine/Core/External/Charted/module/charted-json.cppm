module;
#include <charted/json/charted-json.hpp>

export module charted.json;
export import charted.core;

export namespace charted
{
    using ::charted::Json;
    using ::charted::ObjectItem;
    using ::charted::ObjectItemsIterator;
    using ::charted::ObjectItemsRange;
}
