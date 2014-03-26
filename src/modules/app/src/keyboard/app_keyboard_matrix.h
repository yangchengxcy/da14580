/**
 ****************************************************************************************
 *
 * @file app_keyboard_matrix.h
 *
 * @brief HID Keyboard initialization and helper functions.
 *
 * Copyright (C) 2013. Dialog Semiconductor Ltd, unpublished work. This computer 
 * program includes Confidential, Proprietary Information and is a Trade Secret of 
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited 
 * unless authorized in writing. All Rights Reserved.
 *
 * <bluetooth.support@diasemi.com> and contributors.
 *
 ****************************************************************************************
 */

#ifndef APP_KEYBOARD_MATRIX_H_
#define APP_KEYBOARD_MATRIX_H_


//#define ASCII_DEBUG  
#ifdef ASCII_DEBUG

// Quick and dirty conversion table to allow debugging
__STATIC const uint8 kbd_hut_to_ascii_table[128] = 
{
	'?', '?', '?', '?', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',  // 0x00
	'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '1', '2',  // 0x10
	'3', '4', '5', '6', '7', '8', '9', '0', '\n', 27,  8, '\t', ' ', '-', '=', '[',  // 0x20
	']', '\\','#', ';', '\'','`', ',', '.', '/', '^', '1', '2', '3', '4', '5', '6',  // 0x30
	'7', '8', '9', 'A', 'B', 'C', 'P', 'S', 'P', 'I', 'H', 'U', 127, 'E', 'D', 'R',  // 0x40
	'L', 'D', 'U', 'N', '/', '*', '-', '+', '\n','1', '2', '3', '4', '5', '6', '7',  // 0x50
	'8', '9', '0', '.', '\\','A', 'P', '=', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',  // 0x60
	'L', 'M', 'N', 'O', 'E', 'H', 'M', 'S', 'S', 'A', 'U', 'C', 'C', 'P', 'F', 'M'   // 0x70
};

// note: must be power of 2
#define MAX_KEYBUFFER (16)
__STATIC uint8 kbd_buffer_ascii[MAX_KEYBUFFER];
__STATIC uint16 kbd_keybuffer_idx;
#endif




#if MATRIX_SETUP == 0
/*****************************************************************************************************************************************
 *
 * Rev.1 or Rev.2 DKs (keyboard #1)
 *
 *****************************************************************************************************************************************/
 
#if (BLE_ALT_PAIR)
# define BLE_ALT_I2C_ON     (0)
#endif

#if (KEYBOARD_MEASURE_EXT_SLP)
# define KB_EXTSLP          (0xF4F3)
#else
# define KB_EXTSLP          (0)
#endif

// rows are outputs in this case, because there are fewer rows than columns, making scanning faster
#define KBD_NR_INPUTS   (14)
#define KBD_NR_OUTPUTS  (8)

// column input connection table
// FIXME: a. if inputs were in ascending order then scanning could be implemented in a more efficient and faster way
//        b. they should have been declared as a 32-bit bitmap 0..7: P00-P07, 10..15: P10-P15, 20..29: P20-P29 to save memory space
__STATIC const uint8 kbd_input_ports[KBD_NR_INPUTS] = {
//   port      column
	 0x20,  // column 0
	 0x21,  // column 1
	 0x22,  // column 2
#if DEEP_SLEEP_ENABLED
     0x44,  // column 3 P1_4?
#else    
	 0x44,  // column 3 inactive
#endif    
	 0x23,  // column 4
#if DEEP_SLEEP_ENABLED
     0x45,  // column 5
#warning "Decide what to do with this pin..."    
#else    
	 0x45,  // column 5 inactive
#endif    
	 0x24,  // column 6
	 0x25,  // column 7
	 0x26,  // column 8
	 0x27,  // column 9
	 0x10,  // column 10
	 0x11,  // column 11
	 0x12,  // column 12
	 0x13   // column 13
};

// for a little cycle optimization below
#define P0_HAS_INPUT (0)
#define P1_HAS_INPUT (1)
#define P2_HAS_INPUT (1)
#define P3_HAS_INPUT (0)

// row output connection table
__STATIC const uint8 kbd_output_ports[KBD_NR_OUTPUTS] = {
//   port      row
#if defined(CFG_PRINTF) 
	 0x40,  // row 0 disabled (p00 == uart tx for printf's)
#else
	 0x00,  // row 0
#endif
	 0x28,  // row 1
	 0x29,  // row 2
	 0x03,  // row 3
	 0x04,  // row 4
	 0x05,  // row 5
	 0x06,  // row 6
#if DEEP_SLEEP_ENABLED
     0x07,  // row 7 P1_5
#warning "P1_5 is used as output!"    
#else    
	 0x07   // row 7
#endif    
};

// extra sets for 'hidden modifiers', e.g. the 'Fn' key
#define KBD_NR_SETS (2)

