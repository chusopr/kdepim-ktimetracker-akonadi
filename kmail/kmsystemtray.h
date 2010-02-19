/***************************************************************************
              kmsystemtray.h  -  description
               -------------------
  begin                : Fri Aug 31 22:38:44 EDT 2001
  copyright            : (C) 2001 by Ryan Breen
  email                : ryan@porivo.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMSYSTEMTRAY_H
#define KMSYSTEMTRAY_H

#include <akonadi/collection.h>
#include <kstatusnotifieritem.h>

#include <QAction>
#include <QMap>
#include <QPointer>
#include <QVector>

#include <QPixmap>
#include <QImage>

#include <time.h>

class QPoint;
class QMenu;

/**
 * KMSystemTray extends KStatusNotifierItem and handles system
 * tray notification for KMail
 */
class KMSystemTray : public KStatusNotifierItem
{
  Q_OBJECT
public:
  /** construtor */
  KMSystemTray(QObject* parent=0);
  /** destructor */
  ~KMSystemTray();

  void setMode(int mode);
  int mode() const;

  void hideKMail();
  bool hasUnreadMail() const;

public slots:
  void foldersChanged();

private slots:
#if 0
  void updateNewMessageNotification(KMFolder * folder);
#endif
  void selectedAccount(int);
  void updateNewMessages();
  void slotActivated();
  void slotContextMenuAboutToShow();

protected:
  bool mainWindowIsOnCurrentDesktop();
  void showKMail();
  void buildPopupMenu();
  void updateCount();
#if 0
  QString prettyName(KMFolder *);
#endif
private:

  bool mParentVisible;
  QPoint mPosOfMainWin;
  int mDesktopOfMainWin;

  int mMode;
  int mCount;

  QMenu *mNewMessagesPopup;
  QAction *mSendQueued;
  QPixmap mDefaultIcon;

  QVector<Akonadi::Collection> mPopupFolders;
#if 0
  QMap<QPointer<KMFolder>, int> mFoldersWithUnread;
  QMap<QPointer<KMFolder>, bool> mPendingUpdates;
#endif
  QTimer *mUpdateTimer;
  time_t mLastUpdate;
};

#endif
