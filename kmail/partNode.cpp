/* -*- c++ -*-
    partNode.cpp A node in a MIME tree.

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2002 Klar�lvdalens Datakonsult AB

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KMail is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config.h>

#include "partNode.h"
#include "kmreaderwin.h"

#include <klocale.h>
#include <kdebug.h>
#include "kmmimeparttree.h"
#include <mimelib/utility.h>
#include <qregexp.h>
#include <kasciistricmp.h>
#include "util.h"

/*
  ===========================================================================


  S T A R T    O F     T E M P O R A R Y     M I M E     C O D E


  ===========================================================================
  N O T E :   The partNode structure will most likely be replaced by KMime.
  It's purpose: Speed optimization for KDE 3.   (khz, 28.11.01)
  ===========================================================================
*/

partNode::partNode()
  : mRoot( 0 ), mNext( 0 ), mChild( 0 ),
    mWasProcessed( false ),
    mDwPart( 0 ),
    mType( DwMime::kTypeUnknown ),
    mSubType( DwMime::kSubtypeUnknown ),
    mEncryptionState( KMMsgNotEncrypted ),
    mSignatureState( KMMsgNotSigned ),
    mMsgPartOk( false ),
    mEncodedOk( false ),
    mDeleteDwBodyPart( false ),
    mMimePartTreeItem( 0 ),
    mBodyPartMementoMap(),
    mReader( 0 ),
    mDisplayedEmbedded( false )
{
  adjustDefaultType( this );
}

partNode::partNode( KMReaderWin * win, DwBodyPart* dwPart, int explicitType, int explicitSubType,
		    bool deleteDwBodyPart )
  : mRoot( 0 ), mNext( 0 ), mChild( 0 ),
    mWasProcessed( false ),
    mDwPart( dwPart ),
    mEncryptionState( KMMsgNotEncrypted ),
    mSignatureState( KMMsgNotSigned ),
    mMsgPartOk( false ),
    mEncodedOk( false ),
    mDeleteDwBodyPart( deleteDwBodyPart ),
    mMimePartTreeItem( 0 ),
    mBodyPartMementoMap(),
    mReader( win ),
    mDisplayedEmbedded( false ),
    mDisplayedHidden( false )
{
  if ( explicitType != DwMime::kTypeUnknown ) {
    mType    = explicitType;     // this happens e.g. for the Root Node
    mSubType = explicitSubType;  // representing the _whole_ message
  } else {
    if(dwPart && dwPart->hasHeaders() && dwPart->Headers().HasContentType()) {
      mType    = (!dwPart->Headers().ContentType().Type())?DwMime::kTypeUnknown:dwPart->Headers().ContentType().Type();
      mSubType = dwPart->Headers().ContentType().Subtype();
    } else {
      mType    = DwMime::kTypeUnknown;
      mSubType = DwMime::kSubtypeUnknown;
    }
  }
}

partNode * partNode::fromMessage( const KMMessage * msg, KMReaderWin * win ) {
  if ( !msg )
    return 0;

  int mainType    = msg->type();
  int mainSubType = msg->subtype();
  if(    (DwMime::kTypeNull    == mainType)
      || (DwMime::kTypeUnknown == mainType) ){
    mainType    = DwMime::kTypeText;
    mainSubType = DwMime::kSubtypePlain;
  }

  // we don't want to treat the top-level part special. mimelib does
  // (Message vs. BodyPart, with common base class Entity). But we
  // used DwBodyPart, not DwEntiy everywhere. *shrug*. DwStrings are
  // subscrib-shared, so we just force mimelib to parse the whole mail
  // as just another DwBodyPart...
  DwBodyPart * mainBody = new DwBodyPart( *msg->getTopLevelPart() );

  partNode * root = new partNode( win, mainBody, mainType, mainSubType, true );
  root->buildObjectTree();

  root->setFromAddress( msg->from() );
  //root->dump();
  return root;
}

