/* KMail broadcast status message and related classes

   Copyright (C) 2000 Don Sanders <sanders@kde.org>

   License GPL
*/

#ifndef __kpim_broadcast_status_h
#define __kpim_broadcast_status_h

#include <qobject.h>
#include <qmap.h>

#undef None

namespace KPIM {

class ProgressItem;

/** When KMail is running it is possible to have multiple KMMainWin open
    at the same time. We want certain messages/information to be displayed
    in all active KMMainWins. BroadcastStatus make this possible, it
    defines a singleton object that broadcasts status messages by emitting
    signals. All KMMainWins connect up these signals to appropriate slots
    for updating their status bar. */

class BroadcastStatus : public QObject
{

  Q_OBJECT

public:

  /** Return the instance of the singleton object for this class */
  static BroadcastStatus *instance();
  /** Emit an update status bar signal */
  void setStatusMsg( const QString& message );
  /** Return the last status message from setStatusMsg() */
  QString statusMsg() const { return mStatusMsg; }
  /** Sets a status bar message with timestamp */
  void setStatusMsgWithTimestamp( const QString& message );
  /** Sets a transmission completed status bar message */
  void setStatusMsgTransmissionCompleted( int numMessages,
                                          int numBytes = -1,
                                          int numBytesRead = -1,
                                          int numBytesToRead = -1,
                                          bool mLeaveOnServer = false,
                                          KPIM::ProgressItem* progressItem = 0 ); // set the same status in this progress item
  void setStatusMsgTransmissionCompleted( const QString& account,
                                          int numMessages,
                                          int numBytes = -1,
                                          int numBytesRead = -1,
                                          int numBytesToRead = -1,
                                          bool mLeaveOnServer = false,
                                          KPIM::ProgressItem* progressItem = 0 ); // set the same status in this progress item

signals:

  /** Emitted when setStatusMsg is called. */
  void statusMsg( const QString& );

protected:

  BroadcastStatus();
  QString mStatusMsg;
  static BroadcastStatus* instance_;
};


}
#endif
