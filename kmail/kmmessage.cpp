// kmmessage.cpp

// if you do not want GUI elements in here then set ALLOW_GUI to 0.
#define ALLOW_GUI 1
#include "kmmessage.h"
#include "kmmsgpart.h"
#include "kmreaderwin.h"
#include "mailinglist-magic.h"
#include <kpgp.h>
#include <kpgpblock.h>
#include <kdebug.h>

#include "kmfolder.h"
#include "kmundostack.h"
#include "kmversion.h"
#include "kmidentity.h"
#include "kmkernel.h"
#include "identitymanager.h"

#include <kapplication.h>
#include <kglobalsettings.h>
#include <khtml_part.h>
#include <qcursor.h>

// we need access to the protected member DwBody::DeleteBodyParts()...
#define protected public
#include <mimelib/body.h>
#undef protected
#include <mimelib/field.h>

#include <qtextcodec.h>
#include <qstrlist.h>

#include <kmime_util.h>
#include <mimelib/mimepp.h>
#include <mimelib/string.h>
#include <assert.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <klocale.h>
#include <kglobal.h>
#include <kwin.h>
#include <stdlib.h>

#if ALLOW_GUI
#include <kmessagebox.h>
#include <ktextbrowser.h>
#include <qmultilineedit.h>
#endif

static DwString emptyString("");

// Values that are set from the config file with KMMessage::readConfig()
static QString sReplyLanguage, sReplyStr, sReplyAllStr, sIndentPrefixStr;
static bool sSmartQuote, sReplaceSubjPrefix, sReplaceForwSubjPrefix;
static int sWrapCol;
static QStringList sReplySubjPrefixes, sForwardSubjPrefixes;
static QStringList sPrefCharsets;

QString KMMessage::sForwardStr = "";
int KMMessage::sHdrStyle = KMReaderWin::HdrFancy;


//-----------------------------------------------------------------------------
KMMessage::KMMessage(DwMessage* aMsg)
  : mMsg(aMsg),
    mNeedsAssembly(true),
    mIsComplete(false),
    mTransferInProgress(false),
    mDecodeHTML(false),
    mCodec(0)
{
}

//-----------------------------------------------------------------------------
KMMessage::KMMessage(const KMMessage& other) : KMMessageInherited( other ), mMsg(0)
{
  assign( other );
}

void KMMessage::assign( const KMMessage& other )
{
  if( mMsg ) delete mMsg;

  mNeedsAssembly = true;//other.mNeedsAssembly;
  mMsg = new DwMessage( *(other.mMsg) );
  mCodec = other.mCodec;
  mDecodeHTML = other.mDecodeHTML;
  mIsComplete = false;//other.mIsComplete;
  mTransferInProgress = other.mTransferInProgress;
  mMsgSize = other.mMsgSize;
  mMsgLength = other.mMsgLength;
  mFolderOffset = other.mFolderOffset;
  mStatus  = other.mStatus;
  mEncryptionState = other.mEncryptionState;
  mSignatureState = other.mSignatureState;
  mDate    = other.mDate;
  //mFileName = ""; // we might not want to copy the other messages filename (?)
  //mMsgSerNum = other.mMsgSerNum; // what about serial number ?
  //KMMsgBase::assign( &other );
}

//-----------------------------------------------------------------------------
void KMMessage::setReferences(const QCString& aStr)
{
  if (!aStr) return;
  mMsg->Headers().References().FromString(aStr);
  mNeedsAssembly = TRUE;
}


//-----------------------------------------------------------------------------
QCString KMMessage::id(void) const
{
  DwHeaders& header = mMsg->Headers();
  if (header.HasMessageId())
    return header.MessageId().AsString().c_str();
  else
    return "";
}


//-----------------------------------------------------------------------------
unsigned long KMMessage::getMsgSerNum() const
{
  if (mMsgSerNum)
    return mMsgSerNum;
  return KMMsgBase::getMsgSerNum();
}


//-----------------------------------------------------------------------------
void KMMessage::setMsgSerNum(unsigned long newMsgSerNum)
{
  if (newMsgSerNum)
    mMsgSerNum = newMsgSerNum;
  else if (!mMsgSerNum)
      mMsgSerNum = getMsgSerNum();
}


//-----------------------------------------------------------------------------
KMMessage::KMMessage(KMFolder* parent): KMMessageInherited(parent)
{
  mNeedsAssembly = FALSE;
  mMsg = new DwMessage;
  mCodec = NULL;
  mDecodeHTML = FALSE;
  mIsComplete = FALSE;
  mTransferInProgress = FALSE;
  mMsgSize = 0;
  mMsgLength = 0;
  mFolderOffset = 0;
  mStatus  = KMMsgStatusNew;
  mEncryptionState = KMMsgEncryptionStateUnknown;
  mSignatureState = KMMsgSignatureStateUnknown;
  mDate    = 0;
  mFileName = "";
  mMsgSerNum = 0;
}


//-----------------------------------------------------------------------------
KMMessage::KMMessage(KMMsgInfo& msgInfo): KMMessageInherited()
{
  mNeedsAssembly = FALSE;
  mMsg = new DwMessage;
  mCodec = NULL;
  mDecodeHTML = FALSE;
  mIsComplete = FALSE;
  mTransferInProgress = FALSE;
  mMsgSize = msgInfo.msgSize();
  mMsgLength = 0;
  mFolderOffset = msgInfo.folderOffset();
  mStatus = msgInfo.status();
  mEncryptionState = msgInfo.encryptionState();
  mSignatureState = msgInfo.signatureState();
  mDate = msgInfo.date();
  mFileName = msgInfo.fileName();
  mMsgSerNum = msgInfo.getMsgSerNum();
  KMMsgBase::assign(&msgInfo);
}


//-----------------------------------------------------------------------------
KMMessage::~KMMessage()
{
  if (mMsg) delete mMsg;
  kernel->undoStack()->msgDestroyed( this );
}


//-----------------------------------------------------------------------------
bool KMMessage::isMessage(void) const
{
  return TRUE;
}


//-----------------------------------------------------------------------------
const DwString& KMMessage::asDwString(void)
{
  if (mNeedsAssembly)
  {
    mNeedsAssembly = FALSE;
    mMsg->Assemble();
  }
  return mMsg->AsString();
}


//-----------------------------------------------------------------------------
QCString KMMessage::asString(void)
{
  if (mNeedsAssembly)
  {
    mNeedsAssembly = FALSE;
    mMsg->Assemble();
  }
  return mMsg->AsString().c_str();
}


//-----------------------------------------------------------------------------
QCString KMMessage::asSendableString()
{
  KMMessage msg;
  msg.fromString(asString());
  msg.removeHeaderField("Status");
  msg.removeHeaderField("X-Status");
  msg.removeHeaderField("X-KMail-EncryptionState");
  msg.removeHeaderField("X-KMail-SignatureState");
  msg.removeHeaderField("X-KMail-Transport");
  msg.removeHeaderField("X-KMail-Identity");
  msg.removeHeaderField("X-KMail-Fcc");
  msg.removeHeaderField("X-KMail-Redirect-From");
  msg.removeHeaderField("X-KMail-Link-Message");
  msg.removeHeaderField("X-KMail-Link-Type");
  msg.removeHeaderField("Bcc");
  return msg.asString();
}

//-----------------------------------------------------------------------------
void KMMessage::setStatusFields(void)
{
    char str[3];

  str[0] = (char)status();
  str[1] = '\0';
  setHeaderField("Status", status()==KMMsgStatusNew ? "R " : "RO");
  setHeaderField("X-Status", str);

  str[0] = (char)encryptionState();
  str[1] = '\0';
  setHeaderField("X-KMail-EncryptionState", str);

  str[0] = (char)signatureState();
  str[1] = '\0';
  qDebug( "Setting SignatureState header field to %c", str[0] );
  setHeaderField("X-KMail-SignatureState", str);

}


//----------------------------------------------------------------------------
QString KMMessage::headerAsString(void) const
{
  DwHeaders& header = mMsg->Headers();
  if(header.AsString() != "")
    return header.AsString().c_str();
  return "";
}


//-----------------------------------------------------------------------------
DwMediaType& KMMessage::dwContentType(void)
{
  return mMsg->Headers().ContentType();
}


//-----------------------------------------------------------------------------
void KMMessage::fromString(const QCString& aStr, bool aSetStatus)
{
  int len;
  const char* strPos;
  char* resultPos;
  char ch;
  QCString result;

  if (mMsg) delete mMsg;
  mMsg = new DwMessage;

  // copy string and throw out obsolete control characters
  len = aStr.length();
  mMsgLength = len;
  result.resize(len+1);
  strPos = aStr.data();
  resultPos = (char*)result.data();
  if (strPos) for (; (ch=*strPos)!='\0'; strPos++)
  {
//  Mail header charset(iso-2022-jp) is using all most E-mail system in Japan.
//  ISO-2022-JP code consists of ESC(0x1b) character and 7Bit character which
//  used from '!' character to  '~' character.  toyo
    if ((ch>=' ' || ch=='\t' || ch=='\n' || ch<='\0' || ch == 0x1b)
       && !(ch=='>' && strPos > aStr.data()
            && qstrncmp(strPos-1, "\n>From", 6) == 0))
      *resultPos++ = ch;
  }
  *resultPos = '\0'; // terminate zero for casting
  mMsg->FromString((const char*)result);
  mMsg->Parse();

  if (aSetStatus) {
    setStatus(headerField("Status").latin1(), headerField("X-Status").latin1());
    setEncryptionState(headerField("X-KMail-EncryptionState").latin1());
    setSignatureState(headerField("X-KMail-SignatureState").latin1());
  }

  mNeedsAssembly = FALSE;
    mDate = date();

  // Convert messages with a binary body into a message with attachment.
  QCString ct = dwContentType().TypeStr().c_str();
  QCString st = dwContentType().SubtypeStr().c_str();
  ct = ct.lower();
  if (   ct.isEmpty()
      || ct == "text"
      || ct == "multipart"
      || (ct == "application" && (st == "pkcs7-mime" || st == "x-pkcs7-mime")) )
    return;
  KMMessagePart textPart;
  textPart.setTypeStr("text");
  textPart.setSubtypeStr("plain");
  textPart.setBody("\n");
  KMMessagePart bodyPart;
  bodyPart.setTypeStr(ct);
  bodyPart.setSubtypeStr(subtypeStr());
  bodyPart.setContentDisposition(headerField("Content-Disposition").latin1());
  bodyPart.setCteStr(contentTransferEncodingStr());
  bodyPart.setContentDisposition(headerField("Content-Disposition").latin1());
  bodyPart.setBodyEncodedBinary(bodyDecodedBinary());
  addBodyPart(&textPart);
  addBodyPart(&bodyPart);
  mNeedsAssembly = FALSE;
}


