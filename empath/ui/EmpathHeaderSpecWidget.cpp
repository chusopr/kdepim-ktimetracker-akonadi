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
# pragma implementation "EmpathHeaderSpecWidget.h"
#endif

// Qt includes
#include <qlineedit.h>
#include <qlabel.h>

// Local includes
#include "EmpathHeaderSpecWidget.h"
#include "EmpathAddressSelectionWidget.h"
#include "EmpathUIUtils.h"
#include "Empath.h"

#include <RMM_Enum.h>

EmpathHeaderSpecWidget::EmpathHeaderSpecWidget(
    RMM::RHeader & header, QWidget * parent)
    :
        QHBox(parent, "HeaderSpecWidget"),
        header_(header)
{
    empathDebug("header: " + header.asString());
    
    headerNameWidget_ = new QLabel(this);
    headerNameWidget_->setText(header_.headerName() + ": ");
    
    address_ = false;

    RMM::HeaderDataType t = RMM::headerTypesTable[header_.headerType()];
    
    address_ =  t == RMM::AddressList    ||
                t == RMM::Address        ||
                t == RMM::MailboxList    ||
                t == RMM::Mailbox;

    if (address_) {
        EmpathAddressSelectionWidget * addressWidget = 
                new EmpathAddressSelectionWidget(this);
        headerBodyWidget_ = addressWidget->lineEdit();
    } else 
        headerBodyWidget_ = new QLineEdit(this, "headerBodyWidget");

    headerBodyWidget_->setText(header_.headerBody()->asString());
    headerBodyWidget_->setFocus();
}

EmpathHeaderSpecWidget::~EmpathHeaderSpecWidget()
{
    // Empty.
}

    RMM::RHeader &
EmpathHeaderSpecWidget::header()
{
    QCString headerName = headerNameWidget_->text().local8Bit();
    QCString headerBody = headerBodyWidget_->text().local8Bit();
    header_ = headerName + headerBody;
    return header_;
}

    int
EmpathHeaderSpecWidget::sizeOfColumnOne()
{
    return (headerNameWidget_->sizeHint().width());
}

    void
EmpathHeaderSpecWidget::setColumnOneSize(int i)
{
    headerNameWidget_->setFixedWidth(i);
}

    QString
EmpathHeaderSpecWidget::headerName()
{
    QString s = headerNameWidget_->text();
    s.truncate(s.length() - 1); // Remove trailing ':'
    return s;
}

    QString
EmpathHeaderSpecWidget::headerBody()
{
    QString s = headerBodyWidget_->text();

    if (address_) {
        if (!s.contains('@') && !s.isEmpty())
            s += '@' + empath->hostName();
        return s;
    }
    else
        return s;
}

    void
EmpathHeaderSpecWidget::focusInEvent(QFocusEvent *)
{
    headerBodyWidget_->setFocus();
}

    void
EmpathHeaderSpecWidget::keyPressEvent(QKeyEvent * e)
{
    if (e->state() & ControlButton) 
        switch (e->key()) {
        case Key_P:
            emit lineUp();
            break;
        case Key_N:
            emit lineDown();
            break;
        default:
            e->ignore();
        }
    else
        switch (e->key()) {
        case Key_Up:
            emit lineUp();
            break;
        case Key_Down:
            emit lineDown();
            break;
        default:
            e->ignore();
        }
}

    void
EmpathHeaderSpecWidget::s_setFocus()
{
    headerBodyWidget_->setFocus();
}

// vim:ts=4:sw=4:tw=78
