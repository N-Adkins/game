const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const engine = b.addSharedLibrary(.{
        .name = "engine",
        .root_source_file = b.path("src/entry.zig"),
        .target = target,
        .optimize = optimize,
    });
    b.installArtifact(engine);

    engine.linkLibC();

    if (target.result.os.tag == .linux) {
        engine.linkSystemLibrary("X11");
        engine.linkSystemLibrary("X11-xcb");
        engine.linkSystemLibrary("xcb");
        engine.linkSystemLibrary("xcb-randr");
        engine.linkSystemLibrary("xkbcommon");
    }

    const registry = b.dependency("vulkan-headers", .{}).path("registry/vk.xml");
    const vk_gen = b.dependency("vulkan", .{}).artifact("vulkan-zig-generator");

    const vk_generate_cmd = b.addRunArtifact(vk_gen);
    vk_generate_cmd.addArg(registry.getPath(b));

    const vulkan_zig = b.addModule("vulkan-zig", .{
        .root_source_file = vk_generate_cmd.addOutputFileArg("vk.zig"),
    });
    engine.root_module.addImport("vulkan", vulkan_zig);

    const exe_unit_tests = b.addTest(.{
        .root_source_file = b.path("src/entry.zig"),
        .target = target,
        .optimize = optimize,
    });
    const run_exe_unit_tests = b.addRunArtifact(exe_unit_tests);

    const test_step = b.step("test", "Run unit tests");
    test_step.dependOn(&run_exe_unit_tests.step);
}
