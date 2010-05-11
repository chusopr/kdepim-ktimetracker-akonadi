/*
    This file is part of KMail.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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


#include "recipientseditor.h"

#include "kwindowpositioner.h"
#include <messagecomposer/recipientspicker.h>
#include <messagecomposer/distributionlistdialog.h>
#include <messagecomposer/messagecomposersettings.h>
#include "messageviewer/autoqpointer.h"

#include <kpimutils/email.h>
#include <KMime/Headers>

#include <KDebug>
#include <KLocale>
#include <KCompletionBox>
#include <KMessageBox>

#include <QKeyEvent>
#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollBar>
#include <QTimer>
#include <QtGui/QTextDocument>

#include <boost/bind.hpp>
#include <algorithm>

RecipientComboBox::RecipientComboBox( QWidget *parent )
  : KComboBox( parent )
{
}

void RecipientComboBox::keyPressEvent( QKeyEvent *ev )
{
  if ( ev->key() == Qt::Key_Right ) emit rightPressed();
  else KComboBox::keyPressEvent( ev );
}

RecipientLineEdit::RecipientLineEdit ( QWidget* parent ) : ComposerLineEdit ( parent )
{
}

void RecipientLineEdit::keyPressEvent( QKeyEvent *ev )
{
  if ( ev->key() == Qt::Key_Backspace  &&  text().isEmpty() ) {
    ev->accept();
    emit deleteMe();
  } else if ( ev->key() == Qt::Key_Left && cursorPosition() == 0 &&
              !ev->modifiers().testFlag( Qt::ShiftModifier ) ) {  // Shift would be pressed during selection
    emit leftPressed();
  } else if ( ev->key() == Qt::Key_Right && cursorPosition() == (int)text().length() &&
              !ev->modifiers().testFlag( Qt::ShiftModifier ) ) {  // Shift would be pressed during selection
    emit rightPressed();
  } else {
    MessageComposer::ComposerLineEdit::keyPressEvent( ev );
  }
}

RecipientLine::RecipientLine( QWidget *parent )
  : QWidget( parent ), mRecipientsCount( 0 ), mModified( false )
{
  setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );

  QBoxLayout *topLayout = new QHBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );
  topLayout->setMargin( 0 );

  QStringList recipientTypes = Recipient::allTypeLabels();

  mCombo = new RecipientComboBox( this );
  mCombo->addItems( recipientTypes );
  topLayout->addWidget( mCombo );
  mCombo->setToolTip( i18nc("@label:listbox","Select type of recipient") );

  mEdit = new RecipientLineEdit( this );
  mEdit->setToolTip( i18n( "Set the list of email addresses to receive this message" ) );
  mEdit->setClearButtonShown( true );
  topLayout->addWidget( mEdit );
  connect( mEdit, SIGNAL( returnPressed() ), SLOT( slotReturnPressed() ) );
  connect( mEdit, SIGNAL( deleteMe() ), SLOT( slotPropagateDeletion() ) );
  connect( mEdit, SIGNAL( textChanged( const QString & ) ),
    SLOT( analyzeLine( const QString & ) ) );
  connect( mEdit, SIGNAL( focusUp() ), SLOT( slotFocusUp() ) );
  connect( mEdit, SIGNAL( focusDown() ), SLOT( slotFocusDown() ) );
  connect( mEdit, SIGNAL( rightPressed() ), SIGNAL( rightPressed() ) );

  connect( mEdit, SIGNAL( leftPressed() ), mCombo, SLOT( setFocus() ) );
  connect( mEdit, SIGNAL( editingFinished() ), SLOT( slotEditingFinished() ) );
  connect( mEdit, SIGNAL( clearButtonClicked() ), SLOT( slotPropagateDeletion() ) );
  connect( mCombo, SIGNAL( rightPressed() ), mEdit, SLOT( setFocus() ) );

  connect( mCombo, SIGNAL( activated ( int ) ),
           this, SLOT( slotTypeModified() ) );
}

void RecipientLine::slotFocusUp()
{
  emit upPressed( this );
}

void RecipientLine::slotFocusDown()
{
  emit downPressed( this );
}

void RecipientLine::slotTypeModified()
{
  mModified = true;

  emit typeModified( this );
}

void RecipientLine::analyzeLine( const QString &text )
{
  QStringList r = KPIMUtils::splitAddressList( text );
  if ( int( r.count() ) != mRecipientsCount ) {
    mRecipientsCount = r.count();
    emit countChanged();
  }
}

int RecipientLine::recipientsCount() const
{
  return mRecipientsCount;
}

void RecipientLine::setRecipient( const Recipient &rec )
{
  mEdit->setText( rec.email() );
  mCombo->setCurrentIndex( Recipient::typeToId( rec.type() ) );
}

void RecipientLine::setRecipient( const QString &email )
{
  setRecipient( Recipient( email ) );
}

Recipient RecipientLine::recipient() const
{
  return Recipient( mEdit->text(),
    Recipient::idToType( mCombo->currentIndex() ) );
}

void RecipientLine::setRecipientType( Recipient::Type type )
{
  mCombo->setCurrentIndex( Recipient::typeToId( type ) );
}

Recipient::Type RecipientLine::recipientType() const
{
  return Recipient::idToType( mCombo->currentIndex() );
}

void RecipientLine::activate()
{
  mEdit->setFocus();
}

bool RecipientLine::isActive() const
{
  return mEdit->hasFocus();
}

bool RecipientLine::isEmpty() const
{
  return mEdit->text().isEmpty();
}

bool RecipientLine::isModified()
{
  return mModified || mEdit->isModified();
}

void RecipientLine::clearModified()
{
  mModified = false;
  mEdit->setModified( false );
}

void RecipientLine::slotReturnPressed()
{
  emit returnPressed( this );
}

void RecipientLine::slotPropagateDeletion()
{
  emit deleteLine( this );
}

void RecipientLine::slotEditingFinished()
{
  if ( mEdit->text().isEmpty() ) {
    emit deleteLine( this );
  }
}

void RecipientLine::keyPressEvent( QKeyEvent *ev )
{
  if ( ev->key() == Qt::Key_Up ) {
    emit upPressed( this );
  } else if ( ev->key() == Qt::Key_Down ) {
    emit downPressed( this );
  }
}

int RecipientLine::setComboWidth( int w )
{
  w = qMax( w, mCombo->sizeHint().width() );
  mCombo->setFixedWidth( w );
  mCombo->updateGeometry();
  parentWidget()->updateGeometry();
  return w;
}

void RecipientLine::fixTabOrder( QWidget *previous )
{
  setTabOrder( previous, mCombo );
  setTabOrder( mCombo, mEdit );
}

QWidget *RecipientLine::tabOut() const
{
  return mEdit;
}

void RecipientLine::clear()
{
  mEdit->clear();
}

void RecipientLine::setEditFont( const QFont& font )
{
  mEdit->setFont( font );
}

void RecipientLine::setRecentAddressConfig(KConfig* config)
{
  mEdit->setRecentAddressConfig( config );
}


// ------------ RecipientsView ---------------------

RecipientsView::RecipientsView( QWidget *parent )
  : QScrollArea( parent ), mCurDelLine( 0 ),
    mLineHeight( 0 ), mFirstColumnWidth( 0 ),
    mModified( false ), mRecentAddressConfig( 0 )
{
  mCompletionMode = KGlobalSettings::completionMode();

  setWidgetResizable( true );
  setFrameStyle( QFrame::NoFrame );

  mEditFont = KGlobalSettings::generalFont();
  mPage = new QWidget;

  mPage->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
  setWidget( mPage );

  mTopLayout = new QVBoxLayout;
  mTopLayout->setMargin( 0 );
  mTopLayout->setSpacing( 0 );
  mPage->setLayout( mTopLayout );

  addLine();
}

RecipientLine *RecipientsView::activeLine() const
{
  return mLines.last();
}

RecipientLine *RecipientsView::emptyLine() const
{
  RecipientLine *line;
  foreach( line, mLines ) {
    if ( line->isEmpty() )
      return line;
  }

  return 0;
}

RecipientLine *RecipientsView::addLine()
{
  RecipientLine *line = new RecipientLine( widget() );
  if ( mRecentAddressConfig )
    line->setRecentAddressConfig( mRecentAddressConfig );
  line->setEditFont( mEditFont );
  //addChild( line, 0, mLines.count() * mLineHeight );
  mTopLayout->addWidget( line );
  line->mEdit->setCompletionMode( mCompletionMode );
  line->show();
  connect( line, SIGNAL( returnPressed( RecipientLine * ) ),
    SLOT( slotReturnPressed( RecipientLine * ) ) );
  connect( line, SIGNAL( upPressed( RecipientLine * ) ),
    SLOT( slotUpPressed( RecipientLine * ) ) );
  connect( line, SIGNAL( downPressed( RecipientLine * ) ),
    SLOT( slotDownPressed( RecipientLine * ) ) );
  connect( line, SIGNAL( rightPressed() ), SIGNAL( focusRight() ) );
  connect( line, SIGNAL( deleteLine( RecipientLine * ) ),
    SLOT( slotDecideLineDeletion( RecipientLine * ) ) );
  connect( line, SIGNAL( countChanged() ), SLOT( calculateTotal() ) );
  connect( line, SIGNAL( typeModified( RecipientLine * ) ),
    SLOT( slotTypeModified( RecipientLine * ) ) );
  connect( line->mEdit, SIGNAL( completionModeChanged( KGlobalSettings::Completion ) ),
    SLOT( setCompletionMode( KGlobalSettings::Completion ) ) );
  connect( line->mEdit, SIGNAL( textChanged ( const QString & ) ),
           SLOT( calculateTotal() ) );

  if ( !mLines.isEmpty() ) {
    if ( mLines.count() == 1 ) {
      if ( MessageComposer::MessageComposerSettings::self()->secondRecipientTypeDefault() ==
         MessageComposer::MessageComposerSettings::EnumSecondRecipientTypeDefault::To ) {
        line->setRecipientType( Recipient::To );
      } else {
        if ( mLines.last()->recipientType() == Recipient::Bcc ) {
          line->setRecipientType( Recipient::To );
        } else {
          line->setRecipientType( Recipient::Cc );
        }
      }
    } else {
      line->setRecipientType( mLines.last()->recipientType() );
    }
    line->fixTabOrder( mLines.last()->tabOut() );
  }

  mLines.append( line );

  mFirstColumnWidth = line->setComboWidth( mFirstColumnWidth );

  mLineHeight = line->minimumSizeHint().height();

  line->resize( viewport()->width(), mLineHeight );

  resizeView();

  calculateTotal();

  ensureVisible( 0, mLines.count() * mLineHeight, 0, 0 );

  QTimer::singleShot( 0, this, SLOT(moveScrollBarToEnd()) );

  return line;
}

void RecipientsView::moveScrollBarToEnd()
{
// scroll to bottom
   verticalScrollBar()->triggerAction( QAbstractSlider::SliderToMaximum );
}

void RecipientsView::slotTypeModified( RecipientLine *line )
{
  if ( mLines.count() == 2 ||
       ( mLines.count() == 3 && mLines.at( 2 )->isEmpty() ) ) {
    if ( mLines.at( 1 ) == line ) {
      if ( line->recipientType() == Recipient::To ) {
        MessageComposer::MessageComposerSettings::self()->setSecondRecipientTypeDefault(
          MessageComposer::MessageComposerSettings::EnumSecondRecipientTypeDefault::To );
      } else if ( line->recipientType() == Recipient::Cc ) {
        MessageComposer::MessageComposerSettings::self()->setSecondRecipientTypeDefault(
          MessageComposer::MessageComposerSettings::EnumSecondRecipientTypeDefault::Cc );
      }
    }
  }

  //Update the total tooltip
  calculateTotal();
}

void RecipientsView::calculateTotal()
{
  int count = 0;
  int empty = 0;

  RecipientLine *line;
  foreach( line, mLines ) {
    if ( line->isEmpty() ) ++empty;
    else count += line->recipientsCount();
  }

  if ( empty == 0 ) addLine();

  emit totalChanged( count, mLines.count() );
}

void RecipientsView::slotReturnPressed( RecipientLine *line )
{
  if ( !line->recipient().isEmpty() ) {
    RecipientLine *empty = emptyLine();
    if ( !empty ) empty = addLine();
    activateLine( empty );
  }
}

void RecipientsView::slotDownPressed( RecipientLine *line )
{
  int pos = mLines.indexOf( line );
  if ( pos >= (int)mLines.count() - 1 ) {
    emit focusDown();
  } else if ( pos >= 0 ) {
    activateLine( mLines.at( pos + 1 ) );
  }
}

void RecipientsView::slotUpPressed( RecipientLine *line )
{
  int pos = mLines.indexOf( line );
  if ( pos > 0 ) {
    activateLine( mLines.at( pos - 1 ) );
  } else {
    emit focusUp();
  }
}

void RecipientsView::slotDecideLineDeletion( RecipientLine *line )
{
  if ( !line->isEmpty() )
    mModified = true;
  if ( mLines.count() == 1 ) {
    line->clear();
  } else if ( mLines.indexOf( line ) != mLines.count() - 1 ) {
    mCurDelLine = line;
    QTimer::singleShot( 0, this, SLOT( slotDeleteLine( ) ) );
  }
}

void RecipientsView::slotDeleteLine()
{
  if ( !mCurDelLine )
    return;

  RecipientLine *line = mCurDelLine;
  int pos = mLines.indexOf( line );

  if ( mCurDelLine->isActive() ) {
    int newPos;
    if ( pos == 0 ) newPos = pos + 1;
    else newPos = pos - 1;

    // if there is something left to activate, do so
    if ( mLines.at( newPos ) )
      mLines.at( newPos )->activate();
  }

  mLines.removeAll( line );
  line->setParent( 0 );
  delete line;

  bool atLeastOneToLine = false;
  int firstCC = -1;
  for( int i = pos; i < mLines.count(); ++i ) {
    RecipientLine *line = mLines.at( i );
    if ( line->recipientType() == Recipient::To )
      atLeastOneToLine = true;
    else if ( ( line->recipientType() == Recipient::Cc ) && ( firstCC < 0 ) )
      firstCC = i;
  }

  if ( !atLeastOneToLine && ( firstCC >= 0 ) )
    mLines.at( firstCC )->setRecipientType( Recipient::To );

  calculateTotal();

  resizeView();
}

void RecipientsView::resizeView()
{
  if ( mLines.count() < 6 ) {
    setMinimumHeight( mLineHeight * mLines.count() );
  } else {
    setMinimumHeight( mLineHeight * 5 );
    setMaximumHeight( mLineHeight * mLines.count() );
  }

  parentWidget()->layout()->activate();
  emit sizeHintChanged();
  QTimer::singleShot( 0, this, SLOT(moveCompletionPopup()) );
}

void RecipientsView::activateLine( RecipientLine *line )
{
  line->activate();
  ensureWidgetVisible( line );
}

void RecipientsView::resizeEvent ( QResizeEvent *ev )
{
  QScrollArea::resizeEvent(ev);
  for( int i = 0; i < mLines.count(); ++i ) {
    mLines.at( i )->resize( ev->size().width(), mLineHeight );
  }
  ensureVisible( 0, mLines.count() * mLineHeight, 0, 0 );
}

QSize RecipientsView::sizeHint() const
{
  return QSize( 200, mLineHeight * mLines.count() );
}

QSize RecipientsView::minimumSizeHint() const
{
  int height;
  int numLines = 5;
  if ( mLines.count() < numLines ) height = mLineHeight * mLines.count();
  else height = mLineHeight * numLines;
  return QSize( 200, height );
}

Recipient::List RecipientsView::recipients() const
{
  Recipient::List recipients;

  QListIterator<RecipientLine*> it( mLines );
  RecipientLine *line;
  while( it.hasNext()) {
    line = it.next();
    if ( !line->recipient().isEmpty() ) {
      recipients.append( line->recipient() );
    }
  }

  return recipients;
}

void RecipientsView::setCompletionMode ( KGlobalSettings::Completion mode )
{
  if ( mCompletionMode == mode )
    return;
  mCompletionMode = mode;

  QListIterator<RecipientLine*> it( mLines );
  while( it.hasNext() ) {
    RecipientLine *line = it.next();
    line->mEdit->blockSignals( true );
    line->mEdit->setCompletionMode( mode );
    line->mEdit->blockSignals( false );
  }
  emit completionModeChanged( mode ); //report change to RecipientsEditor
}

void RecipientsView::removeRecipient( const QString & recipient,
                                      Recipient::Type type )
{
  // search a line which matches recipient and type
  QListIterator<RecipientLine*> it( mLines );
  RecipientLine *line = 0;
  while (it.hasNext()) {
    line = it.next();
    if ( ( line->recipient().email() == recipient ) &&
         ( line->recipientType() == type ) ) {
      break;
    }
  }
  if ( line )
    line->slotPropagateDeletion();
}

bool RecipientsView::isModified()
{
  if ( mModified )
    return true;

  QListIterator<RecipientLine*> it( mLines );
  RecipientLine *line;
  while( it.hasNext()) {
  line = it.next();
    if ( line->isModified() ) {
      return true;
    }
  }

  return false;
}

void RecipientsView::clearModified()
{
  mModified = false;

  QListIterator<RecipientLine*> it( mLines );
  RecipientLine *line;
  while( it.hasNext() ) {
  line = it.next();
    line->clearModified();
  }
}

void RecipientsView::setFocus()
{
  if ( !mLines.empty() && mLines.last()->isActive() )
    setFocusBottom();
  else
    setFocusTop();
}

void RecipientsView::setFocusTop()
{
  if ( !mLines.empty() ) {
    RecipientLine *line = mLines.first();
    if ( line ) line->activate();
    else kWarning() <<"No first";
  }
  else kWarning() <<"No first";
}

void RecipientsView::setFocusBottom()
{
  RecipientLine *line = mLines.last();
  if ( line ) {
    ensureWidgetVisible( line );
    line->activate();
  }
  else  kWarning() <<"No last";
}

int RecipientsView::setFirstColumnWidth( int w )
{
  mFirstColumnWidth = w;

  QListIterator<RecipientLine*> it( mLines );
  RecipientLine *line;
  while(it.hasNext()) {
    line = it.next();
    mFirstColumnWidth = line->setComboWidth( mFirstColumnWidth );
  }

  resizeView();
  return mFirstColumnWidth;
}

void RecipientsView::setEditFont( const QFont& font )
{
  mEditFont = font;
  foreach ( RecipientLine *line, mLines ) {
    line->setEditFont( mEditFont );
  }
}

void RecipientsView::moveCompletionPopup()
{
  foreach ( RecipientLine *const line, mLines ) {
    if ( line->lineEdit()->completionBox( false ) ) {
      if ( line->lineEdit()->completionBox()->isVisible() ) {
        // ### trigger moving, is there a nicer way to do that?
        line->lineEdit()->completionBox()->hide();
        line->lineEdit()->completionBox()->show();
      }
    }
  }

}

void RecipientsView::setRecentAddressConfig(KConfig* config)
{
  mRecentAddressConfig = config;
  if ( config )
    std::for_each( mLines.begin(), mLines.end(), boost::bind( &RecipientLine::setRecentAddressConfig, _1, config ) );
}


SideWidget::SideWidget( RecipientsView *view, QWidget *parent )
  : QWidget( parent ), mView( view ), mRecipientPicker( 0 )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );

  topLayout->setSpacing( KDialog::spacingHint() );
  topLayout->setMargin( 0 );
  topLayout->addStretch( 1 );

  mTotalLabel = new QLabel( this );
  mTotalLabel->setAlignment( Qt::AlignCenter );
  topLayout->addWidget( mTotalLabel );
  mTotalLabel->hide();

  topLayout->addStretch( 1 );

  mDistributionListButton = new QPushButton(
    i18nc("@action:button","Save List..."), this );
  topLayout->addWidget( mDistributionListButton );
  mDistributionListButton->hide();
  connect( mDistributionListButton, SIGNAL( clicked() ),
    SIGNAL( saveDistributionList() ) );
  mDistributionListButton->setToolTip(
    i18nc( "@info:tooltip", "Save recipients as distribution list") );

  mSelectButton = new QPushButton(
    i18nc( "@action:button Open recipient selection dialog.", "Se&lect..."), this );
  topLayout->addWidget( mSelectButton );
  connect( mSelectButton, SIGNAL( clicked() ), SLOT( pickRecipient() ) );
  mSelectButton->setToolTip( i18nc("@info:tooltip","Select recipients from address book") );
  updateTotalToolTip();
}

SideWidget::~SideWidget()
{
}

MessageComposer::RecipientsPicker* SideWidget::picker() const
{
  if ( !mRecipientPicker ) {
    // hacks to allow picker() to be const in the presence of lazy loading
    SideWidget *non_const_this = const_cast<SideWidget*>( this );
    mRecipientPicker = new MessageComposer::RecipientsPicker( non_const_this );
    connect( mRecipientPicker, SIGNAL( pickedRecipient( const Recipient & ) ),
             non_const_this, SIGNAL( pickedRecipient( const Recipient & ) ) );
    mPickerPositioner = new KWindowPositioner( mSelectButton, mRecipientPicker );
  }
  return mRecipientPicker;
}

void SideWidget::setFocus()
{
  mSelectButton->setFocus();
}

void SideWidget::setTotal( int recipients, int lines )
{
  QString labelText;
  if ( recipients == 0 ) labelText = i18nc("@info:status No recipients selected"
    , "No recipients");
  else labelText = i18ncp("@info:status Number of recipients selected"
    , "1 recipient","%1 recipients", recipients );
  mTotalLabel->setText( labelText );

  if ( lines > 3 ) mTotalLabel->show();
  else mTotalLabel->hide();

  if ( lines > 2 ) mDistributionListButton->show();
  else mDistributionListButton->hide();

  updateTotalToolTip();
}

void SideWidget::updateTotalToolTip()
{
  QString text = "<qt>";

  QString to;
  QString cc;
  QString bcc;

  Recipient::List recipients = mView->recipients();
  Recipient::List::ConstIterator it;
  for( it = recipients.constBegin(); it != recipients.constEnd(); ++it ) {
    QString emailLine = "&nbsp;&nbsp;" + Qt::escape( (*it).email() ) + "<br/>";
    switch( (*it).type() ) {
      case Recipient::To:
        to += emailLine;
        break;
      case Recipient::Cc:
        cc += emailLine;
        break;
      case Recipient::Bcc:
        bcc += emailLine;
        break;
      default:
        break;
    }
  }

  text += i18nc("@info:tooltip %1 list of emails", "<interface>To:</interface><nl/>%1", to);
  if ( !cc.isEmpty() ) {
    text += i18nc("@info:tooltip %1 list of emails", "<interface>CC:</interface><nl/>%1", cc);
  }
  if ( !bcc.isEmpty() ) {
    text += i18nc("@info:tooltip %1 list of emails", "<interface>BCC:</interface><nl/>%1", bcc);
  }

  text.append( "</qt>" );
  mTotalLabel->setToolTip( text );
}

void SideWidget::pickRecipient()
{
  MessageComposer::RecipientsPicker *p = picker();
  p->setDefaultType( mView->activeLine()->recipientType() );
  p->setRecipients( mView->recipients() );
  mPickerPositioner->reposition();
  p->show();
}


RecipientsEditor::RecipientsEditor( QWidget *parent )
  : QWidget( parent ), mModified( false )
{
  QBoxLayout *topLayout = new QHBoxLayout();
  topLayout->setSpacing( KDialog::spacingHint() );
  topLayout->setMargin( 0 );
  setLayout( topLayout );

  mRecipientsView = new RecipientsView( this );
  topLayout->addWidget( mRecipientsView );
  connect( mRecipientsView, SIGNAL( focusUp() ), SIGNAL( focusUp() ) );
  connect( mRecipientsView, SIGNAL( focusDown() ), SIGNAL( focusDown() ) );
  connect( mRecipientsView, SIGNAL( completionModeChanged( KGlobalSettings::Completion ) ),
    SIGNAL( completionModeChanged( KGlobalSettings::Completion ) ) );

  mSideWidget = new SideWidget( mRecipientsView, this );
  topLayout->addWidget( mSideWidget );
  connect( mSideWidget, SIGNAL( pickedRecipient( const Recipient & ) ),
    SLOT( slotPickedRecipient( const Recipient & ) ) );
  connect( mSideWidget, SIGNAL( saveDistributionList() ),
    SLOT( saveDistributionList() ) );

  connect( mRecipientsView, SIGNAL( totalChanged( int, int ) ),
    mSideWidget, SLOT( setTotal( int, int ) ) );
  connect( mRecipientsView, SIGNAL( focusRight() ),
    mSideWidget, SLOT( setFocus() ) );

  connect( mRecipientsView, SIGNAL(sizeHintChanged()),
           SIGNAL(sizeHintChanged()) );
}

RecipientsEditor::~RecipientsEditor()
{
}

MessageComposer::RecipientsPicker* RecipientsEditor::picker() const
{
  return mSideWidget->picker();
}

void RecipientsEditor::slotPickedRecipient( const Recipient &rec )
{
  RecipientLine *line = mRecipientsView->activeLine();
  if ( !line->isEmpty() ) line = mRecipientsView->addLine();

  Recipient r = rec;
  if ( r.type() == Recipient::Undefined ) {
    r.setType( line->recipientType() );
  }

  line->setRecipient( r );
  mModified = true;
}

void RecipientsEditor::saveDistributionList()
{
  MessageViewer::AutoQPointer<MessageComposer::DistributionListDialog> dlg( new MessageComposer::DistributionListDialog( this ) );
  dlg->setRecipients( mRecipientsView->recipients() );
  dlg->exec();
}

Recipient::List RecipientsEditor::recipients() const
{
  return mRecipientsView->recipients();
}

void RecipientsEditor::setRecipientString( const QList<KMime::Types::Mailbox> &mailboxes,
                                           Recipient::Type type )
{
  clear();

  int count = 1;

  foreach( const KMime::Types::Mailbox &mailbox, mailboxes ) {
    if ( count++ > MessageComposer::MessageComposerSettings::self()->maximumRecipients() ) {
      KMessageBox::sorry( this,
        i18nc("@info:status", "Truncating recipients list to %1 of %2 entries.",
              MessageComposer::MessageComposerSettings::self()->maximumRecipients(),
              mailboxes.count() ) );
      break;
    }
    addRecipient( mailbox.prettyAddress( KMime::Types::Mailbox::QuoteWhenNecessary ), type );
  }
}

QString RecipientsEditor::recipientString( Recipient::Type type )
{
  QString str;

  Recipient::List recipients = mRecipientsView->recipients();
  if ( !recipients.isEmpty() ) {
    Recipient::List::ConstIterator it;
    for( it = recipients.constBegin(); it != recipients.constEnd(); ++it ) {
      if ( (*it).type() == type ) {
        if ( !str.isEmpty() ) str += ", ";
        str.append( (*it).email() );
      }
    }
  }
  return str;
}

void RecipientsEditor::addRecipient( const QString & recipient,
                                     Recipient::Type type )
{
  RecipientLine *line = mRecipientsView->emptyLine();
  if ( !line ) line = mRecipientsView->addLine();
  line->setRecipient( Recipient( recipient, type ) );
}

void RecipientsEditor::removeRecipient( const QString & recipient,
                                        Recipient::Type type )
{
  mRecipientsView->removeRecipient( recipient, type );
}

bool RecipientsEditor::isModified()
{
  return mModified || mRecipientsView->isModified();
}

void RecipientsEditor::clearModified()
{
  mModified = false;
  mRecipientsView->clearModified();
}

void RecipientsEditor::clear()
{
}

void RecipientsEditor::setFocus()
{
  mRecipientsView->setFocus();
}

void RecipientsEditor::setFocusTop()
{
  mRecipientsView->setFocusTop();
}

void RecipientsEditor::setFocusBottom()
{
  mRecipientsView->setFocusBottom();
}

int RecipientsEditor::setFirstColumnWidth( int w )
{
  return mRecipientsView->setFirstColumnWidth( w );
}

void RecipientsEditor::selectRecipients()
{
  mSideWidget->pickRecipient();
}

void RecipientsEditor::setCompletionMode( KGlobalSettings::Completion mode )
{
  mRecipientsView->setCompletionMode( mode );
}

void RecipientsEditor::setEditFont( const QFont& font )
{
  mRecipientsView->setEditFont( font );
}

void RecipientsEditor::setRecentAddressConfig(KConfig* config)
{
  mRecipientsView->setRecentAddressConfig( config );
}


#include "recipientseditor.moc"
