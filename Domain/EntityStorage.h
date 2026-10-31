#pragma once
#include <cstdint>
#include <unordered_map>

namespace entity {
using EntityId = uint32_t;

struct Entity {};

class EntityStorage {
public:
    [[nodiscard]] Entity &Get(EntityId entity_id);
    [[nodiscard]] const Entity &Get(EntityId entity_id) const;
    void Remove(EntityId entity_id);
    EntityId CreateEntity();

private:
    std::unordered_map<EntityId, Entity> entities_;
    EntityId last_id_ = 0;
};
} // namespace entity
