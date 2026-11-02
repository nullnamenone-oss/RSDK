#pragma once

#include "RSDKPlugin.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

// C++ convenience layer built on top of the stable RSDK C ABI.
// User-facing comments in examples are Vietnamese; runtime strings stay English.
namespace rsdk
{
    class Plugin
    {
    public:
        virtual ~Plugin() = default;

        // Called exactly once when the plugin is loaded.
        // Users can create settings, read configuration, and subscribe to events here.
        virtual bool OnLoad()
        {
            return true;
        }

        // Called exactly once before the framework removes all event subscriptions.
        virtual void OnUnload()
        {
        }
    };


    namespace detail
    {
        template <typename Callable>
        struct VoidBinding final
        {
            Callable callable;
        };

        template <typename Callable>
        void RSDK_CALL VoidTrampoline(void* const userData)
        {
            auto* const binding = static_cast<VoidBinding<Callable>*>(userData);
            if (binding != nullptr)
            {
                std::invoke(binding->callable);
            }
        }

        template <typename Callable>
        struct WndProcBinding final
        {
            Callable callable;
        };

        template <typename Callable>
        void RSDK_CALL WndProcTrampoline(
            RSDK_WndProcEvent* const eventData,
            void* const userData)
        {
            auto* const binding = static_cast<WndProcBinding<Callable>*>(userData);
            if (binding != nullptr && eventData != nullptr)
            {
                std::invoke(binding->callable, *eventData);
            }
        }
        template <typename Callable>
        struct CreateObjectBinding final
        {
            Callable callable;
        };

        template <typename Callable>
        void RSDK_CALL CreateObjectTrampoline(
            const std::uintptr_t address,
            const std::uint32_t networkId,
            void* const userData)
        {
            auto* const binding =
                static_cast<CreateObjectBinding<Callable>*>(userData);

            if (binding != nullptr)
            {
                std::invoke(binding->callable, address, networkId);
            }
        }

        template <typename Callable>
        struct DeleteObjectBinding final
        {
            Callable callable;
        };

        template <typename Callable>
        void RSDK_CALL DeleteObjectTrampoline(
            const std::uintptr_t address,
            void* const userData)
        {
            auto* const binding =
                static_cast<DeleteObjectBinding<Callable>*>(userData);

            if (binding != nullptr)
            {
                std::invoke(binding->callable, address);
            }
        }
        template <typename Callable>
        struct NewPathBinding final
        {
            Callable callable;
        };

        template <typename Callable>
        void RSDK_CALL NewPathTrampoline(
            const RSDK_NewPathEvent* const eventData,
            void* const userData)
        {
            auto* const binding = static_cast<NewPathBinding<Callable>*>(userData);

            if (binding == nullptr || eventData == nullptr)
                return;

            NewPathEvent event{};
            event.sender = Object(static_cast<RSDK_ObjectHandle>(eventData->sender));
            event.isDash = eventData->isDash != RSDK_FALSE;
            event.dashSpeed = eventData->dashSpeed;
            event.dashGravityCount = eventData->dashGravityCount;
            event.dashGravityFlags = eventData->dashGravityFlags;

            if (eventData->path != nullptr && eventData->pathCount != 0u)
                event.path.assign(eventData->path, eventData->path + eventData->pathCount);

            std::invoke(binding->callable, event);
        }

        template <typename Callable>
        struct ProcessSpellBinding final
        {
            Callable callable;
        };

        template <typename Callable>
        void RSDK_CALL ProcessSpellTrampoline(
            const RSDK_ProcessSpellEvent* const eventData,
            void* const userData)
        {
            auto* const binding = static_cast<ProcessSpellBinding<Callable>*>(userData);

            if (binding == nullptr || eventData == nullptr)
                return;

            ProcessSpellEvent event
            {
                Object(static_cast<RSDK_ObjectHandle>(eventData->sender)),
                SpellCast(static_cast<RSDK_SpellCastHandle>(eventData->spellCast))
            };

            std::invoke(binding->callable, event);
        }

        template <typename Callable>
        struct DoCastBinding final
        {
            Callable callable;
        };

        template <typename Callable>
        void RSDK_CALL DoCastTrampoline(
            const RSDK_DoCastSpellEvent* const eventData,
            void* const userData)
        {
            auto* const binding = static_cast<DoCastBinding<Callable>*>(userData);
            if (binding == nullptr || eventData == nullptr)
                return;

            DoCastSpellEvent event
            {
                SpellCast(static_cast<RSDK_SpellCastHandle>(eventData->spellCast)),
                Object(static_cast<RSDK_ObjectHandle>(eventData->sender))
            };

            std::invoke(binding->callable, event);
        }

        template <typename Callable>
        struct PlayAnimationBinding final
        {
            Callable callable;
        };

        template <typename Callable>
        void RSDK_CALL PlayAnimationTrampoline(
            const RSDK_PlayAnimationEvent* const eventData,
            void* const userData)
        {
            auto* const binding = static_cast<PlayAnimationBinding<Callable>*>(userData);
            if (binding == nullptr || eventData == nullptr)
                return;

            PlayAnimationEvent event{};
            event.sender = Object(static_cast<RSDK_ObjectHandle>(eventData->sender));
            event.animationName = eventData->animationName != nullptr ? eventData->animationName : "";

            std::invoke(binding->callable, event);
        }

        template <typename Callable>
        struct StopCastBinding final
        {
            Callable callable;
        };

        template <typename Callable>
        void RSDK_CALL StopCastTrampoline(
            const RSDK_StopCastEvent* const eventData,
            void* const userData)
        {
            auto* const binding = static_cast<StopCastBinding<Callable>*>(userData);
            if (binding == nullptr || eventData == nullptr)
                return;

            StopCastEvent event
            {
                Object(static_cast<RSDK_ObjectHandle>(eventData->sender)),
                SpellCast(static_cast<RSDK_SpellCastHandle>(eventData->spellCast)),
                eventData->stopAnimation != RSDK_FALSE,
                eventData->destroyMissile != RSDK_FALSE,
                eventData->missileNetworkId
            };

            std::invoke(binding->callable, event);
        }

        template <typename Callable>
        struct CastSpellBinding final
        {
            Callable callable;
        };

