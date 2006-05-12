/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2006 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include <QByteArray>
#include <QList>
#include <QListWidget>
#include <QLayout>
#include <QLabel>

#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kwin.h>
#include <kapplication.h>
#include <ktoolinvocation.h>
#include <kvbox.h>

#include "knarticlefactory.h"
#include "knconfigmanager.h"
#include "knglobals.h"
#include "kngroupmanager.h"
#include "knaccountmanager.h"
#include "knfoldermanager.h"
#include "knarticlemanager.h"
#include "knfolder.h"
#include "kncomposer.h"
#include "knnntpaccount.h"
#include "mailsendjob.h"
#include "nntpjobs.h"
#include "utilities.h"
#include "resource.h"
#include "settings.h"

using namespace KNode;

KNArticleFactory::KNArticleFactory( QObject *parent )
  : QObject( parent ), s_endErrDlg(0)
{
}


KNArticleFactory::~KNArticleFactory()
{
  for ( QList<KNComposer*>::Iterator it = mCompList.begin(); it != mCompList.end(); ++it )
    delete (*it);
  delete s_endErrDlg;
}


void KNArticleFactory::createPosting(KNNntpAccount *a)
{
  if(!a)
    return;

  QString sig;
  KNLocalArticle *art=newArticle( a, sig, knGlobals.settings()->charset().toLatin1() );
  if(!art)
    return;

  art->setServerId(a->id());
  art->setDoPost(true);
  art->setDoMail(false);

  KNComposer *c = new KNComposer( art, QString(), sig, QString(), true );
  mCompList.append( c );
  connect(c, SIGNAL(composerDone(KNComposer*)), this, SLOT(slotComposerDone(KNComposer*)));
  c->show();
}


void KNArticleFactory::createPosting(KNGroup *g)
{
  if(!g)
    return;

  QByteArray chset;
  if (g->useCharset())
    chset = g->defaultCharset();
  else
    chset = knGlobals.settings()->charset().toLatin1();

  QString sig;
  KNLocalArticle *art=newArticle(g, sig, chset);

  if(!art)
    return;

  art->setServerId(g->account()->id());
  art->setDoPost(true);
  art->setDoMail(false);
  art->newsgroups()->fromUnicodeString(g->groupname(), art->defaultCharset());

  KNComposer *c = new KNComposer( art, QString(), sig, QString(), true );
  mCompList.append( c );
  connect(c, SIGNAL(composerDone(KNComposer*)), this, SLOT(slotComposerDone(KNComposer*)));
  c->show();
}


