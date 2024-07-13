#include <SDL.h>

export module constVar;

export enum class funcFlag
{
    none,
    dim1,
    dim2,
    dim3,
    scalarField,
    vectorField,
    tensorField,
};

export namespace UNI
{
    constexpr int NUL = 0;   // Null char
    constexpr int SOH = 1;   // Start of Heading
    constexpr int STX = 2;   // Start of Text
    constexpr int ETX = 3;   // End of Text
    constexpr int EOT = 4;   // End of Transmission
    constexpr int ENQ = 5;   // Enquiry
    constexpr int ACK = 6;   // Acknowledgment
    constexpr int BEL = 7;   // Bell
    constexpr int BACKSPACE = 8;    // Back Space
    constexpr int TAB = 9;   // Horizontal Tab
    constexpr int LF = 10;   // Line Feed
    constexpr int VT = 11;   // Vertical Tab
    constexpr int FF = 12;   // Form Feed
    constexpr int CR = 13;   // Carriage Return
    constexpr int SO = 14;   // Shift Out / X-On
    constexpr int SI = 15;   // Shift In / X-Off
    constexpr int DLE = 16;  // Data Line Escape
    constexpr int DC1 = 17;  // Device Control 1 (oft. XON)
    constexpr int DC2 = 18;  // Device Control 2
    constexpr int DC3 = 19;  // Device Control 3 (oft. XOFF)
    constexpr int DC4 = 20;  // Device Control 4
    constexpr int NAK = 21;  // Negative Acknowledgement
    constexpr int SYN = 22;  // Synchronous Idle
    constexpr int ETB = 23;  // End of Transmit Block
    constexpr int CAN = 24;  // Cancel
    constexpr int EM = 25;   // End of Medium
    constexpr int SUB = 26;  // Substitute
    constexpr int ESC = 27;  // Escape
    constexpr int FS = 28;   // File Separator
    constexpr int GS = 29;   // Group Separator
    constexpr int RS = 30;   // Record Separator
    constexpr int US = 31;   // Unit Separator
    constexpr int SPACE = 32;
    constexpr int EXCLAMATION_MARK = 33;  // '!'
    constexpr int DOUBLE_QUOTES = 34;  // '"'
    constexpr int HASH = 35;  // '#'
    constexpr int DOLLAR_SIGN = 36;  // '$'
    constexpr int PERCENT_SIGN = 37;  // '%'
    constexpr int AMPERSAND = 38;  // '&'
    constexpr int APOSTROPHE = 39;  // '''
    constexpr int LEFT_PARENTHESIS = 40;  // '('
    constexpr int RIGHT_PARENTHESIS = 41;  // ')'
    constexpr int ASTERISK = 42;  // '*'
    constexpr int PLUS_SIGN = 43;  // '+'
    constexpr int COMMA = 44;  // ','
    constexpr int MINUS_SIGN = 45;  // '-'
    constexpr int PERIOD = 46;  // '.'
    constexpr int SLASH = 47;  // '/'
    constexpr int ZERO = 48;
    constexpr int ONE = 49;
    constexpr int TWO = 50;
    constexpr int THREE = 51;
    constexpr int FOUR = 52;
    constexpr int FIVE = 53;
    constexpr int SIX = 54;
    constexpr int SEVEN = 55;
    constexpr int EIGHT = 56;
    constexpr int NINE = 57;
    constexpr int COLON = 58;  // ':'
    constexpr int SEMICOLON = 59;  // ';'
    constexpr int LESS_THAN_SIGN = 60;  // '<'
    constexpr int EQUAL_SIGN = 61;  // '='
    constexpr int GREATER_THAN_SIGN = 62;  // '>'
    constexpr int QUESTION_MARK = 63;  // '?'
    constexpr int AT_SIGN = 64;  // '@'
    constexpr int A = 65;
    constexpr int B = 66;
    constexpr int C = 67;
    constexpr int D = 68;
    constexpr int E = 69;
    constexpr int F = 70;
    constexpr int G = 71;
    constexpr int H = 72;
    constexpr int I = 73;
    constexpr int J = 74;
    constexpr int K = 75;
    constexpr int L = 76;
    constexpr int M = 77;
    constexpr int N = 78;
    constexpr int O = 79;
    constexpr int P = 80;
    constexpr int Q = 81;
    constexpr int R = 82;
    constexpr int S = 83;
    constexpr int T = 84;
    constexpr int U = 85;
    constexpr int V = 86;
    constexpr int W = 87;
    constexpr int X = 88;
    constexpr int Y = 89;
    constexpr int Z = 90;
    constexpr int LEFT_SQUARE_BRACKET = 91;  // '['
    constexpr int BACKSLASH = 92;  // '\'
    constexpr int RIGHT_SQUARE_BRACKET = 93;  // ']'
    constexpr int CARET = 94;  // '^'
    constexpr int UNDERSCORE = 95;  // '_'
    constexpr int GRAVE_ACCENT = 96;  // '`'
    constexpr int a = 97;
    constexpr int b = 98;
    constexpr int c = 99;
    constexpr int d = 100;
    constexpr int e = 101;
    constexpr int f = 102;
    constexpr int g = 103;
    constexpr int h = 104;
    constexpr int i = 105;
    constexpr int j = 106;
    constexpr int k = 107;
    constexpr int l = 108;
    constexpr int m = 109;
    constexpr int n = 110;
    constexpr int o = 111;
    constexpr int p = 112;
    constexpr int q = 113;
    constexpr int r = 114;
    constexpr int s = 115;
    constexpr int t = 116;
    constexpr int u = 117;
    constexpr int v = 118;
    constexpr int w = 119;
    constexpr int x = 120;
    constexpr int y = 121;
    constexpr int z = 122;
    constexpr int LEFT_CURLY_BRACKET = 123;  // '{'
    constexpr int VERTICAL_BAR = 124;  // '|'
    constexpr int RIGHT_CURLY_BRACKET = 125;  // '}'
    constexpr int TILDE = 126;  // '~'
    constexpr int DEL = 127;   // Delete
};

