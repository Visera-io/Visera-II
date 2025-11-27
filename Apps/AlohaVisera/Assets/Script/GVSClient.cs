using System.Collections;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Text;

public class GVSController
{
    public string ipAddress = "192.168.4.1"; // ESP32 默认 AP IP
    public int port = 5555;

    public float maxVoltage = 4.0f;  // 对应屏幕 Y 的顶部
    public float duration = 5.0f;    // X轴代表的总时长（秒）
    //[Header("组件引用")]
    //public LineRenderer lineRenderer;
    //public Transform scanCursor; // 一个用来显示当前播放进度的小图标（可选）
    // 内部变量
    private UdpClient udpClient;
    //private List<Vector3> points = new List<Vector3>();
    //private bool isSending = false;
    //private Camera mainCam;
    // 画布的边界（根据摄像机 Size=5 计算）
    // 假设屏幕左边是 X=0 (时间0)，右边是 X=10 (时间 duration)
    // 屏幕底部是 Y=-4，顶部是 Y=4
    private float xStart = -8f; // 在世界坐标中画图的左边界
    private float xEnd = 8f;    // 在世界坐标中画图的右边界
    private float yMax = 4f;    // 对应 +4V
    public void Start()
    {
        //mainCam = Camera.main;
        Log.Info("GVS", "Starting UDP Client...");
        udpClient = new UdpClient();
//         if(udpClient.Connect(ipAddress, port) != 0)
//         {
//           Log.Error("GVS", $"Failed to connect to ip:{ipAddress} prot:{port}");
//         }
        // 初始化 LineRenderer
        //lineRenderer.positionCount = 0;
        //lineRenderer.useWorldSpace = true;
    }
    void Update()
    {
//         // 1. 鼠标绘制逻辑 (只在未发送时允许绘制)
//         if (!isSending && Input.GetMouseButton(0))
//         {
//             DrawWaveform();
//         }
    }
    // 绘制功能
    void DrawWaveform()
    {
//         Vector3 mousePos = mainCam.ScreenToWorldPoint(Input.mousePosition);
//         mousePos.z = 0;
//         // 限制 Y 轴范围 (电压范围)
//         mousePos.y = Mathf.Clamp(mousePos.y, -yMax, yMax);
//         // 限制 X 轴范围 (画布范围)
//         mousePos.x = Mathf.Clamp(mousePos.x, xStart, xEnd);
//         // 简单的去重，避免同一个位置存太多点
//         if (points.Count > 0 && Vector3.Distance(points[points.Count - 1], mousePos) < 0.1f)
//             return;
//         // 只有当 X 向右移动时才记录（保证时间单向流逝，防止波形回退）
//         if (points.Count > 0 && mousePos.x <= points[points.Count - 1].x)
//             return;
//         points.Add(mousePos);
//         lineRenderer.positionCount = points.Count;
//         lineRenderer.SetPositions(points.ToArray());
    }
    // 绑定到 "Clear" 按钮
    public void ClearWaveform()
    {
//         points.Clear();
//         lineRenderer.positionCount = 0;
//         StopAllCoroutines();
//         isSending = false;
//         SendUdp("w 0"); // 归零
    }
    // 绑定到 "Send" 按钮
    public void StartSending()
    {
//         if (points.Count < 2) return; // 没画东西
//         if (!isSending)
//         {
//             StartCoroutine(SendWaveformRoutine());
//         }
    }
    // 核心发送协程
    void SendWaveformRoutine()
    {
//         isSending = true;
//         float timer = 0f;
//         // 计算 X 轴长度
//         float xTotalLength = xEnd - xStart;
//         while (timer < duration)
//         {
//             timer += Time.deltaTime;
//             // 计算当前时间进度 (0.0 ~ 1.0)
//             float progress = timer / duration;
//             // 计算当前对应的 X 坐标
//             float currentX = xStart + (progress * xTotalLength);
//             // 可视化：移动光标 (如果有的话)
//             if (scanCursor != null)
//                 scanCursor.position = new Vector3(currentX, 0, 0);
//             // *** 核心算法：获取当前 X 对应的 Y (电压) ***
//             float voltage = GetVoltageAtX(currentX);
//             // 发送 UDP
//             // 格式: "w 1.23"
//             string packet = $"w {voltage:F2}";
//             SendUdp(packet);
//             // 等待一帧
//             yield return null;
//         }
//         // 结束
//         SendUdp("w 0"); // 结束后安全归零
//         isSending = false;
//         Debug.Log("Waveform Finished");
    }

// 根据 X 坐标，在线段上插值找到 Y
    float GetVoltageAtX(float xPos)
    {
//         // 如果超出范围
//         if (points.Count == 0) return 0;
//         if (xPos <= points[0].x) return points[0].y;
//         if (xPos >= points[points.Count - 1].x) return points[points.Count - 1].y;
//         // 查找 xPos 在哪两个点之间
//         // 这里用简单遍历，点多可以用二分法优化，但几百个点遍历也很快
//         for (int i = 0; i < points.Count - 1; i++)
//         {
//             Vector3 p1 = points[i];
//             Vector3 p2 = points[i + 1];
//             if (xPos >= p1.x && xPos <= p2.x)
//             {
//                 // 线性插值 (Linear Interpolation)
//                 float t = (xPos - p1.x) / (p2.x - p1.x);
//                 float y = Mathf.Lerp(p1.y, p2.y, t);
//                 return y; // 直接返回 Y 坐标作为电压 (因为我们设定Y范围就是-4到4)
//             }
//         }
        return 0;
    }
    void SendUdp(string message)
    {
        try
        {
            byte[] data = Encoding.ASCII.GetBytes(message);
            udpClient.Send(data, data.Length, ipAddress, port);
        }
        catch (System.Exception e)
        {
            //Debug.LogError("UDP Error: " + e.Message);
        }
    }
    void OnApplicationQuit()
    {
        SendUdp("w 0");
        udpClient.Close();
    }
}