void KNArticleFactory::createReply(KNRemoteArticle *a, QString selectedText, bool post, bool mail)
{
  if(!a)
    return;

  KNGroup *g=static_cast<KNGroup*>(a->collection());

  QByteArray chset;
  if ( knGlobals.settings()->useOwnCharset() ) {
    if (g->useCharset())
      chset = g->defaultCharset();
    else
      chset = knGlobals.settings()->charset().toLatin1();
  } else
    chset = a->contentType()->charset();

  //create new article
  QString sig;
  KNLocalArticle *art=newArticle(g, sig, chset, true, a);
  if(!art)
    return;

  art->setServerId(g->account()->id());
  art->setDoPost(post);
  art->setDoMail(mail);

  //------------------------- <Headers> ----------------------------

  //subject
  QString subject=a->subject()->asUnicodeString();
  if ( subject.left(3).toUpper() != "RE:" )
    subject.prepend("Re: ");
  art->subject()->fromUnicodeString(subject, a->subject()->rfc2047Charset());

  //newsgroups
  KMime::Headers::FollowUpTo *fup2=a->followUpTo(false);
  if(fup2 && !fup2->isEmpty()) {
    if( ( fup2->as7BitString(false).toUpper()=="POSTER" ) ) { //Followup-To: poster
      if( post && // user wanted to reply by public posting?
          // ask the user if she wants to ignore this F'up-To: poster
          ( KMessageBox::Yes != KMessageBox::questionYesNo(knGlobals.topWidget,
            i18n("The author has requested a reply by email instead\nof a followup to the newsgroup. (Followup-To: poster)\nDo you want to reply in public anyway?"), QString(), i18n("Reply Public"), i18n("Reply by Email")) ))
      {
        art->setDoPost(false);
        art->setDoMail(true);
      }
      art->newsgroups()->from7BitString(a->newsgroups()->as7BitString(false));
    }
    else
      art->newsgroups()->from7BitString(fup2->as7BitString(false));
  }
  else
    art->newsgroups()->from7BitString(a->newsgroups()->as7BitString(false));

  //To
  KMime::Headers::ReplyTo *replyTo=a->replyTo(false);
  KMime::Headers::AddressField address;
  if(replyTo && !replyTo->isEmpty()) {
    if(replyTo->hasName())
      address.setName(replyTo->name());
    if(replyTo->hasEmail())
      address.setEmail(replyTo->email());
  }
  else {
    KMime::Headers::From *from=a->from();
    if(from->hasName())
      address.setName(from->name());
    if(from->hasEmail())
      address.setEmail(from->email());
  }
  art->to()->addAddress(address);

  //References
  KMime::Headers::References *references=a->references(false);
  QByteArray refs;
  if (references)
    refs=references->as7BitString(false);
  else
    refs = "";

  art->references()->from7BitString(refs);
  art->references()->append(a->messageID()->as7BitString(false));

  //Mail-Copies-To
  bool authorDislikesMailCopies=false;
  bool authorWantsMailCopies=false;
  KMime::Headers::MailCopiesTo *mailCopiesTo=a->mailCopiesTo(false);

  if(mailCopiesTo && !mailCopiesTo->isEmpty() && mailCopiesTo->isValid()) {
    authorDislikesMailCopies = mailCopiesTo->neverCopy();
    authorWantsMailCopies = mailCopiesTo->alwaysCopy();
    if (authorWantsMailCopies)         // warn the user
      KMessageBox::information( knGlobals.topWidget, i18n("The author requested a mail copy of your reply. (Mail-Copies-To header)"),
                                QString(), "mailCopiesToWarning" );
    if (authorWantsMailCopies && mailCopiesTo->hasEmail()) {
      address.setName(mailCopiesTo->name());
      address.setEmail(mailCopiesTo->email());
      art->to()->clear();
      art->to()->addAddress(address);
    }
  }

  //------------------------- </Headers> ---------------------------

  //--------------------------- <Body> -----------------------------

  // attribution line
  QString attribution = knGlobals.settings()->intro();
  QString name(a->from()->name());
  if (name.isEmpty())
    name = QString::fromLatin1(a->from()->email());
  attribution.replace(QRegExp("%NAME"),name);
  attribution.replace(QRegExp("%EMAIL"),QString::fromLatin1(a->from()->email()));
  attribution.replace(QRegExp("%DATE"),KGlobal::locale()->formatDateTime(a->date()->qdt(),false));
  attribution.replace(QRegExp("%MSID"),a->messageID()->asUnicodeString());
  attribution.replace(QRegExp("%GROUP"),g->groupname());
  attribution.replace(QRegExp("%L"),"\n");
  attribution+="\n\n";

  QString quoted=attribution;
  QString notRewraped;
  QStringList text;
  QStringList::Iterator line;
  bool incSig = knGlobals.settings()->includeSignature();

  if (selectedText.isEmpty()) {
    KMime::Content *tc = a->textContent();
    if(tc)
      text = tc->decodedText( true, knGlobals.settings()->removeTrailingNewlines() ).split( '\n' );
  }
  else
    text = selectedText.split('\n');

  for(line=text.begin(); line!=text.end(); ++line) {
    if(!incSig && (*line)=="-- ")
      break;

    if ((*line)[0]=='>')
      quoted += '>' + (*line) + '\n';  // second quote level without space
    else
      quoted += "> " +(*line) + '\n';
  }

  if ( knGlobals.settings()->rewrap() ) {  //rewrap original article

    notRewraped=quoted;     // store the original text in here, the user can request it in the composer
    quoted=attribution;

    quoted += KNHelper::rewrapStringList( text, knGlobals.settings()->maxLineLength(), '>', !incSig, false );
  }

  //-------------------------- </Body> -----------------------------

  if ( art->doMail() && knGlobals.settings()->useExternalMailer() ) {
    sendMailExternal(address.asUnicodeString(), subject, quoted);
    art->setDoMail(false);
    if (!art->doPost()) {
      delete art;
      return;
    }
  }

  //open composer
  KNComposer *c=new KNComposer(art, quoted, sig, notRewraped, true, authorDislikesMailCopies, authorWantsMailCopies);
  mCompList.append( c );
  connect(c, SIGNAL(composerDone(KNComposer*)), this, SLOT(slotComposerDone(KNComposer*)));
  c->show();
}