//-----------------------------------------------------------------------------
QString KMMessage::formatString(const QString& aStr) const
{
  QString result, str;
  QChar ch;
  uint j;

  if (aStr.isEmpty())
    return aStr;

  for (uint i=0; i<aStr.length();) {
    ch = aStr[i++];
    if (ch == '%') {
      ch = aStr[i++];
      QString langSave = KGlobal::locale()->language();
      switch ((char)ch) {
      case 'D':
	/* I'm not too sure about this change. Is it not possible
	   to have a long form of the date used? I don't
	   like this change to a short XX/XX/YY date format.
	   At least not for the default. -sanders */
	result += KMime::DateFormatter::formatDate( KMime::DateFormatter::Localized,
						    date(), sReplyLanguage, false );
        break;
      case 'e':
        result += from();
        break;
      case 'F':
        result += stripEmailAddr(from());
        break;
      case 'f':
        str = stripEmailAddr(from());

        for (j=0; str[j]>' '; j++)
          ;
        for (; j < str.length() && str[j] <= ' '; j++)
          ;
        result += str[0];
        if (str[j]>' ')
          result += str[j];
        else
          if (str[1]>' ')
            result += str[1];
        break;
      case 'T':
        result += stripEmailAddr(to());
        break;
      case 't':
        result += to();
        break;
      case 'S':
        result += subject();
        break;
      case '_':
        result += ' ';
        break;
      case 'L':
        result += "\n";
        break;
      case '%':
        result += '%';
        break;
      default:
        result += '%';
        result += ch;
        break;
      }
    } else
      result += ch;
  }
  return result;
}

static void removeTrailingSpace( QString &line )
{
   int i = line.length()-1;
   while( (i >= 0) && ((line[i] == ' ') || (line[i] == '\t')))
      i--;
   line.truncate( i+1);
}

static QString splitLine( QString &line)
{
    removeTrailingSpace( line );
    int i = 0;
    int j = -1;
    int l = line.length();

    // TODO: Replace tabs with spaces first.

    while(i < l)
    {
       QChar c = line[i];
       if ((c == '>') || (c == ':') || (c == '|'))
          j = i+1;
       else if ((c != ' ') && (c != '\t'))
          break;
       i++;
    }

    if ( j <= 0 )
    {
       return "";
    }
    if ( i == l )
    {
       QString result = line.left(j);
       line = QString::null;
       return result;
    }

    QString result = line.left(j);
    line = line.mid(j);
    return result;
}

static QString flowText(QString &text, const QString& indent, int maxLength)
{
// printf("flowText: \"%s\"\n", text.ascii());
   maxLength--;
   if (text.isEmpty())
   {
      return indent+"<NULL>\n";
   }
   QString result;
   while (1)
   {
      int i;
      if ((int) text.length() > maxLength)
      {
         i = maxLength;
         while( (i >= 0) && (text[i] != ' '))
            i--;
         if (i <= 0)
         {
            // Couldn't break before maxLength.
            i = maxLength;
//            while( (i < (int) text.length()) && (text[i] != ' '))
//               i++;
         }
      }
      else
      {
         i = text.length();
      }

      QString line = text.left(i);
      if (i < (int) text.length())
         text = text.mid(i);
      else
         text = QString::null;

      result += indent + line + '\n';

      if (text.isEmpty())
         return result;
   }
}

static bool flushPart(QString &msg, QStringList &part,
                      const QString &indent, int maxLength)
{
   maxLength -= indent.length();
   if (maxLength < 20) maxLength = 20;

   // Remove empty lines at end of quote
   while ((part.begin() != part.end()) && part.last().isEmpty())
   {
      part.remove(part.fromLast());
   }

//printf("Start of part.\n");

   QString text;
   for(QStringList::Iterator it2 = part.begin();
       it2 != part.end();
       it2++)
   {
      QString line = (*it2);

      if (line.isEmpty())
      {
         if (!text.isEmpty())
            msg += flowText(text, indent, maxLength);
         msg += indent + "\n";
      }
      else
      {
         if (text.isEmpty())
            text = line;
         else
            text += " "+line.stripWhiteSpace();

         if (((int) text.length() < maxLength) || ((int) line.length() < (maxLength-10)))
            msg += flowText(text, indent, maxLength);
      }
   }
   if (!text.isEmpty())
      msg += flowText(text, indent, maxLength);
//printf("End of of part.\n");

   bool appendEmptyLine = true;
   if (!part.count())
     appendEmptyLine = false;

   part.clear();
   return appendEmptyLine;
}

static void stripSignature(QString& msg, bool clearSigned)
{
  if (clearSigned)
  {
    msg = msg.left(msg.findRev(QRegExp("\\n--\\s?\\n")));
  }
  else
  {
    msg = msg.left(msg.findRev("\n-- \n"));
  }
}

static void smartQuote( QString &msg, const QString &ownIndent, int maxLength )
{
  QStringList part;
  QString oldIndent;
  bool firstPart = true;

//printf("Smart Quoting.\n");


  QStringList lines = QStringList::split('\n', msg);

  msg = QString::null;
  for(QStringList::Iterator it = lines.begin();
      it != lines.end();
      it++)
  {
     QString line = *it;

     QString indent = splitLine( line );

//     printf("Quoted Line = \"%s\" \"%s\"\n", line.ascii(), indent.ascii());
     if ( line.isEmpty())
     {
        if (!firstPart)
           part.append(QString::null);
        continue;
     };

     if (firstPart)
     {
        oldIndent = indent;
        firstPart = false;
     }

     if (oldIndent != indent)
     {
        QString fromLine;
        // Search if the last non-blank line could be "From" line
        if (part.count() && (oldIndent.length() < indent.length()))
        {
           QStringList::Iterator it2 = part.fromLast();
           while( (it2 != part.end()) && (*it2).isEmpty())
             it2--;

           if ((it2 != part.end()) && ((*it2).right(1) == ":"))
           {
              fromLine = oldIndent + (*it2) + "\n";
              part.remove(it2);
           }
        }
        if (flushPart( msg, part, oldIndent, maxLength))
        {
           if (oldIndent.length() > indent.length())
              msg += indent + "\n";
           else
              msg += oldIndent + "\n";
        }
        if (!fromLine.isEmpty())
        {
           msg += fromLine;
//printf("From = %s", fromLine.ascii());
        }
        oldIndent = indent;
     }
     part.append(line);
  }
  flushPart( msg, part, oldIndent, maxLength);
}


//-----------------------------------------------------------------------------
QCString KMMessage::asQuotedString(const QString& aHeaderStr,
                                   const QString& aIndentStr,
                                   const QString& selection,
                                   bool aStripSignature,
                                   bool allowDecryption) const
{
  QString result;
  QCString cStr;
  QString headerStr;
  KMMessagePart msgPart;
  QRegExp reNL("\\n");
  QString indentStr;
  int i;
  bool clearSigned = false;

  QTextCodec *codec = mCodec;
  if (!codec)
  {
    QCString cset = charset();
    if (!cset.isEmpty())
      codec = KMMsgBase::codecForName(cset);
    if (!codec) codec = kernel->networkCodec();
  }

  indentStr = formatString(aIndentStr);
  headerStr = formatString(aHeaderStr);


  // Quote message. Do not quote mime message parts that are of other
  // type than "text".
  if (numBodyParts() == 0 || !selection.isEmpty() ) {
    if( !selection.isEmpty() ) {
      result = selection;
    } else {
      cStr = bodyDecoded();
      Kpgp::Module* pgp = Kpgp::Module::getKpgp();
      assert(pgp != NULL);

      QPtrList<Kpgp::Block> pgpBlocks;
      QStrList nonPgpBlocks;
      if( allowDecryption &&
          Kpgp::Module::prepareMessageForDecryption( cStr,
                                                     pgpBlocks, nonPgpBlocks ) )
      {
        // Only decrypt/strip off the signature if there is only one OpenPGP
        // block in the message
        if( pgpBlocks.count() == 1 )
        {
          Kpgp::Block* block = pgpBlocks.first();
          if( ( block->type() == Kpgp::PgpMessageBlock ) ||
              ( block->type() == Kpgp::ClearsignedBlock ) )
          {
            if( block->type() == Kpgp::PgpMessageBlock )
              // try to decrypt this OpenPGP block
              block->decrypt();
            else
            {
              // strip off the signature
              block->verify();
              clearSigned = true;
            }

            result = codec->toUnicode( nonPgpBlocks.first() )
                   + codec->toUnicode( block->text() )
                   + codec->toUnicode( nonPgpBlocks.last() );
          }
        }
      }
      if( result.isEmpty() )
        result = codec->toUnicode( cStr );
      if (mDecodeHTML && qstrnicmp(typeStr(),"text/html",9) == 0)
      {
        KHTMLPart htmlPart;
        htmlPart.setOnlyLocalReferences(true);
        htmlPart.setMetaRefreshEnabled(false);
        htmlPart.setPluginsEnabled(false);
        htmlPart.setJScriptEnabled(false);
        htmlPart.setJavaEnabled(false);
        htmlPart.begin();
        htmlPart.write(result);
        htmlPart.end();
        htmlPart.selectAll();
        result = htmlPart.selectedText();
      }
    }

    // Remove blank lines at the beginning
    for( i = 0; i < (int)result.length() && result[i] <= ' '; i++ );
    while (i > 0 && result[i-1] == ' ') i--;
    result.remove(0,i);

    if (aStripSignature)
        stripSignature(result, clearSigned);

    result.replace(reNL, '\n' + indentStr);
    result = indentStr + result + '\n';

    if (sSmartQuote)
      smartQuote(result, indentStr, sWrapCol);
  } else {
    result = "";
    bodyPart(0, &msgPart);

    if (qstricmp(msgPart.typeStr(),"text") == 0 ||
      msgPart.typeStr().isEmpty())
    {
      Kpgp::Module* pgp = Kpgp::Module::getKpgp();
      assert(pgp != NULL);
      QString part;

      QPtrList<Kpgp::Block> pgpBlocks;
      QStrList nonPgpBlocks;
      if( allowDecryption &&
          Kpgp::Module::prepareMessageForDecryption( msgPart.bodyDecoded(),
                                                     pgpBlocks, nonPgpBlocks ) )
      {
        // Only decrypt/strip off the signature if there is only one OpenPGP
        // block in the message
        if( pgpBlocks.count() == 1 )
        {
          Kpgp::Block* block = pgpBlocks.first();
          if( ( block->type() == Kpgp::PgpMessageBlock ) ||
              ( block->type() == Kpgp::ClearsignedBlock ) )
          {
            if( block->type() == Kpgp::PgpMessageBlock )
              // try to decrypt this OpenPGP block
              block->decrypt();
            else
            {
              // strip off the signature
              block->verify();
              clearSigned = true;
            }

            part = codec->toUnicode( nonPgpBlocks.first() )
                 + codec->toUnicode( block->text() )
                 + codec->toUnicode( nonPgpBlocks.last() );
          }
        }
      }
      if( part.isEmpty() )
      {
        part = codec->toUnicode( msgPart.bodyDecoded() );
        //	    debug ("part\n" + part ); inexplicably crashes -sanders
      }

      if (aStripSignature)
        stripSignature(part, clearSigned);

      part.replace(reNL, '\n' + indentStr);
      part = indentStr + part + '\n';
      if (sSmartQuote)
        smartQuote(part, indentStr, sWrapCol);
      result += part;
    }
  }

  QCString c = QString(headerStr + result).utf8();

  return c;
}

