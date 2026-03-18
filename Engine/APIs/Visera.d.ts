/**
 * Visera scripting API: globals and callbacks visible to scripts (e.g. main.js).
 * Entry script is loaded once; the engine calls OnTick(dt) each frame.
 */

/** Sprite descriptor for visera.draw. Path is relative to resource directory. */
export interface ViseraSprite {
    x?: number;
    y?: number;
    width?: number;
    height?: number;
    /** Material path (e.g. "Assets/Material/Sprite.vmaterial"). Use material or materialPath. */
    material?: string;
    materialPath?: string;
}

export interface ViseraLog {
    info(message: string): void;
    warn(message: string): void;
    error(message: string): void;
}

export interface ViseraGlobal {
    log: ViseraLog;
    /** Submit a sprite draw for this frame. Same material path reuses cached material/PSO. */
    draw(sprite: ViseraSprite): void;
}

declare global {
    const visera: ViseraGlobal;

    /**
     * Called by the engine every frame with delta time in seconds.
     * Define this function in your entry script to drive per-frame logic.
     */
    function OnTick(dt: number): void;
}

export {};