// The key map.
// 00xx means regular key
// FCxx means modifier key.
// F8xx means FN Modifier.
// F4xx means special function.
__STATIC const uint16 kbd_keymap[KBD_NR_SETS][KBD_NR_OUTPUTS][KBD_NR_INPUTS] = 
{
  { // No Fn keys pressed
	// 0    // 1    // 2    // 3    // 4    // 5    // 6    // 7    // 8    // 9    // 10   // 11   // 12   // 13 
	{ 0xFC02, 0xFC20, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x000D, 0x0036, 0x000E, 0x002E, 0x0009, 0x000A }, // 0
	{ 0x0000, 0x0000, 0xFC04, 0xFC40, 0x0000, 0x0000, 0x0000, 0x000C, 0x0033, 0x002F, 0x0037, 0x0016, 0x0000, 0x001D }, // 1
	{ 0x0000, 0x0000, 0x0000, 0x0000, 0xFC01, 0xFC10, 0x0000, 0x004F, 0x0013, 0x0025, 0x0004, 0x0018, 0x0000, 0x000F }, // 2
	{ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFC08, 0x0038, 0x0000, 0x0007, 0x0000, 0x0010, /*0xF801*/0x0050, 0x001E }, // 3
	{ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xF801, 0x0023, 0x001C, 0x0039, 0x0000, 0x0014, 0x0000, 0x0030 }, // 4
	{ 0x0031, 0x000B, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x002B, 0x001B, 0x0017, 0x0029, 0x002D, 0x001F, 0x0021 }, // 5
	{ 0x0012, 0x0020, 0x0006, 0x001A, 0x0028, 0x0019, 0x004C, 0x0000, 0x0000, 0x0000, 0x0026, /*0xFC02*/0x0051, 0x0024, 0x0015 }, // 6
	{ 0x0000, 0x0000, 0x0022, 0x0034, 0x0005, 0x0000, 0x002C, 0x0008, 0x0000, 0x0052, 0x0000, 0x0011, 0x0027, 0x002A }  // 7
  },
  { // Fn 0x01 Pressed
	// 0    // 1    // 2    // 3    // 4    // 5    // 6    // 7    // 8    // 9    // 10   // 11   // 12   // 13 
	{ 0xFC02, 0xFC20, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0080/*0x0000*/, 0x0000, 0x0000 }, // 0
	{ 0x0000, 0x0000, 0xFC04, 0xFC40, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // 1
	{ 0x0000, 0x0000, 0x0000, 0x0000, 0xFC01, 0xFC10, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // 2
	{ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFC08, 0x0000, 0x0000, 0x0000, 0x0000, 0xF401, 0xF801, 0x0000 }, // 3
	{ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xF801, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // 4
	{ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0081, 0x0000, 0x0000 }, // 5
	{ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // 6
	{ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, KB_EXTSLP, 0x0000, 0x0000, 0x0000, 0x0000, 0xF400, 0x0000, 0xF404 }  // 7
  }
};






#elif MATRIX_SETUP == 1
/*****************************************************************************************************************************************
 *
 * Rev.3 DK with QFN48 (keyboard #1)
 *
 *****************************************************************************************************************************************/
 
#if (BLE_ALT_PAIR)
# define BLE_ALT_I2C_ON     (0)
#endif

#if (KEYBOARD_MEASURE_EXT_SLP)
# define KB_EXTSLP          (0xF4F3)
#else
# define KB_EXTSLP          (0)
#endif

#define KBD_NR_INPUTS   (14)
#define KBD_NR_OUTPUTS  (8)

__STATIC const uint8 kbd_input_ports[KBD_NR_INPUTS] = {
//  port      column
    0x20,  // column 0
    0x21,  // column 1
    0x22,  // column 2
    0x36,  // column 3 
    0x23,  // column 4
    0x37,  // column 5
    0x24,  // column 6
    0x25,  // column 7
    0x26,  // column 8
    0x27,  // column 9
    0x10,  // column 10
    0x11,  // column 11
    0x12,  // column 12
    0x13   // column 13
};

// for a little cycle optimization below
#define P0_HAS_INPUT (0)
#define P1_HAS_INPUT (1)
#define P2_HAS_INPUT (1)
#define P3_HAS_INPUT (1)

// row output connection table
__STATIC const uint8 kbd_output_ports[KBD_NR_OUTPUTS] = {
//  port      row
#ifdef CFG_PRINTF
    0x40,  // row 0 - inactive
#else    
    0x00,  // row 0
#endif
    0x28,  // row 1
    0x29,  // row 2
    0x03,  // row 3
    0x04,  // row 4
    0x05,  // row 5
    0x06,  // row 6
    0x07   // row 7
};

// extra sets for 'hidden modifiers', e.g. the 'Fn' key
#define KBD_NR_SETS (2)

// The key map.
// 00xx means regular key
// FCxx means modifier key.
// F8xx means FN Modifier.
// F4xx means special function.
__STATIC const uint16 kbd_keymap[KBD_NR_SETS][KBD_NR_OUTPUTS][KBD_NR_INPUTS] = 
{
  {
/*    No Fn key(s) pressed
      0         1         2         3         4         5         6         7         8         9         10        11        12        13        
      --------------------------------------------------------------------------------------------------------------------------------------------
      L-Shift   R-Shift   #         #         #         #         #         #         j         ,         k         =         f         g         
      #         #         L-Alt     R-Alt     #         #         #         i         ;         [         .         s         #         z         
      #         #         #         #         L-Ctrl    R-Ctrl    #         Rgt arrow p         8         a         u         #         L         
      #         #         #         #         #         #         L-Win     /         #         d         #         m         Lft arrow 1         
      #         #         #         #         #         #         Fn        6         y         Caps      #         q         #         ]         
      \         h         #         #         #         #         #         Tab       x         t         Esc       -         2         4         
      o         3         c         w         Enter     v         Del       #         #         #         9         Dn arrow  7         r         
      #         #         5         '         b         #         Space     e         #         Up arrow  #         n         0         Backspc         
      --------------------------------------------------------------------------------------------------------------------------------------------*/     
	{ 0xFC02,   0xFC20,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x000D,   0x0036,   0x000E,   0x002E,   0x0009,   0x000A }, // 0
	{ 0x0000,   0x0000,   0xFC04,   0xFC40,   0x0000,   0x0000,   0x0000,   0x000C,   0x0033,   0x002F,   0x0037,   0x0016,   0x0000,   0x001D }, // 1
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0xFC01,   0xFC10,   0x0000,   0x004F,   0x0013,   0x0025,   0x0004,   0x0018,   0x0000,   0x000F }, // 2
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0xFC08,   0x0038,   0x0000,   0x0007,   0x0000,   0x0010,   0x0050,   0x001E }, // 3
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0xF801,   0x0023,   0x001C,   0x0039,   0x0000,   0x0014,   0x0000,   0x0030 }, // 4
	{ 0x0031,   0x000B,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x002B,   0x001B,   0x0017,   0x0029,   0x002D,   0x001F,   0x0021 }, // 5
	{ 0x0012,   0x0020,   0x0006,   0x001A,   0x0028,   0x0019,   0x004C,   0x0000,   0x0000,   0x0000,   0x0026,   0x0051,   0x0024,   0x0015 }, // 6
	{ 0x0000,   0x0000,   0x0022,   0x0034,   0x0005,   0x0000,   0x002C,   0x0008,   0x0000,   0x0052,   0x0000,   0x0011,   0x0027,   0x002A }  // 7
  },
  {
/*     Fn key 0x01 Pressed
      0         1         2         3         4         5         6         7         8         9         10        11        12        13        
      --------------------------------------------------------------------------------------------------------------------------------------------
      #         #         #         #         #         #         #         #         #         #         #         Vol up    #         #         
      #         #         #         WWW-Home  #         #         #         F8        #         F11       #         #         #         #         
      #         #         #         #         #         App       #         End       F10       Play      #         F7        #         #         
      #         #         #         #         #         #         #         #         #         #         #         #         Home      1 <-------------------        
      #         #         #         #         #         #         Fn        ???       F6        #         #         F1        #         F12         
      #         #         #         #         #         #         #         #         #         F5        `         Vol dn    ???       ???         
      PrintScr  WWW Sear  #         F2        #         #         WWW-Back  #         #         #         Nxt Trk   Pg Dn     Prv Trk   F4         
      #         #         ???       #         #         #         EXT_SLEEP F3        #         Pg Up     #         #         Mute      Insert         
      --------------------------------------------------------------------------------------------------------------------------------------------*/     
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0xF406,   0x0000,   0x0000 }, // 0
	{ 0x0000,   0x0000,   0x0000,   0xF415,   0x0000,   0x0000,   0x0000,   0x0041,   0x0000,   0x0044,   0x0000,   0x0000,   0x0000,   0x0000 }, // 1
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0065,   0x0000,   0x004D,   0x0043,   0xF404,   0x0000,   0x0040,   0x0000,   0x0000 }, // 2
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x004A,   0xF4F0 }, // 3
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0xF801,   0x0000,   0x003F,   0x0000,   0x0000,   0x003A,   0x0000,   0x0045 }, // 4
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x003E,   0x0035,   0xF407,   0x0000,   0x0000 }, // 5
	{ 0x0046,   0xF414,   0x0000,   0x003B,   0x0000,   0x0000,   0xF416,   0x0000,   0x0000,   0x0000,   0xF400,   0x004E,   0xF401,   0x003D }, // 6
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   KB_EXTSLP,0x003C,   0x0000,   0x004B,   0x0000,   0x0000,   0xF405,   0x0049 }  // 7
  }
};






#elif MATRIX_SETUP == 2
/*****************************************************************************************************************************************
 *
 * Rev.3 DK with QFN48 (keyboard #2)
 *
 *****************************************************************************************************************************************/

#if (BLE_ALT_PAIR)
# define BLE_ALT_I2C_ON     (0)
#endif

#if (KEYBOARD_MEASURE_EXT_SLP)
# define KB_EXTSLP          (0xF4F3)
#else
# define KB_EXTSLP          (0)
#endif

#define KBD_NR_INPUTS   (18)
#define KBD_NR_OUTPUTS  (8)

__STATIC const uint8 kbd_input_ports[KBD_NR_INPUTS] = {
//  port       column
    0x05,   // column 0
    0x04,   // column 1
    0x13,   // column 2
    0x12,   // column 3 
    0x11,   // column 4
    0x10,   // column 5
    0x27,   // column 6
    0x26,   // column 7
    0x25,   // column 8
    0x24,   // column 9
    0x37,   // column 10
    0x23,   // column 11
    0x36,   // column 12
    0x22,   // column 13
    0x21,   // column 14
    0x20,   // column 15
    0x31,   // column 16
    0x32    // column 17
};

// for a little cycle optimization below
#define P0_HAS_INPUT (1)
#define P1_HAS_INPUT (1)
#define P2_HAS_INPUT (1)
#define P3_HAS_INPUT (1)

// row output connection table
__STATIC const uint8 kbd_output_ports[KBD_NR_OUTPUTS] = {
//  port       row
    0x34,   // row 0
    0x33,   // row 1
    0x35,   // row 2
    0x06,   // row 3
    0x29,   // row 4
    0x28,   // row 5
    0x07,   // row 6
    0x30    // row 7
};

