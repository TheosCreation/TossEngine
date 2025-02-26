// In a C# script file
using System.Runtime.InteropServices;
using System;

namespace TossEngine
{
    // GameObject class
    public class GameObject
    {
        private IntPtr m_nativePtr; // Pointer to the C++ GameObject

        public GameObject()
        {
            //m_nativePtr = GameObject_Create();
        }
        
        public void AddComponent(Component component)
        {
            //GameObject_AddComponent(m_nativePtr, component.NativePtr);
        }

        //[DllImport("TossEngine.dll")]
        //private static extern IntPtr GameObject_Create();
        //
        //[DllImport("TossEngine.dll")]
        //private static extern void GameObject_AddComponent(IntPtr gameObject, IntPtr component);
    }

    // Base Component class
    public class Component
    {
        public IntPtr NativePtr { get; private set; }

        public Component()
        {
            NativePtr = CreateComponent();
        }

        public void Destroy()
        {
            if (NativePtr != IntPtr.Zero)
            {
                DestroyComponent(NativePtr);
                NativePtr = IntPtr.Zero;
            }
        }

        // Override these methods in C# to implement specific behavior
        public virtual void OnCreate()
        {
            Console.WriteLine("C# Component Created");
        }

        public virtual void OnUpdate(float deltaTime)
        {
            Console.WriteLine("C# Component Updated, DeltaTime: " + deltaTime);
        }

        public virtual void OnFixedUpdate(float fixedDeltaTime)
        {
            Console.WriteLine("C# Component FixedUpdate, FixedDeltaTime: " + fixedDeltaTime);
        }

        public virtual void OnDestroy()
        {
            Console.WriteLine("C# Component Destroyed");
        }

        [DllImport("TossEngine.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr CreateComponent();

        [DllImport("TossEngine.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void DestroyComponent(IntPtr component);

        // Add function pointers to hook C# methods (to be invoked by C++)
        [DllImport("TossEngine.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetOnCreateCallback(IntPtr component, OnCreateCallback callback);

        [DllImport("TossEngine.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetOnUpdateCallback(IntPtr component, OnUpdateCallback callback);

        [DllImport("TossEngine.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetOnFixedUpdateCallback(IntPtr component, OnFixedUpdateCallback callback);

        [DllImport("TossEngine.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetOnDestroyCallback(IntPtr component, OnDestroyCallback callback);

        // Define the delegates for the callbacks
        public delegate void OnCreateCallback();
        public delegate void OnUpdateCallback(float deltaTime);
        public delegate void OnFixedUpdateCallback(float fixedDeltaTime);
        public delegate void OnDestroyCallback();
    }
}
