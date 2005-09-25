/*
    This file is part of kdepim.

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

#include "printer.h"

#include <kdebug.h>
#include <ksavefile.h>

#include <qfile.h>
#include <qtextstream.h>

using namespace KODE;

Printer::Printer()
  : mCreationWarning( false ), mGenerator( "libkode" )
{
}

Printer::Printer( const Style &style )
  : mStyle( style ), mCreationWarning( false ), mGenerator( "libkode" )
{
}

void Printer::setCreationWarning( bool v )
{
  mCreationWarning = v;
}

void Printer::setGenerator( const QString &g )
{
  mGenerator = g;
}

void Printer::setOutputDirectory( const QString &o )
{
  mOutputDirectory = o;
}

void Printer::setSourceFile( const QString &s )
{
  mSourceFile = s;
}

QString Printer::functionSignature( const Function &f,
                                    const QString &className,
                                    bool includeClassQualifier )
{
  QString s;
  
  if ( f.isStatic() && !includeClassQualifier ) {
    s += "static ";
  }

  QString ret = f.returnType();
  if ( !ret.isEmpty() ) {
    s += ret;
    if ( ret.right( 1 ) != "*" && ret.right( 1 ) != "&" ) {
      s += " ";
    }
  }
  if ( includeClassQualifier ) {
    s += mStyle.className( className ) + "::";
  }
  if ( className == f.name() ) {
    // Constructor
    s += mStyle.className( f.name() );
  } else {
    s += f.name();
  }
  s += "(";
  if ( !f.arguments().isEmpty() ) {
    s += " " + f.arguments().join( ", " ) + " ";
  }
  s += ")";
  if ( f.isConst() ) s += " const";

  return s;
}

QString Printer::creationWarning()
{
  // Create warning about generated file
  QString str = "// This file is generated by " + mGenerator;
  if ( !mSourceFile.isEmpty() ) {
    str += " from " + mSourceFile;
  }
  str += ".\n";
  
  str += "// All changes you do to this file will be lost.";

  return str;
}

QString Printer::licenseHeader( const File &file )
{
  Code code;
  code += "/*";
  code.setIndent( 4 );

  code += "This file is part of " + file.project() + ".";
  code.newLine();
  
  QStringList copyrights = file.copyrightStrings();
  if ( !copyrights.isEmpty() ) {
    code.addBlock( copyrights.join( "\n" ) );
    code.newLine();
  }
  
  code.addBlock( file.license().text() );

  code.setIndent( 0 );
  code += "*/";
  
  return code.text();
}

Code Printer::functionHeaders( const Function::List &functions,
                               const QString &className,
                               int access )
{
  bool needNewLine = false;
  bool hasAccess = false;

  Code code;

  Function::List::ConstIterator it;
  for( it = functions.begin(); it != functions.end(); ++it ) {
    Function f = *it;
    if ( f.access() == access ) {
      if ( !hasAccess ) {
        code += f.accessAsString() + ":";
        hasAccess = true;
      }
      code.indent();
      if ( !(*it).docs().isEmpty() ) {
        code += "/**";
        code.indent();
        code.addFormattedText( (*it).docs() );
        code.unindent();
        code += "*/";
      }
      code += functionSignature( *it, className ) + ";";
      code.unindent();
      needNewLine = true;
    }
  }
  if ( needNewLine ) code.newLine();

  return code;
}

