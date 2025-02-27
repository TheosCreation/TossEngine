using System;
using System.Runtime.InteropServices;

namespace TossEngine
{
    public class TossEngine
    {
        private const string DLL_NAME = "TossEngine.dll";

        // Define delegate types for C++ function pointers
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void OnCreateCallback();
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void OnUpdateCallback(float deltaTime);
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void OnFixedUpdateCallback(float deltaTime);
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void OnDestroyCallback();

        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetCSharpComponentCallbacks(IntPtr component,
            OnCreateCallback onCreate,
            OnUpdateCallback onUpdate,
            OnFixedUpdateCallback onFixedUpdate,
            OnDestroyCallback onDestroy);
    }

    public class Component
    {
        public IntPtr nativeComponent;

        // Store delegate instances to prevent GC collection
        private static readonly TossEngine.OnCreateCallback _onCreate = OnCreate;
        private static readonly TossEngine.OnUpdateCallback _onUpdate = OnUpdate;
        private static readonly TossEngine.OnFixedUpdateCallback _onFixedUpdate = OnFixedUpdate;
        private static readonly TossEngine.OnDestroyCallback _onDestroy = OnDestroy;

        public Component(string typeName)
        {
            TossEngine.SetCSharpComponentCallbacks(nativeComponent, _onCreate, _onUpdate, _onFixedUpdate, _onDestroy);
        }

        private static void OnCreate() { Console.WriteLine("C# Component Created!"); }
        private static void OnUpdate(float dt) { Console.WriteLine($"C# Update {dt}"); }
        private static void OnFixedUpdate(float dt) { Console.WriteLine($"C# FixedUpdate {dt}"); }
        private static void OnDestroy() { Console.WriteLine("C# Component Destroyed!"); }
    }
}