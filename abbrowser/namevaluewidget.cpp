/* This file is part of KDE PIM
    Copyright (C) 1999 Don Sanders <dsanders@kde.org>

    License: GNU GPL
*/

#include "namevaluewidget.h"
#include <qlayout.h>
#include <qstringlist.h>
#include <qcombobox.h>
#include "entry.h"
#include <klocale.h>

NameValueSheet::NameValueSheet( QWidget *parent, 
				int rows, 
				QStringList name, 
				QStringList entryField, 
        KAB::Entity *ce )
 : QFrame( parent ), lCell( 0 ), rows( rows )
{
  temp = new QLabel( i18n( "Name" ) + i18n ( "Name" ), 0, "temp" );
  minNameWidth = temp->sizeHint().width();
  minNameHeight = temp->sizeHint().height();
  int minWidth;
  int positiveRows;
  lCell = temp;
  if (rows < 1)
    positiveRows = 1;
  else
    positiveRows = rows;

  QGridLayout *lay2 = new QGridLayout( this, positiveRows, 2, 0 );
  lay2->setSpacing( -1 );
  for( int i = 0; i < rows; ++i ) {
    lCell = new QLabel( name[i], this );
    lCell->setFrameStyle( QFrame::Box | QFrame::Plain );
    minWidth = lCell->sizeHint().width();
    if (minWidth < minNameWidth)
      minWidth = minNameWidth;
    lCell->setMinimumWidth( minWidth );

    lay2->addWidget( lCell, i, 0 );

    QFrame *leFrame = new QFrame( this );
    leFrame->setMargin( 0 );
    leFrame->setFrameStyle( QFrame::Box | QFrame::Plain );
    QBoxLayout *leBox = new QBoxLayout( leFrame, QBoxLayout::LeftToRight, 2, 0 );
    QLineEdit *leCell = new ContactLineEdit( leFrame, entryField[i], ce );
    leFrame->setBackgroundColor( leCell->backgroundColor() );
    leCell->setFrame( false );
    leBox->addWidget( leCell );
    lay2->addWidget( leFrame, i, 1 );
  }
  if (rows == 0) {
    QFrame *filler = new QFrame( this );
    lay2->addWidget( filler, 0, 1 );
  }
  setMaximumHeight( (lCell->height() - verticalTrim*2) * rows );
}

NameValueSheet::~NameValueSheet()
{
  delete temp;
}

QSize NameValueSheet::cellSize()
{
  if (rows == 0)
    return QSize( minNameWidth, minNameHeight - verticalTrim );
  return QSize( lCell->size().width(), lCell->size().height() - verticalTrim );
}

NameValueFrame::NameValueFrame( QWidget *parent, NameValueSheet* vs ) 
 : QScrollView( parent ), vs( vs ) 
{
  setFrameStyle( QFrame::WinPanel | QFrame::Sunken  );
  lName = new QLabel( "Name", this );
  lName->setFrameStyle( QFrame::WinPanel | QFrame::Raised );
  lName->setMinimumSize( lName->sizeHint() );
  lValue = new QLabel( "Value", this );
  lValue->setFrameStyle( QFrame::WinPanel | QFrame::Raised );
  lValue->setMinimumSize( lValue->sizeHint () );

  setMargins( 0, lName->sizeHint().height() - 1, 0, 0 );
  enableClipper( true );
  setHScrollBarMode( QScrollView::AlwaysOff );
  addChild( vs );
  setResizePolicy( QScrollView::AutoOne );
  viewport()->setBackgroundColor( vs->backgroundColor() );
}
  
void NameValueFrame::setSheet( NameValueSheet* vs )
{
  this->vs = vs;
  vs->setMinimumSize( vs->sizeHint() );
  addChild( vs );
  showChild( vs, true );
  resizeContents( vs->width(), vs->height() );
  lName->setMinimumSize( vs->cellSize().width(), lName->height() );
  lName->resize( vs->cellSize().width(), lName->height() );
  lValue->setMinimumSize( visibleWidth() - lName->width(), lName->height() );
  lValue->resize( visibleWidth() - lName->width(), lName->height() );
  lName->move( 2, 2 );
  lValue->move( lName->width() + 2, 2 );
  vs->resize( visibleWidth(), vs->height() );
}

