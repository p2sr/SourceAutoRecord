const SarTarget = enum { linux, windows };

pub fn build(b: *std.Build) void {
    const target_option: SarTarget = b.option(SarTarget, "target", "The target to build SAR for; default 'linux'") orelse .linux;
    const target_query: std.Target.Query = std.Build.parseTargetQuery(switch (target_option) {
        .linux => .{ .arch_os_abi = "x86-linux-gnu.2.28" },
        .windows => .{ .arch_os_abi = "x86-windows-msvc" },
    }) catch unreachable; // the queries are hardcoded and valid
    const target = b.resolveTargetQuery(target_query);
    const optimize = b.standardOptimizeOption(.{});

    const sources = findSarSources(b) catch |err| std.debug.panic("error listing source files: {s}", .{@errorName(err)});

    const disable_watermark = b.option(bool, "no-watermark", "Disable the development watermark in canary builds; default false") orelse false;

    const mod = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
        .link_libcpp = true,
        .pic = true,
    });
    mod.addCSourceFiles(.{
        .root = b.path("src"),
        .files = sources,
        .flags = &.{
            "-Wall",
            "-Wno-parentheses",
            "-Wno-unknown-pragmas",
            "-Wno-delete-non-virtual-dtor",
            "-Wno-overloaded-virtual",
            "-Wno-inconsistent-missing-override",
            "-Wno-date-time",
            "-std=c++20",
            "-fno-sanitize=alignment",
        },
        .language = .cpp,
    });
    if (disable_watermark) mod.addCMacro("NO_DEV_WATERMARK", "1");
    mod.addIncludePath(b.path("src"));
    mod.addIncludePath(generateHeaders(b));
    mod.addIncludePath(b.path("lib/curl/include"));
    mod.addIncludePath(b.path("lib/ffmpeg/include"));
    switch (target_option) {
        .linux => {
            mod.addCMacro("_GNU_SOURCE", "1");
            mod.addCMacro("SFML_STATIC", "1");
            mod.addCMacro("CURL_STATICLIB", "1");
            mod.addObjectFile(b.path("lib/curl/lib/linux/libcrypto.a"));
            mod.addObjectFile(b.path("lib/curl/lib/linux/libcurl.a"));
            mod.addObjectFile(b.path("lib/curl/lib/linux/libnghttp2.a"));
            mod.addObjectFile(b.path("lib/curl/lib/linux/libssl.a"));
            mod.addObjectFile(b.path("lib/ffmpeg/lib/linux/libavcodec.a"));
            mod.addObjectFile(b.path("lib/ffmpeg/lib/linux/libavformat.a"));
            mod.addObjectFile(b.path("lib/ffmpeg/lib/linux/libavutil.a"));
            mod.addObjectFile(b.path("lib/ffmpeg/lib/linux/libogg.a"));
            mod.addObjectFile(b.path("lib/ffmpeg/lib/linux/libopus.a"));
            mod.addObjectFile(b.path("lib/ffmpeg/lib/linux/libswresample.a"));
            mod.addObjectFile(b.path("lib/ffmpeg/lib/linux/libswscale.a"));
            mod.addObjectFile(b.path("lib/ffmpeg/lib/linux/libvorbis.a"));
            mod.addObjectFile(b.path("lib/ffmpeg/lib/linux/libvorbisenc.a"));
            mod.addObjectFile(b.path("lib/ffmpeg/lib/linux/libvorbisfile.a"));
            mod.addObjectFile(b.path("lib/ffmpeg/lib/linux/libvpx.a"));
            mod.addObjectFile(b.path("lib/ffmpeg/lib/linux/libx264.a"));
            mod.linkLibrary(makeSfml(b, target));
            mod.linkLibrary(makeDiscordRpc(b, target));
            mod.linkLibrary(makeX265(b, target));
        },
        .windows => {
            mod.addIncludePath(b.path("lib/minhook"));
            @panic("TODO");
        },
    }

    const lib = b.addSharedLibrary(.{
        .name = "sar",
        .root_module = mod,
    });
    lib.link_z_notext = true;

    // Don't just use `installArtifact` -- that'd name it `libsar.so`.
    const install = b.addInstallArtifact(lib, .{ .dest_sub_path = "sar.so" });
    b.getInstallStep().dependOn(&install.step);
}

