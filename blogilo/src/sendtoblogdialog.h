/***************************************************************************
*   This file is part of the Bilbo Blogger.                               *
*   Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>     *
*   Copyright (C) 2008-2009 Golnaz Nilieh <g382nilieh@gmail.com>          *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#ifndef SENDTOBLOGDIALOG_H
#define SENDTOBLOGDIALOG_H

#include <kdialog.h>
#include "ui_sendtoblogbase.h"

class SendToBlogDialog : public KDialog
{
    Q_OBJECT
public:
    explicit SendToBlogDialog( bool isNew, bool isPrivate, QWidget *parent = 0 );
    ~SendToBlogDialog();

    bool isPrivate();
    bool isNew();
public slots:
    virtual void accept();
private:
    Ui::SendToBlogBase ui;
    bool mIsPrivate;
    bool mIsNew;
};

#endif // SENDTOBLOGDIALOG_H
