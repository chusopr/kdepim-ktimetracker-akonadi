/* User identity information
 *
 * Author: Stefan Taferner <taferner@kde.org>
 * This code is under GPL
 */
#ifndef kmidentity_h
#define kmidentity_h

#include <qstring.h>
#include <qstringlist.h>

/** User identity information */
class KMIdentity
{
public:
  /** Returns list of identity ids written to the config file */
  static QStringList identities();

  /** Save a list of identity ids in the config file */
  static void saveIdentities( const QStringList & ids, bool aWithSync=true );

  /** Returns an identity whose address matches with any in addressList **/
  static QString matchIdentity( const QString & addressList );

  /** Constructor loads config file */
  KMIdentity( const QString & id );

  /** Destructor saves config file */
  ~KMIdentity();

  /** Read configuration from the global config */
  void readConfig();

  /** Write configuration to the global config with optional sync */
  void writeConfig(bool withSync=true) const;

  /** Tests if there are enough values set to allow mailing */
  bool mailingAllowed() const;

  /** Identity/nickname fot this collection */
  QString identity() const { return mIdentity; }

  /** Full name of the user */
  QString fullName() const { return mFullName; }
  void setFullName(const QString&);

  /** The user's organization (optional) */
  QString organization() const { return mOrganization; }
  void setOrganization(const QString&);

  /** The user's PGP identity */
  QCString pgpIdentity() const { return mPgpIdentity; }
  void setPgpIdentity(const QCString&);

  /** email address (without the user name - only name@host) */
  QString emailAddr() const { return mEmailAddr; }
  void setEmailAddr(const QString&);

  /** vCard to attach to outgoing emails */
  QString vCardFile() const { return mVCardFile; }
  void setVCardFile(const QString&);

  /** email address in the format "username <name@host>" suitable
    for the "From:" field of email messages. */
  QString fullEmailAddr() const;

  /** email address for the ReplyTo: field */
  QString replyToAddr() const { return mReplyToAddr; }
  void setReplyToAddr(const QString&);

  /** @return true if the signature is read from the output of a command */
  bool signatureIsCommand() const { return mUseSignatureFile && mSignatureFile.endsWith(QString::fromLatin1("|")); }
  /** @return true if the signature is read from a text file */
  bool signatureIsPlainFile() const { return mUseSignatureFile && !mSignatureFile.endsWith(QString::fromLatin1("|")); }
  /** @return true if the signature was specified directly */
  bool signatureIsInline() const { return !mUseSignatureFile; }

  /** name of the signature file (with path) */
  QString signatureFile() const { return mSignatureFile; }
  void setSignatureFile(const QString&);

  /** inline signature */
  QString signatureInlineText() const { return mSignatureInlineText;}
  void setSignatureInlineText(const QString&);

  /** Inline or signature from a file */
  bool useSignatureFile() const { return mUseSignatureFile; }
  void setUseSignatureFile(bool);

  /** Returns the signature. This method also takes care of special
    signature files that are shell scripts and handles them
    correct. So use this method to rectreive the contents of the
    signature file. If @p prompt is false, no errors will be displayed
    (useful for retries). */
  QString signature(bool prompt=true) const;

  /** The transport that is set for this identity. Used to link a
      transport with an identity. */
  QString transport() const { return mTransport; }
  void setTransport(const QString&);

  /** The folder where sent messages from this identity will be
      stored by default. */
  QString fcc() const { return mFcc; }
  void setFcc(const QString&);

  /** The folder where draft messages from this identity will be
      stored by default. */
  QString drafts() const { return mDrafts; }
  void setDrafts(const QString&);

protected:
  QString mIdentity, mFullName, mOrganization, mEmailAddr;
  QString mFcc;
  QString mDrafts;
  QCString mPgpIdentity;
  QString mReplyToAddr, mSignatureFile;
  QString mSignatureInlineText, mTransport;
  QString mVCardFile;
  bool    mUseSignatureFile;
};

#endif /*kmidentity_h*/