//-----------------------------------------------------------------------------
KMMessage* KMMessage::createReply(bool replyToAll, bool replyToList,
  QString selection, bool noQuote, bool allowDecryption)
{
  KMMessage* msg = new KMMessage;
  QString str, replyStr, mailingListStr, replyToStr, toStr;
  QCString refStr, headerName;

  msg->initFromMessage(this);

  KMMLInfo::name(this, headerName, mailingListStr);
  replyToStr = replyTo();

  msg->setCharset("utf-8");

  if (replyToList && parent()->isMailingList())
  {
    // Reply to mailing-list posting address
    toStr = parent()->mailingListPostAddress();
  }
  else if (replyToAll)
  {
    QStringList recipients;

    // add addresses from the Reply-To header to the list of recipients
    if (!replyToStr.isEmpty())
      recipients += splitEmailAddrList(replyToStr);

    // add From address to the list of recipients if it's not already there
    if (!from().isEmpty())
      if (recipients.grep(getEmailAddr(from()), false).isEmpty()) {
        recipients += from();
        kdDebug(5006) << "Added " << from() << " to the list of recipients"
                      << endl;
      }

    // add only new addresses from the To header to the list of recipients
    if (!to().isEmpty()) {
      QStringList list = splitEmailAddrList(to());
      for (QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        if (recipients.grep(getEmailAddr(*it), false).isEmpty()) {
          recipients += *it;
          kdDebug(5006) << "Added " << *it << " to the list of recipients"
                        << endl;
        }
      }
    }

    // strip my own address from the list of recipients
    QString myAddr = getEmailAddr(msg->from());
    for (QStringList::Iterator it = recipients.begin();
         it != recipients.end(); ) {
      if ((*it).find(myAddr,0,false) != -1) {
        kdDebug(5006) << "Removing " << *it << " from the list of recipients"
                      << endl;
        it = recipients.remove(it);
      }
      else
        ++it;
    }

    toStr = recipients.join(", ");

    // the same for the cc field
    if (!cc().isEmpty()) {
      recipients = splitEmailAddrList(cc());

      // strip my own address
      for (QStringList::Iterator it = recipients.begin();
           it != recipients.end(); ) {
        if ((*it).find(myAddr,0,false) != -1) {
          kdDebug(5006) << "Removing " << *it << " from the cc recipients"
                        << endl;
          it = recipients.remove(it);
        }
        else
          ++it;
      }

      msg->setCc(recipients.join(", "));
    }

  }
  else
  {
    if (!replyToStr.isEmpty()) toStr = replyToStr;
    else if (!from().isEmpty()) toStr = from();
  }

  msg->setTo(toStr);

  refStr = getRefStr();
  if (!refStr.isEmpty())
    msg->setReferences(refStr);
  //In-Reply-To = original msg-id
  msg->setReplyToId(msgId());

  if (replyToAll || replyToList || !mailingListStr.isEmpty()
      || parent()->isMailingList())
    replyStr = sReplyAllStr;
  else replyStr = sReplyStr;
  replyStr += "\n";

  if (!noQuote)
  msg->setBody(asQuotedString(replyStr, sIndentPrefixStr, selection, true, allowDecryption));

  // replace arbitrary sequences of reply prefixes:
  bool recognized = false;
  // construct a big regexp that
  // 1. is anchored to the beginning of the subject (sans whitespace)
  // 2. matches at least one of the part regexps in sReplySubjPrefixes
  QString bigRegExp = QString::fromLatin1("^(?:\\s+|(?:%1))+\\s+")
    .arg( sReplySubjPrefixes.join(")|(?:") );
  kdDebug(5006) << "KMMessage::createReply(): bigRegExp = \"" << bigRegExp
		<< "\"" << endl;
  QRegExp rx( bigRegExp, false /*case insens.*/ );
  if ( !rx.isValid() )
  {
    kdDebug(5006) << "reply prefix regexp is " << "invalid!" << endl;
    // try good ole Re:
    recognized = subject().startsWith("Re:");
  } else { // valid rx
    QString subj = subject();
    if ( rx.search( subj ) == 0 ) { // matches

      recognized = true;
      if ( sReplaceSubjPrefix )
	msg->setSubject( subj.replace( 0, rx.matchedLength(), "Re: " ) );
    } else
      recognized = false;
  }
  if (!recognized)
    msg->setSubject("Re: " + subject());

  // setStatus(KMMsgStatusReplied);
  msg->link(this, KMMsgStatusReplied);

  return msg;
}


//-----------------------------------------------------------------------------
QCString KMMessage::getRefStr()
{
  QCString firstRef, lastRef, refStr, retRefStr;
  int i, j;

  refStr = headerField("References").stripWhiteSpace().latin1();

  if (refStr.isEmpty())
    return headerField("Message-Id").latin1();

  i = refStr.find("<");
  j = refStr.find(">");
  firstRef = refStr.mid(i, j-i+1);
  if (!firstRef.isEmpty())
    retRefStr = firstRef + " ";

  i = refStr.findRev("<");
  j = refStr.findRev(">");

  lastRef = refStr.mid(i, j-i+1);
  if (!lastRef.isEmpty() && lastRef != firstRef)
    retRefStr += lastRef + " ";

  retRefStr += headerField("Message-Id").latin1();
  return retRefStr;
}


KMMessage* KMMessage::createRedirect(void)
{
  KMMessage* msg = new KMMessage;
  KMMessagePart msgPart;
  int i;

  msg->initFromMessage(this);

  /// ### FIXME: The message should be redirected with the same Content-Type
  /// ###        as the original message
  /// ### FIXME: ??Add some Resent-* headers?? (c.f. RFC2822 3.6.6)

  QString st = QString::fromUtf8(asQuotedString("", "", QString::null,
    false, false));
  QCString encoding = autoDetectCharset(charset(), sPrefCharsets, st);
  if (encoding.isEmpty()) encoding = "utf-8";
  QCString str = codecForName(encoding)->fromUnicode(st);

  msg->setCharset(encoding);
  msg->setBody(str);

  if (numBodyParts() > 0)
  {
    msgPart.setBody(str);
    msgPart.setTypeStr("text");
    msgPart.setSubtypeStr("plain");
    msgPart.setCharset(encoding);
    msg->addBodyPart(&msgPart);

    for (i = 0; i < numBodyParts(); i++)
    {
      bodyPart(i, &msgPart);
      if ((qstricmp(msgPart.contentDisposition(),"inline")!=0 && i > 0) ||
	  (qstricmp(msgPart.typeStr(),"text")!=0 &&
	   qstricmp(msgPart.typeStr(),"message")!=0))
      {
	msg->addBodyPart(&msgPart);
      }
    }
  }

//TODO: insert sender here
  msg->setHeaderField("X-KMail-Redirect-From", from());
  msg->setSubject(subject());
  msg->setFrom(from());
  msg->cleanupHeader();

  // setStatus(KMMsgStatusForwarded);
  msg->link(this, KMMsgStatusForwarded);

  return msg;
}

#if ALLOW_GUI
KMMessage* KMMessage::createBounce( bool withUI )
#else
KMMessage* KMMessage::createBounce( bool )
#endif
{
  QString fromStr, bodyStr, senderStr;
  int atIdx, i;

  const char* fromFields[] = { "Errors-To", "Return-Path", "Resent-From",
			       "Resent-Sender", "From", "Sender", 0 };

  // Find email address of sender
  for (i=0; fromFields[i]; i++)
  {
    senderStr = headerField(fromFields[i]);
    if (!senderStr.isEmpty()) break;
  }
  if (senderStr.isEmpty())
  {
#if ALLOW_GUI
    if ( withUI )
      KMessageBox::sorry(0 /*app-global modal*/,
			 i18n("The message has no sender set"),
			 i18n("Bounce Message"));
#endif
    return 0;
  }

  QString receiver = headerField("Received");
  int a = -1, b = -1;
  a = receiver.find("from");
  if (a != -1) a = receiver.find("by", a);
  if (a != -1) a = receiver.find("for", a);
  if (a != -1) a = receiver.find('<', a);
  if (a != -1) b = receiver.find('>', a);
  if (a != -1 && b != -1) receiver = receiver.mid(a+1, b-a-1);
  else receiver = getEmailAddr(to());

#if ALLOW_GUI
  if ( withUI )
    // No composer appears. So better ask before sending.
    if (KMessageBox::warningContinueCancel(0 /*app-global modal*/,
        i18n("Return the message to the sender as undeliverable?\n"
	     "This will only work if the email address of the sender, "
	     "%1, is valid.\n"
             "The failing address will be reported to be %2.")
        .arg(senderStr).arg(receiver),
	i18n("Bounce Message"), i18n("Continue")) == KMessageBox::Cancel)
    {
      return 0;
    }
#endif

  KMMessage *msg = new KMMessage;
  msg->initFromMessage(this, FALSE);
  msg->setTo( senderStr );
  msg->setDateToday();
  msg->setSubject( "mail failed, returning to sender" );

  fromStr = receiver;
  atIdx = fromStr.find('@');
  msg->setFrom( fromStr.replace( 0, atIdx, "MAILER-DAEMON" ) );
  msg->setReferences( id() );

  bodyStr = "|------------------------- Message log follows: -------------------------|\n"
        "no valid recipients were found for this message\n"
	"|------------------------- Failed addresses follow: ---------------------|\n";
  bodyStr += receiver;
  bodyStr += "\n|------------------------- Message text follows: ------------------------|\n";
  bodyStr += asSendableString();

  msg->setBody( bodyStr.latin1() );
  msg->cleanupHeader();

  return msg;
}