void KNArticleFactory::createForward(KNArticle *a)
{
  if(!a)
    return;

  KMime::Headers::ContentType *ct=a->contentType();
  QByteArray chset;
  bool incAtt = ( !knGlobals.settings()->useExternalMailer() &&
                  ct->isMultipart() && ct->isSubtype("mixed") &&
                  KMessageBox::Yes == KMessageBox::questionYesNo(knGlobals.topWidget,
                  i18n("This article contains attachments. Do you want them to be forwarded as well?"), QString(), i18n("Forward"), i18n("Do Not Forward"))
                );

  if ( knGlobals.settings()->useOwnCharset() )
    chset = knGlobals.settings()->charset().toLatin1();
  else
    chset = a->contentType()->charset();

  //create new article
  QString sig;
  KNLocalArticle *art=newArticle(knGlobals.groupManager()->currentGroup(), sig, chset);
  if(!art)
    return;

  art->setDoPost(false);
  art->setDoMail(true);

  //------------------------- <Headers> ----------------------------

  //subject
  QString subject=("Fwd: "+a->subject()->asUnicodeString());
  art->subject()->fromUnicodeString(subject, a->subject()->rfc2047Charset());

  //------------------------- </Headers> ---------------------------

  //--------------------------- <Body> -----------------------------

  QString fwd = QString("\n--------------- %1\n\n").arg(i18n("Forwarded message (begin)"));

  fwd+=( i18n("Subject") + ": " + a->subject()->asUnicodeString() + "\n" );
  fwd+=( i18n("From") + ": " + a->from()->asUnicodeString() + "\n" );
  fwd+=( i18n("Date") + ": " + a->date()->asUnicodeString() + "\n" );
  fwd+=( i18n("Newsgroup") + ": " + a->newsgroups()->asUnicodeString() + "\n\n" );

  KMime::Content *text=a->textContent();
  if(text)
    fwd += text->decodedText( false, false );

  fwd += QString("\n--------------- %1\n").arg(i18n("Forwarded message (end)"));

  //--------------------------- </Body> ----------------------------


  //------------------------ <Attachments> -------------------------

  if(incAtt) {
    KMime::Content::List al = a->attachments( false );
    foreach ( KMime::Content *c, al )
      art->addContent( new KMime::Content(c->head(), c->body()) );
  }

  //------------------------ </Attachments> ------------------------


  if ( knGlobals.settings()->useExternalMailer() ) {
    sendMailExternal( QString(), subject, fwd );
    delete art;
    return;
  }

  //open composer
  KNComposer *c = new KNComposer( art, fwd, sig, QString(), true );
  mCompList.append( c );
  connect(c, SIGNAL(composerDone(KNComposer*)), this, SLOT(slotComposerDone(KNComposer*)));
  c->show();
}


void KNArticleFactory::createCancel(KNArticle *a)
{
  if(!cancelAllowed(a))
    return;

  if( KMessageBox::No == KMessageBox::questionYesNo( knGlobals.topWidget,
      i18n("Do you really want to cancel this article?"), QString(), i18n("Cancel Article"), KStdGuiItem::cancel() ) )
    return;

  bool sendNow;
  switch (KMessageBox::warningYesNoCancel(knGlobals.topWidget, i18n("Do you want to send the cancel\nmessage now or later?"), i18n("Question"),i18n("&Now"),i18n("&Later"))) {
    case KMessageBox::Yes : sendNow = true; break;
    case KMessageBox::No :  sendNow = false; break;
    default :               return;
  }

  KNGroup *grp;
  KNNntpAccount *nntp=0;

  if(a->type()==KMime::Base::ATremote)
    nntp=(static_cast<KNGroup*>(a->collection()))->account();
  else {
    if(!nntp)
      nntp=knGlobals.accountManager()->first();
    if(!nntp) {
      KMessageBox::error(knGlobals.topWidget, i18n("You have no valid news accounts configured."));
      return;
    }
    KNLocalArticle *la=static_cast<KNLocalArticle*>(a);
    la->setCanceled(true);
    la->updateListItem();
    nntp=knGlobals.accountManager()->account(la->serverId());
  }

  grp=knGlobals.groupManager()->group(a->newsgroups()->firstGroup(), nntp);

  QString sig;
  KNLocalArticle *art=newArticle(grp, sig, "us-ascii", false);
  if(!art)
    return;

  //init
  art->setDoPost(true);
  art->setDoMail(false);

  //server
  art->setServerId(nntp->id());

  //subject
  KMime::Headers::MessageID *msgId=a->messageID();
  QByteArray tmp;
  tmp="cancel of "+msgId->as7BitString(false);
  art->subject()->from7BitString(tmp);

  //newsgroups
  art->newsgroups()->from7BitString(a->newsgroups()->as7BitString(false));

  //control
  tmp="cancel "+msgId->as7BitString(false);
  art->control()->from7BitString(tmp);

  //Lines
  art->lines()->setNumberOfLines(1);

  //body
  art->fromUnicodeString(QString::fromLatin1("cancel by original author\n"));

  //assemble
  art->assemble();

  //send
  KNLocalArticle::List lst;
  lst.append(art);
  sendArticles( lst, sendNow );
}


