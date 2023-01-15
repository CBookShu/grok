#pragma once
#include <vector>
#include <unordered_map>

namespace grok::unionlock
{
    template <typename K>
    struct UnionProperty {
        using Key = K;
        using Keys = std::vector<Key>;
    };

    template <typename K>
    struct UnionLockLocal {
        using Key = K;
        using Keys = std::vector<Key>;
    };
} // namespace grok
