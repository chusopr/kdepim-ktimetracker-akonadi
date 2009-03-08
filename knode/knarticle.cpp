/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/


#include <klocale.h>
#include <kcodecs.h>
#include <kmimetype.h>

#include "knhdrviewitem.h"
#include "kngroup.h"
#include "knglobals.h"
#include "knconfigmanager.h"
#include "utilities.h"
#include "settings.h"

//Added by qt3to4:
#include <QByteArray>

using namespace KMime;


KNArticle::KNArticle(KNArticleCollection *c) : i_d(-1), c_ol(c), i_tem(0)
{
}


KNArticle::~KNArticle()
{
  delete i_tem;
}


void KNArticle::clear()
{
  f_lags.clear();
}


void KNArticle::setListItem(KNHdrViewItem *it)
{
  i_tem=it;
  if(i_tem) i_tem->art=this;
}


void KNArticle::setLocked(bool b)
{
  f_lags.set(0, b);
  if(c_ol) {  // local articles may have c_ol==0 !
    if(b)
      c_ol->articleLocked();
    else
      c_ol->articleUnlocked();
  }
}


QByteArray KNArticle::assembleHeaders()
{
  // filter out internal headers
  for ( Headers::Base::List::Iterator it = h_eaders.begin(); it != h_eaders.end(); ) {
    if ( (*it)->isXHeader() && ( strncasecmp( (*it)->type(), "X-KNode", 7 ) == 0 ) ) {
      delete *it;
      it = h_eaders.erase( it );
    }
    else
      ++it;
  }

  return KMime::NewsArticle::assembleHeaders();
}


//=========================================================================================


KNRemoteArticle::KNRemoteArticle(KNGroup *g)
 : KNArticle(g), a_rticleNumber(-1), i_dRef(-1), d_ref(0), t_hrLevel(0), s_core(0),
   c_olor(knGlobals.settings()->unreadThreadColor()),
   u_nreadFups(0), n_ewFups(0), s_ubThreadChangeDate(0)
{
  m_essageID.setParent(this);
  f_rom.setParent(this);
  r_eferences.setParent(this);

  if (g && g->useCharset())
    setDefaultCharset( g->defaultCharset() );
  else
    setDefaultCharset( knGlobals.settings()->charset().toLatin1() );
}


KNRemoteArticle::~KNRemoteArticle()
{}


void KNRemoteArticle::parse()
{
  KNArticle::parse();
  QByteArray raw;
  if( !(raw=rawHeader(m_essageID.type())).isEmpty() )
    m_essageID.from7BitString(raw);

  if( !(raw=rawHeader(f_rom.type())).isEmpty() )
    f_rom.from7BitString(raw);

  if( !(raw=rawHeader(r_eferences.type())).isEmpty() )
    r_eferences.from7BitString(raw);
}


void KNRemoteArticle::clear()
{
  m_essageID.clear();
  f_rom.clear();
  r_eferences.clear();
  KNArticle::clear();
}


Headers::Base* KNRemoteArticle::getHeaderByType(const char *type)
{
  if(strcasecmp("Message-ID", type)==0) {
    if(m_essageID.isEmpty()) return 0;
    else return &m_essageID;
  }
  else if(strcasecmp("From", type)==0) {
    if(f_rom.isEmpty()) return 0;
    else return &f_rom;
  }
  else if(strcasecmp("References", type)==0) {
    if(r_eferences.isEmpty()) return 0;
    else return &r_eferences;
  }
  else
    return KNArticle::getHeaderByType(type);
}


void KNRemoteArticle::setHeader(Headers::Base *h)
{
  bool del=true;
  if(h->is("Message-ID"))
    m_essageID.from7BitString(h->as7BitString(false));
  else if(h->is("From")) {
    foreach ( const KMime::Types::Mailbox &mbox, static_cast<Headers::From*>( h )->mailboxes() ) {
      f_rom.addAddress( mbox );
    }
  }
  else if(h->is("References")) {
    r_eferences.from7BitString(h->as7BitString(false));
  }
  else {
    del=false;
    KNArticle::setHeader(h);
  }

  if(del) delete h;
}


