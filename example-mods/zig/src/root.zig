const std = @import("std");
const sdk = @import("sdk");

export fn guest_initialize() void {
    sdk.log("guest says hello!2");
    sdk.setMetadata("zig mod", "622.7.4.2.0", "All your codebas is belong to us");
    sdk.log("zig zig zig zig zig");
}

export fn guest_update() void {
    update(&sdk.state, &sdk.buffer) catch |err| {
        var writer_buffer: [128]u8 = undefined;
        var writer: std.Io.Writer = .fixed(&writer_buffer);
        writer.writeAll("error: ") catch {};
        writer.writeAll(@errorName(err)) catch {};
        writer.writeByte(0) catch {};
        sdk.log(@ptrCast(writer.buffered()));
    };
}

pub fn update(state: *sdk.State, buffer: *sdk.RenderCommandBuffer) !void {
    _ = state;

    try buffer.draw(.{ .draw_rectangle = .{
        .color = .green,
        .rectangle = .{
            .position = .{ .x = 10, .y = 2 },
            .size = .{ .x = 10, .y = 10 },
        },
    } });
}
