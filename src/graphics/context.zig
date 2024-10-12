const std = @import("std");
const glfw = @import("zglfw");
const vk = @import("vulkan");
const Allocator = std.mem.Allocator;

const apis: []const vk.ApiInfo = &.{
    vk.features.version_1_0,
    vk.extensions.khr_surface,
    vk.extensions.khr_swapchain,
};

const BaseDispatch = vk.BaseWrapper(apis);
const InstanceDispatch = vk.InstanceWrapper(apis);
const DeviceDispatch = vk.DeviceWrapper(apis);

const Instance = vk.InstanceProxy(apis);
const Device = vk.DeviceProxy(apis);

pub const GraphicsContext = struct {
    allocator: Allocator,

    base_dispatch: BaseDispatch,

    instance: Instance,

    pub fn init(allocator: Allocator, app_name: [*:0]const u8) !GraphicsContext {
        var self: GraphicsContext = undefined;
        self.allocator = allocator;

        // Had to kind of hack this bit together because my libraries weren't playing nice.
        self.base_dispatch = try BaseDispatch.load(
            @as(*const fn(instance: vk.Instance, procname: [*:0]const u8) ?*const fn() void, 
                @ptrCast(&glfw.getInstanceProcAddress)
            )
        );

        const glfw_extensions = try glfw.getRequiredInstanceExtensions();

        const app_info: vk.ApplicationInfo = .{
            .p_application_name = app_name,
            .application_version = vk.makeApiVersion(0, 0, 0, 0),
            .p_engine_name = app_name,
            .engine_version = vk.makeApiVersion(0, 0, 0, 0),
            .api_version = vk.API_VERSION_1_0,
        };

        const instance = try self.base_dispatch.createInstance(&.{
            .p_application_info = &app_info,
            .enabled_extension_count = @intCast(glfw_extensions.len),
            .pp_enabled_extension_names = @ptrCast(glfw_extensions.ptr),
        }, null);

        const instance_dispatch = try allocator.create(InstanceDispatch);
        errdefer allocator.destroy(instance_dispatch);

        instance_dispatch.* = try InstanceDispatch.load(
            instance, 
            self.base_dispatch.dispatch.vkGetInstanceProcAddr
        );
        self.instance = Instance.init(instance, instance_dispatch);
        errdefer self.instance.destroyInstance(null);

        return self;
    }

    pub fn deinit(self: *GraphicsContext) void {
        self.instance.destroyInstance(null);
        self.allocator.destroy(self.instance.wrapper);
    }
};