        template <typename Callable>
        void RSDK_CALL CastSpellTrampoline(
            RSDK_CastSpellEvent* const eventData,
            void* const userData)
        {
            auto* const binding = static_cast<CastSpellBinding<Callable>*>(userData);
            if (binding == nullptr || eventData == nullptr)
                return;

            CastSpellEvent event
            {
                Object(static_cast<RSDK_ObjectHandle>(eventData->sender)),
                Object(static_cast<RSDK_ObjectHandle>(eventData->target)),
                SpellData(static_cast<RSDK_SpellDataHandle>(eventData->spellData)),
                Vec3(eventData->sourcePosition),
                Vec3(eventData->destinationPosition),
                Vec3(eventData->castEndPosition),
                Vec3(eventData->unknownPosition),
                eventData->sourceId,
                eventData->targetId,
                static_cast<SpellSlot>(eventData->slot),
                eventData->process != RSDK_FALSE
            };

            std::invoke(binding->callable, event);
            eventData->process = event.process ? RSDK_TRUE : RSDK_FALSE;
        }

        template <typename Callable>
        struct UpdateChargeableSpellBinding final
        {
            Callable callable;
        };

        template <typename Callable>
        void RSDK_CALL UpdateChargeableSpellTrampoline(
            RSDK_UpdateChargeableSpellEvent* const eventData,
            void* const userData)
        {
            auto* const binding = static_cast<UpdateChargeableSpellBinding<Callable>*>(userData);
            if (binding == nullptr || eventData == nullptr)
                return;

            UpdateChargeableSpellEvent event
            {
                Object(static_cast<RSDK_ObjectHandle>(eventData->sender)),
                SpellData(static_cast<RSDK_SpellDataHandle>(eventData->spellData)),
                static_cast<SpellSlot>(eventData->slot),
                Vec3(eventData->position),
                eventData->flag != RSDK_FALSE,
                eventData->process != RSDK_FALSE
            };

            std::invoke(binding->callable, event);
            eventData->process = event.process ? RSDK_TRUE : RSDK_FALSE;
        }

        template <typename Callable>
        struct BuffAddRemoveBinding final
        {
            Callable callable;
        };

        template <typename Callable>
        void RSDK_CALL BuffAddRemoveTrampoline(
            const RSDK_BuffAddRemoveEvent* const eventData,
            void* const userData)
        {
            auto* const binding = static_cast<BuffAddRemoveBinding<Callable>*>(userData);
            if (binding == nullptr || eventData == nullptr)
                return;

            BuffAddRemoveEvent event
            {
                Object(static_cast<RSDK_ObjectHandle>(eventData->sender)),
                Buff(static_cast<RSDK_BuffHandle>(eventData->buff)),
                eventData->added != RSDK_FALSE
            };

            std::invoke(binding->callable, event);
        }

        template <typename Callable>
        struct UpdateBuffBinding final
        {
            Callable callable;
        };

        template <typename Callable>
        void RSDK_CALL UpdateBuffTrampoline(
            const RSDK_UpdateBuffEvent* const eventData,
            void* const userData)
        {
            auto* const binding = static_cast<UpdateBuffBinding<Callable>*>(userData);
            if (binding == nullptr || eventData == nullptr)
                return;

            UpdateBuffEvent event
            {
                Object(static_cast<RSDK_ObjectHandle>(eventData->sender)),
                Buff(static_cast<RSDK_BuffHandle>(eventData->buff)),
                eventData->stacks
            };

            std::invoke(binding->callable, event);
        }

        template <typename Callable>
        struct SpellImpactBinding final
        {
            Callable callable;
        };

        template <typename Callable>
        void RSDK_CALL SpellImpactTrampoline(
            const RSDK_SpellImpactEvent* const eventData,
            void* const userData)
        {
            auto* const binding = static_cast<SpellImpactBinding<Callable>*>(userData);
            if (binding == nullptr || eventData == nullptr)
                return;

            SpellImpactEvent event
            {
                Object(static_cast<RSDK_ObjectHandle>(eventData->sender)),
                Object(static_cast<RSDK_ObjectHandle>(eventData->target)),
                SpellCast(static_cast<RSDK_SpellCastHandle>(eventData->spellCast)),
                eventData->nativeArgument
            };

            std::invoke(binding->callable, event);
        }

        template <typename Callable>
        struct IssueOrderBinding final
        {
            Callable callable;
        };

        template <typename Callable>
        void RSDK_CALL IssueOrderTrampoline(
            RSDK_IssueOrderEvent* const eventData,
            void* const userData)
        {
            auto* const binding =
                static_cast<IssueOrderBinding<Callable>*>(userData);

            if (binding == nullptr || eventData == nullptr)
                return;

            IssueOrderEvent event
            {
                Object(static_cast<RSDK_ObjectHandle>(eventData->sender)),
                static_cast<GameObjectOrder>(eventData->order),
                eventData->position,
                Object(static_cast<RSDK_ObjectHandle>(eventData->target)),
                eventData->isAttackMove != RSDK_FALSE,
                eventData->process != RSDK_FALSE
            };

            std::invoke(binding->callable, event);
            eventData->process = event.process ? RSDK_TRUE : RSDK_FALSE;
        }

        // Normalize arguments before passing them through the printf-style C ABI.
        // Convert std::string to const char* to avoid MSVC C4840.
        inline const char* LogArgument(const std::string& value) noexcept
        {
            return value.c_str();
        }

        template <
            typename T,
            std::enable_if_t<
                !std::is_same_v<
                    std::remove_cv_t<std::remove_reference_t<T>>,
                    std::string>,
                int> = 0>
        decltype(auto) LogArgument(T&& value) noexcept
        {
            using Value = std::remove_cv_t<std::remove_reference_t<T>>;

            static_assert(!std::is_same_v<Value, std::string_view>,
                "std::string_view cannot be passed directly to printf-style logging. Use std::string(value), or value.data() with an explicit precision.");

            static_assert(
                std::is_arithmetic_v<Value> ||
                std::is_enum_v<Value> ||
                std::is_pointer_v<Value> ||
                std::is_array_v<Value> ||
                std::is_same_v<Value, std::nullptr_t>,
                "Unsupported class type passed to printf-style logging.");

            return std::forward<T>(value);
        }
    }

