/* Message info describing a messages in a folder
 *
 * Author: Stefan Taferner <taferner@alpin.or.at>
 */
#ifndef kmmsginfo_h
#define kmmsginfo_h

#include "kmmessage.h"

#include <qarray.h>
#include <qstring.h>

class KMFolder;

class KMMsgInfo
{
  friend class KMFolder;

public:
  /** Init object. Second version takes a status string that is parsed. */
  void init(KMMessage::Status, unsigned long offset, unsigned long size);
  void init(const char* statusStr, unsigned long offset, unsigned long size);

  /** Init from string. Format is "%c %lu %lu": status, offset, size */
  void fromString(const char* str);

  /** Convert to string using above's format */
  const char* asString(void) const;

  /** Return offset in mail folder */
  unsigned long offset(void) const { return mOffset; }

  /** Return size of message including the whole header in bytes */
  unsigned long size(void) const { return mSize; }

  /** The status is only a copy of the status in the message and does not
    need to be up to date if the message is loaded */
  KMMessage::Status& status(void) { return mStatus; }

protected:
  KMMessage::Status mStatus;
  unsigned long     mOffset;
  unsigned long     mSize;
};

typedef QArray<KMMsgInfo> KMMsgInfoList;

#endif /*kmmsginfo_h*/
