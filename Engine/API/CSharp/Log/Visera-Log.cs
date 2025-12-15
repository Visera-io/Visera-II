using System.Runtime.InteropServices;

static class Log
{
    public enum ELevel : byte
    { Trace = 0, Debug = 1, Info = 2, Warn = 3, Error = 4, Fatal = 5 }

    [DllImport("Visera", EntryPoint="Print", CallingConvention = CallingConvention.Cdecl)]
    public static extern void
    Print(ELevel I_Level, string I_Module, string I_Message);

    public static void
    Trace(string I_Module, string I_Message) => Print(ELevel.Trace, I_Module, I_Message);
    public static void
    Debug(string I_Module, string I_Message) => Print(ELevel.Debug, I_Module, I_Message);
    public static void
    Info(string I_Module, string I_Message)  => Print(ELevel.Info,  I_Module, I_Message);
    public static void
    Warn(string I_Module, string I_Message)  => Print(ELevel.Warn,  I_Module, I_Message);
    public static void
    Error(string I_Module, string I_Message) => Print(ELevel.Error, I_Module, I_Message);
    public static void
    Fatal(string I_Module, string I_Message) => Print(ELevel.Fatal, I_Module, I_Message);
}