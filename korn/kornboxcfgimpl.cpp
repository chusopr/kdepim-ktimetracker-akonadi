/*
 * Copyright (C) 2004, Mart Kelder (mart.kde@hccnet.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
 
class KConfig;
#include "kornboxcfgimpl.h"

#include "keditlistboxman.h"
#include "kornaccountcfgimpl.h"
#include "password.h"

#include <kconfig.h>
#include <kcolorbutton.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kfontdialog.h>
#include <klocale.h>
#include <kicondialog.h>
#include <kurlrequester.h>

#include <qcheckbox.h>
#include <qcolor.h>
#include <qfont.h>
#include <qlabel.h>
#include <qstring.h>

KornBoxCfgImpl::KornBoxCfgImpl( QWidget * parent )
	: QWidget( parent ),
	Ui_KornBoxCfg(),
	_config( 0 ),
	_base( 0 ),
	_index( -1 )
{
	setupUi( this );
	
	_fonts[ 0 ] = new QFont;
	_fonts[ 1 ] = new QFont;
	_anims[ 0 ] = new QString;
	_anims[ 1 ] = new QString;

	lbLeft->setText( i18n( "Left mousebutton", "Left" ) );
	if( lbLeft->text() == "Left" )
		lbLeft->setText( i18n( "Left" ) );
	lbRight->setText( i18n( "Right mousebutton", "Right" ) );
	if( lbRight->text() == "Right" )
		lbRight->setText( i18n( "Right" ) );

	connect( parent, SIGNAL( okClicked() ), this, SLOT( slotOK() ) );
	connect( parent, SIGNAL( cancelClicked() ), this, SLOT( slotCancel() ) );
	
	elbAccounts->setTitle( i18n( "Accounts" ) );

	connect( elbAccounts, SIGNAL( elementsSwapped( int, int ) ), this, SLOT( slotAccountsSwapped( int, int ) ) );
	connect( elbAccounts, SIGNAL( elementDeleted( int ) ), this, SLOT( slotAccountDeleted( int ) ) );

	connect( chNormalText, SIGNAL(toggled(bool)), cbNormalText, SLOT(setEnabled(bool)) );
	connect( chNewText, SIGNAL(toggled(bool)), cbNewText, SLOT(setEnabled(bool)) );
	connect( chNormalBack, SIGNAL(toggled(bool)), cbNormalBack, SLOT(setEnabled(bool)) );
	connect( chNewBack, SIGNAL(toggled(bool)), cbNewBack, SLOT(setEnabled(bool)) );
	connect( chNormalIcon, SIGNAL(toggled(bool)), ibNormalIcon, SLOT(setEnabled(bool)) );
	connect( chNewIcon, SIGNAL(toggled(bool)), ibNewIcon, SLOT(setEnabled(bool)) );
	connect( chShowPassive, SIGNAL(toggled(bool)), chPassiveDate, SLOT(setEnabled(bool)) );
	connect( pbEdit, SIGNAL(clicked()), this, SLOT(slotEditBox()) );
	connect( elbAccounts, SIGNAL(activated(const QString&)), this, SLOT(slotActivated(const QString&)) );
	connect( elbAccounts, SIGNAL(setDefaults(const QString&,const int,KConfig*)), this, SLOT(slotSetDefaults(const QString&,const int,KConfig*)) );
	connect( pbNormalFont, SIGNAL(pressed()), this, SLOT(slotChangeNormalFont()) );
	connect( pbNewFont, SIGNAL(pressed()), this, SLOT(slotChangeNewFont()) );
	connect( pbNormalAnim, SIGNAL(released()), this, SLOT(slotChangeNormalAnim()) );
	connect( pbNewAnim, SIGNAL(pressed()), this, SLOT(slotChangeNewAnim()) );
	connect( chNormalFont, SIGNAL(toggled(bool)), pbNormalFont, SLOT(setEnabled(bool)) );
	connect( chNewFont, SIGNAL(toggled(bool)), pbNewFont, SLOT(setEnabled(bool)) );
	connect( chNormalAnim, SIGNAL(toggled(bool)), pbNormalAnim, SLOT(setEnabled(bool)) );
	connect( chNewAnim, SIGNAL(toggled(bool)), pbNewAnim, SLOT(setEnabled(bool)) );
	connect( chNormalAnim, SIGNAL(toggled(bool)), this, SLOT(slotNormalAnimToggled(bool)) );
	connect( chNewAnim, SIGNAL(toggled(bool)), this, SLOT(slotNewAnimToggled(bool)) );
}

KornBoxCfgImpl::~KornBoxCfgImpl()
{
	delete _fonts[ 0 ];
	delete _fonts[ 1 ];
	delete _anims[ 0 ];
	delete _anims[ 1 ];
}

void KornBoxCfgImpl::readConfig( KConfig * config, const int index )
{
	_config = config;
	_index = index;
	
	_config->setGroup( QString( "korn-%1" ).arg( index ) );

	readViewConfig();
	readEventConfig();
	readDCOPConfig();
	readAccountsConfig();
}

void KornBoxCfgImpl::writeConfig( KConfig * config, const int index )
{
	config->setGroup( QString( "korn-%1" ).arg( index ) );
	
	writeViewConfig( config );
	writeEventConfig( config );
	writeDCOPConfig( config );
	writeAccountsConfig( config );
}

//private
void KornBoxCfgImpl::readViewConfig()
{
	this->chNormalText->setChecked(_config->readEntry ( "hasnormalfgcolour", true ) );
	this->cbNormalText->setColor(  _config->readEntry( "normalfgcolour", QColor( Qt::black ) ) );
	this->chNewText->setChecked(   _config->readEntry ( "hasnewfgcolour", true ) );
	this->cbNewText->setColor(     _config->readEntry( "newfgcolour", QColor( Qt::black ) ) );
	this->chNormalBack->setChecked(_config->readEntry ( "hasnormalbgcolour", false ) );
	this->cbNormalBack->setColor(  _config->readEntry( "normalbgcolour", QColor( Qt::white ) ) );
	this->chNewBack->setChecked(   _config->readEntry ( "hasnewbgcolour", false ) );
	this->cbNewBack->setColor(     _config->readEntry( "newbgcolour", QColor( Qt::white ) ) );
	
	this->chNormalIcon->setChecked(_config->readEntry( "hasnormalicon", false ) );
	this->ibNormalIcon->setIcon(   _config->readEntry    ( "normalicon", "" ) );
	this->chNewIcon->setChecked(   _config->readEntry( "hasnewicon", false ) );
	this->ibNewIcon->setIcon(      _config->readEntry    ( "newicon", "" ) );
	
	this->chNormalFont->setChecked(_config->readEntry( "hasnormalfont", false ) );
	this->chNewFont->setChecked   (_config->readEntry( "hasnewfont", false ) );
	this->chNormalAnim->setChecked(_config->readEntry( "hasnormalanim", false ) );
	this->chNewAnim->setChecked(   _config->readEntry( "hasnewanim", false ) );
	*_fonts[ 0 ] = _config->readEntry( "normalfont", QFont() );
	*_fonts[ 1 ] = _config->readEntry( "newfont", QFont() );
	*_anims[ 0 ] = _config->readEntry    ( "normalanim", "" );
	*_anims[ 1 ] = _config->readEntry    ( "newanim", "" );
}

void KornBoxCfgImpl::readEventConfig()
{
	this->chLeftRecheck  ->setChecked( _config->readEntry( "leftrecheck", true ) );
	this->chMiddleRecheck->setChecked( _config->readEntry( "middlerecheck", false ) );
	this->chRightRecheck ->setChecked( _config->readEntry( "rightrecheck", false ) );
	
	this->chLeftReset  ->setChecked( _config->readEntry( "leftreset", false ) );
	this->chMiddleReset->setChecked( _config->readEntry( "middlereset", false ) );
	this->chRightReset ->setChecked( _config->readEntry( "rightreset", false ) );
	
	this->chLeftView  ->setChecked( _config->readEntry( "leftview", false ) );
	this->chMiddleView->setChecked( _config->readEntry( "middleview", false ) );
	this->chRightView ->setChecked( _config->readEntry( "rightview", false ) );
	
	this->chLeftRun  ->setChecked( _config->readEntry( "leftrun", false ) );
	this->chMiddleRun->setChecked( _config->readEntry( "middlerun", false ) );
	this->chRightRun ->setChecked( _config->readEntry( "rightrun", false ) );
	
	this->chLeftPopup  ->setChecked( _config->readEntry( "leftpopup", false ) );
	this->chMiddlePopup->setChecked( _config->readEntry( "middlepopup", false ) );
	this->chRightPopup ->setChecked( _config->readEntry( "rightpopup", true ) );
	
	this->edCommand->setURL( _config->readEntry( "command", "" ) );
	
	this->edNewRun->setURL( _config->readEntry( "newcommand", "" ) );
	this->edPlaySound->setURL( _config->readEntry( "sound", "" ) );
	this->chShowPassive->setChecked( _config->readEntry( "passivepopup", false ) );
	this->chPassiveDate->setChecked( _config->readEntry( "passivedate", false ) );
}

void KornBoxCfgImpl::readAccountsConfig()
{
	elbAccounts->setGroupName( QString( "korn-%1-%2" ).arg( _index ) );
	elbAccounts->setConfig( _config );
}
	
void KornBoxCfgImpl::readDCOPConfig()
{
	elbDCOP->clear();
	elbDCOP->insertStringList( _config->readListEntry( "dcop", ',' ) );
}
	
void KornBoxCfgImpl::writeViewConfig( KConfig* config )
{
	QColor invalid;
	
	config->writeEntry( "hasnormalfgcolour", this->chNormalText->isChecked() );
	config->writeEntry( "normalfgcolour",    this->chNormalText->isChecked() ? this->cbNormalText->color() : invalid );
	config->writeEntry( "hasnewfgcolour",    this->chNewText->isChecked() );
	config->writeEntry( "newfgcolour",       this->chNewText->isChecked()    ? this->cbNewText->color() : invalid );
	config->writeEntry( "hasnormalbgcolour", this->chNormalBack->isChecked() );
	config->writeEntry( "normalbgcolour",    this->chNormalBack->isChecked() ? this->cbNormalBack->color() : invalid );
	config->writeEntry( "hasnewbgcolour",    this->chNewBack->isChecked() );
	config->writeEntry( "newbgcolour",       this->chNewBack->isChecked()    ? this->cbNewBack->color() : invalid );
	
	config->writeEntry( "hasnormalicon", this->chNormalIcon->isChecked() );
	config->writeEntry( "normalicon",    this->chNormalIcon->isChecked() ? this->ibNormalIcon->icon() : "" );
	config->writeEntry( "hasnewicon",    this->chNewIcon->isChecked() );
	config->writeEntry( "newicon",       this->chNewIcon->isChecked()    ? this->ibNewIcon->icon() : "" );
	
	config->writeEntry( "hasnormalfont", this->chNormalFont->isChecked() );
	config->writeEntry( "normalfont",    this->chNormalFont->isChecked() ? *_fonts[ 0 ] : QFont() );
	config->writeEntry( "hasnewfont",    this->chNewFont->isChecked() );
	config->writeEntry( "newfont",       this->chNewFont->isChecked() ? *_fonts[ 1 ] : QFont() );
	config->writeEntry( "hasnormalanim", this->chNormalAnim->isChecked() );
	config->writeEntry( "normalanim",    this->chNormalAnim->isChecked() ? *_anims[ 0 ] : "" );
	config->writeEntry( "hasnewanim",    this->chNewAnim->isChecked() );
	config->writeEntry( "newanim",       this->chNewAnim->isChecked() ? *_anims[ 1 ] : "" );
	
}

void KornBoxCfgImpl::writeEventConfig( KConfig *config )
{
	config->writeEntry( "leftrecheck",   this->chLeftRecheck  ->isChecked() );
	config->writeEntry( "middlerecheck", this->chMiddleRecheck->isChecked() );
	config->writeEntry( "rightrecheck",  this->chRightRecheck ->isChecked() );
	
	config->writeEntry( "leftreset",   this->chLeftReset  ->isChecked() );
	config->writeEntry( "middlereset", this->chMiddleReset->isChecked() );
	config->writeEntry( "rightreset",  this->chRightReset ->isChecked() );
	
	config->writeEntry( "leftview",   this->chLeftView  ->isChecked() );
	config->writeEntry( "middleview", this->chMiddleView->isChecked() );
	config->writeEntry( "rightview",  this->chRightView ->isChecked() );
	
	config->writeEntry( "leftrun",   this->chLeftRun  ->isChecked()  );
	config->writeEntry( "middlerun", this->chMiddleRun->isChecked()  );
	config->writeEntry( "rightrun",  this->chRightRun ->isChecked() );
	
	config->writeEntry( "leftpopup",   this->chLeftPopup  ->isChecked() );
	config->writeEntry( "middlepopup", this->chMiddlePopup->isChecked() );
	config->writeEntry( "rightpopup",  this->chRightPopup ->isChecked() );
	
	config->writeEntry( "command", this->edCommand->url() );
	
	config->writeEntry( "newcommand", this->edNewRun->url() );
	config->writeEntry( "sound", this->edPlaySound->url() );
	config->writeEntry( "passivepopup", this->chShowPassive->isChecked() );
	config->writeEntry( "passivedate", this->chPassiveDate->isChecked() );
}

void KornBoxCfgImpl::writeAccountsConfig( KConfig * /*config */)
{
}

