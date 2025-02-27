#pragma once
#include "Utils.h"

class Resizable;
class Window;

class TOSSENGINE_API TossEngine
{
public:
    static TossEngine& GetInstance()
    {
        static TossEngine instance;
        return instance;
    }

    // Delete the copy constructor and assignment operator to prevent copying
    TossEngine(const TossEngine& other) = delete;
    TossEngine& operator=(const TossEngine& other) = delete;

    void Init();
    void CreateWindowOrChangeOwner(Resizable* owner, Vector2 size, const string& windowName);
    Window* GetWindow();
    void PollEvents();
    void CleanUp();
    float GetCurrentTime();

private:
    bool isInitilized = false;
    std::unique_ptr<Window> m_window;

    /**
     * @brief Private constructor to prevent external instantiation.
     */
    TossEngine() = default;

    /**
     * @brief Private destructor to prevent external deletion.
     */
    ~TossEngine() = default;
};