//-----------------------------------------------------------------------------
QCString KMMessage::createForwardBody(void)
{
  QString s;
  QCString str;

  if (sHdrStyle == KMReaderWin::HdrAll) {
    s = "\n\n----------  " + sForwardStr + "  ----------\n\n";
    s += headerAsString();
    str = asQuotedString(s, "", QString::null, false, false);
    str += "\n-------------------------------------------------------\n";
  } else {
    s = "\n\n----------  " + sForwardStr + "  ----------\n\n";
    s += "Subject: " + subject() + "\n";
    s += "Date: "
         + KMime::DateFormatter::formatDate( KMime::DateFormatter::Localized, 
                                             date(), sReplyLanguage, false )
         + "\n";
    s += "From: " + from() + "\n";
    s += "To: " + to() + "\n";
    if (!cc().isEmpty()) s += "Cc: " + cc() + "\n";
    s += "\n";
    str = asQuotedString(s, "", QString::null, false, false);
    str += "\n-------------------------------------------------------\n";
  }

  return str;
}

//-----------------------------------------------------------------------------
KMMessage* KMMessage::createForward(void)
{
  KMMessage* msg = new KMMessage;
  KMMessagePart msgPart;
  QString id;
  int i;

  msg->initFromMessage(this);

  QString st = QString::fromUtf8(createForwardBody());
  QCString encoding = autoDetectCharset(charset(), sPrefCharsets, st);
  if (encoding.isEmpty()) encoding = "utf-8";
  QCString str = codecForName(encoding)->fromUnicode(st);

  msg->setCharset(encoding);
  msg->setBody(str);

  if (numBodyParts() > 0)
  {
    msgPart.setTypeStr("text");
    msgPart.setSubtypeStr("plain");
    msgPart.setCharset(encoding);
    msgPart.setBody(str);
    msg->addBodyPart(&msgPart);

    for (i = 0; i < numBodyParts(); i++)
    {
      bodyPart(i, &msgPart);
      if (i > 0 || qstricmp(msgPart.typeStr(),"text") != 0)
        msg->addBodyPart(&msgPart);
    }
  }

  QStringList::Iterator it;
  bool recognized = false;
  for (it = sForwardSubjPrefixes.begin(); !recognized && (it != sForwardSubjPrefixes.end()); ++it)
  {
    QString prefix = subject().left((*it).length());
    if (prefix.lower() == (*it).lower()) //recognized
    {
      if (!sReplaceForwSubjPrefix || (prefix == "Fwd:"))
        msg->setSubject(subject());
      else
      {
        //replace recognized prefix with "Fwd: "
        //handle crappy subjects Fwd:  blah blah (note double space)
        int subjStart = (*it).length();
        while (subject()[subjStart].isSpace()) //strip only from beginning
          subjStart++;
        msg->setSubject("Fwd: " + subject().mid(subjStart,
                                   subject().length() - subjStart));
      }
      recognized = true;
    }
  }
  if (!recognized)
    msg->setSubject("Fwd: " + subject());
  msg->cleanupHeader();

  // setStatus(KMMsgStatusForwarded);
  msg->link(this, KMMsgStatusForwarded);

  return msg;
}

KMMessage* KMMessage::createDeliveryReceipt() const
{
  QString str, receiptTo;
  KMMessage *receipt;

  receiptTo = headerField("Disposition-Notification-To");
  if ( receiptTo.stripWhiteSpace().isEmpty() ) return 0;
  receiptTo.replace(QRegExp("\\n"),"");

  receipt = new KMMessage;
  receipt->initFromMessage(this);
  receipt->setTo(receiptTo);
  receipt->setSubject(i18n("Receipt: ") + subject());

  str  = "Your message was successfully delivered.";
  str += "\n\n---------- Message header follows ----------\n";
  str += headerAsString();
  str += "--------------------------------------------\n";
  // Conversion to latin1 is correct here as Mail headers should contain
  // ascii only
  receipt->setBody(str.latin1());
  receipt->setAutomaticFields();

  return receipt;
}

//-----------------------------------------------------------------------------
void KMMessage::initHeader( uint id )
{
  const KMIdentity & ident =
    kernel->identityManager()->identityForUoidOrDefault( id );

  if(ident.fullEmailAddr().isEmpty())
    setFrom("");
  else
    setFrom(ident.fullEmailAddr());

  if(ident.replyToAddr().isEmpty())
    setReplyTo("");
  else
    setReplyTo(ident.replyToAddr());

  if (ident.organization().isEmpty())
    removeHeaderField("Organization");
  else
    setHeaderField("Organization", ident.organization());

  if (ident.isDefault())
    removeHeaderField("X-KMail-Identity");
  else
    setHeaderField("X-KMail-Identity", QString().setNum( ident.uoid() ));

  if (ident.transport().isEmpty())
    removeHeaderField("X-KMail-Transport");
  else
    setHeaderField("X-KMail-Transport", ident.transport());

  if (ident.fcc().isEmpty())
    setFcc( QString::null );
  else
    setFcc( ident.fcc() );

  if (ident.drafts().isEmpty())
    setDrafts( QString::null );
  else
    setDrafts( ident.drafts() );

  setTo("");
  setSubject("");
  setDateToday();

  setHeaderField("User-Agent", "KMail/" KMAIL_VERSION );
  // This will allow to change Content-Type:
  setHeaderField("Content-Type","text/plain");
}


//-----------------------------------------------------------------------------
void KMMessage::initFromMessage(const KMMessage *msg, bool idHeaders)
{
  QString idString = msg->headerField("X-KMail-Identity").stripWhiteSpace();
  bool ok = false;
  uint id = idString.toUInt( &ok );
  
  if ( !ok || id == 0 )
    id = kernel->identityManager()->identityForAddress( msg->to() + msg->cc() ).uoid();
  if ( id == 0 && msg->parent() )
    id = msg->parent()->identity();
  if ( idHeaders ) initHeader(id);
  else setHeaderField("X-KMail-Identity", QString().setNum(id));
  if (!msg->headerField("X-KMail-Transport").isEmpty())
    setHeaderField("X-KMail-Transport", msg->headerField("X-KMail-Transport"));
}


//-----------------------------------------------------------------------------
void KMMessage::cleanupHeader(void)
{
  DwHeaders& header = mMsg->Headers();
  DwField* field = header.FirstField();
  DwField* nextField;

  if (mNeedsAssembly) mMsg->Assemble();
  mNeedsAssembly = FALSE;

  while (field)
  {
    nextField = field->Next();
    if (field->FieldBody()->AsString().empty())
    {
      header.RemoveField(field);
      mNeedsAssembly = TRUE;
    }
    field = nextField;
  }
}


//-----------------------------------------------------------------------------
void KMMessage::setAutomaticFields(bool aIsMulti)
{
  DwHeaders& header = mMsg->Headers();
  header.MimeVersion().FromString("1.0");

  if (aIsMulti || numBodyParts() > 1)
  {
    // Set the type to 'Multipart' and the subtype to 'Mixed'
    DwMediaType& contentType = dwContentType();
    contentType.SetType(   DwMime::kTypeMultipart);
    contentType.SetSubtype(DwMime::kSubtypeMixed );

    // Create a random printable string and set it as the boundary parameter
    contentType.CreateBoundary(0);
  }
  mNeedsAssembly = TRUE;
}


//-----------------------------------------------------------------------------
QString KMMessage::dateStr(void) const
{
  KConfigGroup general( kapp->config(), "General" );
  DwHeaders& header = mMsg->Headers();
  time_t unixTime;

  if (!header.HasDate()) return "";
  unixTime = header.Date().AsUnixTime();

  return KMime::DateFormatter::formatDate( static_cast<KMime::DateFormatter::FormatType>(general.readNumEntry( "dateFormat", KMime::DateFormatter::Fancy )),
					   unixTime, general.readEntry( "customDateFormat" ) );
}


//-----------------------------------------------------------------------------
QCString KMMessage::dateShortStr(void) const
{
  DwHeaders& header = mMsg->Headers();
  time_t unixTime;

  if (!header.HasDate()) return "";
  unixTime = header.Date().AsUnixTime();

  QCString result = ctime(&unixTime);

  if (result[result.length()-1]=='\n')
    result.truncate(result.length()-1);

  return result;
}


//-----------------------------------------------------------------------------
QString KMMessage::dateIsoStr(void) const
{
  DwHeaders& header = mMsg->Headers();
  time_t unixTime;

  if (!header.HasDate()) return "";
  unixTime = header.Date().AsUnixTime();

  char cstr[64];
  strftime(cstr, 63, "%Y-%m-%d %H:%M:%S", localtime(&unixTime));
  return QString(cstr);
}


//-----------------------------------------------------------------------------
time_t KMMessage::date(void) const
{
  DwHeaders& header = mMsg->Headers();
  if (header.HasDate()) return header.Date().AsUnixTime();
  return (time_t)-1;
}


//-----------------------------------------------------------------------------
void KMMessage::setDateToday(void)
{
  struct timeval tval;
  gettimeofday(&tval, NULL);
  setDate((time_t)tval.tv_sec);
}


//-----------------------------------------------------------------------------
void KMMessage::setDate(time_t aDate)
{
  mDate = aDate;
  mMsg->Headers().Date().FromCalendarTime(aDate);
  mMsg->Headers().Date().Assemble();
  mNeedsAssembly = TRUE;
  mDirty = TRUE;
}


//-----------------------------------------------------------------------------
void KMMessage::setDate(const QCString& aStr)
{
  DwHeaders& header = mMsg->Headers();

  header.Date().FromString(aStr);
  header.Date().Parse();
  mNeedsAssembly = TRUE;
  mDirty = TRUE;

  if (header.HasDate())
    mDate = header.Date().AsUnixTime();
}


//-----------------------------------------------------------------------------
QString KMMessage::to(void) const
{
  return headerField("To");
}


//-----------------------------------------------------------------------------
void KMMessage::setTo(const QString& aStr)
{
  setHeaderField("To", aStr);
}

//-----------------------------------------------------------------------------
QString KMMessage::toStrip(void) const
{
  return stripEmailAddr(headerField("To"));
}