    namespace Log
    {
        template <typename... Args>
        void Trace(const char* format, Args&&... args)
        {
            const auto* const api = detail::Runtime::Instance().Api();
            if (api != nullptr && api->log != nullptr && api->log->Trace != nullptr)
            {
                api->log->Trace(format, detail::LogArgument(std::forward<Args>(args))...);
            }
        }

        template <typename... Args>
        void Info(const char* format, Args&&... args)
        {
            const auto* const api = detail::Runtime::Instance().Api();
            if (api != nullptr && api->log != nullptr && api->log->Info != nullptr)
            {
                api->log->Info(format, detail::LogArgument(std::forward<Args>(args))...);
            }
        }

        template <typename... Args>
        void Warning(const char* format, Args&&... args)
        {
            const auto* const api = detail::Runtime::Instance().Api();
            if (api != nullptr && api->log != nullptr && api->log->Warning != nullptr)
            {
                api->log->Warning(format, detail::LogArgument(std::forward<Args>(args))...);
            }
        }

        template <typename... Args>
        void Error(const char* format, Args&&... args)
        {
            const auto* const api = detail::Runtime::Instance().Api();
            if (api != nullptr && api->log != nullptr && api->log->Error != nullptr)
            {
                api->log->Error(format, detail::LogArgument(std::forward<Args>(args))...);
            }
        }

