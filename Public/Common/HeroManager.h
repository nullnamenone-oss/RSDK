#pragma once

#include "../RSDK.h"

#include <utility>
#include <vector>

namespace rsdk::HeroManager
{
    // HeroManager is a convenience/cache layer for the hero list.
    // Cache only identity/Object wrappers; Health/Position/IsDead/... remain live game reads.
    namespace detail
    {
        inline bool initialized{};
        inline std::vector<Object> allHeroes{};
        inline std::vector<Object> allies{};
        inline std::vector<Object> enemies{};
        inline Object player{};

        inline void Reset() noexcept
        {
            initialized = false;
            allHeroes.clear();
            allies.clear();
            enemies.clear();
            player = {};
        }

        inline void EnsureInitialized()
        {
            // Return immediately when the current game is already cached.
            if (initialized)
            {
                // Clear the cache after leaving a game so it is rebuilt when the next game becomes ready.
                if (Game::IsReady())
                    return;

                Reset();
            }

            // Do not query ObjectManager before the game is actually ready.
            if (!Game::IsReady())
                return;

            std::vector<Object> heroes = ObjectManager::AllHeroes();
            if (heroes.empty())
                return;

            Object localPlayer{};

            for (const Object& hero : heroes)
            {
                if (hero && hero.IsMe())
                {
                    localPlayer = hero;
                    break;
                }
            }

            // The cache is not complete until the hero list contains LocalPlayer.
            // The next call will retry.
            if (!localPlayer)
                return;

            std::vector<Object> allyHeroes{};
            std::vector<Object> enemyHeroes{};
            allyHeroes.reserve(heroes.size());
            enemyHeroes.reserve(heroes.size());

            for (const Object& hero : heroes)
            {
                if (!hero)
                    continue;

                if (localPlayer.IsEnemy(hero))
                    enemyHeroes.push_back(hero);
                else
                    allyHeroes.push_back(hero);
            }

            allHeroes = std::move(heroes);
            allies = std::move(allyHeroes);
            enemies = std::move(enemyHeroes);
            player = localPlayer;
            initialized = true;
        }
    }

    [[nodiscard]] inline bool IsInitialized() noexcept
    {
        return detail::initialized;
    }

    [[nodiscard]] inline Object Player()
    {
        detail::EnsureInitialized();
        return detail::player;
    }

    [[nodiscard]] inline const std::vector<Object>& AllHeroes()
    {
        detail::EnsureInitialized();
        return detail::allHeroes;
    }

    [[nodiscard]] inline const std::vector<Object>& Allies()
    {
        detail::EnsureInitialized();
        return detail::allies;
    }

    [[nodiscard]] inline const std::vector<Object>& Enemies()
    {
        detail::EnsureInitialized();
        return detail::enemies;
    }

    // Rebuild the cache explicitly for debugging or when the caller knows the object list changed.
    inline void Refresh()
    {
        detail::Reset();
        detail::EnsureInitialized();
    }
}
