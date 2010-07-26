/*
    Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
    Copyright (C) 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/
#ifndef _FREEBUSYITEMMODEL_TEST_H
#define _FREEBUSYITEMMODEL_TEST_H

#include <QtCore/QObject>

class FreeBusyItemModelTest: public QObject
{
Q_OBJECT
private Q_SLOTS:
    void testModelValidity();
    void testModelValidity2();
    void testInsertFreeBusy();
};
#endif //_FREEBUSYITEMMODEL_TEST_H