        // Clear only Log\Plugins.log; the logger continues operating normally.
        inline bool Clear() noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();
            return api != nullptr && api->log != nullptr && api->log->Clear != nullptr &&
                api->log->Clear() != RSDK_FALSE;
        }
    }

    class VoidEvent final
    {
    public:
        using RegisterFunction = RSDK_EventToken(RSDK_CALL*)(RSDK_OnTickCallback, void*);

        constexpr explicit VoidEvent(const RegisterFunction function) noexcept
            : register_(function)
        {
        }

        template <typename Callable>
        bool Subscribe(Callable&& callable) const
        {
            using StoredCallable = std::decay_t<Callable>;
            using Binding = detail::VoidBinding<StoredCallable>;

            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->events == nullptr || register_ == nullptr)
            {
                return false;
            }

            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const RSDK_EventToken token = register_(
                &detail::VoidTrampoline<StoredCallable>,
                binding);

            return detail::Runtime::Instance().Track(
                token,
                binding,
                &detail::DestroyContext<Binding>);
        }

        template <typename TObject>
        bool Subscribe(TObject* const object, void (TObject::*method)()) const
        {
            return Subscribe([object, method]()
            {
                if (object != nullptr)
                {
                    std::invoke(method, object);
                }
            });
        }

        template <typename Callable>
        [[nodiscard]] EventConnection Connect(Callable&& callable) const
        {
            using StoredCallable = std::decay_t<Callable>;
            using Binding = detail::VoidBinding<StoredCallable>;

            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->events == nullptr || register_ == nullptr)
                return {};

            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const RSDK_EventToken token = register_(&detail::VoidTrampoline<StoredCallable>, binding);
            if (!detail::Runtime::Instance().Track(token, binding, &detail::DestroyContext<Binding>))
                return {};

            return EventConnection(token);
        }

        template <typename Callable>
        void operator+=(Callable&& callable) const
        {
            Subscribe(std::forward<Callable>(callable));
        }

    private:
        RegisterFunction register_{};
    };

    class WndProcEvent final
    {
    public:
        template <typename Callable>
        bool Subscribe(Callable&& callable) const
        {
            using StoredCallable = std::decay_t<Callable>;
            using Binding = detail::WndProcBinding<StoredCallable>;

            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->events == nullptr || api->events->RegisterOnWndProc == nullptr)
            {
                return false;
            }

            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const RSDK_EventToken token = api->events->RegisterOnWndProc(
                &detail::WndProcTrampoline<StoredCallable>,
                binding);

            return detail::Runtime::Instance().Track(
                token,
                binding,
                &detail::DestroyContext<Binding>);
        }

        template <typename TObject>
        bool Subscribe(
            TObject* const object,
            void (TObject::*method)(const RSDK_WndProcEvent&)) const
        {
            return Subscribe([object, method](const RSDK_WndProcEvent& eventData)
            {
                if (object != nullptr)
                {
                    std::invoke(method, object, eventData);
                }
            });
        }

        template <typename Callable>
        [[nodiscard]] EventConnection Connect(Callable&& callable) const
        {
            using StoredCallable = std::decay_t<Callable>;
            using Binding = detail::WndProcBinding<StoredCallable>;

            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->events == nullptr || api->events->RegisterOnWndProc == nullptr)
                return {};

            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const RSDK_EventToken token = api->events->RegisterOnWndProc(&detail::WndProcTrampoline<StoredCallable>, binding);
            if (!detail::Runtime::Instance().Track(token, binding, &detail::DestroyContext<Binding>))
                return {};

            return EventConnection(token);
        }

        template <typename Callable>
        void operator+=(Callable&& callable) const
        {
            Subscribe(std::forward<Callable>(callable));
        }
    };

    class CreateObjectEvent final
    {
    public:
        using RegisterFunction =
            RSDK_EventToken(RSDK_CALL*)(
                RSDK_OnCreateObjectCallback,
                void*);

        constexpr explicit CreateObjectEvent(
            const RegisterFunction function) noexcept
            : register_(function)
        {
        }

        template <typename Callable>
        bool Subscribe(Callable&& callable) const
        {
            using StoredCallable = std::decay_t<Callable>;
            using Binding = detail::CreateObjectBinding<StoredCallable>;

            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->events == nullptr || register_ == nullptr)
                return false;

            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const RSDK_EventToken token = register_(
                &detail::CreateObjectTrampoline<StoredCallable>,
                binding);

            return detail::Runtime::Instance().Track(
                token,
                binding,
                &detail::DestroyContext<Binding>);
        }

        template <typename TObject>
        bool Subscribe(
            TObject* const object,
            void (TObject::*method)(std::uintptr_t, std::uint32_t)) const
        {
            return Subscribe([object, method](
                const std::uintptr_t address,
                const std::uint32_t networkId)
            {
                if (object != nullptr)
                    std::invoke(method, object, address, networkId);
            });
        }

        template <typename Callable>
        [[nodiscard]] EventConnection Connect(Callable&& callable) const
        {
            using StoredCallable = std::decay_t<Callable>;
            using Binding = detail::CreateObjectBinding<StoredCallable>;

            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->events == nullptr || register_ == nullptr)
                return {};

            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const RSDK_EventToken token = register_(&detail::CreateObjectTrampoline<StoredCallable>, binding);
            if (!detail::Runtime::Instance().Track(token, binding, &detail::DestroyContext<Binding>))
                return {};

            return EventConnection(token);
        }

        template <typename Callable>
        void operator+=(Callable&& callable) const
        {
            Subscribe(std::forward<Callable>(callable));
        }

    private:
        RegisterFunction register_{};
    };

    class DeleteObjectEvent final
    {
    public:
        using RegisterFunction =
            RSDK_EventToken(RSDK_CALL*)(
                RSDK_OnDeleteObjectCallback,
                void*);

        constexpr explicit DeleteObjectEvent(
            const RegisterFunction function) noexcept
            : register_(function)
        {
        }

        template <typename Callable>
        bool Subscribe(Callable&& callable) const
        {
            using StoredCallable = std::decay_t<Callable>;
            using Binding = detail::DeleteObjectBinding<StoredCallable>;

            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->events == nullptr || register_ == nullptr)
                return false;

            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const RSDK_EventToken token = register_(
                &detail::DeleteObjectTrampoline<StoredCallable>,
                binding);

            return detail::Runtime::Instance().Track(
                token,
                binding,
                &detail::DestroyContext<Binding>);
        }

        template <typename TObject>
        bool Subscribe(
            TObject* const object,
            void (TObject::*method)(std::uintptr_t)) const
        {
            return Subscribe([object, method](const std::uintptr_t address)
            {
                if (object != nullptr)
                    std::invoke(method, object, address);
            });
        }

        template <typename Callable>
        [[nodiscard]] EventConnection Connect(Callable&& callable) const
        {
            using StoredCallable = std::decay_t<Callable>;
            using Binding = detail::DeleteObjectBinding<StoredCallable>;

            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->events == nullptr || register_ == nullptr)
                return {};

            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const RSDK_EventToken token = register_(&detail::DeleteObjectTrampoline<StoredCallable>, binding);
            if (!detail::Runtime::Instance().Track(token, binding, &detail::DestroyContext<Binding>))
                return {};

            return EventConnection(token);
        }

        template <typename Callable>
        void operator+=(Callable&& callable) const
        {
            Subscribe(std::forward<Callable>(callable));
        }

    private:
        RegisterFunction register_{};
    };

    class IssueOrderEventSource final
    {
    public:
        using RegisterFunction =
            RSDK_EventToken(RSDK_CALL*)(
                RSDK_OnIssueOrderCallback,
                void*);

        constexpr explicit IssueOrderEventSource(
            const RegisterFunction function) noexcept
            : register_(function)
        {
        }

        template <typename Callable>
        bool Subscribe(Callable&& callable) const
        {
            using StoredCallable = std::decay_t<Callable>;
            using Binding = detail::IssueOrderBinding<StoredCallable>;

            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->events == nullptr || register_ == nullptr)
                return false;

            auto* const binding = new Binding{
                std::forward<Callable>(callable)};

            const RSDK_EventToken token = register_(
                &detail::IssueOrderTrampoline<StoredCallable>,
                binding);

            return detail::Runtime::Instance().Track(
                token, binding,
                &detail::DestroyContext<Binding>);
        }

        template <typename Callable>
        [[nodiscard]] EventConnection Connect(Callable&& callable) const
        {
            using StoredCallable = std::decay_t<Callable>;
            using Binding = detail::IssueOrderBinding<StoredCallable>;

            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->events == nullptr || register_ == nullptr)
                return {};

            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const RSDK_EventToken token = register_(&detail::IssueOrderTrampoline<StoredCallable>, binding);
            if (!detail::Runtime::Instance().Track(token, binding, &detail::DestroyContext<Binding>))
                return {};

            return EventConnection(token);
        }

        template <typename Callable>
        void operator+=(Callable&& callable) const
        {
            Subscribe(std::forward<Callable>(callable));
        }

    private:
        RegisterFunction register_{};
    };

    class NewPathEventSource final
    {
    public:
        using RegisterFunction = RSDK_EventToken(RSDK_CALL*)(RSDK_OnNewPathCallback, void*);

        constexpr explicit NewPathEventSource(const RegisterFunction function) noexcept
            : register_(function)
        {
        }

        template <typename Callable>
        bool Subscribe(Callable&& callable) const
        {
            using StoredCallable = std::decay_t<Callable>;
            using Binding = detail::NewPathBinding<StoredCallable>;

            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->events == nullptr || register_ == nullptr)
                return false;

            auto* const binding = new Binding{std::forward<Callable>(callable)};

            const RSDK_EventToken token = register_(
                &detail::NewPathTrampoline<StoredCallable>,
                binding);

            return detail::Runtime::Instance().Track(
                token, binding,
                &detail::DestroyContext<Binding>);
        }

        template <typename Callable>
        [[nodiscard]] EventConnection Connect(Callable&& callable) const
        {
            using StoredCallable = std::decay_t<Callable>;
            using Binding = detail::NewPathBinding<StoredCallable>;

            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->events == nullptr || register_ == nullptr)
                return {};

            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const RSDK_EventToken token = register_(&detail::NewPathTrampoline<StoredCallable>, binding);
            if (!detail::Runtime::Instance().Track(token, binding, &detail::DestroyContext<Binding>))
                return {};

            return EventConnection(token);
        }

        template <typename Callable>
        void operator+=(Callable&& callable) const
        {
            Subscribe(std::forward<Callable>(callable));
        }

    private:
        RegisterFunction register_{};
    };

    class ProcessSpellEventSource final
    {
    public:
        using RegisterFunction = RSDK_EventToken(RSDK_CALL*)(RSDK_OnProcessSpellCallback, void*);

        constexpr explicit ProcessSpellEventSource(const RegisterFunction function) noexcept
            : register_(function)
        {
        }

        template <typename Callable>
        bool Subscribe(Callable&& callable) const
        {
            using StoredCallable = std::decay_t<Callable>;
            using Binding = detail::ProcessSpellBinding<StoredCallable>;

            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->events == nullptr || register_ == nullptr)
                return false;

            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const RSDK_EventToken token = register_(
                &detail::ProcessSpellTrampoline<StoredCallable>,
                binding);

            return detail::Runtime::Instance().Track(
                token, binding,
                &detail::DestroyContext<Binding>);
        }

        template <typename Callable>
        [[nodiscard]] EventConnection Connect(Callable&& callable) const
        {
            using StoredCallable = std::decay_t<Callable>;
            using Binding = detail::ProcessSpellBinding<StoredCallable>;

            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->events == nullptr || register_ == nullptr)
                return {};

            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const RSDK_EventToken token = register_(&detail::ProcessSpellTrampoline<StoredCallable>, binding);
            if (!detail::Runtime::Instance().Track(token, binding, &detail::DestroyContext<Binding>))
                return {};

            return EventConnection(token);
        }

        template <typename Callable>
        void operator+=(Callable&& callable) const
        {
            Subscribe(std::forward<Callable>(callable));
        }

    private:
        RegisterFunction register_{};
    };

    class DoCastEventSource final
    {
    public:
        using RegisterFunction = RSDK_EventToken(RSDK_CALL*)(RSDK_OnDoCastCallback, void*);

        constexpr explicit DoCastEventSource(const RegisterFunction function) noexcept : register_(function) {}

        template <typename Callable>
        bool Subscribe(Callable&& callable) const
        {
            using StoredCallable = std::decay_t<Callable>;
            using Binding = detail::DoCastBinding<StoredCallable>;

            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->events == nullptr || register_ == nullptr)
                return false;

            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const RSDK_EventToken token = register_(&detail::DoCastTrampoline<StoredCallable>, binding);

            return detail::Runtime::Instance().Track(token, binding, &detail::DestroyContext<Binding>);
        }

        template <typename Callable>
        [[nodiscard]] EventConnection Connect(Callable&& callable) const
        {
            using StoredCallable = std::decay_t<Callable>;
            using Binding = detail::DoCastBinding<StoredCallable>;

            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->events == nullptr || register_ == nullptr)
                return {};

            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const RSDK_EventToken token = register_(&detail::DoCastTrampoline<StoredCallable>, binding);
            if (!detail::Runtime::Instance().Track(token, binding, &detail::DestroyContext<Binding>))
                return {};

            return EventConnection(token);
        }

        template <typename Callable>
        void operator+=(Callable&& callable) const
        {
            Subscribe(std::forward<Callable>(callable));
        }

    private:
        RegisterFunction register_{};
    };

    class PlayAnimationEventSource final
    {
    public:
        using RegisterFunction = RSDK_EventToken(RSDK_CALL*)(RSDK_OnPlayAnimationCallback, void*);

        constexpr explicit PlayAnimationEventSource(const RegisterFunction function) noexcept : register_(function) {}

        template <typename Callable>
        bool Subscribe(Callable&& callable) const
        {
            using StoredCallable = std::decay_t<Callable>;
            using Binding = detail::PlayAnimationBinding<StoredCallable>;

            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->events == nullptr || register_ == nullptr)
                return false;

            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const RSDK_EventToken token = register_(&detail::PlayAnimationTrampoline<StoredCallable>, binding);

            return detail::Runtime::Instance().Track(token, binding, &detail::DestroyContext<Binding>);
        }

        template <typename Callable>
        [[nodiscard]] EventConnection Connect(Callable&& callable) const
        {
            using StoredCallable = std::decay_t<Callable>;
            using Binding = detail::PlayAnimationBinding<StoredCallable>;

            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->events == nullptr || register_ == nullptr)
                return {};

            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const RSDK_EventToken token = register_(&detail::PlayAnimationTrampoline<StoredCallable>, binding);
            if (!detail::Runtime::Instance().Track(token, binding, &detail::DestroyContext<Binding>))
                return {};

            return EventConnection(token);
        }

        template <typename Callable>
        void operator+=(Callable&& callable) const
        {
            Subscribe(std::forward<Callable>(callable));
        }

    private:
        RegisterFunction register_{};
    };

    class StopCastEventSource final
    {
    public:
        using RegisterFunction = RSDK_EventToken(RSDK_CALL*)(RSDK_OnStopCastCallback, void*);

        constexpr explicit StopCastEventSource(const RegisterFunction function) noexcept : register_(function) {}

        template <typename Callable>
        bool Subscribe(Callable&& callable) const
        {
            using StoredCallable = std::decay_t<Callable>;
            using Binding = detail::StopCastBinding<StoredCallable>;

            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->events == nullptr || register_ == nullptr)
                return false;

            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const RSDK_EventToken token = register_(&detail::StopCastTrampoline<StoredCallable>, binding);

            return detail::Runtime::Instance().Track(token, binding, &detail::DestroyContext<Binding>);
        }

        template <typename Callable>
        [[nodiscard]] EventConnection Connect(Callable&& callable) const
        {
            using StoredCallable = std::decay_t<Callable>;
            using Binding = detail::StopCastBinding<StoredCallable>;

            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->events == nullptr || register_ == nullptr)
                return {};

            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const RSDK_EventToken token = register_(&detail::StopCastTrampoline<StoredCallable>, binding);
            if (!detail::Runtime::Instance().Track(token, binding, &detail::DestroyContext<Binding>))
                return {};

            return EventConnection(token);
        }

        template <typename Callable>
        void operator+=(Callable&& callable) const
        {
            Subscribe(std::forward<Callable>(callable));
        }

    private:
        RegisterFunction register_{};
    };

    class CastSpellEventSource final
    {
    public:
        using RegisterFunction = RSDK_EventToken(RSDK_CALL*)(RSDK_OnCastSpellCallback, void*);

        constexpr explicit CastSpellEventSource(const RegisterFunction function) noexcept : register_(function) {}

        template <typename Callable>
        bool Subscribe(Callable&& callable) const
        {
            using StoredCallable = std::decay_t<Callable>;
            using Binding = detail::CastSpellBinding<StoredCallable>;

            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->events == nullptr || register_ == nullptr)
                return false;

            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const RSDK_EventToken token = register_(&detail::CastSpellTrampoline<StoredCallable>, binding);
            return detail::Runtime::Instance().Track(token, binding, &detail::DestroyContext<Binding>);
        }

        template <typename Callable>
        [[nodiscard]] EventConnection Connect(Callable&& callable) const
        {
            using StoredCallable = std::decay_t<Callable>;
            using Binding = detail::CastSpellBinding<StoredCallable>;

            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->events == nullptr || register_ == nullptr)
                return {};

            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const RSDK_EventToken token = register_(&detail::CastSpellTrampoline<StoredCallable>, binding);
            if (!detail::Runtime::Instance().Track(token, binding, &detail::DestroyContext<Binding>))
                return {};

            return EventConnection(token);
        }

        template <typename Callable>
        void operator+=(Callable&& callable) const
        {
            Subscribe(std::forward<Callable>(callable));
        }

    private:
        RegisterFunction register_{};
    };

    class UpdateChargeableSpellEventSource final
    {
    public:
        using RegisterFunction = RSDK_EventToken(RSDK_CALL*)(RSDK_OnUpdateChargeableSpellCallback, void*);

        constexpr explicit UpdateChargeableSpellEventSource(const RegisterFunction function) noexcept : register_(function) {}

        template <typename Callable>
        bool Subscribe(Callable&& callable) const
        {
            using StoredCallable = std::decay_t<Callable>;
            using Binding = detail::UpdateChargeableSpellBinding<StoredCallable>;

            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->events == nullptr || register_ == nullptr)
                return false;

            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const RSDK_EventToken token = register_(&detail::UpdateChargeableSpellTrampoline<StoredCallable>, binding);

            return detail::Runtime::Instance().Track(token, binding, &detail::DestroyContext<Binding>);
        }

        template <typename Callable>
        [[nodiscard]] EventConnection Connect(Callable&& callable) const
        {
            using StoredCallable = std::decay_t<Callable>;
            using Binding = detail::UpdateChargeableSpellBinding<StoredCallable>;

            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->events == nullptr || register_ == nullptr)
                return {};

            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const RSDK_EventToken token = register_(&detail::UpdateChargeableSpellTrampoline<StoredCallable>, binding);
            if (!detail::Runtime::Instance().Track(token, binding, &detail::DestroyContext<Binding>))
                return {};

            return EventConnection(token);
        }

        template <typename Callable>
        void operator+=(Callable&& callable) const
        {
            Subscribe(std::forward<Callable>(callable));
        }

    private:
        RegisterFunction register_{};
    };

    class BuffAddRemoveEventSource final
    {
    public:
        using RegisterFunction = RSDK_EventToken(RSDK_CALL*)(RSDK_OnBuffAddRemoveCallback, void*);

        constexpr explicit BuffAddRemoveEventSource(const RegisterFunction function) noexcept : register_(function) {}

        template <typename Callable>
        bool Subscribe(Callable&& callable) const
        {
            using StoredCallable = std::decay_t<Callable>;
            using Binding = detail::BuffAddRemoveBinding<StoredCallable>;

            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->events == nullptr || register_ == nullptr)
                return false;

            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const RSDK_EventToken token = register_(&detail::BuffAddRemoveTrampoline<StoredCallable>, binding);

            return detail::Runtime::Instance().Track(token, binding, &detail::DestroyContext<Binding>);
        }

        template <typename Callable>
        [[nodiscard]] EventConnection Connect(Callable&& callable) const
        {
            using StoredCallable = std::decay_t<Callable>;
            using Binding = detail::BuffAddRemoveBinding<StoredCallable>;

            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->events == nullptr || register_ == nullptr)
                return {};

            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const RSDK_EventToken token = register_(&detail::BuffAddRemoveTrampoline<StoredCallable>, binding);
            if (!detail::Runtime::Instance().Track(token, binding, &detail::DestroyContext<Binding>))
                return {};

            return EventConnection(token);
        }

        template <typename Callable>
        void operator+=(Callable&& callable) const
        {
            Subscribe(std::forward<Callable>(callable));
        }

    private:
        RegisterFunction register_{};
    };

    class UpdateBuffEventSource final
    {
    public:
        using RegisterFunction = RSDK_EventToken(RSDK_CALL*)(RSDK_OnUpdateBuffCallback, void*);

        constexpr explicit UpdateBuffEventSource(const RegisterFunction function) noexcept : register_(function) {}

        template <typename Callable>
        bool Subscribe(Callable&& callable) const
        {
            using StoredCallable = std::decay_t<Callable>;
            using Binding = detail::UpdateBuffBinding<StoredCallable>;

            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->events == nullptr || register_ == nullptr)
                return false;

            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const RSDK_EventToken token = register_(&detail::UpdateBuffTrampoline<StoredCallable>, binding);
            return detail::Runtime::Instance().Track(token, binding, &detail::DestroyContext<Binding>);
        }

        template <typename Callable>
        [[nodiscard]] EventConnection Connect(Callable&& callable) const
        {
            using StoredCallable = std::decay_t<Callable>;
            using Binding = detail::UpdateBuffBinding<StoredCallable>;

            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->events == nullptr || register_ == nullptr)
                return {};

            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const RSDK_EventToken token = register_(&detail::UpdateBuffTrampoline<StoredCallable>, binding);
            if (!detail::Runtime::Instance().Track(token, binding, &detail::DestroyContext<Binding>))
                return {};

            return EventConnection(token);
        }

        template <typename Callable>
        void operator+=(Callable&& callable) const
        {
            Subscribe(std::forward<Callable>(callable));
        }

    private:
        RegisterFunction register_{};
    };

    class SpellImpactEventSource final
    {
    public:
        using RegisterFunction = RSDK_EventToken(RSDK_CALL*)(RSDK_OnSpellImpactCallback, void*);

        constexpr explicit SpellImpactEventSource(const RegisterFunction function) noexcept : register_(function) {}

        template <typename Callable>
        bool Subscribe(Callable&& callable) const
        {
            using StoredCallable = std::decay_t<Callable>;
            using Binding = detail::SpellImpactBinding<StoredCallable>;

            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->events == nullptr || register_ == nullptr)
                return false;

            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const RSDK_EventToken token = register_(&detail::SpellImpactTrampoline<StoredCallable>, binding);
            return detail::Runtime::Instance().Track(token, binding, &detail::DestroyContext<Binding>);
        }

        template <typename Callable>
        [[nodiscard]] EventConnection Connect(Callable&& callable) const
        {
            using StoredCallable = std::decay_t<Callable>;
            using Binding = detail::SpellImpactBinding<StoredCallable>;

            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->events == nullptr || register_ == nullptr)
                return {};

            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const RSDK_EventToken token = register_(&detail::SpellImpactTrampoline<StoredCallable>, binding);
            if (!detail::Runtime::Instance().Track(token, binding, &detail::DestroyContext<Binding>))
                return {};

            return EventConnection(token);
        }

        template <typename Callable>
        void operator+=(Callable&& callable) const
        {
            Subscribe(std::forward<Callable>(callable));
        }

    private:
        RegisterFunction register_{};
    };

    namespace Events
    {
        // Object events register with Core only when a user actually subscribes.
        // Core can use the first subscription as the signal to install the corresponding game hook.
        inline VoidEvent OnTick{nullptr};
        inline VoidEvent OnUpdate{nullptr};
        inline VoidEvent OnDraw{nullptr};
        inline WndProcEvent OnWndProc{};
        inline CreateObjectEvent OnCreateObject{nullptr};
        inline DeleteObjectEvent OnDeleteObject{nullptr};
        inline IssueOrderEventSource OnIssueOrder{nullptr};
        inline NewPathEventSource OnNewPath{nullptr};
        inline ProcessSpellEventSource OnProcessSpell{nullptr};
        inline DoCastEventSource OnDoCast{nullptr};
        inline PlayAnimationEventSource OnPlayAnimation{nullptr};
        inline StopCastEventSource OnStopCast{nullptr};
        inline CastSpellEventSource OnCastSpell{nullptr};
        inline UpdateChargeableSpellEventSource OnUpdateChargeableSpell{nullptr};
        inline BuffAddRemoveEventSource OnBuffAddRemove{nullptr};
        inline UpdateBuffEventSource OnUpdateBuff{nullptr};
        inline SpellImpactEventSource OnSpellImpact{nullptr};
        inline VoidEvent OnGameUpdate{nullptr};

        inline void BindApi(const RSDK_Api* const api)
        {
            // Assign function pointers after Core passes the API table to the plugin.
            OnTick = VoidEvent{
                api != nullptr && api->events != nullptr ? api->events->RegisterOnTick : nullptr};
            OnUpdate = VoidEvent{
                api != nullptr && api->events != nullptr ? api->events->RegisterOnUpdate : nullptr};
            OnDraw = VoidEvent{
                api != nullptr && api->events != nullptr ? api->events->RegisterOnDraw : nullptr};
            OnCreateObject = CreateObjectEvent{
                api != nullptr && api->events != nullptr ? api->events->RegisterOnCreateObject : nullptr};
            OnDeleteObject = DeleteObjectEvent{
                api != nullptr && api->events != nullptr ? api->events->RegisterOnDeleteObject : nullptr};
            OnIssueOrder = IssueOrderEventSource{
                api != nullptr && api->events != nullptr ? api->events->RegisterOnIssueOrder : nullptr};
            OnNewPath = NewPathEventSource{
                api != nullptr && api->events != nullptr ? api->events->RegisterOnNewPath : nullptr};
            OnProcessSpell = ProcessSpellEventSource{
                api != nullptr && api->events != nullptr ? api->events->RegisterOnProcessSpell : nullptr};
            OnDoCast = DoCastEventSource{
                api != nullptr && api->events != nullptr ? api->events->RegisterOnDoCast : nullptr};
            OnPlayAnimation = PlayAnimationEventSource{
                api != nullptr && api->events != nullptr ? api->events->RegisterOnPlayAnimation : nullptr};
            OnStopCast = StopCastEventSource{
                api != nullptr && api->events != nullptr ? api->events->RegisterOnStopCast : nullptr};
            OnCastSpell = CastSpellEventSource{
                api != nullptr && api->events != nullptr ? api->events->RegisterOnCastSpell : nullptr};
            OnUpdateChargeableSpell = UpdateChargeableSpellEventSource{
                api != nullptr && api->events != nullptr ? api->events->RegisterOnUpdateChargeableSpell : nullptr};
            OnBuffAddRemove = BuffAddRemoveEventSource{
                api != nullptr && api->events != nullptr ? api->events->RegisterOnBuffAddRemove : nullptr};
            OnUpdateBuff = UpdateBuffEventSource{
                api != nullptr && api->events != nullptr ? api->events->RegisterOnUpdateBuff : nullptr};
            OnSpellImpact = SpellImpactEventSource{
                api != nullptr && api->events != nullptr ? api->events->RegisterOnSpellImpact : nullptr};
            OnGameUpdate = VoidEvent{
                api != nullptr && api->events != nullptr ? api->events->RegisterOnGameUpdate : nullptr};
        }
    }

    namespace Delay
    {
        using TaskId = std::uint64_t;
        inline constexpr TaskId InvalidTask = 0u;

        namespace detail_delay
        {
            struct Task final
            {
                TaskId id{};
                std::chrono::steady_clock::time_point due{};
                std::chrono::milliseconds interval{};
                bool repeating{};
                bool cancelled{};
                std::function<void()> callback;
            };

            inline std::vector<Task>& Tasks()
            {
                static std::vector<Task> tasks;
                return tasks;
            }

            inline TaskId& NextId()
            {
                static TaskId id = 1u;
                return id;
            }

            inline bool& GameUpdateRegistered()
            {
                static bool registered = false;
                return registered;
            }

            inline void Update()
            {
                const auto now = std::chrono::steady_clock::now();
                auto& tasks = Tasks();

                for (std::size_t i = 0; i < tasks.size();)
                {
                    auto& task = tasks[i];
                    if (task.cancelled)
                    {
                        tasks.erase(tasks.begin() + static_cast<std::ptrdiff_t>(i));
                        continue;
                    }

                    if (task.due > now)
                    {
                        ++i;
                        continue;
                    }

                    auto callback = task.callback;
                    if (task.repeating && !task.cancelled)
                    {
                        task.due = now + task.interval;
                        ++i;
                    }
                    else
                    {
                        tasks.erase(tasks.begin() + static_cast<std::ptrdiff_t>(i));
                    }

                    if (callback)
                    {
                        try { callback(); } catch (...) { Log::Error("Exception in delayed callback."); }
                    }
                }
            }

            inline bool EnsureGameUpdate()
            {
                if (GameUpdateRegistered())
                {
                    return true;
                }

                GameUpdateRegistered() = Events::OnGameUpdate.Subscribe([]() { Update(); });
                return GameUpdateRegistered();
            }

            template <typename Callable>
            TaskId Add(const std::uint32_t delayMs, const bool repeating, Callable&& callable)
            {
                if (!EnsureGameUpdate())
                {
                    return InvalidTask;
                }

                const TaskId id = NextId()++;
                const auto interval = std::chrono::milliseconds(delayMs);
                Tasks().push_back(Task{
                    id,
                    std::chrono::steady_clock::now() + interval,
                    interval,
                    repeating,
                    false,
                    std::function<void()>(std::forward<Callable>(callable))});
                return id;
            }
        }

        template <typename Callable>
        TaskId After(const std::uint32_t delayMs, Callable&& callable)
        {
            return detail_delay::Add(delayMs, false, std::forward<Callable>(callable));
        }

        template <typename Callable>
        TaskId Every(const std::uint32_t intervalMs, Callable&& callable)
        {
            if (intervalMs == 0u)
            {
                return InvalidTask;
            }
            return detail_delay::Add(intervalMs, true, std::forward<Callable>(callable));
        }

        inline bool Cancel(const TaskId id)
        {
            for (auto& task : detail_delay::Tasks())
            {
                if (task.id == id)
                {
                    task.cancelled = true;
                    return true;
                }
            }
            return false;
        }

        inline void Clear() noexcept
        {
            detail_delay::Tasks().clear();
            detail_delay::GameUpdateRegistered() = false;
        }
    }

    namespace detail
    {
        template <typename TPlugin>
        RSDK_Bool LoadPlugin(
            const RSDK_Api* const api,
            RSDK_PluginState* const outputState)
        {
            if (outputState == nullptr || !Runtime::Instance().Initialize(api))
            {
                return RSDK_FALSE;
            }

            Events::BindApi(api);
            auto plugin = std::make_unique<TPlugin>();

            try
            {
                // OnLoad is where users create settings once and subscribe to required events.
                if (!plugin->OnLoad())
                {
                    Runtime::Instance().Shutdown();
                    return RSDK_FALSE;
                }
            }
            catch (...)
            {
                Runtime::Instance().Shutdown();
                return RSDK_FALSE;
            }

            *outputState = plugin.release();
            return RSDK_TRUE;
        }

        template <typename TPlugin>
        void UnloadPlugin(const RSDK_PluginState state)
        {
            std::unique_ptr<TPlugin> plugin(static_cast<TPlugin*>(state));
            if (plugin != nullptr)
            {
                try
                {
                    plugin->OnUnload();
                }
                catch (...)
                {
                }
            }

            // Users normally keep subscriptions for the lifetime of the plugin.
            // The framework unregisters everything here, so users do not need -= or Unhook calls.
            Delay::Clear();
            Runtime::Instance().Shutdown();
        }
    }
}

