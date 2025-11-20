using System.Runtime.InteropServices;

namespace Visera
{
    static class Hash
    {
        [DllImport("Visera-Engine", CallingConvention = CallingConvention.Cdecl)]
        public static extern ulong
        CityHash64(string I_Data);
    }

    static class AssetHub
    {
        [DllImport("Visera-Engine", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool
        LoadShader(string I_Path, string I_EntryPoint);

        [DllImport("Visera-Engine", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool
        LoadTexture(string I_Path);
    }

    static class Keyboard
    {
        [DllImport("Visera-Engine", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool
        IsPressed(int I_Key);
    }
}