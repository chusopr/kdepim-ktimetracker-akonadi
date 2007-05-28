/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

const int gsmlen=138;
const QChar alphabet7bit[]={
    0x0040 /* COMMERCIAL AT */ ,
    0x00A3 /* POUND SIGN */ ,
    0x0024 /* DOLLAR SIGN */ ,
    0x00A5 /* YEN SIGN */ ,
    0x00E8 /* LATIN SMALL LETTER E WITH GRAVE */ ,
    0x00E9 /* LATIN SMALL LETTER E WITH ACUTE */ ,
    0x00F9 /* LATIN SMALL LETTER U WITH GRAVE */ ,
    0x00EC /* LATIN SMALL LETTER I WITH GRAVE */ ,
    0x00F2 /* LATIN SMALL LETTER O WITH GRAVE */ ,
    0x00E7 /* LATIN SMALL LETTER C WITH CEDILLA */ ,
    0x000A /* LINE FEED */ ,
    0x00D8 /* LATIN CAPITAL LETTER O WITH STROKE */ ,
    0x00F8 /* LATIN SMALL LETTER O WITH STROKE */ ,
    0x000D /* CARRIAGE RETURN */ ,
    0x00C5 /* LATIN CAPITAL LETTER A WITH RING ABOVE */ ,
    0x00E5 /* LATIN SMALL LETTER A WITH RING ABOVE */ ,
    0x0394 /* GREEK CAPITAL LETTER DELTA */ ,
    0x005F /* LOW LINE */ ,
    0x03A6 /* GREEK CAPITAL LETTER PHI */ ,
    0x0393 /* GREEK CAPITAL LETTER GAMMA */ ,
    0x039B /* GREEK CAPITAL LETTER LAMDA */ ,
    0x03A9 /* GREEK CAPITAL LETTER OMEGA */ ,
    0x03A0 /* GREEK CAPITAL LETTER PI */ ,
    0x03A8 /* GREEK CAPITAL LETTER PSI */ ,
    0x03A3 /* GREEK CAPITAL LETTER SIGMA */ ,
    0x0398 /* GREEK CAPITAL LETTER THETA */ ,
    0x039E /* GREEK CAPITAL LETTER XI */ ,
    0x00A0 /* ESCAPE TO EXTENSION TABLE (or displayed as NBSP, see note above) */ ,

    0x00C6 /* LATIN CAPITAL LETTER AE */ ,
    0x00E6 /* LATIN SMALL LETTER AE */ ,
    0x00DF /* LATIN SMALL LETTER SHARP S (German) */ ,
    0x00C9 /* LATIN CAPITAL LETTER E WITH ACUTE */ ,
    0x0020 /* SPACE */ ,
    0x0021 /* EXCLAMATION MARK */ ,
    0x0022 /* QUOTATION MARK */ ,
    0x0023 /* NUMBER SIGN */ ,
    0x00A4 /* CURRENCY SIGN */ ,
    0x0025 /* PERCENT SIGN */ ,
    0x0026 /* AMPERSAND */ ,
    0x0027 /* APOSTROPHE */ ,
    0x0028 /* LEFT PARENTHESIS */ ,
    0x0029 /* RIGHT PARENTHESIS */ ,
    0x002A /* ASTERISK */ ,
    0x002B /* PLUS SIGN */ ,
    0x002C /* COMMA */ ,
    0x002D /* HYPHEN-MINUS */ ,
    0x002E /* FULL STOP */ ,
    0x002F /* SOLIDUS */ ,
    0x0030 /* DIGIT ZERO */ ,
    0x0031 /* DIGIT ONE */ ,
    0x0032 /* DIGIT TWO */ ,
    0x0033 /* DIGIT THREE */ ,
    0x0034 /* DIGIT FOUR */ ,
    0x0035 /* DIGIT FIVE */ ,
    0x0036 /* DIGIT SIX */ ,
    0x0037 /* DIGIT SEVEN */ ,
    0x0038 /* DIGIT EIGHT */ ,
    0x0039 /* DIGIT NINE */ ,
    0x003A /* COLON */ ,
    0x003B /* SEMICOLON */ ,
    0x003C /* LESS-THAN SIGN */ ,
    0x003D /* EQUALS SIGN */ ,
    0x003E /* GREATER-THAN SIGN */ ,
    0x003F /* QUESTION MARK */ ,
    0x00A1 /* INVERTED EXCLAMATION MARK */ ,
    0x0041 /* LATIN CAPITAL LETTER A */ ,
    0x0042 /* LATIN CAPITAL LETTER B */ ,
    0x0043 /* LATIN CAPITAL LETTER C */ ,
    0x0044 /* LATIN CAPITAL LETTER D */ ,
    0x0045 /* LATIN CAPITAL LETTER E */ ,
    0x0046 /* LATIN CAPITAL LETTER F */ ,
    0x0047 /* LATIN CAPITAL LETTER G */ ,
    0x0048 /* LATIN CAPITAL LETTER H */ ,
    0x0049 /* LATIN CAPITAL LETTER I */ ,
    0x004A /* LATIN CAPITAL LETTER J */ ,
    0x004B /* LATIN CAPITAL LETTER K */ ,
    0x004C /* LATIN CAPITAL LETTER L */ ,
    0x004D /* LATIN CAPITAL LETTER M */ ,
    0x004E /* LATIN CAPITAL LETTER N */ ,
    0x004F /* LATIN CAPITAL LETTER O */ ,
    0x0050 /* LATIN CAPITAL LETTER P */ ,
    0x0051 /* LATIN CAPITAL LETTER Q */ ,
    0x0052 /* LATIN CAPITAL LETTER R */ ,
    0x0053 /* LATIN CAPITAL LETTER S */ ,
    0x0054 /* LATIN CAPITAL LETTER T */ ,
    0x0055 /* LATIN CAPITAL LETTER U */ ,
    0x0056 /* LATIN CAPITAL LETTER V */ ,
    0x0057 /* LATIN CAPITAL LETTER W */ ,
    0x0058 /* LATIN CAPITAL LETTER X */ ,
    0x0059 /* LATIN CAPITAL LETTER Y */ ,
    0x005A /* LATIN CAPITAL LETTER Z */ ,
    0x00C4 /* LATIN CAPITAL LETTER A WITH DIAERESIS */ ,
    0x00D6 /* LATIN CAPITAL LETTER O WITH DIAERESIS */ ,
    0x00D1 /* LATIN CAPITAL LETTER N WITH TILDE */ ,
    0x00DC /* LATIN CAPITAL LETTER U WITH DIAERESIS */ ,
    0x00A7 /* SECTION SIGN */ ,
    0x00BF /* INVERTED QUESTION MARK */ ,
    0x0061 /* LATIN SMALL LETTER A */ ,
    0x0062 /* LATIN SMALL LETTER B */ ,
    0x0063 /* LATIN SMALL LETTER C */ ,
    0x0064 /* LATIN SMALL LETTER D */ ,
    0x0065 /* LATIN SMALL LETTER E */ ,
    0x0066 /* LATIN SMALL LETTER F */ ,
    0x0067 /* LATIN SMALL LETTER G */ ,
    0x0068 /* LATIN SMALL LETTER H */ ,
    0x0069 /* LATIN SMALL LETTER I */ ,
    0x006A /* LATIN SMALL LETTER J */ ,
    0x006B /* LATIN SMALL LETTER K */ ,
    0x006C /* LATIN SMALL LETTER L */ ,
    0x006D /* LATIN SMALL LETTER M */ ,
    0x006E /* LATIN SMALL LETTER N */ ,
    0x006F /* LATIN SMALL LETTER O */ ,
    0x0070 /* LATIN SMALL LETTER P */ ,
    0x0071 /* LATIN SMALL LETTER Q */ ,
    0x0072 /* LATIN SMALL LETTER R */ ,
    0x0073 /* LATIN SMALL LETTER S */ ,
    0x0074 /* LATIN SMALL LETTER T */ ,
    0x0075 /* LATIN SMALL LETTER U */ ,
    0x0076 /* LATIN SMALL LETTER V */ ,
    0x0077 /* LATIN SMALL LETTER W */ ,
    0x0078 /* LATIN SMALL LETTER X */ ,
    0x0079 /* LATIN SMALL LETTER Y */ ,
    0x007A /* LATIN SMALL LETTER Z */ ,
    0x00E4 /* LATIN SMALL LETTER A WITH DIAERESIS */ ,
    0x00F6 /* LATIN SMALL LETTER O WITH DIAERESIS */ ,
    0x00F1 /* LATIN SMALL LETTER N WITH TILDE */ ,
    0x00FC /* LATIN SMALL LETTER U WITH DIAERESIS */ ,
    0x00E0 /* LATIN SMALL LETTER A WITH GRAVE */,
    // index 128+
    0x000C /* FORM FEED */ ,
    0x005E /* CIRCUMFLEX ACCENT */ ,
    0x007B /* LEFT CURLY BRACKET */ ,
    0x007D /* RIGHT CURLY BRACKET */ ,
    0x005C /* REVERSE SOLIDUS */ ,
    0x005B /* LEFT SQUARE BRACKET */ ,
    0x007E /* TILDE */ ,
    0x005D /* RIGHT SQUARE BRACKET */ ,
    0x007C /* VERTICAL LINE */ ,
    0x20AC /* EURO SIGN */ ,
//     0x00A4 /* EURO SIGN */,
};

const QChar extchars[]={
    0x0A,
    0x14,
    0x28,
    0x29,
    0x2f,
    0x3c,
    0x3d,
    0x3e,
    0x40,
    0x65,
};
