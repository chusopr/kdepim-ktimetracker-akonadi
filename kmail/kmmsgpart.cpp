// kmmsgpart.cpp


#include <kmimemagic.h>
#include <kmimetype.h>
#include <kdebug.h>
#include <kmdcodec.h>

#include "kmmsgpart.h"
#include "kmmessage.h"

#include <kmime_charfreq.h>
#include <mimelib/enum.h>
#include <mimelib/utility.h>
#include <mimelib/string.h>
#include <kiconloader.h>

using namespace KMime;

//-----------------------------------------------------------------------------
KMMessagePart::KMMessagePart()
  : mType("text"), mSubtype("plain"), mCte("7bit"), mBodyDecodedSize(0)
{
}


//-----------------------------------------------------------------------------
KMMessagePart::~KMMessagePart()
{
}


//-----------------------------------------------------------------------------
int KMMessagePart::decodedSize(void) const
{
  if (mBodyDecodedSize < 0)
    mBodyDecodedSize = bodyDecodedBinary().size();
  return mBodyDecodedSize;
}


//-----------------------------------------------------------------------------
void KMMessagePart::setBody(const QCString &aStr)
{
  mBody.duplicate( aStr.data(), aStr.length() );

  int enc = cte();
  if (enc == DwMime::kCte7bit || enc == DwMime::kCte8bit || enc == DwMime::kCteBinary)
    mBodyDecodedSize = mBody.size();
  else
    mBodyDecodedSize = -1; // Can't know the decoded size
}

//-----------------------------------------------------------------------------
// Returns Base64 encoded MD5 digest of a QString
QString KMMessagePart::encodeBase64(const QString& aStr)
{
  DwString dwResult, dwSrc;
  QString result;

  if (aStr.isEmpty())
    return QString();

  // Generate digest
  KMD5 context(aStr.latin1());

  dwSrc = DwString((const char*)context.rawDigest(), 16);
  DwEncodeBase64(dwSrc, dwResult);
  result = QString( dwResult.c_str() );
  result.truncate(22);
  return result;
}


//-----------------------------------------------------------------------------
void KMMessagePart::setBodyEncoded(const QCString& aStr)
{
  mBodyDecodedSize = aStr.length();

  switch (cte())
  {
  case DwMime::kCteQuotedPrintable:
  case DwMime::kCteBase64:
    {
      DwString dwSrc(aStr.data(), mBodyDecodedSize);
      DwString dwResult;
      if (cte() == DwMime::kCteQuotedPrintable)
	DwEncodeQuotedPrintable(dwSrc, dwResult);
      else
	DwEncodeBase64(dwSrc, dwResult);
      mBody.duplicate(dwResult.data(), dwResult.size());
      break;
    }
  default:
    kdWarning(5006) << "unknown encoding '" << cteStr()
		    << "'. Assuming binary." << endl;
  case DwMime::kCte7bit:
  case DwMime::kCte8bit:
  case DwMime::kCteBinary:
    mBody.duplicate( aStr.data(), mBodyDecodedSize );
    break;
  }
}

void KMMessagePart::setBodyAndGuessCte(const QByteArray& aBuf,
				       QValueList<int> & allowedCte,
				       bool allow8Bit )
{
  allowedCte.clear();

  mBodyDecodedSize = aBuf.size();

  CharFreq cf( aBuf ); // save to pass NULL arrays...

  switch ( cf.type() ) {
  case CharFreq::SevenBitText:
    allowedCte << DwMime::kCte7bit;
    if ( allow8Bit )
      allowedCte << DwMime::kCte8bit;
  case CharFreq::SevenBitData:
    allowedCte << DwMime::kCteQp;
    allowedCte << DwMime::kCteBase64;
    break;
  case CharFreq::EightBitText:
    if ( allow8Bit ) 
      allowedCte << DwMime::kCte8bit;
    allowedCte << DwMime::kCteQp;
  case CharFreq::EightBitData:
    allowedCte << DwMime::kCteBase64;
    break;
  }

  kdDebug() << "CharFreq returned " << cf.type() << " and I chose "
	    << allowedCte[0] << endl;
  setCte( allowedCte[0] ); // choose best fitting
  setBodyEncodedBinary( aBuf );
}

void KMMessagePart::setBodyAndGuessCte(const QCString& aBuf,
				       QValueList<int> & allowedCte,
				       bool allow8Bit )
{
  allowedCte.clear();

  mBodyDecodedSize = aBuf.length();

  CharFreq cf( aBuf.data(), mBodyDecodedSize ); // save to pass NULL strings

  switch ( cf.type() ) {
  case CharFreq::SevenBitText:
    allowedCte << DwMime::kCte7bit;
    if ( allow8Bit )
      allowedCte << DwMime::kCte8bit;
  case CharFreq::SevenBitData:
    allowedCte << DwMime::kCteQp;
    allowedCte << DwMime::kCteBase64;
    break;
  case CharFreq::EightBitText:
    if ( allow8Bit ) 
      allowedCte << DwMime::kCte8bit;
    allowedCte << DwMime::kCteQp;
  case CharFreq::EightBitData:
    allowedCte << DwMime::kCteBase64;
    break;
  }

  setCte( allowedCte[0] ); // choose best fitting
  setBodyEncoded( aBuf );
}

