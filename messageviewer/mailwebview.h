/* Copyright 2010 Thomas McGuire <mcguire@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef Q_OS_WINCE
#include <QWebView>
#else
#include <KWebView>
#endif

class QContextMenuEventF;

namespace MessageViewer {

/// MailWebView extends KWebView so that it can emit the popupMenu() signal
#ifdef Q_OS_WINCE
class MailWebView : public QWebView
#else
class MailWebView : public KWebView
#endif
{
  Q_OBJECT
  public:

    MailWebView( QWidget *parent );

  signals:

    /// Emitted when the user right-clicks somewhere
    /// @param url if an URL was under the cursor, this parameter contains it. Otherwise empty
    /// @param point position where the click happened, in local coordinates
    void popupMenu( const QString &url, const QPoint &point );

  protected:
#ifdef KDEPIM_MOBILE_UI
    friend class MessageViewItem;
#endif
    /// Reimplemented to catch context menu events and emit popupMenu()
    virtual bool event( QEvent *event );
};

}
