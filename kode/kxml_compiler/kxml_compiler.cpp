/*
    This file is part of KDE.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "parserrelaxng.h"
#include "parserxsd.h"
#include "creator.h"
#include "schema.h"

#include <libkode/code.h>
#include <libkode/printer.h>
#include <libkode/typedef.h>

#include <kaboutdata.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kurl.h>

#include <QFile>
#include <QTextStream>
#include <qdom.h>
#include <QRegExp>
#include <QMap>
#include <QList>
#include <QFileInfo>

#include <iostream>

int main( int argc, char **argv )
{
  QCoreApplication app( argc, argv );
  KAboutData aboutData( "kxml_compiler", 0, ki18n("KDE xml compiler"), "0.1",
  	ki18n("KDE XML Compiler") , KAboutData::License_LGPL );
  aboutData.addAuthor( ki18n("Cornelius Schumacher"), KLocalizedString(), "schumacher@kde.org" );
  KComponentData data( &aboutData );
  KGlobal::setActiveComponent( data );

  KCmdLineArgs::init( argc, argv, &aboutData, KCmdLineArgs::CmdLineArgNone );

  KCmdLineOptions options;
  options.add("d");
  options.add("directory <dir>", ki18n("Directory to generate files in"), ".");
  options.add("v");
  options.add("verbose", ki18n("Generate debug output"));
  options.add("+schema", ki18n("Schema of XML file"));
  options.add("external-parser", ki18n("Generate parser in separate source file"));
  options.add("custom-parser", ki18n("Generate parser customized for schema"));
  options.add("xsd", ki18n("Schema is XML Schema"));
  options.add("rng", ki18n("Schema is RelaxNG"));
  KCmdLineArgs::addCmdLineOptions( options );

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  bool verbose = args->isSet("verbose");

  if ( args->count() < 1 ) {
    kError() << "Too few arguments." << endl;
    return 1;
  }
  if ( args->count() > 1 ) {
    kError() << "Too many arguments." << endl;
    return 1;
  }

  QString baseDir = args->getOption( "directory" );
  if ( !baseDir.endsWith( "/" ) ) baseDir.append( "/" );

  QString schemaFilename = args->url( 0 ).path();

  QString baseName = args->url( 0 ).fileName();
  int pos = baseName.lastIndexOf( '.' );
  if ( pos > 0 ) baseName = baseName.left( pos );


  QFile schemaFile( schemaFilename );
  if ( !schemaFile.open( QIODevice::ReadOnly ) ) {
    kError() << "Unable to open '" << schemaFilename << "'" << endl;
    return 1;
  }

  if ( verbose ) {
    kDebug() << "Begin parsing" << endl;
  }

  Schema::Document schemaDocument;

  QFileInfo fi( schemaFile );
  if ( args->isSet( "xsd" ) || fi.suffix() == "xsd" ) {
    RNG::ParserXsd p;
    p.setVerbose( verbose );

    schemaDocument = p.parse( schemaFile );

    if ( schemaDocument.isEmpty() ) {
      kError() << "Error parsing schema '" << schemaFilename << "'" << endl;
      return 1;
    }
  } else if ( args->isSet( "rng" ) || fi.suffix() == "rng" ) {
    QString errorMsg;
    int errorLine, errorCol;
    QDomDocument doc;
    if ( !doc.setContent( &schemaFile, false, &errorMsg, &errorLine, &errorCol ) ) {
      kError() << errorMsg << " at " << errorLine << "," << errorCol << endl;
      return 1;
    }

    RNG::ParserRelaxng p;
    p.setVerbose( verbose );
    RNG::Element *start = p.parse( doc.documentElement() );
    if ( !start ) {
      kError() << "Could not find start element" << endl;
      return 1;
    }

    if ( verbose ) {
      p.dumpDefinitionMap();
    }

  //  return 0;

    p.substituteReferences( start );

  #if 1
    if ( verbose ) {
      std::cout << "--- TREE:" << std::endl;
      p.dumpTree( start );
    }
  #endif

    schemaDocument = p.convertToSchema( start );
  } else {
    kError() << "Unable to determine schema type." << endl;
    return 1;
  }

  if ( verbose ) {
    std::cout << "--- SCHEMA:" << std::endl;
    schemaDocument.dump();

    kDebug() << "Begin creating code" << endl;
  }

  Creator::XmlParserType pt;
  if ( args->isSet( "custom-parser" ) ) {
    pt = Creator::XmlParserCustomExternal;
  } else if ( args->isSet( "external-parser" ) ) {
    pt = Creator::XmlParserDomExternal;
  } else {
    pt = Creator::XmlParserDom;
  }

  Creator c( schemaDocument, pt );

  if ( verbose ) {
    kDebug() << "Create classes" << endl;
  }
  foreach( Schema::Element e, schemaDocument.usedElements() ) {
    c.createClass( e );
  }
  if ( verbose ) {
    kDebug() << "Create parser" << endl;
  }
  Schema::Element startElement = schemaDocument.startElement();
  c.setExternalClassPrefix( c.upperFirst( startElement.name() ) );
  c.createFileParser( startElement );
//  c.setDtd( schemaFilename.replace( "rng", "dtd" ) );
  c.createFileWriter( startElement );

  c.createListTypedefs();

  if ( verbose ) {
    kDebug() << "Begin printing code" << endl;
  }
  KODE::File &f = c.file();

  f.setFilename( baseName );

  KODE::Printer printer;
  printer.setCreationWarning( true );
  printer.setGenerator( aboutData.appName() );
  printer.setOutputDirectory( baseDir );
  printer.setSourceFile( args->url( 0 ).fileName() );

  c.printFiles( printer );

  if ( verbose ) {
    kDebug() << "Finished." << endl;
  }
}
