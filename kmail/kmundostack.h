/*
    This file is part of KMail

    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    version 2 as published by the Free Software Foundation.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this library; see the file COPYING. If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef _KMUNDOSTACK_H_
#define _KMUNDOSTACK_H_

#include <qptrlist.h>
#include <qstring.h>
#include <qobject.h>

class KMFolder;
class KMMsgBase;

/** A class for storing Undo information. */
class KMUndoInfo
{
public:
  int               id;
  QValueList<ulong> serNums;
  KMFolder         *srcFolder;
  KMFolder         *destFolder;
};

class KMUndoStack : public QObject
{
  Q_OBJECT

public:
  KMUndoStack(int size);

  void clear();
  int  size() const { return mStack.count(); }
  int  newUndoAction( KMFolder* srcFolder, KMFolder* destFolder );
  void addMsgToAction( int undoId, ulong serNum );
  void undo();

  void pushSingleAction(ulong serNum, KMFolder *folder, KMFolder* destFolder);
  void msgDestroyed( KMMsgBase *msg);
  void folderDestroyed( KMFolder *folder);
protected:
  QPtrList<KMUndoInfo> mStack;
  int mSize;
  int mLastId;
  KMUndoInfo *mCachedInfo;

signals:
   void undoStackChanged();
};

#endif