partNode::partNode( bool deleteDwBodyPart, DwBodyPart* dwPart )
  : mRoot( 0 ), mNext( 0 ), mChild( 0 ),
    mWasProcessed( false ),
    mDwPart( dwPart ),
    mEncryptionState( KMMsgNotEncrypted ),
    mSignatureState( KMMsgNotSigned ),
    mMsgPartOk( false ),
    mEncodedOk( false ),
    mDeleteDwBodyPart( deleteDwBodyPart ),
    mMimePartTreeItem( 0 ),
    mBodyPartMementoMap(),
    mReader( 0 ),
    mDisplayedEmbedded( false )
{
  if ( dwPart && dwPart->hasHeaders() && dwPart->Headers().HasContentType() ) {
    mType    = (!dwPart->Headers().ContentType().Type())?DwMime::kTypeUnknown:dwPart->Headers().ContentType().Type();
    mSubType = dwPart->Headers().ContentType().Subtype();
  } else {
    mType    = DwMime::kTypeUnknown;
    mSubType = DwMime::kSubtypeUnknown;
  }
}

partNode::~partNode() {
  if( mDeleteDwBodyPart )
    delete mDwPart;
  mDwPart = 0;
  delete mChild; mChild = 0;
  delete mNext; mNext = 0;
  for ( std::map<QCString,KMail::Interface::BodyPartMemento*>::const_iterator it = mBodyPartMementoMap.begin(), end = mBodyPartMementoMap.end() ; it != end ; ++it )
      delete it->second;
  mBodyPartMementoMap.clear();
}

#ifndef NDEBUG
void partNode::dump( int chars ) const {
  kdDebug(5006) << nodeId() << " " << QString().fill( ' ', chars ) << "+ "
                << typeString() << '/' << subTypeString() << " embedded:" << mDisplayedEmbedded
                << " address:" << this << endl;
  if ( mChild )
    mChild->dump( chars + 1 );
  if ( mNext )
    mNext->dump( chars );
}
#else
void partNode::dump( int ) const {}
#endif

const QCString & partNode::encodedBody() {
  if ( mEncodedOk )
    return mEncodedBody;

  if ( mDwPart )
    mEncodedBody = KMail::Util::CString( mDwPart->Body().AsString() );
  else
    mEncodedBody = 0;
  mEncodedOk = true;
  return mEncodedBody;
}


void partNode::buildObjectTree( bool processSiblings )
{
    partNode* curNode = this;
    while( curNode && curNode->dwPart() ) {
        //dive into multipart messages
        while( DwMime::kTypeMultipart == curNode->type() ) {
            partNode * newNode = new partNode( mReader, curNode->dwPart()->Body().FirstBodyPart() );
            curNode->setFirstChild( newNode );
            curNode = newNode;
        }
        // go up in the tree until reaching a node with next
        // (or the last top-level node)
        while(     curNode
               && !(    curNode->dwPart()
                     && curNode->dwPart()->Next() ) ) {
            curNode = curNode->mRoot;
        }
        // we might have to leave when all children have been processed
        if( this == curNode && !processSiblings )
            return;
        // store next node
        if( curNode && curNode->dwPart() && curNode->dwPart()->Next() ) {
            partNode* nextNode = new partNode( mReader, curNode->dwPart()->Next() );
            curNode->setNext( nextNode );
            curNode = nextNode;
        } else
            curNode = 0;
    }
}

QCString partNode::typeString() const {
  DwString s;
  DwTypeEnumToStr( type(), s );
  return s.c_str();
}

QCString partNode::subTypeString() const {
  DwString s;
  DwSubtypeEnumToStr( subType(), s );
  return s.c_str();
}

const partNode* partNode::topLevelParent() const {
  const partNode *ret = this;
  while ( ret->parentNode() )
    ret = ret->parentNode();
  return ret;
}

int partNode::childCount() const {
  int count = 0;
  for ( partNode * child = firstChild() ; child ; child = child->nextSibling() )
    ++ count;
  return count;
}

int partNode::totalChildCount() const {
  int count = 0;
  for ( partNode * child = firstChild() ; child ; child = child->nextSibling() ) {
    ++count;
    count += child->totalChildCount();
  }
  return count;
}

QString partNode::contentTypeParameter( const char * name ) const {
  if ( !mDwPart || !mDwPart->hasHeaders() )
    return QString::null;
  DwHeaders & headers = mDwPart->Headers();
  if ( !headers.HasContentType() )
    return QString::null;
  DwString attr = name;
  attr.ConvertToLowerCase();
  for ( DwParameter * param = headers.ContentType().FirstParameter() ; param ; param = param->Next() ) {
    DwString this_attr = param->Attribute();
    this_attr.ConvertToLowerCase(); // what a braindead design!
    if ( this_attr == attr )
      return QString::fromLatin1( param->Value().data(), param->Value().size() );
    // warning: misses rfc2231 handling!
  }
  return QString::null;
}

