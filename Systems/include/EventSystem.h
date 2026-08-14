#pragma once

#include <functional>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <memory>
#include <cstddef>
#include <algorithm>
#include "Events.h"

class EventSystem
{
public:
    using CallbackId = std::size_t;

    static EventSystem &get();

    template <typename Event>
    CallbackId Subscribe(std::function<void(const Event &)> callback)
    {
        auto &channel = GetChannel<Event>();
        return channel.Subscribe(std::move(callback));
    }

    template <typename Event>
    void Unsubscribe(CallbackId id)
    {
        auto it = channels.find(std::type_index(typeid(Event)));
        if (it == channels.end())
            return;

        static_cast<Channel<Event> *>(it->second.get())->Unsubscribe(id);
    }

    template <typename Event>
    void Fire(const Event &event)
    {
        auto it = channels.find(std::type_index(typeid(Event)));
        if (it == channels.end())
            return;

        static_cast<Channel<Event> *>(it->second.get())->Fire(event);
    }

    void Clear();

private:
    EventSystem() = default;
    ~EventSystem() = default;

    EventSystem(const EventSystem &) = delete;
    EventSystem &operator=(const EventSystem &) = delete;

    struct ChannelBase
    {
        virtual ~ChannelBase() = default;
    };

    template <typename Event>
    class Channel : public ChannelBase
    {
    public:
        CallbackId Subscribe(std::function<void(const Event &)> callback)
        {
            CallbackId id = nextId++;
            callbacks.push_back({id, std::move(callback)});
            return id;
        }

        void Unsubscribe(CallbackId id)
        {
            callbacks.erase(
                std::remove_if(
                    callbacks.begin(),
                    callbacks.end(),
                    [id](const auto &entry)
                    {
                        return entry.first == id;
                    }),
                callbacks.end());
        }

        void Fire(const Event &event)
        {
            auto currentCallbacks = callbacks;

            for (const auto &[id, callback] : currentCallbacks)
            {
                auto it = std::find_if(
                    callbacks.begin(),
                    callbacks.end(),
                    [id](const auto &entry)
                    {
                        return entry.first == id;
                    });

                if (it != callbacks.end())
                    it->second(event);
            }
        }

    private:
        CallbackId nextId = 1;
        std::vector<std::pair<CallbackId, std::function<void(const Event &)>>> callbacks;
    };

    template <typename Event>
    Channel<Event> &GetChannel()
    {
        const std::type_index type = std::type_index(typeid(Event));

        auto it = channels.find(type);

        if (it == channels.end())
        {
            auto channel = std::make_unique<Channel<Event>>();
            Channel<Event> *ptr = channel.get();
            channels[type] = std::move(channel);
            return *ptr;
        }

        return *static_cast<Channel<Event> *>(it->second.get());
    }

    std::unordered_map<std::type_index, std::unique_ptr<ChannelBase>> channels;
};


#define SUBSCRIBE_EVENT(EventType, Function) \
    EventSystem::get().Subscribe<EventType>([this](const EventType& event) { this->Function(event); })