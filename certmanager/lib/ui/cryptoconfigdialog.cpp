/*
    cryptoconfigdialog.h

    This file is part of kgpgcertmanager
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2, as published by the Free Software Foundation.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "cryptoconfigdialog.h"
#include "cryptoconfigmodule.h"
#include <klocale.h>

Kleo::CryptoConfigDialog::CryptoConfigDialog( Kleo::CryptoConfig* config, QWidget *parent, const char* name )
  : KDialogBase( parent, name, true /*modal*/,
                 i18n( "Configure" ), Default|Cancel|Apply|Ok|User1,
                 Ok, true /*separator*/, KGuiItem( i18n( "&Reset" ), "undo" ) )
{
  mMainWidget = new CryptoConfigModule( config, this );
  setMainWidget( mMainWidget );
  connect( mMainWidget, SIGNAL( changed() ), SLOT( slotChanged() ) );
  enableButton( Apply, false );
}

void Kleo::CryptoConfigDialog::slotOk()
{
  slotApply();
  accept();
}

void Kleo::CryptoConfigDialog::slotCancel()
{
  mMainWidget->cancel();
  reject();
}

void Kleo::CryptoConfigDialog::slotDefault()
{
  mMainWidget->defaults();
  slotChanged();
}

void Kleo::CryptoConfigDialog::slotApply()
{
  mMainWidget->save();
  enableButton( Apply, false );
}

void Kleo::CryptoConfigDialog::slotUser1() // reset
{
  mMainWidget->reset();
  enableButton( Apply, false );
}

void Kleo::CryptoConfigDialog::slotChanged()
{
  enableButton( Apply, true );
}

#include "cryptoconfigdialog.moc"
