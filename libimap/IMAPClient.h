/*
   RFC 2060 (IMAP4rev1) client.

   Copyright (C) 2000 Rik Hemsley (rikkus) <rik@kde.org>

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

#ifndef IMAP_CLIENT_H
#define IMAP_CLIENT_H

// Qt includes
#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <qiodevice.h>
#include <qasciidict.h>
#include <qobject.h>

class AsyncClientPrivate;
class ClientPrivate;

namespace IMAP {

void log(const QString &);

enum MessageAttribute
{
  Seen      = 1 << 0,
  Answered  = 1 << 1,
  Flagged   = 1 << 2,
  Deleted   = 1 << 3,
  Draft     = 1 << 4,
  Recent    = 1 << 5
};

enum State
{
  Logout,
  NotAuthenticated,
  Authenticated,
  Selected
};

/**
 * Convert mailbox flags from enum to the type of string used by the
 * RFC. Mainly useful for debugging.
 */
QString flagsString(ulong mailboxFlags);

class StatusInfo
{
  public:

    enum Type { MessageCount, RecentCount, NextUID, UIDValidity, Unseen };

    StatusInfo();
    StatusInfo(const QString &);
    StatusInfo(const StatusInfo &);
    StatusInfo & operator = (const StatusInfo &);

    ulong messageCount() const
    { return messageCount_; }

    ulong recentCount() const
    { return recentCount_; }

    ulong nextUID() const
    { return nextUID_; }

    ulong uidValidity() const
    { return uidValidity_; }

    ulong unseenCount() const
    { return unseenCount_; }

    bool hasMessageCount() const
    { return hasMessageCount_; }

    bool hasRecentCount() const
    { return hasRecentCount_; }

    bool hasNextUID() const
    { return hasNextUID_; }

    bool hasUIDValidity() const
    { return hasUIDValidity_; }

    bool hasUnseenCount() const
    { return hasUnseenCount_; }

    void setMessageCount(ulong l)
    { hasMessageCount_ = true; messageCount_ = l; }

    void setRecentCount(ulong l)
    { hasRecentCount_ = true; recentCount_ = l; }

    void setNextUID(ulong l)
    { hasNextUID_ = true; nextUID_ = l; }

    void setUIDValidity(ulong l)
    { hasUIDValidity_ = true; uidValidity_ = l; }

    void setUnseenCount(ulong l)
    { hasUnseenCount_ = true; unseenCount_ = l; }

  private:

    ulong messageCount_;
    ulong recentCount_;
    ulong nextUID_;
    ulong uidValidity_;
    ulong unseenCount_;

    bool hasMessageCount_;
    bool hasRecentCount_;
    bool hasNextUID_;
    bool hasUIDValidity_;
    bool hasUnseenCount_;
};

class MailboxInfo
{
  public:

    MailboxInfo();
    MailboxInfo(const QString &);
    MailboxInfo(const MailboxInfo &);
    MailboxInfo & operator = (const MailboxInfo &);

    ulong _flags(const QString &) const;

    void setCount(ulong l)
    { countAvailable_ = true; count_ = l; }

    void setRecent(ulong l)
    { recentAvailable_ = true; recent_ = l; }

    void setUnseen(ulong l)
    { unseenAvailable_ = true; unseen_ = l; }

    void setUidValidity(ulong l)
    { uidValidityAvailable_ = true; uidValidity_ = l; }

    void setFlags(ulong l)
    { flagsAvailable_ = true; flags_ = l; }

    void setPermanentFlags(ulong l)
    { permanentFlagsAvailable_ = true; permanentFlags_ = l; }

    void setReadWrite(bool b)
    { readWriteAvailable_ = true; readWrite_ = b; }

    ulong count() const
    { return count_; }

    ulong recent() const
    { return recent_; }

    ulong unseen() const
    { return unseen_; }

    ulong uidValidity() const
    { return uidValidity_; }

    ulong flags() const
    { return flags_; }

    ulong permanentFlags() const
    { return permanentFlags_; }

