/*
    knarticlemanager.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/


#include <pthread.h>

#include <qheader.h>

#include <mimelib/string.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <krun.h>
#include <kuserprofile.h>
#include <kopenwith.h>
#include <klocale.h>
#include <kdebug.h>
#include <kwin.h>
#include <kcharsets.h>
#include <ktempfile.h>

#include "knode.h"
#include "knglobals.h"
#include "knconfigmanager.h"
#include "utilities.h"
#include "knarticlemanager.h"
#include "kngroupmanager.h"
#include "knodeview.h"
#include "knarticlewidget.h"
#include "knsearchdialog.h"
#include "knlistview.h"
#include "knfiltermanager.h"
#include "kngroup.h"
#include "knfolder.h"
#include "knarticlefilter.h"
#include "knhdrviewitem.h"
#include "knnetaccess.h"
#include "knpgp.h"
#include "knnntpaccount.h"
#include "knscoring.h"
#include "knmemorymanager.h"
#include "knarticlefactory.h"
#include "knarticlewindow.h"
#include "knfoldermanager.h"


KNArticleManager::KNArticleManager(KNListView *v, KNFilterManager *f) : QObject(0,0)
{
  v_iew=v;
  g_roup=0;
  f_older=0;
  f_ilter=f->currentFilter();
  f_ilterMgr=f;
  s_earchDlg=0;
  s_howThreads=true;

  connect(v, SIGNAL(expanded(QListViewItem*)), this,
    SLOT(slotItemExpanded(QListViewItem*)));
  connect(f, SIGNAL(filterChanged(KNArticleFilter*)), this,
    SLOT(slotFilterChanged(KNArticleFilter*)));
}


KNArticleManager::~KNArticleManager()
{
  delete s_earchDlg;
}


void KNArticleManager::deleteTempFiles()
{
  KTempFile *file;

  while ((file = t_empFiles.first())) {
    file->unlink();                 // deletes file
    t_empFiles.removeFirst();
    delete file;
  }
}


void KNArticleManager::saveContentToFile(KNMimeContent *c, QWidget *parent)
{
  KNSaveHelper helper(c->contentType()->name(),parent);

  QFile *file = helper.getFile(i18n("Save Attachment"));

  if (file) {
    QByteArray data=c->decodedContent();
    file->writeBlock(data.data(), data.size());
  }
}


void KNArticleManager::saveArticleToFile(KNArticle *a, QWidget *parent)
{
  QString fName = a->subject()->asUnicodeString();
  fName.replace(QRegExp("[\\s/:]"),"_");
  KNSaveHelper helper(fName,parent);
  QFile *file = helper.getFile(i18n("Save Article"));

  if (file) {
    QCString tmp=a->encodedContent(false);
    file->writeBlock(tmp.data(), tmp.size());
  }
}


QString KNArticleManager::saveContentToTemp(KNMimeContent *c)
{
  QString path;
  KTempFile* tmpFile;
  KNHeaders::Base *pathHdr=c->getHeaderByType("X-KNode-Tempfile");  // check for existing temp file

  if(pathHdr) {
    path = pathHdr->asUnicodeString();
    bool found=false;

    // lets see if the tempfile-path is still valid...
    for (tmpFile=t_empFiles.first(); tmpFile; tmpFile=t_empFiles.next())
      if (tmpFile->name()==path) {
        found = true;
        break;
      }

    if (found)
      return path;
    else
      c->removeHeader("X-KNode-Tempfile");
  }

  tmpFile=new KTempFile();
  if (tmpFile->status()!=0) {
    KNHelper::displayTempFileError();
    delete tmpFile;
    return QString::null;
  }

  t_empFiles.append(tmpFile);
  QFile *f=tmpFile->file();
  QByteArray data=c->decodedContent();
  f->writeBlock(data.data(), data.size());
  tmpFile->close();
  path=tmpFile->name();
  pathHdr=new KNHeaders::Generic("X-KNode-Tempfile", c, path, "UTF-8");
  c->setHeader(pathHdr);

  return path;
}


void KNArticleManager::openContent(KNMimeContent *c)
{
  QString path=saveContentToTemp(c);
  if(path.isNull()) return;

  KService::Ptr offer = KServiceTypeProfile::preferredService(c->contentType()->mimeType(), true);
  KURL::List lst;
  KURL url;
  url.setPath(path);
  lst.append(url);

  if (offer)
    KRun::run(*offer, lst);
  else {
    KFileOpenWithHandler *openhandler = new KFileOpenWithHandler();
    openhandler->displayOpenWithDialog(lst);
  }
}


void KNArticleManager::showHdrs(bool clear)
{
  if(!g_roup && !f_older) return;

  bool setFirstChild=true;

  if(clear)
    v_iew->clear();

  knGlobals.top->setCursorBusy(true);
  knGlobals.top->setStatusMsg(i18n(" Creating list ..."));
  knGlobals.top->secureProcessEvents();

  if(g_roup) {
    KNRemoteArticle *art, *ref, *current;

    current=static_cast<KNRemoteArticle*>(knGlobals.view->articleView()->article());

    if(current && !current->listItem()) {
      current=0;
      knGlobals.view->articleView()->setArticle(0);
    }

    if(g_roup->isLocked()) {
      if (0!=pthread_mutex_lock(knGlobals.netAccess->nntpMutex())) {
        kdDebug(5003) << "failed to lock nntp mutex" << endl;
        knGlobals.top->setStatusMsg(QString::null);
        updateStatusString();
        knGlobals.top->setCursorBusy(false);
        return;
      }
    }

    if(f_ilter)
      f_ilter->doFilter(g_roup);
    else
      for(int i=0; i<g_roup->length(); i++) {
        art=g_roup->at(i);
        art->setFilterResult(true);
        art->setFiltered(true);
        ref=(art->idRef()!=0) ? g_roup->byId(art->idRef()) : 0;
        art->setDisplayedReference(ref);
        if(ref)
          ref->setVisibleFollowUps(true);
      }

    for(int i=0; i<g_roup->length(); i++) {

      art=g_roup->at(i);
      art->setThreadMode(s_howThreads);

      if(s_howThreads) {

        if( !art->listItem() && art->filterResult() ) {

          if( (ref=art->displayedReference()) ) {
            if( ref->listItem() && ( ref->listItem()->isOpen() || ref->listItem()->childCount()>0 ) ) {
              art->setListItem(new KNHdrViewItem(ref->listItem()));
              art->initListItem();
            }
          }
          else {
            art->setListItem(new KNHdrViewItem(v_iew));
            art->initListItem();
          }

        }
        else if(art->listItem())
          art->updateListItem();

      }
      else {

        if(!art->listItem() && art->filterResult()) {
          art->setListItem(new KNHdrViewItem(v_iew));
          art->initListItem();
        }

        else if(art->listItem())
          art->updateListItem();

      }

    }

    if(current && current->filterResult()) {
      if(!current->listItem())
        createThread(current);
      v_iew->setActive(current->listItem(),true);
      setFirstChild=false;
    }

    if (g_roup->isLocked() && (0!=pthread_mutex_unlock(knGlobals.netAccess->nntpMutex())))
      kdDebug(5003) << "failed to unlock nntp mutex" << endl;
  }

  else { //folder

    KNLocalArticle *art;
    for(int idx=0; idx<f_older->length(); idx++) {
      art=f_older->at(idx);
      if(!art->listItem()) {
        art->setListItem( new KNHdrViewItem(v_iew, art) );
        art->updateListItem();
      }
    }

  }

  if(setFirstChild && v_iew->firstChild()) {
    v_iew->setCurrentItem(v_iew->firstChild());
    knGlobals.view->articleView()->setArticle(0);
  }

  knGlobals.top->setStatusMsg(QString::null);
  updateStatusString();
  knGlobals.top->setCursorBusy(false);
}


void KNArticleManager::updateViewForCollection(KNArticleCollection *c)
{
  if(g_roup==c || f_older==c)
    showHdrs(false);
}


void KNArticleManager::updateListViewItems()
{
  if(!g_roup && !f_older) return;

  if(g_roup) {
    KNRemoteArticle *art;

    for(int i=0; i<g_roup->length(); i++) {
      art=g_roup->at(i);
      if(art->listItem())
        art->updateListItem();
    }
  } else { //folder
    KNLocalArticle *art;

    for(int idx=0; idx<f_older->length(); idx++) {
      art=f_older->at(idx);
      if(art->listItem())
        art->updateListItem();
    }
  }
}


void KNArticleManager::setAllThreadsOpen(bool b)
{
  if(g_roup) {
    knGlobals.top->setCursorBusy(true);
    for(int idx=0; idx<g_roup->length(); idx++)
      if(g_roup->at(idx)->listItem())
        g_roup->at(idx)->listItem()->QListViewItem::setOpen(b);
    knGlobals.top->setCursorBusy(false);
  }
}


void KNArticleManager::search()
{
  if(!g_roup) return;
  if(s_earchDlg) {
    s_earchDlg->show();
    KWin::setActiveWindow(s_earchDlg->winId());
  } else {
    s_earchDlg=new KNSearchDialog(KNSearchDialog::STgroupSearch, 0);
    connect(s_earchDlg, SIGNAL(doSearch(KNArticleFilter*)), this,
      SLOT(slotFilterChanged(KNArticleFilter*)));
    connect(s_earchDlg, SIGNAL(dialogDone()), this,
      SLOT(slotSearchDialogDone()));
    s_earchDlg->show();
  }
}


void KNArticleManager::setGroup(KNGroup *g)
{
  g_roup=g;

  if(g) {
    v_iew->header()->setLabel(1, i18n("From"));
  }
}


void KNArticleManager::setFolder(KNFolder *f)
{
  f_older=f;
  if(f)
    v_iew->header()->setLabel(1, i18n("Newsgroups / To"));
}


KNArticleCollection* KNArticleManager::collection()
{
  if(g_roup)
    return g_roup;
  if(f_older)
   return f_older;

  return 0;
}


void KNArticleManager::verifyPGPSignature(KNArticle* a)
{
  //create a PGP object and check if the posting is signed
  Kpgp *pgp = Kpgp::getKpgp();
  pgp->setMessage(a->body());
  if (!pgp->isSigned()) {
    KMessageBox::sorry(knGlobals.topWidget,i18n("Cannot find a signature in this message!"));
    return;
  }
  kdDebug(5003) << "KNArticleFactory::createVerify() found signed article, check it now" << endl;
  if (pgp->goodSignature()) {
    QString signer = pgp->signedBy();
    QString key = pgp->signedByKey();
    KMessageBox::information(knGlobals.topWidget,i18n("The signature is valid.\nThe message was signed by %1.").arg(signer));
  }
  else {
    KMessageBox::error(knGlobals.topWidget,i18n("This signature is invalid!"));
  }
}


bool KNArticleManager::loadArticle(KNArticle *a)
{
  if (!a)
    return false;

  if (a->hasContent())
    return true;

  if (a->isLocked()) {
    if (a->type()==KNMimeBase::ATremote)
      return true;   // locked == we are already loading this article...
    else
      return false;
  }

  if(a->type()==KNMimeBase::ATremote) {
    KNGroup *g=static_cast<KNGroup*>(a->collection());
    if(g)
      emitJob( new KNJobData(KNJobData::JTfetchArticle, this, g->account(), a) );
    else
      return false;
  }
  else { // local article
    KNFolder *f=static_cast<KNFolder*>(a->collection());
   if( f && f->loadArticle( static_cast<KNLocalArticle*>(a) ) )
      knGlobals.memManager->updateCacheEntry(a);
    else
      return false;
  }
  return true;
}


bool KNArticleManager::unloadArticle(KNArticle *a, bool force)
{
  if(!a || a->isLocked() )
    return false;
  if(!a->hasContent())
    return true;

  if (!force && a->isNotUnloadable())
    return false;

  if (!force && KNArticleWidget::articleVisible(a))
    return false;

  if (!force && (a->type()==KNMimeBase::ATlocal) &&
      (knGlobals.artFactory->findComposer(static_cast<KNLocalArticle*>(a))!=0))
    return false;

  if (!KNArticleWindow::closeAllWindowsForArticle(a, force))
    if (!force)
      return false;

  KNArticleWidget::articleRemoved(a);
  if (!a->type()==KNMimeBase::ATlocal)
    knGlobals.artFactory->deleteComposerForArticle(static_cast<KNLocalArticle*>(a));
  a->KNMimeContent::clear();
  a->updateListItem();
  knGlobals.memManager->removeCacheEntry(a);

  return true;
}


void KNArticleManager::copyIntoFolder(KNArticle::List &l, KNFolder *f)
{
  if(!f) return;

  KNArticle *org;
  KNLocalArticle *loc;
  KNLocalArticle::List l2;

  for(org=l.first(); org; org=l.next()) {
    if(!org->hasContent())
      continue;
    loc=new KNLocalArticle(0);
    loc->setEditDisabled(true);
    loc->setContent(org->encodedContent());
    loc->parse();
    l2.append(loc);
  }

  if(!l2.isEmpty()) {

    f->setNotUnloadable(true);

    if (!f->isLoaded() && !knGlobals.folManager->loadHeaders(f)) {
      l2.setAutoDelete(true);
      l2.clear();
      f->setNotUnloadable(false);
      return;
    }

    if(!f->saveArticles(&l2)) {
      for(KNLocalArticle *a=l2.first(); a; a=l2.next()) {
        if(a->isOrphant())
          delete a; // ok, this is ugly; we simply delete orphant articles
        else
          a->KNMimeContent::clear(); // no need to keep them in memory
      }
      KNHelper::displayInternalFileError();
    } else {
      for(KNLocalArticle *a=l2.first(); a; a=l2.next())
        a->KNMimeContent::clear(); // no need to keep them in memory
      knGlobals.memManager->updateCacheEntry(f);
    }

    f->setNotUnloadable(false);
  }
}


void KNArticleManager::moveIntoFolder(KNLocalArticle::List &l, KNFolder *f)
{
  if(!f) return;

  f->setNotUnloadable(true);

  if (!f->isLoaded() && !knGlobals.folManager->loadHeaders(f)) {
    f->setNotUnloadable(false);
    return;
  }

  if(f->saveArticles(&l)) {
    for(KNLocalArticle *a=l.first(); a; a=l.next())
      knGlobals.memManager->updateCacheEntry( a );
    knGlobals.memManager->updateCacheEntry(f);
  } else {
    for(KNLocalArticle *a=l.first(); a; a=l.next())
      if(a->isOrphant())
        delete a; // ok, this is ugly; we simply delete orphant articles
    KNHelper::displayInternalFileError();
  }

  f->setNotUnloadable(false);
}


bool KNArticleManager::deleteArticles(KNLocalArticle::List &l, bool ask)
{
  if(ask) {
    QStringList lst;
    for(KNLocalArticle *a=l.first(); a; a=l.next()) {
      if(a->isLocked())
        continue;
      if(a->subject()->isEmpty())
        lst << i18n("no subject");
      else
        lst << a->subject()->asUnicodeString();
    }
    if( KMessageBox::No == KMessageBox::questionYesNoList(
      knGlobals.topWidget, i18n("Do you really want to delete these articles?"), lst) )
      return false;
  }

  for(KNLocalArticle *a=l.first(); a; a=l.next())
    knGlobals.memManager->removeCacheEntry(a);

  KNFolder *f=static_cast<KNFolder*>(l.first()->collection());
  if(f) {
    f->removeArticles(&l, true);
    knGlobals.memManager->updateCacheEntry( f );
  }
  else {
    for(KNLocalArticle *a=l.first(); a; a=l.next())
      delete a;
  }

  return true;
}


void KNArticleManager::setAllRead(bool r)
{
  if(!g_roup)
    return;

  int new_count = 0;
  KNRemoteArticle *a;
  for(int i=0; i<g_roup->length(); i++) {
    a=g_roup->at(i);
    if(a->isRead()!=r) {
      a->setRead(r);
      a->setChanged(true);
      if(a->isNew())
        new_count++;
    }
  }

  g_roup->updateThreadInfo();
  if(r) {
    g_roup->setReadCount(g_roup->length());
    g_roup->setNewCount(0);
  } else {
    g_roup->setReadCount(0);
    g_roup->setNewCount(new_count);
  }

  g_roup->updateListItem();
  showHdrs(true);
}


void KNArticleManager::setRead(KNRemoteArticle::List &l, bool r, bool handleXPosts)
{
  if(l.isEmpty())
    return;

  KNRemoteArticle *a=l.first(), *ref=0;
  KNGroup *g=static_cast<KNGroup*>(a->collection() );
  int changeCnt=0, idRef=0;

  for( ; a; a=l.next()) {
    if( r && knGlobals.cfgManager->readNewsGeneral()->markCrossposts() &&
        handleXPosts && a->newsgroups()->isCrossposted() ) {

      QStringList groups = a->newsgroups()->getGroups();
      KNGroup *targetGroup=0;
      KNRemoteArticle *xp=0;
      KNRemoteArticle::List al;
      QCString mid=a->messageID()->as7BitString(false);

      for (QStringList::Iterator it = groups.begin(); it != groups.end(); ++it) {
        targetGroup = knGlobals.grpManager->group(*it, g->account());
        if (targetGroup) {
          if (targetGroup->isLoaded() && (xp=targetGroup->byMessageId(mid)) ) {
            al.clear();
            al.append(xp);
            setRead(al, r, false);
          } else {
            targetGroup->appendXPostID(mid);
          }
        }
      }
    }

    else if(a->isRead()!=r) {
      changeCnt++;
      a->setRead(r);
      a->setChanged(true);
      a->updateListItem();

      idRef=a->idRef();

      while(idRef!=0) {
        ref=g->byId(idRef);
        if(r) {
          ref->decUnreadFollowUps();
          if(a->isNew())
            ref->decNewFollowUps();
        }
        else {
          ref->incUnreadFollowUps();
          if(a->isNew())
            ref->incNewFollowUps();
        }

        if(ref->listItem() &&
           ((ref->unreadFollowUps()==0 || ref->unreadFollowUps()==1) ||
            (ref->newFollowUps()==0 || ref->newFollowUps()==1)))
          ref->updateListItem();

        idRef=ref->idRef();
      }

      if(r) {
        g->incReadCount();
        if(a->isNew())
          g->decNewCount();
      }
      else {
        g->decReadCount();
        if(a->isNew())
          g->incNewCount();
      }
    }
  }

  if(changeCnt>0) {
    g->updateListItem();
    if(g==g_roup)
      updateStatusString();
  }
}



void KNArticleManager::toggleWatched(KNRemoteArticle::List &l)
{
  KNRemoteArticle *a=l.first();
  bool watch=true;
  if (a && a->isWatched())
    watch=false;

  for(KNRemoteArticle *a=l.first(); a; a=l.next()) {
    a->setIgnored(false);
    a->setWatched(watch);
    a->updateListItem();
    a->setChanged(true);
  }
}


void KNArticleManager::toggleIgnored(KNRemoteArticle::List &l)
{
  KNRemoteArticle *a=l.first();
  bool ignore=true;
  if (a && a->isIgnored())
    ignore=false;

  for(; a; a=l.next()) {
    a->setWatched(false);
    a->setIgnored(ignore);
    a->updateListItem();
    a->setChanged(true);
  }
}


void  KNArticleManager::rescoreArticles(KNRemoteArticle::List &l)
{
  KNRemoteArticle *a=l.first();

  if (a) {
    KNGroup *g=static_cast<KNGroup*>(a->collection());
    KScoringManager *sm = knGlobals.scoreManager;
    sm->initCache(g->groupname());

    for(; a; a=l.next()) {
      int defScore = 0;
      if (a->isIgnored())
        defScore = knGlobals.cfgManager->scoring()->ignoredThreshold();
      else if (a->isWatched())
        defScore = knGlobals.cfgManager->scoring()->watchedThreshold();
      a->setScore(defScore);

      KNScorableArticle sa(a);
      sm->applyRules(sa);
      a->updateListItem();
      a->setChanged(true);
    }
  }
}


void KNArticleManager::processJob(KNJobData *j)
{
  if(j->type()==KNJobData::JTfetchArticle && !j->canceled()) {
    if(j->success()) {
      KNRemoteArticle *a=static_cast<KNRemoteArticle*>(j->data());
      KNArticleWidget::articleChanged(a);
      if(!a->isOrphant()) //orphant articles are deleted by the displaying widget
        knGlobals.memManager->updateCacheEntry(a);
      if(a->listItem())
        a->updateListItem();
    }
    else
      KNArticleWidget::articleLoadError(static_cast<KNRemoteArticle*>(j->data()), j->errorString());
  }

  delete j;
}


void KNArticleManager::createHdrItem(KNRemoteArticle *a)
{
  a->setListItem(new KNHdrViewItem(v_iew));
  a->setThreadMode(s_howThreads);
  a->initListItem();
}


void KNArticleManager::createThread(KNRemoteArticle *a)
{
  KNRemoteArticle *ref=a->displayedReference();

  if(ref) {
    if(!ref->listItem())
      createThread(ref);
    a->setListItem(new KNHdrViewItem(ref->listItem()));
  }
  else
    a->setListItem(new KNHdrViewItem(v_iew));

  a->setThreadMode(s_howThreads);
  a->initListItem();
}


void KNArticleManager::updateStatusString()
{
  int displCnt=0;

  if(g_roup) {
    if(f_ilter)
      displCnt=f_ilter->count();
    else
      displCnt=g_roup->count();

    QString name = g_roup->name();
    if (g_roup->status()==KNGroup::moderated)
      name += i18n(" (moderated)");

    knGlobals.top->setStatusMsg(i18n(" %1: %2 new , %3 displayed")
                        .arg(name).arg(g_roup->newCount()).arg(displCnt),SB_GROUP);

    if(f_ilter)
      knGlobals.top->setStatusMsg(i18n(" Filter: %1").arg(f_ilter->translatedName()), SB_FILTER);
    else
      knGlobals.top->setStatusMsg(QString::null, SB_FILTER);
  }
  else if(f_older) {
    knGlobals.top->setStatusMsg(i18n(" %1: %2 displayed")
      .arg(f_older->name()).arg(f_older->count()), SB_GROUP);
    knGlobals.top->setStatusMsg(QString::null, SB_FILTER);
  } else {
    knGlobals.top->setStatusMsg(QString::null, SB_GROUP);
    knGlobals.top->setStatusMsg(QString::null, SB_FILTER);
  }
}


void KNArticleManager::slotFilterChanged(KNArticleFilter *f)
{
  f_ilter=f;
  showHdrs();
}


void KNArticleManager::slotSearchDialogDone()
{
  s_earchDlg->hide();
  slotFilterChanged(f_ilterMgr->currentFilter());
}


void KNArticleManager::slotItemExpanded(QListViewItem *p)
{
  KNRemoteArticle *top, *art, *ref;
  KNHdrViewItem *hdrItem;
  bool inThread=false;
  KNConfig::ReadNewsGeneral *rng=knGlobals.cfgManager->readNewsGeneral();

  if(p->childCount() > 0) {
    //kdDebug(5003) << "KNFetchArticleManager::slotItemExpanded() : childCount = " << p->childCount() << " => returning" << endl;
    return;
  }

  knGlobals.top->setCursorBusy(true);

  hdrItem=static_cast<KNHdrViewItem*>(p);
  top=static_cast<KNRemoteArticle*>(hdrItem->art);

  for(int i=0; i<g_roup->count(); i++) {
    art=g_roup->at(i);
    if(art->filterResult() && !art->listItem()) {

      if(art->displayedReference()==top) {
        art->setListItem(new KNHdrViewItem(hdrItem));
        art->setThreadMode(s_howThreads);
        art->initListItem();
      }
      else if(rng->totalExpandThreads()) { //totalExpand
        ref=art->displayedReference();
        inThread=false;
        while(ref && !inThread) {
          inThread=(ref==top);
          ref=ref->displayedReference();
        }
        if(inThread)
          createThread(art);
      }
    }
  }

  if(rng->totalExpandThreads())
    hdrItem->expandChildren();

  knGlobals.top->setCursorBusy(false);
}

//-----------------------------
#include "knarticlemanager.moc"