KMMsgEncryptionState partNode::overallEncryptionState() const
{
    KMMsgEncryptionState myState = KMMsgEncryptionStateUnknown;
    if( mEncryptionState == KMMsgNotEncrypted ) {
        // NOTE: children are tested ONLY when parent is not encrypted
        if( mChild )
            myState = mChild->overallEncryptionState();
        else
            myState = KMMsgNotEncrypted;
    }
    else { // part is partially or fully encrypted
        myState = mEncryptionState;
    }
    // siblings are tested always
    if( mNext ) {
        KMMsgEncryptionState otherState = mNext->overallEncryptionState();
        switch( otherState ) {
        case KMMsgEncryptionStateUnknown:
            break;
        case KMMsgNotEncrypted:
            if( myState == KMMsgFullyEncrypted )
                myState = KMMsgPartiallyEncrypted;
            else if( myState != KMMsgPartiallyEncrypted )
                myState = KMMsgNotEncrypted;
            break;
        case KMMsgPartiallyEncrypted:
            myState = KMMsgPartiallyEncrypted;
            break;
        case KMMsgFullyEncrypted:
            if( myState != KMMsgFullyEncrypted )
                myState = KMMsgPartiallyEncrypted;
            break;
        case KMMsgEncryptionProblematic:
            break;
        }
    }

    return myState;
}


KMMsgSignatureState  partNode::overallSignatureState() const
{
    KMMsgSignatureState myState = KMMsgSignatureStateUnknown;
    if( mSignatureState == KMMsgNotSigned ) {
        // children are tested ONLY when parent is not signed
        if( mChild )
            myState = mChild->overallSignatureState();
        else
            myState = KMMsgNotSigned;
    }
    else { // part is partially or fully signed
        myState = mSignatureState;
    }
    // siblings are tested always
    if( mNext ) {
        KMMsgSignatureState otherState = mNext->overallSignatureState();
        switch( otherState ) {
        case KMMsgSignatureStateUnknown:
            break;
        case KMMsgNotSigned:
            if( myState == KMMsgFullySigned )
                myState = KMMsgPartiallySigned;
            else if( myState != KMMsgPartiallySigned )
                myState = KMMsgNotSigned;
            break;
        case KMMsgPartiallySigned:
            myState = KMMsgPartiallySigned;
            break;
        case KMMsgFullySigned:
            if( myState != KMMsgFullySigned )
                myState = KMMsgPartiallySigned;
            break;
        case KMMsgEncryptionProblematic:
            break;
        }
    }

    return myState;
}

QCString partNode::path() const
{
    if ( !parentNode() )
        return ':';
    const partNode * p = parentNode();

    // count number of siblings with the same type as us:
    int nth = 0;
    for ( const partNode * c = p->firstChild() ; c != this ; c = c->nextSibling() )
        if ( c->type() == type() && c->subType() == subType() )
            ++nth;

    return p->path() + QCString().sprintf( ":%X/%X[%X]", type(), subType(), nth );
}


int partNode::nodeId() const
{
    int curId = 0;
    partNode* rootNode = const_cast<partNode*>( this );
    while( rootNode->mRoot )
        rootNode = rootNode->mRoot;
    return rootNode->calcNodeIdOrFindNode( curId, this, 0, 0 );
}


partNode* partNode::findId( int id )
{
    int curId = 0;
    partNode* rootNode = this;
    while( rootNode->mRoot )
        rootNode = rootNode->mRoot;
    partNode* foundNode;
    rootNode->calcNodeIdOrFindNode( curId, 0, id, &foundNode );
    return foundNode;
}


