/*
    This file is part of KMail.

    Copyright (c) 2005 David Faure <faure@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2,
    as published by the Free Software Foundation.

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

#ifndef MESSAGECOMPOSER_GLOBALSETTINGS_H
#define MESSAGECOMPOSER_GLOBALSETTINGS_H

#include "messagecomposer_export.h"
#include "messagecomposersettings_base.h"


class QTimer;

namespace MessageComposer {

class MESSAGECOMPOSER_EXPORT MessageComposerSettings : public MessageComposer::MessageComposerSettingsBase
{
  Q_OBJECT
public:
  static MessageComposerSettings *self();

  /** Call this slot instead of directly @ref KConfig::sync() to
      minimize the overall config writes. Calling this slot will
      schedule a sync of the application config file using a timer, so
      that many consecutive calls can be condensed into a single
      sync, which is more efficient. */
  void requestSync();

private slots:
  void slotSyncNow();

private:
  MessageComposerSettings();
  virtual ~MessageComposerSettings();
  static MessageComposerSettings *mSelf;

  QTimer *mConfigSyncTimer;

};

}

#endif /* MESSAGECOMPOSER_GLOBALSETTINGS_H */