// extra sets for 'hidden modifiers', e.g. the 'Fn' key
#define KBD_NR_SETS (2)

// The key map.
// 00xx means regular key
// FCxx means modifier key.
// F8xx means FN Modifier.
// F4xy means special function (x = no of byte in the report, y no of bit set).
__STATIC const uint16 kbd_keymap[KBD_NR_SETS][KBD_NR_OUTPUTS][KBD_NR_INPUTS] = 
{
  { 
/*    No Fn key(s) pressed
      0         1         2         3         4         5           6         7         8         9         10        11        12        13        14        15        16        17
      ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
      #WWW Fav  #Med Sel  L-Ctrl    #WWW Ref  #Wake up  Home        Pg Up     Del       Insert    F9        -         F8        =         6         5         F2        F1        `
      #N Trk    #Calc               #WWW Sea  Print     End         Pg Down   F11       F12       F10       0         9         8         7         4         3         2         1
      App       Vol+      Vol-      Mute      Scroll    + (num)     9 (num)   7 (num)   8 (num)   #         p         o         i         u         r         e         Caps      q
      #Pre-Trk  #Sleep    #LVT      L-Shift   #My Comp  #           6 (num)   4 (num)   5 (num)   Backspc   [         F7        ]         y         t         F3        w         Tab
      L-Win     Fn        #WWW Fwd  #WWW Back #         Enter (num) 3 (num)   1 (num)   2 (num)   \         ;         L         k         j         f         d         #         a
      Play      #WWW Home                     L-Alt     Up arrow    , (num)   #         0 (num)   F5        '         #         F6        h         g         F4        s         Esc
      #Stop     R-Win     R-Ctrl    R-Shift   #Power    Pause       * (num)   Num Lk    / (num)   Enter     \         .         ,         m         v         c         x         z
      #e-mail   #Fn-wrong                     R-Alt     Left arrow  - (num)   Dn arrow  Rgt arrow Space     /         #         #         n         b         #R-K      #L-K      #
      ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/     
	{ 0x0000,   0x0000,   0xFC01,   0x0000,   0x0000,   0x004A,     0x004B,   0x004C,   0x0049,   0x0042,   0x002D,   0x0041,   0x002E,   0x0023,   0x0022,   0x003B,   0x003A,   0x0035 }, // 0
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0046,   0x004D,     0x004E,   0x0044,   0x0045,   0x0043,   0x0027,   0x0026,   0x0025,   0x0024,   0x0021,   0x0020,   0x001F,   0x001E }, // 1
	{ 0x0065,   0xF406,   0xF407,   0xF405,   0x0047,   0x0057,     0x0061,   0x005F,   0x0060,   0x0000,   0x0013,   0x0012,   0x000C,   0x0018,   0x0015,   0x0008,   0x0039,   0x0014 }, // 2
	{ 0x0000,   0x0000,   0x0000,   0xFC02,   0x0000,   0x0000,     0x005E,   0x005C,   0x005D,   0x002A,   0x002F,   0x0040,   0x0030,   0x001C,   0x0017,   0x003C,   0x001A,   0x002B }, // 3
	{ 0xFC08,   0xF801,   0x0000,   0x0000,   0x0000,   0x0058,     0x005B,   0x0059,   0x005A,   0x0064,   0x0033,   0x000F,   0x000E,   0x000D,   0x0009,   0x0007,   0x0000,   0x0004 }, // 4
	{ 0xF404,   0x0000,   0x0000,   0x0000,   0xFC04,   0x0052,     0x0063,   0x0000,   0x0062,   0x003E,   0x0034,   0x0000,   0x003F,   0x000B,   0x000A,   0x003D,   0x0016,   0x0029 }, // 5
	{ 0x0000,   0xFC80,   0xFC10,   0xFC20,   0x0000,   0x0048,     0x0055,   0x0053,   0x0054,   0x0028,   0x0064,   0x0037,   0x0036,   0x0010,   0x0019,   0x0006,   0x001B,   0x001D }, // 6
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0xFC40,   0x0050,     0x0056,   0x0051,   0x004F,   0x002C,   0x0038,   0x0000,   0x0000,   0x0011,   0x0005,   0x0000,   0x0000,   0x0000 }  // 7
  },
  {
/*    Fn key 0x01 pressed
      0         1         2         3         4         5           6         7         8         9         10        11        12        13        14        15        16        17
      ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
      #         #         #         #         #         #           #         #         #         #         #         Nxt Trk   #         #         #         #         WWW Home  #         
      #         #         #         #         Insert    #           #         Break     Scroll Lk #         #         #         #         #         #         #         #         #         
      #         #         #         #         #         #           #         #         #         #         #         #         #         #         #         #         #         #         
      #         #         #         #         #         #           #         #         #         #         #         Prv Trk   #         #         #         #         #         #         
      #         Fn        #         #         #         #           #         #         #         #         #         #         #         #         #         #         #         #         
      #         #         #         #         #         #           #         #         #         #         #         #         Stop      #         #         Eject     #         #         
      #         #         #         #         #         #           #         #         #         #         #         #         #         #         #         #         #         #         
      #         #         #         #         #         #           #         #         #         EXT_SLEEP #         #         #         #         #         #         #         #         
      ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/     
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,     0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0xF400,   0x0000,   0x0000,   0x0000,   0x0000,   0xF415,   0x0000,  }, // 0
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0049,   0x0000,     0x0000,   0x0048,   0x0047,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,  }, // 1
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,     0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,  }, // 2
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,     0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0xF401,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,  }, // 3
	{ 0x0000,   0xF801,   0x0000,   0x0000,   0x0000,   0x0000,     0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,  }, // 4
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,     0x0000,   0x0000,   0x0000,   0xF404,   0x0000,   0x0000,   0xF402,   0x0000,   0x0000,   0xF403,   0x0000,   0x0000,  }, // 5
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,     0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,  }, // 6
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,     0x0000,   0x0000,   0x0000,   KB_EXTSLP,0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,  }  // 7
  }
};




#elif MATRIX_SETUP == 3
/*****************************************************************************************************************************************
 *
 * Rev.3 DK with QFN48 (keyboard #3)
 *
 *****************************************************************************************************************************************/

#if (BLE_ALT_PAIR)
# define BLE_ALT_I2C_ON     (1)
# define PAIR (0xF4F1)
# define CLRP  (0xF4F2)
#else
# define BLE_ALT_I2C_ON     (0)
# define PAIR (0x0000)
# define CLRP  (0x0000)
#endif

#if (KEYBOARD_MEASURE_EXT_SLP)
# define KB_EXTSLP          (0xF4F3)
#else
# define KB_EXTSLP          (0)
#endif

#define KBD_NR_INPUTS   (16)
#define KBD_NR_OUTPUTS  (8)

__STATIC const uint8 kbd_input_ports[KBD_NR_INPUTS] = {
//  port       column
    0x10,   // column 0
    0x20,   // column 1
    0x27,   // column 2
    0x26,   // column 3 
    0x12,   // column 4
    0x11,   // column 5
    0x29,   // column 6
    0x24,   // column 7
    0x05,   // column 8
    0x23,   // column 9
    0x04,   // column 10
    0x21,   // column 11
    0x22,   // column 12
    0x13,   // column 13
    0x02,   // column 14
    0x03    // column 15
};

// for a little cycle optimization below
#define P0_HAS_INPUT (1)
#define P1_HAS_INPUT (1)
#define P2_HAS_INPUT (1)
#define P3_HAS_INPUT (0)

// row output connection table
__STATIC const uint8 kbd_output_ports[KBD_NR_OUTPUTS] = {
//  port       row
    0x31,   // row 0
    0x34,   // row 1
    0x36,   // row 2
    0x30,   // row 3
    0x32,   // row 4
    0x33,   // row 5
    0x35,   // row 6
    0x37    // row 7
};

// extra sets for 'hidden modifiers', e.g. the 'Fn' key
#define KBD_NR_SETS (2)

// The key map.
// 00xx means regular key
// FCxx means modifier key.
// F8xx means FN Modifier.
// F4xy means special function (x = no of byte in the report, y no of bit set).
__STATIC const uint16 kbd_keymap[KBD_NR_SETS][KBD_NR_OUTPUTS][KBD_NR_INPUTS] = 
{
  { 
/*    No Fn key(s) pressed
      0         1         2         3         4         5         6         7         8         9         10        11        12        13        14        15        
      -------------------------------------------------------------------------------------------------------------------------------------------------------------
      Esc       F16?      Rgt arrow Vol+      Mute      Del       n         b         v         Search    x         z         #         L-Alt     #         #      
      #         #???      Vol-      Dn arrow  Play      Insert    h         g         f         c         Print     #???      L-Win     #         #         #      
      #         #???      #         Enter     L arrow   Backspace 5         t         e         r         w         q         #         #         L-Ctrl    L-Shift      
      Fn        Apps      \         #         #???      Up arrow  y         4         3         2         1         #         s         a         #         #      
      #         #???      9         8         i         6         0         =         #         #Share??? Home      #         #         #         R-Ctrl    R-Shift      
      #         R-Win     p         o         k         u         -         ]         PrintScr  #Devices? End       Caps      #         #         #         #      
      #         #???      ;         L         7         j         [         #???      Scroll Lk #Settings?Pg Up     Tab       #         R-Alt     #         #      
      #         Space     /         .         ,         m         '         #???      Pause     #         Pg down   d         `         #???      #         #      
      -------------------------------------------------------------------------------------------------------------------------------------------------------------*/     
	{ 0x0029,   0x006B,   0x004F,   0xF406,   0xF405,   0x004C,   0x0011,   0x0005,   0x0019,   0xF414,   0x001B,   0x001D,   0x0000,   0xFC04,   0x0000,   0x0000,  }, // 0
	{ 0x0000,   0x0000,   0xF407,   0x0051,   0xF404,   0x0049,   0x000B,   0x000A,   0x0009,   0x0006,   0x0046,   0x0000,   0xFC08,   0x0000,   0x0000,   0x0000,  }, // 1
	{ 0x0000,   0x0000,   0x0000,   0x0028,   0x0050,   0x002A,   0x0022,   0x0017,   0x0008,   0x0015,   0x001A,   0x0014,   0x0000,   0x0000,   0xFC01,   0xFC02,  }, // 2
	{ 0xF801,   0x0065,   0x0031,   0x0000,   0x0000,   0x0052,   0x001C,   0x0021,   0x0020,   0x001F,   0x001E,   0x0000,   0x0016,   0x0004,   0x0000,   0x0000,  }, // 3
	{ 0x0000,   0x0000,   0x0026,   0x0025,   0x000C,   0x0023,   0x0027,   0x002E,   0x0000,   0x0000,   0x004A,   0x0000,   0x0000,   0x0000,   0xFC10,   0xFC20,  }, // 4
	{ 0x0000,   0xFC80,   0x0013,   0x0012,   0x000E,   0x0018,   0x002D,   0x0030,   0x0046,   0x0000,   0x004D,   0x0039,   0x0000,   0x0000,   0x0000,   0x0000,  }, // 5
	{ 0x0000,   0x0000,   0x0033,   0x000F,   0x0024,   0x000D,   0x002F,   0x0000,   0x0047,   0x0000,   0x004B,   0x002B,   0x0000,   0xFC40,   0x0000,   0x0000,  }, // 6
	{ 0x0000,   0x002C,   0x0038,   0x0037,   0x0036,   0x0010,   0x0034,   0x0000,   0x0048,   0x0000,   0x004E,   0x0007,   0x0035,   0x0000,   0x0000,   0x0000,  }  // 7
  },
  {
/*    Fn key 0x01 pressed
      0         1         2         3         4         5         6         7         8         9         10        11        12        13        14        15        
      -------------------------------------------------------------------------------------------------------------------------------------------------------------
      #         #         #         F4        F2        #         #         #         #         F5        #         #         #         #         #         #      
      #         #         F3        #         F1        #         #         #         #         Clr pair  F9        #         #         #         #         #      
      #         #         #         #         #         #         #         #         #         #         #         #         #         #         #         #      
      Fn        #         #         #         #         #         #         #         #         #         #         #         #         #         #         #      
      #         #         #         #         #         #         #         #         #         F6        F10       #         #         #         #         #      
      #         #         Pair      #         #         #         #         #         #         F7        F11       #         #         #         #         #      
      #         #         #         #         #         #         #         #         #         F8        F12       #         #         #         #         #      
      #         EXT_SLEEP #         #         #         #         #         #         #         #         #         #         #         #         #         #      
      -------------------------------------------------------------------------------------------------------------------------------------------------------------*/     
	{ 0x0000,   0x0000,   0x0000,   0x003D,   0x003B,   0x0000,   0x0000,   0x0000,   0x0000,   0x003E,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,  }, // 0
	{ 0x0000,   0x0000,   0x003C,   0x0000,   0x003A,   0x0000,   0x0000,   0x0000,   0x0000,   CLRP,     0x0042,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,  }, // 1
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,  }, // 2
	{ 0xF801,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,  }, // 3
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x003F,   0x0043,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,  }, // 4
	{ 0x0000,   0x0000,   PAIR,     0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0040,   0x0044,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,  }, // 5
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0041,   0x0045,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,  }, // 6
	{ 0x0000,   KB_EXTSLP,0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,  }  // 7
  }
};