//-----------------------------------------------------------------------------
QString KMMessage::replyTo(void) const
{
  return headerField("Reply-To");
}


//-----------------------------------------------------------------------------
void KMMessage::setReplyTo(const QString& aStr)
{
  setHeaderField("Reply-To", aStr);
}


//-----------------------------------------------------------------------------
void KMMessage::setReplyTo(KMMessage* aMsg)
{
  setHeaderField("Reply-To", aMsg->from());
}


//-----------------------------------------------------------------------------
QString KMMessage::cc(void) const
{
  return headerField("Cc");
}


//-----------------------------------------------------------------------------
void KMMessage::setCc(const QString& aStr)
{
  setHeaderField("Cc",aStr);
}


//-----------------------------------------------------------------------------
QString KMMessage::bcc(void) const
{
  return headerField("Bcc");
}


//-----------------------------------------------------------------------------
void KMMessage::setBcc(const QString& aStr)
{
  setHeaderField("Bcc", aStr);
}

//-----------------------------------------------------------------------------
QString KMMessage::fcc(void) const
{
  return headerField( "X-KMail-Fcc" );
}


//-----------------------------------------------------------------------------
void KMMessage::setFcc(const QString& aStr)
{
  setHeaderField( "X-KMail-Fcc", aStr );
}

//-----------------------------------------------------------------------------
void KMMessage::setDrafts(const QString& aStr)
{
  mDrafts = aStr;
  kdDebug(5006) << "KMMessage::setDrafts " << aStr << endl;
}

//-----------------------------------------------------------------------------
QString KMMessage::who(void) const
{
  const char* whoField;

  if (mParent) whoField = mParent->whoField();
  else whoField = "From";

  return headerField(whoField);
}


//-----------------------------------------------------------------------------
QString KMMessage::from(void) const
{
  return headerField("From");
}


//-----------------------------------------------------------------------------
void KMMessage::setFrom(const QString& bStr)
{
  QString aStr = bStr;
  if (aStr.isNull())
    aStr = "";
  setHeaderField("From", aStr);
  mDirty = TRUE;
}


//-----------------------------------------------------------------------------
QString KMMessage::fromStrip(void) const
{
  return stripEmailAddr(headerField("From"));
}

//-----------------------------------------------------------------------------
QCString KMMessage::fromEmail(void) const
{
  return getEmailAddr(headerField("From"));
}


//-----------------------------------------------------------------------------
QString KMMessage::subject(void) const
{
  return headerField("Subject");
}


//-----------------------------------------------------------------------------
void KMMessage::setSubject(const QString& aStr)
{
  setHeaderField("Subject",aStr);
  mDirty = TRUE;
}


//-----------------------------------------------------------------------------
QString KMMessage::xmark(void) const
{
  return headerField("X-KMail-Mark");
}


//-----------------------------------------------------------------------------
void KMMessage::setXMark(const QString& aStr)
{
  setHeaderField("X-KMail-Mark", aStr);
  mDirty = TRUE;
}


//-----------------------------------------------------------------------------
QString KMMessage::replyToId(void) const
{
  int leftAngle, rightAngle;
  QString replyTo, references;

  replyTo = headerField("In-Reply-To");
  // search the end of the (first) message id in the In-Reply-To header
  rightAngle = replyTo.find( '>' );
  if (rightAngle != -1)
    replyTo.truncate( rightAngle + 1 );
  // now search the start of the message id
  leftAngle = replyTo.findRev( '<' );
  if (leftAngle != -1)
    replyTo = replyTo.mid( leftAngle );

  // if we have found a good message id we can return immediately
  if (!replyTo.isEmpty() && (replyTo[0] == '<'))
    return replyTo;

  references = headerField("References");
  leftAngle = references.findRev( '<' );
  if (leftAngle != -1)
    references = references.mid( leftAngle );
  rightAngle = references.find( '>' );
  if (rightAngle != -1)
    references.truncate( rightAngle + 1 );

  // if we found a good message id in the References header return it
  if (!references.isEmpty() && references[0] == '<')
    return references;
  // else return the broken message id we found in the In-Reply-To header
  else
    return replyTo;
}


//-----------------------------------------------------------------------------
QString KMMessage::replyToIdMD5(void) const
{
  //  QString result = KMMessagePart::encodeBase64( decodeRFC2047String(replyToId()) );
  QString result = KMMessagePart::encodeBase64( replyToId() );
  return result;
}


//-----------------------------------------------------------------------------
void KMMessage::setReplyToId(const QString& aStr)
{
  setHeaderField("In-Reply-To", aStr);
  mDirty = TRUE;
}


//-----------------------------------------------------------------------------
QString KMMessage::msgId(void) const
{
  int leftAngle, rightAngle;
  QString msgId = headerField("Message-Id");

  // search the end of the message id
  rightAngle = msgId.find( '>' );
  if (rightAngle != -1)
    msgId.truncate( rightAngle + 1 );
  // now search the start of the message id
  leftAngle = msgId.findRev( '<' );
  if (leftAngle != -1)
    msgId = msgId.mid( leftAngle );
  return msgId;
}


//-----------------------------------------------------------------------------
QString KMMessage::msgIdMD5(void) const
{
  //  QString result = KMMessagePart::encodeBase64(  decodeRFC2047String(msgId()) );
  QString result = KMMessagePart::encodeBase64( msgId() );
  return result;
}


//-----------------------------------------------------------------------------
void KMMessage::setMsgId(const QString& aStr)
{
  setHeaderField("Message-Id", aStr);
  mDirty = TRUE;
}


//-----------------------------------------------------------------------------
QStrList KMMessage::headerAddrField(const QCString& aName) const
{
  QString header = headerField(aName);
  QStringList list = splitEmailAddrList(header);
  QStrList resultList;
  int i,j;
  for (QStringList::Iterator it = list.begin(); it != list.end(); it++)
  {
    i = (*it).find('<');
    if (i >= 0)
    {
      j = (*it).find('>', i+1);
      if (j > i) (*it) = (*it).mid(i+1, j-i-1);
    }
    else // if it's "radej@kde.org (Sven Radej)"
    {
      i = (*it).find('(');
      if (i > 0)
        (*it).truncate(i);  // "radej@kde.org "
    }
    (*it) = (*it).stripWhiteSpace();
    if (!(*it).isEmpty())
      resultList.append((*it).latin1());
  }
  return resultList;
}


//-----------------------------------------------------------------------------
QString KMMessage::headerField(const QCString& aName) const
{
  DwHeaders& header = mMsg->Headers();
  DwField* field;
  QString result;

  if (aName.isEmpty() || !(field = header.FindField(aName)))
    result = "";
  else
    result = decodeRFC2047String(header.FieldBody(aName.data()).
                    AsString().c_str());
  return result;
}


//-----------------------------------------------------------------------------
void KMMessage::removeHeaderField(const QCString& aName)
{
  DwHeaders& header = mMsg->Headers();
  DwField* field;

  field = header.FindField(aName);
  if (!field) return;

  header.RemoveField(field);
  mNeedsAssembly = TRUE;
}


//-----------------------------------------------------------------------------
void KMMessage::setHeaderField(const QCString& aName, const QString& bValue)
{
  DwHeaders& header = mMsg->Headers();
  DwString str;
  DwField* field;
  QCString aValue = "";
  if (!bValue.isEmpty())
  {
    QCString encoding = autoDetectCharset(charset(), sPrefCharsets, bValue);
    if (encoding.isEmpty())
       encoding = "utf-8";
    aValue = encodeRFC2047String(bValue, encoding);
  }

  if (aName.isEmpty()) return;

  str = aName;
  if (str[str.length()-1] != ':') str += ": ";
  else str += " ";
  str += aValue;
  if (aValue.right(1)!="\n") str += "\n";

  field = new DwField(str, mMsg);
  field->Parse();

  header.AddOrReplaceField(field);
  mNeedsAssembly = TRUE;
}


//-----------------------------------------------------------------------------
QCString KMMessage::typeStr(void) const
{
  DwHeaders& header = mMsg->Headers();
  if (header.HasContentType()) return header.ContentType().AsString().c_str();
  else return "";
}


//-----------------------------------------------------------------------------
int KMMessage::type(void) const
{
  DwHeaders& header = mMsg->Headers();
  if (header.HasContentType()) return header.ContentType().Type();
  else return DwMime::kTypeNull;
}


//-----------------------------------------------------------------------------
void KMMessage::setTypeStr(const QCString& aStr)
{
  dwContentType().SetTypeStr(DwString(aStr));
  dwContentType().Parse();
  mNeedsAssembly = TRUE;
}


//-----------------------------------------------------------------------------
void KMMessage::setType(int aType)
{
  dwContentType().SetType(aType);
  dwContentType().Assemble();
  mNeedsAssembly = TRUE;
}



//-----------------------------------------------------------------------------
QCString KMMessage::subtypeStr(void) const
{
  DwHeaders& header = mMsg->Headers();
  if (header.HasContentType()) return header.ContentType().SubtypeStr().c_str();
  else return "";
}


//-----------------------------------------------------------------------------
int KMMessage::subtype(void) const
{
  DwHeaders& header = mMsg->Headers();
  if (header.HasContentType()) return header.ContentType().Subtype();
  else return DwMime::kSubtypeNull;
}


//-----------------------------------------------------------------------------
void KMMessage::setSubtypeStr(const QCString& aStr)
{
  dwContentType().SetSubtypeStr(DwString(aStr));
  dwContentType().Parse();
  mNeedsAssembly = TRUE;
}


//-----------------------------------------------------------------------------
void KMMessage::setSubtype(int aSubtype)
{
  dwContentType().SetSubtype(aSubtype);
  dwContentType().Assemble();
  mNeedsAssembly = TRUE;
}


//-----------------------------------------------------------------------------
void KMMessage::setDwMediaTypeParam( DwMediaType &mType,
                                     const QCString& attr,
                                     const QCString& val )
{
  mType.Parse();
  DwParameter *param = mType.FirstParameter();
  while(param) {
    if (!qstricmp(param->Attribute().c_str(), attr))
      break;
    else
      param = param->Next();
  }
  if (!param){
    param = new DwParameter;
    param->SetAttribute(DwString( attr ));
    mType.AddParameter( param );
  }
  else
    mType.SetModified();
  param->SetValue(DwString( val ));
  mType.Assemble();
}


