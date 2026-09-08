#pragma once

namespace siren {

/// @brief All keyboard buttons.
enum class Key {
    // Basic
    Space = 0,
    Apostrophe,
    Comma,
    Minus,
    Period,
    Slash,
    Semicolon,
    Equal,
    LeftBracket,
    Backslash,
    RightBracket,
    GraveAccent,

    // Numbers
    Num0,
    Num1,
    Num2,
    Num3,
    Num4,
    Num5,
    Num6,
    Num7,
    Num8,
    Num9,

    // Control Keys
    Esc,
    Enter,
    Tab,
    Backspace,
    Insert,
    Delete,
    PageUp,
    PageDown,
    Home,
    End,
    CapsLock,
    ScrollLock,
    NumLock,
    PrintScreen,
    Pause,

    // Keypad
    KpDecimal,
    KpDivide,
    KpMultiply,
    KpSubtract,
    KpAdd,
    KpEnter,
    KpEqual,

    // Modifiers & System
    LShift,
    LControl,
    LAlt,
    LSuper,
    RShift,
    RControl,
    RAlt,
    RSuper,
    Menu,

    ArrowLeft,
    ArrowRight,
    ArrowUp,
    ArrowDown,

    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,

    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    F13,
    F14,
    F15,
    F16,
    F17,
    F18,
    F19,
    F20,
    F21,
    F22,
    F23,
    F24,
    F25,

    KeyLast,
    Max,
};

/// @brief Mouse buttons.
enum class Mouse {
    Left = 0,
    Right,
    Middle,
    Max,
};

/// @brief Cursor behaviour types.
enum class CursorMode {
    Invalid = 0,
    Visible,
    Invisible,
    Locked,
};

} // namespace siren