QString Printer::classHeader( const Class &c )
{
  Code code;

  if ( !c.docs().isEmpty() ) {
    code += "/**";
    code.indent();
    code.addFormattedText( c.docs() );
    code.unindent();
    code += "*/";
  }

  QString txt = "class " + mStyle.className( c.name() );

  Class::List baseClasses = c.baseClasses();
  if ( !baseClasses.isEmpty() ) {
    txt += " : ";
    Class::List::ConstIterator it;
    for( it = baseClasses.begin(); it != baseClasses.end(); ++it ) {
      Class bc = *it;
      
      if ( it != baseClasses.begin() ) txt +=", ";
      txt += "public ";
      if ( !bc.nameSpace().isEmpty() ) txt += bc.nameSpace() + "::";
      txt += bc.name();
    }
  }
  code += txt;

  code += "{";
  code.indent();

  if ( c.isQObject() ) {
    code += "Q_OBJECT";
    code.newLine();
  }

  Function::List functions = c.functions();

  Typedef::List typedefs = c.typedefs();
  if ( typedefs.count() > 0 ) {
    code += "public:";
    code.indent();
    Typedef::List::ConstIterator it;
    for( it = typedefs.begin(); it != typedefs.end(); ++it ) {
      code += (*it).declaration();
    }
    code.unindent();
    code.newLine();
  }

  Enum::List enums = c.enums();
  if ( enums.count() > 0 ) {
    code += "public:";
    code.indent();
    Enum::List::ConstIterator it;
    for( it = enums.begin(); it != enums.end(); ++it ) {
      code += (*it).declaration();
    }
    code.unindent();
    code.newLine();
  }

  code.addBlock( functionHeaders( functions, c.name(), Function::Public ) );
  code.addBlock( functionHeaders( functions, c.name(), Function::Public | Function::Slot ) );
  code.addBlock( functionHeaders( functions, c.name(), Function::Signal ) );
  code.addBlock( functionHeaders( functions, c.name(), Function::Protected ) );
  code.addBlock( functionHeaders( functions, c.name(), Function::Protected | Function::Slot ) );
  code.addBlock( functionHeaders( functions, c.name(), Function::Private ) );
  code.addBlock( functionHeaders( functions, c.name(), Function::Private | Function::Slot ) );

  if ( !c.memberVariables().isEmpty() ) {
    Function::List::ConstIterator it;
    for( it = functions.begin(); it != functions.end(); ++it ) {
      if ( (*it).access() == Function::Private ) break;
    }
    if ( it == functions.end() ) code += "private:";

    code.indent();

    MemberVariable::List variables = c.memberVariables();
    MemberVariable::List::ConstIterator it2;
    for( it2 = variables.begin(); it2 != variables.end(); ++it2 ) {
      MemberVariable v = *it2;

      QString decl;
      if ( v.isStatic() ) decl += "static ";
      decl += v.type();
      if ( v.type().right( 1 ) != "*" && v.type().right( 1 ) != "&" ) {
        decl += " ";
      }
      decl += v.name() + ";";

      code += decl;
    }
  }

  code.setIndent( 0 );
  code += "};";

  return code.text();
}

QString Printer::classImplementation( const Class &c )
{
  Code code;

  bool needNewLine = false;

  MemberVariable::List vars = c.memberVariables();
  MemberVariable::List::ConstIterator itV;
  for( itV = vars.begin(); itV != vars.end(); ++itV ) {
    MemberVariable v = *itV;
    if ( !v.isStatic() ) continue;
    code += v.type() + c.name() + "::" + v.name() + " = " + v.initializer() +
            ";";
    needNewLine = true;
  }
  if ( needNewLine ) code.newLine();
  
  Function::List functions = c.functions();
  Function::List::ConstIterator it;
  for( it = functions.begin(); it != functions.end(); ++it ) {
    Function f = *it;

    // Omit signals
    if ( f.access() == Function::Signal )
      continue;

    code += functionSignature( f, c.name(), true );

    if ( !f.initializers().isEmpty() ) {
      code += ": " + f.initializers().join( ", " );
    }

    code += "{";
    code.addBlock( f.body(), 2 );
    code += "}";
    code += "";
  }

  if ( c.isQObject() ) {
    code.newLine();
    code += "#include \"" + c.name().toLower() + ".moc\"";
  }

  return code.text();
}

void Printer::printHeader( const File &f )
{
  Code out;

  if ( mCreationWarning ) out += creationWarning();

  out.addBlock( licenseHeader( f ) );

  // Create include guard
  QString className = f.filename();
  className.replace( "-", "_" );

  QString includeGuard;
  if ( !f.nameSpace().isEmpty() ) includeGuard += f.nameSpace().toUpper() + "_";
  includeGuard += className.toUpper() + "_H";

  out += "#ifndef " + includeGuard;
  out += "#define " + includeGuard;
  
  out.newLine();


  // Create includes
  QStringList processed;
  Class::List classes = f.classes();
  Class::List::ConstIterator it;
  for( it = classes.begin(); it != classes.end(); ++it ) {
    QStringList includes = (*it).headerIncludes();
    QStringList::ConstIterator it2;
    for( it2 = includes.begin(); it2 != includes.end(); ++it2 ) {
      if ( processed.find( *it2 ) == processed.end() ) {
        out += "#include <" + *it2 + ">";
        processed.append( *it2 );
      }
    }
  }
  if ( !processed.isEmpty() ) out.newLine();


  // Create forward declarations
  processed.clear();
  for( it = classes.begin(); it != classes.end(); ++it ) {
    QStringList decls = (*it).forwardDeclarations();
    QStringList::ConstIterator it2;
    for( it2 = decls.begin(); it2 != decls.end(); ++it2 ) {
      if ( processed.find( *it2 ) == processed.end() ) {
        out += "class " + *it2 + ";";
        processed.append( *it2 );
      }
    }
  }
  if ( !processed.isEmpty() ) out.newLine();


  if ( !f.nameSpace().isEmpty() ) {
    out += "namespace " + f.nameSpace() + " {";
    out.newLine();
  }

  // Create content
  for( it = classes.begin(); it != classes.end(); ++it ) {
    out.addBlock( classHeader( *it ) );
    out.newLine();
  }

  if ( !f.nameSpace().isEmpty() ) {
    out += "}";
    out.newLine();
  }

  // Finish file
  out += "#endif";


  // Print to file
  QString filename = f.filename() + ".h";

  if ( !mOutputDirectory.isEmpty() ) filename.prepend( mOutputDirectory + "/" );

  KSaveFile::backupFile( filename, QString::null, ".backup" );

  QFile header( filename );
  if ( !header.open( QIODevice::WriteOnly ) ) {
    kdError() << "Can't open '" << filename << "' for writing." << endl;
    return;
  }

  QTextStream h( &header );

  h << out.text();

  header.close();
}

