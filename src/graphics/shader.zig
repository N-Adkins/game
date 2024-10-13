const std = @import("std");
const vk = @import("vulkan");
const log = @import("../log.zig");
const GraphicsContext = @import("context.zig").GraphicsContext;
const Allocator = std.mem.Allocator;

pub const ShaderStages = struct {
    vert: vk.ShaderModule,
    frag: vk.ShaderModule,
    create_info: [2]vk.PipelineShaderStageCreateInfo,

    pub fn init(graphics_context: *const GraphicsContext, vert_code: []const u8, frag_code: []const u8) !ShaderStages {
        const aligned_vert = std.mem.bytesAsSlice(u32, vert_code);
        const aligned_frag = std.mem.bytesAsSlice(u32, frag_code);

        var self: ShaderStages = undefined;

        self.vert = try graphics_context.logical_device.createShaderModule(&.{
            .code_size = aligned_vert.len,
            .p_code = @ptrCast(aligned_vert.ptr),
        }, null);

        self.frag = try graphics_context.logical_device.createShaderModule(&.{
            .code_size = aligned_frag.len,
            .p_code = @ptrCast(aligned_frag.ptr),
        }, null);

        self.create_info = .{
            .{
                .stage = .{ .vertex_bit = true },
                .module = self.vert,
                .p_name = "main",
            },
            .{
                .stage = .{ .fragment_bit = true },
                .module = self.frag,
                .p_name = "main",
            },
        };
        
        return self;
    }

    pub fn deinit(self: *ShaderStages, graphics_context: *const GraphicsContext) void {
        graphics_context.logical_device.destroyShaderModule(self.vert, null);
        graphics_context.logical_device.destroyShaderModule(self.frag, null);
    }
};
