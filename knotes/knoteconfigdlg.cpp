/*******************************************************************
 KNotes -- Notes for the KDE project

 Copyright (c) 1997-2004, The KNotes Developers

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*******************************************************************/

#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>

#include <kconfig.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kcolorbutton.h>
#include <knuminput.h>
#include <klineedit.h>
#include <kfontrequester.h>
#include <kseparator.h>

#include "knote.h"
#include "knoteconfig.h"
#include "knoteconfigdlg.h"
#include "version.h"


KNoteConfigDlg::KNoteConfigDlg( KNoteConfig *config, const QString& title,
        bool defaults, QWidget *parent, const char *name )
    : KConfigDialog( parent, name, config, IconList,
                     defaults ? Default|Ok|Cancel : Default|Ok|Apply|Cancel, Ok )
{
    setCaption( title );
    setIcon( SmallIcon( "knotes" ) );

    setIconListAllVisible( true );

    config->readConfig();
    config->setVersion( KNOTES_VERSION );

    makeDisplayPage( defaults );
    makeEditorPage();
    makeActionsPage();
}

KNoteConfigDlg::~KNoteConfigDlg()
{
}

void KNoteConfigDlg::slotUpdateCaption()
{
    KNote *note = ::qt_cast<KNote *>(sender());
    if ( note )
        setCaption( note->name() );
}

void KNoteConfigDlg::makeDisplayPage( bool defaults )
{
    QWidget *displayPage = new QWidget();
    QGridLayout *layout = new QGridLayout( displayPage, 6, 2, 0, spacingHint() );

    QLabel *label_FgColor = new QLabel( i18n("&Text color:"), displayPage, "label_FgColor" );
    layout->addWidget( label_FgColor, 0, 0 );

    QLabel *label_BgColor = new QLabel( i18n("&Background color:"), displayPage, "label_BgColor" );
    layout->addWidget( label_BgColor, 1, 0 );

    KColorButton *kcfg_FgColor = new KColorButton( displayPage, "kcfg_FgColor" );
    label_FgColor->setBuddy( kcfg_FgColor );
    layout->addWidget( kcfg_FgColor, 0, 1 );

    KColorButton *kcfg_BgColor = new KColorButton( displayPage, "kcfg_BgColor" );
    label_BgColor->setBuddy( kcfg_BgColor );
    layout->addWidget( kcfg_BgColor, 1, 1 );

    QCheckBox *kcfg_ShowInTaskbar = new QCheckBox( i18n("&Show note in taskbar"),
                                                 displayPage, "kcfg_ShowInTaskbar" );
    layout->addWidget( kcfg_ShowInTaskbar, 4, 0 );


    if ( defaults )
    {
        QLabel *label_Width = new QLabel( i18n("Default &width:"), displayPage, "label_Width" );
        layout->addWidget( label_Width, 2, 0 );

        QLabel *label_Height = new QLabel( i18n("Default &height:"), displayPage, "label_Height" );
        layout->addWidget( label_Height, 3, 0 );

        KIntNumInput *kcfg_Width = new KIntNumInput( displayPage, "kcfg_Width" );
        label_Width->setBuddy( kcfg_Width );
        kcfg_Width->setRange( 100, 2000, 10, false );
        layout->addWidget( kcfg_Width, 2, 1 );

        KIntNumInput *kcfg_Height = new KIntNumInput( displayPage, "kcfg_Height" );
        kcfg_Height->setRange( 100, 2000, 10, false );
        label_Height->setBuddy( kcfg_Height );
        layout->addWidget( kcfg_Height, 3, 1 );
    }

    KSeparator *separator = new KSeparator( Horizontal, displayPage );
    layout->addMultiCellWidget( separator, 5, 5, 0, 1 );

    addPage( displayPage, i18n("Display"), "knotes", i18n("Display Settings") );
}

void KNoteConfigDlg::makeEditorPage()
{
    QWidget *editorPage = new QWidget();
    QGridLayout *layout = new QGridLayout( editorPage, 4, 3, 0, spacingHint() );

    QLabel *label_TabSize = new QLabel( i18n( "&Tab size:" ), editorPage, "label_TabSize" );
    layout->addMultiCellWidget( label_TabSize, 0, 0, 0, 1 );

    KIntNumInput *kcfg_TabSize = new KIntNumInput( editorPage, "kcfg_TabSize" );
    kcfg_TabSize->setRange( 0, 40, 1, false );
    label_TabSize->setBuddy( kcfg_TabSize );
    layout->addWidget( kcfg_TabSize, 0, 2 );

    QCheckBox *kcfg_AutoIndent = new QCheckBox( i18n("Auto &indent"), editorPage, "kcfg_AutoIndent" );
    layout->addMultiCellWidget( kcfg_AutoIndent, 1, 1, 0, 1 );

    QCheckBox *kcfg_RichText = new QCheckBox( i18n("&Rich text"), editorPage, "kcfg_RichText" );
    layout->addWidget( kcfg_RichText, 1, 2 );

    QLabel *label_Font = new QLabel( i18n("Text font:"), editorPage, "label_Font" );
    layout->addWidget( label_Font, 3, 0 );

    KFontRequester *kcfg_Font = new KFontRequester( editorPage, "kcfg_Font" );
    layout->addMultiCellWidget( kcfg_Font, 3, 3, 1, 2 );

    QLabel *label_TitleFont = new QLabel( i18n("Title font:"), editorPage, "label_TitleFont" );
    layout->addWidget( label_TitleFont, 2, 0 );

    KFontRequester *kcfg_TitleFont = new KFontRequester( editorPage, "kcfg_TitleFont" );
    layout->addMultiCellWidget( kcfg_TitleFont, 2, 2, 1, 2 );

    KSeparator *separator = new KSeparator( Horizontal, editorPage );
    layout->addMultiCellWidget( separator, 4, 4, 0, 2 );

    addPage( editorPage, i18n( "Editor" ), "edit", i18n("Editor Settings") );
}

void KNoteConfigDlg::makeActionsPage()
{
    QWidget *actionsPage = new QWidget();
    QGridLayout *layout = new QGridLayout( actionsPage, 2, 2, 0, spacingHint() );

    QLabel *label_MailAction = new QLabel( i18n("&Mail action:"), actionsPage, "label_MailAction" );
    layout->addWidget( label_MailAction, 0, 0 );

    KLineEdit *kcfg_MailAction = new KLineEdit( actionsPage, "kcfg_MailAction" );
    label_MailAction->setBuddy( kcfg_MailAction );
    layout->addWidget( kcfg_MailAction, 0, 1 );

    KSeparator *separator = new KSeparator( Horizontal, actionsPage );
    layout->addMultiCellWidget( separator, 2, 2, 0, 1 );

    addPage( actionsPage, i18n( "Actions" ), "misc", i18n("Action Settings") );
}


#include "knoteconfigdlg.moc"
