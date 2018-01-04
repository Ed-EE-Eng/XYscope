/*
 * VectorFontROM.h
 *
 *  Created on: Jul 13, 2017
 *      Author: EdAndrews
 *
 *      This is a vector stroke font data file for use with the XYscope library. The FontROM
 *      data structure (below) to specify graphics elements, coordinates, and graphic feature parameters.
 *      Each character is defined using graphics primatives within a 16x16 XY coordinate
 *      character cell. Graphics "Opcode" primatives define NOP, POINT, LINE, CIRCLE, ELLIPSE, & RECTANGLE
 *      features that are used to "plot the character" into the XY point list.  Coordinate &
 *      Radius values are held as 4-bit (0-15) values, double packed two to a byte.
 *
 *      Lines & Rectangles (Opcodes LIN & REC) are defined by corner end points.
 *        Variable [P1]: Defines the feature start point (x0,y0)
 *        Variable [P2]: Defines the feature end point (x1,y1)
 *        Variable [P3]: Not used
 *
 *      Circles & Arcs (Opcode CIR) are defined by a center coordinate, feature radius, & segment flag.
 *        Variable [P1]: Defines the circlular feature center point (x),y0)
 *        Variable [P2]: Defines the circle radius (radius).
 *        Variable [P3]: Defines the 'segment' flag bits.
 *           By setting bits within the segment flag to 1, whole or partial circlular features can be plotted.
 *           When segment  bits are set to 1 (value=255), a full circle is drawn.
 *
 *      Ellipses (Opcode ELP) are defined by a center coordinate, feature radius, & segment flag.
 *        Variable [P1]: Defines the ellipse feature center point (x0,y0),
 *        Variable [P2]: Defines the ellipse X & Y radius values (x_radius,y_radius).
 *        Variable [P3]: Defines the 'segment' flag bits.
 *           By setting bits within the segment flag to 1, whole or partial ellipical features can be plotted.
 *           When segment  bits are set to 1 (value=255), a full ellipse is drawn.
 *
 *      A cross reference array, (Ascii2font[]) is used as a pointer to the Starting primative
 *      to each character.
 *
 *      Since the graphical definition of a given  character varies based on the graphics complexity required
 *      to draw the symbol, the MSB of the Opcode is used as an End-Of-Character (EOC) flag bit. When EOC is set,
 *      the character plotting routine that is scanning and executing the graphics primatives stops when it sees
 *      the EOC bit set.
 */

#ifndef VECTORFONTROM_H_
#define VECTORFONTROM_H_

//Define Stroke_Font EOC  (End_Of_Character) flag.
//This flag tells plotChar when the end of a character in the FontROM has been reached (time to plotting data for the current character)
uint8_t const EOC = 0x80;			//End of Character Flag (hi-order bit=1)

//Define Stroke_Font OpCodes (with EOC flag NOT SET); These codes are used within the FontROM data structure and which is the core of the plotChar routine
uint8_t const NOP = 0x0, PNT = 0x1, LIN = 0x2, REC = 0x3, CIR = 0x4, ELP = 0x5;	//NOP = No Operation, PNT = plot a single POINT, LIN = Plot a LINE
																				//REC = Plot a Rectangle, CIR = Plot a fully closed Cirecle or just some circular segment(s)
																				//ELP = Plot a fully closed Ellipse or just some ellipical segment(s).

//Define Stroke_Font OpCodes with EOC flag SET (Opcodes OR EOC_Flag)
uint8_t const NOP_E = NOP | EOC, PNT_E = PNT | EOC, LIN_E = LIN | EOC, REC_E =
		REC | EOC, CIR_E = CIR | EOC, ELP_E = ELP | EOC;

//This is the structure definition for the FontROM lookup table
struct FontROM {
	uint8_t Opcode;	//D7 = EOC Flag, D0-4 = Stroke Font Code (ie: NOP,Point,Line,Circle,Elipse,Rectangle, etc)
	uint8_t P1;		//D4-7= x0, D0-3= y0
	uint8_t P2;	//For LINE & RECT: D4-7= x1, D0-3= y1   For CIR & ELP: D4-7= radius_X, for ELP: D0-3= radius_Y
	uint8_t P3;		//for CIR & ELP (not used for others); D0-7 = Segment Flags.
					// When a SegmentFlag is set to 1, plot the segment; when set to 0, do not plot (skip) the segment
};