    bool readWrite() const
    { return readWrite_; }

    ulong countAvailable() const
    { return countAvailable_; }

    ulong recentAvailable() const
    { return recentAvailable_; }

    ulong unseenAvailable() const
    { return unseenAvailable_; }

    ulong uidValidityAvailable() const
    { return uidValidityAvailable_; }

    ulong flagsAvailable() const
    { return flagsAvailable_; }

    ulong permanentFlagsAvailable() const
    { return permanentFlagsAvailable_; }

    bool readWriteAvailable() const
    { return readWriteAvailable_; }

  private:

    ulong count_;
    ulong recent_;
    ulong unseen_;
    ulong uidValidity_;
    ulong flags_;
    ulong permanentFlags_;
    bool  readWrite_;

    bool countAvailable_;
    bool recentAvailable_;
    bool unseenAvailable_;
    bool uidValidityAvailable_;
    bool flagsAvailable_;
    bool permanentFlagsAvailable_;
    bool readWriteAvailable_;
};

class ListResponse
{
  public:

    ListResponse();
    ListResponse(const QString &);
    ListResponse(const ListResponse &);
    ListResponse & operator = (const ListResponse &);

    QString hierarchyDelimiter() const
    { return hierarchyDelimiter_; }

    QString name() const
    { return name_; }

    bool noInferiors() const
    { return noInferiors_; }
    
    bool noSelect() const
    { return noSelect_; }

    bool marked() const
    { return marked_; }

    bool unmarked() const
    { return unmarked_; }

  private:

    QString hierarchyDelimiter_;
    QString name_;
    bool noInferiors_;
    bool noSelect_;
    bool marked_;
    bool unmarked_;
};

/**
 * Constructed from a server response. Does some simple parsing, working
 * out what type of response this is, what kind of status code it
 * contains (if this is a 'status' type response), creating a string
 * list with the last response line removed (useful in the implementation
 * of many methods of this class.
 */
class Response {

  public:

    enum ResponseType
    {
      ResponseTypeUnknown,
      ResponseTypeStatus,
      ResponseTypeContinuationRequest,
      ResponseTypeServerData
    };

    enum StatusCode
    {
      StatusCodeUnknown,
      StatusCodeAlert,
      StatusCodeNewName,
      StatusCodeParse,
      StatusCodePermanentFlags,
      StatusCodeReadOnly,
      StatusCodeReadWrite,
      StatusCodeTryCreate,
      StatusCodeUIDValidity,
      StatusCodeUnseen,
      StatusCodeOk,
      StatusCodeNo,
      StatusCodeBad,
      StatusCodePreAuth,
      StatusCodeBye,
      StatusCodeCapability,
      StatusCodeList,
      StatusCodeLsub,
      StatusCodeStatus,
      StatusCodeSearch,
      StatusCodeFlags,
      StatusCodeExists,
      StatusCodeRecent,
      StatusCodeExpunge,
      StatusCodeFetch
    };

    Response(const Response & r)
      : data_(r.data_),
        allData_(r.allData_),
        responseType_(r.responseType_),
        statusCode_(r.statusCode_)
    {
    }

    Response(const QString & data);

    ResponseType type() const;

    /**
     * This must be called when you think you have finished using this
     * class. It removes the static data.
     * You can safely continue using the class afterwards, but just
     * remember to call this method again later.
     * TODO: Reference count this class.
     */
    static void cleanup();

    StatusCode statusCode() const;

    /**
     * Data without last line.
     */
    QString data() const;

    /**
     * All the data, as it was passed to the ctor.
     */
    QString allData() const;

  private:

    // Disable operator = and default ctor.
    Response();
    Response & operator = (const Response &);

    /**
     * Convert a string to a status code (enum).
     */
    StatusCode _statusCode(const QString & key);

    QString data_;
    static QAsciiDict<ulong> * statusCodeDict_;
    
    // Order dependency
    QString allData_;
    ResponseType responseType_;
    StatusCode statusCode_;
    // End order dependency
};

