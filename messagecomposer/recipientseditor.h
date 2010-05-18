/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
    This file was part of KMail.
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef MESSAGECOMPOSER_RECIPIENTSEDITOR_H
#define MESSAGECOMPOSER_RECIPIENTSEDITOR_H

#include "messagecomposer_export.h"

#include <kcombobox.h>

#include <QList>
#include <QScrollArea>
#include <messagecomposer/composerlineedit.h>
#include <messagecomposer/recipient.h>

class KWindowPositioner;

class QLabel;
class QPushButton;

namespace KMime {
  namespace Types {
    class Mailbox;
  }
}

namespace MessageComposer {

class RecipientsPicker;
class SideWidget;

class RecipientComboBox : public KComboBox
{
    Q_OBJECT
  public:
    explicit RecipientComboBox( QWidget *parent );

  signals:
    void rightPressed();

  protected:
    void keyPressEvent( QKeyEvent *ev );
};

class RecipientLineEdit : public MessageComposer::ComposerLineEdit
{
    Q_OBJECT
  public:
    explicit RecipientLineEdit( QWidget * parent );

  signals:
    void deleteMe();
    void leftPressed();
    void rightPressed();

  protected:
    void keyPressEvent( QKeyEvent *ev );
};

class RecipientLine : public QWidget
{
    Q_OBJECT
  public:
    explicit RecipientLine( QWidget *parent );

    void setRecipient( const Recipient & );
    Recipient recipient() const;

    void setRecipientType( Recipient::Type );
    Recipient::Type recipientType() const;

    void setRecipient( const QString & );

    void activate();
    bool isActive() const;

    bool isEmpty() const;

    /** Returns true if the user has made any modifications to this
        RecipientLine.
    */
    bool isModified();

    /** Resets the modified flag to false.
    */
    void clearModified();

    int setComboWidth( int w );

    void fixTabOrder( QWidget *previous );
    QWidget *tabOut() const;

    void clear();

    int recipientsCount() const;

    /**
     * Sets the font which will be used for the lineedits.
     */
    void setEditFont( const QFont& font );

    /**
     * Sets the config file used for storing recent addresses.
     */
    void setRecentAddressConfig( KConfig *config );

  signals:
    void returnPressed( RecipientLine * );
    void downPressed( RecipientLine * );
    void upPressed( RecipientLine * );
    void rightPressed();
    void deleteLine( RecipientLine * );
    void countChanged();
    void typeModified( RecipientLine * );

  protected:
    void keyPressEvent( QKeyEvent * );
    RecipientLineEdit* lineEdit() const { return mEdit; }

  protected slots:
    void slotReturnPressed();
    void analyzeLine( const QString & );
    void slotFocusUp();
    void slotFocusDown();
    void slotEditingFinished();
    void slotPropagateDeletion();
    void slotTypeModified();

  private:
    friend class RecipientsView;
    KComboBox *mCombo;
    RecipientLineEdit *mEdit;
    int mRecipientsCount;
    bool mModified;
};

class RecipientsView : public QScrollArea
{
    Q_OBJECT
  public:
    explicit RecipientsView( QWidget *parent );

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    RecipientLine *activeLine() const;

    RecipientLine *emptyLine() const;

    Recipient::List recipients() const;

    /** Removes the recipient provided it can be found and has the given type.
        @param recipient The recipient(s) you want to remove.
        @param type      The recipient type.
    */
    void removeRecipient( const QString & recipient, Recipient::Type type );

    /** Returns true if the user has made any modifications to the list of
        recipients.
    */
    bool isModified();

    /** Resets the modified flag to false.
    */
    void clearModified();

    void activateLine( RecipientLine * );

    /**
      * Set the width of the left most column to be the argument width.
      * This method allows other widgets to align their label/combobox column with ours
      * by communicating how many pixels that first column is for them.
      * Returns the width that is actually being used.
      */
    int setFirstColumnWidth( int );

    /**
     * Sets the font which will be used for the lineedits.
     */
    void setEditFont( const QFont& font );

    /**
     * Sets the config file used for storing recent addresses.
     */
    void setRecentAddressConfig( KConfig *config );

  public slots:
    void setCompletionMode( KGlobalSettings::Completion );
    RecipientLine *addLine();

