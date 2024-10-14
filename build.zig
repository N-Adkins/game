const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const editor = b.dependency("editor", .{
        .target = target,
        .optimize = optimize,
    });
    const editor_artifact = editor.artifact("editor");
    b.installArtifact(editor_artifact);

    const engine = b.dependency("engine", .{
        .target = target,
        .optimize = optimize,
    });
    const engine_artifact = engine.artifact("engine");
    b.installArtifact(engine_artifact);

    const copy_engine_step = b.addInstallBinFile(
        engine_artifact.getEmittedBin(), 
        "libengine.so",
    );
    b.getInstallStep().dependOn(&copy_engine_step.step);

    const run_cmd = b.addRunArtifact(editor_artifact);
    run_cmd.step.dependOn(b.getInstallStep());

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}