void Printer::printImplementation( const File &f, bool createHeaderInclude )
{
  Code out;

  if ( mCreationWarning ) out += creationWarning();

  out.addBlock( licenseHeader( f ) );

  out.newLine();

  // Create includes
  if ( createHeaderInclude ) {
    out += "#include \"" + f.filename() + ".h\"";
    out.newLine();
  }

  QStringList includes = f.includes();
  QStringList::ConstIterator it2;
  for( it2 = includes.begin(); it2 != includes.end(); ++it2 ) {
    out += "#include <" + *it2 + ">";
  }
  if ( !includes.isEmpty() ) out.newLine();

  // Create class includes
  QStringList processed;
  Class::List classes = f.classes();
  Class::List::ConstIterator it;
  for( it = classes.begin(); it != classes.end(); ++it ) {
    QStringList includes = (*it).includes();
    QStringList::ConstIterator it2;
    for( it2 = includes.begin(); it2 != includes.end(); ++it2 ) {
      if ( processed.find( *it2 ) == processed.end() ) {
        out += "#include <" + *it2 + ">";
        processed.append( *it2 );
      }
    }
  }
  if ( !processed.isEmpty() ) out.newLine();

  if ( !f.nameSpace().isEmpty() ) {
    out += "using namespace " + f.nameSpace() + ";";
    out.newLine();
  }

  // 'extern "C"' declarations
  QStringList externCDeclarations = f.externCDeclarations();
  if ( !externCDeclarations.isEmpty() ) {
    out += "extern \"C\" {";
    QStringList::ConstIterator it;
    for( it = externCDeclarations.begin(); it != externCDeclarations.end();
         ++it ) {
      out += *it + ";";
    }
    out += "}";
    out.newLine();
  }

  // File variables
  Variable::List vars = f.fileVariables();
  Variable::List::ConstIterator itV;
  for( itV = vars.begin(); itV != vars.end(); ++itV ) {
    Variable v = *itV;
    QString str;
    if ( v.isStatic() ) str += "static ";
    str += v.type() + " " + v.name() + ";";
    out += str;
  }
  if ( !vars.isEmpty() ) out.newLine();

  // File code
  if ( !f.fileCode().isEmpty() ) {
    out += f.fileCode();
    out.newLine();
  }

  // File functions
  Function::List funcs = f.fileFunctions();
  Function::List::ConstIterator itF;
  for( itF = funcs.begin(); itF != funcs.end(); ++itF ) {
    Function f = *itF;
    out += functionSignature( f );
    out += "{";
    out.addBlock( f.body(), 2 );
    out += "}";
    out.newLine();
  }

  // Classes
  for( it = classes.begin(); it != classes.end(); ++it ) {
    QString str = classImplementation( *it );
    if ( !str.isEmpty() ) out += classImplementation( *it );
  }

  // Print to file
  QString filename = f.filename() + ".cpp";

  if ( !mOutputDirectory.isEmpty() ) filename.prepend( mOutputDirectory + "/" );

  KSaveFile::backupFile( filename, QString::null, ".backup" );

  QFile implementation( filename );
  if ( !implementation.open( QIODevice::WriteOnly ) ) {
    kdError() << "Can't open '" << filename << "' for writing." << endl;
    return;
  }

  QTextStream h( &implementation );

  h << out.text();

  implementation.close();
}

void Printer::printAutoMakefile( const AutoMakefile &am )
{
  QString filename = "Makefile.am";

  if ( !mOutputDirectory.isEmpty() ) filename.prepend( mOutputDirectory + "/" );

  KSaveFile::backupFile( filename, QString::null, ".backup" );

  QFile file( filename );
  if ( !file.open( QIODevice::WriteOnly ) ) {
    kdError() << "Can't open '" << filename << "' for writing." << endl;
    return;
  }

  QTextStream ts( &file );

  ts << am.text();
}
