/*******************************************************************
 KNotes -- Notes for the KDE project

 Copyright (c) 1997-2005, The KNotes Developers

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*******************************************************************/

#ifndef KNOTE_H
#define KNOTE_H

#include <QString>
#include <QEvent>
#include <QFrame>
#include <QPoint>
#include <QColor>
#include <QDragEnterEvent>
#include <QLabel>
#include <QShowEvent>
#include <QResizeEvent>
#include <QDropEvent>
#include <QCloseEvent>

#include <kconfig.h>
#include <kxmlguiclient.h>

class QLabel;
class QSizeGrip;

class KXMLGUIBuilder;

class KFind;
class KMenu;
class KNoteButton;
class KNoteEdit;
class KNoteConfig;
class KToolBar;
class KListAction;
class KToggleAction;

namespace KCal {
    class Journal;
}


class KNote : public QFrame, virtual public KXMLGUIClient
{
    Q_OBJECT
public:
    KNote( QDomDocument buildDoc, KCal::Journal *journal, QWidget *parent = 0 );
    ~KNote();

    void saveData();
    void saveConfig() const;

    QString noteId() const;
    QString name() const;
    QString text() const;

    void setName( const QString& name );
    void setText( const QString& text );

    void find( const QString& pattern, long options );

    bool isModified() const;

    static void setStyle( int style );

public slots:
    void slotKill( bool force = false );

signals:
    void sigRequestNewNote();
    void sigShowNextNote();
    void sigNameChanged();
    void sigDataChanged();
    void sigColorChanged();
    void sigKillNote( KCal::Journal* );

    void sigFindFinished();

protected:
    virtual void contextMenuEvent( QContextMenuEvent * );
    virtual void drawFrame( QPainter* );
    virtual void showEvent( QShowEvent* );
    virtual void resizeEvent( QResizeEvent* );
    virtual void closeEvent( QCloseEvent* );
    virtual void dropEvent( QDropEvent* );
    virtual void dragEnterEvent( QDragEnterEvent* );

    virtual bool event( QEvent* );
    virtual bool eventFilter( QObject*, QEvent* );

    virtual bool focusNextPrevChild( bool );

private slots:
    void slotRename();
    void slotUpdateReadOnly();
    void slotClose();

    void slotSend();
    void slotMail();
    void slotPrint();
    void slotSaveAs();

    void slotInsDate();
    void slotSetAlarm();

    void slotPreferences();
    void slotPopupActionToDesktop( int id );

    void slotFindNext();
    void slotHighlight( const QString& txt, int idx, int len );

    void slotApplyConfig();
    void slotUpdateKeepAboveBelow();
    void slotUpdateShowInTaskbar();
    void slotUpdateDesktopActions();

    void slotUpdateViewport( int, int );

private:
    void updateFocus();
    void updateMask();
    void updateLayout();
    void updateLabelAlignment();
    void updateBackground( int offset = -1 );

    void setColor( const QColor&, const QColor& );

    void createFold();

    void toDesktop( int desktop );

private:
    QLabel        *m_label, *m_pushpin, *m_fold;
    QSizeGrip     *m_grip;
    KNoteButton   *m_button;
    KToolBar      *m_tool;
    KNoteEdit     *m_editor;

    KNoteConfig   *m_config;
    KCal::Journal *m_journal;

    KFind         *m_find;
    KMenu         *m_menu;

    KToggleAction *m_readOnly;

    KListAction   *m_toDesktop;
    KToggleAction *m_keepAbove;
    KToggleAction *m_keepBelow;

    KSharedConfig::Ptr m_kwinConf;

    static int s_ppOffset;
};

#endif
