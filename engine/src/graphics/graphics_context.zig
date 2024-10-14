const std = @import("std");
const glfw = @import("zglfw");
const vk = @import("vulkan");
const log = @import("../log.zig");
const VulkanContext = @import("vulkan_context.zig").VulkanContext;
const Swapchain = @import("swapchain.zig").Swapchain;
const Allocator = std.mem.Allocator;

pub const GraphicsContext = struct {
    vulkan_ctx: VulkanContext,
    swapchain: Swapchain,
    
    //render_pass: vk.RenderPass,
    //framebuffers: []vk.Framebuffer,
    
    pub fn init(allocator: Allocator, window: *glfw.Window) !GraphicsContext {
        const vulkan_ctx = try VulkanContext.init(allocator, "Engine", window);
        errdefer vulkan_ctx.deinit();

        const window_size = window.getSize();

        const swapchain = try Swapchain.init(
            &vulkan_ctx, 
            allocator, 
            .{ 
                .width = @intCast(window_size[0]),
                .height = @intCast(window_size[1]),
            }
        );

        return GraphicsContext {
            .vulkan_ctx = vulkan_ctx,
            .swapchain = swapchain,
        };
    }

    pub fn deinit(self: *GraphicsContext) void {
        self.swapchain.deinit();
        self.vulkan_ctx.deinit();
    }
};
