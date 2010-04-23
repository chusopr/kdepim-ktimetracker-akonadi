/*
    kngroupselectdialog.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNGROUPSELECTDIALOG_H
#define KNGROUPSELECTDIALOG_H

#include "kngroupbrowser.h"


/** Group selection dialog (used in the composer). */
class KNGroupSelectDialog : public KNGroupBrowser {

  Q_OBJECT

  public:
    /**
      Creates a group selection dialog.
      @param parent Parent widget.
      @param a An NNTP account.
      @param groups a list of groups that are already selected when this dialog shows.
    */
    KNGroupSelectDialog( QWidget *parent, KNNntpAccount::Ptr a, const QStringList &groups );
    ~KNGroupSelectDialog();

    QString selectedGroups()const;
    void itemChangedState(CheckItem *it, bool s);

  protected:
    void updateItemState(CheckItem *it);
    Q3ListView *selView;

  protected slots:
    void slotItemSelected(Q3ListViewItem *it);
    /** deactivates the button when a root item is selected */
    void slotSelectionChanged();
    void slotArrowBtn1();
    void slotArrowBtn2();

};


#endif