/**
 * IMAP4rev1 client conforming to RFC 2060.
 * @author Rik Hemsley (rikkus) <rik@kde.org>
 */
class AsyncClient : public QObject
{
  public:

    /**
     * Create an AsyncClient.
     */
    AsyncClient(QObject * parent = 0, const char * name = 0);

    /**
     * Logs out from the server, deletes the static data from class
     * Response.
     */
    ~AsyncClient();

    /**
     * Connect to an IMAP server.
     * Will emit hostFound() at the appropriate time, and connected()
     * when ready.
     */
    void connectToHost(const QString & server, uint port);

    /**
     * Disconnect from IMAP server.
     * No response given.
     */
    void disconnectFromHost();

    /**
     * @return greeting message given by server at connect.
     * @see RFC
     */
    QString greeting() const;

    /**
     * @return capability string given by server
     * @see RFC
     */
    void capability();

    /**
     * @see RFC
     */
    void checkpoint();

    /**
     * Attempt to do plaintext authentication with server.
     * WARNING: Password transmitted in the clear !!!
     */
    void login(const QString & username, const QString & password);

    /**
     * Tell the server we want to log out. If this returns true,
     * you'll have to re-login to do anything else.
     */
    void logout();

    /**
     * Attempt to authenticate ourselves to the server. authType must
     * be one of those given in the capability string and is case-
     * -sensitive.
     */
    void authenticate(
        const QString & username,
        const QString & password,
        const QString & authType
    );

    /**
     * Send 'NOOP' to the server. This is handy if you're just polling
     * for new messages. Well, it will be when it actually returns something.
     */
    void noop();

    /**
     * Close connection to server. You'll have to re-open your connection
     * before doing anything else. This means disconnecting and reconnecting
     * your socket.
     */
    void close();

    /**
     * Select a mailbox. The client has a concept of the 'current' mailbox.
     * Some operations (check, close, expunge, search, fetch, store, copy)
     * require a mailbox to be selected before they will work.
     */
    void selectMailbox(const QString & name);

    /**
     * Get info about a mailbox without selecting it, otherwise identical
     * to @see select.
     */
    void examineMailbox(const QString & name);

    /**
     * Attempt to create a new mailbox with the given name.
     */
    void createMailbox(const QString & name);

    /**
     * Attempt to remove the new mailbox with the given name.
     */
    void removeMailbox(const QString & name);

    /**
     * Attempt to rename the new mailbox with the given name.
     */
    void renameMailbox(const QString & from, const QString & to);

    /**
     * Attempt to subscribe to the new mailbox with the given name.
     * @see RFC
     */
    void subscribeMailbox(const QString & name);

    /**
     * Attempt to unsubscribe from the new mailbox with the given name.
     * @see RFC
     */
    void unsubscribeMailbox(const QString & name);

    /**
     * Get a list of mailboxes matching the specification given in wild,
     * under the mailbox path given in ref.
     */
    void list(
        const QString & ref,
        const QString & wild,
        bool subscribedOnly = false
    );

    /**
     * Request status information on a mailbox. items is a logical OR of
     * StatusInfo::Type members.
     */
    void status(const QString & mailboxName, ulong items);

    /**
     * Attempt to add a message to a mailbox, with optional initial flags.
     * If date is specified, the date of the message is set accordingly.
     * For the format of the date parameter, @see RFC
     */
    void appendMessage(
        const QString & mailboxName,
        const QString & messageData,
        ulong flags = 0,
        const QString & date = ""
    );

    /**
     * Attempt to remove all message from a mailbox. Note that some
     * implementations may do something like move all messages to the
     * 'trash' box, unless of course the currently selected mailbox IS
     * the trash box.
     */
    void expunge();

    /**
     * Search for messages. For the format of the spec parameter, @see RFC.
     * You may specify the charset to be used when searching.
     * If you don't specify usingUID, then the returned message numbers
     * will be indices, not UIDs.
     */
    void search(
        const QString & spec,
        const QString & charSet = "",
        bool usingUID = false
    );