//-----------------------------------------------------------------------------
void KMMessage::setContentTypeParam(const QCString& attr, const QCString& val)
{
  if (mNeedsAssembly) mMsg->Assemble();
  mNeedsAssembly = FALSE;
  setDwMediaTypeParam( dwContentType(), attr, val );
  mNeedsAssembly = TRUE;
}


//-----------------------------------------------------------------------------
QCString KMMessage::contentTransferEncodingStr(void) const
{
  DwHeaders& header = mMsg->Headers();
  if (header.HasContentTransferEncoding())
    return header.ContentTransferEncoding().AsString().c_str();
  else return "";
}


//-----------------------------------------------------------------------------
int KMMessage::contentTransferEncoding(void) const
{
  DwHeaders& header = mMsg->Headers();
  if (header.HasContentTransferEncoding())
    return header.ContentTransferEncoding().AsEnum();
  else return DwMime::kCteNull;
}


//-----------------------------------------------------------------------------
void KMMessage::setContentTransferEncodingStr(const QCString& aStr)
{
  mMsg->Headers().ContentTransferEncoding().FromString(aStr);
  mMsg->Headers().ContentTransferEncoding().Parse();
  mNeedsAssembly = TRUE;
}


//-----------------------------------------------------------------------------
void KMMessage::setContentTransferEncoding(int aCte)
{
  mMsg->Headers().ContentTransferEncoding().FromEnum(aCte);
  mNeedsAssembly = TRUE;
}


//-----------------------------------------------------------------------------
DwHeaders& KMMessage::headers(void)
{
  return mMsg->Headers();
}


//-----------------------------------------------------------------------------
QCString KMMessage::body(void) const
{
  DwString body = mMsg->Body().AsString();
  QCString str = body.c_str();
  kdWarning( str.length() != body.length(), 5006 )
    << "KMMessage::body(): body is binary but used as text!" << endl;
  return str;
}


//-----------------------------------------------------------------------------
QByteArray KMMessage::bodyDecodedBinary(void) const
{
  DwString dwstr;
  DwString dwsrc = mMsg->Body().AsString();

  switch (cte())
  {
  case DwMime::kCteBase64:
    DwDecodeBase64(dwsrc, dwstr);
    break;
  case DwMime::kCteQuotedPrintable:
    DwDecodeQuotedPrintable(dwsrc, dwstr);
    break;
  default:
    dwstr = dwsrc;
    break;
  }

  int len = dwstr.size();
  QByteArray ba(len);
  memcpy(ba.data(),dwstr.data(),len);
  return ba;
}


//-----------------------------------------------------------------------------
QCString KMMessage::bodyDecoded(void) const
{
  DwString dwstr;
  DwString dwsrc = mMsg->Body().AsString();

  switch (cte())
  {
  case DwMime::kCteBase64:
    DwDecodeBase64(dwsrc, dwstr);
    break;
  case DwMime::kCteQuotedPrintable:
    DwDecodeQuotedPrintable(dwsrc, dwstr);
    break;
  default:
    dwstr = dwsrc;
    break;
  }

  unsigned int len = dwstr.size();
  QCString result(len+1);
  memcpy(result.data(),dwstr.data(),len);
  result[len] = 0;
  kdWarning(result.length() != len, 5006)
    << "KMMessage::bodyDecoded(): body is binary but used as text!" << endl;
  return result;
}


//-----------------------------------------------------------------------------
void KMMessage::setBodyEncoded(const QCString& aStr)
{
  DwString dwSrc(aStr.data(), aStr.size()-1 /* not the trailing NUL */);
  DwString dwResult;

  switch (cte())
  {
  case DwMime::kCteBase64:
    DwEncodeBase64(dwSrc, dwResult);
    break;
  case DwMime::kCteQuotedPrintable:
    DwEncodeQuotedPrintable(dwSrc, dwResult);
    break;
  default:
    dwResult = dwSrc;
    break;
  }

  mMsg->Body().FromString(dwResult);
  mNeedsAssembly = TRUE;
}

//-----------------------------------------------------------------------------
void KMMessage::setBodyEncodedBinary(const QByteArray& aStr)
{
  DwString dwSrc(aStr.data(), aStr.size());
  DwString dwResult;

  switch (cte())
  {
  case DwMime::kCteBase64:
    DwEncodeBase64(dwSrc, dwResult);
    break;
  case DwMime::kCteQuotedPrintable:
    DwEncodeQuotedPrintable(dwSrc, dwResult);
    break;
  default:
    dwResult = dwSrc;
    break;
  }

  mMsg->Body().FromString(dwResult);
  mNeedsAssembly = TRUE;
}


//-----------------------------------------------------------------------------
void KMMessage::setBody(const QCString& aStr)
{
  mMsg->Body().FromString(aStr.data());
  mNeedsAssembly = TRUE;
}


// Patched by Daniel Moisset <dmoisset@grulic.org.ar>
// modified numbodyparts, bodypart to take nested body parts as
// a linear sequence.
// third revision, Sep 26 2000

// this is support structure for traversing tree without recursion

//-----------------------------------------------------------------------------
int KMMessage::numBodyParts(void) const
{
  int count = 0;
  DwBodyPart* part = getFirstDwBodyPart();
  QPtrList< DwBodyPart > parts;
  QString mp = "multipart";

  while (part)
  {
    //dive into multipart messages
    while ( part && part->Headers().HasContentType() &&
	    (mp == part->Headers().ContentType().TypeStr().c_str()) )
    {
      parts.append( part );
      part = part->Body().FirstBodyPart();
    }
    // this is where currPart->msgPart contains a leaf message part
    count++;
    // go up in the tree until reaching a node with next
    // (or the last top-level node)
    while (part && !(part->Next()) && !(parts.isEmpty()))
    {
      part = parts.getLast();
      parts.removeLast();
    };

    if (part)
      part = part->Next();
  }

  return count;
}


//-----------------------------------------------------------------------------
DwBodyPart * KMMessage::getFirstDwBodyPart() const
{
  return mMsg->Body().FirstBodyPart();
}


//-----------------------------------------------------------------------------
int KMMessage::partNumber( DwBodyPart * aDwBodyPart ) const
{
  DwBodyPart *curpart;
  QPtrList< DwBodyPart > parts;
  int curIdx = 0;
  int idx = 0;
  // Get the DwBodyPart for this index

  curpart = getFirstDwBodyPart();

  while (curpart && !idx) {
    //dive into multipart messages
    while(    curpart
           && curpart->Headers().HasContentType()
           && (DwMime::kTypeMultipart == curpart->Headers().ContentType().Type()) )
    {
      parts.append( curpart );
      curpart = curpart->Body().FirstBodyPart();
    }
    // this is where currPart->msgPart contains a leaf message part
    if (curpart == aDwBodyPart)
      idx = curIdx;
    curIdx++;
    // go up in the tree until reaching a node with next
    // (or the last top-level node)
    while (curpart && !(curpart->Next()) && !(parts.isEmpty()))
    {
      curpart = parts.getLast();
      parts.removeLast();
    } ;
    if (curpart)
      curpart = curpart->Next();
  }
  return idx;
}


//-----------------------------------------------------------------------------
DwBodyPart * KMMessage::dwBodyPart( int aIdx ) const
{
  DwBodyPart *part, *curpart;
  QPtrList< DwBodyPart > parts;
  int curIdx = 0;
  // Get the DwBodyPart for this index

  curpart = getFirstDwBodyPart();
  part = 0;

  while (curpart && !part) {
    //dive into multipart messages
    while(    curpart
           && curpart->Headers().HasContentType()
           && (DwMime::kTypeMultipart == curpart->Headers().ContentType().Type()) )
    {
      parts.append( curpart );
      curpart = curpart->Body().FirstBodyPart();
    }
    // this is where currPart->msgPart contains a leaf message part
    if (curIdx==aIdx)
        part = curpart;
    curIdx++;
    // go up in the tree until reaching a node with next
    // (or the last top-level node)
    while (curpart && !(curpart->Next()) && !(parts.isEmpty()))
    {
      curpart = parts.getLast();
      parts.removeLast();
    } ;
    if (curpart)
      curpart = curpart->Next();
  }
  return part;
}



//-----------------------------------------------------------------------------
void KMMessage::bodyPart(DwBodyPart* aDwBodyPart, KMMessagePart* aPart)
{
  if( aPart ) {
    if( aDwBodyPart ) {
      // This must not be an empty string, because we'll get a
      // spurious empty Subject: line in some of the parts.
      aPart->setName(" ");
      DwHeaders& headers = aDwBodyPart->Headers();
      // Content-type
      QCString additionalCTypeParams;
      if (headers.HasContentType())
      {
        DwMediaType& ct = headers.ContentType();
        aPart->setOriginalContentTypeStr( ct.AsString().c_str() );
        aPart->setTypeStr(ct.TypeStr().c_str());
        aPart->setSubtypeStr(ct.SubtypeStr().c_str());
        DwParameter *param = ct.FirstParameter();
        while(param)
        {
          if (!qstricmp(param->Attribute().c_str(), "charset"))
            aPart->setCharset(QCString(param->Value().c_str()).lower());
          else if (param->Attribute().c_str()=="name*")
            aPart->setName(KMMsgBase::decodeRFC2231String(
              param->Value().c_str()));
          else {
            additionalCTypeParams += ";";
            additionalCTypeParams += param->AsString().c_str();
          }
          param=param->Next();
        }
      }
      else
      {
        aPart->setTypeStr("text");      // Set to defaults
        aPart->setSubtypeStr("plain");
      }
      aPart->setAdditionalCTypeParamStr( additionalCTypeParams );
      // Modification by Markus
      if (aPart->name().isEmpty())
      {
	if (!headers.ContentType().Name().empty()) {
	  aPart->setName(KMMsgBase::decodeRFC2047String(headers.
							ContentType().Name().c_str()) );
	} else if (!headers.Subject().AsString().empty()) {
	  aPart->setName( KMMsgBase::decodeRFC2047String(headers.
							 Subject().AsString().c_str()) );
	}
      }

      // Content-transfer-encoding
      if (headers.HasContentTransferEncoding())
        aPart->setCteStr(headers.ContentTransferEncoding().AsString().c_str());
      else
        aPart->setCteStr("7bit");

      // Content-description
      if (headers.HasContentDescription())
        aPart->setContentDescription(headers.ContentDescription().AsString().c_str());
      else
        aPart->setContentDescription("");

      // Content-disposition
      if (headers.HasContentDisposition())
        aPart->setContentDisposition(headers.ContentDisposition().AsString().c_str());
      else
        aPart->setContentDisposition("");

      // Body
      aPart->setBody( aDwBodyPart->Body().AsString().c_str() );
    }
    // If no valid body part was not given,
    // set all MultipartBodyPart attributes to empty values.
    else
    {
      aPart->setTypeStr("");
      aPart->setSubtypeStr("");
      aPart->setCteStr("");
      // This must not be an empty string, because we'll get a
      // spurious empty Subject: line in some of the parts.
      aPart->setName(" ");
      aPart->setContentDescription("");
      aPart->setContentDisposition("");
      aPart->setBody("");
    }
  }
}


