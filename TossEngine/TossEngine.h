#pragma once
#include "All.h"

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
    void LoadScripts();
    void LoadGenericResources();
    void TryCreateWindow(Resizable* owner, Vector2 size, const string& windowName);
    Window* GetWindow();
    void PollEvents();
    void CleanUp();
    float GetTime();
    string openFileDialog(const std::string& filter);

private:
    bool isInitilized = false;
    std::unique_ptr<Window> m_window = nullptr;

    /**
     * @brief Private constructor to prevent external instantiation.
     */
    TossEngine() = default;

    /**
     * @brief Private destructor to prevent external deletion.
     */
    ~TossEngine() = default;
};

