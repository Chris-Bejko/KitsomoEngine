#pragma once

#include "ECS.h"
#include <vector>
#include <memory>
#include "Component.h"
#include "Transform.h"

class Entity
{
public:
    Transform* transform;
    Entity()
    {
        this->transform = &this->AddComponent<Transform>(0, 0);
    }
    virtual ~Entity() {}

    template <typename T, typename... TArgs>
    inline T& AddComponent(TArgs &&...args)
    {
        T* comp(new T(std::forward<TArgs>(args)...));
        std::unique_ptr<Component> uptr{ comp };
        components.emplace_back(std::move(uptr));

        comp->entity = this;

        if (comp->Init())
        {
            componentsList[getComponentTypeID<T>()] = comp;
            componentsBitset[getComponentTypeID<T>()] = true;
            return *comp;
        }

        return *static_cast<T*>(nullptr);
    }

    template <typename T>
    inline T& GetComponent() const
    {
        auto ptr(componentsList[getComponentTypeID<T>()]);
        return *static_cast<T*>(ptr);
    }

    template <typename T>
    inline bool HasComponent() const
    {
        return componentsBitset[getComponentTypeID<T>()];
    }

    inline bool IsActive() const
    {
        return isActive;
    }

    inline void Destroy()
    {
        isActive = false;
    }

    inline void Draw()
    {
        for (auto& comp : components)
        {
            comp->draw();
        }
    }

    inline void Update()
    {
        for (auto& comp : components)
        {
            comp->update();
        }
    }

    inline void OnCollisionEnter(BoxCollider2D& other)
    {
        for(auto& comp : components)
        {
            comp->OnCollisionEnter(other);
        }
    }

private:
    bool isActive;
    ComponentList componentsList;
    ComponentBitset componentsBitset;

    std::vector<std::unique_ptr<Component>> components;
};