//-----------------------------------------------------------------------------
void KMMessage::bodyPart(int aIdx, KMMessagePart* aPart) const
{
  if( aPart ) {
    // If the DwBodyPart was found get the header fields and body
    DwBodyPart *part = dwBodyPart( aIdx );
    if( part )
    {
      KMMessage::bodyPart(part, aPart);
      if( aPart->name().isEmpty() )
        aPart->setName( i18n("Attachment: ") + QString( "%1" ).arg( aIdx ) );
    }
  }
}


//-----------------------------------------------------------------------------
void KMMessage::deleteBodyParts(void)
{
  mMsg->Body().DeleteBodyParts();
}


//-----------------------------------------------------------------------------
DwBodyPart* KMMessage::createDWBodyPart(const KMMessagePart* aPart)
{
  DwBodyPart* part = DwBodyPart::NewBodyPart(emptyString, 0);

  if( aPart ) {
    QCString charset  = aPart->charset();
    QCString type     = aPart->typeStr();
    QCString subtype  = aPart->subtypeStr();
    QCString cte      = aPart->cteStr();
    QCString contDesc = aPart->contentDescriptionEncoded();
    QCString contDisp = aPart->contentDisposition();
    QCString encoding = autoDetectCharset(charset, sPrefCharsets, aPart->name());
    if (encoding.isEmpty()) encoding = "utf-8";
    QCString name     = KMMsgBase::encodeRFC2231String(aPart->name(), encoding);
    bool RFC2231encoded = aPart->name() != QString(name);
    QCString paramAttr  = aPart->parameterAttribute();

    DwHeaders& headers = part->Headers();

    DwMediaType& ct = headers.ContentType();
    if (type != "" && subtype != "")
    {
      ct.SetTypeStr(type.data());
      ct.SetSubtypeStr(subtype.data());
      if (!charset.isEmpty()){
	DwParameter *param;
	param=new DwParameter;
	param->SetAttribute("charset");
	param->SetValue(charset.data());
	ct.AddParameter(param);
      }
    }

    QCString additionalParam = aPart->additionalCTypeParamStr();
    if( !additionalParam.isEmpty() )
    {
      QCString parAV;
      DwString parA, parV;
      int iL, i1, i2, iM;
      iL = additionalParam.length();
      i1 = 0;
      i2 = additionalParam.find(';', i1, false);
      while ( i1 < iL )
      {
        if( -1 == i2 )
          i2 = iL;
        if( i1+1 < i2 ) {
          parAV = additionalParam.mid( i1, (i2-i1) );
          iM = parAV.find('=');
          if( -1 < iM )
          {
            parA = parAV.left( iM );
            parV = parAV.right( parAV.length() - iM - 1 );
            if( ('"' == parV.at(0)) && ('"' == parV.at(parV.length()-1)) )
            {
              parV.erase( 0,  1);
              parV.erase( parV.length()-1 );
            }
          }
          else
          {
            parA = parAV;
            parV = "";
          }
          DwParameter *param;
          param = new DwParameter;
          param->SetAttribute( parA );
          param->SetValue(     parV );
          ct.AddParameter( param );
        }
        i1 = i2+1;
        i2 = additionalParam.find(';', i1, false);
      }
    }

    if (RFC2231encoded)
    {
      DwParameter *nameParam;
      nameParam = new DwParameter;
      nameParam->SetAttribute("name*");
      nameParam->SetValue(name.data(),true);
      ct.AddParameter(nameParam);
    } else {
      if(!name.isEmpty())
        ct.SetName(name.data());
    }

    if (!paramAttr.isEmpty())
    {
      QCString encoding = autoDetectCharset(charset, sPrefCharsets,
        aPart->parameterValue());
      if (encoding.isEmpty()) encoding = "utf-8";
      QCString paramValue;
      paramValue = KMMsgBase::encodeRFC2231String(aPart->parameterValue(),
                                                  encoding);
      DwParameter *param = new DwParameter;
      if (aPart->parameterValue() != QString(paramValue))
      {
        param->SetAttribute((paramAttr + '*').data());
	param->SetValue(paramValue.data(),true);
      } else {
        param->SetAttribute(paramAttr.data());
	param->SetValue(paramValue.data());
      }
      ct.AddParameter(param);
    }

    if (!cte.isEmpty())
      headers.Cte().FromString(cte);

    if (!contDesc.isEmpty())
      headers.ContentDescription().FromString(contDesc);

    if (!contDisp.isEmpty())
      headers.ContentDisposition().FromString(contDisp);

    if (!aPart->body().isNull())
      part->Body().FromString(aPart->body());
    else
      part->Body().FromString("");
  }
  return part;
}


//-----------------------------------------------------------------------------
void KMMessage::addDwBodyPart(DwBodyPart * aDwPart)
{
  mMsg->Body().AddBodyPart( aDwPart );
  mNeedsAssembly = TRUE;
}


//-----------------------------------------------------------------------------
void KMMessage::addBodyPart(const KMMessagePart* aPart)
{
  DwBodyPart* part = createDWBodyPart( aPart );
  addDwBodyPart( part );
}


//-----------------------------------------------------------------------------
void KMMessage::viewSource(const QString& aCaption, QTextCodec *codec, bool fixedfont)
{
  QString str = (codec) ? codec->toUnicode(asString()) :
    kernel->networkCodec()->toUnicode(asString());

#if ALLOW_GUI
  KTextBrowser *browser = new KTextBrowser();
  browser->setTextFormat( Qt::PlainText );

  KWin::setIcons(browser->winId(), kapp->icon(), kapp->miniIcon());
  if (!aCaption.isEmpty()) browser->setCaption(aCaption);

  browser->setText(str);
  if (fixedfont)
    browser->setFont(KGlobalSettings::fixedFont());

  // Well, there is no widget to be seen here, so we have to use QCursor::pos()
  int scnum = QApplication::desktop()->screenNumber(QCursor::pos());
  browser->resize(QApplication::desktop()->screenGeometry(scnum).width()/2,
	      2*QApplication::desktop()->screenGeometry(scnum).height()/3);
  browser->show();

#else //not ALLOW_GUI
  kdDebug(5006) << "Message source: " << (aCaption.isEmpty() ? "" : (const char*)aCaption) << "\n" << str << "\n--- end of message ---" << endl;

#endif
}


//-----------------------------------------------------------------------------
QString KMMessage::generateMessageId( const QString& addr )
{
  QDateTime datetime = QDateTime::currentDateTime();
  QString msgIdStr;

  msgIdStr = "<" + datetime.toString( "yyyyMMddhhmm.sszzz" );

  QString msgIdSuffix;
  KConfigGroup general( kapp->config(), "General" );

  if( general.readBoolEntry( "useCustomMessageIdSuffix", false ) )
    msgIdSuffix = general.readEntry( "myMessageIdSuffix", "" );

  if( !msgIdSuffix.isEmpty() )
    msgIdStr += "@" + msgIdSuffix;
  else
    msgIdStr += "." + addr;

  msgIdStr += ">";

  return msgIdStr;
}


//-----------------------------------------------------------------------------
QCString KMMessage::html2source( const QCString & src )
{
  QCString result( 1 + 6*src.length() );  // maximal possible length

  QCString::ConstIterator s = src.begin();
  QCString::Iterator d = result.begin();
  while ( *s ) {
    switch ( *s ) {
    case '<': {
        *d++ = '&';
        *d++ = 'l';
        *d++ = 't';
        *d++ = ';';
        ++s;
      }
      break;
    case '\r': {
        ++s;
      }
      break;
    case '\n': {
        *d++ = '<';
        *d++ = 'b';
        *d++ = 'r';
        *d++ = ' ';
        *d++ = '/';
        *d++ = '>';
        ++s;
      }
      break;
    case '>': {
        *d++ = '&';
        *d++ = 'g';
        *d++ = 't';
        *d++ = ';';
        ++s;
      }
      break;
    case '&': {
        *d++ = '&';
        *d++ = 'a';
        *d++ = 'm';
        *d++ = 'p';
        *d++ = ';';
        ++s;
      }
      break;
    case '\"': {
        *d++ = '&';
        *d++ = 'q';
        *d++ = 'u';
        *d++ = 'o';
        *d++ = 't';
        *d++ = ';';
        ++s;
      }
      break;
    default:
        *d++ = *s++;
    }
  }
  result.truncate( d - result.begin() ); // adds trailing NUL
  return result;
}


//-----------------------------------------------------------------------------
QCString KMMessage::lf2crlf( const QCString & src )
{
  QCString result( 1 + 2*src.length() );  // maximal possible length

  QCString::ConstIterator s = src.begin();
  QCString::Iterator d = result.begin();
  while ( *s ) {
    if ( '\n' == *s )
      *d++ = '\r';
    *d++ = *s++;
  }
  result.truncate( d - result.begin() ); // adds trailing NUL
  return result;
}


//-----------------------------------------------------------------------------
QString KMMessage::stripEmailAddr(const QString& aStr)
{
  QStringList list = splitEmailAddrList(aStr);
  QString result, totalResult, partA, partB;
  int i, j, len;
  for (QStringList::Iterator it = list.begin(); it != list.end(); ++it)
  {
    char endCh = '>';

    i = (*it).find('<');
    if (i<0)
    {
      i = (*it).find('(');
      endCh = ')';
    }
    if (i<0) result = *it;
    else {
      partA = (*it).left(i).stripWhiteSpace();
      j = (*it).find(endCh,i+1);
      if (j<0) result = *it;
      else {
        partB = (*it).mid(i+1, j-i-1).stripWhiteSpace();

        if (partA.find('@') >= 0 && !partB.isEmpty()) result = partB;
        else if (!partA.isEmpty()) result = partA;
        else result = (*it);

        len = result.length();
        if (result[0]=='"' && result[len-1]=='"')
          result = result.mid(1, result.length()-2);
        else if (result[0]=='<' && result[len-1]=='>')
          result = result.mid(1, result.length()-2);
        else if (result[0]=='(' && result[len-1]==')')
          result = result.mid(1, result.length()-2);
      }
    }
    if (!totalResult.isEmpty()) totalResult += ", ";
    totalResult += result;
  }
  return totalResult;
}