export namespace col
{
    constexpr SDL_Color black = { 0x00, 0x00, 0x00 };
    constexpr SDL_Color yellow = { 0xff,0xff,0x00 };
    constexpr SDL_Color brown = { 0x5c,0x33,0x17 };
    constexpr SDL_Color gray = { 0x63,0x63,0x63 };
    constexpr SDL_Color green = { 0x00,0x6e,0x00 };
    constexpr SDL_Color blueberry = { 0x64,0x64,0xff };
    constexpr SDL_Color red = { 0xf9,0x29,0x29 };
    constexpr SDL_Color white = { 0xff,0xff,0xff };
    constexpr SDL_Color lightGray = { 0x96,0x96,0x96 };
    constexpr SDL_Color blue = { 0x21,0x4a,0xea };
    constexpr SDL_Color yellowGreen = { 0x3a, 0xf5, 0x43 };
    constexpr SDL_Color monaLisa = { 0xff,0x96,0x96 };
    constexpr SDL_Color bondiBlue = { 0x00,0x96,0xb4 };
    constexpr SDL_Color hotPink = { 0x8b,0x3a,0x62 };
    constexpr SDL_Color pink = { 0xfe,0x00,0xfe };
    constexpr SDL_Color skyBlue = { 0x00,0xf0,0xff };
    constexpr SDL_Color blueDart = { 0x4e,0x8e,0xd2 };
    constexpr SDL_Color orange = { 0xf2, 0x65, 0x22 };
    constexpr SDL_Color cyan = { 0x00,0xa3,0xd2 };
};

export namespace lowCol
{
    constexpr SDL_Color black = { 0x00,0x00,0x00 };
    constexpr SDL_Color white = { 0xff,0xff,0xff };
    constexpr SDL_Color red = { 0xd0,0x3f,0x3f };
    constexpr SDL_Color orange = { 0xd0,0x7a,0x3f };
    constexpr SDL_Color yellow = { 0xd0,0xc3,0x3f };
    constexpr SDL_Color green = { 0x75,0xd0,0x3f };
    constexpr SDL_Color mint = { 0x3f,0xd0,0x7f };
    constexpr SDL_Color skyBlue = { 0x3f,0xba,0xd0 };
    constexpr SDL_Color deepBlue = { 0x20,0x50,0xa8 };
    constexpr SDL_Color blue = { 0x2b,0x81,0xe8 };
    constexpr SDL_Color purple = { 0x43,0x3e,0x8e };
    constexpr SDL_Color pink = { 0xbe,0x3f,0xd0 };
    constexpr SDL_Color crimson = { 0xd0,0x3f,0x89 };
};


export constexpr double BOX_SIZE = 10.849215;
export constexpr int DENSITY_GRID = 20;