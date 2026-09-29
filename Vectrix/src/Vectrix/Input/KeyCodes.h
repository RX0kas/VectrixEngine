#pragma once

/**
 * @file KeyCodes.h
 * @brief The key codes understood by Input::isKeyPressed and carried by KeyEvent
 * @ingroup input
 *
 * The values match the ones GLFW uses, so a code coming from the windowing layer can be
 * compared to a `VC_KEY_*` macro directly.
 * @see Input::isKeyPressed
 * @see KeyEvent::getKeyCode
 */

// From GLFW
#define VC_KEY_SPACE              32   ///< The space bar
#define VC_KEY_APOSTROPHE         39   ///< The apostrophe key, '
#define VC_KEY_COMMA              44   ///< The comma key, ,
#define VC_KEY_MINUS              45   ///< The minus key, -
#define VC_KEY_PERIOD             46   ///< The period key, .
#define VC_KEY_SLASH              47   ///< The slash key, /
#define VC_KEY_0                  48   ///< The 0 key of the main row
#define VC_KEY_1                  49   ///< The 1 key of the main row
#define VC_KEY_2                  50   ///< The 2 key of the main row
#define VC_KEY_3                  51   ///< The 3 key of the main row
#define VC_KEY_4                  52   ///< The 4 key of the main row
#define VC_KEY_5                  53   ///< The 5 key of the main row
#define VC_KEY_6                  54   ///< The 6 key of the main row
#define VC_KEY_7                  55   ///< The 7 key of the main row
#define VC_KEY_8                  56   ///< The 8 key of the main row
#define VC_KEY_9                  57   ///< The 9 key of the main row
#define VC_KEY_SEMICOLON          59   ///< The semicolon key, ;
#define VC_KEY_EQUAL              61   ///< The equal key, =
#define VC_KEY_A                  65   ///< The A key
#define VC_KEY_B                  66   ///< The B key
#define VC_KEY_C                  67   ///< The C key
#define VC_KEY_D                  68   ///< The D key
#define VC_KEY_E                  69   ///< The E key
#define VC_KEY_F                  70   ///< The F key
#define VC_KEY_G                  71   ///< The G key
#define VC_KEY_H                  72   ///< The H key
#define VC_KEY_I                  73   ///< The I key
#define VC_KEY_J                  74   ///< The J key
#define VC_KEY_K                  75   ///< The K key
#define VC_KEY_L                  76   ///< The L key
#define VC_KEY_M                  77   ///< The M key
#define VC_KEY_N                  78   ///< The N key
#define VC_KEY_O                  79   ///< The O key
#define VC_KEY_P                  80   ///< The P key
#define VC_KEY_Q                  81   ///< The Q key
#define VC_KEY_R                  82   ///< The R key
#define VC_KEY_S                  83   ///< The S key
#define VC_KEY_T                  84   ///< The T key
#define VC_KEY_U                  85   ///< The U key
#define VC_KEY_V                  86   ///< The V key
#define VC_KEY_W                  87   ///< The W key
#define VC_KEY_X                  88   ///< The X key
#define VC_KEY_Y                  89   ///< The Y key
#define VC_KEY_Z                  90   ///< The Z key
#define VC_KEY_LEFT_BRACKET       91   ///< The left square bracket key, [
#define VC_KEY_BACKSLASH          92   ///< The backslash key
#define VC_KEY_RIGHT_BRACKET      93   ///< The right square bracket key, ]
#define VC_KEY_GRAVE_ACCENT       96   ///< The grave accent key, `
#define VC_KEY_WORLD_1            161  ///< The first extra key of a non US keyboard
#define VC_KEY_WORLD_2            162  ///< The second extra key of a non US keyboard