    /**
     * Retrieve information about message(s) within the range [start ... end].
     * For the format of the spec parameter and the returned value, @see RFC.
     * To operate on only one message, specify end == start.
     * Specifying usingUID means you're giving UIDs, not indices.
     */
    void fetch(
        ulong start,
        ulong end,
        const QString & spec,
        bool usingUID = false
    );

    enum FlagSetStyle { Set, Add, Remove };

    /**
     * Set flags on the message(s) within the range [start ... end].
     * To operate on only one message, specify end == start.
     * You may set, add or remove flags. @see FlagSetStyle.
     * Specifying usingUID means you're giving UIDs, not indices.
     */
    void setFlags(
        ulong start,
        ulong end,
        FlagSetStyle style,
        ulong flags,
        bool usingUID = false
    );

    /**
     * Copy message(s) within the range [start ... end] from the currently
     * selected mailbox to that specified in destination. 
     * Specifying usingUID means you're giving UIDs, not indices.
     */
    void copy(
        ulong start,
        ulong end,
        const QString & destination,
        bool usingUID = false
    );

  signals:

    void hostFound();
    void connected();
    void loginComplete(bool ok);
    void authenticateComplete(bool ok);
    void capabilityComplete(bool ok, const QString & data);
    void noopComplete(bool ok);
    void selectMailboxComplete(bool ok, const MailboxInfo & data);
    void examineMailboxComplete(bool ok, const MailboxInfo & data);
    void createMailboxComplete(bool ok);
    void subscribeMailboxComplete(bool ok);
    void listComplete(bool ok, const ListResponse & data);
    void appendMessageComplete(bool ok);
    void closeComplete(bool ok);
    void searchComplete(bool ok, const QValueList<ulong> data);
    void fetchComplete(bool ok, const QString & data);
    void setFlagsComplete(bool ok);
    void copyComplete(bool ok);

  protected:

    /**
     * Run a command and return the response from the server.
     * Takes care of prepending the unique string necessary.
     */
    void runCommand(const QString & cmd);

  protected slots:

    void slotDataReady();

  private:

    // Disable copying and default ctor.
    AsyncClient();
    AsyncClient(const AsyncClient &);
    AsyncClient & operator = (const AsyncClient &);

    AsyncClientPrivate * d;
};

/**
 * IMAP4rev1 client conforming to RFC 2060.
 * @author Rik Hemsley (rikkus) <rik@kde.org>
 */
class Client
{
  public:

    /**
     * Create an Client. The QIODevice must be connected to the
     * server. The greeting message from the server is read immediately.
     */
    Client(QIODevice *);

    /**
     * Logs out from the server, deletes the static data from class
     * Response.
     */
    ~Client();

    /**
     * @return greeting message given by server at connect.
     * @see RFC
     */
    QString greeting() const;

    /**
     * @return capability string given by server
     * @see RFC
     */
    QString capability();

    /**
     * @see RFC
     */
    bool checkpoint();

    /**
     * Attempt to do plaintext authentication with server.
     * WARNING: Password transmitted in the clear !!!
     */
    bool login(const QString & username, const QString & password);

    /**
     * Tell the server we want to log out. If this returns true,
     * you'll have to re-login to do anything else.
     */
    bool logout();

    /**
     * Attempt to authenticate ourselves to the server. authType must
     * be one of those given in the capability string and is case-
     * -sensitive.
     */
    bool authenticate(
        const QString & username,
        const QString & password,
        const QString & authType
    );

    /**
     * Send 'NOOP' to the server. This is handy if you're just polling
     * for new messages. Well, it will be when it actually returns something.
     */
    bool noop();

    /**
     * Close connection to server. You'll have to re-open your connection
     * before doing anything else. This means disconnecting and reconnecting
     * your socket.
     */
    bool close();

