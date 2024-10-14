const std = @import("std");
const vk = @import("vulkan");
const log = @import("../log.zig");
const GraphicsContext = @import("context.zig").GraphicsContext;
const Swapchain = @import("swapchain.zig").Swapchain;
const Allocator = std.mem.Allocator;

pub fn createFramebufers(
    graphics_context: *const GraphicsContext, 
    allocator: Allocator,
    render_pass: vk.RenderPass,
    swapchain: Swapchain,
) ![]vk.Framebuffer {
    const framebuffers = try allocator.alloc(vk.Framebuffer, swapchain.swap_images.len);
    errdefer allocator.free(framebuffers);

    var i: usize = 0;
    errdefer for (framebuffers[0..i]) |buffer| graphics_context.logical_device.destroyFramebuffer(buffer, null);

    for (framebuffers) |*buffer| {
        buffer.* = try graphics_context.logical_device.createFramebuffer(&.{
            .render_pass = render_pass,
            .attachment_count = 1,
            .p_attachments = @ptrCast(&swapchain.swap_images[i].view),
            .width = swapchain.extent.width,
            .height = swapchain.extent.height,
            .layers = 1,
        }, null);
        i += 1;
    }

    return framebuffers;
}

pub fn destroyFramebuffers(
    graphics_context: *const GraphicsContext,
    allocator: Allocator,
    framebuffers: []const vk.Framebuffer,
) void {
    for (framebuffers) |buffer| graphics_context.logical_device.destroyFramebuffer(buffer, null);
    allocator.free(framebuffers);
}
