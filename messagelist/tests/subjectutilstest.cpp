/*
   Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   Author: Tobias Koenig <tobias.koenig@kdab.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <qtest_kde.h>

#include "subjectutils_p.h"

class SubjectUtilsTest: public QObject
{
  Q_OBJECT

  private Q_SLOTS:

  void testStripOffMessagePrefix()
  {
    const QString subject = QLatin1String( "Fwd: Hello World Subject" );
    QBENCHMARK {
      MessageList::Core::SubjectUtils::stripOffPrefixes( subject );
    }
  }
};

QTEST_KDEMAIN_CORE( SubjectUtilsTest )

#include "subjectutilstest.moc"