#elif MATRIX_SETUP == 4
/*****************************************************************************************************************************************
 *
 * Rev.3 DK with QFN48 (keyboard #1 with EEPROM)
 *
 *****************************************************************************************************************************************/
 
#if (BLE_ALT_PAIR)
# define BLE_ALT_I2C_ON     (1)
# define PAIR (0xF4F1)
# define CLRP  (0xF4F2)
#else
# define BLE_ALT_I2C_ON     (0)
# define PAIR (0x0000)
# define CLRP  (0x0000)
#endif

#if (KEYBOARD_MEASURE_EXT_SLP)
# define KB_EXTSLP          (0xF4F3)
#else
# define KB_EXTSLP          (0)
#endif

#define KBD_NR_INPUTS   (14)
#define KBD_NR_OUTPUTS  (8)

__STATIC const uint8 kbd_input_ports[KBD_NR_INPUTS] = {
//  port      column
    0x20,  // column 0
    0x21,  // column 1
    0x22,  // column 2
    0x36,  // column 3 
    0x23,  // column 4
    0x37,  // column 5
    0x24,  // column 6
    0x25,  // column 7
    0x26,  // column 8
    0x27,  // column 9
    0x10,  // column 10
    0x11,  // column 11
    0x12,  // column 12
    0x13   // column 13
};

// for a little cycle optimization below
#define P0_HAS_INPUT (0)
#define P1_HAS_INPUT (1)
#define P2_HAS_INPUT (1)
#define P3_HAS_INPUT (1)

// row output connection table
__STATIC const uint8 kbd_output_ports[KBD_NR_OUTPUTS] = {
//   port      row
#ifdef CFG_PRINTF
    0x40,  // row 0 - inactive
#else    
    0x00,  // row 0
#endif
    0x28,  // row 1
    0x29,  // row 2
    0x03,  // row 3
    0x04,  // row 4
    0x05,  // row 5
    0x31,  // row 6
    0x32   // row 7
};

// extra sets for 'hidden modifiers', e.g. the 'Fn' key
#define KBD_NR_SETS (2)

