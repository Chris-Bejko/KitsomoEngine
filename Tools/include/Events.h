#pragma once
#include <functional>
#include <string>
#include <vector>
#include <utility>
#include <cstddef>
#include <algorithm>
#include <cstdint>
#include <type_traits>
#include <cstring>

class Entity;

template <typename T>
struct HasPendingDestroy
{
private:
    template <typename U>
    static auto test(int) -> decltype(std::declval<U &>().IsPendingDestroy(), std::true_type{});

    template <typename>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template <typename T>
struct HasEntityMember
{
private:
    template <typename U>
    static auto test(int) -> decltype(std::declval<U &>().entity, std::true_type{});

    template <typename>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template <typename... TArgs>
class Action
{
public:
    using Callback = std::function<void(TArgs...)>;

    template <typename TClass>
    static std::uintptr_t MakeMethodToken(void (TClass::*method)(TArgs...))
    {
        std::uintptr_t token = 0;
        std::memcpy(&token, &method, sizeof(method));
        return token;
    }

    template <typename TClass>
    static std::uintptr_t MakeMethodToken(void (TClass::*method)(TArgs...) const)
    {
        std::uintptr_t token = 0;
        std::memcpy(&token, &method, sizeof(method));
        return token;
    }

    struct Listener
    {
        void *instance = nullptr;
        std::uintptr_t methodToken = 0;
        std::function<bool()> isAlive = [] { return true; };
        Callback callback;

        bool Matches(void *obj, std::uintptr_t token) const
        {
            return instance == obj && methodToken == token;
        }

        bool IsDead() const
        {
            return isAlive && !isAlive();
        }
    };

    template <typename TClass>
    void Bind(TClass *instance, void (TClass::*method)(TArgs...))
    {
        auto token = MakeMethodToken(method);
        RemoveIfMatched(instance, token);

        listeners.push_back({
            instance,
            token,
            [instance]() -> bool
            {
                if constexpr (HasPendingDestroy<TClass>::value)
                {
                    return instance != nullptr && !instance->IsPendingDestroy();
                }
                else if constexpr (HasEntityMember<TClass>::value)
                {
                    return instance != nullptr && instance->entity != nullptr && !instance->entity->IsPendingDestroy();
                }
                return instance != nullptr;
            },
            [instance, method](TArgs... args)
            {
                if constexpr (HasPendingDestroy<TClass>::value)
                {
                    if (!instance || instance->IsPendingDestroy())
                        return;
                }
                else if constexpr (HasEntityMember<TClass>::value)
                {
                    if (!instance || !instance->entity || instance->entity->IsPendingDestroy())
                        return;
                }

                (instance->*method)(args...);
            }});
    }

    template <typename TClass>
    void Bind(TClass *instance, void (TClass::*method)(TArgs...) const)
    {
        auto token = MakeMethodToken(method);
        RemoveIfMatched(instance, token);

        listeners.push_back({
            instance,
            token,
            [instance]() -> bool
            {
                if constexpr (HasPendingDestroy<TClass>::value)
                {
                    return instance != nullptr && !instance->IsPendingDestroy();
                }
                else if constexpr (HasEntityMember<TClass>::value)
                {
                    return instance != nullptr && instance->entity != nullptr && !instance->entity->IsPendingDestroy();
                }
                return instance != nullptr;
            },
            [instance, method](TArgs... args)
            {
                if constexpr (HasPendingDestroy<TClass>::value)
                {
                    if (!instance || instance->IsPendingDestroy())
                        return;
                }
                else if constexpr (HasEntityMember<TClass>::value)
                {
                    if (!instance || !instance->entity || instance->entity->IsPendingDestroy())
                        return;
                }

                (instance->*method)(args...);
            }});
    }

    template <typename TClass>
    void Unsubscribe(TClass *instance, void (TClass::*method)(TArgs...))
    {
        RemoveIfMatched(instance, MakeMethodToken(method));
    }

    template <typename TClass>
    void Unsubscribe(TClass *instance, void (TClass::*method)(TArgs...) const)
    {
        RemoveIfMatched(instance, MakeMethodToken(method));
    }

    void Clear()
    {
        listeners.clear();
    }

    void Invoke(TArgs... args)
    {
        RemoveDeadListeners();
        auto snapshot = listeners;
        for (const auto &listener : snapshot)
        {
            if (!listener.isAlive || listener.isAlive())
            {
                if (listener.callback)
                    listener.callback(args...);
            }
        }
    }

    void Fire(TArgs... args) const
    {
        Invoke(args...);
    }

    void operator()(TArgs... args) const
    {
        Invoke(args...);
    }

private:
    void RemoveIfMatched(void *instance, std::uintptr_t token)
    {
        listeners.erase(
            std::remove_if(
                listeners.begin(),
                listeners.end(),
                [instance, token](const Listener &listener)
                {
                    return listener.Matches(instance, token);
                }),
            listeners.end());
    }

    void RemoveDeadListeners()
    {
        listeners.erase(
            std::remove_if(
                listeners.begin(),
                listeners.end(),
                [](const Listener &listener)
                {
                    return listener.isAlive && !listener.isAlive();
                }),
            listeners.end());
    }

    std::vector<Listener> listeners;
};

struct PlayEvent {};
struct PauseEvent {};
struct ResetEvent {};

struct SaveProjectEvent {};
struct LoadProjectEvent {};
struct OpenProjectLoadDialogEvent {};
struct ProjectLoadSuccessEvent {};
struct ProjectLoadFailedEvent {};
struct OpenProjectEvent {
    std::string projectPath;
};
struct NewProjectEvent {};

struct ReloadScriptsEvent {};


struct DeleteEntityEvent {
    Entity* entity;
};

struct OpenExportDialogEvent {};