void KNArticleFactory::createSupersede(KNArticle *a)
{
  if (!a)
    return;

  if(!cancelAllowed(a))
    return;

  if ( KMessageBox::No==KMessageBox::questionYesNo( knGlobals.topWidget,
       i18n("Do you really want to supersede this article?"), QString(), i18n("Supersede"), KStdGuiItem::cancel() ) )
    return;

  KNGroup *grp;
  KNNntpAccount *nntp;

  if(a->type()==KMime::Base::ATremote)
    nntp=(static_cast<KNGroup*>(a->collection()))->account();
  else {
    KNLocalArticle *la=static_cast<KNLocalArticle*>(a);
    la->setCanceled(true);
    la->updateListItem();
    nntp=knGlobals.accountManager()->account(la->serverId());
    if(!nntp)
      nntp=knGlobals.accountManager()->first();
    if(!nntp) {
      KMessageBox::error(knGlobals.topWidget, i18n("You have no valid news accounts configured."));
      return;
    }
  }

  grp=knGlobals.groupManager()->group(a->newsgroups()->firstGroup(), nntp);

  //new article
  QString sig;
  KNLocalArticle *art = newArticle( grp, sig, a->contentType()->charset() );
  if(!art)
    return;

  art->setDoPost(true);
  art->setDoMail(false);

  //server
  art->setServerId(nntp->id());

  //subject
  art->subject()->fromUnicodeString(a->subject()->asUnicodeString(), a->subject()->rfc2047Charset());

  //newsgroups
  art->newsgroups()->from7BitString(a->newsgroups()->as7BitString(false));

  //followup-to
  art->followUpTo()->from7BitString(a->followUpTo()->as7BitString(false));

  //References
  if ( !a->references()->isEmpty() )
    art->references()->from7BitString( a->references()->as7BitString(false) );

  //Supersedes
  art->supersedes()->from7BitString(a->messageID()->as7BitString(false));

  //Body
  QString text;
  KMime::Content *textContent=a->textContent();
  if(textContent)
    text = textContent->decodedText();

  //open composer
  KNComposer *c=new KNComposer(art, text, sig);
  mCompList.append( c );
  connect(c, SIGNAL(composerDone(KNComposer*)), this, SLOT(slotComposerDone(KNComposer*)));
  c->show();
}


void KNArticleFactory::createMail(KMime::Headers::AddressField *address)
{
  if ( knGlobals.settings()->useExternalMailer() ) {
    sendMailExternal(address->asUnicodeString());
    return;
  }

  //create new article
  QString sig;
  KNLocalArticle *art = newArticle( knGlobals.groupManager()->currentGroup(), sig, knGlobals.settings()->charset().toLatin1() );
  if(!art)
    return;

  art->setDoMail(true);
  art->setDoPost(false);
  art->to()->addAddress((*address));

  //open composer
  KNComposer *c = new KNComposer( art, QString(), sig, QString(), true );
  mCompList.append( c );
  connect(c, SIGNAL(composerDone(KNComposer*)), this, SLOT(slotComposerDone(KNComposer*)));
  c->show();
}


void KNArticleFactory::sendMailExternal(const QString &address, const QString &subject, const QString &body)
{
  KUrl mailtoURL;
  QStringList queries;
  QString query;
  mailtoURL.setProtocol("mailto");

  if (!address.isEmpty())
    mailtoURL.setPath(address);
  if (!subject.isEmpty())
    queries.append("subject="+KUrl::encode_string(subject));
  if (!body.isEmpty())
    queries.append("body="+KUrl::encode_string(body));

  if (queries.count() > 0) {
    query = "?";
    for ( QStringList::Iterator it = queries.begin(); it != queries.end(); ++it ) {
      if (it != queries.begin())
        query.append("&");
      query.append((*it));
    }
  }

  if (!query.isEmpty())
    mailtoURL.setQuery(query);

  KToolInvocation::invokeMailer(mailtoURL);
}


