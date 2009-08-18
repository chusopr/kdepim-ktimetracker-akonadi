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
#ifndef __KMAIL_MESSAGELISTVIEW_CORE_THEMECONFIGBUTTON_H__
#define __KMAIL_MESSAGELISTVIEW_CORE_THEMECONFIGBUTTON_H__

#include <KPushButton>

namespace KMail
{

namespace MessageListView
{

namespace Core
{

class ThemeComboBox;

/**
 * A specialized KPushButton that displays the theme
 * configure dialog when pressed.
 */
class ThemeConfigButton : public KPushButton
{
  Q_OBJECT

public:
  /** Constructor.
   * @param parent The parent widget for the button.
   * @param themeComboBox Optional ThemeComboBox to be kept in sync
   * with changes made by the configure dialog.
   */
  explicit ThemeConfigButton( QWidget * parent, const ThemeComboBox * themeComboBox = 0 );

signals:
  /**
   * A signal emitted when configure dialog has been successfully completed.
   */
  void configureDialogCompleted();

private slots:
  void slotConfigureThemes();

private:
  const ThemeComboBox * mThemeComboBox;
};

} // namespace Core

} // namespace MessageListView

} // namespace KMail

#endif //!__KMAIL_MESSAGELISTVIEW_CORE_THEMECONFIGBUTTON_H__
