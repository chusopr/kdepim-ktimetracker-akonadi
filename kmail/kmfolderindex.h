/* Virtual base class for mail folder with .*.index style index
 *
 * Author: Don Sanders <sanders@kde.org>
 * License: GPL
 */
#ifndef kmfolderindex_h
#define kmfolderindex_h

#include "kmfolder.h"
#include "kmmsglist.h"

class KMFolderIndex: public KMFolder
{
  Q_OBJECT
  //TODO:Have to get rid of this friend declaration and add necessary pure
  //virtuals to kmfolder.h so that KMMsgBase::parent() can be a plain KMFolder
  //rather than a KMFolderIndex. Need this for database indices.
  friend class KMMsgBase;
public:

  /** This enum indicates the status of the index file. It's returned by
      indexStatus().
   */
  enum IndexStatus { IndexOk,
                     IndexMissing,
                     IndexTooOld
  };

  /** Usually a parent is given. But in some cases there is no
    fitting parent object available. Then the name of the folder
    is used as the absolute path to the folder file. */
  KMFolderIndex(KMFolderDir* parent=0, const QString& name=QString::null);
  virtual ~KMFolderIndex();
  virtual int count(bool cache = false) const;

  virtual KMMsgBase* takeIndexEntry( int idx ) { return mMsgList.take( idx ); }
  virtual KMMsgInfo* setIndexEntry( int idx, KMMessage *msg );
  virtual void clearIndex(bool autoDelete=true, bool syncDict = false);
  virtual void fillDictFromIndex(KMMsgDict *dict);
  virtual void truncateIndex();

  virtual const KMMsgBase* getMsgBase(int idx) const { return mMsgList[idx]; }
  virtual KMMsgBase* getMsgBase(int idx) { return mMsgList[idx]; }

  virtual int find(const KMMsgBase* msg) const { return mMsgList.find((KMMsgBase*)msg); }

  /** Registered unique serial number for the index file */
  int serialIndexId() const { return mIndexId; }

  uchar *indexStreamBasePtr() { return mIndexStreamPtr; }

  bool indexSwapByteOrder() { return mIndexSwapByteOrder; }
  int  indexSizeOfLong() { return mIndexSizeOfLong; }

  virtual QString indexLocation() const;
  virtual int writeIndex();

public slots:
  /** Incrementally update the index if possible else call writeIndex */
  virtual int updateIndex();

protected:
  bool readIndex();

  /** Read index header. Called from within readIndex(). */
  bool readIndexHeader(int *gv=0);

  /** Create index file from messages file and fill the message-info list
      mMsgList. Returns 0 on success and an errno value (see fopen) on
      failure. */
  virtual int createIndexFromContents() = 0;

  bool updateIndexStreamPtr(bool just_close=FALSE);

  /** Tests whether the contents of this folder is newer than the index.
      Should return IndexTooOld if the index is older than the contents.
      Should return IndexMissing if there is contents but no index.
      Should return IndexOk if the folder doesn't exist anymore "physically"
      or if the index is not older than the contents.
  */
  virtual IndexStatus indexStatus() = 0;

  /** table of contents file */
  FILE* mIndexStream;
  /** list of index entries or messages */
  KMMsgList mMsgList;

  /** offset of header of index file */
  off_t mHeaderOffset;

  uchar *mIndexStreamPtr;
  int mIndexStreamPtrLength, mIndexId;
  bool mIndexSwapByteOrder; // Index file was written with swapped byte order
  int mIndexSizeOfLong; // Index file was written with longs of this size
};

#endif /*kmfolderindex_h*/
