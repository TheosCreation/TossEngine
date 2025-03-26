#pragma once
#include "Utils.h"
#include <coroutine>
#include <optional>

class CoroutineTask {
public:
    struct promise_type {
        std::function<void()> onComplete = nullptr;
        std::shared_ptr<bool> completionFired = std::make_shared<bool>(false);

        CoroutineTask get_return_object() {
            return CoroutineTask{ std::coroutine_handle<promise_type>::from_promise(*this) };
        }

        std::suspend_never initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };

    std::coroutine_handle<promise_type> handle;

    CoroutineTask(std::coroutine_handle<promise_type> h) : handle(h) {}

    CoroutineTask(const CoroutineTask&) = delete;
    CoroutineTask& operator=(const CoroutineTask&) = delete;

    CoroutineTask(CoroutineTask&& other) noexcept : handle(other.handle) {
        other.handle = nullptr;
    }

    CoroutineTask& operator=(CoroutineTask&& other) noexcept {
        if (this != &other) {
            if (handle)
                handle.destroy();
            handle = other.handle;
            other.handle = nullptr;
        }
        return *this;
    }

    ~CoroutineTask() {
        if (!handle) {
            return;
        }

        auto& promise = handle.promise();
        if (promise.onComplete && !*promise.completionFired) {
            *promise.completionFired = true;
            promise.onComplete();
        }

        handle.destroy();
    }

    void SetOnComplete(std::function<void()> callback) {
        if (handle)
            handle.promise().onComplete = std::move(callback);
    }

    bool Resume() {
        if (!handle.done())
            handle.resume();
        return !handle.done();
    }

    bool Done() const {
        return !handle || handle.done();
    }
};
