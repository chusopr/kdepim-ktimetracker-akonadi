#ifndef FILTEREDITDIALOG_H
#define FILTEREDITDIALOG_H

/*                                                                      
    This file is part of KAddressBook.                                  
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>                   
                                                                        
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
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

class QWidget;
class QToolButton;
class QString;

class KListBox;

class FilterEditWidget;

#include <kdialogbase.h>

#include "filter.h"

class FilterEditDialog : public KDialogBase
{
  Q_OBJECT
  
  public:
    FilterEditDialog(QWidget *parent, const char *name = 0);
    ~FilterEditDialog();
    
    void setFilters(const Filter::List &list);
    const Filter::List &filters() const { return mFilterList; }
    
  signals:
    void filtersChanged(const Filter::List &list);
    
  protected slots:
    void add();
    void remove();
    void rename();
    
    void filterHighlighted(int);
    
    void slotOk();
    void slotApply();
    
  private:
    void initGUI();
    
    Filter::List mFilterList;
    int mCurrentIndex;
    
    KListBox *mFilterListBox;
    QToolButton *mRemoveButton;
    QToolButton *mRenameButton;
    FilterEditWidget *mEditWidget;
};

#endif
