// Alpaca (with C).

pub const logging = struct {
    // @ alpakka/src/headers/logging.h
    // void info(char *msg, ...);
    pub extern fn info(msg: [*:0]const u8, ...) void;
};