bool KNRemoteArticle::removeHeader(const char *type)
{
  if(strcasecmp("Message-ID", type)==0)
    m_essageID.clear();
  else if(strcasecmp("From", type)==0)
    f_rom.clear();
  else if(strcasecmp("References", type)==0)
    r_eferences.clear();
  else
     return KNArticle::removeHeader(type);

  return true;
}


void KNRemoteArticle::initListItem()
{
  if(!i_tem) return;

  KMime::Types::Mailbox mbox;
  if ( !f_rom.isEmpty() )
    mbox = f_rom.mailboxes().first();
  if ( mbox.hasName() )
    i_tem->setText( 1, mbox.name() );
  else
    i_tem->setText( 1, QString::fromLatin1( mbox.address() ) );

  updateListItem();
}


void KNRemoteArticle::updateListItem()
{
  if(!i_tem) return;

  KNode::Appearance *app=knGlobals.configManager()->appearance();

  if(isRead()) {
    if(hasContent())
      i_tem->setPixmap(0, app->icon(KNode::Appearance::greyBallChkd));
    else
      i_tem->setPixmap(0, app->icon(KNode::Appearance::greyBall));
  }
  else {
    if(hasContent())
      i_tem->setPixmap(0,app->icon(KNode::Appearance::redBallChkd));
    else
      i_tem->setPixmap(0, app->icon(KNode::Appearance::redBall));
  }

  if(hasNewFollowUps())
    i_tem->setPixmap(1, app->icon(KNode::Appearance::newFups));
  else
    i_tem->setPixmap(1, app->icon(KNode::Appearance::null));

  if(isWatched())
    i_tem->setPixmap(2, app->icon(KNode::Appearance::eyes));
  else {
    if(isIgnored())
      i_tem->setPixmap(2, app->icon(KNode::Appearance::ignore));
    else
      i_tem->setPixmap(2, app->icon(KNode::Appearance::null));
  }

  i_tem->setExpandable( (threadMode() && hasVisibleFollowUps()) );

  i_tem->repaint(); //force repaint
}


void KNRemoteArticle::thread(KNRemoteArticle::List &l)
{
  KNRemoteArticle *tmp=0, *ref=this;
  KNGroup *g=static_cast<KNGroup*>(c_ol);
  int idRef=i_dRef, topID=-1;

  while(idRef!=0) {
    ref=g->byId(idRef);
    if(!ref)
      return; // sh#t !!
    idRef=ref->idRef();
  }

  topID=ref->id();
  l.append(ref);

  for(int i=0; i<g->length(); i++) {
    tmp=g->at(i);
    if(tmp->idRef()!=0) {
      idRef=tmp->idRef();
      while(idRef!=0) {
        ref=g->byId(idRef);
        idRef=ref->idRef();
      }
      if(ref->id()==topID)
        l.append(tmp);
    }
  }
}


void KNRemoteArticle::setForceDefaultCharset(bool b)
{
  if (!b) { // restore default
    KNGroup *g=static_cast<KNGroup*>(c_ol);
    if (g && g->useCharset())
      setDefaultCharset( g->defaultCharset() );
    else
      setDefaultCharset( knGlobals.settings()->charset().toLatin1() );
  }
  KNArticle::setForceDefaultCharset( b );
  initListItem();
}


void KNRemoteArticle::propagateThreadChangedDate()
{
  KNRemoteArticle *ref=this;
  KNGroup *g=static_cast<KNGroup*>(c_ol);
  int idRef=i_dRef;

  while (idRef!=0) {
    ref=g->byId(idRef);
    if(!ref)
      return; // sh#t !!
    idRef=ref->idRef();
  }

  if (date()->dateTime() > ref->date()->dateTime()) {
    ref->setSubThreadChangeDate(date()->dateTime().toTime_t());
  }
}


