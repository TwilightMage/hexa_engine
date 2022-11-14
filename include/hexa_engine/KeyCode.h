﻿#pragma once

enum class KeyCode
{
    Unknown = 0,
    BackSpace = 8,
    Tab = 9,
    Return = 13,
    Escape = 27,
    Space = 32,
    Exclaim = 33,
    DoubleQuote = 34,
    Hash = 35,
    Dollar = 36,
    Percent = 37,
    Ampersand = 38,
    Quote = 39,
    LeftParen = 40,
    RightParen = 41,
    Asterisk = 42,
    Plus = 43,
    Comma = 44,
    Minus = 45,
    Period = 46,
    Slash = 47,
    Alpha0 = 48,
    Alpha1 = 49,
    Alpha2 = 50,
    Alpha3 = 51,
    Alpha4 = 52,
    Alpha5 = 53,
    Alpha6 = 54,
    Alpha7 = 55,
    Alpha8 = 56,
    Alpha9 = 57,
    Colon = 58,
    Semicolon = 59,
    Less = 60,
    Equals = 61,
    Greater = 62,
    Question = 63,
    At = 64,
    LeftBracket = 91,
    BackSlash = 92,
    RightBracket = 93,
    Caret = 94,
    Underscore = 95,
    BackQuote = 96,
    A = 97,
    B = 98,
    C = 99,
    D = 100,
    E = 101,
    F = 102,
    G = 103,
    H = 104,
    I = 105,
    J = 106,
    K = 107,
    L = 108,
    M = 109,
    N = 110,
    O = 111,
    P = 112,
    Q = 113,
    R = 114,
    S = 115,
    T = 116,
    U = 117,
    V = 118,
    W = 119,
    X = 120,
    Y = 121,
    Z = 122,
    Delete = 127,
    CapsLock = 1073741881,
    F1 = 1073741882,
    F2 = 1073741883,
    F3 = 1073741884,
    F4 = 1073741885,
    F5 = 1073741886,
    F6 = 1073741887,
    F7 = 1073741888,
    F8 = 1073741889,
    F9 = 1073741890,
    F10 = 1073741891,
    F11 = 1073741892,
    F12 = 1073741893,
    PrintScreen = 1073741894,
    ScrollLock = 1073741895,
    Pause = 1073741896,
    Insert = 1073741897,
    Home = 1073741898,
    PageUp = 1073741899,
    End = 1073741901,
    PageDown = 1073741902,
    Right = 1073741903,
    Left = 1073741904,
    Down = 1073741905,
    Up = 1073741906,
    NumLockClear = 1073741907,
    KP_Divide = 1073741908,
    KP_Multiply = 1073741909,
    KP_Minus = 1073741910,
    KP_Plus = 1073741911,
    KP_Enter = 1073741912,
    KP_1 = 1073741913,
    KP_2 = 1073741914,
    KP_3 = 1073741915,
    KP_4 = 1073741916,
    KP_5 = 1073741917,
    KP_6 = 1073741918,
    KP_7 = 1073741919,
    KP_8 = 1073741920,
    KP_9 = 1073741921,
    KP_0 = 1073741922,
    KP_Period = 1073741923,
    Application = 1073741925,
    Power = 1073741926,
    KP_Equals = 1073741927,
    F13 = 1073741928,
    F14 = 1073741929,
    F15 = 1073741930,
    F16 = 1073741931,
    F17 = 1073741932,
    F18 = 1073741933,
    F19 = 1073741934,
    F20 = 1073741935,
    F21 = 1073741936,
    F22 = 1073741937,
    F23 = 1073741938,
    F24 = 1073741939,
    Execute = 1073741940,
    Help = 1073741941,
    Menu = 1073741942,
    Select = 1073741943,
    Stop = 1073741944,
    Again = 1073741945,
    Undo = 1073741946,
    Cut = 1073741947,
    Copy = 1073741948,
    Paste = 1073741949,
    Find = 1073741950,
    Mute = 1073741951,
    VolumeUp = 1073741952,
    VolumeDown = 1073741953,
    KP_Comma = 1073741957,
    // KP_EQUALSAS400 = 1073741958,
    AltErase = 1073741977,
    SysReq = 1073741978,
    Cancel = 1073741979,
    Clear = 1073741980,
    Prior = 1073741981,
    Return2 = 1073741982,
    Separator = 1073741983,
    Out = 1073741984,
    Oper = 1073741985,
    ClearAgain = 1073741986,
    // CRSEL = 1073741987,
    // EXSEL = 1073741988,
    // KP_00 = 1073742000,
    // KP_000 = 1073742001,
    ThousandsSeparator = 1073742002,
    DecimalSeparator = 1073742003,
    CurrencyUnit = 1073742004,
    CurrencySubUnit = 1073742005,
    KP_LeftParen = 1073742006,
    KP_RightParen = 1073742007,
    KP_LeftBrace = 1073742008,
    KP_RightBrace = 1073742009,
    KP_Tab = 1073742010,
    KP_BackSpace = 1073742011,
    KP_A = 1073742012,
    KP_B = 1073742013,
    KP_C = 1073742014,
    KP_D = 1073742015,
    KP_E = 1073742016,
    KP_F = 1073742017,
    KP_XOR = 1073742018,
    KP_Power = 1073742019,
    KP_Percent = 1073742020,
    KP_Less = 1073742021,
    KP_Greater = 1073742022,
    KP_Ampersand = 1073742023,
    KP_DoubleAmpersand = 1073742024,
    KP_VerticalBar = 1073742025,
    KP_DoubleVerticalBar = 1073742026,
    KP_Colon = 1073742027,
    KP_Hash = 1073742028,
    KP_Space = 1073742029,
    KP_At = 1073742030,
    KP_Exclaim = 1073742031,
    KP_MemStore = 1073742032,
    KP_MemRecall = 1073742033,
    KP_MemClear = 1073742034,
    KP_MemAdd = 1073742035,
    KP_MemSubtract = 1073742036,
    KP_MemMultiply = 1073742037,
    KP_MemDivide = 1073742038,
    KP_PlusMinus = 1073742039,
    KP_Clear = 1073742040,
    KP_ClearEntry = 1073742041,
    KP_Binary = 1073742042,
    KP_Octal = 1073742043,
    KP_Decimal = 1073742044,
    KP_Hexadecimal = 1073742045,
    LeftCtrl = 1073742048,
    LeftShift = 1073742049,
    LeftAlt = 1073742050,
    LeftGUI = 1073742051,
    RightCtrl = 1073742052,
    RightShift = 1073742053,
    RightAlt = 1073742054,
    RightGUI = 1073742055,
    Mode = 1073742081,
    AudioNext = 1073742082,
    AudioPrev = 1073742083,
    AudioStop = 1073742084,
    AudioPlay = 1073742085,
    AudioMute = 1073742086,
    MediaSelect = 1073742087,
    WWW = 1073742088,
    Mail = 1073742089,
    Calculator = 1073742090,
    Computer = 1073742091,
    AC_Search = 1073742092,
    AC_Home = 1073742093,
    AC_Back = 1073742094,
    AC_Forward = 1073742095,
    AC_Stop = 1073742096,
    AC_Refresh = 1073742097,
    AC_Bookmarks = 1073742098,
    BrightnessDown = 1073742099,
    BrightnessUp = 1073742100,
    DisplaySwitch = 1073742101,
    KBDILLUMTOGGLE = 1073742102,
    KBDILLUMDOWN = 1073742103,
    KBDILLUMUP = 1073742104,
    Eject = 1073742105,
    Sleep = 1073742106
};