//-----------------------------------------------------------------------------
QCString KMMessage::getEmailAddr(const QString& aStr)
{
  int a, i, j, len, found = 0;
  QChar c;
  // Find the '@' in the email address:
  a = aStr.find('@');
  if (a<0) return aStr.latin1();
  // Loop backwards until we find '<', '(', ' ', or beginning of string.
  for (i = a - 1; i >= 0; i--) {
    c = aStr[i];
    if (c == '<' || c == '(' || c == ' ') found = 1;
    if (found) break;
  }
  // Reset found for next loop.
  found = 0;
  // Loop forwards until we find '>', ')', ' ', or end of string.
  for (j = a + 1; j < (int)aStr.length(); j++) {
    c = aStr[j];
    if (c == '>' || c == ')' || c == ' ') found = 1;
    if (found) break;
  }
  // Calculate the length and return the result.
  len = j - (i + 1);
  return aStr.mid(i+1,len).latin1();
}


//-----------------------------------------------------------------------------
QString KMMessage::emailAddrAsAnchor(const QString& aEmail, bool stripped)
{
  QString result, addr, tmp2;
  QChar ch;
  bool insideQuote = false;

  QString email = aEmail;

  if (email.isEmpty()) return email;

  result = "<a href='mailto:";
  for (uint pos = 0; pos < email.length(); pos++)
  {
    ch = email[pos];
    if (ch == '"') insideQuote = !insideQuote;
    if (ch == '<') addr += "&lt;";
    else if (ch == '>') addr += "&gt;";
    else if (ch == '&') addr += "&amp;";
    else if (ch == '\'') addr += "&#39;";
    else if (ch != ',' || insideQuote) addr += ch;

    if (ch != ',' || insideQuote)
      tmp2 += ch;

    if ((ch == ',' && !insideQuote) || pos + 1 >= email.length())
    {
      result += addr;
      result += "'>";
      if (stripped) result += KMMessage::stripEmailAddr(tmp2);
      else result += addr;
      tmp2 = "";
      result += "</a>";
      if (ch == ',')
      {
	result += ", <a href='mailto:";
	while (email[pos+1] == ' ') pos++;
      }
      addr = "";
    }
  }
  result = result.replace(QRegExp("\n"),"");
  return result;
}


//-----------------------------------------------------------------------------
QStringList KMMessage::splitEmailAddrList(const QString& aStr)
{
  // Features:
  // - always ignores quoted characters
  // - ignores everything (including parentheses and commas)
  //   inside quoted strings
  // - supports nested comments
  // - ignores everything (including double quotes and commas)
  //   inside comments

  QStringList list;

  if (aStr.isEmpty())
    return list;

  QString addr;
  uint addrstart = 0;
  int commentlevel = 0;
  bool insidequote = false;

  for (uint index=0; index<aStr.length(); index++) {
    // the following conversion to latin1 is o.k. because
    // we can safely ignore all non-latin1 characters
    switch (aStr[index].latin1()) {
    case '"' : // start or end of quoted string
      if (commentlevel == 0)
        insidequote = !insidequote;
      break;
    case '(' : // start of comment
      if (!insidequote)
        commentlevel++;
      break;
    case ')' : // end of comment
      if (!insidequote) {
        if (commentlevel > 0)
          commentlevel--;
        else {
          kdDebug(5006) << "Error in address splitting: Unmatched ')'"
                        << endl;
          return list;
        }
      }
      break;
    case '\\' : // quoted character
      index++; // ignore the quoted character
      break;
    case ',' :
      if (!insidequote && (commentlevel == 0)) {
        addr = aStr.mid(addrstart, index-addrstart);
        if (!addr.isEmpty())
          list += addr.simplifyWhiteSpace();
        addrstart = index+1;
      }
      break;
    }
  }
  // append the last address to the list
  if (!insidequote && (commentlevel == 0)) {
    addr = aStr.mid(addrstart, aStr.length()-addrstart);
    if (!addr.isEmpty())
      list += addr.simplifyWhiteSpace();
  }
  else
    kdDebug(5006) << "Error in address splitting: "
                  << "Unexpected end of address list"
                  << endl;

  return list;
}


//-----------------------------------------------------------------------------
void KMMessage::readConfig(void)
{
  KConfig *config=kapp->config();
  KConfigGroupSaver saver(config, "General");

  config->setGroup("General");

  int languageNr = config->readNumEntry("reply-current-language",0);

  { // area for config group "KMMessage #n"
    KConfigGroupSaver saver(config, QString("KMMessage #%1").arg(languageNr));
    sReplyLanguage = config->readEntry("language",KGlobal::locale()->language());
    sReplyStr = config->readEntry("phrase-reply",
      i18n("On %D, you wrote:"));
    sReplyAllStr = config->readEntry("phrase-reply-all",
      i18n("On %D, %F wrote:"));
    sForwardStr = config->readEntry("phrase-forward",
      i18n("Forwarded Message"));
    sIndentPrefixStr = config->readEntry("indent-prefix",">%_");
  }

  { // area for config group "Composer"
    KConfigGroupSaver saver(config, "Composer");
    sReplySubjPrefixes = config->readListEntry("reply-prefixes", ',');
    if (sReplySubjPrefixes.count() == 0)
      sReplySubjPrefixes.append("Re:");
    sReplaceSubjPrefix = config->readBoolEntry("replace-reply-prefix", true);
    sForwardSubjPrefixes = config->readListEntry("forward-prefixes", ',');
    if (sForwardSubjPrefixes.count() == 0)
      sForwardSubjPrefixes.append("Fwd:");
    sReplaceForwSubjPrefix = config->readBoolEntry("replace-forward-prefix", true);

    sSmartQuote = config->readBoolEntry("smart-quote", true);
    sWrapCol = config->readNumEntry("break-at", 78);
    if ((sWrapCol == 0) || (sWrapCol > 78))
      sWrapCol = 78;
    if (sWrapCol < 30)
      sWrapCol = 30;

    sPrefCharsets = config->readListEntry("pref-charsets");
  }

  { // area for config group "Reader"
    KConfigGroupSaver saver(config, "Reader");
    sHdrStyle = config->readNumEntry("hdr-style", KMReaderWin::HdrFancy);
  }
}

QCString KMMessage::defaultCharset()
{
  QCString retval;

  if (!sPrefCharsets.isEmpty())
    retval = sPrefCharsets[0].latin1();

  if (retval.isEmpty()  || (retval == "locale"))
    retval = QCString(kernel->networkCodec()->mimeName()).lower();

  if (retval == "jisx0208.1983-0") retval = "iso-2022-jp";
  else if (retval == "ksc5601.1987-0") retval = "euc-kr";
  return retval;
}

const QStringList &KMMessage::preferredCharsets()
{
  return sPrefCharsets;
}

//-----------------------------------------------------------------------------
QCString KMMessage::charset(void) const
{
  DwMediaType &mType=mMsg->Headers().ContentType();
  mType.Parse();
  DwParameter *param=mType.FirstParameter();
  while(param){
    if (!qstricmp(param->Attribute().c_str(), "charset"))
      return param->Value().c_str();
    else param=param->Next();
  }
  return ""; // us-ascii, but we don't have to specify it
}

//-----------------------------------------------------------------------------
void KMMessage::setCharset(const QCString& bStr)
{
  QCString aStr = bStr.lower();
  if (aStr.isNull())
    aStr = "";
  DwMediaType &mType = dwContentType();
  mType.Parse();
  DwParameter *param=mType.FirstParameter();
  while(param)
    // FIXME use the mimelib functions here for comparison.
    if (!qstricmp(param->Attribute().c_str(), "charset")) break;
    else param=param->Next();
  if (!param){
    param=new DwParameter;
    param->SetAttribute("charset");
    mType.AddParameter(param);
  }
  else
    mType.SetModified();
  param->SetValue(DwString(aStr));
  mType.Assemble();
}

//-----------------------------------------------------------------------------
void KMMessage::setStatus(const KMMsgStatus aStatus, int idx)
{
  if (mStatus == aStatus)
    return;
  KMMsgBase::setStatus(aStatus, idx);
  mStatus = aStatus;
  mDirty = TRUE;
}


//-----------------------------------------------------------------------------
void KMMessage::setEncryptionState( const KMMsgEncryptionState aStatus,
                                    int idx )
{
    if( mEncryptionState == aStatus )
        return;
    KMMsgBase::setEncryptionState( aStatus, idx );
    mEncryptionState = aStatus;
    mDirty = true;
}


//-----------------------------------------------------------------------------
void KMMessage::setSignatureState( const KMMsgSignatureState aStatus,
                                   int idx )
{
    if( mSignatureState == aStatus )
        return;
    KMMsgBase::setSignatureState( aStatus, idx );
    mSignatureState = aStatus;
    mDirty = true;
}


//-----------------------------------------------------------------------------
void KMMessage::link(const KMMessage *aMsg, KMMsgStatus aStatus)
{
  Q_ASSERT(aStatus == KMMsgStatusReplied || aStatus == KMMsgStatusForwarded);

  QString message = headerField("X-KMail-Link-Message");
  if (!message.isEmpty())
    message += ",";
  QString type = headerField("X-KMail-Link-Type");
  if (!type.isEmpty())
    type += ",";

  message += QString::number(aMsg->getMsgSerNum());
  if (aStatus == KMMsgStatusReplied)
    type += "reply";
  else if (aStatus == KMMsgStatusForwarded)
    type += "forward";

  setHeaderField("X-KMail-Link-Message", message);
  setHeaderField("X-KMail-Link-Type", type);
}

//-----------------------------------------------------------------------------
void KMMessage::getLink(int n, ulong *retMsgSerNum, KMMsgStatus *retStatus) const
{
  *retMsgSerNum = 0;
  *retStatus = KMMsgStatusUnknown;

  QString message = headerField("X-KMail-Link-Message");
  QString type = headerField("X-KMail-Link-Type");
  message = message.section(',', n, n);
  type = type.section(',', n, n);

  if (!message.isEmpty() && !type.isEmpty()) {
    *retMsgSerNum = message.toULong();
    if (type == "reply")
      *retStatus = KMMsgStatusReplied;
    else if (type == "forward")
      *retStatus = KMMsgStatusForwarded;
  }
}
