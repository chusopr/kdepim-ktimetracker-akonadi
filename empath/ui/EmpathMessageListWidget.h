/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef __GNUG__
# pragma interface "EmpathMessageListWidget.h"
#endif

#ifndef EMPATHMESSAGELISTWIDGET_H
#define EMPATHMESSAGELISTWIDGET_H

// Qt includes
#include <qpixmap.h>
#include <qlist.h>
#include <qstring.h>
#include <qpopupmenu.h>
#include <qtimer.h>
#include <qobject.h>
#include <qdragobject.h>
#include <qpoint.h>

// Local includes
#include "EmpathListView.h"
#include "EmpathDefines.h"
#include "EmpathIndexRecord.h"
#include "EmpathMessageListItem.h"
#include "EmpathURL.h"

class QActionCollection;

class KAction;
class KToggleAction;

class EmpathFolder;
class EmpathMessageListWidget;
class EmpathMessageListItem;
class EmpathMainWindow;

class EmpathMarkAsReadTimer : public QObject
{
    Q_OBJECT
    
    public:
        
        EmpathMarkAsReadTimer(EmpathMessageListWidget * parent);
        ~EmpathMarkAsReadTimer();
        
        void go(EmpathMessageListItem *);
        void cancel();
        
    protected slots:

        void s_timeout();
        
    private:
        
        QTimer timer_;
        EmpathMessageListItem * item_;
        
        EmpathMessageListWidget * parent_;
};

/**
 * This is the widget that shows the threaded mail list.
 */
class EmpathMessageListWidget : public EmpathListView
{
    Q_OBJECT

    public:
    
        EmpathMessageListWidget(QWidget * parent = 0);
        
        virtual ~EmpathMessageListWidget();
        
        EmpathMessageListItem * find(RMM::RMessageID & msgId);
        EmpathMessageListItem * findRecursive(
                EmpathMessageListItem * initialItem, RMM::RMessageID & msgId);
        
        EmpathURL firstSelectedMessage();
        
        const EmpathURL & currentFolder() { return url_; }
        
        void selectTagged();
        void selectRead();
        void selectAll();
        void selectInvert();
        
        virtual void setSelected(QListViewItem *, bool);
        virtual void clearSelection();

        void setStatus(EmpathMessageListItem * item, RMM::MessageStatus s)
        { item->setStatus(s); }
 
        void listenTo(unsigned int);
        
        QActionCollection * actionCollection() { return actionCollection_; }

    public slots:

        void s_messageDelete();

    protected slots:
   
        void s_goPrevious();
        void s_goNext();
        void s_goNextUnread();
        
        void s_messageMark();
        void s_messageMarkRead();
        void s_messageMarkReplied();
        void s_messageMarkMany();
        void s_messageView();

        void s_messageReply();
        void s_messageReplyAll();
        void s_messageForward();
        void s_messageBounce();
        void s_messageSaveAs();
        void s_messageCopyTo();
        void s_messageMoveTo();
        void s_messagePrint();
        void s_messageFilter();

        void s_threadExpand();
        void s_threadCollapse();
        
        void s_rightButtonPressed   (QListViewItem *, const QPoint &, int, Area);
        void s_doubleClicked        (QListViewItem *);
        void s_linkChanged          (QListViewItem *);
        void s_startDrag            (const QList<QListViewItem> &);
        
        void s_showFolder           (const EmpathURL &, unsigned int);
        void s_headerClicked        (int);
        void s_itemGone             (const QString &);
        void s_itemCome             (const QString &);
    
        void s_hideRead();

        void s_updateActions        (QListViewItem *);

    signals:
        
        void changeView(const EmpathURL &);
        void hideReadChanged(bool);
        
    private:
       
        class ThreadNode {

            public:

                ThreadNode(EmpathIndexRecord * data)
                    :
                    data_(data)
                {
                    childList_.setAutoDelete(true);
                }