//-----------------------------------------------------------------------------
void KMMessagePart::setBodyEncodedBinary(const QByteArray& aStr)
{
  mBodyDecodedSize = aStr.size();
  if (aStr.isEmpty())
  {
    mBody.resize(0);
    return;
  }

  switch (cte())
  {
  case DwMime::kCteQuotedPrintable:
  case DwMime::kCteBase64:
    {
      DwString dwSrc(aStr.data(), aStr.size());
      DwString dwResult;
      if (cte() == DwMime::kCteQuotedPrintable)
	DwEncodeQuotedPrintable(dwSrc, dwResult);
      else
	DwEncodeBase64(dwSrc, dwResult);
      mBody.duplicate(dwResult.data(), dwResult.size());
      break;
    }
  default:
    kdWarning(5006) << "unknown encoding '" << cteStr()
		    << "'. Assuming binary." << endl;
  case DwMime::kCte7bit:
  case DwMime::kCte8bit:
  case DwMime::kCteBinary:
    mBody.duplicate( aStr );
    break;
  }
}


//-----------------------------------------------------------------------------
QByteArray KMMessagePart::bodyDecodedBinary(void) const
{
  if (mBody.isEmpty()) return QByteArray();
  QByteArray result;

  switch (cte())
  {
  case DwMime::kCteQuotedPrintable:
  case DwMime::kCteBase64:
    {
      DwString dwSrc(mBody.data(), mBody.size());
      DwString dwResult;
      if (cte() == DwMime::kCteQuotedPrintable)
	DwDecodeQuotedPrintable(dwSrc, dwResult);
      else
	DwDecodeBase64(dwSrc, dwResult);
      result.duplicate( dwResult.data(), dwResult.size() );
      break;
    }
  default:
    kdWarning(5006) << "unknown encoding '" << cteStr()
		    << "'. Assuming binary." << endl;
  case DwMime::kCte7bit:
  case DwMime::kCte8bit:
  case DwMime::kCteBinary:
    result.duplicate(mBody);
    break;
  }

  assert( mBodyDecodedSize < 0
	  || (unsigned int)mBodyDecodedSize == result.size() );
  if ( mBodyDecodedSize < 0 )
    mBodyDecodedSize = result.size(); // cache the decoded size.

  return result;
}

QCString KMMessagePart::bodyDecoded(void) const
{
  if (mBody.isEmpty()) return QCString("");
  QCString result;
  int len;

  switch (cte())
  {
  case DwMime::kCteQuotedPrintable:
  case DwMime::kCteBase64:
    {
      DwString dwSrc(mBody.data(), mBody.size());
      DwString dwResult;
      if (cte() == DwMime::kCteQuotedPrintable)
	DwDecodeQuotedPrintable(dwSrc, dwResult);
      else
	DwDecodeBase64(dwSrc, dwResult);
      len = dwResult.size();
      result.resize( len+1 /* trailing NUL */ );
      memcpy(result.data(), dwResult.data(), len);
      break;
    }
  default:
    kdWarning(5006) << "unknown encoding '" << cteStr()
		    << "'. Assuming binary." << endl;
  case DwMime::kCte7bit:
  case DwMime::kCte8bit:
  case DwMime::kCteBinary:
    {
      len = mBody.size();
      result.resize( len+1 /* trailing NUL */ );
      memcpy(result.data(), mBody.data(), len);
      break;
    }
  }
  result[len] = 0;

  kdWarning( result.length() != (unsigned int)len, 5006 )
    << "KMMessagePart::bodyDecoded(): body is binary but used as text!" << endl;

  assert( mBodyDecodedSize < 0 || mBodyDecodedSize == len );
  if ( mBodyDecodedSize < 0 )
    mBodyDecodedSize = len; // cache decoded size

  return result;
}


//-----------------------------------------------------------------------------
void KMMessagePart::magicSetType(bool aAutoDecode)
{
  QString mimetype;
  QByteArray body;
  KMimeMagicResult *result;

  int sep;

  KMimeMagic::self()->setFollowLinks(TRUE); // is it necessary ?

  if (aAutoDecode)
    body = bodyDecodedBinary();
  else
    body = mBody;

  result = KMimeMagic::self()->findBufferType( body );
  mimetype = result->mimeType();
  sep = mimetype.find('/');
  mType = mimetype.left(sep).latin1();
  mSubtype = mimetype.mid(sep+1).latin1();
}


//-----------------------------------------------------------------------------
QString KMMessagePart::iconName(const QString& mimeType) const
{
  QString fileName = KMimeType::mimeType(mimeType.isEmpty() ?
    (mType + "/" + mSubtype).lower() : mimeType.lower())->icon(QString(),FALSE);
  fileName = KGlobal::instance()->iconLoader()->iconPath( fileName,
    KIcon::Desktop );
  return fileName;
}


