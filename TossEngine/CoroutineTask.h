/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : CoroutineTask.h
Description : Lightweight coroutine wrapper for managing asynchronous tasks in TossEngine.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

#include "Utils.h"
#include <coroutine>
#include <optional>

/**
 * @class CoroutineTask
 * @brief A simple coroutine wrapper for managing and resuming asynchronous tasks.
 *        Supports manual resumption, completion callbacks, and move semantics.
 */
class CoroutineTask {
public:

    /**
     * @struct promise_type
     * @brief Manages the state and behavior of a CoroutineTask coroutine.
     */
    struct promise_type {
        std::function<void()> onComplete = nullptr;         //!< Callback to be invoked upon task completion.
        std::shared_ptr<bool> completionFired = std::make_shared<bool>(false); //!< Tracks whether the onComplete callback has already been called.

        /**
         * @brief Called when the coroutine is created.
         * @return A CoroutineTask linked to this promise.
         */
        CoroutineTask get_return_object() {
            return CoroutineTask{ std::coroutine_handle<promise_type>::from_promise(*this) };
        }

        /**
         * @brief Called immediately upon coroutine creation.
         * @return Always resumes execution immediately (no suspension at start).
         */
        std::suspend_never initial_suspend() { return {}; }

        /**
         * @brief Called at the end of the coroutine.
         * @return Suspends at the end, allowing manual cleanup.
         */
        std::suspend_always final_suspend() noexcept { return {}; }

        /**
         * @brief Called when the coroutine finishes normally.
         */
        void return_void() {}

        /**
         * @brief Called if an exception is thrown inside the coroutine.
         */
        void unhandled_exception() {}
    };

    /**
     * @brief Handle to the underlying coroutine.
     */
    std::coroutine_handle<promise_type> handle;

    /**
     * @brief Constructor from coroutine handle.
     * @param h The coroutine handle.
     */
    CoroutineTask(std::coroutine_handle<promise_type> h) : handle(h) {}

    // Deleted copy constructor and assignment (coroutines cannot be copied)
    CoroutineTask(const CoroutineTask&) = delete;
    CoroutineTask& operator=(const CoroutineTask&) = delete;

    /**
     * @brief Move constructor.
     * @param other The task to move.
     */
    CoroutineTask(CoroutineTask&& other) noexcept : handle(other.handle) {
        other.handle = nullptr;
    }

    /**
     * @brief Move assignment operator.
     * @param other The task to move.
     * @return Reference to this.
     */
    CoroutineTask& operator=(CoroutineTask&& other) noexcept {
        if (this != &other) {
            if (handle)
                handle.destroy();
            handle = other.handle;
            other.handle = nullptr;
        }
        return *this;
    }

    /**
     * @brief Destructor. Destroys the coroutine handle and calls onComplete if not already fired.
     */
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

    /**
     * @brief Sets a callback to be called when the coroutine completes.
     * @param callback Function to call upon completion.
     */
    void SetOnComplete(std::function<void()> callback) {
        if (handle)
            handle.promise().onComplete = std::move(callback);
    }

    /**
     * @brief Resumes execution of the coroutine if it is not already complete.
     * @return True if coroutine is still running after resume, false if finished.
     */
    bool Resume() {
        if (!handle.done())
            handle.resume();
        return !handle.done();
    }

    /**
     * @brief Checks if the coroutine has completed.
     * @return True if done or handle is invalid.
     */
    bool Done() const {
        return !handle || handle.done();
    }
};
