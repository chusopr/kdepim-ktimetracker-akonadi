/*
  This file is part of KOrganizer.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#ifndef EDITORATTACHMENTS_H
#define EDITORATTACHMENTS_H

#include <KDialog>
#include <KMimeType>

class AttachmentIconItem;
class AttachmentIconView;

namespace KCal {
  class Attachment;
  class Incidence;
}

class KAction;
class KJob;
class KLineEdit;
class KMenu;
class KUrlRequester;

class QListWidgetItem;
class QCheckBox;
class QDragEnterEvent;
class QDropEvent;
class QLabel;
class QPushButton;

class AttachmentEditDialog : public KDialog
{
  Q_OBJECT
  public:
    AttachmentEditDialog( AttachmentIconItem *item,
                          QWidget *parent, bool modal=true );

    void accept();

  protected slots:
    void urlChanged( const KUrl &url );
    void urlChanged( const QString & url );
    virtual void slotApply();

  private:
    KMimeType::Ptr mMimeType;
    AttachmentIconItem *mItem;
    QLabel *mTypeLabel, *mIcon;
    KLineEdit *mLabelEdit;
    KUrlRequester *mURLRequester;
    QCheckBox *mInline;
};

class EditorAttachments : public QWidget
{
  Q_OBJECT
  public:
    explicit EditorAttachments( int spacing = 8, QWidget *parent = 0 );
    ~EditorAttachments();

    void addUriAttachment( const QString &uri,
                           const QString &mimeType = QString(),
                           const QString &label = QString(),
                           bool inLine = false );
    void addAttachment( KCal::Attachment *attachment );
    void addDataAttachment( const QByteArray &data,
                            const QString &mimeType = QString(),
                            const QString &label = QString() );

    /** Set widgets to default values */
    void setDefaults();
    /** Read event object and setup widgets accordingly */
    void readIncidence( KCal::Incidence * );
    /** Write event settings to event object */
    void fillIncidence( KCal::Incidence * );

    bool hasAttachments();

  public slots:
    /** Applies all deferred delete and copy operations */
    void applyChanges();

  protected slots:
    void showAttachment( QListWidgetItem *item );
    void saveAttachment( QListWidgetItem *item );
    void slotAdd();
    void slotEdit();
    void slotRemove();
    void slotShow();
    void slotSaveAs();
    void dragEnterEvent( QDragEnterEvent *event );
    void dropEvent( QDropEvent *event );
    void slotItemRenamed ( QListWidgetItem * item );
    void downloadComplete( KJob *job );
    void slotCopy();
    void slotCut();
    void slotPaste();
    void selectionChanged();
    void contextMenu( const QPoint &pos );

  signals:
    void openURL( const KUrl &url );

  private:
    void handlePasteOrDrop( const QMimeData *mimeData );

    AttachmentIconView *mAttachments;
    KMenu *mPopupMenu;
    QString mUid; // used only to generate attachments' filenames
    QPushButton *mRemoveBtn;
    KAction *mOpenAction;
    KAction *mSaveAsAction;
    KAction *mCopyAction;
    KAction *mCutAction;
    KAction *mDeleteAction;
    KAction *mEditAction;
};

#endif
