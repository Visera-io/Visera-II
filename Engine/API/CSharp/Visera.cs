using System.Runtime.InteropServices;

namespace Visera
{
    static class AssetHub
    {
        [DllImport("Visera", EntryPoint="LoadTexture", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool
        LoadTexture(string I_Path);
    }

    static class Keyboard
    {
        [DllImport("Visera", EntryPoint="IsPressed", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool
        IsPressed(int I_Key);
    }
}