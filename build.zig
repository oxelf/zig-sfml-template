const std = @import("std");
const ios = @import("build_ios.zig");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const module = b.createModule(
        .{
            .target = target,
            .optimize = optimize,
            .link_libcpp = true,
            .link_libc = true,
        },
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
        .sysroot = b.sysroot,
    });

    module.addIncludePath(sfml.path("include"));
    module.addCMacro("SFML_STATIC", "1");

    if (target.result.os.tag == .ios) {
        exe.linkLibrary(sfml.artifact("sfml-main"));
    } else {
        exe.linkLibrary(sfml.artifact("sfml-window"));
    }
    exe.linkLibrary(sfml.artifact("sfml-system"));
    exe.linkLibrary(sfml.artifact("sfml-graphics"));
    exe.linkLibrary(sfml.artifact("sfml-audio"));

    switch (target.result.os.tag) {
        .windows => {},
        .macos => {
            exe.force_load_objc = true;
        },
        .ios => {
            const sdk_path = ios.getSdk(b, target);
            b.sysroot = sdk_path;
            module.addSystemIncludePath(ios.includePath(b, sdk_path));

            exe.force_load_objc = true;
        },
        else => {},
    }

    // install the SFML headers in include/SFML
    const install_sfml = b.addInstallArtifact(sfml.artifact("sfml-system"), .{ .h_dir = .{ .override = .{ .custom = "../include" } } });
    exe.step.dependOn(&install_sfml.step);

    b.installArtifact(exe);
    b.default_step.dependOn(&exe.step);

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(&exe.step);
    const run_step = b.step("run", "Run the SFML application");
    run_step.dependOn(&run_cmd.step);
}