int partNode::calcNodeIdOrFindNode( int &curId, const partNode* findNode, int findId, partNode** foundNode )
{
    // We use the same algorithm to determine the id of a node and
    //                           to find the node when id is known.
    curId++;
    // check for node ?
    if( findNode && this == findNode )
        return curId;
    // check for id ?
    if(  foundNode && curId == findId ) {
        *foundNode = this;
        return curId;
    }
    if( mChild )
    {
        int res = mChild->calcNodeIdOrFindNode( curId, findNode, findId, foundNode );
        if (res != -1) return res;
    }
    if( mNext )
        return mNext->calcNodeIdOrFindNode( curId, findNode, findId, foundNode );

    if(  foundNode )
        *foundNode = 0;
    return -1;
}


partNode* partNode::findType( int type, int subType, bool deep, bool wide )
{
    if(    (mType != DwMime::kTypeUnknown)
           && (    (type == DwMime::kTypeUnknown)
                   || (type == mType) )
           && (    (subType == DwMime::kSubtypeUnknown)
                   || (subType == mSubType) ) )
        return this;
    if ( mChild && deep )
        return mChild->findType( type, subType, deep, wide );
    if ( mNext && wide )
        return mNext->findType(  type, subType, deep, wide );
    return 0;
}

partNode* partNode::findNodeForDwPart( DwBodyPart* part )
{
    partNode* found = 0;
    if( kasciistricmp( dwPart()->partId(), part->partId() ) == 0 )
        return this;
    if( mChild )
        found = mChild->findNodeForDwPart( part );
    if( mNext && !found )
        found = mNext->findNodeForDwPart( part );
    return found;
}

partNode* partNode::findTypeNot( int type, int subType, bool deep, bool wide )
{
    if(    (mType != DwMime::kTypeUnknown)
           && (    (type == DwMime::kTypeUnknown)
                   || (type != mType) )
           && (    (subType == DwMime::kSubtypeUnknown)
                   || (subType != mSubType) ) )
        return this;
    if ( mChild && deep )
        return mChild->findTypeNot( type, subType, deep, wide );
    if ( mNext && wide )
        return mNext->findTypeNot(  type, subType, deep, wide );
    return 0;
}

void partNode::fillMimePartTree( KMMimePartTreeItem* parentItem,
                                 KMMimePartTree*     mimePartTree,
                                 QString labelDescr,
                                 QString labelCntType,
                                 QString labelEncoding,
                                 KIO::filesize_t size,
                                 bool revertOrder )
{
  if( parentItem || mimePartTree ) {

    if( mNext )
        mNext->fillMimePartTree( parentItem, mimePartTree,
                                 QString::null, QString::null, QString::null, 0,
                                 revertOrder );

    QString cntDesc, cntType, cntEnc;
    KIO::filesize_t cntSize = 0;

    if( labelDescr.isEmpty() ) {
        DwHeaders* headers = 0;
        if( mDwPart && mDwPart->hasHeaders() )
          headers = &mDwPart->Headers();
        if( headers && headers->HasSubject() )
            cntDesc = KMMsgBase::decodeRFC2047String( headers->Subject().AsString().c_str() );
        if( headers && headers->HasContentType()) {
            cntType = headers->ContentType().TypeStr().c_str();
            cntType += '/';
            cntType += headers->ContentType().SubtypeStr().c_str();
        }
        else
            cntType = "text/plain";
        if( cntDesc.isEmpty() )
            cntDesc = msgPart().contentDescription();
        if( cntDesc.isEmpty() )
            cntDesc = msgPart().name().stripWhiteSpace();
        if( cntDesc.isEmpty() )
            cntDesc = msgPart().fileName();
        if( cntDesc.isEmpty() ) {
            if( mRoot && mRoot->mRoot )
                cntDesc = i18n("internal part");
            else
                cntDesc = i18n("body part");
        }
        cntEnc = msgPart().contentTransferEncodingStr();
        if( mDwPart )
            cntSize = mDwPart->BodySize();
    } else {
        cntDesc = labelDescr;
        cntType = labelCntType;
        cntEnc  = labelEncoding;
        cntSize = size;
    }
    // remove linebreak+whitespace from folded Content-Description
    cntDesc.replace( QRegExp("\\n\\s*"), " " );

    if( parentItem )
      mMimePartTreeItem = new KMMimePartTreeItem( parentItem,
                                                  this,
                                                  cntDesc,
                                                  cntType,
                                                  cntEnc,
                                                  cntSize,
                                                  revertOrder );
    else if( mimePartTree )
      mMimePartTreeItem = new KMMimePartTreeItem( mimePartTree,
                                                  this,
                                                  cntDesc,
                                                  cntType,
                                                  cntEnc,
                                                  cntSize );
    mMimePartTreeItem->setOpen( true );
    if( mChild )
        mChild->fillMimePartTree( mMimePartTreeItem, 0,
                                  QString::null, QString::null, QString::null, 0,
                                  revertOrder );

  }
}

