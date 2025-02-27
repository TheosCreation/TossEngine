using System;
using TossEngine;

namespace Scripts
{
    // Ship class inherits from Component, part of TossEngine.
    public class Ship : Component
    {
        public Ship() : base("Ship")
        {
            Console.WriteLine("Created a ship Component!");
        }

        public void LogSomething()
        {
            Console.WriteLine("Log something from the Ship class!");
        }
    }
}