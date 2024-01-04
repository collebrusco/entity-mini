//
//  ECS.cpp
//  opengl-tools
//
//  Created by Frank Collebrusco on 1/4/23.
//
//

#include "ECS.h"

ECS::ObjectPool::ObjectPool(){
    obj_size = -1;
    data = 0;
}
ECS::ObjectPool::ObjectPool(size_t s){
    obj_size = s;
    data = new char[obj_size * MAX_ENTITIES];
}
ECS::ObjectPool::~ObjectPool(){
    if (data) delete [] data;
}
void* ECS::ObjectPool::get(size_t i){
    assert(i < MAX_ENTITIES);
    return data + (i * obj_size);
}

// ECS::ObjectPool::ObjectPool(ECS::ObjectPool& other) {
//     this->obj_size = other.obj_size;
//     this->data = other.data;
//     other.obj_size = -1;
//     other.data = 0;
// }
ECS::ObjectPool::ObjectPool(ECS::ObjectPool&& other) {
    this->obj_size = other.obj_size;
    this->data = other.data;
    other.obj_size = -1;
    other.data = 0;
}
ECS::ObjectPool & ECS::ObjectPool::operator=(ECS::ObjectPool&& other) {
    this->obj_size = other.obj_size;
    this->data = other.data;
    other.obj_size = -1;
    other.data = 0;
    return *this;
}

// check freelist for empty spot, or append id/bitmask pair
entID ECS::newEntity(){
    if (!freelist.empty()){
        uint32_t index = (uint32_t)freelist.back();
        freelist.pop_back();
        entities[index].id = replace_entity_id_at(index);
        return entities[index].id;
    }
    entities.push_back({gen_entity_id_at((uint32_t)entities.size()), ComponentMask()});
    return entities.back().id;
}

// invalidate ID and add index to free list
void ECS::removeEntity(entID& i) {
    uint32_t index = get_entity_index(i);
    assert(index < entities.size());
    assert(i == entities.at(index).id);
    entities.at(index).mask.reset();
    clear_entity_id(entities.at(index).id);
    i = entities.at(index).id;
    freelist.push_back(index);
}

// generate version 0 entity ID at an index
entID ECS::gen_entity_id_at(uint32_t index) {
    return (entID{0} | index) << 32;
}

// increment version number at index
entID ECS::replace_entity_id_at(uint32_t index) {
    assert(index < entities.size());
    entID res = entities.at(index).id;
    return ((res & ~0xFFFFFFFF00000000) | ((uint64_t)index << 32)) | ((res & 0xFFFFFFFF) + 1);
}

// invalidate id while maintaining version
void ECS::clear_entity_id(entID& i) {
    auto index = get_entity_index(i);
    assert(index < entities.size());
    assert(i == entities.at(index).id);
    entID res = entities.at(index).id;
    i = ((res & 0xFFFFFFFF) + 1) | (0xFFFFFFFF00000000);
    entities[index].id = i;
}

// isolate index half of id
uint32_t ECS::get_entity_index(const entID & i) const {
    return i >> 32;
}

// check if an id matches the one at its index
bool ECS::entityValid(entID i) const {
    if (get_entity_index(i) == 0xFFFFFFFF){return false;}
    return i == (entities.at(get_entity_index(i)).id);
}

// number of active enemies
size_t ECS::numEntities() { 
    return entities.size() - freelist.size();
}

// number of defined components
size_t ECS::numComponents() { 
    return p_componentCounter;
}