void partNode::adjustDefaultType( partNode* node )
{
    // Only bodies of  'Multipart/Digest'  objects have
    // default type 'Message/RfC822'.  All other bodies
    // have default type 'Text/Plain'  (khz, 5.12.2001)
    if( node && DwMime::kTypeUnknown == node->type() ) {
        if(    node->mRoot
               && DwMime::kTypeMultipart == node->mRoot->type()
               && DwMime::kSubtypeDigest == node->mRoot->subType() ) {
            node->setType(    DwMime::kTypeMessage   );
            node->setSubType( DwMime::kSubtypeRfc822 );
        }
        else
            {
                node->setType(    DwMime::kTypeText     );
                node->setSubType( DwMime::kSubtypePlain );
            }
    }
}

bool partNode::isAttachment() const
{
  if( !dwPart() )
    return false;
  if ( !dwPart()->hasHeaders() )
    return false;
  DwHeaders& headers = dwPart()->Headers();
  if ( headers.HasContentType() &&
       headers.ContentType().Type() == DwMime::kTypeMessage &&
       headers.ContentType().Subtype() == DwMime::kSubtypeRfc822 ) {
    // Messages are always attachments. Normally message attachments created from KMail have a content
    // disposition, but some mail clients omit that.
    return true;
  }
  if( !headers.HasContentDisposition() )
    return false;
  return ( headers.ContentDisposition().DispositionType()
	   == DwMime::kDispTypeAttachment );
}

bool partNode::isHeuristicalAttachment() const {
  if ( isAttachment() )
    return true;
  const KMMessagePart & p = msgPart();
  return !p.fileName().isEmpty() || !p.name().isEmpty() ;
}

partNode * partNode::next( bool allowChildren ) const {
  if ( allowChildren )
    if ( partNode * c = firstChild() )
      return c;
  if ( partNode * s = nextSibling() )
    return s;
  for ( partNode * p = parentNode() ; p ; p = p->parentNode() )
    if ( partNode * s = p->nextSibling() )
      return s;
  return 0;
}

bool partNode::isFirstTextPart() const {
  if ( type() != DwMime::kTypeText )
    return false;
  const partNode * root = this;
  // go up until we reach the root node of a message (of the actual message or
  // of an attached message)
  while ( const partNode * p = root->parentNode() ) {
    if ( p->type() == DwMime::kTypeMessage )
      break;
    else
      root = p;
  }
  for ( const partNode * n = root ; n ; n = n->next() )
    if ( n->type() == DwMime::kTypeText )
      return n == this;
  kdFatal() << "partNode::isFirstTextPart(): Didn't expect to end up here..." << endl;
  return false; // make comiler happy
}

bool partNode::isToltecMessage() const
{
  if ( type() != DwMime::kTypeMultipart || subType() != DwMime::kSubtypeMixed )
    return false;

  if ( childCount() != 3 )
    return false;

  const DwField* library = dwPart()->Headers().FindField( "X-Library" );
  if ( !library )
    return false;

  if ( !library->FieldBody() ||
       QString( library->FieldBody()->AsString().c_str() ) != QString( "Toltec" ) )
    return false;

  const DwField* kolabType = dwPart()->Headers().FindField( "X-Kolab-Type" );
  if ( !kolabType )
    return false;

  if ( !kolabType->FieldBody() ||
       !QString( kolabType->FieldBody()->AsString().c_str() ).startsWith( "application/x-vnd.kolab" ) )
    return false;

  return true;
}

bool partNode::isInEncapsulatedMessage() const
{
  const partNode * const topLevel = topLevelParent();
  const partNode *cur = this;
  while ( cur && cur != topLevel ) {
    const bool parentIsMessage = cur->parentNode() &&
                                 cur->parentNode()->msgPart().typeStr().lower() == "message";
    if ( parentIsMessage && cur->parentNode() != topLevel )
      return true;
    cur = cur->parentNode();
  }
  return false;
}