//=========================================================================================


KNLocalArticle::KNLocalArticle(KNArticleCollection *c)
  : KNArticle(c), s_Offset(0), e_Offset(0), s_erverId(-1)
{
  n_ewsgroups.setParent(this);
  t_o.setParent(this);
  setDefaultCharset( knGlobals.settings()->charset().toLatin1() );
}


KNLocalArticle::~KNLocalArticle()
{}


void KNLocalArticle::parse()
{
  KNArticle::parse();
  QByteArray raw;

  if( !(raw=rawHeader(n_ewsgroups.type())).isEmpty() )
    n_ewsgroups.from7BitString(raw);

  if( !(raw=rawHeader(t_o.type())).isEmpty() )
    t_o.from7BitString(raw);
}


void KNLocalArticle::clear()
{
  KNArticle::clear();
  n_ewsgroups.clear();
  t_o.clear();
}


Headers::Base* KNLocalArticle::getHeaderByType(const char *type)
{
  if(strcasecmp("Newsgroups", type)==0)
    return newsgroups(false);
  else if(strcasecmp("To", type)==0)
    return to(false);
  else
    return KNArticle::getHeaderByType(type);
}


void KNLocalArticle::setHeader(Headers::Base *h)
{
  bool del=true;
  if(h->is("To"))
    t_o.from7BitString(h->as7BitString(false));
  else if(h->is("Newsgroups"))
    n_ewsgroups.from7BitString(h->as7BitString(false));
  else {
    del=false;
    KNArticle::setHeader(h);
  }

  if(del) delete h;
}


bool KNLocalArticle::removeHeader(const char *type)
{
  if(strcasecmp("To", type)==0)
    t_o.clear();
  else if(strcasecmp("Newsgroups", type)==0)
    n_ewsgroups.clear();
  else
     return KNArticle::removeHeader(type);

  return true;
}


void KNLocalArticle::updateListItem()
{
  if(!i_tem)
    return;

  QString tmp;
  int idx=0;
  KNode::Appearance *app=knGlobals.configManager()->appearance();

  if(isSavedRemoteArticle()) {
    i_tem->setPixmap(0, app->icon(KNode::Appearance::savedRemote));
    if (!n_ewsgroups.isEmpty())
      tmp=n_ewsgroups.asUnicodeString();
    else
      tmp=t_o.asUnicodeString();
  }
  else {

    if(doPost()) {
      tmp+=n_ewsgroups.asUnicodeString();
      if(canceled())
        i_tem->setPixmap(idx++, app->icon(KNode::Appearance::canceledPosting));
      else
        i_tem->setPixmap(idx++, app->icon(KNode::Appearance::posting));
    }

    if(doMail()) {
      i_tem->setPixmap(idx++, app->icon(KNode::Appearance::mail));
      if(doPost())
        tmp+=" / ";
      tmp+=t_o.asUnicodeString();
    }

  }

  i_tem->setText(1, tmp);
}


void KNLocalArticle::setForceDefaultCharset( bool b )
{
  if (!b)  // restore default
    setDefaultCharset( knGlobals.settings()->charset().toLatin1() );
  KNArticle::setForceDefaultCharset( b );
  updateListItem();
}


//=========================================================================================


KNAttachment::KNAttachment(Content *c)
  : c_ontent(c), l_oadHelper(0), f_ile(0), i_sAttached(true)
{
  Headers::ContentType  *t=c->contentType();
  Headers::ContentTransferEncoding   *e=c->contentTransferEncoding();
  Headers::ContentDescription *d=c->contentDescription(false);

  n_ame=t->name();

  if(d)
    d_escription=d->asUnicodeString();


  setMimeType(t->mimeType());

  if(e->encoding()==Headers::CEuuenc) {
    setCte( Headers::CEbase64 );
    updateContentInfo();
  }
  else
    e_ncoding.setEncoding( e->encoding() );


  h_asChanged=false; // has been set to "true" in setMimeType()
}


