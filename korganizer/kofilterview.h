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
#ifndef KOFILTERVIEW_H
#define KOFILTERVIEW_H
// $Id$

#include "kofilterview_base.h"

#include <libkcal/calfilter.h>

using namespace KCal;

class KOFilterView : public KOFilterView_base
{
    Q_OBJECT
  public:
    KOFilterView(QPtrList<CalFilter> *filterList,QWidget* parent=0,const char* name=0, WFlags fl=0);
    ~KOFilterView();

    void updateFilters();

    bool filtersEnabled();
    CalFilter *selectedFilter();

  signals:
    void filterChanged();
    void editFilters();
    
  private:
    QPtrList<CalFilter> *mFilters;
};

#endif // KOFILTERVIEW_H
