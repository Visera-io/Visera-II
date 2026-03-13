# Runtime.Graphics.Scene.Camera (Visera.Runtime.Graphics.Scene.Camera)

Scene camera: left-handed coordinate system, view/projection matrices and parameters. Used for viewport rendering and culling.

## FCamera

Left-handed scene camera. Supports **perspective** and **orthographic** projection. View, Projection, and ViewProjection matrices are **lazily updated** via dirty flags when position/rotation or projection parameters change.

### Projection type

| Enum | Description |
|------|-------------|
| `EProjectionType::Perspective` | Perspective (FOV, aspect, near/far). |
| `EProjectionType::Orthographic` | Orthographic (width, height, near/far). |
| `EProjectionType::Default` | Same as Perspective. |

### Factory methods

- **`FCamera::MakePerspective(I_FOVY, I_AspectRatio, I_NearPlane, I_FarPlane)`**  
  Creates a perspective camera. `I_FOVY`: vertical FOV (degrees); `I_AspectRatio`: width/height; `I_NearPlane` / `I_FarPlane`: clip distances.

- **`FCamera::MakeOrthographic(I_Width, I_Height, I_NearPlane, I_FarPlane)`**  
  Creates an orthographic camera. `I_Width` / `I_Height`: viewport size; near/far: clip distances.

### Transform (position and rotation)

| API | Description |
|-----|-------------|
| `GetPosition()` / `SetPosition(I_Position)` | World-space position. SetPosition marks view (and view-proj) dirty. |
| `GetRotation()` / `SetRotation(I_Rotation)` | Orientation as quaternion. SetRotation marks view (and view-proj) dirty. |
| `SetEulerAngles(I_Yaw, I_Pitch, I_Roll)` | Set rotation from Euler angles (Yaw–Pitch–Roll; order Y×X×Z). Marks view dirty. |
| `GetEulerAngles()` | Returns Euler angles (X=Pitch, Y=Yaw, Z=Roll) extracted from current rotation; handles gimbal lock. |

### Projection parameters

| API | Description |
|-----|-------------|
| `GetProjectionType()` | Current projection type (perspective or orthographic). |
| `GetFOVY()` | Vertical FOV (perspective only). |
| `GetAspectRatio()` | Width/height. |
| `GetNearPlane()` / `GetFarPlane()` | Clip plane distances. |
| `GetOrthoWidth()` / `GetOrthoHeight()` | Orthographic viewport size (ortho only). |

### Setters that change projection

- **`SetPerspective(I_FOVY, I_AspectRatio, I_NearPlane, I_FarPlane)`**  
  Switch to perspective and set parameters. Marks projection (and view-proj) dirty.

- **`SetOrthographic(I_Width, I_Height, I_NearPlane, I_FarPlane)`**  
  Switch to orthographic and set parameters. Marks projection (and view-proj) dirty.

### Matrix access

View and projection matrices are recomputed only when dirty; getters return const references.

| API | Description |
|-----|-------------|
| `GetViewMatrix()` | View matrix (left-handed; Right/Up/Forward rows, translation in fourth column). |
| `GetProjectionMatrix()` | Projection matrix (perspective or ortho; left-handed NDC). |
| `GetViewProjectionMatrix()` | View × Projection; updated when either view or projection is dirty. |

## See also

- [Scene](index.md) — parent module
- [Light](Light.md) — lights
- [Renderable](Renderable.md) — renderables
