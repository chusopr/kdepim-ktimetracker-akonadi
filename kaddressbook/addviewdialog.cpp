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

#include <qbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qradiobutton.h>

#include <klocale.h>

#include "addviewdialog.h"
#include "viewwrapper.h"

AddViewDialog::AddViewDialog( QDict<ViewWrapper> *viewWrapperDict, 
                              QWidget *parent, const char *name )
  : KDialogBase( KDialogBase::Plain, i18n( "Add View" ),
                 KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                 parent, name ),
   mViewWrapperDict( viewWrapperDict )
{
  mTypeId = 0;
    
  QWidget *page = plainPage();
    
  QGridLayout *layout = new QGridLayout( page, 2, 2 );
  layout->setSpacing( spacingHint() );
  layout->setRowStretch( 1, 1 );
  layout->setColStretch( 1, 1 );
    
  QLabel *label = new QLabel( i18n( "View name:" ), page );
  layout->addWidget( label, 0, 0 );
    
  mViewNameEdit = new QLineEdit( page );
  connect( mViewNameEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  layout->addWidget( mViewNameEdit, 0, 1 );
    
  mTypeGroup = new QButtonGroup( 2, Qt::Horizontal, i18n( "View Type" ), page );
  connect( mTypeGroup, SIGNAL( clicked( int ) ), this, SLOT( clicked( int ) ) );
  layout->addMultiCellWidget( mTypeGroup, 1, 1, 0, 1 );
    
  // Now create the radio buttons. This needs some layout work.
  QDictIterator<ViewWrapper> iter( *mViewWrapperDict );
  for ( iter.toFirst(); iter.current(); ++iter ) {
    QRadioButton *rb = new QRadioButton( (*iter)->type(), mTypeGroup );
    label = new QLabel( (*iter)->description(), mTypeGroup );
    label->setAlignment( Qt::AlignLeft | Qt::AlignTop | Qt::WordBreak );
  }
    
  mTypeGroup->setButton( 0 );
  mViewNameEdit->setFocus();
  enableButton( KDialogBase::Ok, false );
}

AddViewDialog::~AddViewDialog()
{
}
    
QString AddViewDialog::viewName()
{
  return mViewNameEdit->text();
}
    
QString AddViewDialog::viewType()
{
  return mTypeGroup->find( mTypeId )->text();
}

void AddViewDialog::clicked( int id )
{
  mTypeId = id;
}

void AddViewDialog::textChanged( const QString &text )
{
  enableButton( KDialogBase::Ok, !text.isEmpty() );
}

#include "addviewdialog.moc"
