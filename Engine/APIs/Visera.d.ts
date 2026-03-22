/**
 * Visera scripting API: globals and callbacks visible to scripts (e.g. main.js).
 * Entry script is loaded once; the engine calls OnTick(dt) each frame.
 */

/** 2D position for visera.drawSprite; z maps to depth (MakeTransform2D I_Depth). */
export interface ViseraSpritePosition {
    x?: number;
    y?: number;
    z?: number;
}

/** Transform for visera.drawSprite; rotation is in degrees. */
export interface ViseraSpriteTransform {
    position?: ViseraSpritePosition;
    rotation?: number;
}

export interface ViseraSpriteExtent {
    width?: number;
    height?: number;
}

/**
 * Descriptor for visera.drawSprite. Material path is relative to the resource directory.
 * `frame` is written to instance CustomData.x for materials/shaders (e.g. sprite sheets).
 */
export interface ViseraDrawSpriteOptions {
    transform?: ViseraSpriteTransform;
    extent?: ViseraSpriteExtent;
    frame?: number;
    /** Use material or materialPath. */
    material?: string;
    materialPath?: string;
}

export interface ViseraLog {
    info(message: string): void;
    warn(message: string): void;
    error(message: string): void;
}

/** Single mapping: physical input -> action (camelCase keys; engine still accepts PascalCase in JSON at runtime). */
export interface ViseraInputMappingDescriptor {
    action?: string;
    source?: string;
    trigger?: string;
    key?: string;
    button?: string;
    modifiers?: string[];
}

export interface ViseraInputCursor {
    /** Snapshot of cursor position when read. */
    readonly position: { x: number; y: number };
    /** Snapshot of cursor offset when read. */
    readonly offset: { x: number; y: number };
}

export interface ViseraInputMouse {
    readonly cursor: ViseraInputCursor;
}

/** Passed to addAction callback when the mapped input fires. */
export interface ViseraInputActionInfo {
    name: string;
}

/** Main window options for visera.setMainWindow (OnInit); camelCase only in typings. */
export interface ViseraMainWindowOptions {
    title?: string;
    width?: number;
    height?: number;
    /** GLFW window hint GLFW_RESIZABLE; default true. */
    resizable?: boolean;
    /** Center on the primary monitor work area (glfwGetMonitorWorkarea); default false. Ignored when fullscreen is true. */
    center?: boolean;
    /** Exclusive fullscreen on the primary monitor at current video mode resolution; default false. */
    fullscreen?: boolean;
}

export interface ViseraInput {
    addMapping(descriptor: ViseraInputMappingDescriptor): void;
    /** One callback per action; later calls replace the previous. */
    addAction(name: string, callback: (info: ViseraInputActionInfo) => void): void;
    removeMappings(actionName: string): void;
    isActionActive(actionName: string): boolean;
    readonly mouse: ViseraInputMouse;
}

export interface ViseraGlobal {
    log: ViseraLog;
    /** Submit a sprite draw for this frame. Same material path reuses cached material/PSO. */
    drawSprite(descriptor: ViseraDrawSpriteOptions): void;
    /**
     * Configure the main window before the engine shows it. Only when the game did not pass FEngineCreateInfo.MainWindow;
     * call from OnInit(). Omitted fields keep FWindowCreateInfo defaults (title/size).
     */
    setMainWindow?(options: ViseraMainWindowOptions): void;
    /** Present when Input and Scripting are both enabled. */
    input?: ViseraInput;
}

declare global {
    const visera: ViseraGlobal;

    /**
     * Called once after the entry script loads, before the main window is created (when the window comes from script path).
     */
    function OnInit(): void;

    /**
     * Called by the engine every frame with delta time in seconds.
     * Define this function in your entry script to drive per-frame logic.
     */
    function OnTick(dt: number): void;
}

export {};
