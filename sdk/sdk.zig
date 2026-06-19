const std = @import("std");

const Vector2 = extern struct {
    x: f32,
    y: f32,
};

const Vector2i = extern struct {
    x: i32,
    y: i32,
};

pub const Rgba = extern struct {
    r: u8,
    g: u8,
    b: u8,
    a: u8 = 255,

    pub const black = std.mem.zeroes(Rgba);
    pub const white: Rgba = .{ .r = 255, .g = 255, .b = 255 };
    pub const red: Rgba = .{ .r = 255, .g = 0, .b = 0 };
    pub const green: Rgba = .{ .r = 0, .g = 255, .b = 0 };
    pub const blue: Rgba = .{ .r = 0, .g = 0, .b = 255 };
};

pub const Rectangle = extern struct {
    position: Vector2,
    size: Vector2,
};

pub const Player = extern struct {
    position: Vector2,
    velocity: Vector2,
    state: Player.State,

    dash_timer: f32,
    dash_direction: f32,

    hook_target: Vector2,
    hook_rope_length: f32,
    hook_cooldown: f32,

    jump_buffer_timer: f32,

    pub const State = enum(u32) {
        normal,
        dash,
        slam,
        hook,
    };
};

pub const Time = extern struct {
    delta: f32,
};

pub const Map = extern struct {
    grid: [height][width]c_int,

    pub const height = 9;
    pub const width = 30;
};

pub const Camera = extern struct {
    position: Vector2,
    viewport: Vector2i,
};

pub const Input = extern struct {
    jump: Action,
    dash: Action,
    slam: Action,
    hook: Action,
    left: Action,
    right: Action,
    /// Mod-specific input requests.
    custom: [custom_input_max_actions]Action,

    pub const custom_input_max_actions = 256;

    pub const Action = extern struct {
        is_down: bool,
        pressed: bool,
        released: bool,
    };
};

pub const State = extern struct {
    player: Player,
    camera: Camera,
    time: Time,
    map: Map,
    input: Input,

    ok: bool,
};

pub const RenderCommandBuffer = extern struct {
    memory: extern union {
        ptr: [*]u8,
        padding: u64,
    },
    len: u32 = 0,
    capacity: u32,

    pub const Command = union(Type) {
        clear_screen: extern struct {
            type: Type = .clear_screen,
            color: Rgba,
        },
        draw_rectangle: extern struct {
            type: Type = .draw_rectangle,
            rectangle: Rectangle,
            color: Rgba,
        },
        draw_line: struct {
            type: Type = .draw_line,
            start: Vector2,
            end: Vector2,
            color: Rgba,
        },
        draw_debug_text0: struct {
            type: Type = .draw_debug_text0,
            position: Vector2,
            text: [*:0]const u8,
            color: Rgba,
        },

        pub const Type = enum(u32) {
            clear_screen = 1,
            draw_rectangle,
            draw_line,
            draw_debug_text0,
        };
    };

    pub fn push(self: *RenderCommandBuffer, comptime T: type) !*T {
        const aligned_bytes_needed = std.mem.alignForward(u32, @sizeOf(T), 4);

        const has_enough_space = (self.len + aligned_bytes_needed) <= self.capacity;

        if (has_enough_space) {
            const current_aligned_offset = std.mem.alignForward(u32, self.len, 4);
            const address: usize = @as(usize, @intFromPtr(self.memory.ptr)) + current_aligned_offset;

            self.len = current_aligned_offset + aligned_bytes_needed;

            return @ptrFromInt(address);
        }

        return error.OutOfMemory;
    }

    pub fn draw(self: *RenderCommandBuffer, command: Command) !void {
        inline for (std.meta.fields(Command)) |field| {
            const Inner: type = @FieldType(Command, field.name);
            if (std.mem.eql(u8, @tagName(command), field.name)) {
                const inner = try self.push(Inner);
                inner.* = @field(command, field.name);
            }
        }
        // switch (command) {
        //     inline else => |target| {
        //         const Inner: type = @TypeOf(target);
        //         const inner = try self.push(Inner);
        //         inner.* = @field(command, @tagName(std.meta.activeTag(command)));
        //     },
        // }
    }
};

extern fn host_log(message: [*:0]const u8) void;
pub const log = host_log;
extern fn host_set_metadata(name: [*:0]const u8, version: [*:0]const u8, summary: [*:0]const u8) void;
pub const setMetadata = host_set_metadata;

/// The returned index represents the index it was assigned to in Input.custom array.
/// It is recommended to prefix `name` with the name of the mod to avoid any conflicts. For example:
/// `example_mod_do_a_cool_thing`
extern fn host_register_action(name: [*:0]const u8) i32;
pub const registerAction = host_register_action;
/// Register `action` with `RegisterAction` first before calling this.
extern fn host_register_default_key(action: [*:0]const u8, key: [*:0]const u8) void;
pub const registerDefaultKey = host_register_default_key;

pub var state: State = undefined;
pub var buffer_backing_memory: [128 * 1024]u8 = @splat(0);
pub var buffer: RenderCommandBuffer = .{
    .memory = .{ .ptr = &buffer_backing_memory },
    .capacity = buffer_backing_memory.len,
};

export fn guest_get_state() *State {
    return &state;
}

export fn guest_get_buffer() *RenderCommandBuffer {
    return &buffer;
}
