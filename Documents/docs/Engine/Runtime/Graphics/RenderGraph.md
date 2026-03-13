# Runtime.Graphics.RenderGraph (Visera.Runtime.Graphics.RenderGraph)

Render graph: declarative passes and resource dependencies. Drives frame rendering. Pass execution receives a **pass context** (command list and **render list**); compile produces an executable that runs against `FRenderContext`.

## FRDGPassContext

Per-pass execution context passed to each pass’s execute lambda.

| Member       | Type                  | Description |
|--------------|-----------------------|-------------|
| `CommandList` | `FRHICommandList&`    | Command list for this frame. |
| `RenderList`  | `const FRenderList&`  | Sorted batches (opaque/transparent); draw passes iterate this. |

## FRenderGraph

- **`AddPass(I_Name, I_Setup, I_Execute)`** — Adds a pass. `I_Name` is `FName` (e.g. `EName::PresentTransition`). Setup lambda configures reads/writes; execute lambda receives `FRDGPassContext` (command list + render list).
- **`Compile(I_RHI)`** — Builds barriers and order; returns an executable.
- **`Execute(I_RenderContext)`** — Creates a command list from `I_RenderContext->RHI`, runs all passes with `FRenderContext` (including `RenderList`), then **submits** the command list. Requires `I_RenderContext->RenderList` to be non-null.

Swap chain is identified by `FRHISwapChainID` (no separate type alias).

## CullDeadPasses

Dead pass culling: removes passes whose outputs are never consumed by any live pass. Algorithm is **O(N+E) reverse BFS** (replacing the previous O(N³) fixed-point iteration):

1. Build a *ResourceWriters* map (resource → node indices that write it).
2. Seed the alive set with nodes that write to external resources (back buffer, cached textures).
3. BFS backwards: for each alive node’s reads, mark all writers of that resource alive. Passes not reached are removed.

## See also

- [Graphics](index.md) — parent module
- [Framework](Framework.md) — FRenderContext and FRenderList
- [Scene](Scene/index.md) — scene data
- [RHI](../RHI/index.md) — command list and resources
