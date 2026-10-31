#pragma once
#include "EntityStorage.h"
#include <unordered_map>

namespace component {
template <typename T> class ComponentStorage {
public:
    void Add(entity::EntityId entity, T component);

    void Remove(entity::EntityId entity);
    void Clear();

    [[nodiscard]] T *Get(entity::EntityId entity);
    [[nodiscard]] const T *Get(entity::EntityId entity) const;

    [[nodiscard]] bool Has(entity::EntityId entity) const;
    [[nodiscard]] bool Empty() const;

    [[nodiscard]] auto begin();
    [[nodiscard]] auto end();
    [[nodiscard]] auto begin() const;
    [[nodiscard]] auto end() const;

private:
    std::unordered_map<entity::EntityId, T> data_;
};

template <typename T> void ComponentStorage<T>::Add(entity::EntityId entity, T component) {
    data_[entity] = std::move(component);
}

template <typename T> void ComponentStorage<T>::Remove(entity::EntityId entity) {
    data_.erase(entity);
}

template <typename T> void ComponentStorage<T>::Clear() { data_.clear(); }

template <typename T> T *ComponentStorage<T>::Get(entity::EntityId entity) {
    auto it = data_.find(entity);
    if (it != data_.end()) {
        return &it->second;
    }
    return nullptr;
}

template <typename T> const T *ComponentStorage<T>::Get(entity::EntityId entity) const {
    auto it = data_.find(entity);
    if (it != data_.end()) {
        return &it->second;
    }
    return nullptr;
}

template <typename T> bool ComponentStorage<T>::Has(entity::EntityId entity) const {
    return data_.find(entity) != data_.end();
}

template <typename T> bool ComponentStorage<T>::Empty() const { return data_.empty(); }

template <typename T> auto ComponentStorage<T>::begin() { return data_.begin(); }

template <typename T> auto ComponentStorage<T>::end() { return data_.end(); }

template <typename T> auto ComponentStorage<T>::begin() const { return data_.begin(); }

template <typename T> auto ComponentStorage<T>::end() const { return data_.end(); }

} // namespace component
