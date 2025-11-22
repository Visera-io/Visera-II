using System.Runtime.InteropServices;

namespace Visera
{
    static class Hash
    {
        [DllImport("Visera-Engine", EntryPoint="CityHash64", CallingConvention = CallingConvention.Cdecl)]
        public static extern ulong
        CityHash64(string I_Data);
    }

    static class AssetHub
    {
        [DllImport("Visera-Engine", EntryPoint="LoadShader", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool
        LoadShader(string I_Path, string I_EntryPoint);

        [DllImport("Visera-Engine", EntryPoint="LoadTexture", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool
        LoadTexture(string I_Path);
    }

    static class Keyboard
    {
        [DllImport("Visera-Engine", EntryPoint="IsPressed", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool
        IsPressed(int I_Key);
    }
}