void KornBoxCfgImpl::writeDCOPConfig( KConfig *config )
{
	config->writeEntry( "dcop", elbDCOP->items(), ',' );
}

void KornBoxCfgImpl::slotEditBox()
{
	if( _base )
		return; //Already a dialog open
	if( elbAccounts->listBox()->currentItem() < 0 )
		return; //No item selected
	elbAccounts->setEnabled( false );
	
	_base = new KDialog( this, i18n("Box Configuration"), KDialog::Ok | KDialog::Cancel );
	_base->setModal( false );
	_base->enableButtonSeparator( true );
	KornAccountCfgImpl *widget = new KornAccountCfgImpl( _base );

	_base->setMainWidget( widget );
	
	connect( _base, SIGNAL( finished() ), this, SLOT( slotDialogDestroyed() ) );

	_group = new KConfigGroup( _config, QString( "korn-%1-%2" ).
			arg( _index ).arg(elbAccounts->listBox()->currentItem() ) );
	
	QMap< QString, QString > *map = new QMap< QString, QString >( _config->entryMap( QString( "korn-%1-%2" ).
			                        arg( _index ).arg(elbAccounts->listBox()->currentItem() ) ) );
	widget->readConfig( _group, map, _index, elbAccounts->listBox()->currentItem() );
	delete map;

	_base->show();
}
	
