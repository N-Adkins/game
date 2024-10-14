const std = @import("std");
const vk = @import("vulkan");
const log = @import("../log.zig");
const VulkanContext = @import("vulkan_context.zig").VulkanContext;
const Allocator = std.mem.Allocator;

pub const Swapchain = struct {
    pub const PresentState = enum {
        optimal,
        suboptimal,
    };

    vulkan_ctx: *const VulkanContext,
    allocator: Allocator,

    surface_format: vk.SurfaceFormatKHR,
    present_mode: vk.PresentModeKHR,
    extent: vk.Extent2D,
    handle: vk.SwapchainKHR,

    swap_images: []SwapImage,
    image_index: u32,
    next_image_acquired: vk.Semaphore,

    pub fn init(
        vulkan_ctx: *const VulkanContext, 
        allocator: Allocator,
        extent: vk.Extent2D,
    ) !Swapchain {
        return initRecycle(vulkan_ctx, allocator, extent, .null_handle);
    }

    pub fn initRecycle(
        vulkan_ctx: *const VulkanContext,
        allocator: Allocator,
        extent: vk.Extent2D,
        old_handle: vk.SwapchainKHR,
    ) !Swapchain {
        const capabilities = try vulkan_ctx.instance.getPhysicalDeviceSurfaceCapabilitiesKHR(
            vulkan_ctx.physical_device,
            vulkan_ctx.surface,
        );
        const actual_extent = findActualExtent(capabilities, extent);
        if (actual_extent.width == 0 or actual_extent.height == 0) {
            return error.InvalidSurfaceDimensions;
        }

        const surface_format = try findSurfaceFormat(vulkan_ctx, allocator);
        const present_mode = try findPresentMode(vulkan_ctx, allocator);

        var image_count = capabilities.min_image_count + 1;
        if (capabilities.max_image_count > 0) {
            image_count = @min(image_count, capabilities.max_image_count);
        }

        const queue_indices = [_]u32{ 
            vulkan_ctx.graphics_queue.family,
            vulkan_ctx.present_queue.family,
        };

        const families_equal = vulkan_ctx.graphics_queue.family == vulkan_ctx.present_queue.family;
        const sharing_mode: vk.SharingMode = if (!families_equal) 
            .concurrent
        else
            .exclusive;

        const handle = try vulkan_ctx.logical_device.createSwapchainKHR(&.{
            .surface = vulkan_ctx.surface,
            .min_image_count = image_count,
            .image_format = surface_format.format,
            .image_color_space = surface_format.color_space,
            .image_extent = actual_extent,
            .image_array_layers = 1,
            .image_usage = .{ .color_attachment_bit = true, .transfer_dst_bit = true },
            .image_sharing_mode = sharing_mode,
            .queue_family_index_count = @intCast(queue_indices.len),
            .p_queue_family_indices = @ptrCast(&queue_indices),
            .pre_transform = capabilities.current_transform,
            .composite_alpha = .{ .opaque_bit_khr = true },
            .present_mode = present_mode,
            .clipped = vk.TRUE,
            .old_swapchain = old_handle,
        }, null);
        errdefer vulkan_ctx.logical_device.destroySwapchainKHR(handle, null);
        
        // Delete old one if there is one
        if (old_handle != .null_handle) {
            vulkan_ctx.logical_device.destroySwapchainKHR(old_handle, null);
        }

        const swap_images = try initSwapchainImages(vulkan_ctx, handle, surface_format.format, allocator);
        errdefer {
            for (swap_images) |image| image.deinit(vulkan_ctx);
            allocator.free(swap_images);
        }

        var next_image_acquired = try vulkan_ctx.logical_device.createSemaphore(&.{}, null);
        errdefer vulkan_ctx.logical_device.destroySemaphore(next_image_acquired, null);

        const result = try vulkan_ctx.logical_device.acquireNextImageKHR(
            handle,
            std.math.maxInt(u64),
            next_image_acquired,
            .null_handle,
        );
        if (result.result != .success) {
            return error.ImageAcquireFailed;
        }

        std.mem.swap(vk.Semaphore, &swap_images[result.image_index].image_acquired, &next_image_acquired);

        return Swapchain{
            .vulkan_ctx = vulkan_ctx,
            .allocator = allocator,
            .surface_format = surface_format,
            .present_mode = present_mode,
            .extent = actual_extent,
            .handle = handle,
            .swap_images = swap_images,
            .image_index = result.image_index,
            .next_image_acquired = next_image_acquired,
        };
    }

    fn deinitExceptSwapchain(self: *Swapchain) void {
        for (self.swap_images) |image| image.deinit(self.vulkan_ctx);
        self.allocator.free(self.swap_images);
        self.vulkan_ctx.logical_device.destroySemaphore(self.next_image_acquired, null);
    }

    pub fn waitForAllFences(self: *Swapchain) void {
        for (self.swap_images) |image| image.waitForFence(self.vulkan_ctx) catch {};
    }

    pub fn deinit(self: *Swapchain) void {
        self.deinitExceptSwapchain();
        self.vulkan_ctx.logical_device.destroySwapchainKHR(self.handle, null);
    }

    pub fn recreate(self: *Swapchain, new_extent: vk.Extent2D) !void {
        const vulkan_ctx = self.vulkan_ctx;
        const allocator = self.allocator;
        const old_handle = self.handle;
        self.deinitExceptSwapchain();
        self.* = try initRecycle(vulkan_ctx, allocator, new_extent, old_handle);
    }

    pub fn currentImage(self: *Swapchain) vk.Image {
        return self.swap_images[self.image_index].image;
    }

    pub fn currentSwapImage(self: *Swapchain) *const SwapImage {
        return &self.swap_images[self.image_index];
    }

    pub fn present(self: *Swapchain, command_buffer: vk.CommandBuffer) !PresentState {
        const current = self.currentSwapImage();
        try current.waitForFence(self.vulkan_ctx);
        try self.vulkan_ctx.logical_device.resetFences(1, @ptrCast(&current.frame_fence));

        const wait_stage = [_]vk.PipelineStageFlags{.{ .top_of_pipe_bit = true }};
        try self.vulkan_ctx.logical_device.queueSubmit(
            self.vulkan_ctx.graphics_queue.handle, 
            1, 
            &[_]vk.SubmitInfo{
                .{
                    .wait_semaphore_count = 1,
                    .p_wait_semaphores = @ptrCast(&current.image_acquired),
                    .p_wait_dst_stage_mask = &wait_stage,
                    .command_buffer_count = 1,
                    .p_command_buffers = @ptrCast(&command_buffer),
                    .signal_semaphore_count = 1,
                    .p_signal_semaphores = @ptrCast(&current.render_finished),
                }
            }, 
            current.frame_fence
        );

        _ = try self.vulkan_ctx.logical_device.queuePresentKHR(
            self.vulkan_ctx.present_queue.handle, 
            &.{
                .wait_semaphore_count = 1,
                .p_wait_semaphores = @ptrCast(&current.render_finished),
                .swapchain_count = 1,
                .p_swapchains = @ptrCast(&self.handle),
                .p_image_indices = @ptrCast(&self.image_index),
            }
        );

        const result = try self.vulkan_ctx.logical_device.acquireNextImageKHR(
            self.handle,
            self.math.maxInt(u64),
            self.next_image_acquired,
            .null_handle
        );

        std.mem.swap(vk.Semaphore, &self.swap_images[result.image_index].image_acquired, &self.next_image_acquired);
        self.image_index = result.image_index;

        return switch (result.result) {
            .success => .optimal,
            .suboptimal_khr => .suboptimal,
            else => unreachable,
        };
    }
};

