using System.Runtime.InteropServices;

namespace Visera
{
    static class Hash
    {
        [DllImport("Visera-Engine", CallingConvention = CallingConvention.Cdecl)]
        public static extern ulong
        CityHash64(string I_Data);
    }
}