fn makeSfml(b: *std.Build, target: std.Build.ResolvedTarget) *std.Build.Step.Compile {
    const sfml = b.dependency("SFML", .{});
    const sfml_sources: []const []const u8 = &.{
        "SFML/System/Thread.cpp",
        "SFML/System/Unix/SleepImpl.cpp",
        "SFML/System/Unix/ThreadLocalImpl.cpp",
        "SFML/System/Unix/ThreadImpl.cpp",
        "SFML/System/Unix/ClockImpl.cpp",
        "SFML/System/Unix/MutexImpl.cpp",
        "SFML/System/Err.cpp",
        "SFML/System/MemoryInputStream.cpp",
        "SFML/System/Mutex.cpp",
        //"SFML/System/Win32/SleepImpl.cpp",
        //"SFML/System/Win32/ThreadLocalImpl.cpp",
        //"SFML/System/Win32/ThreadImpl.cpp",
        //"SFML/System/Win32/ClockImpl.cpp",
        //"SFML/System/Win32/MutexImpl.cpp",
        "SFML/System/Clock.cpp",
        "SFML/System/FileInputStream.cpp",
        "SFML/System/Sleep.cpp",
        "SFML/System/Lock.cpp",
        "SFML/System/ThreadLocal.cpp",
        "SFML/System/Time.cpp",
        "SFML/System/String.cpp",
        "SFML/Network/UdpSocket.cpp",
        "SFML/Network/Unix/SocketImpl.cpp",
        "SFML/Network/Packet.cpp",
        "SFML/Network/TcpSocket.cpp",
        //"SFML/Network/Win32/SocketImpl.cpp",
        "SFML/Network/Socket.cpp",
        "SFML/Network/SocketSelector.cpp",
        "SFML/Network/Ftp.cpp",
        "SFML/Network/TcpListener.cpp",
        "SFML/Network/Http.cpp",
        "SFML/Network/IpAddress.cpp",
    };
    const mod = b.createModule(.{
        .target = target,
        .optimize = .ReleaseFast,
        .link_libc = true,
        .link_libcpp = true,
        .pic = true,
    });
    mod.addCSourceFiles(.{
        .root = sfml.path("src"),
        .files = sfml_sources,
        .flags = &.{"-std=c++20"},
        .language = .cpp,
    });
    mod.addIncludePath(sfml.path("include"));
    mod.addIncludePath(sfml.path("src"));
    mod.addCMacro("SFML_STATIC", "1");
    const lib = b.addStaticLibrary(.{
        .name = "SFML",
        .root_module = mod,
    });
    lib.installHeadersDirectory(sfml.path("include"), "", .{ .include_extensions = &.{ ".hpp", ".inl" } });
    return lib;
}

fn makeDiscordRpc(b: *std.Build, target: std.Build.ResolvedTarget) *std.Build.Step.Compile {
    const discord_rpc = b.dependency("discord_rpc", .{});
    const rapidjson = b.dependency("rapidjson", .{});
    const discord_rpc_sources: []const []const u8 = &.{
        "rpc_connection.cpp",
        "discord_rpc.cpp",
        "connection_unix.cpp",
        //"connection_win.cpp",
        //"dllmain.cpp",
        "discord_register_linux.cpp",
        //"discord_register_win.cpp",
        "serialization.cpp",
    };
    const mod = b.createModule(.{
        .target = target,
        .optimize = .ReleaseFast,
        .link_libc = true,
        .link_libcpp = true,
        .pic = true,
    });
    mod.addCSourceFiles(.{
        .root = discord_rpc.path("src"),
        .files = discord_rpc_sources,
        .flags = &.{"-std=c++11"},
        .language = .cpp,
    });
    mod.addIncludePath(discord_rpc.path("include"));
    mod.addIncludePath(rapidjson.path("include"));
    const lib = b.addStaticLibrary(.{
        .name = "discord-rpc",
        .root_module = mod,
    });
    lib.installHeadersDirectory(discord_rpc.path("include"), "discord-rpc", .{});
    return lib;
}

