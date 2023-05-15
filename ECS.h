//
//  ECS.h
//  opengl-tools
//
//  Created by Frank Collebrusco on 1/4/23.
//
//

 /*
    An ECS manages a list of entities, each with a set of components
    components are stored contigiously so looping over a given type of component is effecient
    
    useful when many disjointed systems need to act on a similar large set of organized data
    usually game engines
 */


#ifndef ECS_h
#define ECS_h
#include <bitset>
#include <vector>

const uint32_t MAX_COMPONENTS = 64;
const uint32_t MAX_ENTITIES = 0x100;

typedef uint64_t entID;
typedef std::bitset<MAX_COMPONENTS> ComponentMask;

class ECS {
private:
public:
    // entity id parsing
    entID genEntityIDat(uint32_t index);
    entID replaceEntityIDat(uint32_t index);
    void clearEntityID(entID&);
    uint32_t getEntityIndex(entID const&);
    // component ID
    unsigned int p_componentCounter{0};
    template <class T>
    int getCompID(){
        static int cid = p_componentCounter++;
        return cid;
    }
    // entity/mask list
    struct EntityData {
        entID id;
        ComponentMask mask;
    };
    std::vector<EntityData> entities;
    std::vector<size_t> freelist;

    // pools of components
    struct ObjectPool {
        size_t obj_size;
        char* data{0};
        ObjectPool(size_t s){
            obj_size = s;
            data = new char[obj_size * MAX_ENTITIES];
        }
        ~ObjectPool(){
            delete [] data;
        }
        inline void* get(size_t i){
            assert(i < MAX_COMPONENTS);
            return data + i * obj_size;
        }
    };
    
    std::vector<ObjectPool*> pools;
    
public:
    entID newEntity();
    void removeEntity(entID&);
    bool entityValid(entID);
    
    size_t numEntities();
    size_t numComponents();
    
    template <class T>
    T* addComp(entID i){
        assert(entityValid(i));
        int compID = getCompID<T>();
        if (compID >= pools.size()){
            pools.resize(compID + 1, nullptr);
        }
        if (pools[compID] == nullptr){
            pools[compID] = new ObjectPool(sizeof(T));
        }
        T* component = new (pools[compID]->get(getEntityIndex(i))) T();
        entities.at(getEntityIndex(i)).mask.set(getCompID<T>());
        return component;
    }
    
    template <class T>
    T* getComp(entID i){
        assert(entityValid(i));
        int compID = getCompID<T>();
        if (!entities.at(i).mask.test(compID)){
            return nullptr;
        }
        return (T*) pools.at(compID)->get(i);
    }
    
    template <class T>
    void removeComp(entID i){
        assert(entityValid(i));
        entities.at(i).mask.reset(getCompID<T>());
        //TODO: dealloc?
    }
};

#endif /* ECS_h */
