/*
    This file is part of libkdepim.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "kconfigpropagator.h"

#include <kdebug.h>
#include <kconfig.h>
#include <kconfigskeleton.h>

#include <qfile.h>
#include <qstringlist.h>

KConfigPropagator::KConfigPropagator( KConfigSkeleton *skeleton,
                                      const QString &kcfgFile )
  : mSkeleton( skeleton ), mKcfgFile( kcfgFile )
{
  readKcfgFile();
}

void KConfigPropagator::readKcfgFile()
{
  QFile input( mKcfgFile );
  QDomDocument doc;
  QString errorMsg;
  int errorRow;
  int errorCol;
  if ( !doc.setContent( &input, &errorMsg, &errorRow, &errorCol ) ) {
    kdError() << "Parse error in " << mKcfgFile << ", line " << errorRow << ", col " << errorCol << ": " << errorMsg << endl;
    return;
  }

  QDomElement cfgElement = doc.documentElement();

  if ( cfgElement.isNull() ) {
    kdError() << "No document in kcfg file" << endl;
    return;
  }

  mRules.clear();

  QDomNode n;
  for ( n = cfgElement.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    QDomElement e = n.toElement();

    QString tag = e.tagName();

    if ( tag == "propagation" ) {
      Rule rule = parsePropagation( e );
      mRules.append( rule );
    } else if ( tag == "condition" ) {
      Condition condition = parseCondition( e );
      QDomNode n2;
      for( n2 = e.firstChild(); !n2.isNull(); n2 = n2.nextSibling() ) {
        QDomElement e2 = n2.toElement();
        if ( e2.tagName() == "propagation" ) {
          Rule rule = parsePropagation( e2 );
          rule.condition = condition;
          mRules.append( rule );
        } else {
          kdError() << "Unknow tag: " << e2.tagName() << endl;
        }
      }
    }
  }
}

KConfigPropagator::Rule KConfigPropagator::parsePropagation( const QDomElement &e )
{
  Rule r;

  QString source = e.attribute( "source" );
  parseConfigEntryPath( source, r.sourceFile, r.sourceGroup, r.sourceEntry );

  QString target = e.attribute( "target" );
  parseConfigEntryPath( target, r.targetFile, r.targetGroup, r.targetEntry );

  return r;
}

void KConfigPropagator::parseConfigEntryPath( const QString &path,
                                              QString &file,
                                              QString &group,
                                              QString &entry )
{
  QStringList p = QStringList::split( "/", path );

  if ( p.count() != 3 ) {
    kdError() << "Path has to be of form file/group/entry" << endl;
    file = QString::null;
    group = QString::null;
    entry = QString::null;
    return;
  }
  
  file = p[ 0 ];
  group = p[ 1 ];  
  entry = p[ 2 ];
  
  return;
}

KConfigPropagator::Condition KConfigPropagator::parseCondition( const QDomElement &e )
{
  Condition c;
  
  QString key = e.attribute( "key" );
  
  parseConfigEntryPath( key, c.file, c.group, c.key );
  
  c.value = e.attribute( "value" );

  c.isValid = true;

  return c;
}

void KConfigPropagator::commit()
{
  Change::List changeList = changes();
  
  Change::List::ConstIterator it;
  for( it = changeList.begin(); it != changeList.end(); ++it ) {
    Change c = *it;
    KConfig cfg( c.file );
    cfg.setGroup( c.group );
    cfg.writeEntry( c.name, c.value );
    
    cfg.sync();
  }
}

KConfigSkeletonItem *KConfigPropagator::findItem( const QString &group,
                                                  const QString &name )
{
//  kdDebug() << "KConfigPropagator::findItem()" << endl;

  KConfigSkeletonItem::List items = mSkeleton->items();
  KConfigSkeletonItem::List::ConstIterator it;
  for( it = items.begin(); it != items.end(); ++it ) {
//    kdDebug() << "  Item: " << (*it)->name() << "  Type: "
//              << (*it)->property().typeName() << endl;
    if ( (*it)->group() == group && (*it)->name() == name ) {
      break;
    }
  }
  if ( it == items.end() ) return 0;
  else return *it;
}

QString KConfigPropagator::itemValueAsString( KConfigSkeletonItem *item )
{
  QVariant p = item->property();

  if ( p.type() == QVariant::Bool ) {
    if ( p.toBool() ) return "true";
    else return "false";
  }
  
  return p.toString();
}

KConfigPropagator::Change::List KConfigPropagator::changes()
{
  Change::List changes;

  Rule::List::ConstIterator it;
  for( it = mRules.begin(); it != mRules.end(); ++it ) {
    Rule r = *it;
    Condition c = r.condition;
    if ( c.isValid ) {
      KConfigSkeletonItem *item = findItem( c.group, c.key );
      kdDebug() << "Item " << c.group << "/" << c.key << ":" << endl;
      if ( !item ) {
        kdError() << "  Item not found." << endl;
      } else {
        QString value = itemValueAsString( item );
        kdDebug() << "  Value: " << value << endl;
        if ( value != c.value ) {
          continue;
        }
      }
    }

    KConfigSkeletonItem *item = findItem( r.sourceGroup, r.sourceEntry );
    if ( !item ) {
      kdError() << "Item " << r.sourceGroup << "/" << r.sourceEntry 
                << " not found." << endl;
      continue;
    }
    QString value = itemValueAsString( item );

    KConfig target( r.targetFile );
    target.setGroup( r.targetGroup );
    QString targetValue = target.readEntry( r.targetEntry );
    if ( targetValue != value ) {
      Change change;
      change.file = r.targetFile;
      change.group = r.targetGroup;
      change.name = r.targetEntry;
      change.value = value;
      changes.append( change );
    }
  }

  return changes;
}

KConfigPropagator::Rule::List KConfigPropagator::rules()
{
  return mRules;
}