fn makeX265(b: *std.Build, target: std.Build.ResolvedTarget) *std.Build.Step.Compile {
    const x265 = b.dependency("x265", .{});
    const x265_sources: []const []const u8 = &.{
        "x265.cpp",
        "abrEncApp.cpp",
        "output/output.cpp",
        "output/raw.cpp",
        "output/yuv.cpp",
        "output/y4m.cpp",
        "output/reconplay.cpp",
        "encoder/frameencoder.cpp",
        "encoder/search.cpp",
        "encoder/encoder.cpp",
        "encoder/nal.cpp",
        "encoder/api.cpp",
        "encoder/level.cpp",
        "encoder/slicetype.cpp",
        "encoder/sei.cpp",
        "encoder/bitcost.cpp",
        "encoder/analysis.cpp",
        "encoder/ratecontrol.cpp",
        "encoder/dpb.cpp",
        "encoder/entropy.cpp",
        "encoder/sao.cpp",
        "encoder/motion.cpp",
        "encoder/framefilter.cpp",
        "encoder/reference.cpp",
        "encoder/weightPrediction.cpp",
        "common/md5.cpp",
        "common/pixel.cpp",
        "common/ipfilter.cpp",
        "common/deblock.cpp",
        "common/constants.cpp",
        "common/cudata.cpp",
        "common/threading.cpp",
        "common/frame.cpp",
        "common/threadpool.cpp",
        "common/bitstream.cpp",
        "common/x86/asm-primitives.cpp",
        "common/picyuv.cpp",
        "common/loopfilter.cpp",
        "common/slice.cpp",
        "common/wavefront.cpp",
        "common/primitives.cpp",
        "common/common.cpp",
        "common/lowres.cpp",
        "common/lowpassdct.cpp",
        "common/framedata.cpp",
        "common/version.cpp",
        "common/piclist.cpp",
        "common/param.cpp",
        "common/yuv.cpp",
        "common/predict.cpp",
        "common/scaler.cpp",
        "common/shortyuv.cpp",
        "common/dct.cpp",
        "common/quant.cpp",
        "common/cpu.cpp",
        "common/scalinglist.cpp",
        "common/intrapred.cpp",
        "input/input.cpp",
        "input/yuv.cpp",
        "input/y4m.cpp",
    };
    const mod = b.createModule(.{
        .target = target,
        .optimize = .ReleaseFast,
        .link_libc = true,
        .link_libcpp = true,
        .pic = true,
    });
    mod.addCSourceFiles(.{
        .root = x265.path("source"),
        .files = x265_sources,
        .flags = &.{ "-std=c++11", "-Wno-nontrivial-memaccess" },
        .language = .cpp,
    });
    mod.addIncludePath(x265.path("source"));
    mod.addIncludePath(x265.path("source/common"));
    mod.addIncludePath(x265.path("source/encoder"));
    const generate_headers = b.addWriteFiles();
    _ = generate_headers.add("x265_config.h",
        \\#ifndef X265_CONFIG_H
        \\#define X265_CONFIG_H
        \\#define X265_BUILD 200
        \\#endif
        \\
    );
    mod.addIncludePath(generate_headers.getDirectory());
    mod.addCMacro("X265_ARCH_X86", "1");
    mod.addCMacro("HAVE_INT_TYPES_H", "1");
    mod.addCMacro("HIGH_BIT_DEPTH", "0");
    mod.addCMacro("X265_DEPTH", "8");
    mod.addCMacro("EXPORT_C_API", "1");
    mod.addCMacro("X265_NS", "x265");
    mod.addCMacro("HAVE_STRTOK_R", "1");
    return b.addStaticLibrary(.{
        .name = "x265",
        .root_module = mod,
    });
}

fn generateHeaders(b: *std.Build) std.Build.LazyPath {
    const generate_headers = b.addWriteFiles();
    var git_exit: u8 = undefined;
    const version: []const u8 = b.runAllowFail(
        &.{ "git", "describe", "--tags" },
        &git_exit,
        .Inherit,
    ) catch |err| v: {
        std.log.warn("failed to run git: {s}", .{@errorName(err)});
        std.log.warn("using blank version string", .{});
        break :v "";
    };
    const is_canary = b.option(bool, "canary", "Whether this is a canary build of SAR; default true") orelse true;
    const demo_sign_pubkey = b.option([]const u8, "demo-sign-pubkey", "The public key to use for demo signing.") orelse "";
    const demo_sign_privkey = b.option([]const u8, "demo-sign-privkey", "The private key to use for demo signing.") orelse "";
    const version_hpp_contents = b.fmt(
        \\#define SAR_VERSION "{s}{s}"
        \\{s}
        \\#define SAR_DEMO_SIGN_PUBKEY {{ {s} }}
        \\#define SAR_DEMO_SIGN_PRIVKEY {{ {s} }}
        \\
    , .{
        std.mem.trim(u8, version, " \t\r\n"),
        if (is_canary) "-canary" else "",
        if (is_canary) "#define SAR_DEV_BUILD 1" else "",
        demo_sign_pubkey,
        demo_sign_privkey,
    });
    _ = generate_headers.add("Version.hpp", version_hpp_contents);
    return generate_headers.getDirectory();
}

/// Walks the `src/` directory to find all files whose names end `.cpp`.
/// Returns a slice of their paths relative to `src/`.
fn findSarSources(b: *std.Build) ![]const []const u8 {
    const arena = b.graph.arena;
    var sources: std.ArrayListUnmanaged([]const u8) = .empty;
    var dir = try b.build_root.handle.openDir("src/", .{ .iterate = true });
    defer dir.close();
    var w = try dir.walk(arena);
    defer w.deinit();
    while (try w.next()) |entry| {
        if (entry.kind != .file) continue;
        if (!std.mem.endsWith(u8, entry.path, ".cpp")) continue;
        try sources.append(arena, b.dupe(entry.path));
    }
    return sources.items;
}

const std = @import("std");
