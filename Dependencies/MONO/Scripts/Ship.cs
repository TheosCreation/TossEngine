using System;
using TossEngine;

namespace Scripts
{
    // Ship class inherits from Component, part of TossEngine.
    public class Ship : Component
    {
        public override void OnCreate()
        {
            Console.WriteLine("Ship OnCreate - Initialization done.");
        }

        public override void OnUpdate(float deltaTime)
        {
            Console.WriteLine($"Ship OnUpdate - DeltaTime: {deltaTime}");
        }

        public override void OnFixedUpdate(float fixedDeltaTime)
        {
            Console.WriteLine($"Ship OnFixedUpdate - FixedDeltaTime: {fixedDeltaTime}");
        }

        public override void OnDestroy()
        {
            Console.WriteLine("Ship OnDestroy - Cleanup done.");
        }

        public void LogSomething()
        {
            Console.WriteLine("Log something from the Ship class!");
        }
    }
}