/*
    This file is part of Akregator.

    Copyright (C) 2004 Sashmit Bhaduri <smt@vfemail.net>
                  2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

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

#ifndef AKREGATOR_FEEDICONMANAGER_H
#define AKREGATOR_FEEDICONMANAGER_H


#include <QObject>

class KUrl;

class QPixmap;
class QString;

namespace Akregator {

class Feed;
class TreeNode;

class FeedIconManager:public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.akregator.feediconmanager")
    
    public:


        ~FeedIconManager();
        
        static FeedIconManager *self();

        void fetchIcon(Feed* feed);
        
        QString iconLocation(const KUrl &) const;
        
    public Q_SLOTS:
        Q_SCRIPTABLE void slotIconChanged(bool, const QString&, const QString&);

    signals:
        void signalIconChanged(const QString &, const QPixmap &);

    public slots:
        void slotFeedDestroyed(TreeNode* node);

    protected:

        /** returns the url used to access the icon, e.g.
            http://dot.kde.org/ for "dot.kde.org/1113317400/" */
        QString getIconURL(const KUrl& url);

        void loadIcon(const QString &);
    
    private:
        static FeedIconManager *m_instance;

        FeedIconManager();
	
        class FeedIconManagerPrivate;
        FeedIconManagerPrivate* d;
};

} // namespace Akregator

#endif // AKREGATOR_FEEDICONMANAGER_H
