/*
    This file is part of KOrganizer.
    Copyright (c) 2000,2001,2002,2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef KOPREFSDIALOG_H
#define KOPREFSDIALOG_H

#include <libkdepim/kprefsdialog.h>
#include <libkdepim/kcmdesignerfields.h>

#include <QHash>
#include <QLabel>

class QLineEdit;
class QLabel;
class QComboBox;
class KColorButton;
class KPushButton;
class QColor;
class Q3ListView;
class KComponentData;

namespace Ui {
  class KOGroupwarePrefsPage;
}

class KDE_EXPORT KOPrefsDialogMain : public KPrefsModule
{
    Q_OBJECT
  public:
    KOPrefsDialogMain( const KComponentData &inst, QWidget *parent );

  protected slots:
    void toggleEmailSettings( bool on );
  private:
    QWidget *mUserEmailSettings;
};

class KDE_EXPORT KOPrefsDialogColors : public KPrefsModule
{
    Q_OBJECT
  public:
    KOPrefsDialogColors( const KComponentData &inst, QWidget *parent );

  protected:
    void usrWriteConfig();
    void usrReadConfig();

  protected slots:
    void updateCategories();
    void setCategoryColor();
    void updateCategoryColor();

    void updateResources();
    void setResourceColor();
    void updateResourceColor();
  private:
    QComboBox     *mCategoryCombo;
    KColorButton  *mCategoryButton;
    QHash<QString, QColor> mCategoryDict;

    QComboBox     *mResourceCombo;
    KColorButton  *mResourceButton;
    QHash<QString, QColor> mResourceDict;
    //For translation Identifier <->idx in Combo
    QStringList mResourceIdentifier;
};

class KDE_EXPORT KOPrefsDialogGroupScheduling : public KPrefsModule
{
    Q_OBJECT
  public:
    KOPrefsDialogGroupScheduling( const KComponentData &inst, QWidget *parent );

  protected:
    void usrReadConfig();
    void usrWriteConfig();

  protected slots:
    void addItem();
    void removeItem();
    void updateItem();
    void updateInput();

  private:
    Q3ListView *mAMails;
    QLineEdit *aEmailsEdit;
};

class KOGroupwarePrefsPage;

class KDE_EXPORT KOPrefsDialogGroupwareScheduling : public KPrefsModule
{
    Q_OBJECT
  public:
    KOPrefsDialogGroupwareScheduling( const KComponentData &inst, QWidget *parent );
    ~KOPrefsDialogGroupwareScheduling();

  protected:
    void usrReadConfig();
    void usrWriteConfig();

  private:
    Ui::KOGroupwarePrefsPage *mGroupwarePage;
};

class KDE_EXPORT KOPrefsDialogPlugins : public KPrefsModule
{
    Q_OBJECT
  public:
    KOPrefsDialogPlugins( const KComponentData &inst, QWidget *parent );

  protected slots:
    void usrReadConfig();
    void usrWriteConfig();
    void configure();
    void selectionChanged( Q3ListViewItem* );

  private:
    void buildList();
    Q3ListView *mListView;
    QLabel *mDescription;
    KPushButton *mConfigureButton;
};

class KDE_EXPORT KOPrefsDesignerFields : public KPIM::KCMDesignerFields
{
  public:
    KOPrefsDesignerFields( const KComponentData &inst, QWidget *parent = 0 );

  protected:
    QString localUiDir();
    QString uiPath();
    void writeActivePages( const QStringList & );
    QStringList readActivePages();
    QString applicationName();
};

#endif
