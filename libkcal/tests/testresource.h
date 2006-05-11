/* This file is part of the KDE project
   Copyright (C) 2004 Till Adam <adam@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef TESTRESOURCE_H
#define TESTRESOURCE_H

#include <QObject>
class KConfig;

namespace KCal {

  class ResourceCalendar;

class TestResource : public QObject
{
    Q_OBJECT

public:
    TestResource( const QString& type, KConfig *config );
    void setup();
    void runAll();
    void cleanup();

    // tests
    void testOpenAndClose();
    void testResourceAttributes();
    void testResourceCalendarAttributes();
    
    void testEventAddRemove();
    void testTodoAddRemove();
    void testJournalAddRemove();

private:
    bool check(const QString& txt, QString a, QString b);
    QString m_resource_type;
    KConfig *m_config;
    ResourceCalendar *m_res;
};
}
#endif
