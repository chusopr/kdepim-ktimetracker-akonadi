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
# pragma interface "EmpathAttachmentEditDialog.h"
#endif

#ifndef EMPATHATTACHMENTEDITDIALOG_H
#define EMPATHATTACHMENTEDITDIALOG_H

// Qt includes
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qcombobox.h>

// KDE includes
#include <kdialog.h>

#include "EmpathAttachmentSpec.h"

class EmpathFileSelectWidget;

class EmpathAttachmentEditDialog : public KDialog
{
    Q_OBJECT

    public:
    
        EmpathAttachmentEditDialog(QWidget * parent = 0, const char * name = 0);
        virtual ~EmpathAttachmentEditDialog();
        
        void setSpec(const EmpathAttachmentSpec & s);
        EmpathAttachmentSpec spec();
        void browse() { s_browse(); }
        
    protected slots:
        
        void s_OK();
        void s_cancel();
        void s_help();
        
        void s_browse();
        void s_typeChanged(int);
        
        void s_encodingChanged(int);

    private:
        
        void    _init();

        QButtonGroup    * bg_encoding_;

        EmpathFileSelectWidget * efsw_filename_;

        QLineEdit       * le_description_;
        
        QPushButton     * pb_OK_;
        QPushButton     * pb_cancel_;
        QPushButton     * pb_help_;
        
        QRadioButton    * rb_base64_;
        QRadioButton    * rb_8bit_;
        QRadioButton    * rb_7bit_;
        QRadioButton    * rb_qp_;
        
        QComboBox       * cb_type_;
        QComboBox       * cb_subType_;
        QComboBox       * cb_charset_;

        QStringList txtST_, msgST_, appST_, imgST_, vidST_, audST_, chrT_;

        EmpathAttachmentSpec spec_;
};

#endif

// vim:ts=4:sw=4:tw=78
