/*   -*- c++ -*-
 *   kmail: KDE mail client
 *   This file: Copyright (C) 2000 Espen Sand, espen@kde.org
 *              Copyright (C) 2001-2002 Marc Mutz <mutz@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifndef _CONFIGURE_DIALOG_H_
#define _CONFIGURE_DIALOG_H_

#include <qguardedptr.h>
#include <kcmultidialog.h>

class KConfig;
class ProfileDialog;

class ConfigureDialog : public KCMultiDialog
{
  Q_OBJECT

public:
  ConfigureDialog( QWidget *parent=0, const char *name=0, bool modal=true );
  ~ConfigureDialog();

signals:
  /** Installs a new profile (in the dislog's widgets; to apply, the
      user has to hit the apply button). Profiles are normal kmail
      config files which have an additonal group "KMail Profile"
      containing keys "Name" and "Comment" for the name and description,
      resp. Only keys that this profile is supposed to alter should be
      included in the file.
  */
  void installProfile( KConfig *profile );
protected:
  void hideEvent( QHideEvent *i );
protected slots:
  /** @reimplemented
   * Saves the GlobalSettings stuff before passing on to KCMultiDialog.
   */
  void slotApply();

  /** @reimplemented
   * Saves the GlobalSettings stuff before passing on to KCMultiDialog.
   */
  void slotOk();

  /** @reimplemented
   * Brings up the profile loading/editing dialog. We can't use User1, as
   * KCMultiDialog uses that for "Reset". */
  void slotUser2();

private:
  QGuardedPtr<ProfileDialog>  mProfileDialog;
};

#endif