#ifndef Plugin_Name
#define Plugin_Name "Plugin_Name"
#endif

#ifndef Author_Name
#define Author_Name ""
#endif

#ifndef Plugin_Version
#define Plugin_Version "1.0.0"
#endif

#ifndef Plugin_Description
#define Plugin_Description "RSDK plugin"
#endif

#ifndef Plugin_Version_Number
#define Plugin_Version_Number RSDK_MAKE_VERSION(1, 0, 0)
#endif

#ifndef Plugin_Dependencies
#define Plugin_Dependencies nullptr
#endif

#ifndef Plugin_Dependency_Count
#define Plugin_Dependency_Count 0u
#endif

#ifdef Plugin_Id
#define RSDK_PLUGIN_PERSISTENT_ID Plugin_Id
#else
#define RSDK_PLUGIN_PERSISTENT_ID nullptr
#endif

// The declarations in RSDKPlugin.h and the definitions below must use the
// SAME RSDK_PLUGIN_EXPORT linkage to avoid MSVC C2375. When a plugin DLL
// does not define RSDK_PLUGIN_BUILD, request the 3 exports from the linker
// instead of adding __declspec(dllexport) only to their definitions.
// Windows x64 C ABI symbols are undecorated (the supported RSDK target).
#if defined(_WIN32) && !defined(RSDK_PLUGIN_BUILD) && !defined(RSDK_BUILD)
#define RSDK_PLUGIN_AUTO_EXPORTS \
    __pragma(comment(linker, "/EXPORT:RSDK_PluginQuery")) \
    __pragma(comment(linker, "/EXPORT:RSDK_PluginLoad")) \
    __pragma(comment(linker, "/EXPORT:RSDK_PluginUnload"))