bool partNode::hasContentDispositionInline() const
{
  if( !dwPart() )
    return false;
  DwHeaders& headers = dwPart()->Headers();
  if( headers.HasContentDisposition() )
    return ( headers.ContentDisposition().DispositionType()
             == DwMime::kDispTypeInline );
  else
    return false;
}

const QString& partNode::trueFromAddress() const
{
  const partNode* node = this;
  while( node->mFromAddress.isEmpty() && node->mRoot )
    node = node->mRoot;
  return node->mFromAddress;
}

KMail::Interface::BodyPartMemento * partNode::bodyPartMemento( const QCString & which ) const
{
    if ( const KMReaderWin * r = reader() )
        return r->bodyPartMemento( this, which );
    else
        return internalBodyPartMemento( which );
}

KMail::Interface::BodyPartMemento * partNode::internalBodyPartMemento( const QCString & which ) const
{
    assert( !reader() );

    const std::map<QCString,KMail::Interface::BodyPartMemento*>::const_iterator it = mBodyPartMementoMap.find( which.lower() );
    return it != mBodyPartMementoMap.end() ? it->second : 0 ;
}

void partNode::setBodyPartMemento( const QCString & which, KMail::Interface::BodyPartMemento * memento )
{
    if ( KMReaderWin * r = reader() )
        r->setBodyPartMemento( this, which, memento );
    else
        internalSetBodyPartMemento( which, memento );
}

void partNode::internalSetBodyPartMemento( const QCString & which, KMail::Interface::BodyPartMemento * memento )
{
    assert( !reader() );

    const std::map<QCString,KMail::Interface::BodyPartMemento*>::iterator it = mBodyPartMementoMap.lower_bound( which.lower() );
    if ( it != mBodyPartMementoMap.end() && it->first == which.lower() ) {
        delete it->second;
        if ( memento ) {
            it->second = memento;
        }
        else {
            mBodyPartMementoMap.erase( it );
        }
    } else {
        mBodyPartMementoMap.insert( it, std::make_pair( which.lower(), memento ) );
    }
}

bool partNode::isDisplayedEmbedded() const
{
  return mDisplayedEmbedded;
}

void partNode::setDisplayedEmbedded( bool displayedEmbedded )
{
  mDisplayedEmbedded = displayedEmbedded;
}

bool partNode::isDisplayedHidden() const
{
  return mDisplayedHidden;
}

void partNode::setDisplayedHidden( bool displayedHidden )
{
  mDisplayedHidden = displayedHidden;
}


QString partNode::asHREF( const QString &place ) const
{
  return QString( "attachment:%1?place=%2" ).arg( nodeId() ).arg( place );
}

partNode::AttachmentDisplayInfo partNode::attachmentDisplayInfo() const
{
  AttachmentDisplayInfo info;
  info.icon = msgPart().iconName( KIcon::Small );
  info.label = msgPart().name().stripWhiteSpace();
  if ( info.label.isEmpty() )
    info.label = msgPart().fileName();
  if ( info.label.isEmpty() )
    info.label = msgPart().contentDescription();
  bool typeBlacklisted = msgPart().typeStr().lower() == "multipart";
  if ( !typeBlacklisted && msgPart().typeStr().lower() == "application" ) {
    typeBlacklisted = msgPart().subtypeStr() == "pgp-encrypted"
        || msgPart().subtypeStr().lower() == "pgp-signature"
        || msgPart().subtypeStr().lower() == "pkcs7-mime"
        || msgPart().subtypeStr().lower() == "pkcs7-signature";
  }
  typeBlacklisted = typeBlacklisted || this == topLevelParent();
  bool firstTextChildOfEncapsulatedMsg = msgPart().typeStr().lower() == "text" &&
                                         msgPart().subtypeStr().lower() == "plain" &&
                                         parentNode() &&
                                         parentNode()->msgPart().typeStr().lower() == "message";
  typeBlacklisted = typeBlacklisted || firstTextChildOfEncapsulatedMsg;
  info.displayInHeader = !info.label.isEmpty() && !info.icon.isEmpty() && !typeBlacklisted;
  return info;
}
