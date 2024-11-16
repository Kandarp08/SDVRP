#pragma once

#include "problem.h"
#include "solution.h"

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <utility>

#include "route_context.h"

using namespace std;

class Cache
{
public:
    virtual ~Cache() = default;
    virtual void Reset(const SpecificSolution &solution, const RouteContext &context) = 0;
    virtual void AddRoute(Node route_index) = 0;
    virtual void RemoveRoute(Node route_index) = 0;
    virtual void MoveRoute(Node dest_route_index, Node src_route_index) = 0;
    virtual void Save(const SpecificSolution &solution, const RouteContext &context) = 0;
};

class CacheMap : public Cache
{
public:
    template <class T>
    T &Get(const SpecificSolution &solution, const RouteContext &context)
    {
        auto it = caches_.find(typeid(T));
        if (it == caches_.end())
        {
            auto cache = make_u nique<T>();
            cache->Reset(solution, context);
            auto &cache_ref = *cache;
            caches_.emplace(typeid(T), move(cache));
            return cache_ref;
        }
        return *static_cast<T *>(it->second.get());
    }
    void Reset(const SpecificSolution &solution, const RouteContext &context) override
    {
        for (auto &[_, cache] : caches_)
        {
            cache->Reset(solution, context);
        }
    }
    void AddRoute(Node route_index) override
    {
        for (auto &[_, cache] : caches_)
        {
            cache->AddRoute(route_index);
        }
    }
    void RemoveRoute(Node route_index) override
    {
        for (auto &[_, cache] : caches_)
        {
            cache->RemoveRoute(route_index);
        }
    }
    void MoveRoute(Node dest_route_index, Node src_route_index) override
    {
        for (auto &[_, cache] : caches_)
        {
            cache->MoveRoute(dest_route_index, src_route_index);
        }
    }
    void Save(const SpecificSolution &solution, const RouteContext &context) override
    {
        for (auto &[_, cache] : caches_)
        {
            cache->Save(solution, context);
        }
    }

private:
    unordered_map<type_index, unique_ptr<Cache>> caches_;
};