void KNArticleFactory::edit(KNLocalArticle *a)
{
  if(!a)
    return;

  KNComposer *com=findComposer(a);
  if(com) {
    KWin::activateWindow(com->winId());
    return;
  }

  if(a->editDisabled()) {
    KMessageBox::sorry(knGlobals.topWidget, i18n("This article cannot be edited."));
    return;
  }

  //find signature
  KNode::Identity *id=knGlobals.configManager()->identity();

  if(a->doPost()) {
    KNNntpAccount *acc=knGlobals.accountManager()->account(a->serverId());
    if(acc) {
      KMime::Headers::Newsgroups *grps=a->newsgroups();
      KNGroup *grp=knGlobals.groupManager()->group(grps->firstGroup(), acc);
      if (grp && grp->identity())
        id=grp->identity();
      else if (acc->identity())
        id=acc->identity();
    }
  }

  //load article body
  if(!a->hasContent())
    knGlobals.articleManager()->loadArticle(a);

  //open composer
  com = new KNComposer( a, QString(), id->getSignature() );
  if(id->useSigGenerator() && !id->getSigGeneratorStdErr().isEmpty())
  KMessageBox::information(knGlobals.topWidget,
                            i18n("<qt>The signature generator program produced the "
                                "following output:<br><br>%1</qt>",
                                 id->getSigGeneratorStdErr()));

  mCompList.append( com );
  connect(com, SIGNAL(composerDone(KNComposer*)), this, SLOT(slotComposerDone(KNComposer*)));
  com->show();
}


void KNArticleFactory::sendArticles( KNLocalArticle::List &l, bool now )
{
  KNJobData *job=0;
  KNServerInfo *ser=0;

  KNLocalArticle::List unsent, sent;
  for ( KNLocalArticle::List::Iterator it = l.begin(); it != l.end(); ++it ) {
    if ( (*it)->pending() )
      unsent.append( (*it) );
    else
      sent.append( (*it) );
  }

  if(!sent.isEmpty()) {
    showSendErrorDialog();
    for ( KNLocalArticle::List::Iterator it = sent.begin(); it != sent.end(); ++it )
      s_endErrDlg->append( (*it)->subject()->asUnicodeString(), i18n("Article has already been sent.") );
  }

  if(!now) {
    knGlobals.articleManager()->moveIntoFolder(unsent, knGlobals.folderManager()->outbox());
    return;
  }


  for ( KNLocalArticle::List::Iterator it = unsent.begin(); it != unsent.end(); ++it ) {

    if ( (*it)->isLocked() )
      continue;

    if ( !(*it)->hasContent() ) {
      if ( !knGlobals.articleManager()->loadArticle( (*it) ) ) {
        showSendErrorDialog();
        s_endErrDlg->append( (*it)->subject()->asUnicodeString(), i18n("Unable to load article.") );
        continue;
      }
    }

    if ( (*it)->doPost() && !(*it)->posted() ) {
      ser = knGlobals.accountManager()->account( (*it)->serverId() );
      job = new ArticlePostJob( this, ser, (*it) );
      emitJob(job);
    }
    else if( (*it)->doMail() && !(*it)->mailed() ) {
      ser = knGlobals.accountManager()->smtp();
      job = new MailSendJob( this, ser, (*it) );
      emitJob(job);
    }
  }
}


void KNArticleFactory::sendOutbox()
{
  KNLocalArticle::List lst;
  KNFolder *ob=0;

  if(!knGlobals.folderManager()->loadOutbox()) {
    KMessageBox::error(knGlobals.topWidget, i18n("Unable to load the outbox-folder."));
    return;
  }

  ob=knGlobals.folderManager()->outbox();
  for(int i=0; i< ob->length(); i++)
    lst.append(ob->at(i));

  sendArticles( lst, true );
}


bool KNArticleFactory::closeComposeWindows()
{
  while ( !mCompList.isEmpty() ) {
    QList<KNComposer*>::Iterator it = mCompList.begin();
    if ( !(*it)->close() )
      return false;
  }

  return true;
}


void KNArticleFactory::deleteComposerForArticle(KNLocalArticle *a)
{
  KNComposer *com = findComposer( a );
  if ( com ) {
    mCompList.removeAll( com );
    delete com;
  }
}


