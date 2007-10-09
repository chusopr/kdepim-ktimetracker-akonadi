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

#ifndef DISTRIBUTIONLISTWIDGET_H
#define DISTRIBUTIONLISTWIDGET_H

#include <config.h> // for KDEPIM_NEW_DISTRLISTS
#include <KDialog>
#include <QTreeWidget>

#include "extensionwidget.h"
//Added by qt3to4:
#include <QDragEnterEvent>
#include <QLabel>
#include <QDragMoveEvent>
#include <QDropEvent>

class QButtonGroup;
class QComboBox;

class DistributionListView;

namespace KAB {
class Core;
}

namespace KABC {
class AddressBook;
}

class DistributionListWidget : public KAB::ExtensionWidget
{
    Q_OBJECT

  public:
    DistributionListWidget( KAB::Core*, QWidget *parent );
    virtual ~DistributionListWidget();

    void contactsSelectionChanged();

    QString title() const;
    QString identifier() const;

  Q_SIGNALS:
    void modified();

  public slots:
    void dropped( QDropEvent* );

    void removeContact();

  private slots:
    void createList();
    void editList();
    void removeList();
    void addContact();
    void changeEmail();
    void updateNameCombo();
    void updateContactView();
    void selectionContactViewChanged();

  private:
#ifdef KDEPIM_NEW_DISTRLISTS
    void changed( const KABC::Addressee& dist );
#else
    void changed();
#endif
    bool alreadyExists( const QString& distrListName ) const;

  protected:
    void dropEvent( QDropEvent* );

  private:
    QComboBox *mNameCombo;
    QLabel *mEntryCountLabel;
    DistributionListView *mContactView;

    QPushButton *mCreateListButton;
    QPushButton *mEditListButton;
    QPushButton *mRemoveListButton;
    QPushButton *mChangeEmailButton;
    QPushButton *mAddContactButton;
    QPushButton *mRemoveContactButton;
};

/**
  @short Helper class
*/
class DistributionListView : public QTreeWidget
{
  Q_OBJECT

  public:
    DistributionListView( QWidget *parent );

  protected:
    void dragEnterEvent( QDragEnterEvent *e );
    void dropEvent( QDropEvent *e );
    void dragMoveEvent( QDragMoveEvent *e );

  Q_SIGNALS:
    void dropped( QDropEvent *e );
};

/**
  @short Helper class
*/
class EmailSelector : public KDialog
{
  public:
    EmailSelector( const QStringList &emails, const QString &current,
                   QWidget *parent );

    QString selected() const;

    static QString getEmail( const QStringList &emails, const QString &current,
                             QWidget *parent, bool & canceled );

  private:
    QButtonGroup *mButtonGroup;
    QMap<int, QString> mEmailMap;
};

#endif