                ~ThreadNode()
                {
                    delete data_;
                    data_ = 0L;
                }

                EmpathIndexRecord * data()
                {
                    return data_;
                }
            
                const QList<ThreadNode> & childList() const
                {
                    return childList_;
                }

                void addChild(ThreadNode * n)
                {
                    childList_.append(n);
                }

            private:

                EmpathIndexRecord * data_;

                QList<ThreadNode> childList_; 
        };
         
        void _reconnectToFolder(const EmpathURL &);

        void _fillDisplay();
        void _fillNormal();
        void _fillThreading();
       
        void _createThreads(
            ThreadNode * root,
            EmpathTask * = 0L,
            EmpathMessageListItem * parent = 0L
        );
       
        EmpathMessageListItem * _createListItem(
            EmpathIndexRecord &,
            EmpathTask * = 0L,
            EmpathMessageListItem * = 0L
        );

        EmpathMessageListItem *
            _pool(EmpathIndexRecord &, EmpathMessageListItem * = 0L);
        
        void _clear();
        
        void _init();
        void _initActions();
        void _connectUp();
    
        void _restoreColumnSizes();
        void _saveColumnSizes();
        
        void _setupMessageMenu();
        void _setupThreadMenu();
        
        void _setSelected(EmpathMessageListItem *, bool);
        Q_UINT32 _nSelected();
        
        EmpathMessageListItem *
            _threadItem(EmpathIndexRecord & item);
        
        void _removeItem(EmpathMessageListItem *);
        
        void getDescendants(
            EmpathMessageListItem * initialItem,
            QList<EmpathMessageListItem> * itemList);

        void append(EmpathMessageListItem * item);

        QPopupMenu  messageMenu_;
        QPopupMenu  multipleMessageMenu_;
        QPopupMenu  messageMarkMenu_;
        QPopupMenu  threadMenu_;
 
        QActionCollection * actionCollection_;

        // Navigation actions
        KAction * ac_goPrevious_;
        KAction * ac_goNext_;
        KAction * ac_goNextUnread_;
        
        // Message related actions
        KToggleAction * ac_messageTag_;
        KToggleAction * ac_messageMarkRead_;
        KToggleAction * ac_messageMarkReplied_;
        
        KAction * ac_messageMarkMany_;
        
        KAction * ac_messageView_;
        KAction	* ac_messageReply_;
        KAction	* ac_messageReplyAll_;
        KAction	* ac_messageForward_;
        KAction	* ac_messageBounce_;
        KAction	* ac_messageDelete_;
        KAction	* ac_messageSaveAs_;
        KAction	* ac_messageCopyTo_;
        KAction	* ac_messageMoveTo_;
        KAction	* ac_messagePrint_;
        KAction	* ac_messageFilter_;

        // Thread related actions
        KAction * ac_threadExpand_;
        KAction * ac_threadCollapse_;
      
        QPixmap px_unread_, px_read_, px_marked_,
            px_attachments_, px_replied_;
        
        EmpathURL url_;

        int lastHeaderClicked_;
        bool sortType_; // Ascending, Descending
        
        int sortColumn_;
        bool sortAscending_;
        
        friend class EmpathMarkAsReadTimer;
        
        EmpathMarkAsReadTimer * markAsReadTimer_;
        
        void markAsRead(EmpathMessageListItem *);
        
        /**
         * Mark with the status given
         */
        void mark(RMM::MessageStatus);

        /**
         * Flip the flag(s) in the given status
         */
        void markOne(RMM::MessageStatus);
        
        static QListViewItem * lastSelected_;
        
        EmpathMessageListItemList selected_;
        
        unsigned int listenTo_;

        bool hideRead_;

        static QList<EmpathMessageListItem> * listItemPool_;
        
        // Order dependency
        bool                filling_;
        Q_UINT32            itemListCount_;
        // End order dependency
};

#endif

// vim:ts=4:sw=4:tw=78
