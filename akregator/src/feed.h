/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
#ifndef AKREGATORFEED_H
#define AKREGATORFEED_H

#include "feedgroup.h"

#include <rss/librss.h>

using namespace RSS;

namespace Akregator
{
    class FeedsCollection;

    // use opml attributes:
    // -STANDARD
    // type="akrss" // since it is extension of "rss"
    // version="RSS"
    // text="titleOfFeed"
    // xmlUrl="urlOfRSSFeed"
    // -ADDITIONAL
    // htmlUrl="urlOfWebPageForThisFeed(AbilonCompatible)" // probably fetched from RSS document
    // description="verboseDescriptionOfTheFeed" ??
    // -SUPPLEMENTAL (aKregator-specific)
    // isLiveJournal="true|false"
    // ljUserName="madfire"
    // ljAuthMode="none|local|global"
    // ljLogin="login"
    // ljPassword="password? or hpassword?"
    // updateTitle="[x] Use feed name from RSS", when true causes feed to update text attribute

    class Feed : public FeedGroup
    {
        Q_OBJECT
        public:
            Feed(QListViewItem *i, FeedsCollection *coll);
            ~Feed();

            void updateView();
            void destroy();

            //virtual void open(QTextStream &ts);?
            virtual void save(QTextStream &ts, int depth = 0);

            virtual bool isGroup() { return false; }

            enum LJAuthMode { AuthNone, AuthLocal, AuthGlobal };

            QString ljAuthModeStr();
            static LJAuthMode authModeFromString(const QString &mode);

            // -- ATTRIBUTES
            QString        title;         ///< Feed title
            QString        xmlUrl;        ///< URL of RSS feed itself
            QString        htmlUrl;       ///< URL of HTML page for this feed
            QString        description;   ///< Verbose feed description.
            bool           isLiveJournal; ///< Is this a LiveJournal feed?
            QString        ljUserName;    ///< Name of LJ user whose feed we are fetching.
            LJAuthMode     ljAuthMode;
            QString        ljLogin;       ///< LiveJournal username
            QString        ljPassword;    ///< LiveJournal password md5 digest
            bool           updateTitle;   ///< Whether to update feed title based on fetched rss.
            Article::List  articles;      ///< List of just fetched feed articles (will be merged with archive?)

            void fetch();                 ///< Start fetching rss

            QListViewItem *item() { return m_item; }

        signals:
            void fetched(Feed *);         ///< Emitted when feed finishes fetching

        private slots:
            void fetchCompleted(Loader *loader, Document doc, Status status);

        private:
            QListViewItem *m_item;         ///< Corresponding list view item.
            FeedsCollection *m_collection; ///< Parent collection.

            // TODO
            //Archived articles
            //QValueList<Article> m_archive;
            //void saveArchive(QTextStream &ts);
    };
}

#endif
