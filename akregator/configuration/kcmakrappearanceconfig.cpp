/*
    This file is part of Akregator.
    Copyright (c) 2008 Frank Osterfeld <osterfeld@kde.org>

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

#include "kcmakrappearanceconfig.h"
#include "akregatorconfig.h"

#include "ui_settings_appearance.h"

#include <KAboutData>
#include <KConfigDialogManager>
#include <KGenericFactory>
#include <KLocalizedString>
#include <kdemacros.h>

#include <QVBoxLayout>

using namespace Akregator;

K_PLUGIN_FACTORY(KCMAkregatorAppearanceConfigFactory, registerPlugin<KCMAkregatorAppearanceConfig>();)
K_EXPORT_PLUGIN(KCMAkregatorAppearanceConfigFactory( "kcmakrappearanceconfig" ))

KCMAkregatorAppearanceConfig::KCMAkregatorAppearanceConfig( QWidget* parent, const QVariantList& args )
    : KCModule( KCMAkregatorAppearanceConfigFactory::componentData(), parent, args ), m_widget( new QWidget )
{  
    Ui::SettingsAppearance ui;
    ui.setupUi( m_widget );

    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->addWidget( m_widget );
   
    connect( ui.kcfg_UseCustomColors, SIGNAL( toggled( bool ) ),
             ui.kcfg_ColorUnreadArticles, SLOT( setEnabled( bool ) ) );
    connect( ui.kcfg_UseCustomColors, SIGNAL( toggled( bool ) ),
             ui.kcfg_ColorNewArticles, SLOT( setEnabled( bool ) ) );
    connect( ui.kcfg_UseCustomColors, SIGNAL( toggled( bool ) ),
             ui.lbl_newArticles, SLOT( setEnabled( bool ) ) );
    connect( ui.kcfg_UseCustomColors, SIGNAL( toggled( bool ) ),
             ui.lbl_unreadArticles, SLOT( setEnabled( bool ) ) );
    connect( ui.slider_minimumFontSize, SIGNAL( sliderMoved( int ) ),
             ui.kcfg_MinimumFontSize , SLOT( setValue( int ) ) );
    connect( ui.slider_mediumFontSize, SIGNAL( sliderMoved( int ) ),
             ui.kcfg_MediumFontSize , SLOT( setValue( int ) ) );
    connect( ui.kcfg_MinimumFontSize, SIGNAL( valueChanged( int ) ),
             ui.slider_minimumFontSize, SLOT( setValue( int ) ) );
    connect( ui.kcfg_MediumFontSize, SIGNAL( valueChanged( int ) ),
             ui.slider_mediumFontSize, SLOT( setValue( int ) ) );

    KAboutData *about = new KAboutData( I18N_NOOP( "kcmakrappearanceconfig" ), 0,
                                        ki18n( "Configure Feed Reader Appearance" ),
                                        0, KLocalizedString(), KAboutData::License_GPL,
                                        ki18n( "(c), 2004 - 2008 Frank Osterfeld" ) );

    about->addAuthor( ki18n( "Frank Osterfeld" ), KLocalizedString(), "osterfeld@kde.org" );
    setAboutData( about );

    addConfig( Settings::self(), m_widget );
}

#include "kcmakrappearanceconfig.moc"
