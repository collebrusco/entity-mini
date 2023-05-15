//
//  ECS.cpp
//  opengl-tools
//
//  Created by Frank Collebrusco on 1/4/23.
//
//

#include "ECS.h"

ECS::ObjectPool::ObjectPool(size_t s){
    obj_size = s;
    data = new char[obj_size * MAX_ENTITIES];
}
ECS::ObjectPool::~ObjectPool(){
    delete [] data;
}
void* ECS::ObjectPool::get(size_t i){
    assert(i < MAX_COMPONENTS);
    return data + (i * obj_size);
}

// check freelist for empty spot, or append id/bitmask pair
entID ECS::newEntity(){
    if (!freelist.empty()){
        uint32_t index = (uint32_t)freelist.back();
        freelist.pop_back();
        entities[index].id = replaceEntityIDat(index);
        return entities[index].id;
    }
    entities.push_back({genEntityIDat((uint32_t)entities.size()), ComponentMask()});
    return entities.back().id;
}

// invalidate ID and add index to free list
void ECS::removeEntity(entID& i) {
    uint32_t index = getEntityIndex(i);
    assert(index < entities.size());
    assert(i == entities.at(index).id);
    entities.at(index).mask.reset();
    clearEntityID(entities.at(index).id);
    i = entities.at(index).id;
    freelist.push_back(index);
}

// generate version 0 entity ID at an index
entID ECS::genEntityIDat(uint32_t index) {
    return (entID{0} | index) << 32;
}

// increment version number at index
entID ECS::replaceEntityIDat(uint32_t index) {
    assert(index < entities.size());
    entID res = entities.at(index).id;
    return ((res & ~0xFFFFFFFF00000000) | ((uint64_t)index << 32)) | ((res & 0xFFFFFFFF) + 1);
}

// invalidate id while maintaining version
void ECS::clearEntityID(entID& i) {
    auto index = getEntityIndex(i);
    assert(index < entities.size());
    assert(i == entities.at(index).id);
    entID res = entities.at(index).id;
    i = ((res & 0xFFFFFFFF) + 1) | (0xFFFFFFFF00000000);
    entities[index].id = i;
}

// isolate index half of id
uint32_t ECS::getEntityIndex(const entID & i) const {
    return i >> 32;
}

// check if an id matches the one at its index
bool ECS::entityValid(entID i) const {
    return i == (entities.at(getEntityIndex(i)).id);
}

// number of active enemies
size_t ECS::numEntities() { 
    return entities.size() - freelist.size();
}

// number of defined components
size_t ECS::numComponents() { 
    return p_componentCounter;
}





