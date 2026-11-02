#pragma once

#include "RSDKApi.h"

#include <algorithm>
#include <mutex>
#include <vector>

namespace rsdk::detail
{
    struct SubscriptionRecord final
    {
        RSDK_EventToken token{RSDK_INVALID_EVENT};
        void* context{};
        void (*destroyContext)(void*){};
        RSDK_Bool(RSDK_CALL* unregister)(RSDK_EventToken){};
    };

    class Runtime final
    {
    public:
        static Runtime& Instance() noexcept
        {
            static Runtime runtime;
            return runtime;
        }

        bool Initialize(const RSDK_Api* api) noexcept
        {
            if (api == nullptr || api->events == nullptr || api->log == nullptr || api->menu == nullptr)
                return false;
            api_ = api;
            return true;
        }

        [[nodiscard]] const RSDK_Api* Api() const noexcept
        {
            return api_;
        }

        bool Track(RSDK_EventToken token, void* context, void (*destroyContext)(void*))
        {
            if (token == RSDK_INVALID_EVENT)
            {
                if (destroyContext != nullptr)
                    destroyContext(context);
                return false;
            }
            {
                std::scoped_lock lock(subscriptionMutex_);
                subscriptions_.push_back({token, context, destroyContext, nullptr});
            }
            return true;
        }

        bool TrackCustom(RSDK_EventToken token, void* context, void (*destroyContext)(void*),
            RSDK_Bool(RSDK_CALL* unregister)(RSDK_EventToken))
        {
            if (token == RSDK_INVALID_EVENT)
            {
                if (destroyContext != nullptr)
                    destroyContext(context);
                return false;
            }
            {
                std::scoped_lock lock(subscriptionMutex_);
                subscriptions_.push_back({token, context, destroyContext, unregister});
            }
            return true;
        }

        bool Disconnect(const RSDK_EventToken token) noexcept
        {
            SubscriptionRecord record{};
            {
                std::scoped_lock lock(subscriptionMutex_);
                const auto it = std::find_if(subscriptions_.begin(), subscriptions_.end(),
                    [token](const SubscriptionRecord& candidate) { return candidate.token == token; });
                if (it == subscriptions_.end())
                    return false;
                record = *it;
                subscriptions_.erase(it);
            }

            if (record.token != RSDK_INVALID_EVENT)
            {
                if (record.unregister != nullptr)
                    record.unregister(record.token);
                else if (api_ != nullptr && api_->events != nullptr && api_->events->Unregister != nullptr)
                    api_->events->Unregister(record.token);
            }

            // Keep callable storage alive until framework shutdown. This is required for
            // self-disconnect, while Core guarantees normal disconnects wait until callbacks are idle.
            record.token = RSDK_INVALID_EVENT;
            record.unregister = nullptr;
            {
                std::scoped_lock lock(subscriptionMutex_);
                retiredSubscriptions_.push_back(record);
            }
            return true;
        }

        void ClearSubscriptions() noexcept
        {
            std::vector<SubscriptionRecord> active;
            std::vector<SubscriptionRecord> retired;
            {
                std::scoped_lock lock(subscriptionMutex_);
                active.swap(subscriptions_);
                retired.swap(retiredSubscriptions_);
            }

            for (auto it = active.rbegin(); it != active.rend(); ++it)
            {
                if (it->token != RSDK_INVALID_EVENT)
                {
                    if (it->unregister != nullptr)
                        it->unregister(it->token);
                    else if (api_ != nullptr && api_->events != nullptr && api_->events->Unregister != nullptr)
                        api_->events->Unregister(it->token);
                }
                if (it->destroyContext != nullptr)
                    it->destroyContext(it->context);
            }

            for (auto& record : retired)
            {
                if (record.destroyContext != nullptr)
                    record.destroyContext(record.context);
            }
        }

        void Shutdown() noexcept
        {
            ClearSubscriptions();
            api_ = nullptr;
        }

    private:
        const RSDK_Api* api_{};
        std::mutex subscriptionMutex_;
        std::vector<SubscriptionRecord> subscriptions_;
        std::vector<SubscriptionRecord> retiredSubscriptions_;
    };

    template <typename T>
    void DestroyContext(void* context)
    {
        delete static_cast<T*>(context);
    }
}

namespace rsdk
{
    class EventConnection final
    {
    public:
        EventConnection() = default;
        explicit EventConnection(const RSDK_EventToken token) noexcept : token_(token) {}

        EventConnection(const EventConnection&) = delete;
        EventConnection& operator=(const EventConnection&) = delete;

        EventConnection(EventConnection&& other) noexcept : token_(other.token_)
        {
            other.token_ = RSDK_INVALID_EVENT;
        }

        EventConnection& operator=(EventConnection&& other) noexcept
        {
            if (this != &other)
            {
                Disconnect();
                token_ = other.token_;
                other.token_ = RSDK_INVALID_EVENT;
            }
            return *this;
        }

        ~EventConnection()
        {
            Disconnect();
        }

        [[nodiscard]] bool IsConnected() const noexcept
        {
            return token_ != RSDK_INVALID_EVENT;
        }

        [[nodiscard]] explicit operator bool() const noexcept
        {
            return IsConnected();
        }

        bool Disconnect() noexcept
        {
            if (token_ == RSDK_INVALID_EVENT)
                return false;

            const RSDK_EventToken token = token_;
            token_ = RSDK_INVALID_EVENT;
            return detail::Runtime::Instance().Disconnect(token);
        }

        [[nodiscard]] RSDK_EventToken Token() const noexcept
        {
            return token_;
        }

    private:
        RSDK_EventToken token_{RSDK_INVALID_EVENT};
    };
}
