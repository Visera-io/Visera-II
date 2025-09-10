- [ ] DLLs -> Target's Files :[modify]: $<TARGET_FILE_DIR>:${VISERA_APP} -> $<TARGET_FILE_DIR>:${in_target}
- [ ] https://graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2

Phase 1 Complete When:
✅ All subsystem interfaces defined
✅ VulkanGraphicsSubsystem fully functional
✅ GLFW window creation works
✅ Event system handles 60+ FPS without performance loss
✅ Multi-mode builds (FULL/ENGINE_ONLY/STUDIOONLY) compile and run
Phase 2 Complete When:
✅ Resource loading/unloading works
✅ Input handling integrated
✅ Engine and Studio communicate only through Runtime interfaces
✅ Cross-platform builds work (Windows/Linux)