const std = @import("std");
const builtin = @import("builtin");
const Build = std.Build;

const bundle_identifier = "com.yourcompany.sfml_project";
const app_name = "spingame";

pub fn getSdk(b: *Build, t: Build.ResolvedTarget) []const u8 {
    const sdk = if (t.result.abi == .simulator) "iphonesimulator" else "iphoneos";
    const child = std.process.Child.run(.{
        .argv = &[_][]const u8{ "xcrun", "--show-sdk-path", "--sdk", sdk },
        .allocator = b.allocator,
    }) catch @panic("Failed to run xcrun");
    var path = child.stdout;
    return path[0 .. path.len - 1];
}

pub fn includePath(b: *Build, sdk_path: []const u8) Build.LazyPath {
    const path = std.fmt.allocPrint(b.allocator, "{s}/usr/include", .{sdk_path}) catch "";
    std.debug.print("iOS include path: {s}\n", .{path});
    return .{ .cwd_relative = path };
}

pub const CreateIosApp = struct {
    step: std.Build.Step,
    exe_path: std.Build.LazyPath,
    output: ?std.Build.LazyPath,
    assets: std.Build.LazyPath,
    simulator: bool = false,

    pub fn create(owner: *Build, exe_path: Build.LazyPath, assets: Build.LazyPath) *CreateIosApp {
        const self = owner.allocator.create(CreateIosApp) catch @panic("OOM");
        self.* = .{
            .step = std.Build.Step.init(.{
                .id = .custom,
                .name = "CreateIosApp",
                .owner = owner,
                .makeFn = make,
            }),
            .assets = assets,
            .exe_path = exe_path,
            .output = null,
        };
        return self;
    }

    pub fn make(step: *std.Build.Step, opts: std.Build.Step.MakeOptions) !void {
        const b = step.owner;
        const self: *CreateIosApp = @fieldParentPtr("step", step);
        var prog_node = opts.progress_node.start("create ios app", 10);
        defer prog_node.end();

        //const exe = self.exe_path.getPath(b);

        // create .app directory
        const app_dir_name = app_name ++ ".app";
        const app_dir = b.pathJoin(&.{ b.install_path, "bin", app_dir_name });
        std.debug.print("Creating iOS app directory at: {s}\n", .{app_dir});
        std.fs.deleteDirAbsolute(app_dir) catch {};
        std.debug.print("Making directory: {s}\n", .{app_dir});
        try std.fs.makeDirAbsolute(app_dir);
        std.debug.print("Created directory: {s}\n", .{app_dir});
        const app_dir_handle = try std.fs.openDirAbsolute(app_dir, .{});
        std.debug.print("Created iOS app directory: {s}\n", .{app_dir});
        std.debug.print("Copying assets from {s} to {s}\n", .{ self.assets.getPath(b), app_dir });
        std.debug.print("Copying executable from {s} to {s}\n", .{ self.exe_path.getPath(b), app_dir });
        try std.fs.cwd().copyFile(self.exe_path.getPath(b), app_dir_handle, app_name, .{});

        const info_plist = try std.fs.createFileAbsolute(b.pathJoin(&.{ app_dir, "Info.plist" }), .{ .mode = 0o644 });

        defer info_plist.close();

        var buffer: [4096]u8 = undefined;

        var plist_writer = info_plist.writer(&buffer).interface;

        plist_writer.print(
            \\<?xml version="1.0" encoding="UTF-8"?>
            \\<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
            \\<plist version="1.0">
            \\<dict>
            \\  <key>CFBundleDevelopmentRegion</key>
            \\  <string>en</string>
            \\  <key>CFBundleDisplayName</key>
            \\  <string>{s}</string>
            \\   <key>CFBundleExecutable</key>
            \\   <string>{s}</string>
            \\   <key>CFBundleIdentifier</key>
            \\   <string>{s}</string>
            \\   <key>CFBundleInfoDictionaryVersion</key>
            \\   <string>6.0</string>
            \\   <key>CFBundleName</key>
            \\   <string>{s}</string>
            \\   <key>CFBundlePackageType</key>
            \\   <string>APPL</string>
            \\   <key>CFBundleShortVersionString</key>
            \\   <string>1.0.0</string>
            \\   <key>CFBundleSignature</key>
            \\   <string>MAKE</string>
            \\   <key>CFBundleVersion</key>
            \\   <string>1</string>
            \\   <key>DTPlatformName</key>
            \\   <string>iPhoneOS</string>
            \\   <key>DTPlatformVersion</key>
            \\   <string>15.4</string>
            \\   <key>LSRequiresIPhoneOS</key>
            \\   <true/>
            \\   <key>UISupportedInterfaceOrientations</key>
            \\   <array>
            \\     <string>UIInterfaceOrientationPortrait</string>
            \\     <string>UIInterfaceOrientationPortraitUpsideDown</string>
            \\     <string>UIInterfaceOrientationLandscapeLeft</string>
            \\     <string>UIInterfaceOrientationLandscapeRight</string>
            \\   </array>
            \\   <key>UILaunchScreen</key>
            \\   <dict>
            \\     <key>UIColorName</key>
            \\     <string>red</string>
            \\   </dict>
            \\ </dict>
            \\ </plist>
        , .{ app_name, app_name, bundle_identifier, bundle_identifier }) catch {};

        //   <string>SpinGame</string>
        //   <key>CFBundleExecutable</key>
        //   <string>spingame</string>
        //   <key>CFBundleIdentifier</key>
        //   <string>com.oxelf.madewithzig</string>
        //   <key>CFBundleInfoDictionaryVersion</key>
        //   <string>6.0</string>
        //   <key>CFBundleName</key>
        //   <string>com.jakubkonka.madewithzig</string>
        //   <key>CFBundlePackageType</key>
        //   <string>APPL</string>
        //   <key>CFBundleShortVersionString</key>
        //   <string>1.0.0</string>
        //   <key>CFBundleSignature</key>
        //   <string>MAKE</string>
        //   <key>CFBundleVersion</key>
        //   <string>1</string>
        //   <key>DTPlatformName</key>
        //   <string>iPhoneOS</string>
        //   <key>DTPlatformVersion</key>
        //   <string>15.4</string>
        //   <key>LSRequiresIPhoneOS</key>
        //   <true/>
        //   <key>UISupportedInterfaceOrientations</key>
        //   <array>
        //     <string>UIInterfaceOrientationPortrait</string>
        //     <string>UIInterfaceOrientationPortraitUpsideDown</string>
        //     <string>UIInterfaceOrientationLandscapeLeft</string>
        //     <string>UIInterfaceOrientationLandscapeRight</string>
        //   </array>
        //   <key>UILaunchScreen</key>
        //   <dict>
        //     <key>UIColorName</key>
        //     <string>red</string>
        //   </dict>
        // </dict>
        // </plist>

        // // read xcrun output
        // var buf: [std.c.PATH_MAX]u8 = undefined;
        // var path = try std.fs.cwd().readFile(self.sdkroot.getPath(b), &buf);
        // // want first line
        // for (path, 0..) |c, i| {
        //     if (c == '\n') {
        //         path.len = i;
        //         break;
        //     }
        // }
        // prog_node.completeOne();
        //
        // var man = b.graph.cache.obtain();
        // defer man.deinit();
        //
        // man.hash.addBytes("GenLibCFile");
        // man.hash.addBytes(&[_]u8{ 0x2e, 0x4c, 0x48, 0x3d, 0x66, 0x3c, 0xc2, 0x80 });
        // man.hash.addBytes(path);
        //
        // if (try step.cacheHit(&man)) {
        //     const digest = man.final();
        //     self.generated.path = try b.cache_root.join(b.allocator, &.{ "o", &digest, "libc.txt" });
        //     return;
        // }
        //
        // const digest = man.final();
        // self.generated.path = try b.cache_root.join(b.allocator, &.{ "o", &digest, "libc.txt" });
        //
        // const cache_path = "o" ++ std.fs.path.sep_str ++ digest;
        // var cache_dir = b.cache_root.handle.makeOpenPath(cache_path, .{}) catch |err| {
        //     return step.fail("unable to make path '{any}{s}': {s}", .{
        //         b.cache_root, cache_path, @errorName(err),
        //     });
        // };
        // defer cache_dir.close();
        //
        // var f = try cache_dir.createFile("libc.txt", .{});
        // defer f.close();
        //
        // try f.writeAll("include_dir=");
        // try f.writeAll(path);
        // try f.writeAll(std.fs.path.sep_str ++ "usr" ++ std.fs.path.sep_str ++ "include");
        // try f.writeAll("\nsys_include_dir=");
        // try f.writeAll(path);
        // try f.writeAll(std.fs.path.sep_str ++ "usr" ++ std.fs.path.sep_str ++ "include");
        // try f.writeAll("\ncrt_dir=");
        // try f.writeAll("\nmsvc_lib_dir=");
        // try f.writeAll("\nkernel32_lib_dir=");
        // try f.writeAll("\ngcc_dir=");
        // try f.writeAll("\n");
        //
        // try step.writeManifest(&man);
        prog_node.completeOne();
    }
};

