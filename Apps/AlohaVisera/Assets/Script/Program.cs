using System;
using System.Runtime.InteropServices;

using Visera;

public static class App
{
    public static int
    HelloWorld(IntPtr Data, int Size) // https://github.com/dotnet/docs/issues/31796?utm_source=chatgpt.com
    {
        string Author = RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
                        ? Marshal.PtrToStringUni(Data)
                        : Marshal.PtrToStringUTF8(Data);

        Log.Info("C#", $"Hello World! - {Author}");
        return 0;
    }

    public static int
    Main(IntPtr Data, int Size)
    {
        Log.Info("C#", "Running Visera App...");
        var Num = 10;
        Log.Info("C#", "Windows" + 1 + 1);
        Log.Info("C#", $"{Hash.CityHash64("Hello World!")}");

        return 0;
    }
}