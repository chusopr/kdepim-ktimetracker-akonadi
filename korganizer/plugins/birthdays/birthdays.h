/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KORG_BIRTHDAYS_H
#define KORG_BIRTHDAYS_H

#include <korganizer/part.h>

#include <libkcal/calendar.h>

class Birthdays : public KOrg::Part {
    Q_OBJECT
  public:
    Birthdays(KOrg::MainWindow *, const char *);
    ~Birthdays();

    QString info();

  private slots:
    void importBirthdays();

  private:
    QWidget *mParent;
};

#endif