const SwapImage = struct {
    image: vk.Image,
    view: vk.ImageView,
    image_acquired: vk.Semaphore,
    render_finished: vk.Semaphore,
    frame_fence: vk.Fence,

    fn init(vulkan_ctx: *const VulkanContext, image: vk.Image, format: vk.Format) !SwapImage {
        const view = try vulkan_ctx.logical_device.createImageView(&.{
            .image = image,
            .view_type = .@"2d",
            .format = format,
            .components = .{ .r = .identity, .g = .identity, .b = .identity, .a = .identity },
            .subresource_range = .{
                .aspect_mask = .{ .color_bit = true },
                .base_mip_level = 0,
                .level_count = 1,
                .base_array_layer = 0,
                .layer_count = 1,
            },
        }, null);
        errdefer vulkan_ctx.logical_device.destroyImageView(view, null);

        const image_acquired = try vulkan_ctx.logical_device.createSemaphore(&.{}, null);
        errdefer vulkan_ctx.logical_device.destroySemaphore(image_acquired, null);

        const render_finished = try vulkan_ctx.logical_device.createSemaphore(&.{}, null);
        errdefer vulkan_ctx.logical_device.destroySemaphore(render_finished, null);

        const frame_fence = try vulkan_ctx.logical_device.createFence(&.{ .flags = .{ .signaled_bit = true } }, null);
        errdefer vulkan_ctx.logical_device.destroyFence(frame_fence, null);

        return SwapImage{
            .image = image,
            .view = view,
            .image_acquired = image_acquired,
            .render_finished = render_finished,
            .frame_fence = frame_fence,
        };
    }

    fn deinit(self: *const SwapImage, vulkan_ctx: *const VulkanContext) void {
        self.waitForFence(vulkan_ctx) catch return;
        vulkan_ctx.logical_device.destroyImageView(self.view, null);
        vulkan_ctx.logical_device.destroySemaphore(self.image_acquired, null);
        vulkan_ctx.logical_device.destroySemaphore(self.render_finished, null);
        vulkan_ctx.logical_device.destroyFence(self.frame_fence, null);
    }

    fn waitForFence(self: *const SwapImage, vulkan_ctx: *const VulkanContext) !void {
        _ = try vulkan_ctx.logical_device.waitForFences(1, @ptrCast(&self.frame_fence), vk.TRUE, std.math.maxInt(u64));
    }
};

