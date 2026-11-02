#pragma once

#include "RSDKRuntime.hpp"
#include "Common/Vector.h"
#include "Common/Color.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace rsdk
{
    enum class ImageShape : std::uint8_t
    {
        Square = RSDK_IMAGE_SHAPE_SQUARE,
        Rounded = RSDK_IMAGE_SHAPE_ROUNDED,
        Circle = RSDK_IMAGE_SHAPE_CIRCLE
    };

    struct ImageStyle final
    {
        ImageShape shape{ImageShape::Square};
        Color tint{255, 255, 255, 255};
        Color borderColor{0, 0, 0, 0};
        float borderThickness{};
        float rounding{8.0f};
    };

    class Texture final
    {
    public:
        constexpr Texture() noexcept = default;
        explicit constexpr Texture(const RSDK_TextureHandle handle) noexcept : handle_(handle) {}
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
        Texture(Texture&& other) noexcept : handle_(std::exchange(other.handle_, RSDK_INVALID_TEXTURE)) {}
        Texture& operator=(Texture&& other) noexcept
        {
            if (this != &other)
            {
                Reset();
                handle_ = std::exchange(other.handle_, RSDK_INVALID_TEXTURE);
            }
            return *this;
        }
        ~Texture() { Reset(); }

        [[nodiscard]] constexpr RSDK_TextureHandle Handle() const noexcept { return handle_; }
        [[nodiscard]] explicit constexpr operator bool() const noexcept { return handle_ != RSDK_INVALID_TEXTURE; }

        static Texture LoadFromFile(const char* filePath) noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();
            return Texture(api != nullptr && api->render != nullptr && api->render->LoadTextureFromFile != nullptr ? api->render->LoadTextureFromFile(filePath) : RSDK_INVALID_TEXTURE);
        }

        static Texture LoadFromMemory(const void* data, const std::size_t size) noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->render == nullptr || api->render->LoadTextureFromMemory == nullptr || size > UINT32_MAX)
                return {};
            return Texture(api->render->LoadTextureFromMemory(data, static_cast<std::uint32_t>(size)));
        }

        void Reset() noexcept
        {
            if (handle_ == RSDK_INVALID_TEXTURE)
                return;
            const auto* const api = detail::Runtime::Instance().Api();
            if (api != nullptr && api->render != nullptr && api->render->ReleaseTexture != nullptr)
                api->render->ReleaseTexture(handle_);
            handle_ = RSDK_INVALID_TEXTURE;
        }

    private:
        RSDK_TextureHandle handle_{};
    };

    namespace Render
    {
        [[nodiscard]] inline bool IsReady() noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();
            return api != nullptr && api->render != nullptr && api->render->IsReady != nullptr && api->render->IsReady() != RSDK_FALSE;
        }

        inline void Text(const Vec2& position, const char* text, const Color color, const float fontSize = 0.0f) noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();
            if (api != nullptr && api->render != nullptr && api->render->DrawText != nullptr)
                api->render->DrawText(position, text, color, fontSize);
        }

        inline void Line2D(const Vec2& from, const Vec2& to, const Color color, const float thickness = 1.0f) noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();
            if (api != nullptr && api->render != nullptr && api->render->DrawLine2D != nullptr)
                api->render->DrawLine2D(from, to, color, thickness);
        }

        inline void Circle2D(const Vec2& center, const float radius, const Color color, const float thickness = 1.0f, const std::uint32_t segments = 64u) noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();
            if (api != nullptr && api->render != nullptr && api->render->DrawCircle2D != nullptr)
                api->render->DrawCircle2D(center, radius, color, thickness, segments);
        }

        inline void Rect(const Vec2& position, const Vec2& size, const Color color, const float thickness = 1.0f, const float rounding = 0.0f) noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();
            if (api != nullptr && api->render != nullptr && api->render->DrawRect != nullptr)
                api->render->DrawRect(position, size, color, thickness, rounding);
        }

        inline void FilledRect(const Vec2& position, const Vec2& size, const Color color, const float rounding = 0.0f) noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();
            if (api != nullptr && api->render != nullptr && api->render->DrawFilledRect != nullptr)
                api->render->DrawFilledRect(position, size, color, rounding);
        }

        inline void Line3D(const Vec3& from, const Vec3& to, const Color color, const float thickness = 1.0f) noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();
            if (api != nullptr && api->render != nullptr && api->render->DrawLine3D != nullptr)
                api->render->DrawLine3D(from, to, color, thickness);
        }

        inline void Circle3D(const Vec3& center, const float radius, const Color color, const float thickness = 1.0f,
            const std::uint32_t segments = 64u, const bool clipTerrain = false) noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->render == nullptr)
                return;

            if (api->render->DrawCircle3DEx != nullptr)
            {
                api->render->DrawCircle3DEx(center, radius, color, thickness, segments, clipTerrain ? RSDK_TRUE : RSDK_FALSE);
                return;
            }

            if (api->render->DrawCircle3D != nullptr)
                api->render->DrawCircle3D(center, radius, color, thickness, segments);
        }

        // Draws soft concentric strokes behind the main world circle. v4.7 performs the layers
        // inside Core so a terrain wall mask can be sampled once and reused by every glow pass.
        inline void CircleGlow3D(const Vec3& center, const float radius, const Color color, const float thickness = 2.0f,
            const float glowSize = 10.0f, const std::uint32_t glowLayers = 4u, const std::uint32_t segments = 64u,
            const bool clipTerrain = false) noexcept
        {
            if (radius <= 0.0f || thickness <= 0.0f)
                return;

            const auto* const api = detail::Runtime::Instance().Api();
            if (api != nullptr && api->render != nullptr && api->render->DrawCircleGlow3D != nullptr)
            {
                api->render->DrawCircleGlow3D(center, radius, color, thickness, glowSize, glowLayers, segments, clipTerrain ? RSDK_TRUE : RSDK_FALSE);
                return;
            }

            if (glowSize > 0.0f && glowLayers > 0u)
            {
                for (std::uint32_t layer = glowLayers; layer > 0u; --layer)
                {
                    const float t = static_cast<float>(layer) / static_cast<float>(glowLayers);
                    const float alphaScale = 0.06f + (1.0f - t) * 0.16f;
                    const std::uint8_t layerAlpha = static_cast<std::uint8_t>(std::clamp(static_cast<float>(color.a) * alphaScale, 0.0f, 255.0f));
                    Circle3D(center, radius + glowSize * t * 0.15f, color.WithAlpha(layerAlpha), thickness + glowSize * t, segments, clipTerrain);
                }
            }

            Circle3D(center, radius, color, thickness, segments, clipTerrain);
        }

        // Rainbow hue varies around the circumference. `clockwise` controls the animation
        // direction; the spatial hue order remains stable.
        inline void RainbowCircle3D(const Vec3& center, const float radius, const float thickness = 2.0f,
            const std::uint32_t segments = 96u, const float speed = 0.20f, const bool clockwise = true,
            const std::uint8_t alpha = 255u, const bool clipTerrain = false) noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();
            if (api != nullptr && api->render != nullptr && api->render->DrawRainbowCircle3D != nullptr)
                api->render->DrawRainbowCircle3D(center, radius, thickness, segments, speed, clockwise ? RSDK_TRUE : RSDK_FALSE, alpha, clipTerrain ? RSDK_TRUE : RSDK_FALSE);
        }

        inline void Image(const Texture& texture, const Vec2& position, const Vec2& size, const ImageStyle& style = {}) noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();
            if (api != nullptr && api->render != nullptr && api->render->DrawImage != nullptr && texture)
                api->render->DrawImage(texture.Handle(), position, size, static_cast<std::uint8_t>(style.shape), style.tint, style.borderColor, style.borderThickness, style.rounding);
        }
    }
}
