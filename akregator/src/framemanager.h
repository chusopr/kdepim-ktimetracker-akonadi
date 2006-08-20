/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>

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

#ifndef AKREGATOR_FRAMEMANAGER_H
#define AKREGATOR_FRAMEMANAGER_H

#include <QHash>
#include <QObject>

class QString;

class KUrl;

namespace KParts
{
    class URLArgs;
}

namespace Akregator {

class Frame;
class OpenURLRequest;
    
class FrameManager : public QObject
{
    Q_OBJECT

    public:

        FrameManager(QWidget* mainWin=0, QObject* parent=0);
        virtual ~FrameManager();

        Frame* currentFrame() const;

        void addFrame(Frame* frame);
        void removeFrame(Frame* frame);
       
        void setMainWindow(QWidget* mainWin);

    public slots:

        void slotChangeFrame(Frame* frame);
        void slotOpenURLRequest(OpenURLRequest& request);

        void slotBrowserBack();
        void slotBrowserForward();
        void slotBrowserReload();
        void slotBrowserStop();
        void slotBrowserBackAboutToShow();
        void slotBrowserForwardAboutToShow();
        
    signals:

        void signalFrameAdded(Frame*);
        void signalFrameRemoved(Frame*);

        void signalRequestNewFrame(int& id);
        
        // TODO: merge signals
        /** emitted when the active frame is switched */
        void signalCurrentFrameChanged(Frame*);

        /** 
         * emitted when the active frame is switched
         * @param f1 the the deactivated frame
         * @param f2 the activated frame
         */
        void signalCurrentFrameChanged(Frame* f1, Frame* f2);

        void signalStarted();
        void signalCanceled(const QString&);
        void signalCompleted();
        void signalCaptionChanged(const QString&);
        void signalTitleChanged(const QString&);
        void signalLoadingProgress(int);
        void signalStatusText(const QString&);

    protected:
        
        void openURL(OpenURLRequest& request);
        void openInExternalBrowser(const OpenURLRequest& request);
        
    protected slots:

        void slotSetStarted(Frame* frame);
        void slotSetCanceled(Frame* frame, const QString& reason);
        void slotSetCompleted(Frame* frame);
        void slotSetProgress(Frame* frame, int progress);
        void slotSetCaption(Frame* frame, const QString& caption);
        void slotSetTitle(Frame* frame, const QString& title);
        void slotSetStatusText(Frame* frame, const QString& statusText);

        void slotCanGoBackToggled(Frame*, bool);
        void slotCanGoForwardToggled(Frame*, bool);
        void slotIsReloadableToggled(Frame*, bool);
        void slotIsLoadingToggled(Frame*, bool);

        void slotFoundMimeType(const OpenURLRequest& request);

    private:

        QWidget* m_mainWin;
        Frame* m_currentFrame;
        QHash<int, Frame*> m_frames;
};

} // namespace Akregator

#endif // AKREGATOR_FRAMEMANAGER_H