fn findActualExtent(capabilities: vk.SurfaceCapabilitiesKHR, extent: vk.Extent2D) vk.Extent2D {
    if (capabilities.current_extent.width != 0xFFFF_FFFF) {
        return capabilities.current_extent;
    } else {
        return .{
            .width = std.math.clamp(
                extent.width, 
                capabilities.min_image_extent.width, 
                capabilities.max_image_extent.width,
            ),
            .height = std.math.clamp(
                extent.width, 
                capabilities.min_image_extent.height,
                capabilities.max_image_extent.height,
            ),
        };
    }
}

fn findSurfaceFormat(vulkan_ctx: *const VulkanContext, allocator: Allocator) !vk.SurfaceFormatKHR {
    const preferred: vk.SurfaceFormatKHR = .{
        .format = .b8g8r8a8_srgb,
        .color_space = .srgb_nonlinear_khr,
    };

    const surface_formats = try vulkan_ctx.instance.getPhysicalDeviceSurfaceFormatsAllocKHR(
        vulkan_ctx.physical_device,
        vulkan_ctx.surface,
        allocator,
    );
    defer allocator.free(surface_formats);

    for (surface_formats) |format| {
        if (std.meta.eql(format, preferred)) {
            log.debug("Found preferred surface format {any}", .{preferred});
            return preferred;
        }
    }

    const format = surface_formats[0];

    log.debug("Did not find preferred surface format, found this instead {any}", .{format});

    return format;
}

fn findPresentMode(vulkan_ctx: *const VulkanContext, allocator: Allocator) !vk.PresentModeKHR {
    const present_modes = try vulkan_ctx.instance.getPhysicalDeviceSurfacePresentModesAllocKHR(
        vulkan_ctx.physical_device,
        vulkan_ctx.surface,
        allocator,
    );
    defer allocator.free(present_modes);

    const preferred_modes = [_]vk.PresentModeKHR{
        .mailbox_khr,
        .immediate_khr,
    };

    for (preferred_modes) |mode| {
        if (std.mem.indexOfScalar(vk.PresentModeKHR, present_modes, mode)) |_| {
            return mode;
        }
    }

    return .fifo_khr; // fallback
}

fn initSwapchainImages(
    vulkan_ctx: *const VulkanContext,
    swapchain: vk.SwapchainKHR,
    format: vk.Format,
    allocator: Allocator,
) ![]SwapImage {
    const images = try vulkan_ctx.logical_device.getSwapchainImagesAllocKHR(swapchain, allocator);
    defer allocator.free(images);

    const swap_images = try allocator.alloc(SwapImage, images.len);
    errdefer allocator.free(swap_images);

    var i: usize = 0;
    errdefer for (swap_images[0..i]) |image| image.deinit(vulkan_ctx);

    for (images) |image| {
        swap_images[i] = try SwapImage.init(vulkan_ctx, image, format);
        i += 1;
    }

    return swap_images;
}
