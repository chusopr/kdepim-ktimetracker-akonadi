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
# pragma interface "EmpathSendingSettingsDialog.h"
#endif

#ifndef EMPATHSENDINGSETTINGSDIALOG_H
#define EMPATHSENDINGSETTINGSDIALOG_H

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
#include <kdialog.h>
#include <kbuttonbox.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathPathSelectWidget.h"

class EmpathAddressSelectionWidget;
class EmpathFolderChooserWidget;

class EmpathSendingSettingsDialog : public KDialog
{
    Q_OBJECT

    public:
        
        EmpathSendingSettingsDialog(QWidget * parent = 0);
        ~EmpathSendingSettingsDialog();

        void saveData();
        void loadData();
        
    protected slots:

        void s_OK();
        void s_cancel();
        void s_help();
        void s_default();
        void s_apply();

    private:

        EmpathFileSelectWidget * efsw_sendmail_;
        EmpathFileSelectWidget * efsw_qmail_;
        QLineEdit * le_smtpServer_;

        QCheckBox * cb_copyOther_;
        
        QRadioButton * rb_sendmail_;
        QRadioButton * rb_qmail_;
        QRadioButton * rb_smtp_;

        QSpinBox * sb_smtpPort_;

        QButtonGroup * bg_server_;
        
        EmpathAddressSelectionWidget * asw_copyOther_;

        KButtonBox    * buttonBox_;
        QPushButton   * pb_help_;
        QPushButton   * pb_default_;
        QPushButton   * pb_apply_;
        QPushButton   * pb_OK_;
        QPushButton   * pb_cancel_;
        
        bool          applied_;

};

#endif
// vim:ts=4:sw=4:tw=78