// The key map.
// 00xx means regular key
// FCxx means modifier key.
// F8xx means FN Modifier.
// F4xx means special function.
__STATIC const uint16 kbd_keymap[KBD_NR_SETS][KBD_NR_OUTPUTS][KBD_NR_INPUTS] = 
{
  {
/*    No Fn key(s) pressed
      0         1         2         3         4         5         6         7         8         9         10        11        12        13        
      --------------------------------------------------------------------------------------------------------------------------------------------
      L-Shift   R-Shift   #         #         #         #         #         #         j         ,         k         =         f         g         
      #         #         L-Alt     R-Alt     #         #         #         i         ;         [         .         s         #         z         
      #         #         #         #         L-Ctrl    R-Ctrl    #         Rgt arrow p         8         a         u         #         L         
      #         #         #         #         #         #         L-Win     /         #         d         #         m         Lft arrow 1         
      #         #         #         #         #         #         Fn        6         y         Caps      #         q         #         ]         
      \         h         #         #         #         #         #         Tab       x         t         Esc       -         2         4         
      o         3         c         w         Enter     v         Del       #         #         #         9         Dn arrow  7         r         
      #         #         5         '         b         #         Space     e         #         Up arrow  #         n         0         Backspc         
      --------------------------------------------------------------------------------------------------------------------------------------------*/     
	{ 0xFC02,   0xFC20,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x000D,   0x0036,   0x000E,   0x002E,   0x0009,   0x000A }, // 0
	{ 0x0000,   0x0000,   0xFC04,   0xFC40,   0x0000,   0x0000,   0x0000,   0x000C,   0x0033,   0x002F,   0x0037,   0x0016,   0x0000,   0x001D }, // 1
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0xFC01,   0xFC10,   0x0000,   0x004F,   0x0013,   0x0025,   0x0004,   0x0018,   0x0000,   0x000F }, // 2
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0xFC08,   0x0038,   0x0000,   0x0007,   0x0000,   0x0010,   0x0050,   0x001E }, // 3
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0xF801,   0x0023,   0x001C,   0x0039,   0x0000,   0x0014,   0x0000,   0x0030 }, // 4
	{ 0x0031,   0x000B,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x002B,   0x001B,   0x0017,   0x0029,   0x002D,   0x001F,   0x0021 }, // 5
	{ 0x0012,   0x0020,   0x0006,   0x001A,   0x0028,   0x0019,   0x004C,   0x0000,   0x0000,   0x0000,   0x0026,   0x0051,   0x0024,   0x0015 }, // 6
	{ 0x0000,   0x0000,   0x0022,   0x0034,   0x0005,   0x0000,   0x002C,   0x0008,   0x0000,   0x0052,   0x0000,   0x0011,   0x0027,   0x002A }  // 7
  },
  {
/*     Fn key 0x01 Pressed
      0         1         2         3         4         5         6         7         8         9         10        11        12        13        
      --------------------------------------------------------------------------------------------------------------------------------------------
      #         #         #         #         #         #         #         #         #         #         #         Vol up    #         #         
      #         #         #         WWW-Home  #         #         #         F8        #         F11       #         #         #         #         
      #         #         #         #         #         App       #         End       F10       Play      Pair      F7        #         #         
      #         #         #         #         #         #         #         #         #         #         #         #         Home      1 <-------------------        
      #         #         #         #         #         #         Fn        ???       F6        #         #         F1        #         F12         
      #         #         #         #         #         #         #         #         #         F5        `         Vol dn    ???       ???         
      PrintScr  WWW Sear  Clr pair  F2        #         #         WWW-Back  #         #         #         Nxt Trk   Pg Dn     Prv Trk   F4         
      #         #         ???       #         #         #         EXT_SLEEP F3        #         Pg Up     #         #         Mute      Insert         
      --------------------------------------------------------------------------------------------------------------------------------------------*/     
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0xF406,   0x0000,   0x0000 }, // 0
	{ 0x0000,   0x0000,   0x0000,   0xF415,   0x0000,   0x0000,   0x0000,   0x0041,   0x0000,   0x0044,   0x0000,   0x0000,   0x0000,   0x0000 }, // 1
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0065,   0x0000,   0x004D,   0x0043,   0xF404,   PAIR,     0x0040,   0x0000,   0x0000 }, // 2
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x004A,   0xF4F0 }, // 3
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0xF801,   0x0000,   0x003F,   0x0000,   0x0000,   0x003A,   0x0000,   0x0045 }, // 4
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x003E,   0x0035,   0xF407,   0x0000,   0x0000 }, // 5
	{ 0x0046,   0xF414,   CLRP,     0x003B,   0x0000,   0x0000,   0xF416,   0x0000,   0x0000,   0x0000,   0xF400,   0x004E,   0xF401,   0x003D }, // 6
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   KB_EXTSLP,0x003C,   0x0000,   0x004B,   0x0000,   0x0000,   0xF405,   0x0049 }  // 7
  }
};







#elif MATRIX_SETUP == 5
/*****************************************************************************************************************************************
 *
 * Rev.3 DK with QFN48 (keyboard #1 for Apple products)
 *
 *****************************************************************************************************************************************/

#if (BLE_ALT_PAIR)
# define BLE_ALT_I2C_ON     (0)
#endif

#if (KEYBOARD_MEASURE_EXT_SLP)
# define KB_EXTSLP          (0xF4F3)
#else
# define KB_EXTSLP          (0)
#endif

#define KBD_NR_INPUTS   (14)
#define KBD_NR_OUTPUTS  (8)

__STATIC const uint8 kbd_input_ports[KBD_NR_INPUTS] = {
//  port      column
    0x20,  // column 0
    0x21,  // column 1
    0x22,  // column 2
    0x36,  // column 3 
    0x23,  // column 4
    0x37,  // column 5
    0x24,  // column 6
    0x25,  // column 7
    0x26,  // column 8
    0x27,  // column 9
    0x10,  // column 10
    0x11,  // column 11
    0x12,  // column 12
    0x13   // column 13
};

// for a little cycle optimization below
#define P0_HAS_INPUT (0)
#define P1_HAS_INPUT (1)
#define P2_HAS_INPUT (1)
#define P3_HAS_INPUT (1)

// row output connection table
__STATIC const uint8 kbd_output_ports[KBD_NR_OUTPUTS] = {
//  port      row
#ifdef CFG_PRINTF
    0x40,  // row 0 - inactive
#else    
    0x00,  // row 0
#endif
    0x28,  // row 1
    0x29,  // row 2
    0x03,  // row 3
    0x04,  // row 4
    0x05,  // row 5
    0x06,  // row 6
    0x07   // row 7
};

// extra sets for 'hidden modifiers', e.g. the 'Fn' key
#define KBD_NR_SETS (2)

