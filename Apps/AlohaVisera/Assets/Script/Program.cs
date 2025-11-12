using System;
using System.Runtime.InteropServices;

using ViseraApp;

internal static class Visera
{

    [DllImport("Visera-Engine", CallingConvention = CallingConvention.Cdecl)]
    internal static extern void
    SetWindowTitle(string I_Title);
}
internal static class Log
{
    internal enum ELevel : byte
    { Trace = 0, Debug = 1, Info = 2, Warn = 3, Error = 4, Fatal = 5 }

    [DllImport("Visera-Engine", CallingConvention = CallingConvention.Cdecl)]
    private static extern void
    Print(ELevel I_Level, string I_Module, string I_Message);

    internal static void
    Trace(string I_Module, string I_Message) => Print(ELevel.Trace, I_Module, I_Message);
    internal static void
    Debug(string I_Module, string I_Message) => Print(ELevel.Debug, I_Module, I_Message);
    internal static void
    Info(string I_Module, string I_Message)  => Print(ELevel.Info,  I_Module, I_Message);
    internal static void
    Warn(string I_Module, string I_Message)  => Print(ELevel.Warn,  I_Module, I_Message);
    internal static void
    Error(string I_Module, string I_Message) => Print(ELevel.Error, I_Module, I_Message);
    internal static void
    Fatal(string I_Module, string I_Message) => Print(ELevel.Fatal, I_Module, I_Message);
}

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
}