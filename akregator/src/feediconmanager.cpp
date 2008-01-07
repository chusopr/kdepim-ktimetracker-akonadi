/*
    This file is part of Akregator.

    Copyright (C) 2004 Sashmit Bhaduri <smt@vfemail.net>
                  2005 Frank Osterfeld <frank.osterfeld@kdemail.net>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "feed.h"
#include "feediconmanager.h"

#include <kapplication.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <k3staticdeleter.h>
#include <kurl.h>

#include <QList>
#include <QMultiHash>
#include <QPixmap>
#include <QtDBus/QtDBus>
#define FAVICONINTERFACE "org.kde.FavIcon"


namespace Akregator {

class FeedIconManager::FeedIconManagerPrivate
{
    public:
    QList<Feed*> registeredFeeds;
    QMultiHash<QString, Feed*> urlDict;
  QDBusInterface *m_favIconsModule;
};

FeedIconManager *FeedIconManager::m_instance = 0;

static K3StaticDeleter<FeedIconManager> feediconmanagersd;

FeedIconManager* FeedIconManager::self()
{
    if (!m_instance)
        m_instance = feediconmanagersd.setObject(m_instance, new FeedIconManager);
    return m_instance;
}

void FeedIconManager::fetchIcon(Feed* feed)
{
    if (!d->registeredFeeds.contains(feed))
    {
        d->registeredFeeds.append(feed);
        connect(feed, SIGNAL(signalDestroyed(Akregator::TreeNode*)), this, SLOT(slotFeedDestroyed(Akregator::TreeNode*)));
    }
    const QString iconUrl = getIconUrl(KUrl(feed->xmlUrl()));
    d->urlDict.insert(iconUrl, feed);
    loadIcon(iconUrl);
}

FeedIconManager::FeedIconManager()
:  QObject(), d(new FeedIconManagerPrivate)
{
   QDBusConnection::sessionBus().registerObject("/FeedIconManager", this, QDBusConnection::ExportScriptableSlots);
   d->m_favIconsModule =
     new QDBusInterface("org.kde.kded", "/modules/favicons", FAVICONINTERFACE);
   Q_ASSERT( d->m_favIconsModule );
   connect( d->m_favIconsModule, SIGNAL(iconChanged(bool,QString,QString)),
           this, SLOT(slotIconChanged(bool,QString,QString)));
}


FeedIconManager::~FeedIconManager()
{
    delete d->m_favIconsModule;
    delete d;
    d = 0;
}

void FeedIconManager::loadIcon(const QString & url)
{
    KUrl u(url);

    QString iconFile = iconLocation(u);

    if (iconFile.isNull())
    {
        d->m_favIconsModule->call( "downloadHostIcon", u.url() );
    }
    else
        slotIconChanged(false, url, iconFile);

}

QString FeedIconManager::getIconUrl(const KUrl& url)
{
    return "http://" + url.host() + '/';
}

QString FeedIconManager::iconLocation(const KUrl & url) const
{
    QDBusReply<QString> reply = d->m_favIconsModule->call( "iconForUrl", url.url() );

    if (reply.isValid()) {
      QString result = reply;
      return result;
    }
    return QString();
}

void FeedIconManager::slotFeedDestroyed(TreeNode* node)
{
    Feed* feed = dynamic_cast<Feed*>(node);
    if (feed)
        d->registeredFeeds.removeAll(feed);
}

void FeedIconManager::slotIconChanged(bool /*isHost*/, const QString& hostOrURL,
                                  const QString& iconName)
{
    QString iconFile = KGlobal::dirs()->findResource("cache",
                                 iconName+".png");
    Feed* f;
    QPixmap p = QPixmap(iconFile);
    if (!p.isNull()) // we don't set null pixmaps, as feed checks pixmap.isNull() to find out whether the icon was already loaded or not. It would request the icon another time, resulting an infinite loop (until stack overflow that is
    {
        while (( f = d->urlDict.take(hostOrURL) ))
            if (d->registeredFeeds.contains(f))
                f->setFavicon(p);
    }
    emit signalIconChanged(hostOrURL, iconFile);
}

} // namespace Akregator

#include "feediconmanager.moc"
