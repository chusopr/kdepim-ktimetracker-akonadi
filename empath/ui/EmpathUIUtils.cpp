/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
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

#ifdef __GNUG__
# pragma implementation "EmpathUIUtils.h"
#endif

// Qt includes
#include <qstring.h>

// KDE includes
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>

// Local includes
#include "EmpathUIUtils.h"

QPixmap empathIcon(const QString & name)
{
    return KGlobal::iconLoader()->loadIcon(name);
}

QIconSet empathIconSet(const QString & name)
{
    QIconSet iconSet(empathIcon("menu-" + name), QIconSet::Small);
    iconSet.setPixmap(empathIcon("toolbar-" + name), QIconSet::Large);
    return iconSet;
}

// vim:ts=4:sw=4:tw=78