#else
#define RSDK_PLUGIN_AUTO_EXPORTS
#endif

// Generates metadata and the three required plugin exports.
// Users only need to define a class derived from rsdk::Plugin and invoke this macro once.
#define RSDK_REGISTER_PLUGIN(PluginType) \
    RSDK_PLUGIN_AUTO_EXPORTS \
    namespace \
    { \
        const RSDK_PluginInfo g_rsdkPluginInfo \
        { \
            sizeof(RSDK_PluginInfo), \
            RSDK_PLUGIN_ABI_VERSION, \
            RSDK_ABI_VERSION, \
            Plugin_Name, \
            Author_Name, \
            Plugin_Version, \
            Plugin_Description, \
            Plugin_Version_Number, \
            Plugin_Dependencies, \
            Plugin_Dependency_Count, \
            RSDK_PLUGIN_PERSISTENT_ID \
        }; \
    } \
    RSDK_PLUGIN_EXPORT const RSDK_PluginInfo* RSDK_CALL RSDK_PluginQuery() \
    { \
        return &g_rsdkPluginInfo; \
    } \
    RSDK_PLUGIN_EXPORT RSDK_Bool RSDK_CALL RSDK_PluginLoad( \
        const RSDK_Api* api, RSDK_PluginState* outputState) \
    { \
        return ::rsdk::detail::LoadPlugin<PluginType>(api, outputState); \
    } \
    RSDK_PLUGIN_EXPORT void RSDK_CALL RSDK_PluginUnload(RSDK_PluginState state) \
    { \
        ::rsdk::detail::UnloadPlugin<PluginType>(state); \
    }
