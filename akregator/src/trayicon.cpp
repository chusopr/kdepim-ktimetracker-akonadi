/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

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

#include "akregatorconfig.h"
#include "trayicon.h"

#include <kapplication.h>
#include <kwin.h>
#include <kiconeffect.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kmenu.h>
#include <kiconloader.h>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QBitmap>
#include <QPainter>
#include <QFont>
#include <QToolTip>
#ifdef Q_WS_X11
#include <QX11Info>
#endif
//Added by qt3to4:
#include <QPixmap>


namespace Akregator {

TrayIcon* TrayIcon::m_instance = 0;

TrayIcon* TrayIcon::getInstance()
{
    return m_instance;
}

void TrayIcon::setInstance(TrayIcon* trayIcon)
{
    m_instance = trayIcon;
}


TrayIcon::TrayIcon(QWidget *parent)
        : KSystemTray(parent), m_unread(0)
{
    m_defaultIcon=KSystemTray::loadIcon("akregator");
    QPixmap m_unreadIcon=KSystemTray::loadIcon("akregator_empty");
    m_lightIconImage=m_unreadIcon.toImage();
    KIconEffect::deSaturate(m_lightIconImage, 0.60);
    setPixmap(m_defaultIcon);
    this->setToolTip( i18n("Akregator - RSS Feed Reader"));
}


TrayIcon::~TrayIcon()
{}


void TrayIcon::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) {
        emit showPart();
    }

    KSystemTray::mousePressEvent(e);
}


QPixmap TrayIcon::takeScreenshot() const
{
    QPoint g = mapToGlobal(pos());
    int desktopWidth  = kapp->desktop()->width();
    int desktopHeight = kapp->desktop()->height();
    int tw = width();
    int th = height();
    int w = desktopWidth / 4;
    int h = desktopHeight / 9;
    int x = g.x() + tw/2 - w/2; // Center the rectange in the systray icon
    int y = g.y() + th/2 - h/2;
    if (x < 0)
        x = 0; // Move the rectangle to stay in the desktop limits
    if (y < 0)
        y = 0;
    if (x + w > desktopWidth)
        x = desktopWidth - w;
    if (y + h > desktopHeight)
        y = desktopHeight - h;

        // Grab the desktop and draw a circle around the icon:
#ifdef Q_WS_X11
    QPixmap shot = QPixmap::grabWindow(QX11Info::appRootWindow(), x, y, w, h);
    QPainter painter(&shot);
    const int MARGINS = 6;
    const int WIDTH   = 3;
    int ax = g.x() - x - MARGINS -1;
    int ay = g.y() - y - MARGINS -1;
    painter.setPen( QPen(Qt::red/*KApplication::palette().active().highlight()*/, WIDTH) );
    painter.drawArc(ax, ay, tw + 2*MARGINS, th + 2*MARGINS, 0, 16*360);
    painter.end();

    // Paint the border
    const int BORDER = 1;
    QPixmap finalShot(w + 2*BORDER, h + 2*BORDER);
    finalShot.fill( KApplication::palette().color( QPalette::Foreground ));
    painter.begin(&finalShot);
    painter.drawPixmap(BORDER, BORDER, shot);
    painter.end();
    return shot; // not finalShot?? -fo
#else
    return QPixmap();
#endif
}

void TrayIcon::slotSetUnread(int unread)
{
    if (unread==m_unread)
        return;
    
    m_unread=unread;
    
    this->setToolTip( i18np("Akregator - 1 unread article", "Akregator - %n unread articles", unread > 0 ? unread : 0));
    
    if (unread <= 0)
    {    
        setPixmap(m_defaultIcon);
    }
    else
    {               
        // from KMSystemTray
        int oldW = pixmap()->size().width();
        int oldH = pixmap()->size().height();

        QString uStr=QString::number( unread );
        QFont f=KGlobalSettings::generalFont();
        f.setBold(true);
        float pointSize=f.pointSizeF();
        QFontMetrics fm(f);
        int w=fm.width(uStr);
        if( w > (oldW) )
        {
            pointSize *= float(oldW) / float(w);
            f.setPointSizeF(pointSize);
        }

        QPixmap pix(oldW, oldH);
        pix.fill(Qt::white);
        QPainter p(&pix);
        p.setFont(f);
        p.setPen(Qt::blue);
        p.drawText(pix.rect(), Qt::AlignCenter, uStr);

        pix.setMask(pix.createHeuristicMask());
        QImage img=pix.toImage();

        // overlay
        QImage overlayImg=m_lightIconImage.copy();
        KIconEffect::overlay(overlayImg, img);

        setPixmap(QPixmap::fromImage(overlayImg));
    }
}

void TrayIcon::viewButtonClicked()
{
	QWidget *p=static_cast<QWidget*>(parent());
	KWin::forceActiveWindow(p->winId());
}

void TrayIcon::settingsChanged()
{
    if ( Settings::showTrayIcon() )
        show();
    else
        hide();
}

} // namespace Akregator

#include "trayicon.moc"
