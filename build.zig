const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const module = b.createModule(
        .{ .link_libcpp = true, .target = target, .optimize = optimize },
    );

    const src_files = &[_][]const u8{
        "main.cpp",
    };
    module.addCSourceFiles(.{
        .root = b.path("src"),
        .files = src_files,
    });

    var exe = b.addExecutable(.{
        .name = "spingame",
        .root_module = module,
    });

    const sfml = b.dependency("sfml", .{
        .target = target,
        .optimize = optimize,
    });

    module.addIncludePath(sfml.path("include"));

    exe.linkLibrary(sfml.artifact("sfml-system"));
    exe.linkLibrary(sfml.artifact("sfml-window"));
    exe.linkLibrary(sfml.artifact("sfml-graphics"));
    exe.linkLibrary(sfml.artifact("sfml-audio"));

    module.addCMacro("SFML_STATIC", "1");
    switch (target.result.os.tag) {
        .windows => {},
        .macos => {
            exe.force_load_objc = true;
        },
        else => {},
    }

    // this installs the SFML headers in include/SFML
    const install_sfml = b.addInstallArtifact(sfml.artifact("sfml-system"), .{ .h_dir = .{ .override = .{ .custom = "../include" } } });
    exe.step.dependOn(&install_sfml.step);

    b.installArtifact(exe);
    b.default_step.dependOn(&exe.step);

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(&exe.step);
    const run_step = b.step("run", "Run the SFML application");
    run_step.dependOn(&run_cmd.step);
}