KNAttachment::KNAttachment(KNLoadHelper *helper)
  : c_ontent(0), l_oadHelper(helper), f_ile(helper->getFile()), i_sAttached(false), h_asChanged(true)
{
  setMimeType(KMimeType::findByPath(f_ile->fileName())->name());
  n_ame=helper->getURL().fileName();
}


KNAttachment::~KNAttachment()
{
  if(!i_sAttached && c_ontent)
    delete c_ontent;
  delete l_oadHelper;
}


void KNAttachment::setMimeType(const QString &s)
{
  mMimeType = s;
  h_asChanged=true;

  if ( !mMimeType.contains( "text/", Qt::CaseInsensitive ) ) {
    f_b64=true;
    e_ncoding.setEncoding(Headers::CEbase64);
  }
  else {
    f_b64=false;
    if ( knGlobals.settings()->allow8BitBody() )
      setCte(Headers::CE8Bit);
    else
      setCte(Headers::CEquPr);
  }
}


QString KNAttachment::contentSize() const
{
  QString ret;
  int s=0;

  if(c_ontent && c_ontent->hasContent())
    s=c_ontent->size();
  else {
    if (f_ile)
      s=f_ile->size();
  }

  if(s > 1023) {
    s=s/1024;
    ret.setNum(s);
    ret+=" kB";
  }
  else {
    ret.setNum(s);
    ret+=" Bytes";
  }

  return ret;
}


void KNAttachment::updateContentInfo()
{
  if(!h_asChanged || !c_ontent)
    return;

  //Content-Type
  Headers::ContentType *t=c_ontent->contentType();
  t->setMimeType( mMimeType.toLatin1() );
  t->setName(n_ame, "UTF-8");
  t->setCategory(Headers::CCmixedPart);

  //Content-Description
  if(d_escription.isEmpty())
    c_ontent->removeHeader("Content-Description");
  else
    c_ontent->contentDescription()->fromUnicodeString(d_escription, "UTF-8");

  //Content-Disposition
  Headers::ContentDisposition *d=c_ontent->contentDisposition();
  d->setDisposition(Headers::CDattachment);
  d->setFilename(n_ame);

  //Content-Transfer-Encoding
  if(i_sAttached)
    c_ontent->changeEncoding(e_ncoding.encoding());
  else
    c_ontent->contentTransferEncoding()->setEncoding(e_ncoding.encoding());

  c_ontent->assemble();

  h_asChanged=false;
}



void KNAttachment::attach(Content *c)
{
  if(i_sAttached || !f_ile)
    return;

  c_ontent=new Content();
  updateContentInfo();
  Headers::ContentType *type=c_ontent->contentType();
  Headers::ContentTransferEncoding *e=c_ontent->contentTransferEncoding();
  QByteArray data;

  data = f_ile->readAll();

  if ( data.size() < f_ile->size() && f_ile->error() != QFile::NoError ) {
    KNHelper::displayExternalFileError();
    delete c_ontent;
    c_ontent=0;
  } else {
    if (e_ncoding.encoding()==Headers::CEbase64 || !type->isText()) { //encode base64
      c_ontent->setBody( KCodecs::base64Encode(data, true) + '\n' );
      //      c_ontent->b_ody += '\n';
      e->setEncoding(Headers::CEbase64);
      e->setDecoded(false);
    } else  {
      c_ontent->setBody( data + '\n' );
      //      c_ontent->b_ody += '\n';
      e->setDecoded(true);
    }
  }

  if(c_ontent) {
    c->addContent(c_ontent);
    i_sAttached=true;
  }
}


void KNAttachment::detach(Content *c)
{
  if(i_sAttached) {
    c->removeContent(c_ontent, false);
    i_sAttached=false;
  }
}
