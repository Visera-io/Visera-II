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
    Tick(IntPtr Data, int Size)
    {
//         bool bInitGVS = false;
//         if(!bInitGVS)
//         {
//             Log.Info("GVS", "initing GVS");
//             var GVS = new GVSController();
//             GVS.Start();
//             Log.Fatal("GVS", "Testing");
//         }
        float DeltaTime = Marshal.PtrToStructure<float>(Data);
        if(DeltaTime > 1.0)
        {
            Log.Warn("C#", $"Triggered a slow loop (time: {DeltaTime}).");
        }
//         if(Keyboard.IsPressed(32))
//         {
//             Log.Info("C#", "Pressed Space.");
//         }
        //Log.Info("C#", $"DeltaTime: {DeltaTime}s");
//
//         if(!AssetHub.LoadShader("shader.slang", "vertMain"))
//         {
//             Log.Warn("C#", "Failed to load the shader!");
//         }
//         else Log.Debug("C#", "Loaded the shader.");
//
//         if(!AssetHub.LoadTexture("Firework.png"))
//         {
//             Log.Warn("C#", "Failed to load the Firework.png!");
//         }

        return 0;
    }
}