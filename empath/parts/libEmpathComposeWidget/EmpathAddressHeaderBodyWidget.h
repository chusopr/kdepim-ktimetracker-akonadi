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

#ifndef EMPATH_ADDRESS_HEADER_BODY_WIDGET_H
#define EMPATH_ADDRESS_HEADER_BODY_WIDGET_H

#include "EmpathHeaderBodyWidget.h"

class KLineEdit;

class EmpathAddressHeaderBodyWidget : public EmpathHeaderBodyWidget
{
    Q_OBJECT
    
    public:
    
        EmpathAddressHeaderBodyWidget(QWidget * parent = 0);
        virtual ~EmpathAddressHeaderBodyWidget();

        QString text() const;
        void setText(const QString &);

    protected slots:

        void s_browseClicked();

    private:

        KLineEdit   * le_address_;
};

#endif
// vim:ts=4:sw=4:tw=78
