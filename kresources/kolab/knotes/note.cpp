/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

    Copyright (c) 2004 Bo Thorsen <bo@klaralvdalens-datakonsult.se>

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

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "note.h"

#include <libkcal/journal.h>
using namespace Kolab;


KCal::Journal* Note::xmlToJournal( const QString xml )
{
  // TODO: Well...
  return 0;
}

QString Note::journalToXML( KCal::Journal* journal )
{
  Note note( journal );
  return note.save();
}

Note::Note( KCal::Journal* journal )
{
  setFields( journal );
}

Note::~Note()
{
}

void Note::setSummary( const QString& summary )
{
  mSummary = summary;
}

QString Note::summary() const
{
  return mSummary;
}

void Note::setBackgroundColor( const QColor& bgColor )
{
  mBackgroundColor = bgColor;
}

QColor Note::backgroundColor() const
{
  return mBackgroundColor;
}

void Note::setForegroundColor( const QColor& fgColor )
{
  mForegroundColor = fgColor;
}

QColor Note::foregroundColor() const
{
  return mForegroundColor;
}

bool Note::loadAttribute( QDomElement& element )
{
  QString tagName = element.tagName();

  if ( tagName == "summary" )
    setSummary( element.text() );
  else if ( tagName == "foreground-color" )
    setForegroundColor( stringToColor( element.text() ) );
  else if ( tagName == "background-color" )
    setBackgroundColor( stringToColor( element.text() ) );
  else
    return KolabBase::loadAttribute( element );

  // We handled this
  return true;
}

bool Note::saveAttributes( QDomElement& element ) const
{
  // Save the base class elements
  KolabBase::saveAttributes( element );

  // Save the elements
#if 0
  QDomComment c = element.ownerDocument().createComment( "Note specific attributes" );
  element.appendChild( c );
#endif

  writeString( element, "summary", summary() );
  writeString( element, "foreground-color", colorToString( foregroundColor() ) );
  writeString( element, "background-color", colorToString( backgroundColor() ) );

  return true;
}


bool Note::load( const QDomDocument& document )
{
  QDomElement top = document.documentElement();

  if ( top.tagName() != "note" ) {
    qWarning( "XML error: Top tag was %s instead of the expected Note",
              top.tagName().ascii() );
    return false;
  }

  for ( QDomNode n = top.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    if ( n.isComment() )
      continue;
    if ( n.isElement() ) {
      QDomElement e = n.toElement();
      if ( !loadAttribute( e ) )
        // TODO: Unhandled tag - save for later storage
        qDebug( "Warning: Unhandled tag %s", e.tagName().ascii() );
    } else
      qDebug( "Node is not a comment or an element???" );
  }

  return true;
}

QString Note::save() const
{
  QDomDocument document = domTree();
  QDomElement element = document.createElement( "note" );
  element.setAttribute( "version", "1.0" );
  saveAttributes( element );
  document.appendChild( element );
  return document.toString();
}

void Note::setFields( KCal::Journal* journal )
{
  KolabBase::setFields( journal );

  // TODO: background and foreground
  setSummary( journal->summary() );
}
