const std = @import("std");

// Although this function looks imperative, note that its job is to
// declaratively construct a build graph that will be executed by an external
// runner.
pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});

    const optimize = b.standardOptimizeOption(.{
        .preferred_optimize_mode = .Debug
    });

    const exe = b.addExecutable(.{
        .name = "game",
        .root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = optimize,
    });
    b.installArtifact(exe);

    const zglfw = b.dependency("zglfw", .{
        .target = target,
        .optimize = .ReleaseSafe,
    });
    exe.root_module.addImport("zglfw", zglfw.module("root"));
    exe.linkLibrary(zglfw.artifact("glfw"));

    //const vulkan_headers = b.dependency("vulkan-headers", .{});
    //const vkzig_dep = b.dependency("vulkan", .{
    //    .registry = @as([]const u8, vulkan_headers.path("registry/vk.xml").getPath(b))
    //});
    //const vkzig_bindings = vkzig_dep.module("vulkan-zig");
    //exe.root_module.addImport("vulkan", vkzig_bindings);

    const registry = b.dependency("vulkan-headers", .{}).path("registry/vk.xml");
    const vk_gen = b.dependency("vulkan", .{}).artifact("vulkan-zig-generator");

    const vk_generate_cmd = b.addRunArtifact(vk_gen);
    vk_generate_cmd.addArg(registry.getPath(b));

    const vulkan_zig = b.addModule("vulkan-zig", .{
        .root_source_file = vk_generate_cmd.addOutputFileArg("vk.zig"),
    });
    exe.root_module.addImport("vulkan", vulkan_zig);

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);

    const exe_unit_tests = b.addTest(.{
        .root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = optimize,
    });
    const run_exe_unit_tests = b.addRunArtifact(exe_unit_tests);

    const test_step = b.step("test", "Run unit tests");
    test_step.dependOn(&run_exe_unit_tests.step);
}