/* Function keys */
#define VC_KEY_ESCAPE             256  ///< The escape key
#define VC_KEY_ENTER              257  ///< The enter key
#define VC_KEY_TAB                258  ///< The tabulation key
#define VC_KEY_BACKSPACE          259  ///< The backspace key
#define VC_KEY_INSERT             260  ///< The insert key
#define VC_KEY_DELETE             261  ///< The delete key
#define VC_KEY_RIGHT              262  ///< The right arrow key
#define VC_KEY_LEFT               263  ///< The left arrow key
#define VC_KEY_DOWN               264  ///< The down arrow key
#define VC_KEY_UP                 265  ///< The up arrow key
#define VC_KEY_PAGE_UP            266  ///< The page up key
#define VC_KEY_PAGE_DOWN          267  ///< The page down key
#define VC_KEY_HOME               268  ///< The home key
#define VC_KEY_END                269  ///< The end key
#define VC_KEY_CAPS_LOCK          280  ///< The caps lock key
#define VC_KEY_SCROLL_LOCK        281  ///< The scroll lock key
#define VC_KEY_NUM_LOCK           282  ///< The num lock key
#define VC_KEY_PRINT_SCREEN       283  ///< The print screen key
#define VC_KEY_PAUSE              284  ///< The pause key
#define VC_KEY_F1                 290  ///< The F1 function key
#define VC_KEY_F2                 291  ///< The F2 function key
#define VC_KEY_F3                 292  ///< The F3 function key
#define VC_KEY_F4                 293  ///< The F4 function key
#define VC_KEY_F5                 294  ///< The F5 function key
#define VC_KEY_F6                 295  ///< The F6 function key
#define VC_KEY_F7                 296  ///< The F7 function key
#define VC_KEY_F8                 297  ///< The F8 function key
#define VC_KEY_F9                 298  ///< The F9 function key
#define VC_KEY_F10                299  ///< The F10 function key
#define VC_KEY_F11                300  ///< The F11 function key
#define VC_KEY_F12                301  ///< The F12 function key
#define VC_KEY_F13                302  ///< The F13 function key
#define VC_KEY_F14                303  ///< The F14 function key
#define VC_KEY_F15                304  ///< The F15 function key
#define VC_KEY_F16                305  ///< The F16 function key
#define VC_KEY_F17                306  ///< The F17 function key
#define VC_KEY_F18                307  ///< The F18 function key
#define VC_KEY_F19                308  ///< The F19 function key
#define VC_KEY_F20                309  ///< The F20 function key
#define VC_KEY_F21                310  ///< The F21 function key
#define VC_KEY_F22                311  ///< The F22 function key
#define VC_KEY_F23                312  ///< The F23 function key
#define VC_KEY_F24                313  ///< The F24 function key
#define VC_KEY_F25                314  ///< The F25 function key
#define VC_KEY_KP_0               320  ///< The 0 key of the numeric keypad
#define VC_KEY_KP_1               321  ///< The 1 key of the numeric keypad
#define VC_KEY_KP_2               322  ///< The 2 key of the numeric keypad
#define VC_KEY_KP_3               323  ///< The 3 key of the numeric keypad
#define VC_KEY_KP_4               324  ///< The 4 key of the numeric keypad
#define VC_KEY_KP_5               325  ///< The 5 key of the numeric keypad
#define VC_KEY_KP_6               326  ///< The 6 key of the numeric keypad
#define VC_KEY_KP_7               327  ///< The 7 key of the numeric keypad
#define VC_KEY_KP_8               328  ///< The 8 key of the numeric keypad
#define VC_KEY_KP_9               329  ///< The 9 key of the numeric keypad
#define VC_KEY_KP_DECIMAL         330  ///< The decimal separator key of the numeric keypad
#define VC_KEY_KP_DIVIDE          331  ///< The divide key of the numeric keypad
#define VC_KEY_KP_MULTIPLY        332  ///< The multiply key of the numeric keypad
#define VC_KEY_KP_SUBTRACT        333  ///< The subtract key of the numeric keypad
#define VC_KEY_KP_ADD             334  ///< The add key of the numeric keypad
#define VC_KEY_KP_ENTER           335  ///< The enter key of the numeric keypad
#define VC_KEY_KP_EQUAL           336  ///< The equal key of the numeric keypad
#define VC_KEY_LEFT_SHIFT         340  ///< The left shift key
#define VC_KEY_LEFT_CONTROL       341  ///< The left control key
#define VC_KEY_LEFT_ALT           342  ///< The left alt key
#define VC_KEY_LEFT_SUPER         343  ///< The left super key, the windows or command key
#define VC_KEY_RIGHT_SHIFT        344  ///< The right shift key
#define VC_KEY_RIGHT_CONTROL      345  ///< The right control key
#define VC_KEY_RIGHT_ALT          346  ///< The right alt key
#define VC_KEY_RIGHT_SUPER        347  ///< The right super key, the windows or command key
#define VC_KEY_MENU               348  ///< The menu key
