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

#include <ksystemtray.h>

#include <qmap.h>
#include <qguardedptr.h>
#include <qptrvector.h>
#include <qpixmap.h>
#include <qimage.h>

class KMFolder;
class KMMainWidget;
class QMouseEvent;
class KPopupMenu;
class QPoint;

/**
 * KMSystemTray extends KSystemTray and handles system
 * tray notification for KMail
 */
class KMSystemTray : public KSystemTray
{
  Q_OBJECT
  public:
  /** construtor */
  KMSystemTray(QWidget* parent=0, const char *name=0);
  /** destructor */
  ~KMSystemTray();

  void setMode(int mode);

  enum SystrayMode {
    AlwaysOn,
    OnNewMail
  };

private slots:
  void updateNewMessageNotification(KMFolder * folder);
  void foldersChanged();
  void selectedAccount(int);

protected:
  void mousePressEvent(QMouseEvent *);
  bool mainWindowIsOnCurrentDesktop();
  void showKMail();
  void hideKMail();
  void buildPopupMenu();
  void updateCount();

  QString prettyName(KMFolder *);
  KMMainWidget * getKMMainWidget();

private:

  bool mParentVisible;
  QPoint mPosOfMainWin;
  int mDesktopOfMainWin;

  int mMode;
  int mCount;
  int mNewMessagePopupId;

  KPopupMenu * mPopupMenu;
  QPixmap mDefaultIcon;
  QImage mLightIconImage;

  QPtrVector<KMFolder> mPopupFolders;
  QMap<QGuardedPtr<KMFolder>, int> mFoldersWithUnread;
};

#endif
