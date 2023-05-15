//
//  ECS.cpp
//  opengl-tools
//
//  Created by Frank Collebrusco on 1/4/23.
//
//

#include "ECS.h"

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

void ECS::removeEntity(entID& i) {
    uint32_t index = getEntityIndex(i);
    assert(index < entities.size());
    assert(i == entities.at(index).id);
    entities.at(index).mask.reset();
    clearEntityID(entities.at(index).id);
    i = entities.at(index).id;
    freelist.push_back(index);
}

entID ECS::genEntityIDat(uint32_t index) {
    return (entID{0} | index) << 32;
}

entID ECS::replaceEntityIDat(uint32_t index) {
    assert(index < entities.size());
    entID res = entities.at(index).id;
    return ((res & ~0xFFFFFFFF00000000) | ((uint64_t)index << 32)) | ((res & 0xFFFFFFFF) + 1);
}

void ECS::clearEntityID(entID& i) {
    auto index = getEntityIndex(i);
    assert(index < entities.size());
    assert(i == entities.at(index).id);
    entID res = entities.at(index).id;
    i = ((res & 0xFFFFFFFF) + 1) | (0xFFFFFFFF00000000);
    entities[index].id = i;
}

uint32_t ECS::getEntityIndex(const entID & i) {
    return i >> 32;
}

bool ECS::entityValid(entID i) {
    return i == (entities.at(getEntityIndex(i)).id);
}

size_t ECS::numEntities() { 
    return entities.size() - freelist.size();
}

size_t ECS::numComponents() { 
    return p_componentCounter;
}





