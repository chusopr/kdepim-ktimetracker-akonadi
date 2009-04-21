/*
    This file is part of KContactManager.

    Copyright (c) 2009 Laurent Montel <montel@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef KCONTACTMANAGER_PART_H
#define KCONTACTMANAGER_PART_H

#include <kparts/event.h>
#include <kparts/part.h>

class KContactManagerPart: public KParts::ReadOnlyPart
{
  Q_OBJECT

  public:
    KContactManagerPart( QWidget *parentWidget, QObject *parent, const QVariantList& );
    virtual ~KContactManagerPart();
  protected:
    virtual bool openFile();
    virtual void guiActivateEvent( KParts::GUIActivateEvent* );
};


#endif /* KCONTACTMANAGER_PART_H */

