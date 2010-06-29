/*
    Copyright (c) 2007 Till Adam <adam@kde.org>
    Copyright (C) 2008 Omat Holding B.V. <info@omat.nl>
    Copyright (C) 2009 Kevin Ottens <ervin@kde.org>

    Copyright (c) 2010 Klarälvdalens Datakonsult AB,
                       a KDAB Group company <info@kdab.com>
    Author: Kevin Ottens <kevin@kdab.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "imapidlemanager.h"

#include <KDE/KDebug>

#include <kimap/idlejob.h>
#include <kimap/selectjob.h>
#include <kimap/session.h>

#include <QtCore/QTimer>

#include "imapresource.h"
#include "sessionpool.h"

ImapIdleManager::ImapIdleManager( Akonadi::Collection &col, const QString &mailBox,
                                  SessionPool *pool, ImapResource *parent )
  : QObject( parent ), m_sessionRequestId( 0 ), m_session( 0 ),
    m_idle( 0 ), m_resource( parent ),  m_mailBox( mailBox ), m_collection( col ),
    m_lastMessageCount( -1 ), m_lastRecentCount( -1 )
{
  connect( pool, SIGNAL(sessionRequestDone(qint64, KIMAP::Session*, int, QString)),
           this, SLOT(onSessionRequestDone(qint64, KIMAP::Session*, int, QString)) );
  m_sessionRequestId = pool->requestSession();
}

ImapIdleManager::~ImapIdleManager()
{
}

KIMAP::Session *ImapIdleManager::session() const
{
  return m_session;
}

void ImapIdleManager::onSessionRequestDone( qint64 requestId, KIMAP::Session *session,
                                            int /*errorCode*/, const QString &/*errorString*/ )
{
  if ( requestId!=m_sessionRequestId || session==0 ) {
    return;
  }

  m_session = session;
  m_sessionRequestId = 0;

  KIMAP::SelectJob *select = new KIMAP::SelectJob( m_session );
  select->setMailBox( m_mailBox );
  connect( select, SIGNAL(result(KJob*)),
           this, SLOT(onSelectDone(KJob*)) );
  select->start();

  m_idle = new KIMAP::IdleJob( m_session );
  connect( m_idle, SIGNAL(mailBoxStats(KIMAP::IdleJob*, QString, int, int)),
           this, SLOT(onStatsReceived(KIMAP::IdleJob*, QString, int, int)) );
  connect( m_idle, SIGNAL(result(KJob*)),
           this, SLOT(onIdleStopped()) );
  m_idle->start();
}

void ImapIdleManager::onSelectDone( KJob *job )
{
  KIMAP::SelectJob *select = static_cast<KIMAP::SelectJob*>( job );

  m_lastMessageCount = select->messageCount();
  m_lastRecentCount = select->recentCount();
}

void ImapIdleManager::onIdleStopped()
{
  kDebug(5327) << "IDLE dropped maybe we should reconnect?";
  if ( m_resource->isOnline() ) {
    kDebug(5327) << "Restarting the IDLE session!";
    QTimer::singleShot( 0, m_resource, SLOT( startIdle() ) );
  }
}

void ImapIdleManager::onStatsReceived(KIMAP::IdleJob *job, const QString &mailBox,
                                      int messageCount, int recentCount)
{
  kDebug(5327) << "IDLE stats received:" << job << mailBox << messageCount << recentCount;
  kDebug(5327) << "Cached information:" << m_collection.remoteId() << m_collection.id()
           << m_lastMessageCount << m_lastRecentCount;

  // It seems we're not in sync with the cache, resync is needed
  if ( messageCount!=m_lastMessageCount || recentCount!=m_lastRecentCount ) {
    m_lastMessageCount = messageCount;
    m_lastRecentCount = recentCount;

    kDebug(5327) << "Resync needed for" << mailBox << m_collection.id();
    m_resource->synchronizeCollection( m_collection.id() );
  }
}

#include "imapidlemanager.moc"


