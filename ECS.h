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
    // entity id parsing
    entID genEntityIDat(uint32_t index);
    entID replaceEntityIDat(uint32_t index);
    void clearEntityID(entID&);
    uint32_t getEntityIndex(entID const&) const;
    // component ID
    unsigned int p_componentCounter{0};
    template <class T>
    int getCompID();
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
        ObjectPool(size_t s);
        ~ObjectPool();
        void* get(size_t i);
    };
    
    std::vector<ObjectPool*> pools;
    
public:
    entID newEntity();
    void removeEntity(entID&);
    bool entityValid(entID) const;
    
    size_t numEntities();
    size_t numComponents();
    
    template <class T>
    T* addComp(entID i);
    template <class T>
    T* getComp(entID i);
    template <class T>
    void removeComp(entID i);
    
    template <class... Types>
    class SceneView {
        ECS & home;
        ComponentMask mask;
        bool filter;
    public:
        SceneView(ECS* h) : home(*h) {
            filter = !(sizeof...(Types) == 0);
            mask.reset();
            if (filter){
                int ids[] = { 0, home.getCompID<Types>() ... };
                for (int i = 0; i < sizeof...(Types); i++){
                    mask.set(ids[i+1]);
                }
            }
        }
        class ViewIterator {
        private:
            ECS & home;
            uint32_t index;
            ComponentMask mask;
        public:
            ViewIterator(ECS & h, uint32_t i, ComponentMask m) : home(h) {
                index = i; mask = m;
            }
            entID operator*() const {
                return home.entities.at(index).id;
            };
            bool operator==(ViewIterator const& other) const {
                return this->index == other.index;
            }
            bool operator!=(ViewIterator const& other) const {
                return this->index != other.index;
            };
            ViewIterator& operator++(){
                while (1) {
                    index++;
                    if (index >= home.entities.size()){
                        index = -1;
                        return *this;
                    }
                    auto next = home.entities.at(index);
                    if (home.entityValid(next.id) && (next.mask == mask)){
                        return *this;
                    }
                }
            }
        };
        
        const ViewIterator begin() const {
            return ViewIterator(home, 0, mask);
        }
        const ViewIterator end() const {
            return ViewIterator(home, -1, mask);
        }
    };
    
    template <class... Types>
    SceneView<Types...> view(){
        return SceneView<Types...>(this);
    }
    
};


template <class T>
int ECS::getCompID() {
    static int cid = p_componentCounter++;
    return cid;
}

template <class T>
T* ECS::addComp(entID i){
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
T* ECS::getComp(entID i){
    assert(entityValid(i));
    int compID = getCompID<T>();
    if (!entities.at(i).mask.test(compID)){
        return nullptr;
    }
    return (T*) pools.at(compID)->get(i);
}

template <class T>
void ECS::removeComp(entID i){
    assert(entityValid(i));
    entities.at(i).mask.reset(getCompID<T>());
    //TODO: dealloc?
}

#endif /* ECS_h */
