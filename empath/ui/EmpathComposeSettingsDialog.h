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
# pragma interface "EmpathComposeSettingsDialog.h"
#endif

#ifndef EMPATHCOMPOSESETTINGSDIALOG_H
#define EMPATHCOMPOSESETTINGSDIALOG_H

// Qt includes
#include <qlayout.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qspinbox.h>

// KDE includes
#include <kbuttonbox.h>
#include <kdialog.h>

// Local includes
#include "EmpathDefines.h"

class Empath;

/**
 * Used to configure the settings for composing messages.
 */
class EmpathComposeSettingsDialog : public KDialog
{
    Q_OBJECT

    public:
        
        EmpathComposeSettingsDialog(QWidget * parent = 0);
        ~EmpathComposeSettingsDialog();

        void saveData();
        void loadData();

    protected slots:

        void s_OK();
        void s_cancel();
        void s_help();
        void s_default();
        void s_apply();

    private:

        QButtonGroup    * buttonGroup_;
        
        QLabel          * l_extra_;
        QLabel          * l_reply_;
        QLabel          * l_replyAll_;
        QLabel          * l_forward_;

        QLineEdit       * le_extra_;
        QLineEdit       * le_reply_;
        QLineEdit       * le_replyAll_;
        QLineEdit       * le_forward_;
    
        QSpinBox        * sb_wrap_;

        QCheckBox       * cb_addSig_;
        QCheckBox       * cb_digSign_;
        QCheckBox       * cb_wrap_;
        QCheckBox       * cb_quote_;
    
        QRadioButton    * rb_sendNow_;
        QRadioButton    * rb_sendLater_;
        
        QCheckBox       * cb_externalEditor_;
        QLineEdit       * le_externalEditor_;

        KButtonBox      * buttonBox_;
        QPushButton     * pb_help_;
        QPushButton     * pb_default_;
        QPushButton     * pb_apply_;
        QPushButton     * pb_OK_;
        QPushButton     * pb_cancel_;
        
        bool applied_;

};

#endif
// vim:ts=4:sw=4:tw=78