void NameValueFrame::resizeEvent(QResizeEvent* e) 
{ 
  QScrollView::resizeEvent( e ); 
  vs->resize( visibleWidth(), vs->height() );
  lName->resize( vs->cellSize() );
  lValue->resize( visibleWidth() - lName->width(), lName->height() );
  lName->move( 2, 2 );
  lValue->move( lName->width() + 2, 2 );
}

ContactLineEdit::ContactLineEdit( QWidget * parent, 
				  const char * name, 
          KAB::Entity *ce )
 : QLineEdit( parent, name ), ce( ce )
{
  KAB::Field * f = ce->find(name);
  if (f != 0)
    setText(f->data());
  connect( ce, SIGNAL( changed() ), this, SLOT( sync() ));
}

void ContactLineEdit::focusOutEvent ( QFocusEvent * )
{	
  ce->replace(name(),  text());
}

void ContactLineEdit::setName( const char *name )
{
  setText( "" );
  QLineEdit::setName( name );
  sync();
} 

void ContactLineEdit::sync()
{
  KAB::Field * f = ce->find(name());
  if (f == 0) return;
  QString value = f->data();
  if ((!value.isEmpty()) && (value != text()))
    setText(value);
}

ContactMultiLineEdit::ContactMultiLineEdit( QWidget * parent, 
					    const char * name, 
              KAB::Entity *ce )
 : QMultiLineEdit( parent, name ), ce( ce )
{
  connect( ce, SIGNAL( changed() ), this, SLOT( sync() ));
}

void ContactMultiLineEdit::focusOutEvent( QFocusEvent * )
{	
  ce->replace( QString( name()), text());
}

void ContactMultiLineEdit::setName( const char *name )
{
  setText( "" );
  QMultiLineEdit::setName( name );
  sync();
} 

void ContactMultiLineEdit::sync()
{
  KAB::Field * f = ce->find(name());
  if (f == 0)
    return;
  QString value = f->data();
  if ((!value.isEmpty()) && (value != text()))
    setText( value );
}

FileAsComboBox::FileAsComboBox( QWidget * parent, 
				const char * name, 
        KAB::Entity *ce )
 : QComboBox( true, parent, name ), ce( ce )
{
  connect( ce, SIGNAL( changed() ), this, SLOT( sync() ));
}

void FileAsComboBox::updateContact()
{	
  debug( "FileAsComboBox::focusOutEvent" );
  debug( currentText() );
  ce->replace( QString( name()), currentText());
}

void FileAsComboBox::setName( const char *name )
{
  setEditText( "" );
  QComboBox::setName( name );
  sync();
} 

void FileAsComboBox::sync()
{
  KAB::Field * f = ce->find(name());
  if (f == 0)
    return;
  
  QString value = f->data();
  if ((!value.isEmpty()) && (value != currentText()))
    setEditText( value );
}

ContactComboBox::ContactComboBox( QWidget *parent )
 : QComboBox( false, parent), buddy( 0 )
{}

void ContactComboBox::setBuddy( QWidget *buddy )
{
  this->buddy = buddy;
  connect( this, SIGNAL( activated(int)), this, SLOT( updateBuddy(int)));
}

void ContactComboBox::insertItem( const QString & text, const QString & vText )
{
  QComboBox::insertItem( text );
  vlEntryField.append( vText );
}

void ContactComboBox::updateBuddy( int index )
{
  if (index < (int)vlEntryField.count())
    if (buddy)
      buddy->setName( vlEntryField[index] );
};

QString ContactComboBox::currentEntryField()
{
  if (currentItem() < (int)vlEntryField.count())
    return vlEntryField[currentItem()];
  else
    return "";
};