KNComposer* KNArticleFactory::findComposer(KNLocalArticle *a)
{
  for ( QList<KNComposer*>::Iterator it = mCompList.begin(); it != mCompList.end(); ++it )
    if ( (*it)->article() == a )
      return (*it);
  return 0;
}


void KNArticleFactory::configChanged()
{
  for ( QList<KNComposer*>::Iterator it = mCompList.begin(); it != mCompList.end(); ++it )
    (*it)->setConfig( false );
}


void KNArticleFactory::processJob(KNJobData *j)
{
  KNLocalArticle *art=static_cast<KNLocalArticle*>(j->data());
  KNLocalArticle::List lst;
  lst.append(art);

  if(j->canceled()) {
    delete j;

    //sending of this article was canceled => move it to the "Outbox-Folder"
    if(art->collection()!=knGlobals.folderManager()->outbox())
      knGlobals.articleManager()->moveIntoFolder(lst, knGlobals.folderManager()->outbox());

    KMessageBox::information(knGlobals.topWidget, i18n("You have aborted the posting of articles. The unsent articles are stored in the \"Outbox\" folder."));

    return;
  }

  if(!j->success()) {
    showSendErrorDialog();
    s_endErrDlg->append(art->subject()->asUnicodeString(), j->errorString());
    delete j; //unlock article

    //sending of this article failed => move it to the "Outbox-Folder"
    if(art->collection()!=knGlobals.folderManager()->outbox())
      knGlobals.articleManager()->moveIntoFolder(lst, knGlobals.folderManager()->outbox());
  }
  else {

    //disable edit
    art->setEditDisabled(true);

    switch(j->type()) {

      case KNJobData::JTpostArticle:
        delete j; //unlock article
        art->setPosted(true);
        if(art->doMail() && !art->mailed()) { //article has been posted, now mail it
          sendArticles( lst, true );
          return;
        }
      break;

      case KNJobData::JTmail:
        delete j; //unlock article
        art->setMailed(true);
      break;

      default: break;
    };

    //article has been sent successfully => move it to the "Sent-folder"
    knGlobals.articleManager()->moveIntoFolder(lst, knGlobals.folderManager()->sent());
  }
}


