#pragma once
#include "All.h"

class ScriptLoader;

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
    void CompileScriptsProject();
    void LoadGenericResources();
    void TryCreateWindow(Resizable* owner, Vector2 size, const string& windowName, bool maximized = false);
    Window* GetWindow();
    void PollEvents();
    void CleanUp();
    float GetTime();
    string openFileDialog(const std::string& filter);

    bool IsDebugMode();
    void SetDebugMode(bool enabled);
    ScriptLoader* getScriptLoader();

private:
    bool isInitilized = false;
    bool isDebugMode = false;
    std::unique_ptr<Window> m_window = nullptr;
    ScriptLoader* m_scriptLoader = nullptr;

    /**
     * @brief Private constructor to prevent external instantiation.
     */
    TossEngine() = default;

    /**
     * @brief Private destructor to prevent external deletion.
     */
    ~TossEngine() = default;
};

