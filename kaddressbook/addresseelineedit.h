/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Helge Deller <deller@gmx.de>
                  2002 Lubos Lunak <llunak@suse.cz>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef ADDRESSEELINEEDIT_H
#define ADDRESSEELINEEDIT_H

#include <qobject.h>
#include <qptrlist.h>
#include <qtimer.h>

#include "klineedit.h"
#include "kcompletion.h"

class KConfig;

namespace KABC {
class LdapSearch;
}

class AddresseeLineEdit : public KLineEdit
{
  Q_OBJECT

  public:
    AddresseeLineEdit( QWidget* parent, bool useCompletion = true,
                     const char *name = 0L);
    virtual ~AddresseeLineEdit();

    virtual void setFont( const QFont& );

    static KConfig *config();

  public slots:
    void cursorAtEnd();
    void enableCompletion( bool enable );

  protected:
    virtual void loadAddresses();
    void addAddress( const QString& );
    virtual void keyPressEvent( QKeyEvent* );
    virtual void paste();
    virtual void insert( const QString &t );
    void doCompletion( bool ctrlT );

  private slots:
    void slotCompletion() { doCompletion(false); }
    void slotPopupCompletion( const QString& );
    void slotStartLDAPLookup();
    void slotLDAPSearchData( const QStringList& );

  private:
    void init();
    void startLoadingLDAPEntries();
    void stopLDAPLookup();
    QStringList addresses();

    QString m_previousAddresses;
    bool m_useCompletion;
    bool m_completionInitialized;
    bool m_smartPaste;

    static bool s_addressesDirty;
    static KCompletion *s_completion;
    static QTimer *s_LDAPTimer;
    static KABC::LdapSearch *s_LDAPSearch;
    static QString *s_LDAPText;
    static AddresseeLineEdit *s_LDAPLineEdit;
};

#endif
