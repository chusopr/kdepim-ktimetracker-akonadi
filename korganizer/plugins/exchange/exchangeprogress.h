/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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
#ifndef EXCHANGEPROGRESS_H
#define EXCHANGEPROGRESS_H

#include <kprogress.h>

class QComboBox;

class ExchangeProgress : public KProgressDialog
{
  Q_OBJECT
  
  public:
    ExchangeProgress(QWidget *parent=0);
    virtual ~ExchangeProgress();
    
  public slots:
    void slotTransferStarted();
    void slotTransferFinished();
    
  signals:
    void complete( ExchangeProgress* );

  private:
    int m_total;
    int m_finished;
    
  private:
    void updateLabel();
};

#endif
