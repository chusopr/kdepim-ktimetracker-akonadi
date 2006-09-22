/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QToolTip>
//Added by qt3to4:
#include <QHBoxLayout>

#include <kacceleratormanager.h>
#include <kbuttonbox.h>
#include <klineedit.h>
#include <klocale.h>
#include "addhostdialog.h"

AddHostDialog::AddHostDialog( KPIM::LdapServer *server, QWidget* parent )
  : KDialog( parent )
{
  setCaption( i18n( "Add Host" ) );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );
  setModal( true );
  showButtonSeparator( true );

  mServer = server;

  QWidget *page = new QWidget( this );
  setMainWidget( page );
  QHBoxLayout *layout = new QHBoxLayout( page );
  layout->setSpacing( spacingHint() );
  layout->setMargin( marginHint() );

  mCfg = new KLDAP::LdapConfigWidget(
       KLDAP::LdapConfigWidget::W_USER |
       KLDAP::LdapConfigWidget::W_PASS |
       KLDAP::LdapConfigWidget::W_BINDDN |
       KLDAP::LdapConfigWidget::W_REALM |
       KLDAP::LdapConfigWidget::W_HOST |
       KLDAP::LdapConfigWidget::W_PORT |
       KLDAP::LdapConfigWidget::W_VER |
       KLDAP::LdapConfigWidget::W_TIMELIMIT |
       KLDAP::LdapConfigWidget::W_SIZELIMIT |
       KLDAP::LdapConfigWidget::W_DN |
       KLDAP::LdapConfigWidget::W_SECBOX |
       KLDAP::LdapConfigWidget::W_AUTHBOX,
        page );

  layout->addWidget( mCfg );
  mCfg->setHost( mServer->host() );
  mCfg->setPort( mServer->port() );
  mCfg->setDn( mServer->baseDN() );
  mCfg->setUser( mServer->user() );
  mCfg->setBindDn( mServer->bindDN() );
  mCfg->setPassword( mServer->pwdBindDN() );
  mCfg->setTimeLimit( mServer->timeLimit() );
  mCfg->setSizeLimit( mServer->sizeLimit() );
  mCfg->setVersion( mServer->version() );

  switch ( mServer->security() ) {
    case KPIM::LdapServer::TLS:
      mCfg->setSecurity( KLDAP::LdapConfigWidget::TLS );
      break;
    case KPIM::LdapServer::SSL:
      mCfg->setSecurity( KLDAP::LdapConfigWidget::SSL );
      break;
    default:
      mCfg->setSecurity( KLDAP::LdapConfigWidget::None );
  }

  switch ( mServer->auth() ) {
    case KPIM::LdapServer::Simple:
      mCfg->setAuth( KLDAP::LdapConfigWidget::Simple );
      break;
    case KPIM::LdapServer::SASL:
      mCfg->setAuth( KLDAP::LdapConfigWidget::SASL );
      break;
    default:
      mCfg->setAuth( KLDAP::LdapConfigWidget::Anonymous );
  }
  mCfg->setMech( mServer->mech() );

  KAcceleratorManager::manage( this );

}

AddHostDialog::~AddHostDialog()
{
}

void AddHostDialog::slotHostEditChanged( const QString &text )
{
  enableButtonOk( !text.isEmpty() );
}

void AddHostDialog::slotOk()
{
  mServer->setHost( mCfg->host() );
  mServer->setPort( mCfg->port() );
  mServer->setBaseDN( mCfg->dn().trimmed() );
  mServer->setUser( mCfg->user() );
  mServer->setBindDN( mCfg->bindDn() );
  mServer->setPwdBindDN( mCfg->password() );
  mServer->setTimeLimit( mCfg->timeLimit() );
  mServer->setSizeLimit( mCfg->sizeLimit() );
  mServer->setVersion( mCfg->version() );
  switch ( mCfg->security() ) {
    case KLDAP::LdapConfigWidget::None:
      mServer->setSecurity( KPIM::LdapServer::Sec_None );
      break;
    case KLDAP::LdapConfigWidget::TLS:
      mServer->setSecurity( KPIM::LdapServer::TLS );
      break;
    case KLDAP::LdapConfigWidget::SSL:
      mServer->setSecurity( KPIM::LdapServer::SSL );
      break;
  }
  switch ( mCfg->auth() ) {
    case KLDAP::LdapConfigWidget::Anonymous:
      mServer->setSecurity( KPIM::LdapServer::Anonymous );
      break;
    case KLDAP::LdapConfigWidget::Simple:
      mServer->setSecurity( KPIM::LdapServer::Simple );
      break;
    case KLDAP::LdapConfigWidget::SASL:
      mServer->setSecurity( KPIM::LdapServer::SASL );
      break;
  }
  mServer->setMech( mCfg->mech() );
  KDialog::accept();
}

#include "addhostdialog.moc"
