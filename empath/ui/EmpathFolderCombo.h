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

#ifndef EMPATH_FOLDER_COMBO_H
#define EMPATH_FOLDER_COMBO_H

// Qt includes
#include <qcombobox.h>

// Local includes
#include "EmpathURL.h"

class EmpathFolderCombo : public QComboBox
{
    Q_OBJECT

    public:

        EmpathFolderCombo(QWidget * parent);
        virtual ~EmpathFolderCombo();

        void activate(const EmpathURL &);

    protected slots:

        void s_update();
        void s_activated(const QString &);

    signals:

        void folderSelected(const EmpathURL &);
};

#endif
// vim:ts=4:sw=4:tw=78
