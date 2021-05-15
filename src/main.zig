const std = @import("std");

export fn zig_main_hook() void {
    std.debug.maybeEnableSegfaultHandler();
}
