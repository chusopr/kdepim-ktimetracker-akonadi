/*  -*- mode: C++; c-file-style: "gnu" -*-
 *
 *  This file is part of libkdepim.
 *  Copyright (c) 2003 Zack Rusin <zack@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#ifndef ADDRESSESDIALOG_H
#define ADDRESSESDIALOG_H


#include <kdialogbase.h>
#include <klistview.h>
#include <qstringlist.h>
#include <kabc/addressee.h>

namespace KPIM {

  class AddresseeViewItem : public KListViewItem
  {
  public:
    enum Category {
      To    =0,
      CC    =1,
      BCC   =2,
      Group =3,
      Entry =4
    };
    AddresseeViewItem( AddresseeViewItem *parent, const KABC::Addressee& addr );
    AddresseeViewItem( KListView *lv, const QString& name, Category cat=Group );
    //AddresseeViewItem( AddresseeViewItem *parent, const QString& name,
    //                       const QString& email = QString::null );
    ~AddresseeViewItem();

    KABC::Addressee  addressee() const;
    Category         category() const;

    QString name()  const;
    QString email() const;

    virtual int compare( QListViewItem * i, int col, bool ascending ) const;

  private:
    struct AddresseeViewItemPrivate;
    AddresseeViewItemPrivate *d;
  };

  class AddressesDialog : public KDialogBase
  {
    Q_OBJECT
  public:
    AddressesDialog( QWidget *widget=0, const char *name=0 );
    ~AddressesDialog();

    /**
     * Returns the list of picked "To" addresses as a QStringList.
     */
    QStringList to()  const;
    /**
     * Returns the list of picked "CC" addresses as a QStringList.
     */
    QStringList cc()  const;
    /**
     * Returns the list of picked "BCC" addresses as a QStringList.
     */
    QStringList bcc() const;

    /**
     * Returns the list of picked "To" addresses as KABC::Addressee::List.
     */
    KABC::Addressee::List toAddresses()  const;
    /**
     * Returns the list of picked "CC" addresses as KABC::Addressee::List.
     */
    KABC::Addressee::List ccAddresses()  const;
    /**
     * Returns the list of picked "BCC" addresses as KABC::Addressee::List.
     */
    KABC::Addressee::List bccAddresses() const;

  public slots:
    /**
     * Displays the CC field if @p b is true, else
     * hides it. By default displays it.
     */
    void setShowCC( bool b );
    /**
     * Displays the BCC field if @p b is true, else
     * hides it. By default displays it.
     */
    void setShowBCC( bool b );
    /**
     * If called adds "Recent Addresses" item to the picker list view,
     * with the addresses given in @p addr.
     */
    void setRecentAddresses( const KABC::Addressee::List& addr );
    /**
     * Adds addresses in @p l to the selected "To" group.
     */
    void setSelectedTo( const QStringList& l );
     /**
     * Adds addresses in @p l to the selected "CC" group.
     */
    void setSelectedCC( const QStringList& l );
     /**
     * Adds addresses in @p l to the selected "BCC" group.
     */
    void setSelectedBCC( const QStringList& l );

  protected slots:
    void addSelectedTo();
    void addSelectedCC();
    void addSelectedBCC();

    void removeEntry();
    void saveAs();

    void editEntry();
    void newEntry();
    void deleteEntry();

    void cleanEdit();
    void filterChanged( const QString & );

  protected:
    void initGUI();
    void initConnections();
    void addAddresseeToAvailable( const KABC::Addressee& addr,
                                  AddresseeViewItem* defaultParent=0 );
    void addAddresseeToSelected( const KABC::Addressee& addr,
                                 AddresseeViewItem* defaultParent=0 );
    QStringList entryToString( const KABC::Addressee::List& l ) const;
    KABC::Addressee::List selectedAddressee( KListView* view ) const;
    KABC::Addressee::List allAddressee( AddresseeViewItem* parent ) const;
  private:
    struct AddressesDialogPrivate;
    AddressesDialogPrivate *d;
    static QString sPersonalGroup;
    static QString sRecentGroup;
  };

}

#endif /* ADDRESSESDIALOG_H */