FontROM DigitFont[280] PROGMEM= {

	{	NOP_E,0x0,0x0,0x00},    // ASCII: 32   SP   (Space)

	{	LIN ,0x2E,0x28,0x00},    // ASCII: 33   !   (Exclamation mark)
	{	CIR_E,0x25,0x11,0xFF},    // ASCII: 33   !   (Exclamation mark)

	{	LIN ,0x2D,0x2B,0x00}, // ASCII: 34   "   (Quotation mark (&quot; in HTML))
	{	LIN_E,0x4D,0x4B,0x00}, // ASCII: 34   "   (Quotation mark (&quot; in HTML))

	{	LIN ,0x3D,0x26,0x3E},    // ASCII: 35   #   (Cross hatch (number sign))
	{	LIN ,0x5D,0x46,0x7C},    // ASCII: 35   #   (Cross hatch (number sign))
	{	LIN ,0x1B,0x6B,0x00},    // ASCII: 35   #   (Cross hatch (number sign))
	{	LIN_E,0x18,0x68,0x00},    // ASCII: 35   #   (Cross hatch (number sign))

	{	CIR ,0x3B,0x22,0xC7},    // ASCII: 36   $   (Dollar sign)
	{	CIR ,0x37,0x22,0x7C},    // ASCII: 36   $   (Dollar sign)
	{	LIN_E,0x3E,0x34,0x00},    // ASCII: 36   $   (Dollar sign)

	{	LIN ,0x6E,0x14,0xFC},    // ASCII: 37   %   (Percent sign)
	{	CIR ,0x2C,0x11,0xFF},    // ASCII: 37   %   (Percent sign)
	{	CIR_E,0x55,0x11,0xFF},    // ASCII: 37   %   (Percent sign)

	{	CIR ,0x3C,0x11,0x4F},    // ASCII: 38   &   (Ampersand)
	{	CIR ,0x37,0x22,0xE1},    // ASCII: 38   &   (Ampersand)
	{	LIN ,0x65,0x2B,0x00},    // ASCII: 38   &   (Ampersand)
	{	LIN ,0x4B,0x18,0x00},    // ASCII: 38   &   (Ampersand)
	{	LIN_E,0x56,0x67,0x00},    // ASCII: 38   &   (Ampersand)

	{	LIN_E,0x2D,0x2B,0x80}, // ASCII: 39   `   (Closing single quote (apostrophe))

	{	ELP_E,0x39,0x25,0xC3},    // ASCII: 40   (   (Opening parentheses)

	{	ELP_E,0x19,0x25,0x3C},    // ASCII: 41   )   (Closing parentheses)

	{	LIN ,0x19,0x79,0x00},    // ASCII: 42   *   (Asterisk (star, multiply))
	{	LIN ,0x27,0x6B,0x00},    // ASCII: 42   *   (Asterisk (star, multiply))
	{	LIN ,0x46,0x4C,0x00},    // ASCII: 42   *   (Asterisk (star, multiply))
	{	LIN_E,0x2B,0x67,0x00},    // ASCII: 42   *   (Asterisk (star, multiply))

	{	LIN ,0x19,0x79,0x00},    // ASCII: 43   +   (Plus)
	{	LIN_E,0x46,0x4C,0x00},    // ASCII: 43   +   (Plus)

	{	CIR ,0x25,0x11,0xFF},    // ASCII: 44   ,   (Comma)
	{	ELP_E,0x25,0x13,0x30},    // ASCII: 44   ,   (Comma)

	{	LIN_E,0x19,0x79,0x00},    // ASCII: 45   -   (Hyphen, dash, minus)

	{	CIR_E,0x35,0x11,0xFF},    // ASCII: 46   .   (Period)

	{	LIN_E,0x6E,0x14,0x00}, // ASCII: 47   /   (Slant (forward slash, divide))

	{	ELP ,0x49,0x35,0xFF},    // ASCII: 48   0   (Zero)
	{	LIN_E,0x14,0x7E,0x00},    // ASCII: 48   0   (Zero)

	{	LIN ,0x2C,0x4E,0x00},    // ASCII: 49   1   (One)
	{	LIN ,0x4E,0x44,0x00},    // ASCII: 49   1   (One)
	{	LIN_E,0x24,0x64,0x00},    // ASCII: 49   1   (One)

	{	ELP ,0x3C,0x22,0x03},    // ASCII: 50   2   (Two)
	{	ELP ,0x4C,0x22,0x1C},    // ASCII: 50   2   (Two)
	{	LIN ,0x3E,0x4E,0x00},    // ASCII: 50   2   (Two)
	{	LIN ,0x6B,0x15,0x00},    // ASCII: 50   2   (Two)
	{	ELP ,0x34,0x22,0x01},    // ASCII: 50   2   (Two)
	{	LIN_E,0x14,0x64,0x00},    // ASCII: 50   2   (Two)

	{	ELP ,0x3C,0x22,0x3E},    // ASCII: 51   3   (Three)
	{	ELP_E,0x37,0x33,0x7C},    // ASCII: 51   3   (Three)

	{	LIN ,0x54,0x5E,0x00},    // ASCII: 52   4   (Four)
	{	LIN ,0x5E,0x18,0x00},    // ASCII: 52   4   (Four)
	{	LIN_E,0x18,0x68,0x00},    // ASCII: 52   4   (Four)

	{	ELP ,0x47,0x33,0xFC},    // ASCII: 53   5   (Five)
	{	LIN ,0x4A,0x2A,0x00},    // ASCII: 53   5   (Five)
	{	LIN ,0x2A,0x3E,0x00},    // ASCII: 53   5   (Five)
	{	LIN_E,0x3E,0x7E,0x00},    // ASCII: 53   5   (Five)

	{	ELP ,0x47,0x33,0xFF},    // ASCII: 54   6   (Six)
	{	LIN_E,0x29,0x5E,0x00},    // ASCII: 54   6   (Six)

	{	LIN ,0x24,0x7E,0x00},    // ASCII: 55   7   (Seven)
	{	LIN_E,0x7E,0x1E,0x00},    // ASCII: 55   7   (Seven)

	{	ELP ,0x47,0x33,0xFF},    // ASCII: 56   8   (Eight)
	{	ELP_E,0x4C,0x22,0xFF},    // ASCII: 56   8   (Eight)

	{	ELP ,0x4B,0x33,0xFF},    // ASCII: 57   9   (Nine)
	{	LIN_E,0x7A,0x44,0x00},    // ASCII: 57   9   (Nine)

	{	CIR ,0x3B,0x11,0xFF},    // ASCII: 58   :   (Colon)
	{	CIR_E,0x37,0x11,0xFF},    // ASCII: 58   :   (Colon)

	{	CIR ,0x3B,0x11,0xFF},    // ASCII: 59   ;   (Semicolon)
	{	CIR ,0x37,0x11,0xFF},    // ASCII: 59   ;   (Semicolon)
	{	LIN_E,0x46,0x34,0x01},    // ASCII: 59   ;   (Semicolon)

	{	LIN ,0x19,0x4C,0x00}, // ASCII: 60   <   (Less than sign (&lt; in HTML))
	{	LIN_E,0x19,0x46,0x00}, // ASCII: 60   <   (Less than sign (&lt; in HTML))

	{	LIN ,0x1A,0x5A,0x00},    // ASCII: 61   =   (Equals sign)
	{	LIN_E,0x17,0x57,0x00},    // ASCII: 61   =   (Equals sign)

	{	LIN ,0x1C,0x49,0x00}, // ASCII: 62   >   (Greater than sign (&gt; in HTML))
	{	LIN_E,0x16,0x49,0x00}, // ASCII: 62   >   (Greater than sign (&gt; in HTML))

	{	CIR ,0x3C,0x22,0x3F},    // ASCII: 63   ?   (Question mark)
	{	LIN ,0x3A,0x38,0x00},    // ASCII: 63   ?   (Question mark)
	{	CIR_E,0x35,0x11,0xFF},    // ASCII: 63   ?   (Question mark)

	{	ELP ,0x49,0x35,0xCF},    // ASCII: 64   @   (At-sign)
	{	LIN ,0x44,0x74,0x00},    // ASCII: 64   @   (At-sign)
	{	ELP ,0x4A,0x12,0xC7},    // ASCII: 64   @   (At-sign)
	{	LIN ,0x5B,0x58,0x00},    // ASCII: 64   @   (At-sign)
	{	ELP_E,0x59,0x21,0x70},    // ASCII: 64   @   (At-sign)

	{	LIN ,0x14,0x4E,0x00},    // ASCII: 65   A   (Uppercase A)
	{	LIN ,0x4E,0x74,0x00},    // ASCII: 65   A   (Uppercase A)
	{	LIN_E,0x38,0x58,0x00},    // ASCII: 65   A   (Uppercase A)

	{	LIN ,0x34,0x14,0x00},    // ASCII: 66   B   (Uppercase B)
	{	LIN ,0x14,0x1E,0x00},    // ASCII: 66   B   (Uppercase B)
	{	LIN ,0x1A,0x3A,0x00},    // ASCII: 66   B   (Uppercase B)
	{	LIN ,0x1E,0x3E,0x00},    // ASCII: 66   B   (Uppercase B)
	{	CIR ,0x3C,0x22,0x3C},    // ASCII: 66   B   (Uppercase B)
	{	CIR_E,0x37,0x33,0x3C},    // ASCII: 66   B   (Uppercase B)

	{	ELP_E,0x49,0x35,0xE7},    // ASCII: 67   C   (Uppercase C)

	{	LIN ,0x34,0x14,0xFF},    // ASCII: 68   D   (Uppercase D)
	{	LIN ,0x14,0x1E,0x00},    // ASCII: 68   D   (Uppercase D)
	{	LIN ,0x1E,0x3E,0x00},    // ASCII: 68   D   (Uppercase D)
	{	ELP_E,0x39,0x35,0x3C},    // ASCII: 68   D   (Uppercase D)

	{	LIN ,0x6E,0x1E,0x00},    // ASCII: 69   E   (Uppercase E)
	{	LIN ,0x1E,0x14,0x00},    // ASCII: 69   E   (Uppercase E)
	{	LIN ,0x19,0x49,0x00},    // ASCII: 69   E   (Uppercase E)
	{	LIN_E,0x14,0x64,0x00},    // ASCII: 69   E   (Uppercase E)

	{	LIN ,0x6E,0x1E,0x00},    // ASCII: 70   F   (Uppercase F)
	{	LIN ,0x1E,0x14,0x00},    // ASCII: 70   F   (Uppercase F)
	{	LIN_E,0x19,0x49,0x00},    // ASCII: 70   F   (Uppercase F)

	{	LIN ,0x58,0x68,0x00},    // ASCII: 71   G   (Uppercase G)
	{	LIN ,0x68,0x64,0x00},    // ASCII: 71   G   (Uppercase G)
	{	ELP_E,0x49,0x35,0xE7},    // ASCII: 71   G   (Uppercase G)

	{	LIN ,0x1E,0x14,0x00},    // ASCII: 72   H   (Uppercase H)
	{	LIN ,0x6E,0x64,0x00},    // ASCII: 72   H   (Uppercase H)
	{	LIN_E,0x19,0x69,0x00},    // ASCII: 72   H   (Uppercase H)

	{	LIN ,0x3E,0x34,0x00},    // ASCII: 73   I   (Uppercase I)
	{	LIN ,0x1E,0x5E,0x00},    // ASCII: 73   I   (Uppercase I)
	{	LIN_E,0x14,0x54,0x00},    // ASCII: 73   I   (Uppercase I)

	{	CIR ,0x36,0x22,0xF0},    // ASCII: 74   J   (Uppercase J)
	{	LIN ,0x56,0x5E,0x00},    // ASCII: 74   J   (Uppercase J)
	{	LIN_E,0x3E,0x6E,0x00},    // ASCII: 74   J   (Uppercase J)

	{	LIN ,0x14,0x1E,0x00},    // ASCII: 75   K   (Uppercase K)
	{	LIN ,0x64,0x19,0x00},    // ASCII: 75   K   (Uppercase K)
	{	LIN_E,0x6E,0x19,0x00},    // ASCII: 75   K   (Uppercase K)

	{	LIN ,0x64,0x14,0x00},    // ASCII: 76   L   (Uppercase L)
	{	LIN_E,0x14,0x1E,0x00},    // ASCII: 76   L   (Uppercase L)

	{	LIN ,0x14,0x1E,0x00},    // ASCII: 77   M   (Uppercase M)
	{	LIN ,0x74,0x7E,0x00},    // ASCII: 77   M   (Uppercase M)
	{	LIN ,0x1E,0x47,0x00},    // ASCII: 77   M   (Uppercase M)
	{	LIN_E,0x47,0x7E,0x00},    // ASCII: 77   M   (Uppercase M)

	{	LIN ,0x14,0x1E,0x00},    // ASCII: 78   N   (Uppercase N)
	{	LIN ,0x6E,0x64,0x00},    // ASCII: 78   N   (Uppercase N)
	{	LIN_E,0x1E,0x64,0x00},    // ASCII: 78   N   (Uppercase N)

	{	ELP_E,0x49,0x35,0xFF},    // ASCII: 79   O   (Uppercase O)

	{	LIN ,0x14,0x1E,0x00},    // ASCII: 80   P   (Uppercase P)
	{	LIN ,0x19,0x49,0x00},    // ASCII: 80   P   (Uppercase P)
	{	LIN ,0x1E,0x4E,0x00},    // ASCII: 80   P   (Uppercase P)
	{	LIN ,0x6C,0x6A,0x3C},    // ASCII: 80   P   (Uppercase P)
	{	CIR ,0x4C,0x22,0xC},    // ASCII: 80   P   (Uppercase P)
	{	CIR_E,0x4B,0x22,0x30},    // ASCII: 80   P   (Uppercase P)

	{	ELP ,0x49,0x35,0xFF},    // ASCII: 81   Q   (Uppercase Q)
	{	LIN_E,0x56,0x74,0x00},    // ASCII: 81   Q   (Uppercase Q)

	{	LIN ,0x14,0x1E,0x00},    // ASCII: 82   R   (Uppercase R)
	{	LIN ,0x19,0x49,0x00},    // ASCII: 82   R   (Uppercase R)
	{	LIN ,0x1E,0x4E,0x00},    // ASCII: 82   R   (Uppercase R)
	{	LIN ,0x6C,0x6A,0x3C},    // ASCII: 82   R   (Uppercase R)
	{	CIR ,0x4C,0x22,0xC},    // ASCII: 82   R   (Uppercase R)
	{	CIR ,0x4B,0x22,0x30},    // ASCII: 82   R   (Uppercase R)
	{	LIN_E,0x49,0x64,0x00},    // ASCII: 82   R   (Uppercase R)

	{	CIR ,0x4C,0x22,0xC7},    // ASCII: 83   S   (Uppercase S)
	{	CIR_E,0x47,0x33,0x7C},    // ASCII: 83   S   (Uppercase S)

	{	LIN ,0x44,0x4E,0x00},    // ASCII: 84   T   (Uppercase T)
	{	LIN ,0x1E,0x4E,0x00},    // ASCII: 84   T   (Uppercase T)
	{	LIN_E,0x7E,0x4E,0x00},    // ASCII: 84   T   (Uppercase T)

	{	CIR ,0x47,0x33,0xF0},    // ASCII: 85   U   (Uppercase U)
	{	LIN ,0x1E,0x17,0x00},    // ASCII: 85   U   (Uppercase U)
	{	LIN_E,0x7E,0x77,0x00},    // ASCII: 85   U   (Uppercase U)

	{	LIN ,0x1E,0x44,0x00},    // ASCII: 86   V   (Uppercase V)
	{	LIN_E,0x44,0x7E,0x00},    // ASCII: 86   V   (Uppercase V)

	{	LIN ,0x1E,0x34,0x00},    // ASCII: 87   W   (Uppercase W)
	{	LIN ,0x34,0x5A,0x00},    // ASCII: 87   W   (Uppercase W)
	{	LIN ,0x5A,0x74,0x00},    // ASCII: 87   W   (Uppercase W)
	{	LIN_E,0x74,0x9E,0x00},    // ASCII: 87   W   (Uppercase W)

	{	LIN ,0x1E,0x74,0x00},    // ASCII: 88   X   (Uppercase X)
	{	LIN_E,0x14,0x7E,0x00},    // ASCII: 88   X   (Uppercase X)

	{	LIN ,0x1E,0x49,0x00},    // ASCII: 89   Y   (Uppercase Y)
	{	LIN ,0x7E,0x49,0x00},    // ASCII: 89   Y   (Uppercase Y)
	{	LIN_E,0x44,0x49,0x00},    // ASCII: 89   Y   (Uppercase Y)

	{	LIN ,0x1E,0x7E,0x00},    // ASCII: 90   Z   (Uppercase Z)
	{	LIN ,0x7E,0x14,0x00},    // ASCII: 90   Z   (Uppercase Z)
	{	LIN_E,0x14,0x74,0x00},    // ASCII: 90   Z   (Uppercase Z)

	{	LIN ,0x1E,0x14,0x00},    // ASCII: 91   [   (Opening square bracket)
	{	LIN ,0x1E,0x3e,0x00},    // ASCII: 91   [   (Opening square bracket)
	{	LIN_E,0x14,0x34,0x00},    // ASCII: 91   [   (Opening square bracket)

	{	LIN_E,0x1E,0x64,0x00},    // ASCII: 92   \   (Reverse slant (Backslash))

	{	LIN ,0x3E,0x34,0x00},    // ASCII: 93   ]   (Closing square bracket)
	{	LIN ,0x1E,0x3E,0x00},    // ASCII: 93   ]   (Closing square bracket)
	{	LIN_E,0x14,0x34,0x00},    // ASCII: 93   ]   (Closing square bracket)

	{	LIN ,0x19,0x3D,0x00},    // ASCII: 94   ^   (Caret (Circumflex))
	{	LIN_E,0x3D,0x59,0x00},    // ASCII: 94   ^   (Caret (Circumflex))

	{	LIN_E,0x2,0x72,0x00},    // ASCII: 95   _   (Underscore)

	{	LIN_E,0x2D,0x4B,0x00},    // ASCII: 96   `   (Opening single quote)

	{	ELP ,0x37,0x23,0xFF},    // ASCII: 97   a   (Lowercase a)
	{	LIN_E,0x57,0x54,0x00},    // ASCII: 97   a   (Lowercase a)

	{	ELP ,0x37,0x23,0xFF},    // ASCII: 98   b   (Lowercase b)
	{	LIN_E,0x1E,0x14,0x00},    // ASCII: 98   b   (Lowercase b)

	{	ELP_E,0x37,0x23,0xE7},    // ASCII: 99   c   (Lowercase c)

	{	ELP ,0x37,0x23,0xFF},    // ASCII: 100   d   (Lowercase d)
	{	LIN_E,0x5E,0x54,0x00},    // ASCII: 100   d   (Lowercase d)

	{	ELP ,0x37,0x23,0xEF},    // ASCII: 101   e   (Lowercase e)
	{	LIN_E,0x17,0x57,0x00},    // ASCII: 101   e   (Lowercase e)

	{	CIR ,0x4C,0x22,0x07},    // ASCII: 102   f   (Lowercase f)
	{	LIN ,0x19,0x49,0x00},    // ASCII: 102   f   (Lowercase f)
	{	LIN_E,0x24,0x2c,0x00},    // ASCII: 102   f   (Lowercase f)

	{	ELP ,0x37,0x23,0xFF},    // ASCII: 103   g   (Lowercase g)
	{	LIN ,0x56,0x52,0x00},    // ASCII: 103   g   (Lowercase g)
	{	CIR_E,0x32,0x22,0x70},    // ASCII: 103   g   (Lowercase g)

	{	LIN ,0x1E,0x14,0x00},    // ASCII: 104   h   (Lowercase h)
	{	ELP ,0x38,0x22,0xF},    // ASCII: 104   h   (Lowercase h)
	{	LIN_E,0x58,0x54,0x00},    // ASCII: 104   h   (Lowercase h)

	{	CIR ,0x2C,0x11,0xFF},    // ASCII: 105   i   (Lowercase i)
	{	LIN_E,0x24,0x29,0x00},    // ASCII: 105   i   (Lowercase i)

	{	CIR ,0x3C,0x11,0xFF},    // ASCII: 106   j   (Lowercase j)
	{	LIN ,0x39,0x33,0x00},    // ASCII: 106   j   (Lowercase j)
	{	CIR_E,0x13,0x22,0x30},    // ASCII: 106   j   (Lowercase j)

	{	LIN ,0x1E,0x14,0x00},    // ASCII: 107   k   (Lowercase k)
	{	LIN ,0x17,0x5A,0x00},    // ASCII: 107   k   (Lowercase k)
	{	LIN_E,0x54,0x38,0x00},    // ASCII: 107   k   (Lowercase k)

	{	LIN ,0x1E,0x15,0x00},    // ASCII: 108   l   (Lowercase l)
	{	LIN_E,0x24,0x15,0x00},    // ASCII: 108   l   (Lowercase l)

	{	LIN ,0x0A,0x4,0x00},    // ASCII: 109   m   (Lowercase m)
	{	LIN ,0x44,0x48,0x00},    // ASCII: 109   m   (Lowercase m)
	{	LIN ,0x84,0x88,0x00},    // ASCII: 109   m   (Lowercase m)
	{	CIR ,0x28,0x22,0xF},    // ASCII: 109   m   (Lowercase m)
	{	CIR_E,0x68,0x22,0xF},    // ASCII: 109   m   (Lowercase m)

	{	LIN ,0x1A,0x14,0x00},    // ASCII: 110   n   (Lowercase n)
	{	LIN ,0x54,0x58,0x00},    // ASCII: 110   n   (Lowercase n)
	{	CIR_E,0x38,0x22,0xF},    // ASCII: 110   n   (Lowercase n)

	{	ELP_E,0x37,0x23,0xFF},    // ASCII: 111   o   (Lowercase o)

	{	ELP ,0x37,0x23,0xFF},    // ASCII: 112   p   (Lowercase p)
	{	LIN_E,0x18,0x10,0x00},    // ASCII: 112   p   (Lowercase p)

	{	ELP ,0x37,0x23,0xFF},    // ASCII: 113   q   (Lowercase q)
	{	LIN_E,0x58,0x50,0x00},    // ASCII: 113   q   (Lowercase q)

	{	ELP ,0x37,0x23,0x07},    // ASCII: 114   r   (Lowercase r)
	{	LIN_E,0x1A,0x14,0x00},    // ASCII: 114   r   (Lowercase r)

	{	ELP ,0x39,0x21,0xC7},    // ASCII: 115   s   (Lowercase s)
	{	CIR_E,0x36,0x22,0x7C},    // ASCII: 115   s   (Lowercase s)

	{	LIN ,0x3E,0x35,0xE3},    // ASCII: 116   t   (Lowercase t)
	{	LIN ,0x1B,0x5B,0x3E},    // ASCII: 116   t   (Lowercase t)
	{	CIR_E,0x45,0x11,0xC0},    // ASCII: 116   t   (Lowercase t)

	{	LIN ,0x1A,0x16,0x00},    // ASCII: 117   u   (Lowercase u)
	{	LIN ,0x5A,0x54,0x00},    // ASCII: 117   u   (Lowercase u)
	{	CIR_E,0x36,0x22,0xF0},    // ASCII: 117   u   (Lowercase u)

	{	LIN ,0x1A,0x34,0xF},    // ASCII: 118   v   (Lowercase v)
	{	LIN_E,0x5A,0x34,0x00},    // ASCII: 118   v   (Lowercase v)

	{	LIN ,0x1A,0x34,0x00},    // ASCII: 119   w   (Lowercase w)
	{	LIN ,0x5A,0x34,0x00},    // ASCII: 119   w   (Lowercase w)
	{	LIN ,0x5A,0x74,0x00},    // ASCII: 119   w   (Lowercase w)
	{	LIN_E,0x9A,0x74,0x00},    // ASCII: 119   w   (Lowercase w)

	{	LIN ,0x1A,0x54,0x00},    // ASCII: 120   x   (Lowercase x)
	{	LIN_E,0x14,0x5A,0x00},    // ASCII: 120   x   (Lowercase x)

	{	LIN ,0x1A,0x34,0x00},    // ASCII: 121   y   (Lowercase y)
	{	LIN_E,0x5A,0x21,0x00},    // ASCII: 121   y   (Lowercase y)

	{	LIN ,0x1A,0x4A,0x00},    // ASCII: 122   z   (Lowercase z)
	{	LIN ,0x4A,0x14,0x00},    // ASCII: 122   z   (Lowercase z)
	{	LIN_E,0x14,0x44,0x00},    // ASCII: 122   z   (Lowercase z)

	{	CIR ,0x3D,0x11,0x03},    // ASCII: 123   {   (Opening curly brace)
	{	LIN ,0x2D,0x2A,0x30},    // ASCII: 123   {   (Opening curly brace)
	{	CIR ,0x1A,0x11,0x30},    // ASCII: 123   {   (Opening curly brace)
	{	CIR ,0x18,0x11,0xC},    // ASCII: 123   {   (Opening curly brace)
	{	LIN ,0x28,0x25,0x00},    // ASCII: 123   {   (Opening curly brace)
	{	CIR_E,0x35,0x11,0xC0},    // ASCII: 123   {   (Opening curly brace)

	{	LIN_E,0x2E,0x24,0x00},    // ASCII: 124   |   (Vertical line)

	{	CIR ,0x1D,0x11,0xC},    // ASCII: 125   }   (Closing curly brace)
	{	LIN ,0x2D,0x2A,0x00},    // ASCII: 125   }   (Closing curly brace)
	{	CIR ,0x3A,0x11,0xC0},    // ASCII: 125   }   (Closing curly brace)
	{	CIR ,0x38,0x11,0x03},    // ASCII: 125   }   (Closing curly brace)
	{	LIN ,0x28,0x25,0x00},    // ASCII: 125   }   (Closing curly brace)
	{	CIR_E,0x15,0x11,0x30},    // ASCII: 125   }   (Closing curly brace)

	{	CIR ,0x2B,0x11,0xF},    // ASCII: 126   ~   (Tilde (approximate))
	{	CIR_E,0x4B,0x11,0xF0},    // ASCII: 126   ~   (Tilde (approximate))

	{	REC ,0x0E,0x74,0x00}, // ASCII: 127   DEL   (Delete (rubout), cross-hatch box)
	{	LIN ,0x1D,0x65,0x00}, // ASCII: 127   DEL   (Delete (rubout), cross-hatch box)
	{	LIN_E,0x15,0x6D,0x00}, // ASCII: 127   DEL   (Delete (rubout), cross-hatch box)

	{	CIR ,0x69,0x55,0xE7},    //
	{	LIN ,0x9D,0xD9,0x00},    //
	{	LIN ,0xD9,0x95,0x00},    //
	{	LIN_E,0x39,0x99,0x00},    //

	{	CIR ,0x79,0x55,0x7E},    //
	{	LIN ,0x4D,0x9,0x00},    //
	{	LIN ,0x9,0x45,0x00},    //
	{	LIN ,0x49,0xA9,0x00},    //
	{	LIN_E,0x7C,0x76,0x00},    //

};