KNLocalArticle* KNArticleFactory::newArticle(KNCollection *col, QString &sig, QByteArray defChset, bool withXHeaders, KNArticle *origPost)
{
  if ( knGlobals.settings()->generateMessageID() && knGlobals.settings()->hostname().isEmpty() ) {
    KMessageBox::sorry(knGlobals.topWidget, i18n("Please set a hostname for the generation\nof the message-id or disable it."));
    return 0;
  }

  KNLocalArticle *art=new KNLocalArticle(0);
  KNode::Identity *tmpId=0, *id=0;

  if (col) {
    if (col->type() == KNCollection::CTgroup) {
      id = (static_cast<KNGroup *>(col))->identity();
      tmpId = (static_cast<KNGroup *>(col))->account()->identity();
    } else
      if (col->type() == KNCollection::CTnntpAccount) {
        id = (static_cast<KNNntpAccount *>(col))->identity();
      }
  }

  // determine active innermost non-empty identity
  if (!id) {
    if (tmpId)
      id = tmpId;
    else
      id = knGlobals.configManager()->identity();
  }

  //Message-id
  if ( knGlobals.settings()->generateMessageID() )
    art->messageID()->generate( knGlobals.settings()->hostname().toLatin1() );

  //From
  KMime::Headers::From *from=art->from();
  from->setRFC2047Charset( knGlobals.settings()->charset().toLatin1() );

  //name
  if(id->hasName())
    from->setName(id->name());

  //email
  if(id->hasEmail()&&id->emailIsValid())
    from->setEmail(id->email().toLatin1());
  else {
    if ( id->hasEmail() )
      KMessageBox::sorry(knGlobals.topWidget,
	i18n("Please enter a valid email address at the identity tab of the account configuration dialog."));
    else
      KMessageBox::sorry(knGlobals.topWidget,
         i18n("Please enter a valid email address at the identity section of the configuration dialog."));
    delete art;
    return 0;
  }

  //Reply-To
  if(id->hasReplyTo()) {
    art->replyTo()->fromUnicodeString( id->replyTo(), knGlobals.settings()->charset().toLatin1() );
    if (!art->replyTo()->hasEmail())   // the header is invalid => drop it
      art->removeHeader("Reply-To");
  }

  //Mail-Copies-To
  if(id->hasMailCopiesTo()) {
    art->mailCopiesTo()->fromUnicodeString( id->mailCopiesTo(), knGlobals.settings()->charset().toLatin1() );
    if (!art->mailCopiesTo()->isValid())   // the header is invalid => drop it
      art->removeHeader("Mail-Copies-To");
  }

  //Organization
  if(id->hasOrga())
    art->organization()->fromUnicodeString( id->orga(), knGlobals.settings()->charset().toLatin1() );

  //Date
  art->date()->setUnixTime(); //set current date+time

  //User-Agent
  if( !knGlobals.settings()->noUserAgent() ) {
    art->userAgent()->from7BitString("KNode/" KNODE_VERSION);
  }

  //Mime
  KMime::Headers::ContentType *type=art->contentType();
  type->setMimeType("text/plain");

  type->setCharset(defChset);

  if ( defChset.toLower() == "us-ascii"  )
    art->contentTransferEncoding()->setCte(KMime::Headers::CE7Bit);
  else
    art->contentTransferEncoding()->setCte( knGlobals.settings()->allow8BitBody() ? KMime::Headers::CE8Bit : KMime::Headers::CEquPr );

  //X-Headers
  if(withXHeaders) {
    XHeader::List xhdr = knGlobals.settings()->xHeaders();
    for ( XHeader::List::Iterator it = xhdr.begin(); it != xhdr.end(); ++it ) {
      QString value = (*it).value();
      if(origPost) {
        QString name(origPost->from()->name());
        if (name.isEmpty())
          name = QString::fromLatin1(origPost->from()->email());
        value.replace(QRegExp("%NAME"), name);
        value.replace(QRegExp("%EMAIL"), QString::fromLatin1(origPost->from()->email()));
      }
      else
        if ( value.indexOf( "%NAME" ) != -1 || value.indexOf( "%EMAIL" ) != -1 )
          continue;

      art->setHeader( new KMime::Headers::Generic( (*it).name().toLatin1(), art, value,
                      knGlobals.settings()->charset().toLatin1() ) );
    }
  }

  //Signature
  if(id->hasSignature())
  {
    sig=id->getSignature();
    if(id->useSigGenerator() && !id->getSigGeneratorStdErr().isEmpty())
      KMessageBox::information(knGlobals.topWidget,
                               i18n("<qt>The signature generator program produced the "
                                   "following output:<br><br>%1</qt>",
                                    id->getSigGeneratorStdErr()));
  }
  else
    sig.clear();

  return art;
}


bool KNArticleFactory::cancelAllowed(KNArticle *a)
{
  if(!a)
    return false;

  if(a->type()==KMime::Base::ATlocal) {
    KNLocalArticle *localArt=static_cast<KNLocalArticle*>(a);

    if(localArt->doMail() && !localArt->doPost()) {
      KMessageBox::sorry(knGlobals.topWidget, i18n("Emails cannot be canceled or superseded."));
      return false;
    }

    KMime::Headers::Control *ctrl=localArt->control(false);
    if(ctrl && ctrl->isCancel()) {
      KMessageBox::sorry(knGlobals.topWidget, i18n("Cancel messages cannot be canceled or superseded."));
      return false;
    }

    if(!localArt->posted()) {
      KMessageBox::sorry(knGlobals.topWidget, i18n("Only sent articles can be canceled or superseded."));
      return false;
    }

    if(localArt->canceled()) {
      KMessageBox::sorry(knGlobals.topWidget, i18n("This article has already been canceled or superseded."));
      return false;
    }

    KMime::Headers::MessageID *mid=localArt->messageID(false);
    if(!mid || mid->isEmpty()) {
      KMessageBox::sorry(knGlobals.topWidget, i18n(
"This article cannot be canceled or superseded,\n\
because its message-id has not been created by KNode.\n\
But you can look for your article in the newsgroup\n\
and cancel (or supersede) it there."));
      return false;
    }

    return true;
  }
  else if(a->type()==KMime::Base::ATremote) {

    KNRemoteArticle *remArt=static_cast<KNRemoteArticle*>(a);
    KNGroup *g=static_cast<KNGroup*>(a->collection());
    KNode::Identity  *defId=knGlobals.configManager()->identity(),
                        *gid=g->identity(),
                        *accId=g->account()->identity();
    bool ownArticle = false;

    if (gid && gid->hasName())
      ownArticle |= ( gid->name() == remArt->from()->name() );
    if (accId && accId->hasName())
      ownArticle |= ( accId->name() == remArt->from()->name() );
    ownArticle |= ( defId->name() == remArt->from()->name() );

    if(ownArticle) {
      ownArticle = false;
      if(gid && gid->hasEmail())
        ownArticle |= ( gid->email().toLatin1() == remArt->from()->email() );
      if (accId && accId->hasEmail())
        ownArticle |= ( accId->email().toLatin1() == remArt->from()->email() );
      ownArticle |= ( defId->email().toLatin1() == remArt->from()->email() );
    }

    if(!ownArticle) {
      KMessageBox::sorry(knGlobals.topWidget, i18n("This article does not appear to be from you.\nYou can only cancel or supersede your own articles."));
      return false;
    }

    if(!remArt->hasContent())  {
      KMessageBox::sorry(knGlobals.topWidget, i18n("You have to download the article body\nbefore you can cancel or supersede the article."));
      return false;
    }

    return true;
  }

  return false;
}