    /**
     * Select a mailbox. The client has a concept of the 'current' mailbox.
     * Some operations (check, close, expunge, search, fetch, store, copy)
     * require a mailbox to be selected before they will work.
     */
    bool selectMailbox(const QString & name, MailboxInfo & ret);

    /**
     * Get info about a mailbox without selecting it, otherwise identical
     * to @see select.
     */
    bool examineMailbox(const QString & name, MailboxInfo & ret);

    /**
     * Attempt to create a new mailbox with the given name.
     */
    bool createMailbox(const QString & name);

    /**
     * Attempt to remove the new mailbox with the given name.
     */
    bool removeMailbox(const QString & name);

    /**
     * Attempt to rename the new mailbox with the given name.
     */
    bool renameMailbox(const QString & from, const QString & to);

    /**
     * Attempt to subscribe to the new mailbox with the given name.
     * @see RFC
     */
    bool subscribeMailbox(const QString & name);

    /**
     * Attempt to unsubscribe from the new mailbox with the given name.
     * @see RFC
     */
    bool unsubscribeMailbox(const QString & name);

    /**
     * Get a list of mailboxes matching the specification given in wild,
     * under the mailbox path given in ref.
     */
    bool list(
        const QString & ref,
        const QString & wild,
        QValueList<ListResponse> &,
        bool subscribedOnly = false
    );

    /**
     * Request status information on a mailbox. items is a logical OR of
     * StatusInfo::Type members.
     */
    bool status(
        const QString & mailboxName,
        ulong items,
        StatusInfo & retval
    );

    /**
     * Attempt to add a message to a mailbox, with optional initial flags.
     * If date is specified, the date of the message is set accordingly.
     * For the format of the date parameter, @see RFC
     */
    bool appendMessage(
        const QString & mailboxName,
        const QString & messageData,
        ulong flags = 0,
        const QString & date = ""
    );

    /**
     * Attempt to remove all message from a mailbox. Note that some
     * implementations may do something like move all messages to the
     * 'trash' box, unless of course the currently selected mailbox IS
     * the trash box.
     */
    bool expunge(QValueList<ulong> & ret);

    /**
     * Search for messages. For the format of the spec parameter, @see RFC.
     * You may specify the charset to be used when searching.
     * If you don't specify usingUID, then the returned message numbers
     * will be indices, not UIDs.
     */
    QValueList<ulong> search(
        const QString & spec,
        const QString & charSet = "",
        bool usingUID = false
    );

    /**
     * Retrieve information about message(s) within the range [start ... end].
     * For the format of the spec parameter and the returned value, @see RFC.
     * To operate on only one message, specify end == start.
     * Specifying usingUID means you're giving UIDs, not indices.
     */
    QString fetch(
        ulong start,
        ulong end,
        const QString & spec,
        bool usingUID = false
    );

    enum FlagSetStyle { Set, Add, Remove };

    /**
     * Set flags on the message(s) within the range [start ... end].
     * To operate on only one message, specify end == start.
     * You may set, add or remove flags. @see FlagSetStyle.
     * Specifying usingUID means you're giving UIDs, not indices.
     */
    bool setFlags(
        ulong start,
        ulong end,
        FlagSetStyle style,
        ulong flags,
        bool usingUID = false
    );

    /**
     * Copy message(s) within the range [start ... end] from the currently
     * selected mailbox to that specified in destination. 
     * Specifying usingUID means you're giving UIDs, not indices.
     */
    bool copy(
        ulong start,
        ulong end,
        const QString & destination,
        bool usingUID = false
    );

  protected:

    /**
     * Run a command and return the response from the server.
     * Takes care of prepending the unique string necessary.
     */
    Response runCommand(const QString & cmd);

    /**
     * Reads response from server. Usually passed to the ctor of
     * class Response for initial parsing.
     */
    QString response(const QString & endIndicator);

  private:

    // Disable copying and default ctor.
    Client();
    Client(const Client &);
    Client & operator = (const Client &);

    ClientPrivate * d;
};


} // End namespace IMAP

#endif // Included this file
