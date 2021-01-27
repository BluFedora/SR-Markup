#ifndef BIFROST_IMGUI_GLFW
#define BIFROST_IMGUI_GLFW

#include "bf/PlatformFwd.h"
#include "bf/bf_gfx_handle.h"

namespace bf
{
  using Event = bfEvent;

  namespace imgui
  {
    void startup(bfGfxContextHandle graphics, bfWindow* window);
    void onEvent(bfWindow* target_window, Event& evt);
    void beginFrame(bfTextureHandle surface, float window_width, float window_height, float delta_time);
    void endFrame();
    void shutdown();

    // Helpers

    void setupDefaultRenderPass(bfGfxCommandListHandle command_list, bfTextureHandle surface);
  }  // namespace imgui

}  // namespace bf

#endif /* BIFROST_IMGUI_GLFW */