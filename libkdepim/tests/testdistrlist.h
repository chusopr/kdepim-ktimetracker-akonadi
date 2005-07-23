/* This file is part of the KDE project
   Copyright (C) 2004 David Faure <faure@kde.org>

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

#ifndef TESTDISTRLIST_H
#define TESTDISTRLIST_H

#include <qobject.h>

class TestDistrList : public QObject
{
    Q_OBJECT

public:
    TestDistrList() {}
    void setup();
    void runAll();
    void cleanup();

    // tests
    void testEmpty();
    void testNewList();
    void testInsertEntry();
    void testRemoveEntry();
    void testDuplicate();
    void testDeleteList();

private:
    bool check(const QString& txt, QString a, QString b);
};

#endif