void KornBoxCfgImpl::slotActivated( const QString& )
{
	slotEditBox();
}

void KornBoxCfgImpl::slotActivated( const int )
{
	slotEditBox();
}

void KornBoxCfgImpl::slotSetDefaults( const QString& name, const int, KConfig* config )
{
	config->writeEntry( "name", name );
	config->writeEntry( "protocol", "mbox" );
	config->writeEntry( "host", QString() );
	config->writeEntry( "port", QString() );
	config->writeEntry( "username", QString() );
	config->writeEntry( "mailbox", "/var/spool/mail/" );
	config->writeEntry( "savepassword", 0 );
	config->writeEntry( "password", QString() );
	config->writeEntry( "auth", QString() );
	config->writeEntry( "interval", 300 );
	config->writeEntry( "boxsettings", true );
	config->writeEntry( "command", "" );
	config->writeEntry( "sound", "" );
	config->writeEntry( "passivepopup", false );
	config->writeEntry( "passivedate", false );
}

void KornBoxCfgImpl::slotChangeNormalAnim()
{
	*_anims[ 0 ] = KFileDialog::getOpenFileName( *_anims[ 0 ], ".mng .gif", this, i18n("Normal animation") );
}

void KornBoxCfgImpl::slotChangeNewAnim()
{
	*_anims[ 1 ] = KFileDialog::getOpenFileName( *_anims[ 1 ], ".mng .gif", this, i18n("Normal animation") );
}

