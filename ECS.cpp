//
//  ECS.cpp
//  opengl-tools
//
//  Created by Frank Collebrusco on 1/4/23.
//
//

#include "ECS.h"

entID ECS::newEntity(){
    if (!freelist.empty()){
        uint32_t index = (uint32_t)freelist.back();
        freelist.pop_back();
        return genEntityIDat(index);
    }
    entities.push_back({genEntityIDat((uint32_t)entities.size()), ComponentMask()});
    return entities.back().id;
}

void ECS::removeEntity(entID i) {
    uint32_t index = getEntityIndex(i);
    assert(index < entities.size());
    entities.at(index).mask.reset();
    clearEntityID(entities.at(index).id);
    freelist.push_back(index);
}

entID ECS::genEntityIDat(uint32_t index) { 
    return (entID{0} | index) << 32;
}

entID ECS::replaceEntityIDat(uint32_t index) {
    assert(index < entities.size());
    entID res = entities.at(index).id;
    return (res & ~0xFFFFFFFF) | ((res & 0xFFFFFFFF) + 1);
}

void ECS::clearEntityID(entID & i) {
    assert(getEntityIndex(i) < entities.size());
    entID res = entities.at(i).id;
    res = ((res & ~0xFFFFFFFF) | ((res & 0xFFFFFFFF) + 1)) | (0xFFFFFFFF00000000);
}

uint32_t ECS::getEntityIndex(const entID & i) {
    return i >> 32;
}

bool ECS::entityValid(entID i) {
    return i == (entities.at(getEntityIndex(i)).id);
}