//-----------------------------------------------------------------------------
QCString KMMessagePart::typeStr(void) const
{
  return mType;
}


//-----------------------------------------------------------------------------
int KMMessagePart::type(void) const
{
  int type = DwTypeStrToEnum(DwString(mType));
  return type;
}


//-----------------------------------------------------------------------------
void KMMessagePart::setTypeStr(const QCString &aStr)
{
    mType = aStr;
}


//-----------------------------------------------------------------------------
void KMMessagePart::setType(int aType)
{
  DwString dwType;
  DwTypeEnumToStr(aType, dwType);
  mType = dwType.c_str();

}

//-----------------------------------------------------------------------------
QCString KMMessagePart::subtypeStr(void) const
{
  return mSubtype;
}


//-----------------------------------------------------------------------------
int KMMessagePart::subtype(void) const
{
  int subtype = DwSubtypeStrToEnum(DwString(mSubtype));
  return subtype;
}


//-----------------------------------------------------------------------------
void KMMessagePart::setSubtypeStr(const QCString &aStr)
{
  mSubtype = aStr;
}


//-----------------------------------------------------------------------------
void KMMessagePart::setSubtype(int aSubtype)
{
  DwString dwSubtype;
  DwSubtypeEnumToStr(aSubtype, dwSubtype);
  mSubtype = dwSubtype.c_str();

}

//-----------------------------------------------------------------------------
QCString KMMessagePart::parameterAttribute(void) const
{
  return mParameterAttribute;
}

//-----------------------------------------------------------------------------
QString KMMessagePart::parameterValue(void) const
{
  return mParameterValue;
}

//-----------------------------------------------------------------------------
void KMMessagePart::setParameter(const QCString &attribute,
                                 const QString &value)
{
  mParameterAttribute = attribute;
  mParameterValue = value;
}

//-----------------------------------------------------------------------------
QCString KMMessagePart::contentTransferEncodingStr(void) const
{
  return mCte;
}


//-----------------------------------------------------------------------------
int KMMessagePart::contentTransferEncoding(void) const
{
  int cte = DwCteStrToEnum(DwString(mCte));
  return cte;
}


//-----------------------------------------------------------------------------
void KMMessagePart::setContentTransferEncodingStr(const QCString &aStr)
{
    mCte = aStr;
}


//-----------------------------------------------------------------------------
void KMMessagePart::setContentTransferEncoding(int aCte)
{
  DwString dwCte;
  DwCteEnumToStr(aCte, dwCte);
  mCte = dwCte.c_str();

}


//-----------------------------------------------------------------------------
QString KMMessagePart::contentDescription(void) const
{
  return KMMsgBase::decodeRFC2047String(mContentDescription);
}


//-----------------------------------------------------------------------------
void KMMessagePart::setContentDescription(const QString &aStr)
{
  QCString encoding = KMMessage::autoDetectCharset(charset(),
    KMMessage::preferredCharsets(), aStr);
  if (encoding.isEmpty()) encoding = "utf-8";
  mContentDescription = KMMsgBase::encodeRFC2047String(aStr, encoding);
}


//-----------------------------------------------------------------------------
QString KMMessagePart::fileName(void) const
{
  int i, j, len;
  QCString str;
  int RFC2231encoded = 0;

  i = mContentDisposition.find("filename*=", 0, FALSE);
  if (i >= 0) { RFC2231encoded = 1; }
  else {
    i = mContentDisposition.find("filename=", 0, FALSE);
    if (i < 0) return QString::null;
  }
  j = mContentDisposition.find(';', i+9);

  if (j < 0) j = 32767;
  str = mContentDisposition.mid(i+9+RFC2231encoded, j-i-9-RFC2231encoded).
    stripWhiteSpace();

  len = str.length();
  if (len>1) {
    if (str[0]=='"' && str[len-1]=='"')
      str = str.mid(1, len-2);
  };

  if (RFC2231encoded)
    return KMMsgBase::decodeRFC2231String(str);
  else
    return KMMsgBase::decodeRFC2047String(str);
}


//-----------------------------------------------------------------------------
QCString KMMessagePart::contentDisposition(void) const
{
  return mContentDisposition;
}


//-----------------------------------------------------------------------------
void KMMessagePart::setContentDisposition(const QCString &aStr)
{
  mContentDisposition = aStr;
}


//-----------------------------------------------------------------------------
QCString KMMessagePart::body(void) const
{
  int len = mBody.size();
  QCString result(len+1); // space for trailing NUL
  memcpy(result.data(), mBody.data(), len);
  result[len] = 0;
  return result;
}


//-----------------------------------------------------------------------------
QString KMMessagePart::name(void) const
{
  return mName;
}


//-----------------------------------------------------------------------------
void KMMessagePart::setName(const QString &aStr)
{
  mName = aStr;
}


//-----------------------------------------------------------------------------
QCString KMMessagePart::charset(void) const
{
   return mCharset;
}

//-----------------------------------------------------------------------------
void KMMessagePart::setCharset(const QCString &aStr)
{
  mCharset=aStr;
}