void KornBoxCfgImpl::slotChangeNormalFont()
{
	KFontDialog fd( this, "font dialog" );
	fd.setFont( *_fonts[ 0 ], false );
	fd.exec();
	*_fonts[ 0 ] = fd.font();
}

void KornBoxCfgImpl::slotChangeNewFont()
{
	KFontDialog fd( this, "font dialog" );
	fd.setFont( *_fonts[ 1 ], false );
	fd.exec();
	*_fonts[ 1 ] = fd.font();
}

void KornBoxCfgImpl::slotNormalAnimToggled( bool enabled )
{
	this->chNormalText->setEnabled( !enabled );
	//this->chNormalBack->setEnabled( !enabled );
	this->chNormalIcon->setEnabled( !enabled );
	this->chNormalFont->setEnabled( !enabled );

	this->cbNormalText->setEnabled( !enabled && this->chNormalText->isChecked() );
	//this->cbNormalBack->setEnabled( !enabled && this->chNormalBack->isChecked() );
	this->ibNormalIcon->setEnabled( !enabled && this->chNormalIcon->isChecked() );
	this->pbNormalFont->setEnabled( !enabled && this->chNormalFont->isChecked() );
}

void KornBoxCfgImpl::slotNewAnimToggled( bool enabled )
{
	this->chNewText->setEnabled( !enabled );
	//this->chNewBack->setEnabled( !enabled );
	this->chNewIcon->setEnabled( !enabled );
	this->chNewFont->setEnabled( !enabled );
	
	this->cbNewText->setEnabled( !enabled && this->chNewText->isChecked() );
	//this->cbNewBack->setEnabled( !enabled && this->chNewBack->isChecked() );
	this->ibNewIcon->setEnabled( !enabled && this->chNewIcon->isChecked() );
	this->pbNewFont->setEnabled( !enabled && this->chNewFont->isChecked() );
}

void KornBoxCfgImpl::slotOK()
{
	writeConfig( _config, _index );
}

void KornBoxCfgImpl::slotCancel()
{
	readConfig( _config, _index );
}

void KornBoxCfgImpl::slotDialogDestroyed()
{
	_base->deleteLater(); _base = 0;
	delete _group;
	elbAccounts->setEnabled( true );
}

void KornBoxCfgImpl::slotAccountsSwapped( int account1, int account2 )
{
	KOrnPassword::swapKOrnWalletPassword( _index, account1, _index, account2 );
}

void KornBoxCfgImpl::slotAccountDeleted( int account )
{
	KOrnPassword::deleteKOrnPassword( _index, account );
}

#include "kornboxcfgimpl.moc"

