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

#ifndef KOLABBASE_H
#define KOLABBASE_H

#include <qdom.h>
#include <qdatetime.h>
#include <qcolor.h>

class QFile;

namespace KCal {
  class Incidence;
}

namespace ResourceKolab {

class KolabBase {
public:
  enum Sensitivity { Public = 0, Private = 1, Confidential = 2 };

  KolabBase();
  virtual ~KolabBase();

  // Return a string identifying this type
  virtual QString type() const = 0;

  virtual void setUid( const QString& uid );
  virtual QString uid() const;

  virtual void setBody( const QString& body );
  virtual QString body() const;

  virtual void setCategories( const QString& categories );
  virtual QString categories() const;

  virtual void setCreationDate( const QDateTime& date );
  virtual QDateTime creationDate() const;

  virtual void setLastModified( const QDateTime& date );
  virtual QDateTime lastModified() const;

  virtual void setSensitivity( Sensitivity sensitivity );
  virtual Sensitivity sensitivity() const;

  // String - Date conversion methods
  static QString dateTimeToString( const QDateTime& time );
  static QString dateToString( const QDate& date );
  static QDateTime stringToDateTime( const QString& time );
  static QDate stringToDate( const QString& date );

  // String - Sensitivity conversion methods
  static QString sensitivityToString( Sensitivity );
  static Sensitivity stringToSensitivity( const QString& );

  // String - Color conversion methods
  static QString colorToString( const QColor& );
  static QColor stringToColor( const QString& );

  // Load this object by reading the XML file
  virtual bool load( const QString& xml );
  virtual bool load( QFile& xml );

  // Load this QDomDocument
  virtual bool load( const QDomDocument& xml ) = 0;

  // Serialize this object to an XML string
  virtual QString save() const = 0;

protected:
  // Read all known fields from this ical incidence
  void setFields( KCal::Incidence* );

  // This just makes the initial dom tree with version and doctype
  static QDomDocument domTree();

  // Load the attributes of this class
  virtual bool loadAttribute( QDomElement& );

  // Save the attributes of this class
  virtual bool saveAttributes( QDomElement& ) const;

  // Write a string tag
  static void writeString( QDomElement&, const QString&, const QString& );

  QString mUid;
  QString mBody;
  QString mCategories;
  QDateTime mCreationDate;
  QDateTime mLastModified;
  Sensitivity mSensitivity;
};

}

#endif // KOLABBASE_H
