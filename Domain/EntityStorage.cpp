#include "EntityStorage.h"

namespace entity {
Entity &EntityStorage::Get(EntityId entity_id) { return entities_.at(entity_id); }

const Entity &EntityStorage::Get(EntityId entity_id) const { return entities_.at(entity_id); }

void EntityStorage::Remove(EntityId entity_id) { entities_.erase(entity_id); }

EntityId EntityStorage::CreateEntity() {
    const Entity entity;
    last_id_ += 1;
    entities_.emplace(last_id_, entity);
    return last_id_;
}
} // namespace entity
