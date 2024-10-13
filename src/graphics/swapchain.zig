const std = @import("std");
const vk = @import("vulkan");
const log = @import("../log.zig");
const GraphicsContext = @import("context.zig").GraphicsContext;
const Allocator = std.mem.Allocator;

pub const Swapchain = struct {
    pub const PresentState = enum {
        optimal,
        suboptimal,
    };

    graphics_context: *const GraphicsContext,
    allocator: Allocator,

    surface_format: vk.SurfaceFormatKHR,
    present_mode: vk.PresentModeKHR,
    extent: vk.Extent2D,
    handle: vk.SwapchainKHR,

    swap_images: []SwapImage,
    image_index: u32,
    next_image_acquired: vk.Semaphore,

    pub fn init(
        graphics_context: *const GraphicsContext, 
        allocator: Allocator,
        extent: vk.Extent2D,
    ) !Swapchain {
        return initRecycle(graphics_context, allocator, extent, .null_handle);
    }

    pub fn initRecycle(
        graphics_context: *const GraphicsContext,
        allocator: Allocator,
        extent: vk.Extent2D,
        old_handle: vk.SwapchainKHR,
    ) !Swapchain {
        const capabilities = try graphics_context.instance.getPhysicalDeviceSurfaceCapabilitiesKHR(
            graphics_context.physical_device,
            graphics_context.surface,
        );
        const actual_extent = findActualExtent(capabilities, extent);
        if (actual_extent.width == 0 or actual_extent.height == 0) {
            return error.InvalidSurfaceDimensions;
        }

        const surface_format = try findSurfaceFormat(graphics_context, allocator);
        const present_mode = try findPresentMode(graphics_context, allocator);

        var image_count = capabilities.min_image_count + 1;
        if (capabilities.max_image_count > 0) {
            image_count = @min(image_count, capabilities.max_image_count);
        }

        const queue_indices = [_]u32{ 
            graphics_context.graphics_queue.family,
            graphics_context.present_queue.family,
        };

        const families_equal = graphics_context.graphics_queue.family == graphics_context.present_queue.family;
        const sharing_mode: vk.SharingMode = if (!families_equal) 
            .concurrent
        else
            .exclusive;

        const handle = try graphics_context.logical_device.createSwapchainKHR(&.{
            .surface = graphics_context.surface,
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
        errdefer graphics_context.logical_device.destroySwapchainKHR(handle, null);
        
        // Delete old one if there is one
        if (old_handle != .null_handle) {
            graphics_context.logical_device.destroySwapchainKHR(old_handle, null);
        }

        const swap_images = try initSwapchainImages(graphics_context, handle, surface_format.format, allocator);
        errdefer {
            for (swap_images) |image| image.deinit(graphics_context);
            allocator.free(swap_images);
        }

        var next_image_acquired = try graphics_context.logical_device.createSemaphore(&.{}, null);
        errdefer graphics_context.logical_device.destroySemaphore(next_image_acquired, null);

        const result = try graphics_context.logical_device.acquireNextImageKHR(
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
            .graphics_context = graphics_context,
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
        for (self.swap_images) |image| image.deinit(self.graphics_context);
        self.allocator.free(self.swap_images);
        self.graphics_context.logical_device.destroySemaphore(self.next_image_acquired, null);
    }

    pub fn waitForAllFences(self: *Swapchain) void {
        for (self.swap_images) |image| image.waitForFence(self.graphics_context) catch {};
    }

    pub fn deinit(self: *Swapchain) void {
        self.deinitExceptSwapchain();
        self.graphics_context.logical_device.destroySwapchainKHR(self.handle, null);
    }

    pub fn recreate(self: *Swapchain, new_extent: vk.Extent2D) !void {
        const graphics_context = self.graphics_context;
        const allocator = self.allocator;
        const old_handle = self.handle;
        self.deinitExceptSwapchain();
        self.* = try initRecycle(graphics_context, allocator, new_extent, old_handle);
    }

    pub fn currentImage(self: *Swapchain) vk.Image {
        return self.swap_images[self.image_index].image;
    }

    pub fn currentSwapImage(self: *Swapchain) *const SwapImage {
        return &self.swap_images[self.image_index];
    }

    pub fn present(self: *Swapchain, command_buffer: vk.CommandBuffer) !PresentState {
        const current = self.currentSwapImage();
        try current.waitForFence(self.graphics_context);
        try self.graphics_context.logical_device.resetFences(1, @ptrCast(&current.frame_fence));

        const wait_stage = [_]vk.PipelineStageFlags{.{ .top_of_pipe_bit = true }};
        try self.graphics_context.logical_device.queueSubmit(
            self.graphics_context.graphics_queue.handle, 
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

        _ = try self.graphics_context.logical_device.queuePresentKHR(
            self.graphics_context.present_queue.handle, 
            &.{
                .wait_semaphore_count = 1,
                .p_wait_semaphores = @ptrCast(&current.render_finished),
                .swapchain_count = 1,
                .p_swapchains = @ptrCast(&self.handle),
                .p_image_indices = @ptrCast(&self.image_index),
            }
        );

        const result = try self.graphics_context.logical_device.acquireNextImageKHR(
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

    fn init(graphics_context: *const GraphicsContext, image: vk.Image, format: vk.Format) !SwapImage {
        const view = try graphics_context.logical_device.createImageView(&.{
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
        errdefer graphics_context.logical_device.destroyImageView(view, null);

        const image_acquired = try graphics_context.logical_device.createSemaphore(&.{}, null);
        errdefer graphics_context.logical_device.destroySemaphore(image_acquired, null);

        const render_finished = try graphics_context.logical_device.createSemaphore(&.{}, null);
        errdefer graphics_context.logical_device.destroySemaphore(render_finished, null);

        const frame_fence = try graphics_context.logical_device.createFence(&.{ .flags = .{ .signaled_bit = true } }, null);
        errdefer graphics_context.logical_device.destroyFence(frame_fence, null);

        return SwapImage{
            .image = image,
            .view = view,
            .image_acquired = image_acquired,
            .render_finished = render_finished,
            .frame_fence = frame_fence,
        };
    }

    fn deinit(self: *const SwapImage, graphics_context: *const GraphicsContext) void {
        self.waitForFence(graphics_context) catch return;
        graphics_context.logical_device.destroyImageView(self.view, null);
        graphics_context.logical_device.destroySemaphore(self.image_acquired, null);
        graphics_context.logical_device.destroySemaphore(self.render_finished, null);
        graphics_context.logical_device.destroyFence(self.frame_fence, null);
    }

    fn waitForFence(self: *const SwapImage, graphics_context: *const GraphicsContext) !void {
        _ = try graphics_context.logical_device.waitForFences(1, @ptrCast(&self.frame_fence), vk.TRUE, std.math.maxInt(u64));
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

fn findSurfaceFormat(graphics_context: *const GraphicsContext, allocator: Allocator) !vk.SurfaceFormatKHR {
    const preferred: vk.SurfaceFormatKHR = .{
        .format = .b8g8r8a8_srgb,
        .color_space = .srgb_nonlinear_khr,
    };

    const surface_formats = try graphics_context.instance.getPhysicalDeviceSurfaceFormatsAllocKHR(
        graphics_context.physical_device,
        graphics_context.surface,
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

fn findPresentMode(graphics_context: *const GraphicsContext, allocator: Allocator) !vk.PresentModeKHR {
    const present_modes = try graphics_context.instance.getPhysicalDeviceSurfacePresentModesAllocKHR(
        graphics_context.physical_device,
        graphics_context.surface,
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
    graphics_context: *const GraphicsContext,
    swapchain: vk.SwapchainKHR,
    format: vk.Format,
    allocator: Allocator,
) ![]SwapImage {
    const images = try graphics_context.logical_device.getSwapchainImagesAllocKHR(swapchain, allocator);
    defer allocator.free(images);

    const swap_images = try allocator.alloc(SwapImage, images.len);
    errdefer allocator.free(swap_images);

    var i: usize = 0;
    errdefer for (swap_images[0..i]) |image| image.deinit(graphics_context);

    for (images) |image| {
        swap_images[i] = try SwapImage.init(graphics_context, image, format);
        i += 1;
    }

    return swap_images;
}
