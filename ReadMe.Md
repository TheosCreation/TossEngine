# Toss Editor
TossEngine is a custom-built C++ game engine with an integrated Editor featuring real-time scene editing, live component reloading, prefab creation, and asset management.

Built for fast iteration and modular design, TossEditor provides a fully functioning play mode, asset manager, component system, and C++ script hot-reloading at runtime.

# Features
* Real-time Scene Editing (move, rotate, scale objects with ImGuizmo)
* Prefab System (drag and drop GameObjects into Assets panel)
* Live C++ Script Reloading (Ctrl+R to rebuild and reload C++Scripts/)
* Inspector Window (edit component properties at runtime)
* Hierarchy Window (parent/child relationships, drag-and-drop parenting)
* Assets Window (manage all engine resources: textures, models, sounds, etc.)
* Scene and Game Viewports (with dedicated framebuffers)
* Physics Integration (ReactPhysics3D, gravity, collision layers)
* Audio Engine (FMOD or similar integration)
* Custom Layer System (configure collision layers)
* Settings and Build Panel (control player settings, gravity, rendering path)
* Window and Resolution Management
* Fully Dockable UI (via ImGui DockSpace)

# Windows
* Hierarchy	Shows all GameObjects in the scene. Supports drag/drop parenting.
* Inspector	Modify selected GameObject components or resource properties.
* Assets	View all engine-managed resources (materials, prefabs, textures).
* Scene View	Edit the scene using translation, rotation, and scaling gizmos.
* Game View	View your game as the player would.
* Settings & Build	Edit project settings and build the player project.
* Console	Debug output with logs, warnings, errors.

# Quick Guide
1. Launch TossEditor
    * It will automatically open your last edited scene or load Scenes/Scene.json.

2. Create and Edit Scenes
   * Hierarchy Window: Right-click to add empty GameObjects.
   * Inspector Window: Add components like Camera, Rigidbody, MeshRenderer, etc.
   * Scene View: Use T (Translate), R (Rotate), S (Scale) to manipulate objects.
   * Assets Window: Drag and drop Prefabs or Resources.

3. Save and Play
   * Save Scene: Ctrl + S
   * Play Game: Click Play button at the top.
   * Stop Game: Click Stop button to return to editing.

4. Add New Scripts (C++ Hot Reloading)
Add a new .cpp and .h file into the C++Scripts/ project in the sln file.

Example custom component:
```c++ 
    #include <TossEngine.h>

    class ExampleComponent : public Component {
    public:
        void OnInspectorGUI() override
        {
            FloatSliderField("Float", m_float);
            ImGui::Checkbox("Bool", m_bool);
        }
        void onCreate() override 
        {
            Debug::Log("ExampleComponent created!");
        }
    
        void onUpdate() override 
        {
        }
        
        //this is how to serialize member variables
        private:
        float m_float = 1.0f; //it will default to the value assigned here if not serialized before
        bool m_bool = true; 
        SERIALIZABLE_MEMBERS(m_float, m_bool)
    };
    REGISTER_COMPONENT(ExampleComponent)
```

* Press Ctrl + R inside TossEditor:
    * It recompiles C++Scripts/
    * Hot-reloads your new components live without restarting the editor!

# Shortcuts
* Ctrl + N	Create new scene
* Ctrl + O	Open existing scene
* Ctrl + S	Save current scene
* Ctrl + R	Reload scripts (recompile C++Scripts live)
* Play Button	Start playing game inside editor
* Stop Button	Stop playing and return to edit mode

# Future Work
* Full undo/Redo system
* Allowing Children to save to prefabs
* Window for prefab editing
* More ui and ux features
* Allow serialization references to components
* Animations
* Materials
* Beter asset importing and automatic asset detection with .meta files