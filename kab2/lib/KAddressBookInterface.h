/*
    KAddressBook version 2
    
    Copyright (C) 1999 The KDE PIM Team <kde-pim@kde.org>
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef KADDRESSBOOK_INTERFACE_H
#define KADDRESSBOOK_INTERFACE_H

#include <qstring.h>
#include <qstringlist.h>
#include <dcopobject.h>

#include <Entry.h>

class KAddressBookBackend;

class KAddressBookInterface : virtual public DCOPObject
{
  K_DCOP

  public:

    KAddressBookInterface(QString name, QString path);
    virtual ~KAddressBookInterface();

  k_dcop:

    virtual QString name();
    virtual QString path();
    virtual Entry   entry(QString);
    virtual QString insert(Entry);
    virtual bool    remove(QString);
    virtual bool    replace(Entry);
    virtual bool    contains(QString);

    virtual QStringList entryList();

  private:

    QString name_;
    QString path_;

    KAddressBookBackend * backend_;
};

#endif
