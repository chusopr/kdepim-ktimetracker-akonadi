/*
 * kmail: KDE mail client
 * Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef kmfoldernode_h
#define kmfoldernode_h

#include <QList>
#include <QObject>
#include <QString>


class KMFolderNode: public QObject
{
  Q_OBJECT

public:
  KMFolderNode( /*KMFolderDir * parent,*/ const QString & name );
  virtual ~KMFolderNode();

  /** Is it a directory where mail folders are stored or is it a folder that
    contains mail ?
    Note that there are some kinds of mail folders like the type mh uses that
    are directories on disk but are handled as folders here. */
  virtual bool isDir(void) const;
  virtual void setDir(bool aDir) { mDir = aDir; }

  /** Returns full path to the directory where this node is stored or 0
   if the node has no parent. Example: if this object represents a folder
   ~joe/Mail/inbox then path() returns "/home/joe/Mail" and name() returns
   "inbox". */
  virtual QString path() const;

  /** Name of the node. Also used as file name. */
  QString name() const { return mName; }
  void setName(const QString& aName) { mName = aName; }

  /** Label of the node for visualzation purposes. Default the same as
      the name. */
  virtual QString label() const;

  /** URL of the node for visualization purposes. */
  virtual QString prettyUrl() const = 0;

  /** ID of the node */
  uint id() const;
  void setId( uint id ) { mId = id; }

protected:
  QString mName;
  bool mDir;
  uint mId;
};

typedef QList<KMFolderNode*> KMFolderNodeList;
typedef QListIterator<KMFolderNode*> KMFolderNodeListIterator;


#endif /*kmfoldernode_h*/
