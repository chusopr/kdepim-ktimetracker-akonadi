#ifndef MAKEDOC_H
#define MAKEDOC_H
// based on: MakeDoc, version 2
// I only took the tBuf class from there and adapted it.
//
// Compresses text files into a format that is ready to export to a Pilot
// and work with Rick Bram's PilotDOC reader.
// Copyright (C) Reinhold Kainhofer, 2002
// Copyrigth (C) Pat Beirne, 2000
//
// Original file (makedoc9.cpp) copyright by:
// Copyright (C) Pat Beirne, 2000.
// Distributable under the GNU General Public License Version 2 or later.
//
// ver 0.6 enforce 31 char limit on database names
// ver 0.7 change header and record0 to structs
// ver 2.0 added category control on the command line
//              changed extensions from .prc to .pdb

#include <stdio.h>

typedef unsigned char byte;
typedef unsigned long DWORD;
typedef unsigned short WORD;

#define DISP_BITS 11
#define COUNT_BITS 3




/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////                                  //////////////////////
/////////////////////      tBuf class                  //////////////////////
/////////////////////                                  //////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


class tBuf {
 private:
//      byte hichar[10];
//      int hicharnum;
//      bool space;

	byte * buf;
	unsigned len;
	bool isCompressed;
 public:
	 tBuf() {
		buf = 0L;
		len=0;
		isCompressed=false;
	};

	~tBuf()
	{
		if (buf)
			delete[]buf;
	}

	void Clear() {
		delete[]buf;
		buf = 0L;
	}
	void setText(const byte * text, unsigned int txtlen =
		0, bool txtcomp = false);
	byte *text() const {
		return buf;
	}
	unsigned Len() const {
		return len;
	}
	void setCompressed(bool compressed = true) {
		isCompressed = compressed;
	}
	bool compressed() const {
		return isCompressed;
	}
	unsigned RemoveBinary();
	unsigned DuplicateCR();

	unsigned Decompress();
	unsigned Compress();

 private:
	unsigned Issue(byte src, int &bSpace);
	void Dump() const {
		printf("\nbuffer len=%d", len);
}};


#endif
