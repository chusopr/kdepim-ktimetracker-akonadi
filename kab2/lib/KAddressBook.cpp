/*
    KAddressBook version 2
    
    Copyright (C) 1999 Rik Hemsley rik@kde.org
    
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// System includes
#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <iostream.h>

// Qt includes
#include <qdir.h>
#include <qfile.h>
#include <qtextstream.h>

// Local includes
#include <config.h>
#include "KAddressBookInterface.h"
#include "Entry.h"
#include "Field.h"

int KABUniqueEntryID = 0;

// Addressbook /////////////////////////////////////////////////////////////

KAddressBook::KAddressBook(QString name, QString path)
  : DCOPObject(name.utf8()),
		name_(name),
    path_(path)
{
  _init();
}

KAddressBook::~KAddressBook()
{
}

	QString
KAddressBook::path()
{
	return path_;
}

	QString
KAddressBook::name()
{
	return name_;
}

  Entry
KAddressBook::entry(QString id)
{
  Entry e;

  if (!index_.contains(id))
    return e;

  for (QStringList::ConstIterator it(index_.begin()); it != index_.end(); ++it)
    if (*it == id) {
      Entry * loaded = _readEntry(*it);
      if (loaded != 0)
        e = *loaded;
      break;
    }

  return e;
}

  bool
KAddressBook::contains(QString id)
{
  return index_.contains(id);
}

  QString
KAddressBook::insert(Entry e)
{
  e.setID(_generateUniqueID());

  index_.append(e.id());
  
  bool ok = _writeEntry(e);

  if (!ok)
    return QString::null;

  return e.id();
}

  bool
KAddressBook::remove(QString id)
{
  if (!index_.contains(id))
    return false;

  index_.remove(id);

  _removeEntry(id);
  
  return true;
}

  bool
KAddressBook::replace(Entry /* e */)
{
#warning STUB needs implementing
	return false;
  //return index_.contains(e.id());
}

  QString
KAddressBook::_generateUniqueID()
{
  return uniquePartOne_ + "_" + QString().setNum(KABUniqueEntryID++);
}

  void
KAddressBook::_init()
{
  struct utsname utsName;

  if (uname(&utsName) == 0)
    uniquePartOne_ = utsName.nodename;
  else
    uniquePartOne_ = "localhost";

  struct timeval timeVal;
  struct timezone timeZone;
  gettimeofday(&timeVal, &timeZone);

  uniquePartOne_ += "_" + QString().setNum(timeVal.tv_sec);
  uniquePartOne_ += "_" + QString().setNum(getpid());

  _checkDirs();

  _initIndex();
}

  void
KAddressBook::_checkDirs()
{
  QDir base(path_);

  if (!base.exists())
    if (!base.mkdir(path_)) {
      cerr << "Could not create dir `" + path_ + "' - exiting" << endl;
      exit(1);
    }

  QDir tmp(path_ + "/tmp");
  
  if (!tmp.exists())
    if (!tmp.mkdir(path_ + "/tmp")) {
      cerr << "Could not create dir `" + path_ + "/tmp' - exiting" << endl;
      exit(1);
    }

  QDir entries(path_ + "/entries");
  
  if (!entries.exists())
    if (!entries.mkdir(path_ + "/entries")) {
      cerr << "Could not create dir `" + path_ + "/entries' - exiting" << endl;
      exit(1);
    }
}

  void
KAddressBook::_initIndex()
{
  QDir d(path_ + "/entries");

  d.setFilter(QDir::Files | QDir::NoSymLinks | QDir::Readable);
  
  index_ = d.entryList();
}

  bool
KAddressBook::_writeEntry(const Entry & e)
{
  qDebug("KAddressBook::_writeEntry()");
  QString filename = path_ + "/tmp" + e.id();

  QFile f(filename);

  if (f.exists()) {
    qDebug("File `" + filename + "' exists");
    usleep(2000);
  }

  if (f.exists()) {
    qDebug("File `" + filename + "' still exists");
    return false;
  }

  if (!f.open(IO_WriteOnly)) {
    qDebug("Couldn't open file `" + filename + "' for writing");
    return false;
  }

  QDomDocument doc("kab-entry");

  e.insertInDomTree(doc, doc);

  qDebug("Doc as string: %s", doc.toString().ascii());

  QTextStream str(&f);

  str << doc.toString();

  f.flush();
  f.close();

  if (f.status() != IO_Ok) {
    qDebug("Couldn't flush file `" + filename + "'");
    f.remove();
    return false;
  }
  
  QString linkTarget(path_ + "/entries/" + e.id());

  if (::link(QFile::encodeName(filename), QFile::encodeName(linkTarget)) != 0) {
    qDebug("Couldn't successfully link `" + filename +
      "' to `" + linkTarget + "' - giving up");
    return false;
  }

  f.remove();

  return true;
}

  QStringList
KAddressBook::entryList()
{
  return
    QDir(path_ + "/entries").entryList
    (QDir::Files | QDir::NoSymLinks | QDir::Readable, QDir::Unsorted);
}

  Entry *
KAddressBook::_readEntry(const QString & id)
{
  QString filename = path_ + "/entries/" + id;
  QFile f(filename);

  if (!f.exists()) {
    qDebug("File `" + filename + "' does not exist");
    return 0;
  }
  
  if (!f.open(IO_ReadOnly)) {
    qDebug("Couldn't open `" + filename + "' for reading");
    return 0;
  }
  
  QDomDocument doc;

  if (!doc.setContent(&f))
  {
    qDebug("Couldn't set content to `" + filename + "'");
    return 0;
  }

  QDomElement docElem = doc.documentElement();

  QDomElement e = docElem.firstChild().toElement();

  if (e.isNull())
  {
    qDebug("Can't parse file `" + filename + "'");
    return 0;
  }

  if (e.tagName() != "kab:entry")
  {
    qDebug("Can't parse file `" + filename + "'");
    return 0;
  }

  return new Entry(e);
}

  bool
KAddressBook::_removeEntry(const QString & id)
{
  QFile f(path_ + "/entries/" + id);

  if (!f.exists())
    return false;

  if (!f.remove())
    return false;

  index_.remove(id);

  return true;
}

