#pragma once

namespace siren {
/// @todo: to_string functionality for debugging/logging purposes
/// @todo: Some mapping of actions to groups? Like "moveLeft" -> {A_KEY, X_BUTTON}

/**
 * @brief Represents the state of modifier keys during a key press.
 */
struct Modifiers {
    /** @brief Either the left or right shift key. */
    bool shift : 1;
    /** @brief Either the left or control key. */
    bool control : 1;
    /** @brief Either the left or alt key. */
    bool alt : 1;
    /** @brief Either the windows ket or command key. */
    bool super : 1;
    /** @brief Caps lock is toggled on. */
    bool caps_lock : 1;
    /** @brief Num lock is toggled on. */
    bool num_lock : 1;
};

/**
 * @brief All keyboard buttons.
 */
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
    Max // Used for array sizing
};

/**
 * @brief Mouse buttons.
 */
enum class Mouse {
    Left = 0,
    Right,
    Middle,
    Max, // Do not use
};

/**
 * @brief Cursor behaviour types.
 */
enum class CursorMode {
    /** @brief Indicates some invalid state. May occur when querying too soon etc. */
    Invalid = 0,
    /** @brief Regular visible cursor. */
    Visible,
    /** @brief Regular visible cursor. */
    Invisible,
    /** @brief Cursor is invisible and cannot leave the window and is centered each frame. */
    Locked,
};

} // namespace siren
