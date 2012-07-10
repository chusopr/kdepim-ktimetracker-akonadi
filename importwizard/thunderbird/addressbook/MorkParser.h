////////////////////////////////////////////////////////////////////
///
///   MorkParser.h - Mozilla Mork Format Parser/Reader 
///
///   Copyright (C) 2007 ScalingWeb.com
///   All rights reserved. 
/// 
///   Authors: Yuriy Soroka <ysoroka@scalingweb.com>
///	       Anton Fedoruk <afedoruk@scalingweb.com>
///
///
///////////////////////////////////////////////////////////////////


#ifndef __MorkParser_h__
#define __MorkParser_h__

#include <QMap>
#include <QByteArray>
#include <iostream>
class QString;
// Types

typedef QMap< int, std::string > MorkDict;
typedef QMap< int, int > MorkCells;					// ColumnId : ValueId
typedef QMap< int, MorkCells > MorkRowMap;			// Row id
typedef QMap< int, MorkRowMap > RowScopeMap;		// Row scope
typedef QMap< int, RowScopeMap > MorkTableMap;		// Table id
typedef QMap< int, MorkTableMap > TableScopeMap;	// Table Scope

// Mork header of supported format version
const char MorkMagicHeader[] = "// <!-- <mdb:mork:z v=\"1.4\"/> -->";

const char MorkDictColumnMeta[] = "<(a=c)>";

// Error codes
enum MorkErrors
{
	NoError = 0,
	FailedToOpen,
	UnsupportedVersion,
	DefectedFormat
};

// Mork term types
enum MorkTerm
{
	NoneTerm = 0,
	DictTerm,
	GroupTerm,
	TableTerm,
	RowTerm,
	CellTerm,
	CommentTerm,
	LiteralTerm
};


/// Class MorkParser

class MorkParser
{
public:

	MorkParser( int defaultScope = 0x80 );

	///
	/// Open and parse mork file

	bool open( const QString &path );

	///
	/// Return error status

    MorkErrors error() const;

	///
	/// Returns all tables of specified scope

	MorkTableMap *getTables( int tableScope );

	///
	/// Rerturns all rows under specified scope

	MorkRowMap *getRows( int rowScope, RowScopeMap *table );

	///
	/// Return value of specified value oid

	std::string &getValue( int oid );

	///
	/// Return value of specified column oid

	std::string &getColumn( int oid );


protected: // Members

	void initVars();

	bool isWhiteSpace( char c );
	char nextChar();

	void parseScopeId( const std::string &TextId, int *Id, int *Scope );
	void setCurrentRow( int TableScope, int TableId, int RowScope, int RowId );

	// Parse methods
	bool parse();
	bool parseDict();
	bool parseComment();
	bool parseCell();
	bool parseTable();
	bool parseMeta( char c );
	bool parseRow( int TableId, int TableScope );
	bool parseGroup();

protected: // Data

	// Columns in mork means value names
	MorkDict columns_;
	MorkDict values_;

	// All mork file data
	TableScopeMap mork_;
	MorkCells *currentCells_;

	// Error status of last operation
	MorkErrors error_;

	// All Mork data
	QByteArray morkData_;

	int morkPos_;
	int nextAddValueId_;
	int defaultScope_;

	// Indicates intity is being parsed
	enum { NPColumns, NPValues, NPRows } nowParsing_;

};

#endif // __MorkParser_h__


