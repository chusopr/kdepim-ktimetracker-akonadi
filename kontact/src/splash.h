/*
   This file is part of the KDE project
   Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef SPLASH_H
#define SPLASH_H

#include <qvbox.h>

class QProgressBar;

namespace Kontact {

class Splash : public QVBox
{
  Q_OBJECT

  public:
    Splash( QWidget *parent, const char *name = 0 );
    ~Splash() {};

     QProgressBar* progressBar() const { return mProgressBar; }
  protected:
     QProgressBar* mProgressBar;
};

}

#endif