// The key map.
// 00xx means regular key
// FCxx means modifier key.
// F8xx means FN Modifier.
// F4xx means special function.
__STATIC const uint16 kbd_keymap[KBD_NR_SETS][KBD_NR_OUTPUTS][KBD_NR_INPUTS] = 
{
  {
/*    No Fn key(s) pressed
      0         1         2         3         4         5         6         7         8         9         10        11        12        13        
      --------------------------------------------------------------------------------------------------------------------------------------------
      L-Shift   R-Shift   #         #         #         #         #         #         j         ,         k         =         f         g         
      #         #         L-Option  R-Cmd     #         #         #         i         ;         [         .         s         #         z         
      #         #         #         #         Fn        #         #         Rgt arrow p         8         a         u         #         L         
      #         #         #         #         #         L-Cmd     #         /         #         d         #         m         Lft arrow 1         
      #         #         #         #         #         #         L-Ctrl    6         y         Caps      \         q         '         ]         
      #         h         #         #         #         #         #         Tab       x         t         Esc       -         2         4         
      o         3         c         w         Enter     v         #         R-Option  #         #         9         Dn arrow  7         r         
      #         #         5         '         b         #         Space     e         WWW Sear  Up arrow  #         n         0         Backspc         
      --------------------------------------------------------------------------------------------------------------------------------------------*/     
	{ 0xFC02,   0xFC20,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x000D,   0x0036,   0x000E,   0x002E,   0x0009,   0x000A }, // 0
	{ 0x0000,   0x0000,   0xFC04,   0xFC80,   0x0000,   0x0000,   0x0000,   0x000C,   0x0033,   0x002F,   0x0037,   0x0016,   0x0000,   0x001D }, // 1
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0xF801,   0x0000,   0x0000,   0x004F,   0x0013,   0x0025,   0x0004,   0x0018,   0x0000,   0x000F }, // 2
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0xFC08,   0x0000,   0x0038,   0x0000,   0x0007,   0x0000,   0x0010,   0x0050,   0x001E }, // 3
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0xFC01,   0x0023,   0x001C,   0x0039,   0x0031,   0x0014,   0x0035,   0x0030 }, // 4
	{ 0x0000,   0x000B,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x002B,   0x001B,   0x0017,   0x0029,   0x002D,   0x001F,   0x0021 }, // 5
	{ 0x0012,   0x0020,   0x0006,   0x001A,   0x0028,   0x0019,   0x0000,   0xFC40,   0x0000,   0x0000,   0x0026,   0x0051,   0x0024,   0x0015 }, // 6
	{ 0x0000,   0x0000,   0x0022,   0x0034,   0x0005,   0x0000,   0x002C,   0x0008,   0xF414,   0x0052,   0x0000,   0x0011,   0x0027,   0x002A }  // 7
  },
  {
/*     Fn key 0x01 Pressed
      0         1         2         3         4         5         6         7         8         9         10        11        12        13        
      --------------------------------------------------------------------------------------------------------------------------------------------
      #         #         #         #         #         #         #         #         #         #         #         Vol up    #         #         
      #         #         #         #         #         #         #         #         #         #         #         #         #         #         
      #         #         #         #         Fn        #         #         #         #         Play      #         #         #         #         
      #         #         #         #         #         #         #         #         #         #         #         #         #         #???        
      #         #         #         #         #         #         #         #???      #         #         #         #         #         #         
      #         #         #         #         #         #         #         #         #         #         #         Vol dn    #???      #         
      #         #         #         #         #         #         #         #         #         #         Nxt Trk   #         Prv Trk   #         
      #         #         #???      #         #         #         EXT_SLEEP #         #         #         #         #         Mute      #???         
      --------------------------------------------------------------------------------------------------------------------------------------------*/     
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0xF406,   0x0000,   0x0000 }, // 0
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000 }, // 1
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0xF801,   0x0000,   0x0000,   0x0000,   0x0000,   0xF404,   0x0000,   0x0000,   0x0000,   0x0000 }, // 2
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000 }, // 3
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000 }, // 4
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0xF407,   0x0000,   0x0000 }, // 5
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0xF400,   0x0000,   0xF401,   0x0000 }, // 6
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   KB_EXTSLP,0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0xF405,   0x0000 }  // 7
  }
};






#elif MATRIX_SETUP == 6
/*****************************************************************************************************************************************
 *
 * Rev.3 DK with QFN48 (keyboard #1 for Apple products)
 *
 *****************************************************************************************************************************************/
 
#if (BLE_ALT_PAIR)
# define BLE_ALT_I2C_ON     (1)
# define PAIR               (0xF4F1)
# define CLRP               (0xF4F2)
#else
# define BLE_ALT_I2C_ON     (0)
# define PAIR               (0x0000)
# define CLRP               (0x0000)
#endif

#if (KEYBOARD_MEASURE_EXT_SLP)
# define KB_EXTSLP          (0xF4F3)
#else
# define KB_EXTSLP          (0)
#endif

#define KBD_NR_INPUTS   (14)
#define KBD_NR_OUTPUTS  (8)

__STATIC const uint8 kbd_input_ports[KBD_NR_INPUTS] = {
//  port      column
    0x20,  // column 0
    0x21,  // column 1
    0x22,  // column 2
    0x36,  // column 3 
    0x23,  // column 4
    0x37,  // column 5
    0x24,  // column 6
    0x25,  // column 7
    0x26,  // column 8
    0x27,  // column 9
    0x10,  // column 10
    0x11,  // column 11
    0x12,  // column 12
    0x13   // column 13
};

// for a little cycle optimization below
#define P0_HAS_INPUT (0)
#define P1_HAS_INPUT (1)
#define P2_HAS_INPUT (1)
#define P3_HAS_INPUT (1)

// row output connection table
__STATIC const uint8 kbd_output_ports[KBD_NR_OUTPUTS] = {
//  port      row
#ifdef CFG_PRINTF
    0x40,  // row 0 - inactive
#else    
    0x00,  // row 0
#endif
    0x28,  // row 1
    0x29,  // row 2
    0x03,  // row 3
    0x04,  // row 4
    0x05,  // row 5
    0x31,  // row 6
    0x32   // row 7
};

// extra sets for 'hidden modifiers', e.g. the 'Fn' key
#define KBD_NR_SETS (2)

// The key map.
// 00xx means regular key
// FCxx means modifier key.
// F8xx means FN Modifier.
// F4xx means special function.
__STATIC const uint16 kbd_keymap[KBD_NR_SETS][KBD_NR_OUTPUTS][KBD_NR_INPUTS] = 
{
  {
/*    No Fn key(s) pressed
      0         1         2         3         4         5         6         7         8         9         10        11        12        13        
      --------------------------------------------------------------------------------------------------------------------------------------------
      L-Shift   R-Shift   #         #         #         #         #         #         j         ,         k         =         f         g         
      #         #         L-Option  R-Cmd     #         #         #         i         ;         [         .         s         #         z         
      #         #         #         #         Fn        #         #         Rgt arrow p         8         a         u         #         L         
      #         #         #         #         #         L-Cmd     #         /         #         d         #         m         Lft arrow 1         
      #         #         #         #         #         #         L-Ctrl    6         y         Caps      \         q         '         ]         
      #         h         #         #         #         #         #         Tab       x         t         Esc       -         2         4         
      o         3         c         w         Enter     v         #         R-Option  #         #         9         Dn arrow  7         r         
      #         #         5         '         b         #         Space     e         WWW Sear  Up arrow  #         n         0         Backspc         
      --------------------------------------------------------------------------------------------------------------------------------------------*/     
	{ 0xFC02,   0xFC20,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x000D,   0x0036,   0x000E,   0x002E,   0x0009,   0x000A }, // 0
	{ 0x0000,   0x0000,   0xFC04,   0xFC80,   0x0000,   0x0000,   0x0000,   0x000C,   0x0033,   0x002F,   0x0037,   0x0016,   0x0000,   0x001D }, // 1
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0xF801,   0x0000,   0x0000,   0x004F,   0x0013,   0x0025,   0x0004,   0x0018,   0x0000,   0x000F }, // 2
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0xFC08,   0x0000,   0x0038,   0x0000,   0x0007,   0x0000,   0x0010,   0x0050,   0x001E }, // 3
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0xFC01,   0x0023,   0x001C,   0x0039,   0x0031,   0x0014,   0x0035,   0x0030 }, // 4
	{ 0x0000,   0x000B,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x002B,   0x001B,   0x0017,   0x0029,   0x002D,   0x001F,   0x0021 }, // 5
	{ 0x0012,   0x0020,   0x0006,   0x001A,   0x0028,   0x0019,   0x0000,   0xFC40,   0x0000,   0x0000,   0x0026,   0x0051,   0x0024,   0x0015 }, // 6
	{ 0x0000,   0x0000,   0x0022,   0x0034,   0x0005,   0x0000,   0x002C,   0x0008,   0xF414,   0x0052,   0x0000,   0x0011,   0x0027,   0x002A }  // 7
  },
  {
/*     Fn key 0x01 Pressed
      0         1         2         3         4         5         6         7         8         9         10        11        12        13        
      --------------------------------------------------------------------------------------------------------------------------------------------
      #         #         #         #         #         #         #         #         #         #         #         Vol up    #         #         
      #         #         #         #         #         #         #         #         #         #         #         #         #         #         
      #         #         #         #         Fn        #         #         #         Pair      Play      #         #         #         #         
      #         #         #         #         #         #         #         #         #         #         #         #         #         #???        
      #         #         #         #         #         #         #         #???      #         #         #         #         #         #         
      #         #         #         #         #         #         #         #         #         #         #         Vol dn    #???      #         
      #         #         Clr pair  #         #         #         #         #         #         #         Nxt Trk   #         Prv Trk   #         
      #         #         #???      #         #         #         EXT_SLEEP #         #         #         #         #         Mute      #???         
      --------------------------------------------------------------------------------------------------------------------------------------------*/     
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0xF406,   0x0000,   0x0000 }, // 0
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000 }, // 1
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0xF801,   0x0000,   0x0000,   0x0000,   PAIR,     0xF404,   0x0000,   0x0000,   0x0000,   0x0000 }, // 2
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000 }, // 3
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000 }, // 4
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0xF407,   0x0000,   0x0000 }, // 5
	{ 0x0000,   0x0000,   CLRP,     0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0xF400,   0x0000,   0xF401,   0x0000 }, // 6
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   KB_EXTSLP,0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0xF405,   0x0000 }  // 7
  }
};






