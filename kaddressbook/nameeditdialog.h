#ifndef NAMEEDITDIALOG_H
#define NAMEEDITDIALOG_H
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

#include <kdialogbase.h>
#include <kabc/addressee.h>

#include "addresseeconfig.h"

class QCheckBox;

class KLineEdit;
class KComboBox;

/**
  Editor dialog for name details, like given name, family name etc.
*/
class NameEditDialog : public KDialogBase
{
  Q_OBJECT

  public:
    NameEditDialog( const KABC::Addressee &addr, QWidget *parent, const char *name = 0);
    ~NameEditDialog();
   
    QString familyName() const;
    QString givenName() const;
    QString prefix() const;
    QString suffix() const;
    QString additionalName() const;
   
  protected slots:
    void parseBoxChanged(bool);

  private:
    KComboBox *mSuffixCombo;
    KComboBox *mPrefixCombo;
    KLineEdit *mFamilyNameEdit;
    KLineEdit *mGivenNameEdit;
    KLineEdit *mAdditionalNameEdit;
    QCheckBox *mParseBox;

    AddresseeConfig mAddresseeConfig;
};

#endif
