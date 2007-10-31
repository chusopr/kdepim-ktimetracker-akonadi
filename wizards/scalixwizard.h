/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Bo Thorsen <bo@sonofthor.dk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef SCALIXWIZARD_H
#define SCALIXWIZARD_H

#include <kconfigwizard.h>

class KComboBox;
class KLineEdit;
class QCheckBox;

using namespace KPIM;

class ScalixWizard : public KConfigWizard
{
  public:
    ScalixWizard();
    ~ScalixWizard();

    QString validate();
    void usrReadConfig();
    void usrWriteConfig();

  private:
    KLineEdit *mServerEdit;
    KLineEdit *mUserEdit;
    KLineEdit *mEMailEdit;
    KLineEdit *mRealNameEdit;
    KLineEdit *mPasswordEdit;
    QCheckBox *mSavePasswordCheck;
    KComboBox *mSecurity;
    KComboBox *mAuthentication;
};

#endif