pub const GenLibCFile = struct {
    step: std.Build.Step,
    sdkroot: std.Build.LazyPath,
    generated: std.Build.GeneratedFile,
    output: std.Build.LazyPath,

    pub fn create(owner: *std.Build, sdkroot: std.Build.LazyPath) *GenLibCFile {
        const self = owner.allocator.create(GenLibCFile) catch @panic("OOM");
        self.* = .{
            .step = std.Build.Step.init(.{
                .id = .custom,
                .name = "GenLibCFile",
                .owner = owner,
                .makeFn = make,
            }),
            .sdkroot = sdkroot,
            .generated = .{ .step = &self.step },
            .output = .{ .generated = .{ .file = &self.generated } },
        };
        return self;
    }

    pub fn make(step: *std.Build.Step, opts: std.Build.Step.MakeOptions) !void {
        const b = step.owner;
        const self: *GenLibCFile = @fieldParentPtr("step", step);
        var prog_node = opts.progress_node.start("gen libc file", 2);
        defer prog_node.end();

        // read xcrun output
        var buf: [std.c.PATH_MAX]u8 = undefined;
        var path = try std.fs.cwd().readFile(self.sdkroot.getPath(b), &buf);
        // want first line
        for (path, 0..) |c, i| {
            if (c == '\n') {
                path.len = i;
                break;
            }
        }
        prog_node.completeOne();

        var man = b.graph.cache.obtain();
        defer man.deinit();

        man.hash.addBytes("GenLibCFile");
        man.hash.addBytes(&[_]u8{ 0x2e, 0x4c, 0x48, 0x3d, 0x66, 0x3c, 0xc2, 0x80 });
        man.hash.addBytes(path);

        if (try step.cacheHit(&man)) {
            const digest = man.final();
            self.generated.path = try b.cache_root.join(b.allocator, &.{ "o", &digest, "libc.txt" });
            return;
        }

        const digest = man.final();
        self.generated.path = try b.cache_root.join(b.allocator, &.{ "o", &digest, "libc.txt" });

        const cache_path = "o" ++ std.fs.path.sep_str ++ digest;
        var cache_dir = b.cache_root.handle.makeOpenPath(cache_path, .{}) catch |err| {
            return step.fail("unable to make path '{any}{s}': {s}", .{
                b.cache_root, cache_path, @errorName(err),
            });
        };
        defer cache_dir.close();

        var f = try cache_dir.createFile("libc.txt", .{});
        defer f.close();

        try f.writeAll("include_dir=");
        try f.writeAll(path);
        try f.writeAll(std.fs.path.sep_str ++ "usr" ++ std.fs.path.sep_str ++ "include");
        try f.writeAll("\nsys_include_dir=");
        try f.writeAll(path);
        try f.writeAll(std.fs.path.sep_str ++ "usr" ++ std.fs.path.sep_str ++ "include");
        try f.writeAll("\ncrt_dir=");
        try f.writeAll("\nmsvc_lib_dir=");
        try f.writeAll("\nkernel32_lib_dir=");
        try f.writeAll("\ngcc_dir=");
        try f.writeAll("\n");

        try step.writeManifest(&man);
        prog_node.completeOne();
    }
};