uint16_t Ascii2Font[260] PROGMEM= {	//bits 0-8 = Pointer into VectorFontROM, bits 9-15 = Prop Character Width
	Ascii2Font[0] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[1] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[2] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[3] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[4] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[5] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[6] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[7] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[8] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[9] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[10] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[11] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[12] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[13] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[14] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[15] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[16] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[17] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[18] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[19] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[20] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[21] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[22] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[23] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[24] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[25] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[26] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[27] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[28] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[29] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[30] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[31] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[32] = 4608,// ASCII: 32   SP   (Space)
	Ascii2Font[33] = 2561,// ASCII: 33   !   (Exclamation mark)
	Ascii2Font[34] = 3075,// ASCII: 34   "   (Quotation mark (&quot; in HTML))
	Ascii2Font[35] = 4101,// ASCII: 35   #   (Cross hatch (number sign))
	Ascii2Font[36] = 3593,// ASCII: 36   $   (Dollar sign)
	Ascii2Font[37] = 4108,// ASCII: 37   %   (Percent sign)
	Ascii2Font[38] = 4111,// ASCII: 38   &   (Ampersand)
	Ascii2Font[39] = 2580,// ASCII: 39   `   (Closing single quote (apostrophe))
	Ascii2Font[40] = 2581,// ASCII: 40   (   (Opening parentheses)
	Ascii2Font[41] = 2582,// ASCII: 41   )   (Closing parentheses)
	Ascii2Font[42] = 4631,// ASCII: 42   *   (Asterisk (star, multiply))
	Ascii2Font[43] = 4635,// ASCII: 43   +   (Plus)
	Ascii2Font[44] = 2589,// ASCII: 44   ,   (Comma)
	Ascii2Font[45] = 4639,// ASCII: 45   -   (Hyphen, dash, minus)
	Ascii2Font[46] = 4640,// ASCII: 46   .   (Period)
	Ascii2Font[47] = 4129,// ASCII: 47   /   (Slant (forward slash, divide))
	Ascii2Font[48] = 4642,// ASCII: 48   0   (Zero)
	Ascii2Font[49] = 4644,// ASCII: 49   1   (One)
	Ascii2Font[50] = 4647,// ASCII: 50   2   (Two)
	Ascii2Font[51] = 4653,// ASCII: 51   3   (Three)
	Ascii2Font[52] = 4655,// ASCII: 52   4   (Four)
	Ascii2Font[53] = 4658,// ASCII: 53   5   (Five)
	Ascii2Font[54] = 4662,// ASCII: 54   6   (Six)
	Ascii2Font[55] = 4664,// ASCII: 55   7   (Seven)
	Ascii2Font[56] = 4666,// ASCII: 56   8   (Eight)
	Ascii2Font[57] = 4668,// ASCII: 57   9   (Nine)
	Ascii2Font[58] = 3646,// ASCII: 58   :   (Colon)
	Ascii2Font[59] = 3648,// ASCII: 59   ;   (Semicolon)
	Ascii2Font[60] = 3651,// ASCII: 60   <   (Less than sign (&lt; in HTML))
	Ascii2Font[61] = 3653,// ASCII: 61   =   (Equals sign)
	Ascii2Font[62] = 3143,// ASCII: 62   >   (Greater than sign (&gt; in HTML))
	Ascii2Font[63] = 3657,// ASCII: 63   ?   (Question mark)
	Ascii2Font[64] = 4684,// ASCII: 64   @   (At-sign)
	Ascii2Font[65] = 4689,// ASCII: 65   A   (Uppercase A)
	Ascii2Font[66] = 4180,// ASCII: 66   B   (Uppercase B)
	Ascii2Font[67] = 4186,// ASCII: 67   C   (Uppercase C)
	Ascii2Font[68] = 4187,// ASCII: 68   D   (Uppercase D)
	Ascii2Font[69] = 4191,// ASCII: 69   E   (Uppercase E)
	Ascii2Font[70] = 4195,// ASCII: 70   F   (Uppercase F)
	Ascii2Font[71] = 4198,// ASCII: 71   G   (Uppercase G)
	Ascii2Font[72] = 4201,// ASCII: 72   H   (Uppercase H)
	Ascii2Font[73] = 3692,// ASCII: 73   I   (Uppercase I)
	Ascii2Font[74] = 4207,// ASCII: 74   J   (Uppercase J)
	Ascii2Font[75] = 4210,// ASCII: 75   K   (Uppercase K)
	Ascii2Font[76] = 4213,// ASCII: 76   L   (Uppercase L)
	Ascii2Font[77] = 4727,// ASCII: 77   M   (Uppercase M)
	Ascii2Font[78] = 4219,// ASCII: 78   N   (Uppercase N)
	Ascii2Font[79] = 4734,// ASCII: 79   O   (Uppercase O)
	Ascii2Font[80] = 4223,// ASCII: 80   P   (Uppercase P)
	Ascii2Font[81] = 4741,// ASCII: 81   Q   (Uppercase Q)
	Ascii2Font[82] = 4231,// ASCII: 82   R   (Uppercase R)
	Ascii2Font[83] = 4238,// ASCII: 83   S   (Uppercase S)
	Ascii2Font[84] = 4752,// ASCII: 84   T   (Uppercase T)
	Ascii2Font[85] = 4755,// ASCII: 85   U   (Uppercase U)
	Ascii2Font[86] = 4758,// ASCII: 86   V   (Uppercase V)
	Ascii2Font[87] = 5784,// ASCII: 87   W   (Uppercase W)
	Ascii2Font[88] = 4764,// ASCII: 88   X   (Uppercase X)
	Ascii2Font[89] = 4766,// ASCII: 89   Y   (Uppercase Y)
	Ascii2Font[90] = 4769,// ASCII: 90   Z   (Uppercase Z)
	Ascii2Font[91] = 2724,// ASCII: 91   [   (Opening square bracket)
	Ascii2Font[92] = 4263,// ASCII: 92   \   (Reverse slant (Backslash))
	Ascii2Font[93] = 2728,// ASCII: 93   ]   (Closing square bracket)
	Ascii2Font[94] = 3755,// ASCII: 94   ^   (Caret (Circumflex))
	Ascii2Font[95] = 4269,// ASCII: 95   _   (Underscore)
	Ascii2Font[96] = 3246,// ASCII: 96   `   (Opening single quote)
	Ascii2Font[97] = 3759,// ASCII: 97   a   (Lowercase a)
	Ascii2Font[98] = 3761,// ASCII: 98   b   (Lowercase b)
	Ascii2Font[99] = 3251,// ASCII: 99   c   (Lowercase c)
	Ascii2Font[100] = 3764,// ASCII: 100   d   (Lowercase d)
	Ascii2Font[101] = 3766,// ASCII: 101   e   (Lowercase e)
	Ascii2Font[102] = 3256,// ASCII: 102   f   (Lowercase f)
	Ascii2Font[103] = 3771,// ASCII: 103   g   (Lowercase g)
	Ascii2Font[104] = 3774,// ASCII: 104   h   (Lowercase h)
	Ascii2Font[105] = 2753,// ASCII: 105   i   (Lowercase i)
	Ascii2Font[106] = 2755,// ASCII: 106   j   (Lowercase j)
	Ascii2Font[107] = 3782,// ASCII: 107   k   (Lowercase k)
	Ascii2Font[108] = 2249,// ASCII: 108   l   (Lowercase l)
	Ascii2Font[109] = 5323,// ASCII: 109   m   (Lowercase m)
	Ascii2Font[110] = 3792,// ASCII: 110   n   (Lowercase n)
	Ascii2Font[111] = 3795,// ASCII: 111   o   (Lowercase o)
	Ascii2Font[112] = 3796,// ASCII: 112   p   (Lowercase p)
	Ascii2Font[113] = 3798,// ASCII: 113   q   (Lowercase q)
	Ascii2Font[114] = 3800,// ASCII: 114   r   (Lowercase r)
	Ascii2Font[115] = 3802,// ASCII: 115   s   (Lowercase s)
	Ascii2Font[116] = 3804,// ASCII: 116   t   (Lowercase t)
	Ascii2Font[117] = 3807,// ASCII: 117   u   (Lowercase u)
	Ascii2Font[118] = 3810,// ASCII: 118   v   (Lowercase v)
	Ascii2Font[119] = 5860,// ASCII: 119   w   (Lowercase w)
	Ascii2Font[120] = 3816,// ASCII: 120   x   (Lowercase x)
	Ascii2Font[121] = 3818,// ASCII: 121   y   (Lowercase y)
	Ascii2Font[122] = 3308,// ASCII: 122   z   (Lowercase z)
	Ascii2Font[123] = 2799,// ASCII: 123   {   (Opening curly brace)
	Ascii2Font[124] = 2293,// ASCII: 124   |   (Vertical line)
	Ascii2Font[125] = 2806,// ASCII: 125   }   (Closing curly brace)
	Ascii2Font[126] = 3836,// ASCII: 126   ~   (Tilde (approximate))
	Ascii2Font[127] = 4350,// ASCII: 127   DEL   (Delete (rubout), cross-hatch box)
	Ascii2Font[128] = 6913,//
	Ascii2Font[129] = 7429,//
	Ascii2Font[130] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[131] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[132] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[133] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[134] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[135] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[136] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[137] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[138] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[139] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[140] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[141] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[142] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[143] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[144] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[145] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[146] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[147] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[148] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[149] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[150] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[151] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[152] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[153] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[154] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[155] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[156] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[157] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[158] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[159] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[160] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[161] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[162] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[163] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[164] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[165] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[166] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[167] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[168] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[169] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[170] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[171] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[172] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[173] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[174] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[175] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[176] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[177] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[178] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[179] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[180] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[181] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[182] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[183] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[184] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[185] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[186] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[187] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[188] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[189] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[190] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[191] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[192] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[193] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[194] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[195] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[196] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[197] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[198] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[199] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[200] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[201] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[202] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[203] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[204] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[205] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[206] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[207] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[208] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[209] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[210] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[211] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[212] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[213] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[214] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[215] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[216] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[217] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[218] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[219] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[220] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[221] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[222] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[223] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[224] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[225] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[226] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[227] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[228] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[229] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[230] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[231] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[232] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[233] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[234] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[235] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[236] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[237] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[238] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[239] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[240] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[241] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[242] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[243] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[244] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[245] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[246] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[247] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[248] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[249] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[250] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[251] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[252] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[253] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[254] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol
	Ascii2Font[255] = 4860,//No symbol defined in FontROM. As a diagnostic helper, show a TILDE symbol

};

#endif /* VECTORFONTROM_H_ */
