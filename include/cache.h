#ifndef CACHE_H
#define CACHE_H

#include "problem.h"
#include "solution.h"

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <utility>

#include "route_context.h"

using namespace std;

// Base class for all cache types
class Cache
{
public:
    virtual ~Cache() = default;

    // Reset the cache
    virtual void Reset(const SpecificSolution &solution, const RouteContext &context) = 0;

    // Add a route
    virtual void AddRoute(Node route_index) = 0;

    // Remove a route
    virtual void RemoveRoute(Node route_index) = 0;

    // Move a route
    virtual void MoveRoute(Node dest_route_index, Node src_route_index) = 0;

    // Save the cache state
    virtual void Save(const SpecificSolution &solution, const RouteContext &context) = 0;
};

// Handles multiple caches of different types
class CacheMap : public Cache
{
public:
    // Get or create a specific cache
    template <class T>
    T &Get(const SpecificSolution &solution, const RouteContext &context)
    {
        auto it = caches_.find(typeid(T));
        if (it == caches_.end())
        {
            auto cache = make_unique<T>();
            cache->Reset(solution, context);
            auto &cache_ref = *cache;
            caches_.emplace(typeid(T), move(cache));
            return cache_ref;
        }
        return *static_cast<T *>(it->second.get());
    }

    // Reset all caches
    void Reset(const SpecificSolution &solution, const RouteContext &context) override
    {
        for (auto &[_, cache] : caches_)
        {
            cache->Reset(solution, context);
        }
    }

    // Add a route to all caches
    void AddRoute(Node route_index) override
    {
        for (auto &[_, cache] : caches_)
        {
            cache->AddRoute(route_index);
        }
    }

    // Remove a route from all caches
    void RemoveRoute(Node route_index) override
    {
        for (auto &[_, cache] : caches_)
        {
            cache->RemoveRoute(route_index);
        }
    }

    // Move a route in all caches
    void MoveRoute(Node dest_route_index, Node src_route_index) override
    {
        for (auto &[_, cache] : caches_)
        {
            cache->MoveRoute(dest_route_index, src_route_index);
        }
    }

    // Save all caches
    void Save(const SpecificSolution &solution, const RouteContext &context) override
    {
        for (auto &[_, cache] : caches_)
        {
            cache->Save(solution, context);
        }
    }

private:
    unordered_map<type_index, unique_ptr<Cache>> caches_; // Stores caches by type
};

#endif
