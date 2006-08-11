/*
    Copyright (c) 2006 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include <QtDBus/QDBusConnection>
#include <QtGui/QApplication>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>

#include "profileview.h"

class Dialog : public QDialog
{
  public:
    Dialog( QWidget *parent = 0 )
      : QDialog( parent )
    {
      QVBoxLayout *layout = new QVBoxLayout( this );

      mView = new PIM::ProfileView( this );
      QDialogButtonBox *box = new QDialogButtonBox( this );

      layout->addWidget( mView );
      layout->addWidget( box );

      QPushButton *ok = box->addButton( QDialogButtonBox::Ok );
      connect( ok, SIGNAL( clicked() ), this, SLOT( accept() ) );

      QPushButton *cancel = box->addButton( QDialogButtonBox::Cancel );
      connect( cancel, SIGNAL( clicked() ), this, SLOT( reject() ) );

      resize( 450, 320 );
    }

    virtual void done( int r )
    {
      if ( r == Accepted ) {
        qDebug( "'%s' selected", qPrintable( mView->currentProfile() ) );
      }

      QDialog::done( r );
    }

  private:
    PIM::ProfileView *mView;
};

int main( int argc, char **argv )
{
  QApplication app( argc, argv );

  if ( !QDBus::sessionBus().registerService( "org.kde.Test.profileviewtest" ) ) {
    qDebug( "Unable to register service at dbus" );
    return 1;
  }

  Dialog dlg;
  dlg.exec();

  return 0;
};