#elif MATRIX_SETUP == 7
/*****************************************************************************************************************************************
 *
 * Rev.3 DK with QFN48 (keyboard #2 with EEPROM)
 *
 *****************************************************************************************************************************************/

#if (BLE_ALT_PAIR)
# define BLE_ALT_I2C_ON     (1)
# define PAIR               (0xF4F1)
# define CLRP               (0xF4F2)
#else
# define BLE_ALT_I2C_ON     (0)
# define PAIR               (0x0000)
# define CLRP               (0x0000)
#endif

#if (KEYBOARD_MEASURE_EXT_SLP)
# define KB_EXTSLP          (0xF4F3)
#else
# define KB_EXTSLP          (0)
#endif

#define KBD_NR_INPUTS   (18)
#define KBD_NR_OUTPUTS  (8)

__STATIC const uint8 kbd_input_ports[KBD_NR_INPUTS] = {
//  port       column
    0x05,   // column 0
    0x04,   // column 1
    0x13,   // column 2
    0x12,   // column 3 
    0x11,   // column 4
    0x10,   // column 5
    0x27,   // column 6
    0x26,   // column 7
    0x25,   // column 8
    0x24,   // column 9
    0x37,   // column 10
    0x23,   // column 11
    0x36,   // column 12
    0x22,   // column 13
    0x21,   // column 14
    0x20,   // column 15
    0x31,   // column 16
    0x32    // column 17
};

// for a little cycle optimization below
#define P0_HAS_INPUT (1)
#define P1_HAS_INPUT (1)
#define P2_HAS_INPUT (1)
#define P3_HAS_INPUT (1)

// row output connection table
__STATIC const uint8 kbd_output_ports[KBD_NR_OUTPUTS] = {
//  port       row
    0x34,   // row 0
    0x33,   // row 1
    0x35,   // row 2
    0x06,   // row 3
    0x29,   // row 4
    0x28,   // row 5
    0x07,   // row 6
    0x30    // row 7
};

// extra sets for 'hidden modifiers', e.g. the 'Fn' key
#define KBD_NR_SETS (2)

