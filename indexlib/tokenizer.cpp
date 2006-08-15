/* This file is part of indexlib.
 * Copyright (C) 2005 Lu�s Pedro Coelho <luis@luispedro.org>
 *
 * Indexlib is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation and available as file
 * GPL_V2 which is distributed along with indexlib.
 *
 * Indexlib is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of this program with any edition of
 * the Qt library by Trolltech AS, Norway (or with modified versions
 * of Qt that use the same license as Qt), and distribute linked
 * combinations including the two.  You must obey the GNU General
 * Public License in all respects for all of the code used other than
 * Qt.  If you modify this file, you may extend this exception to
 * your version of the file, but you are not obligated to do so.  If
 * you do not wish to do so, delete this exception statement from
 * your version.
 */

#include "tokenizer.h"
#include <algorithm>
#include <vector>
#include <string>
#include <cassert>

using std::string;
using std::vector;

namespace {
vector<string> split( const char* str, const char delim ) {
	assert( str );
	vector<string> res;
	while ( *str == delim ) ++str;
	while ( *str ) {
		const char* start = str++;
		while ( *str && *str != delim ) ++str;
		res.push_back( string( start, str ) );
		while ( *str == delim ) ++str;
	}
	return res;
}

class latin1_tokenizer : public indexlib::detail::tokenizer {
	private:
		static const char stop = 46; // .
		static void normalize( char& c ) {
			const char result[] = {
				stop, //  [ 0 ]
				stop, //  [ 1 ]
				stop, //  [ 2 ]
				stop, //  [ 3 ]
				stop, //  [ 4 ]
				stop, //  [ 5 ]
				stop, //  [ 6 ]
				stop, //  [ 7 ]
				stop, // ^H [ 8 ]
				stop, // \t [ 9 ]
				stop, // [ 10 ]
				stop, //  [ 11 ]
				stop, //  [ 12 ]
				stop, //  [ 13 ]
				stop, //  [ 14 ]
				stop, //  [ 15 ]
				stop, //  [ 16 ]
				stop, //  [ 17 ]
				stop, //  [ 18 ]
				stop, //  [ 19 ]
				stop, //  [ 20 ]
				stop, //  [ 21 ]
				stop, //  [ 22 ]
				stop, //  [ 23 ]
				stop, //  [ 24 ]
				stop, //  [ 25 ]
				stop, //  [ 26 ]
				stop, //  [ 27 ]
				stop, //  [ 28 ]
				stop, //  [ 29 ]
				stop, //  [ 30 ]
				stop, //  [ 31 ]
				stop, //  [ 32 ]
				stop, // ! [ 33 ]
				stop, // " [ 34 ]
				stop, // # [ 35 ]
				stop, // $ [ 36 ]
				stop, // % [ 37 ]
				stop, // & [ 38 ]
				stop, // ' [ 39 ]
				stop, // ( [ 40 ]
				stop, // ) [ 41 ]
				stop, // * [ 42 ]
				stop, // + [ 43 ]
				stop, // , [ 44 ]
				stop, // - [ 45 ]
				stop, // . [ 46 ]
				stop, // / [ 47 ]
				'0', // 0 [ 48 ]
				'1', // 1 [ 49 ]
				'2', // 2 [ 50 ]
				'3', // 3 [ 51 ]
				'4', // 4 [ 52 ]
				'5', // 5 [ 53 ]
				'6', // 6 [ 54 ]
				'7', // 7 [ 55 ]
				'8', // 8 [ 56 ]
				'9', // 9 [ 57 ]
				stop, // : [ 58 ]
				stop, // ; [ 59 ]
				stop, // < [ 60 ]
				stop, // = [ 61 ]
				stop, // > [ 62 ]
				stop, // ? [ 63 ]
				stop, // @ [ 64 ]
				'A', // A [ 65 ]
				'B', // B [ 66 ]
				'C', // C [ 67 ]
				'D', // D [ 68 ]
				'E', // E [ 69 ]
				'F', // F [ 70 ]
				'G', // G [ 71 ]
				'H', // H [ 72 ]
				'I', // I [ 73 ]
				'J', // J [ 74 ]
				'K', // K [ 75 ]
				'L', // L [ 76 ]
				'M', // M [ 77 ]
				'N', // N [ 78 ]
				'O', // O [ 79 ]
				'P', // P [ 80 ]
				'Q', // Q [ 81 ]
				'R', // R [ 82 ]
				'S', // S [ 83 ]
				'T', // T [ 84 ]
				'U', // U [ 85 ]
				'V', // V [ 86 ]
				'W', // W [ 87 ]
				'X', // X [ 88 ]
				'Y', // Y [ 89 ]
				'Z', // Z [ 90 ]
				stop, // [ [ 91 ]
				stop, // \ [ 92 ]
				stop, // ] [ 93 ]
				stop, // ^ [ 94 ]
				stop, // _ [ 95 ]
				stop, // ` [ 96 ]
				'A', // a [ 97 ]
				'B', // b [ 98 ]
				'C', // c [ 99 ]
				'D', // d [ 100 ]
				'E', // e [ 101 ]
				'F', // f [ 102 ]
				'G', // g [ 103 ]
				'H', // h [ 104 ]
				'I', // i [ 105 ]
				'J', // j [ 106 ]
				'K', // k [ 107 ]
				'L', // l [ 108 ]
				'M', // m [ 109 ]
				'N', // n [ 110 ]
				'O', // o [ 111 ]
				'P', // p [ 112 ]
				'Q', // q [ 113 ]
				'R', // r [ 114 ]
				'S', // s [ 115 ]
				'T', // t [ 116 ]
				'U', // u [ 117 ]
				'V', // v [ 118 ]
				'W', // w [ 119 ]
				'X', // x [ 120 ]
				'Y', // y [ 121 ]
				'Z', // z [ 122 ]
				stop, // { [ 123 ]
				stop, // | [ 124 ]
				stop, // } [ 125 ]
				stop, // ~ [ 126 ]
				stop, //  [ 127 ]
				stop, // � [ 128 ]
				stop, // � [ 129 ]
				stop, // � [ 130 ]
				stop, // � [ 131 ]
				stop, // � [ 132 ]
				stop, // � [ 133 ]
				stop, // � [ 134 ]
				stop, // � [ 135 ]
				stop, // � [ 136 ]
				stop, // � [ 137 ]
				stop, // � [ 138 ]
				stop, // � [ 139 ]
				stop, // � [ 140 ]
				stop, // � [ 141 ]
				stop, // � [ 142 ]
				stop, // � [ 143 ]
				stop, // � [ 144 ]
				stop, // � [ 145 ]
				stop, // � [ 146 ]
				stop, // � [ 147 ]
				stop, // � [ 148 ]
				stop, // � [ 149 ]
				stop, // � [ 150 ]
				stop, // � [ 151 ]
				stop, // � [ 152 ]
				stop, // � [ 153 ]
				stop, // � [ 154 ]
				stop, // � [ 155 ]
				stop, // � [ 156 ]
				stop, // � [ 157 ]
				stop, // � [ 158 ]
				stop, // � [ 159 ]
				stop, // � [ 160 ]
				stop, // � [ 161 ]
				stop, // � [ 162 ]
				stop, // � [ 163 ]
				stop, // � [ 164 ]
				stop, // � [ 165 ]
				stop, // � [ 166 ]
				stop, // � [ 167 ]
				stop, // � [ 168 ]
				stop, // � [ 169 ]
				stop, // � [ 170 ]
				stop, // � [ 171 ]
				stop, // � [ 172 ]
				stop, // � [ 173 ]
				stop, // � [ 174 ]
				stop, // � [ 175 ]
				stop, // � [ 176 ]
				stop, // � [ 177 ]
				stop, // � [ 178 ]
				stop, // � [ 179 ]
				stop, // � [ 180 ]
				stop, // � [ 181 ]
				stop, // � [ 182 ]
				stop, // � [ 183 ]
				stop, // � [ 184 ]
				stop, // � [ 185 ]
				stop, // � [ 186 ]
				stop, // � [ 187 ]
				stop, // � [ 188 ]
				stop, // � [ 189 ]
				stop, // � [ 190 ]
				stop, // � [ 191 ]
				'A', // � [ 192 ]
				'A', // � [ 193 ]
				'A', // � [ 194 ]
				'A', // � [ 195 ]
				'A', // � [ 196 ]
				'A', // � [ 197 ]
				'A', // � [ 198 ]
				'C', // � [ 199 ]
				'E', // � [ 200 ]
				'E', // � [ 201 ]
				'E', // � [ 202 ]
				'E', // � [ 203 ]
				'I', // � [ 204 ]
				'I', // � [ 205 ]
				'I', // � [ 206 ]
				'I', // � [ 207 ]
				'D', // � [ 208 ]
				'N', // � [ 209 ]
				'O', // � [ 210 ]
				'O', // � [ 211 ]
				'O', // � [ 212 ]
				'O', // � [ 213 ]
				'O', // � [ 214 ]
				'X', // � [ 215 ]
				'O', // � [ 216 ]
				'U', // � [ 217 ]
				'U', // � [ 218 ]
				'U', // � [ 219 ]
				'U', // � [ 220 ]
				'Y', // � [ 221 ]
				'T', // � [ 222 ]
				'S', // � [ 223 ]
				'A', // � [ 224 ]
				'A', // � [ 225 ]
				'A', // � [ 226 ]
				'A', // � [ 227 ]
				'A', // � [ 228 ]
				'A', // � [ 229 ]
				'A', // � [ 230 ]
				'C', // � [ 231 ]
				'E', // � [ 232 ]
				'E', // � [ 233 ]
				'E', // � [ 234 ]
				'E', // � [ 235 ]
				'I', // � [ 236 ]
				'I', // � [ 237 ]
				'I', // � [ 238 ]
				'I', // � [ 239 ]
				stop, // � [ 240 ]
				'N', // � [ 241 ]
				'O', // � [ 242 ]
				'O', // � [ 243 ]
				'O', // � [ 244 ]
				'O', // � [ 245 ]
				'O', // � [ 246 ]
				stop, // � [ 247 ]
				'O', // � [ 248 ]
				'U', // � [ 249 ]
				'U', // � [ 250 ]
				'U', // � [ 251 ]
				'U', // � [ 252 ]
				'Y', // � [ 253 ]
				'T', // � [ 254 ]
				'Y' // � [ 255 ]
			};
			c = result[ static_cast<unsigned char>( c ) ];
		}
		std::vector<std::string> do_string_to_words( const char* str ) {
			string complete = str;
			std::for_each( complete.begin(), complete.end(), normalize );
			return split( complete.c_str(), stop );
		}
};
}


std::auto_ptr<indexlib::detail::tokenizer> indexlib::detail::get_tokenizer( std::string name ) {
	if ( name == "latin-1:european" ) return std::auto_ptr<indexlib::detail::tokenizer>( new latin1_tokenizer );
	return std::auto_ptr<indexlib::detail::tokenizer>( 0 );
}
