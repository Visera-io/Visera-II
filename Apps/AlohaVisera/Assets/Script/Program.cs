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

class Program
{
    static int Main()
    {
        for(int i = 0; i < 1; ++i)
        {
            Log.Info("C#", $"Hello World {i}!");
        }
        Visera.SetWindowTitle("Hello World");


        return 0;
    }
}