    void setFocus();
    void setFocusTop();
    void setFocusBottom();

  signals:
    void totalChanged( int recipients, int lines );
    void focusUp();
    void focusDown();
    void focusRight();
    void completionModeChanged( KGlobalSettings::Completion );
    void sizeHintChanged();

  protected:
    void resizeEvent( QResizeEvent * );
    void resizeView();

  protected slots:
    void slotReturnPressed( RecipientLine * );
    void slotDownPressed( RecipientLine * );
    void slotUpPressed( RecipientLine * );
    void slotDecideLineDeletion(  RecipientLine * );
    void slotDeleteLine();
    void calculateTotal();
    void slotTypeModified( RecipientLine * );
    void moveCompletionPopup();
    void moveScrollBarToEnd();

  private:
    QList<RecipientLine*> mLines;
    QFont mEditFont;
    QPointer<RecipientLine> mCurDelLine;
    int mLineHeight;
    int mFirstColumnWidth;
    bool mModified;
    KGlobalSettings::Completion mCompletionMode;
    QWidget *mPage;
    QLayout *mTopLayout;
    KConfig *mRecentAddressConfig;
};

class SideWidget : public QWidget
{
    Q_OBJECT
  public:
    SideWidget( RecipientsView *view, QWidget *parent );
    ~SideWidget();

    MessageComposer::RecipientsPicker* picker() const;

  public slots:
    void setTotal( int recipients, int lines );
    void setFocus();
    void updateTotalToolTip();
    void pickRecipient();

  signals:
    void pickedRecipient( const Recipient & );
    void saveDistributionList();

  private:
    RecipientsView *mView;
    QLabel *mTotalLabel;
    QPushButton *mDistributionListButton;
    QPushButton *mSelectButton;
    /** The RecipientsPicker is lazy loaded, never access it directly,
      only through picker() */
    mutable MessageComposer::RecipientsPicker *mRecipientPicker;
    /** lazy loaded, don't access directly, unless you've called picker() */
    mutable KWindowPositioner *mPickerPositioner;
};

class MESSAGECOMPOSER_EXPORT RecipientsEditor : public QWidget
{
    Q_OBJECT
  public:
    explicit RecipientsEditor( QWidget *parent = 0 );
    ~RecipientsEditor();

    void clear();

    Recipient::List recipients() const;
    MessageComposer::RecipientsPicker* picker() const;

    void setRecipientString( const QList<KMime::Types::Mailbox> &mailboxes, Recipient::Type );
    QString recipientString( Recipient::Type ) const;
    QStringList recipientStringList( Recipient::Type ) const;

    /** Adds a recipient (or multiple recipients) to one line of the editor.
        @param recipient The recipient(s) you want to add.
        @param type      The recipient type.
    */
    void addRecipient( const QString & recipient, Recipient::Type type );

    /** Removes the recipient provided it can be found and has the given type.
        @param recipient The recipient(s) you want to remove.
        @param type      The recipient type.
    */
    void removeRecipient( const QString & recipient, Recipient::Type type );

    /** Returns true if the user has made any modifications to the list of
        recipients.
    */
    bool isModified();

    /** Resets the modified flag to false.
    */
    void clearModified();

    /**
      * Set the width of the left most column to be the argument width.
      * This method allows other widgets to align their label/combobox column with ours
      * by communicating how many pixels that first column is for them.
      * Returns the width that is actually being used.
      */
    int setFirstColumnWidth( int );

    /**
      * Set completion mode for all lines
      */
    void setCompletionMode( KGlobalSettings::Completion );

    /**
     * Sets the font which will be used for the lineedits.
     */
    void setEditFont( const QFont& font );

    /**
     * Sets the config file used for storing recent addresses.
     */
    void setRecentAddressConfig( KConfig *config );

  public slots:
    void setFocus();
    void setFocusTop();
    void setFocusBottom();

    void selectRecipients();
    void saveDistributionList();

  signals:
    void focusUp();
    void focusDown();
    void completionModeChanged( KGlobalSettings::Completion );
    void sizeHintChanged();

  protected slots:
    void slotPickedRecipient( const Recipient & );

  private:
    RecipientsView *mRecipientsView;
    SideWidget* mSideWidget;
    bool mModified;
};

}

#endif
