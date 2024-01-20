const std = @import("std");
const W = std.unicode.utf8ToUtf16LeStringLiteral;
const windows = @cImport({
    @cInclude("windows.h");
});
extern fn init_entry([*c]const u8, [*c]const u8) callconv(.C) c_int;


pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer if (gpa.deinit() != .ok) @panic("memory leak!");
    const al = gpa.allocator();
    var dependencies_file: ?[]u8 = undefined;
    defer if (dependencies_file) |f| al.free(f);
    var buffer: [300]u8 = undefined;
    const cwd = try std.os.getcwd(&buffer);
    if (std.fs.cwd().openFileW(W("dependencies"), .{.mode = .read_only}) catch null) |f| {
        f.close();
        dependencies_file = try std.fmt.allocPrint(al, "{s}/dependencies", .{cwd});
    } else dependencies_file = null;
    var args = try std.process.argsWithAllocator(al);
    defer args.deinit();
    _ = args.next();
    var dll_path: [*c]const u8 = null;
    var hide_console_window = true;
    while (args.next()) |arg| {
        if (std.mem.startsWith(u8, arg, "--")) {
            if (std.mem.startsWith(u8, arg[2..], "pydll=")) {
                if (arg.len < 9) @panic("invalid input");
                dll_path = arg[8..];
            } else if (std.mem.startsWith(u8, arg[2..], "show-console")) {
                hide_console_window = false;
            }
        }
    }
    if (dependencies_file) |file| {
        var path = try al.alloc(u8, file.len + 1);
        defer al.free(path);
        path[file.len] = 0;
        for (0.., file) |i, v| {
            path[i] = v;
        }
        _ = init_entry(dll_path, path[0..file.len:0]);
        // rename
        {
            var old_path = al.alloc(u16, file.len + 1) catch null;
            var new_path = al.alloc(u16, file.len + 2) catch null;
            if (new_path) |new| {
                defer al.free(new);
                for (0.., cwd) |i, v| {
                    new[i] = @as(u16, v);
                }
                new[cwd.len] = @as(u16, '/');
                for (cwd.len + 1.., "_dependencies") |i, v| {
                    new[i] = @as(u16, v);
                }
                new[file.len + 1] = 0;
                if (old_path) |old| {
                    defer al.free(old);
                    for (0.., file) |i, v| {
                        old[i] = @as(u16, v);
                    }
                    old[file.len] = 0;
                    _ = std.os.renameW(old[0..file.len:0], new[0..file.len + 1:0]) catch null;
                }
            }
        }
    } else {
        if (hide_console_window) {
            const hwnd = windows.GetConsoleWindow();
            // const hwnd = windows.FindWindowA("ConsoleWindowClass", null);
            if (hwnd) |_hwnd| {
                _ = windows.ShowWindow(_hwnd, windows.SW_HIDE);
            }
        }
        _ = init_entry(dll_path, null);
    }
}

test "simple test" {
    var list = std.ArrayList(i32).init(std.testing.allocator);
    defer list.deinit(); // try commenting this out and see if zig detects the memory leak!
    try list.append(42);
    try std.testing.expectEqual(@as(i32, 42), list.pop());
}
