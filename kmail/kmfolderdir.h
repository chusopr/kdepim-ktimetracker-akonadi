#ifndef kmfolderdir_h
#define kmfolderdir_h

#include <qstring.h>
#include "kmfoldernode.h"
#include "kmfoldertype.h"

class KMFolder;

/** KMail list that manages the contents of one directory that may
 * contain folders and/or other directories.
 */
class KMFolderDir: public KMFolderNode, public KMFolderNodeList
{
  Q_OBJECT

public:
  KMFolderDir(KMFolderDir* parent=0, const QString& path=QString::null, bool imap=FALSE);
  virtual ~KMFolderDir();

  virtual bool isDir() const { return TRUE; }

  /** Read contents of directory. */
  virtual bool reload();

  /** Return full pathname of this directory. */
  virtual QString path() const;

  /** Create a mail folder in this directory with given name. If sysFldr==TRUE
   the folder is marked as a (KMail) system folder.
   Returns Folder on success. */
  virtual KMFolder* createFolder(const QString& folderName,
				 bool sysFldr=FALSE,
                                 KMFolderType folderType=KMFolderTypeMbox);

  /** Returns folder with given name or zero if it does not exist */
  virtual KMFolderNode* hasNamedFolder(const QString& name);

protected:
  bool mImap;
};


//-----------------------------------------------------------------------------

class KMFolderRootDir: public KMFolderDir
{
  Q_OBJECT

public:
  KMFolderRootDir(const QString& path=QString::null, bool imap=FALSE);
  virtual ~KMFolderRootDir();
  virtual QString path() const;

  /** set the absolute path */
  virtual void setPath(const QString&);

protected:
  QString mPath;
};

#endif /*kmfolderdir_h*/
