#pragma once

#include "../RSDKTypes.h"
#include "../RSDKRuntime.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>
#include <type_traits>

namespace rsdk
{
    struct Color final
    {
        std::uint8_t r{255};
        std::uint8_t g{255};
        std::uint8_t b{255};
        std::uint8_t a{255};

        constexpr Color() noexcept = default;
        constexpr Color(const std::uint8_t red, const std::uint8_t green, const std::uint8_t blue, const std::uint8_t alpha = 255) noexcept
            : r(red), g(green), b(blue), a(alpha) {}
        constexpr Color(const RSDK_Color& value) noexcept : r(value.r), g(value.g), b(value.b), a(value.a) {}

        [[nodiscard]] constexpr operator RSDK_Color() const noexcept { return {r, g, b, a}; }
        [[nodiscard]] constexpr Color WithAlpha(const std::uint8_t alpha) const noexcept { return {r, g, b, alpha}; }

        // General HSV -> RGB utility. hue wraps in [0,1), saturation/value are clamped to [0,1].
        [[nodiscard]] static Color FromHSV(float hue, float saturation = 1.0f, float value = 1.0f, const std::uint8_t alpha = 255) noexcept
        {
            hue = std::fmod(hue, 1.0f);
            if (hue < 0.0f)
                hue += 1.0f;
            saturation = std::clamp(saturation, 0.0f, 1.0f);
            value = std::clamp(value, 0.0f, 1.0f);

            const float scaled = hue * 6.0f;
            const int sector = static_cast<int>(std::floor(scaled));
            const float fraction = scaled - static_cast<float>(sector);
            const float p = value * (1.0f - saturation);
            const float q = value * (1.0f - saturation * fraction);
            const float t = value * (1.0f - saturation * (1.0f - fraction));

            float red{}, green{}, blue{};
            switch (sector % 6)
            {
            case 0: red = value; green = t; blue = p; break;
            case 1: red = q; green = value; blue = p; break;
            case 2: red = p; green = value; blue = t; break;
            case 3: red = p; green = q; blue = value; break;
            case 4: red = t; green = p; blue = value; break;
            default: red = value; green = p; blue = q; break;
            }

            return {
                static_cast<std::uint8_t>(std::clamp(red * 255.0f, 0.0f, 255.0f)),
                static_cast<std::uint8_t>(std::clamp(green * 255.0f, 0.0f, 255.0f)),
                static_cast<std::uint8_t>(std::clamp(blue * 255.0f, 0.0f, 255.0f)),
                alpha};
        }

        // Generates one random RGB color. Call once and cache it if a stable random color is desired.
        [[nodiscard]] static Color Random(const std::uint8_t alpha = 255) noexcept
        {
            static thread_local std::mt19937 generator{std::random_device{}()};
            static thread_local std::uniform_int_distribution<int> distribution(0, 255);
            return {
                static_cast<std::uint8_t>(distribution(generator)),
                static_cast<std::uint8_t>(distribution(generator)),
                static_cast<std::uint8_t>(distribution(generator)),
                alpha};
        }

        // Animated hue in cycles/second. Uses the same Game::TimeGame() source as gameplay/render animation.
        [[nodiscard]] static Color Rainbow(const float speed = 0.25f, const std::uint8_t alpha = 255, const float hueOffset = 0.0f) noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();
            const float now = api != nullptr && api->game != nullptr && api->game->GetTime != nullptr ? api->game->GetTime() : 0.0f;
            return FromHSV(now * speed + hueOffset, 1.0f, 1.0f, alpha);
        }
    };

    static_assert(sizeof(Color) == sizeof(RSDK_Color));
    static_assert(std::is_standard_layout_v<Color> && std::is_trivially_copyable_v<Color>);
}