void KNArticleFactory::showSendErrorDialog()
{
  if(!s_endErrDlg) {
    s_endErrDlg=new KNSendErrorDialog();
    connect(s_endErrDlg, SIGNAL(closeClicked()), this, SLOT(slotSendErrorDialogDone()));
  }
  s_endErrDlg->show();
}


void KNArticleFactory::slotComposerDone(KNComposer *com)
{
  bool delCom=true;
  KNLocalArticle::List lst;
  lst.append(com->article());

  switch(com->result()) {

    case KNComposer::CRsendNow:
      delCom=com->hasValidData();
      if(delCom) {
        if ( com->applyChanges() )
          sendArticles( lst, true );
        else
          delCom = false;
      }
    break;

    case KNComposer::CRsendLater:
      delCom=com->hasValidData();
      if(delCom) {
        if ( com->applyChanges() )
          sendArticles( lst, false );
        else
          delCom = false;
      }
    break;

    case KNComposer::CRsave :
      if ( com->applyChanges() )
        knGlobals.articleManager()->moveIntoFolder(lst, knGlobals.folderManager()->drafts());
    break;

    case KNComposer::CRdelAsk:
      delCom=knGlobals.articleManager()->deleteArticles(lst, true);
    break;

    case KNComposer::CRdel:
      delCom=knGlobals.articleManager()->deleteArticles(lst, false);
    break;

    case KNComposer::CRcancel:
      // just close...
    break;

    default: break;

  };

  if ( delCom ) {
    mCompList.removeAll( com );
    delete com;
  } else
    KWin::activateWindow(com->winId());
}


void KNArticleFactory::slotSendErrorDialogDone()
{
  s_endErrDlg->deleteLater();
  s_endErrDlg=0;
}


//======================================================================================================


KNSendErrorDialog::KNSendErrorDialog()
  : KDialog(knGlobals.topWidget, i18n("Errors While Sending"), Close )
{
  KVBox *page = new KVBox( this );
  setMainWidget( page );
  setDefaultButton( Close );

  new QLabel(QString("<b>%1</b><br>%2").arg(i18n("Errors occurred while sending these articles:"))
                                       .arg(i18n("The unsent articles are stored in the \"Outbox\" folder.")), page);
  mErrorList = new QListWidget( page );
  mError = new QLabel( QString(), page );
  mError->setWordWrap( true );

  connect( mErrorList, SIGNAL( currentRowChanged( int ) ), SLOT( slotHighlighted( int ) ) );

  KNHelper::restoreWindowSize("sendDlg", this, QSize(320,250));
}


KNSendErrorDialog::~KNSendErrorDialog()
{
  KNHelper::saveWindowSize("sendDlg", size());
}


void KNSendErrorDialog::append(const QString &subject, const QString &error)
{
  ErrorListItem *item = new ErrorListItem( subject, error );
  item->setIcon( UserIcon("snderr") );
  mErrorList->addItem( item );
  mErrorList->setCurrentItem( item );
}


void KNSendErrorDialog::slotHighlighted(int idx)
{
  ErrorListItem *item = static_cast<ErrorListItem*>( mErrorList->item( idx ) );
  if ( item ) {
    QString tmp = i18n("<b>Error message:</b><br>") + item->error();
    mError->setText( tmp );
  }
}

//-------------------------------
#include "knarticlefactory.moc"
