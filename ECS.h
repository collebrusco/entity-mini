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
const uint32_t MAX_ENTITIES = 0x10000;

typedef uint64_t entID;
typedef std::bitset<MAX_COMPONENTS> ComponentMask;

class ECS {
private:
    // entity id parsing
    entID gen_entity_id_at(uint32_t index);
    entID replace_entity_id_at(uint32_t index);
    void clear_entity_id(entID&);
    uint32_t get_entity_index(entID const&) const;
    // component ID
    unsigned int p_componentCounter{0};
    template <class T>
    int get_comp_id();
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
        ObjectPool();
        ObjectPool(size_t s);
        ObjectPool(ObjectPool& other) = delete;
        ObjectPool(ObjectPool&& other);
        ObjectPool(ObjectPool const& other) = delete;
        ObjectPool& operator=(ObjectPool&& other);
        // ObjectPool& operator=(ObjectPool const& other);
        ~ObjectPool();
        void* get(size_t i);
    };
    
    std::vector<ObjectPool> pools;
    
public:
    entID newEntity();
    void removeEntity(entID&);
    bool entityValid(entID) const;
    
    size_t numEntities();
    size_t numComponents();
    
    template <class T>
    T& addComp(entID i);
    template <class T, typename... ArgTypes>
    T& addComp(entID i, ArgTypes... args);
    template <class T>
    T& getComp(entID i);
    template <class T>
    T* tryGetComp(entID i);
    template <class T>
    void removeComp(entID i);

    // template <class T>
    // void shuffleComp();
    
    template <class... Types>
    class SceneView {
        friend class ECS;
        ECS & home;
        ComponentMask mask;
        bool filter;
        SceneView(ECS* h) : home(*h) {
            filter = !(sizeof...(Types) == 0);
            mask.reset();
            if (filter){
                int ids[] = { 0, home.get_comp_id<Types>() ... };
                for (size_t i = 0; i < sizeof...(Types); i++){
                    mask.set(ids[i+1]);
                }
            }
        }
    public:
        class ViewIterator {
        private:
            ECS & home;
            uint32_t index;
            ComponentMask mask;
            bool filter;
            bool maskValid(ComponentMask& other){
                return !(((other & mask) ^ mask).any());
            }
        public:
            ViewIterator(ECS & h, uint32_t i, ComponentMask m, bool f) : home(h) {
                index = i; mask = m; filter = f;
                if (index == 0xFFFFFFFF){return;}
                auto next = home.entities.at(index);
                if (maskValid(next.mask)) {return;}
                while (1) {
                    index++;
                    if (index >= (uint32_t)home.entities.size()){
                        index = 0xFFFFFFFF;
                        return;
                    }
                    next = home.entities.at(index);
                    if (home.entityValid(next.id) && (!filter || (maskValid(next.mask)))){
                        return;
                    }
                }
            }
            entID operator*() const {
                if (index == 0xFFFFFFFF){ return 0xFFFFFFFFFFFFFFFF; }
                return home.entities.at(index).id;
            }
            bool operator==(ViewIterator const& other) const {
                return this->index == other.index;
            }
            bool operator!=(ViewIterator const& other) const {
                return this->index != other.index;
            }
            ViewIterator& operator++(){
                if (index == 0xFFFFFFFF){return *this;}
                while (1) {
                    index++;
                    if (index >= (uint32_t)home.entities.size()){
                        index = 0xFFFFFFFF;
                        return *this;
                    }
                    auto next = home.entities.at(index);
                    if (home.entityValid(next.id) && (!filter || (maskValid(next.mask)))){
                        return *this;
                    }
                }
            }
        };
        
        const ViewIterator begin() const {
            return ViewIterator(home, 0, mask, filter);
        }
        const ViewIterator end() const {
            return ViewIterator(home, -1, mask, filter);
        }
    };
    
    template <class... Types>
    SceneView<Types...> view(){
        return SceneView<Types...>(this);
    }
    
};


template <class T>
int ECS::get_comp_id() {
    static int cid = p_componentCounter++;
    return cid;
}


// template <class T>
// void ECS::shuffleComp() {
//     int compID = get_comp_id<T>();
//     if (compID >= (int)pools.size() || pools[compID] == nullptr){
//         return;
//     }
//     pools[compID]
// }

template <class T>
T& ECS::addComp(entID i){
    assert(entityValid(i));
    int compID = get_comp_id<T>();
    if (compID >= (int)pools.size()){
        pools.emplace_back(std::move(ObjectPool()));
    }
    if (pools[compID].data == nullptr){
        pools[compID] = std::move(ObjectPool(sizeof(T)));
    }
    T* component = new (pools[compID].get(get_entity_index(i))) T();
    entities.at(get_entity_index(i)).mask.set(get_comp_id<T>());
    return *component;
}

template <class T, typename... ArgTypes>
T& ECS::addComp(entID i, ArgTypes... args){
    assert(entityValid(i));
    int compID = get_comp_id<T>();
    if (compID >= (int)pools.size()){
        pools.emplace_back(std::move(ObjectPool()));
    }
    if (pools[compID].data == nullptr){
        pools[compID] = std::move(ObjectPool(sizeof(T)));
    }
    T* component = new (pools[compID].get(get_entity_index(i))) T(args...);
    entities.at(get_entity_index(i)).mask.set(get_comp_id<T>());
    return *component;
}

template <class T>
T& ECS::getComp(entID i){
    assert(entityValid(i));
    int compID = get_comp_id<T>();
    assert(entities.at(get_entity_index(i)).mask.test(compID));
    return *((T*) pools.at(compID).get(get_entity_index(i)));
}

template <class T>
T* ECS::tryGetComp(entID i){
    assert(entityValid(i));
    int compID = get_comp_id<T>();
    if (!entities.at(get_entity_index(i)).mask.test(compID)){
        return nullptr;
    }
    return ((T*) pools.at(compID).get(get_entity_index(i)));
}

template <class T>
void ECS::removeComp(entID i){
    assert(entityValid(i));
    entities.at(get_entity_index(i)).mask.reset(get_comp_id<T>());
}

#endif /* ECS_h */
