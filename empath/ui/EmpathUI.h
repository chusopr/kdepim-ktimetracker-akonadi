/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
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
# pragma interface "EmpathUI.h"
#endif

#ifndef EMPATH_UI_H
#define EMPATH_UI_H

// Qt includes
#include <qobject.h>

// Local includes
#include "Empath.h"
#include "EmpathURL.h"

/**
 * @short A KDE interface to Empath
 * A KDE interface to Empath.
 */
class EmpathUI : public QObject
{
    Q_OBJECT

    public:

        EmpathUI();
        ~EmpathUI();
        
    protected slots:
    
        void s_setupWizard();
        void s_setupDisplay();
        void s_setupIdentity();
        void s_setupSending();
        void s_setupComposing();
        void s_setupAccounts();
        void s_setupFilters();
        void s_about();
        void s_bugReport();
        void s_configureMailbox(const EmpathURL &, QWidget * = 0);
        void s_infoMessage(const QString &);
        void s_getSaveName(const EmpathURL &);
        
        void s_newComposer(Empath::ComposeType, const EmpathURL &);
        void s_newComposer(const QString &);
        void s_sendEmail(const QString &, const QString &);
};

#endif

// vim:ts=4:sw=4:tw=78
