/* Copyright 2009 James Bendig <james@imptalk.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "messagelist/utils/themeconfigbutton.h"

#include "messagelist/core/theme.h"
#include "messagelist/utils/themecombobox.h"
#include "messagelist/utils/themecombobox_p.h"
#include "messagelist/utils/configurethemesdialog.h"
#include "messagelist/core/manager.h"

#include <klocale.h>

using namespace MessageList::Core;
using namespace MessageList::Utils;

class MessageList::Utils::ThemeConfigButtonPrivate
{
public:
  ThemeConfigButtonPrivate( ThemeConfigButton *owner )
    : q( owner ) { }

  ThemeConfigButton * const q;

  const ThemeComboBox * mThemeComboBox;

  void slotConfigureThemes();
};

void ThemeConfigButtonPrivate::slotConfigureThemes()
{
  QString currentThemeID;
  if ( mThemeComboBox != 0 )
    currentThemeID = mThemeComboBox->currentTheme();

  ConfigureThemesDialog *dialog = new ConfigureThemesDialog( q->window() );
  dialog->selectTheme( currentThemeID );

  QObject::connect( dialog, SIGNAL(okClicked()),
                    q, SIGNAL(configureDialogCompleted()) );

  dialog->show();
}


ThemeConfigButton::ThemeConfigButton( QWidget * parent, const ThemeComboBox * themeComboBox )
: KPushButton( i18n( "Configure..." ), parent ), d( new ThemeConfigButtonPrivate( this ) )
{
  d->mThemeComboBox = themeComboBox;
  connect( this, SIGNAL(pressed()),
           this, SLOT(slotConfigureThemes()) );

  //Keep theme combo up-to-date with any changes made in the configure dialog.
  if ( d->mThemeComboBox != 0 )
    connect( this, SIGNAL(configureDialogCompleted()),
             d->mThemeComboBox, SLOT(slotLoadThemes()) );
}

ThemeConfigButton::~ThemeConfigButton()
{
  delete d;
}


#include "themeconfigbutton.moc"
