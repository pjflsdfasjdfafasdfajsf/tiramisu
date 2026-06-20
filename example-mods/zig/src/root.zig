const std = @import("std");
const sdk = @import("sdk");

pub const std_options: std.Options = .{
    .logFn = sdk.log,
};

comptime {
    _ = sdk;
}

var miguel: i32 = -1;

pub fn init() !void {
    const metadata: sdk.Metadata = .{ .name = "zig mod", .summary = "a nice zig mod" };
    try metadata.set();

    miguel = sdk.registerAction("miguel");
    sdk.registerDefaultKey("miguel", "f");
}

pub fn update(state: *sdk.State, buffer: *sdk.RenderCommandBuffer) !void {
    try buffer.draw(.{ .draw_rectangle = .{
        .color = .green,
        .rectangle = .{
            .position = .{ .x = 10, .y = 2 },
            .size = .{ .x = 10, .y = 10 },
        },
    } });

    if (state.input.custom[@intCast(miguel)].pressed) {
        std.log.info("{any}", .{state.player.position});
    }
}