// The key map.
// 00xx means regular key
// FCxx means modifier key.
// F8xx means FN Modifier.
// F4xy means special function (x = no of byte in the report, y no of bit set).
__STATIC const uint16 kbd_keymap[KBD_NR_SETS][KBD_NR_OUTPUTS][KBD_NR_INPUTS] = 
{
  { 
/*    No Fn key(s) pressed
      0         1         2         3         4         5           6         7         8         9         10        11        12        13        14        15        16        17
      ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
      #WWW Fav  #Med Sel  L-Ctrl    #WWW Ref  #Wake up  Home        Pg Up     Del       Insert    F9        -         F8        =         6         5         F2        F1        `
      #N Trk    #Calc               #WWW Sea  Print     End         Pg Down   F11       F12       F10       0         9         8         7         4         3         2         1
      App       Mute      Vol+      Vol-      Scroll    + (num)     9 (num)   7 (num)   8 (num)   #         p         o         i         u         r         e         Caps      q
      #Pre-Trk  #Sleep    #LVT      L-Shift   #My Comp  #           6 (num)   4 (num)   5 (num)   Backspc   [         F7        ]         y         t         F3        w         Tab
      L-Win     Fn        #WWW Fwd  #WWW Back #         Enter (num) 3 (num)   1 (num)   2 (num)   \         ;         L         k         j         f         d         #         a
      Play      #WWW Home                     L-Alt     Up arrow    , (num)   #         0 (num)   F5        '         #         F6        h         g         F4        s         Esc
      #Stop     R-Win     R-Ctrl    R-Shift   #Power    Pause       * (num)   Num Lk    / (num)   Enter     \         .         ,         m         v         c         x         z
      #e-mail   #Fn-wrong                     R-Alt     Left arrow  - (num)   Dn arrow  Rgt arrow Space     /         #         #         n         b         #R-K      #L-K      #
      ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/     
	{ 0x0000,   0x0000,   0xFC01,   0x0000,   0x0000,   0x004A,     0x004B,   0x004C,   0x0049,   0x0042,   0x002D,   0x0041,   0x002E,   0x0023,   0x0022,   0x003B,   0x003A,   0x0035 }, // 0
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0046,   0x004D,     0x004E,   0x0044,   0x0045,   0x0043,   0x0027,   0x0026,   0x0025,   0x0024,   0x0021,   0x0020,   0x001F,   0x001E }, // 1
	{ 0x0065,   0xF405,   0xF406,   0xF407,   0x0047,   0x0057,     0x0061,   0x005F,   0x0060,   0x0000,   0x0013,   0x0012,   0x000C,   0x0018,   0x0015,   0x0008,   0x0039,   0x0014 }, // 2
	{ 0x0000,   0x0000,   0x0000,   0xFC02,   0x0000,   0x0000,     0x005E,   0x005C,   0x005D,   0x002A,   0x002F,   0x0040,   0x0030,   0x001C,   0x0017,   0x003C,   0x001A,   0x002B }, // 3
	{ 0xFC08,   0xF801,   0x0000,   0x0000,   0x0000,   0x0058,     0x005B,   0x0059,   0x005A,   0x0064,   0x0033,   0x000F,   0x000E,   0x000D,   0x0009,   0x0007,   0x0000,   0x0004 }, // 4
	{ 0xF404,   0x0000,   0x0000,   0x0000,   0xFC04,   0x0052,     0x0063,   0x0000,   0x0062,   0x003E,   0x0034,   0x0000,   0x003F,   0x000B,   0x000A,   0x003D,   0x0016,   0x0029 }, // 5
	{ 0x0000,   0xFC80,   0xFC10,   0xFC20,   0x0000,   0x0048,     0x0055,   0x0053,   0x0054,   0x0028,   0x0064,   0x0037,   0x0036,   0x0010,   0x0019,   0x0006,   0x001B,   0x001D }, // 6
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0xFC40,   0x0050,     0x0056,   0x0051,   0x004F,   0x002C,   0x0038,   0x0000,   0x0000,   0x0011,   0x0005,   0x0000,   0x0000,   0x0000 }  // 7
  },
  {
/*    Fn key 0x01 pressed
      0         1         2         3         4         5           6         7         8         9         10        11        12        13        14        15        16        17
      ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
      #         #         #         #         #         #           #         #         #         #         #         Nxt Trk   #         #         #         #         WWW Home  #         
      #         #         #         #         Insert    #           #         Break     Scroll Lk #         #         #         #         #         #         #         #         #         
      #         #         #         #         #         #           #         #         #         #         Pair      #         #         #         #         #         #         #         
      #         #         #         #         #         #           #         #         #         #         #         Prv Trk   #         #         #         #         #         #         
      #         Fn        #         #         #         #           #         #         #         #         #         #         #         #         #         #         #         #         
      #         #         #         #         #         #           #         #         #         #         #         #         Stop      #         #         Eject     #         #         
      #         #         #         #         #         #           #         #         #         #         #         #         #         #         #         Clr pair  #         #         
      #         #         #         #         #         #           #         #         #         EXT_SLEEP #         #         #         #         #         #         #         #         
      ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/     
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,     0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0xF400,   0x0000,   0x0000,   0x0000,   0x0000,   0xF415,   0x0000,  }, // 0
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0049,   0x0000,     0x0000,   0x0048,   0x0047,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,  }, // 1
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,     0x0000,   0x0000,   0x0000,   0x0000,   PAIR,     0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,  }, // 2
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,     0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0xF401,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,  }, // 3
	{ 0x0000,   0xF801,   0x0000,   0x0000,   0x0000,   0x0000,     0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,  }, // 4
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,     0x0000,   0x0000,   0x0000,   0xF404,   0x0000,   0x0000,   0xF402,   0x0000,   0x0000,   0xF403,   0x0000,   0x0000,  }, // 5
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,     0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   CLRP,     0x0000,   0x0000,  }, // 6
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,     0x0000,   0x0000,   0x0000,   KB_EXTSLP,0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,  }  // 7
  }
};






#elif MATRIX_SETUP == 8
/*****************************************************************************************************************************************
 *
 * Reference Design (#1)
 *
 *****************************************************************************************************************************************/

#if (BLE_ALT_PAIR)
# define BLE_ALT_I2C_ON     (1)
# define PAIR               (0xF4F1)
# define CLRP               (0xF4F2)
#else
# define BLE_ALT_I2C_ON     (0)
# define PAIR               (0x0000)
# define CLRP               (0x0000)
#endif

#define KBD_NR_INPUTS   (18)
#define KBD_NR_OUTPUTS  (8)

__STATIC const uint8 kbd_input_ports[KBD_NR_INPUTS] = {
//  port       column
    0x05,   // column 0
    0x04,   // column 1
    0x01,   // column 2
    0x00,   // column 3 
    0x11,   // column 4
    0x10,   // column 5
    0x27,   // column 6
    0x26,   // column 7
    0x25,   // column 8
    0x24,   // column 9
    0x37,   // column 10
    0x23,   // column 11
    0x36,   // column 12
    0x22,   // column 13
    0x21,   // column 14
    0x20,   // column 15
    0x31,   // column 16
    0x32    // column 17
};

// for a little cycle optimization below
#define P0_HAS_INPUT (1)
#define P1_HAS_INPUT (1)
#define P2_HAS_INPUT (1)
#define P3_HAS_INPUT (1)

// row output connection table
__STATIC const uint8 kbd_output_ports[KBD_NR_OUTPUTS] = {
//  port       row
    0x34,   // row 0
    0x33,   // row 1
    0x35,   // row 2
    0x06,   // row 3
    0x29,   // row 4
    0x28,   // row 5
    0x07,   // row 6
    0x30    // row 7
};

// extra sets for 'hidden modifiers', e.g. the 'Fn' key
#define KBD_NR_SETS (2)

// The key map.
// 00xx means regular key
// FCxx means modifier key.
// F8xx means FN Modifier.
// F4xy means special function (x = no of byte in the report, y no of bit set).
__STATIC const uint16 kbd_keymap[KBD_NR_SETS][KBD_NR_OUTPUTS][KBD_NR_INPUTS] = 
{
  { 
/*    No Fn key(s) pressed
      0         1         2         3         4         5         6         7         8         9           10        11        12        13        14        15        16        17
      ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
      F7        F8        F9        F10       F11       F12       App       f         r         t           z         x         c         #???      v         Enter     Esc       #???
      a         s         d         q         w         e         Up arrow  Lft arrow Dn arrow  Rgt arrow   #???      #???      Calc      Space     F5        F6        Pause     Tab
      `(~)      -(_)      =         j         k         L         ;         5         6         Backspc     1         2         3         4         F1        F2        F3        F4
      y         u         i         o         Print     Scr Lock  Num Lock  / (num)   * (num)   - (num)     Vol+      Vol-      Mute      WWW Stop  Play      Prv Track Nxt Track #???
      p         [         ]         \         Home      End       7 (num)   8 (num)   9 (num)   + (num)     R-Win     #???      #???      #???      L-Win     #???      #???      #???
      7         8         9         0         Del       Pg Up     4 (num)   5 (num)   6 (num)   . (num)     WWW Home  R-Alt     WWW Back  #???      WWW Fwd   L-Alt     #???      #???
      g         h         '         #???      Ins       Pg Down   1 (num)   2 (num)   3 (num)   Enter (num) #???      #Sleep??? R-Shift   #???      Fn        #???      L-Shift   #???
      b         n         m         ,         .         /         #???      0 (num)   . (num)?? Caps Lock   Mail      #???      #???      R-Ctrl    #???      #???      #???      L-Ctrl
      ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/     
	{ 0x0040,   0x0041,   0x0042,   0x0043,   0x0044,   0x0045,   0x0065,   0x0009,   0x0015,   0x0017,     0x001D,   0x001B,   0x0006,   0x0000,   0x0019,   0x0028,   0x0029,   0x0000 }, // 0
	{ 0x0004,   0x0016,   0x0007,   0x0014,   0x001A,   0x0008,   0x0052,   0x0050,   0x0051,   0x004F,     0x0000,   0x0000,   0xF412,   0x002C,   0x003E,   0x003F,   0x0048,   0x002B }, // 1
	{ 0x0035,   0x002D,   0x002E,   0x000D,   0x000E,   0x000F,   0x0033,   0x0022,   0x0023,   0x002A,     0x001E,   0x001F,   0x0020,   0x0021,   0x003A,   0x003B,   0x003C,   0x003D }, // 2
	{ 0x001C,   0x0018,   0x000C,   0x0012,   0x0046,   0x0047,   0x0053,   0x0054,   0x0055,   0x0056,     0xF406,   0xF407,   0xF405,   0xF420,   0xF404,   0xF401,   0xF400,   0x0000 }, // 3
	{ 0x0013,   0x002F,   0x0030,   0x0031,   0x004A,   0x004D,   0x005F,   0x0060,   0x0061,   0x0057,     0xFC80,   0x0000,   0x0000,   0x0000,   0xFC08,   0x0000,   0x0000,   0x0000 }, // 4
	{ 0x0024,   0x0025,   0x0026,   0x0027,   0x004C,   0x004B,   0x005C,   0x005D,   0x005E,   0x0063,     0xF415,   0xFC40,   0xF416,   0x0000,   0xF417,   0xFC04,   0x0000,   0x0000 }, // 5
	{ 0x000A,   0x000B,   0x0034,   0x0000,   0x0049,   0x004E,   0x0059,   0x005A,   0x005B,   0x0058,     0x0000,   0x0000,   0xFC20,   0x0000,   0xF801,   0x0000,   0xFC02,   0x0000 }, // 6
	{ 0x0005,   0x0011,   0x0010,   0x0036,   0x0037,   0x0038,   0x0000,   0x0062,   0x0063,   0x0039,     0xF411,   0x0000,   0x0000,   0xFC10,   0x0000,   0x0000,   0x0000,   0xFC01 }  // 7
  },
  {
/*    Fn key 0x01 pressed
      0         1         2         3         4         5           6         7         8         9         10        11        12        13        14        15        16        17
      ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
      #         #         #         #         #         #           #         #         #         #         #         #         Clear     #         #         #         #         #         
      #         #         #         #         #         #           #         #         #         #         #         #         #         #         #         #         #         #         
      #         #         #         #         #         #           #         #         #         #         #         #         #         #         #         #         #         #         
      #         #         #         #         #         #           #         #         #         #         #         #         #         #         #         #         #         #         
      Pair      #         #         #         #         #           #         #         #         #         #         #         #         #         #         #         #         #         
      #         #         #         #         #         #           #         #         #         #         #         #         #         #         #         #         #         #         
      #         #         #         #         #         #           #         #         #         #         #         #         #         #         Fn        #         #         #         
      #         #         #         #         #         #           #         #         #         #         #         #         #         #         #         #         #         #         
      ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/     
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,     0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   CLRP,     0x0000,   0x0000,   0x0000,   0x0000,   0x0000,  }, // 0
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,     0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,  }, // 1
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,     0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,  }, // 2
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,     0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,  }, // 3
	{ PAIR,     0x0000,   0x0000,   0x0000,   0x0000,   0x0000,     0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,  }, // 4
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,     0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,  }, // 5
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,     0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0xF801,   0x0000,   0x0000,   0x0000,  }, // 6
	{ 0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,     0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,   0x0000,  }  // 7
  }
};
#endif // MATRIX_SETUP


#if KBD_NR_INPUTS < 17
typedef uint16_t scan_t;
#else
typedef uint32_t scan_t;
#endif

#endif // APP_KEYBOARD_MATRIX_H_
