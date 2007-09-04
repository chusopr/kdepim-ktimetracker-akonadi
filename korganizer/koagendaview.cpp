/*
  This file is part of KOrganizer.
  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#include "koagendaview.h"
#include "koglobals.h"
#ifndef KORG_NOPLUGINS
#include "kocore.h"
#include "kodecorationlabel.h"
#endif
#include "koprefs.h"
#include "koagenda.h"
#include "koagendaitem.h"
#include "kogroupware.h"
#include "kodialogmanager.h"
#include "koeventpopupmenu.h"
#include "koalternatelabel.h"

#include <kcal/calendar.h>
#include <kcal/icaldrag.h>
#include <kcal/dndfactory.h>
#include <kcal/calfilter.h>
#include <kcal/incidenceformatter.h>

#include <kapplication.h>
#include <kcalendarsystem.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kholidays.h>
#include <KVBox>
#include <ksystemtimezone.h>
#include <kpushbutton.h>
#include <kcombobox.h>

#include <QLabel>
#include <QFrame>
#include <QLayout>
#ifndef KORG_NOSPLITTER
#include <QSplitter>
#endif
#include <QFont>
#include <QFontMetrics>
#include <QMenu>
#include <QPainter>
#include <QPushButton>
#include <QToolButton>
#include <QCursor>
#include <QBitArray>
#include <QPaintEvent>
#include <QGridLayout>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QListWidget>

#include "koagendaview.moc"

using namespace KOrg;

TimeScaleConfigDialog::TimeScaleConfigDialog( QWidget *parent )
  : QDialog( parent )
{
  ui.setupUi( this );

  QStringList list;
  const KTimeZones::ZoneMap timezones = KSystemTimeZones::zones();
  for (KTimeZones::ZoneMap::ConstIterator it = timezones.begin();  it != timezones.end();  ++it) {
    list.append(i18n(it.key().toUtf8()));
  }
  list.sort();
  ui.zoneCombo->addItems( list );
  ui.zoneCombo->setCurrentIndex( 0 );

  ui.addButton->setIcon( KIcon( "plus" ) );
  ui.removeButton->setIcon( KIcon( "edit-delete" ) );
  ui.upButton->setIcon( KIcon( "arrow-up" ) );
  ui.downButton->setIcon( KIcon( "arrow-down" ) );

  connect( ui.addButton, SIGNAL( clicked() ), SLOT( add() ) );
  connect( ui.removeButton, SIGNAL( clicked() ), SLOT( remove() ) );
  connect( ui.upButton, SIGNAL( clicked() ), SLOT( up() ) );
  connect( ui.downButton, SIGNAL( clicked() ), SLOT( down() ) );

  connect( ui.okButton, SIGNAL( clicked() ), SLOT( okClicked() ) );
  connect( ui.cancelButton, SIGNAL( clicked() ), SLOT( reject() ) );

  ui.listWidget->addItems( KOPrefs::instance()->timeScaleTimezones() );
}

void TimeScaleConfigDialog::okClicked()
{
  KOPrefs::instance()->setTimeScaleTimezones( zones() );
  accept();
}

void TimeScaleConfigDialog::add()
{
  // Do not add duplicates
  for ( int i=0; i < ui.listWidget->count(); i++ )
  {
    if ( ui.listWidget->item( i )->text() == ui.zoneCombo->currentText() )
      return;
  }

  ui.listWidget->addItem( ui.zoneCombo->currentText() );
}

void TimeScaleConfigDialog::remove()
{
  delete ui.listWidget->takeItem( ui.listWidget->currentRow() );
}

void TimeScaleConfigDialog::up()
{
  int row = ui.listWidget->currentRow();
  QListWidgetItem *item = ui.listWidget->takeItem( row );
  ui.listWidget->insertItem( qMax( row - 1, 0 ), item );
  ui.listWidget->setCurrentRow( qMax( row - 1, 0 ) );
}

void TimeScaleConfigDialog::down()
{
  int row = ui.listWidget->currentRow();
  QListWidgetItem *item = ui.listWidget->takeItem( row );
  ui.listWidget->insertItem( qMin( row + 1, ui.listWidget->count() ), item );
  ui.listWidget->setCurrentRow( qMin( row + 1, ui.listWidget->count() - 1 ) );
}

QStringList TimeScaleConfigDialog::zones()
{
  QStringList list;
  for ( int i=0; i < ui.listWidget->count(); i++ )
  {
    list << ui.listWidget->item( i )->text();
  }
  return list;
}

TimeLabels::TimeLabels( const KDateTime::Spec &spec, int rows, TimeLabelsZone *parent, Qt::WFlags f) :
// TODO_QT4: Use constructor without *name=0 param
  Q3ScrollView(parent,/*name*/0,f)
{
  mTimeLabelsZone = parent;
  mSpec = spec;

  mRows = rows;
  mMiniWidth = 0;

  mCellHeight = KOPrefs::instance()->mHourSize*4;

  enableClipper(true);

  setHScrollBarMode(AlwaysOff);
  setVScrollBarMode(AlwaysOff);

  resizeContents(50, int(mRows * mCellHeight) );

  viewport()->setBackgroundRole( QPalette::Background );
  setBackgroundRole( QPalette::Background );

  mMousePos = new QFrame(this);
  mMousePos->setLineWidth( 1 );
  mMousePos->setFrameStyle( QFrame::HLine );
//  mMousePos->setMargin(0);
  QPalette pal;
  pal.setColor( QPalette::Dark,
                KOPrefs::instance()->agendaMarcusBainsLineLineColor() );
  mMousePos->setPalette( pal );
  mMousePos->setFixedSize(width(), 1);
  addChild(mMousePos, 0, 0);

  if ( mSpec.isValid() )
    setToolTip( i18n( "Timezone:" ) + mSpec.timeZone().name() );
  else
    setToolTip( i18n( "Calendar display timezone:" )
                + KOPrefs::instance()->timeSpec().timeZone().name() );

}

void TimeLabels::mousePosChanged(const QPoint &pos)
{
  moveChild(mMousePos, 0, pos.y());

  // The repaint somehow prevents that the red line leaves a black artifact when
  // moved down. It's not a full solution, though.
  repaint();
}

void TimeLabels::showMousePos()
{
  mMousePos->show();
}

void TimeLabels::hideMousePos()
{
  mMousePos->hide();
}

void TimeLabels::setCellHeight(double height)
{
  mCellHeight = height;
}

/*
  Optimization so that only the "dirty" portion of the scroll view
  is redrawn.  Unfortunately, this is not called by default paintEvent() method.
*/
void TimeLabels::drawContents( QPainter *p, int cx, int cy, int cw, int ch )
{
  int beginning;

  if ( !mSpec.isValid() )
    beginning = 0;
  else
    beginning = ( mSpec.timeZone().currentOffset()
                 - KOPrefs::instance()->timeSpec().timeZone().currentOffset() )
                 / ( 60 * 60 );


  p->setBrush( palette().background() ); // TODO: theming, see if we want sth here...
  p->drawRect( cx, cy, cw, ch);

  // bug:  the parameters cx and cw are the areas that need to be
  //       redrawn, not the area of the widget.  unfortunately, this
  //       code assumes the latter...

  // now, for a workaround...
  cx = contentsX() + frameWidth()*2;
  cw = contentsWidth();

  // end of workaround
  int cell = ((int)(cy / mCellHeight)) + beginning;  // indicates which hour we start drawing with
  double y = ( cell - beginning ) * mCellHeight;
  QFontMetrics fm = fontMetrics();
  QString hour;
  int timeHeight = fm.ascent();
  QFont hourFont = KOPrefs::instance()->agendaTimeLabelsFont();
  p->setFont( font() );

  QString suffix;
  if ( ! KGlobal::locale()->use12Clock() ) {
      suffix = "00";
  } else {
    suffix = "am";
    if (cell > 11) suffix = "pm";
  }

  // We adjust the size of the hour font to keep it reasonable
  if ( timeHeight >  mCellHeight ) {
    timeHeight = int(mCellHeight-1);
    int pointS = hourFont.pointSize();
    while ( pointS > 4 ) { // TODO: use smallestReadableFont() when added to kdelibs
      hourFont.setPointSize( pointS );
      fm = QFontMetrics( hourFont );
      if ( fm.ascent() < mCellHeight )
        break;
      -- pointS;
    }
    fm = QFontMetrics( hourFont );
    timeHeight = fm.ascent();
  }
  //timeHeight -= (timeHeight/4-2);
  QFont suffixFont = hourFont;
  suffixFont.setPointSize( suffixFont.pointSize()/2 );
  QFontMetrics fmS(  suffixFont );
  int startW = mMiniWidth - frameWidth()-2 ;
  int tw2 = fmS.width(suffix);
  int divTimeHeight = (timeHeight-1) /2 - 1;
  //testline
  //p->drawLine(0,0,0,contentsHeight());
  while (y < cy + ch+mCellHeight) {
    // hour, full line
    p->drawLine( cx, int(y), cw+2, int(y) );

    hour.setNum(cell % 24 );
    // handle different timezones
    if ( cell < 0 )
      hour.setNum( cell + 24 );
    // handle 24h and am/pm time formats
    if (KGlobal::locale()->use12Clock()) {
      if (cell == 12) suffix = "pm";
      if (cell == 0) hour.setNum(12);
      if (cell > 12) hour.setNum(cell - 12);
    }

    QPen pen;
    if ( cell < 0 || cell >= 24 ) {
      pen.setColor( QColor( 150, 150, 150 ) );
    } else {
      pen.setColor( QPalette::Text );
    }
    p->setPen( pen );

    // center and draw the time label
    int timeWidth = fm.width(hour);
    int offset = startW - timeWidth - tw2 -1 ;
    p->setFont( hourFont );
    p->drawText( offset, int(y+timeHeight), hour);
    p->setFont( suffixFont );
    offset = startW - tw2;
    p->drawText( offset, int(y+timeHeight-divTimeHeight), suffix);

    // increment indices
    y += mCellHeight;
    cell++;
  }

}

/**
   Calculates the minimum width.
*/
int TimeLabels::minimumWidth() const
{
  return mMiniWidth;
}

/** updates widget's internal state */
void TimeLabels::updateConfig()
{
  setFont(KOPrefs::instance()->agendaTimeLabelsFont());

  QString test = "20";
  if ( KGlobal::locale()->use12Clock() )
      test = "12";
  mMiniWidth = fontMetrics().width( test );
  if ( KGlobal::locale()->use12Clock() )
      test = "pm";
  else {
      test = "00";
  }
  QFont sFont = font();
  sFont.setPointSize(  sFont.pointSize()/2 );
  QFontMetrics fmS(   sFont );
  mMiniWidth += fmS.width(  test ) + frameWidth()*2+4 ;
  // update geometry restrictions based on new settings
  setFixedWidth(  mMiniWidth );

  // update HourSize
  mCellHeight = KOPrefs::instance()->mHourSize*4;
  // If the agenda is zoomed out so that more then 24 would be shown,
  // the agenda only shows 24 hours, so we need to take the cell height
  // from the agenda, which is larger than the configured one!
  if ( mCellHeight < 4*mAgenda->gridSpacingY() )
       mCellHeight = 4*mAgenda->gridSpacingY();
  resizeContents( mMiniWidth, int(mRows * mCellHeight+1) );
}

/** update time label positions */
void TimeLabels::positionChanged()
{
  int adjustment = mAgenda->contentsY();
  setContentsPos(0, adjustment);
}

/**  */
void TimeLabels::setAgenda(KOAgenda* agenda)
{
  mAgenda = agenda;

  connect(mAgenda, SIGNAL(mousePosSignal(const QPoint &)), this, SLOT(mousePosChanged(const QPoint &)));
  connect(mAgenda, SIGNAL(enterAgenda()), this, SLOT(showMousePos()));
  connect(mAgenda, SIGNAL(leaveAgenda()), this, SLOT(hideMousePos()));
  connect(mAgenda, SIGNAL(gridSpacingYChanged( double ) ), this, SLOT( setCellHeight( double ) ) );
}


/** This is called in response to repaint() */
void TimeLabels::paintEvent(QPaintEvent*)
{
//  kDebug(5850) <<"paintevent...";
  // this is another hack!
//  QPainter painter(this);
  //QString c
  repaintContents(contentsX(), contentsY(), visibleWidth(), visibleHeight());
}

void TimeLabels::contextMenuEvent( QContextMenuEvent *event )
{
  QMenu popup( this );
  QAction *editTimeZones = popup.addAction( KIcon( "edit" ), i18n( "&Edit timezones" ) );
  QAction *removeTimeZone = popup.addAction( KIcon( "delete" ), i18n( "&Remove %1 timezone", mSpec.timeZone().name() ) );
  if ( !mSpec.isValid() )
    removeTimeZone->setEnabled( false );

  QAction *activatedAction = popup.exec( QCursor::pos() );
  if ( activatedAction == editTimeZones ) {
    TimeScaleConfigDialog dialog( this );
    if ( dialog.exec() == QDialog::Accepted )
      mTimeLabelsZone->reset();
  } else if ( activatedAction == removeTimeZone ) {
    QStringList list = KOPrefs::instance()->timeScaleTimezones();
    list.removeAll( mSpec.timeZone().name() );
    KOPrefs::instance()->setTimeScaleTimezones( list );
    mTimeLabelsZone->reset();
    hide();
    deleteLater();
  }
}

KDateTime::Spec TimeLabels::timeSpec()
{
  return mSpec;
}

TimeLabelsZone::TimeLabelsZone( KOAgendaView *parent, KOAgenda *agenda)
  : QWidget( parent ), mAgenda( agenda ),  mParent( parent )
{
  mTimeLabelsLayout = new QHBoxLayout( this );
  mTimeLabelsLayout->setMargin( 0 );
  mTimeLabelsLayout->setSpacing( 0 );

  init();
}

void TimeLabelsZone::reset()
{
  foreach( TimeLabels* label, mTimeLabelsList ) {
    label->hide();
    label->deleteLater();
  }
  mTimeLabelsList.clear();

  init();

  // Update some related geometry from the agenda view
  updateAll();
  mParent->createDayLabels();
  mParent->updateTimeBarWidth();
}

void TimeLabelsZone::init()
{
  addTimeLabels( KDateTime::Spec() );

  foreach( QString zoneStr, KOPrefs::instance()->timeScaleTimezones() ) {
    KTimeZone zone = KSystemTimeZones::zone( zoneStr );
    if ( zone.isValid() )
      addTimeLabels( zone );
  }
}

void TimeLabelsZone::addTimeLabels( const KDateTime::Spec &spec )
{
  TimeLabels *labels = new TimeLabels( spec, 24, this );
  mTimeLabelsList.prepend( labels );
  mTimeLabelsLayout->insertWidget( 0, labels );
  setupTimeLabel( labels );
}

void TimeLabelsZone::setupTimeLabel( TimeLabels* timeLabel )
{
  timeLabel->setAgenda( mAgenda );
  connect( mAgenda->verticalScrollBar(), SIGNAL( valueChanged(int) ),
           timeLabel, SLOT( positionChanged() ) );
  connect( timeLabel->verticalScrollBar(), SIGNAL( valueChanged(int) ),
           mParent, SLOT( setContentsPos(int) ) );
}

int TimeLabelsZone::timeLabelsWidth()
{
  if ( mTimeLabelsList.isEmpty() )
    return 0;
  else {
    return mTimeLabelsList.first()->width() * mTimeLabelsList.count();
  }
}

void TimeLabelsZone::updateAll()
{
  foreach( TimeLabels* timeLabel, mTimeLabelsList ) {
    timeLabel->updateConfig();
    timeLabel->positionChanged();
    timeLabel->repaint();
  }
}

void TimeLabelsZone::setTimeLabelsWidth( int width )
{
  foreach( TimeLabels* timeLabel, mTimeLabelsList ) {
    timeLabel->setFixedWidth( width / mTimeLabelsList.count() );
  }
}

////////////////////////////////////////////////////////////////////////////

EventIndicator::EventIndicator( Location loc, QWidget *parent ) : QFrame( parent )
{
  mColumns = 1;
  mTopBox = 0;
  mLocation = loc;
  mTopLayout = 0;

  if (mLocation == Top) mPixmap = KOGlobals::self()->smallIcon("upindicator");
  else mPixmap = KOGlobals::self()->smallIcon("downindicator");

  setMinimumHeight(mPixmap.height());
}

EventIndicator::~EventIndicator()
{
}

void EventIndicator::drawContents(QPainter *p)
{
//  kDebug(5850) <<"======== top:" << contentsRect().top() <<" bottom"
//         << contentsRect().bottom() << " left" << contentsRect().left()
//         << " right" << contentsRect().right();

  int i;
  for(i=0;i<mColumns;++i) {
    if (mEnabled[i]) {
      int cellWidth = contentsRect().right()/mColumns;
      int xOffset = KOGlobals::self()->reverseLayout() ?
               (mColumns - 1 - i)*cellWidth + cellWidth/2 -mPixmap.width()/2 :
               i*cellWidth + cellWidth/2 -mPixmap.width()/2;
      p->drawPixmap(QPoint(xOffset,0),mPixmap);
    }
  }
}

void EventIndicator::changeColumns(int columns)
{
  mColumns = columns;
  mEnabled.resize(mColumns);

  update();
}

void EventIndicator::enableColumn(int column, bool enable)
{
  mEnabled[column] = enable;
}


#include <kcal/incidence.h>
#include <kvbox.h>

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

KOAgendaView::KOAgendaView( Calendar *cal, QWidget *parent ) :
  KOrg::AgendaView( cal, parent ),
  mExpandButton( 0 ),
  mAllowAgendaUpdate( true ),
  mUpdateItem( 0 ),
  mResource( 0 ),
  mTimeLabelsZone( 0 )
{
  mSelectedDates.append( QDate::currentDate() );

  mLayoutDayLabels = 0;
  mDayLabelsFrame = 0;
  mDayLabels = 0;
  mLayoutBottomDayLabels = 0;
  mBottomDayLabelsFrame = 0;
  mBottomDayLabels = 0;

  bool isRTL = KOGlobals::self()->reverseLayout();

  if ( KOPrefs::instance()->compactDialogs() ) {
    if ( KOPrefs::instance()->mVerticalScreen ) {
      mExpandedPixmap = KOGlobals::self()->smallIcon( "arrow-down" );
      mNotExpandedPixmap = KOGlobals::self()->smallIcon( "arrow-up" );
    } else {
      mExpandedPixmap = KOGlobals::self()->smallIcon( isRTL ? "arrow-left"
                                                            : "arrow-right" );
      mNotExpandedPixmap = KOGlobals::self()->smallIcon( isRTL ? "arrow-right"
                                                               : "arrow-left" );
    }
  }

  mTopLayout = new QGridLayout( this );
  mTopLayout->setMargin( 0 );
  /* Create agenda splitter */
#ifndef KORG_NOSPLITTER
  mSplitterAgenda = new QSplitter( Qt::Vertical, this );
  mTopLayout->addWidget( mSplitterAgenda, 1, 0 );
  mSplitterAgenda->setOpaqueResize( KGlobalSettings::opaqueResize() );
#else
  // If we don't use splitters, we still need to order the widgets nevertheless
  KVBox *mainBox = new KVBox( this );
  mTopLayout->addWidget( mainBox, 1, 0 );
#endif

  /* Create day name labels for agenda columns */
#ifndef KORG_NOSPLITTER
  mDayLabelsFrame = new KHBox( mSplitterAgenda );
#else
  mDayLabelsFrame = new KHBox( this );
  mTopLayout->addWidget( mDayLabelsFrame, 0, 0 );
#endif
  mDayLabelsFrame->setSpacing( 2 );

  /* Create all-day agenda widget */
#ifndef KORG_NOSPLITTER
  mAllDayFrame = new KHBox( mSplitterAgenda );
#else
  mAllDayFrame = new KHBox( mainBox );
#endif
  mAllDayFrame->setSpacing( 2 );

  // Alignment and description widgets
  mDummyAllDayLeft = new KVBox( mAllDayFrame );

  if ( KOPrefs::instance()->compactDialogs() ) {
    mExpandButton = new QPushButton( mDummyAllDayLeft );
    mExpandButton->setIcon( QIcon(mNotExpandedPixmap) );
    mExpandButton->setSizePolicy( QSizePolicy( QSizePolicy::Fixed,
                                               QSizePolicy::Fixed ) );
    connect( mExpandButton, SIGNAL( clicked() ), SIGNAL( toggleExpand() ) );
  } else {
    QLabel *label = new QLabel( i18n("All Day"), mDummyAllDayLeft );
    label->setAlignment( Qt::AlignRight | Qt::AlignVCenter  );
    label->setWordWrap( true );
  }

  // The widget itself
  mAllDayAgenda = new KOAgenda( 1, mAllDayFrame );
  QWidget *dummyAllDayRight = new QWidget( mAllDayFrame );

  // Create the event context menu for the all-day agenda
  mAllDayAgendaPopup = eventPopup();

  /* Create the main agenda widget and the related widgets */
#ifndef KORG_NOSPLITTER
  QWidget *agendaFrame = new QWidget( mSplitterAgenda );
#else
  QWidget *agendaFrame = new QWidget( mainBox );
#endif
  mAgendaLayout = new QGridLayout( agendaFrame );
  mAgendaLayout->setMargin( 0 );
  mAgendaLayout->setHorizontalSpacing( 2 );
  mAgendaLayout->setVerticalSpacing( 0 );

  // Create event indicator bars
  mEventIndicatorTop = new EventIndicator( EventIndicator::Top, agendaFrame );
  mAgendaLayout->addWidget( mEventIndicatorTop, 0, 1 );
  mEventIndicatorBottom = new EventIndicator( EventIndicator::Bottom,
                                              agendaFrame );
  mAgendaLayout->addWidget( mEventIndicatorBottom, 2, 1 );

  // Alignment and description widgets
  QWidget *dummyAgendaRight = new QWidget( agendaFrame );
  mAgendaLayout->addWidget( dummyAgendaRight, 0, 2 );

  // Create agenda
  mAgenda = new KOAgenda( 1, 96, KOPrefs::instance()->mHourSize, agendaFrame );
  mAgendaLayout->addWidget( mAgenda, 1, 1, 1, 2 );
  mAgendaLayout->setColumnStretch( 1, 1 );

  // Create time labels
  mTimeLabelsZone = new TimeLabelsZone( this, mAgenda );
  mAgendaLayout->addWidget( mTimeLabelsZone, 1, 0 );

  // Create event context menu for agenda
  mAgendaPopup = eventPopup();

  // Scrolling
  connect( mAgenda,
           SIGNAL( zoomView(const int, const QPoint &, const Qt::Orientation) ),
           SLOT( zoomView(const int, const QPoint &, const Qt::Orientation) ) );

  // Event indicator updates
  connect( mAgenda, SIGNAL( lowerYChanged(int) ),
           SLOT( updateEventIndicatorTop(int) ) );
  connect( mAgenda, SIGNAL( upperYChanged(int) ),
           SLOT( updateEventIndicatorBottom(int) ) );

  /* Create a frame at the bottom which may be used by decorations */
#ifndef KORG_NOSPLITTER
  mBottomDayLabelsFrame = new KHBox( mSplitterAgenda );
#else
  mBottomDayLabelsFrame = new KHBox( this );
  mTopLayout->addWidget( mBottomDayLabelsFrame, 2, 0 );
#endif
  mBottomDayLabelsFrame->setSpacing( 2 );


  /* Make the all-day and normal agendas line up with each other */
  dummyAllDayRight->setFixedWidth( mAgenda->verticalScrollBar()->width()
                                   - mAgendaLayout->horizontalSpacing() );
  updateTimeBarWidth();


  /* Update widgets to reflect user preferences */
//  updateConfig();
  createDayLabels();


  /* Connect the agendas */
  connectAgenda( mAgenda, mAgendaPopup, mAllDayAgenda );
  connectAgenda( mAllDayAgenda, mAllDayAgendaPopup, mAgenda);
}


KOAgendaView::~KOAgendaView()
{
  delete mAgendaPopup;
  delete mAllDayAgendaPopup;
}

void KOAgendaView::connectAgenda( KOAgenda *agenda, QMenu *popup,
                                  KOAgenda *otherAgenda )
{
  connect( agenda, SIGNAL( showIncidencePopupSignal( Incidence *, const QDate & ) ),
           popup, SLOT( showIncidencePopup( Incidence *, const QDate & ) ) );

  connect( agenda, SIGNAL( showNewEventPopupSignal() ),
           SLOT( showNewEventPopup() ) );

  agenda->setCalendar( calendar() );

  // Create/Show/Edit/Delete Event
  connect( agenda, SIGNAL( newEventSignal() ), SIGNAL( newEventSignal() ) );

  connect( agenda,
           SIGNAL( newTimeSpanSignal( const QPoint &, const QPoint & ) ),
           SLOT( newTimeSpanSelected( const QPoint &, const QPoint & ) ) );

  connect( agenda, SIGNAL( newStartSelectSignal() ),
           otherAgenda, SLOT( clearSelection() ) );
  connect( agenda, SIGNAL( newStartSelectSignal() ),
           SIGNAL( timeSpanSelectionChanged()) );

  connect( agenda, SIGNAL( editIncidenceSignal( Incidence * ) ),
                   SIGNAL( editIncidenceSignal( Incidence * ) ) );
  connect( agenda, SIGNAL( showIncidenceSignal( Incidence * ) ),
                   SIGNAL( showIncidenceSignal( Incidence * ) ) );
  connect( agenda, SIGNAL( deleteIncidenceSignal( Incidence * ) ),
                   SIGNAL( deleteIncidenceSignal( Incidence * ) ) );

  connect( agenda, SIGNAL( startMultiModify( const QString & ) ),
                   SIGNAL( startMultiModify( const QString & ) ) );
  connect( agenda, SIGNAL( endMultiModify() ),
                   SIGNAL( endMultiModify() ) );

  connect( agenda, SIGNAL( itemModified( KOAgendaItem * ) ),
                   SLOT( updateEventDates( KOAgendaItem * ) ) );
  connect( agenda, SIGNAL( enableAgendaUpdate( bool ) ),
                   SLOT( enableAgendaUpdate( bool ) ) );

  // drag signals
  connect( agenda, SIGNAL( startDragSignal( Incidence * ) ),
           SLOT( startDrag( Incidence * ) ) );

  // synchronize selections
  connect( agenda, SIGNAL( incidenceSelected( Incidence * ) ),
           otherAgenda, SLOT( deselectItem() ) );
  connect( agenda, SIGNAL( incidenceSelected( Incidence * ) ),
           SIGNAL( incidenceSelected( Incidence * ) ) );

  // rescheduling of todos by d'n'd
  connect( agenda, SIGNAL( droppedToDo( Todo *, const QPoint &, bool ) ),
           SLOT( slotTodoDropped( Todo *, const QPoint &, bool ) ) );

}

void KOAgendaView::zoomInVertically( )
{
  KOPrefs::instance()->mHourSize++;
  mAgenda->updateConfig();
  mAgenda->checkScrollBoundaries();

  mTimeLabelsZone->updateAll();

  updateView();

}

void KOAgendaView::zoomOutVertically( )
{

  if ( KOPrefs::instance()->mHourSize > 4 ) {

    KOPrefs::instance()->mHourSize--;
    mAgenda->updateConfig();
    mAgenda->checkScrollBoundaries();

    mTimeLabelsZone->updateAll();
    updateView();
  }
}

void KOAgendaView::zoomInHorizontally( const QDate &date)
{
  QDate begin;
  QDate newBegin;
  QDate dateToZoom = date;
  int ndays,count;

  begin = mSelectedDates.first();
  ndays = begin.daysTo( mSelectedDates.last() );

  // zoom with Action and are there a selected Incidence?, Yes, I zoom in to it.
  if ( ! dateToZoom.isValid () )
    dateToZoom=mAgenda->selectedIncidenceDate();

  if( !dateToZoom.isValid() ) {
    if ( ndays > 1 ) {
      newBegin=begin.addDays(1);
      count = ndays-1;
      emit zoomViewHorizontally ( newBegin , count );
    }
  } else {
    if ( ndays <= 2 ) {
      newBegin = dateToZoom;
      count = 1;
    } else  {
      newBegin = dateToZoom.addDays( -ndays/2 +1  );
      count = ndays -1 ;
    }
    emit zoomViewHorizontally ( newBegin , count );
  }
}

void KOAgendaView::zoomOutHorizontally( const QDate &date )
{
  QDate begin;
  QDate newBegin;
  QDate dateToZoom = date;
  int ndays,count;

  begin = mSelectedDates.first();
  ndays = begin.daysTo( mSelectedDates.last() );

  // zoom with Action and are there a selected Incidence?, Yes, I zoom out to it.
  if ( ! dateToZoom.isValid () )
    dateToZoom=mAgenda->selectedIncidenceDate();

  if ( !dateToZoom.isValid() ) {
    newBegin = begin.addDays(-1);
    count = ndays+3 ;
  } else {
    newBegin = dateToZoom.addDays( -ndays/2-1 );
    count = ndays+3;
  }

  if ( abs( count ) >= 31 )
    kDebug(5850) <<"change to the month view?";
  else
    //We want to center the date
    emit zoomViewHorizontally( newBegin, count );
}

void KOAgendaView::zoomView( const int delta, const QPoint &pos,
  const Qt::Orientation orient )
{
  static QDate zoomDate;
  static QTimer t( this );


  //Zoom to the selected incidence, on the other way
  // zoom to the date on screen after the first mousewheel move.
  if ( orient == Qt::Horizontal ) {
    QDate date=mAgenda->selectedIncidenceDate();
    if ( date.isValid() )
      zoomDate=date;
    else{
      if ( !t.isActive() ) {
        zoomDate= mSelectedDates[pos.x()];
      }
      t.setSingleShot( true );
      t.start ( 1000 );
    }
    if ( delta > 0 )
      zoomOutHorizontally( zoomDate );
    else
      zoomInHorizontally( zoomDate );
  } else {
    // Vertical zoom
    QPoint posConstentsOld = mAgenda->gridToContents(pos);
    if ( delta > 0 ) {
      zoomOutVertically();
    } else {
      zoomInVertically();
    }
    QPoint posConstentsNew = mAgenda->gridToContents(pos);
    mAgenda->scrollBy( 0, posConstentsNew.y() - posConstentsOld.y() );
  }
}

void KOAgendaView::createDayLabels()
{
//  kDebug(5850) <<"KOAgendaView::createDayLabels()";

  // ### Before deleting and recreating we could check if mSelectedDates changed...
  // It would remove some flickering and gain speed (since this is called by
  // each updateView() call)
  delete mDayLabels;
  delete mBottomDayLabels;

  mDayLabels = new QFrame (mDayLabelsFrame);
  mLayoutDayLabels = new QHBoxLayout(mDayLabels);
  mLayoutDayLabels->setMargin(0);
  KVBox *weekLabelBox = new KVBox( mDayLabels );
  mLayoutDayLabels->addWidget( weekLabelBox );
  weekLabelBox->setFixedWidth( mTimeLabelsZone->width()
                               - mAgendaLayout->horizontalSpacing() );

  mBottomDayLabels = new QFrame (mBottomDayLabelsFrame);
  mBottomDayLabelsFrame->setStretchFactor(mBottomDayLabels, 1);
  mLayoutBottomDayLabels = new QHBoxLayout(mBottomDayLabels);
  mLayoutBottomDayLabels->setMargin(0);
  KVBox *bottomWeekLabelBox = new KVBox( mBottomDayLabels );
  mLayoutBottomDayLabels->addWidget( bottomWeekLabelBox );
  bottomWeekLabelBox->setFixedWidth( mTimeLabelsZone->width()
                                     - mAgendaLayout->horizontalSpacing() );

  const KCalendarSystem *calsys = KOGlobals::self()->calendarSystem();

#ifndef KORG_NOPLUGINS
#ifndef KORG_NOSPLITTER
  if ( KOPrefs::instance()->decorationsAtAgendaViewTop().count() > 0 ) {
    mDayLabelsFrame->setParent( mSplitterAgenda );
  } else {
    mDayLabelsFrame->setParent( this );
    mTopLayout->addWidget( mDayLabelsFrame, 0, 0 );
  }
  if ( KOPrefs::instance()->decorationsAtAgendaViewBottom().count() > 0 ) {
    mBottomDayLabelsFrame->setParent( mSplitterAgenda );
  } else {
    mBottomDayLabelsFrame->setParent( this );
    mTopLayout->addWidget( mBottomDayLabelsFrame, 0, 0 );
  }
#endif
#endif

  DateList::ConstIterator dit;
  for( dit = mSelectedDates.begin(); dit != mSelectedDates.end(); ++dit ) {
    QDate date = *dit;
    KVBox *dayLabelBox = new KVBox( mDayLabels );
    mLayoutDayLabels->addWidget( dayLabelBox );
    KVBox *bottomDayLabelBox = new KVBox( mBottomDayLabels );
    mLayoutBottomDayLabels->addWidget( bottomDayLabelBox );

    int dW = calsys->dayOfWeek(date);
    QString veryLongStr = KGlobal::locale()->formatDate( date );
    QString longstr = i18nc( "short_weekday date (e.g. Mon 13)","%1 %2" ,
          calsys->weekDayName( dW, KCalendarSystem::ShortDayName ) ,
          calsys->day(date) );
    QString shortstr = QString::number(calsys->day(date));

    KOAlternateLabel *dayLabel = new KOAlternateLabel(shortstr,
      longstr, veryLongStr, dayLabelBox);
    dayLabel->setMinimumWidth(1);
    dayLabel->setAlignment(Qt::AlignHCenter);
    if (date == QDate::currentDate()) {
      QFont font = dayLabel->font();
      font.setBold(true);
      dayLabel->setFont(font);
    }

    // if a holiday region is selected, show the holiday name
    QStringList texts = KOGlobals::self()->holiday( date );
    QStringList::ConstIterator textit = texts.begin();
    for ( ; textit != texts.end(); ++textit ) {
      // use a KOAlternateLabel so when the text doesn't fit any more a tooltip is used
      KOAlternateLabel*label = new KOAlternateLabel( (*textit), (*textit), QString(), dayLabelBox );
      label->setMinimumWidth(1);
      label->setAlignment(Qt::AlignCenter);
    }

#ifndef KORG_NOPLUGINS
    foreach ( QString decoName,
              KOPrefs::instance()->decorationsAtAgendaViewTop() ) {
      if ( KOPrefs::instance()->selectedPlugins().contains( decoName ) ) {
        CalendarDecoration::Decoration* deco
          = KOCore::self()->loadCalendarDecoration( decoName );

        CalendarDecoration::Element::List elements;
        elements = deco->dayElements( date );
        if ( elements.count() > 0 ) {
          KHBox *decoHBox = new KHBox( dayLabelBox );
          decoHBox->setFrameShape( QFrame::StyledPanel );
          decoHBox->setMinimumWidth( 1 );

          foreach ( CalendarDecoration::Element* it, elements ) {
            kDebug() << "adding Element " << it->id() << " of Decoration "
                     << deco->info() << " to the top of the agenda view";
            KODecorationLabel *label = new KODecorationLabel( it, decoHBox );
            label->setAlignment( Qt::AlignBottom );
            label->setMinimumWidth( 1 );
          }
        }
      }
    }

    foreach ( QString decoName,
              KOPrefs::instance()->decorationsAtAgendaViewBottom() ) {
      if ( KOPrefs::instance()->selectedPlugins().contains( decoName ) ) {
        CalendarDecoration::Decoration* deco
          = KOCore::self()->loadCalendarDecoration( decoName );

        CalendarDecoration::Element::List elements;
        elements = deco->dayElements( date );
        if ( elements.count() > 0 ) {
          KHBox *decoHBox = new KHBox( bottomDayLabelBox );
          decoHBox->setFrameShape( QFrame::StyledPanel );
          decoHBox->setMinimumWidth( 1 );

          foreach ( CalendarDecoration::Element* it, elements ) {
            kDebug() << "adding Element " << it->id() << " of Decoration "
                     << deco->info() << " to the bottom of the agenda view";
            KODecorationLabel *label = new KODecorationLabel( it, decoHBox );
            label->setAlignment( Qt::AlignBottom );
            label->setMinimumWidth( 1 );
          }
        }
      }
    }
#endif
  }

#ifndef KORG_NOPLUGINS
  // Week decoration labels
  foreach ( QString decoName,
            KOPrefs::instance()->decorationsAtAgendaViewTop() ) {
    if ( KOPrefs::instance()->selectedPlugins().contains( decoName ) ) {
      CalendarDecoration::Decoration* deco
        = KOCore::self()->loadCalendarDecoration( decoName );

      CalendarDecoration::Element::List elements;
      elements = deco->weekElements( mSelectedDates.first() );
      if ( elements.count() > 0 ) {
        KHBox *decoHBox = new KHBox( weekLabelBox );
        decoHBox->setFrameShape( QFrame::StyledPanel );
        decoHBox->setMinimumWidth( 1 );

        foreach ( CalendarDecoration::Element* it, elements ) {
          kDebug() << "adding Element " << it->id() << " of Decoration "
                   << deco->info() << " to the week part of the agenda view";
          KODecorationLabel *label = new KODecorationLabel( it, decoHBox );
          label->setAlignment( Qt::AlignBottom );
          label->setMinimumWidth( 1 );
        }
      }
    }
  }
  foreach ( QString decoName,
            KOPrefs::instance()->decorationsAtAgendaViewBottom() ) {
    if ( KOPrefs::instance()->selectedPlugins().contains( decoName ) ) {
      CalendarDecoration::Decoration* deco
         = KOCore::self()->loadCalendarDecoration( decoName );

      CalendarDecoration::Element::List elements;
      elements = deco->weekElements( mSelectedDates.first() );
      if ( elements.count() > 0 ) {
        KHBox *decoHBox = new KHBox( bottomWeekLabelBox );
        decoHBox->setFrameShape( QFrame::StyledPanel );
        decoHBox->setMinimumWidth( 1 );

        foreach ( CalendarDecoration::Element* it, elements ) {
          kDebug() << "adding Element " << it->id() << " of Decoration "
                   << deco->info() << " to the week part of the agenda view";
          KODecorationLabel *label = new KODecorationLabel( it, decoHBox );
          label->setAlignment( Qt::AlignBottom );
          label->setMinimumWidth( 1 );
        }
      }
    }
  }
#endif

  mLayoutDayLabels->addSpacing(mAgenda->verticalScrollBar()->width());
  mDayLabels->show();
  mLayoutBottomDayLabels->addSpacing(mAgenda->verticalScrollBar()->width());
  mBottomDayLabels->show();
}

void KOAgendaView::enableAgendaUpdate( bool enable )
{
  mAllowAgendaUpdate = enable;
}

int KOAgendaView::maxDatesHint()
{
  // Not sure about the max number of events, so return 0 for now.
  return 0;
}

int KOAgendaView::currentDateCount()
{
  return mSelectedDates.count();
}

Incidence::List KOAgendaView::selectedIncidences()
{
  Incidence::List selected;
  Incidence *incidence;

  incidence = mAgenda->selectedIncidence();
  if (incidence) selected.append(incidence);

  incidence = mAllDayAgenda->selectedIncidence();
  if (incidence) selected.append(incidence);

  return selected;
}

DateList KOAgendaView::selectedDates()
{
  DateList selected;
  QDate qd;

  qd = mAgenda->selectedIncidenceDate();
  if (qd.isValid()) selected.append(qd);

  qd = mAllDayAgenda->selectedIncidenceDate();
  if (qd.isValid()) selected.append(qd);

  return selected;
}

bool KOAgendaView::eventDurationHint( QDateTime &startDt, QDateTime &endDt,
                                      bool &allDay )
{
  if ( selectionStart().isValid() ) {
    QDateTime start = selectionStart();
    QDateTime end = selectionEnd();

    if ( start.secsTo( end ) == 15*60 ) {
      // One cell in the agenda view selected, e.g.
      // because of a double-click, => Use the default duration
      QTime defaultDuration( KOPrefs::instance()->mDefaultDuration.time() );
      int addSecs = ( defaultDuration.hour()*3600 ) +
                    ( defaultDuration.minute()*60 );
      end = start.addSecs( addSecs );
    }

    startDt = start;
    endDt = end;
    allDay = selectedIsAllDay();
    return true;
  }
  return false;
}

/** returns if only a single cell is selected, or a range of cells */
bool KOAgendaView::selectedIsSingleCell()
{
  if ( !selectionStart().isValid() || !selectionEnd().isValid() ) return false;

  if (selectedIsAllDay()) {
    int days = selectionStart().daysTo(selectionEnd());
    return ( days < 1 );
  } else {
    int secs = selectionStart().secsTo(selectionEnd());
    return ( secs <= 24*60*60/mAgenda->rows() );
  }
}


void KOAgendaView::updateView()
{
//  kDebug(5850) <<"KOAgendaView::updateView()";
  fillAgenda();
}


/*
  Update configuration settings for the agenda view. This method is not
  complete.
*/
void KOAgendaView::updateConfig()
{
//  kDebug(5850) <<"KOAgendaView::updateConfig()";

  mAgenda->updateConfig();
  mAllDayAgenda->updateConfig();

  mTimeLabelsZone->updateAll();

  updateTimeBarWidth();

#ifdef __GNUC__
#warning port me!
#endif
#if 0
  // ToolTips displaying summary of events
  KOAgendaItem::toolTipGroup()->setEnabled(KOPrefs::instance()
                                           ->mEnableToolTips);
#endif

  setHolidayMasks();

  createDayLabels();

  updateView();
}



void KOAgendaView::updateTimeBarWidth()
{
  int width = qMax( mDummyAllDayLeft->fontMetrics().width( i18n("All Day") ),
                    mTimeLabelsZone->timeLabelsWidth() );

  mDummyAllDayLeft->setFixedWidth( width );
  mTimeLabelsZone->setTimeLabelsWidth( width );
}


void KOAgendaView::updateEventDates( KOAgendaItem *item )
{
  kDebug(5850) <<"KOAgendaView::updateEventDates():" << item->text();

  KDateTime startDt,endDt;

  // Start date of this incidence, calculate the offset from it (so recurring and
  // non-recurring items can be treated exactly the same, we never need to check
  // for recurs(), because we only move the start day by the number of days the
  // agenda item was really moved. Smart, isn't it?)
  QDate thisDate;
  if ( item->cellXLeft() < 0 ) {
    thisDate = ( mSelectedDates.first() ).addDays( item->cellXLeft() );
  } else {
    thisDate = mSelectedDates[ item->cellXLeft() ];
  }
  QDate oldThisDate( item->itemDate() );
  int daysOffset = oldThisDate.daysTo( thisDate );
  int daysLength = 0;
//  startDt.setDate( startDate );

  Incidence *incidence = item->incidence();
  if ( !incidence ) return;
  if ( !mChanger || !mChanger->beginChange(incidence) ) return;
  Incidence *oldIncidence = incidence->clone();

  QTime startTime(0,0,0), endTime(0,0,0);
  if ( incidence->allDay() ) {
    daysLength = item->cellWidth() - 1;
  } else {
    startTime = mAgenda->gyToTime( item->cellYTop() );
    if ( item->lastMultiItem() ) {
      endTime = mAgenda->gyToTime( item->lastMultiItem()->cellYBottom() + 1 );
      daysLength = item->lastMultiItem()->cellXLeft() - item->cellXLeft();
    } else {
      endTime = mAgenda->gyToTime( item->cellYBottom() + 1 );
    }
  }

//  kDebug(5850) <<"KOAgendaView::updateEventDates(): now setting dates";
  // FIXME: use a visitor here
  if ( incidence->type() == "Event" ) {
    startDt = incidence->dtStart();
    // convert to calendar timespec because we then manipulate it with time coming from
    // the calendar
    startDt = startDt.toTimeSpec( KOPrefs::instance()->timeSpec() );
    startDt = startDt.addDays( daysOffset );
    if ( !startDt.isDateOnly() )
      startDt.setTime( startTime );
    endDt = startDt.addDays( daysLength );
    if ( !endDt.isDateOnly() )
      endDt.setTime( endTime );
    Event*ev = static_cast<Event*>(incidence);
    if( incidence->dtStart().toTimeSpec( KOPrefs::instance()->timeSpec() ) == startDt
        && ev->dtEnd().toTimeSpec( KOPrefs::instance()->timeSpec() ) == endDt ) {
      // No change
      delete oldIncidence;
      return;
    }
  } else if ( incidence->type() == "Todo" ) {
    Todo *td = static_cast<Todo*>(incidence);

    startDt = td->hasStartDate() ? td->dtStart() : td->dtDue();
    // convert to calendar timespec because we then manipulate it with time coming from
    // the calendar
    startDt = startDt.toTimeSpec( KOPrefs::instance()->timeSpec() );
    startDt.setDate( thisDate.addDays( td->dtDue().daysTo( startDt ) ) );
    if ( !startDt.isDateOnly() ) {
      startDt.setTime( startTime );
    }

    endDt = startDt;
    endDt.setDate( thisDate );
    if ( !endDt.isDateOnly() )
      endDt.setTime( endTime );

    if( td->dtDue().toTimeSpec( KOPrefs::instance()->timeSpec() )  == endDt ) {
      // No change
      delete oldIncidence;
      return;
    }
  }
  // FIXME: Adjusting the recurrence should really go to CalendarView so this
  // functionality will also be available in other views!
  // TODO_Recurrence: This does not belong here, and I'm not really sure
  // how it's supposed to work anyway.
/* Recurrence *recur = incidence->recurrence();
  if ( recur->recurs() && daysOffset != 0 ) {
    switch ( recur->recurrenceType() ) {
      case Recurrence::rYearlyPos: {
        int freq = recur->frequency();
        int duration = recur->duration();
        QDate endDt( recur->endDate() );
        bool negative = false;

        QPtrList<Recurrence::rMonthPos> monthPos( recur->yearMonthPositions() );
        if ( monthPos.first() ) {
          negative = monthPos.first()->negative;
        }
        QBitArray days( 7 );
        int pos = 0;
        days.fill( false );
        days.setBit( thisDate.dayOfWeek() - 1 );
        if ( negative ) {
          pos =  - ( thisDate.daysInMonth() - thisDate.day() - 1 ) / 7 - 1;
        } else {
          pos =  ( thisDate.day()-1 ) / 7 + 1;
        }
        // Terrible hack: to change the month days, I have to unset the recurrence, and set all days manually again
        recur->unsetRecurs();
        if ( duration != 0 ) {
          recur->setYearly( Recurrence::rYearlyPos, freq, duration );
        } else {
          recur->setYearly( Recurrence::rYearlyPos, freq, endDt );
        }
        recur->addYearlyMonthPos( pos, days );
        recur->addYearlyNum( thisDate.month() );

        break; }
        case Recurrence::rYearlyDay: {
          int freq = recur->frequency();
          int duration = recur->duration();
          QDate endDt( recur->endDate() );
        // Terrible hack: to change the month days, I have to unset the recurrence, and set all days manually again
          recur->unsetRecurs();
          if ( duration == 0 ) { // end by date
            recur->setYearly( Recurrence::rYearlyDay, freq, endDt );
          } else {
            recur->setYearly( Recurrence::rYearlyDay, freq, duration );
          }
          recur->addYearlyNum( thisDate.dayOfYear() );
          break; }
          case Recurrence::rYearlyMonth: {
            int freq = recur->frequency();
            int duration = recur->duration();
            QDate endDt( recur->endDate() );
        // Terrible hack: to change the month days, I have to unset the recurrence, and set all days manually again
            recur->unsetRecurs();
            if ( duration != 0 ) {
              recur->setYearlyByDate( thisDate.day(), recur->feb29YearlyType(), freq, duration );
            } else {
              recur->setYearlyByDate( thisDate.day(), recur->feb29YearlyType(), freq, endDt );
            }
            recur->addYearlyNum( thisDate.month() );
            break; }
            case Recurrence::rMonthlyPos: {
              int freq = recur->frequency();
              int duration = recur->duration();
              QDate endDt( recur->endDate() );
              QPtrList<Recurrence::rMonthPos> monthPos( recur->monthPositions() );
              if ( !monthPos.isEmpty() ) {
          // FIXME: How shall I adapt the day x of week Y if we move the date across month borders???
          // for now, just use the date of the moved item and assume the recurrence only occurs on that day.
          // That's fine for korganizer, but might mess up other organizers.
                QBitArray rDays( 7 );
                rDays = monthPos.first()->rDays;
                bool negative = monthPos.first()->negative;
                int newPos;
                rDays.fill( false );
                rDays.setBit( thisDate.dayOfWeek() - 1 );
                if ( negative ) {
                  newPos =  - ( thisDate.daysInMonth() - thisDate.day() - 1 ) / 7 - 1;
                } else {
                  newPos =  ( thisDate.day()-1 ) / 7 + 1;
                }

          // Terrible hack: to change the month days, I have to unset the recurrence, and set all days manually again
                recur->unsetRecurs();
                if ( duration == 0 ) { // end by date
                  recur->setMonthly( Recurrence::rMonthlyPos, freq, endDt );
                } else {
                  recur->setMonthly( Recurrence::rMonthlyPos, freq, duration );
                }
                recur->addMonthlyPos( newPos, rDays );
              }
              break;}
              case Recurrence::rMonthlyDay: {
                int freq = recur->frequency();
                int duration = recur->duration();
                QDate endDt( recur->endDate() );
                QPtrList<int> monthDays( recur->monthDays() );
        // Terrible hack: to change the month days, I have to unset the recurrence, and set all days manually again
                recur->unsetRecurs();
                if ( duration == 0 ) { // end by date
                  recur->setMonthly( Recurrence::rMonthlyDay, freq, endDt );
                } else {
                  recur->setMonthly( Recurrence::rMonthlyDay, freq, duration );
                }
        // FIXME: How shall I adapt the n-th day if we move the date across month borders???
        // for now, just use the date of the moved item and assume the recurrence only occurs on that day.
        // That's fine for korganizer, but might mess up other organizers.
                recur->addMonthlyDay( thisDate.day() );

                break;}
                case Recurrence::rWeekly: {
                  QBitArray days(7), oldDays( recur->days() );
                  int offset = daysOffset % 7;
                  if ( offset<0 ) offset = (offset+7) % 7;
        // rotate the days
                  for (int d=0; d<7; d++ ) {
                    days.setBit( (d+offset) % 7, oldDays.at(d) );
                  }
                  if ( recur->duration() == 0 ) { // end by date
                    recur->setWeekly( recur->frequency(), days, recur->endDate(), recur->weekStart() );
                  } else { // duration or no end
                    recur->setWeekly( recur->frequency(), days, recur->duration(), recur->weekStart() );
                  }
                  break;}
      // nothing to be done for the following:
      case Recurrence::rDaily:
      case Recurrence::rHourly:
      case Recurrence::rMinutely:
      case Recurrence::rNone:
      default:
        break;
    }
    if ( recur->duration()==0 ) { // end by date
      recur->setEndDate( recur->endDate().addDays( daysOffset ) );
    }
    KMessageBox::information( this, i18n("A recurring calendar item was moved "
                              "to a different day. The recurrence settings "
                              "have been updated with that move. Please check "
                              "them in the editor."),
                              i18n("Recurrence Moved"),
                              "RecurrenceMoveInAgendaWarning" );
  }*/

  // FIXME: use a visitor here
  if ( incidence->type() == "Event" ) {
    incidence->setDtStart( startDt.toTimeSpec( incidence->dtStart().timeSpec() ) );
    (static_cast<Event*>( incidence ) )->setDtEnd( endDt.toTimeSpec( incidence->dtEnd().timeSpec() ) );
  } else if ( incidence->type() == "Todo" ) {
    Todo *td = static_cast<Todo*>( incidence );
    if ( td->hasStartDate() )
      td->setDtStart( startDt.toTimeSpec( incidence->dtStart().timeSpec() ) );
    td->setDtDue( endDt.toTimeSpec( td->dtDue().timeSpec() ) );
  }
  item->setItemDate( startDt.toTimeSpec( KOPrefs::instance()->timeSpec() ).date() );

  item->setToolTip( IncidenceFormatter::toolTipString( incidence ));

  mChanger->changeIncidence( oldIncidence, incidence );
  mChanger->endChange(incidence);
  delete oldIncidence;

  // don't update the agenda as the item already has the correct coordinates.
  // an update would delete the current item and recreate it, but we are still
  // using a pointer to that item! => CRASH
  enableAgendaUpdate( false );
  // We need to do this in a timer to make sure we are not deleting the item
  // we are currently working on, which would lead to crashes
  // Only the actually moved agenda item is already at the correct position and mustn't be
  // recreated. All others have to!!!
  if ( incidence->recurs() ) {
    mUpdateItem = incidence;
    QTimer::singleShot( 0, this, SLOT( doUpdateItem() ) );
  }

    enableAgendaUpdate( true );

//  kDebug(5850) <<"KOAgendaView::updateEventDates() done";
}

void KOAgendaView::doUpdateItem()
{
  if ( mUpdateItem ) {
    changeIncidenceDisplay( mUpdateItem, KOGlobals::INCIDENCEEDITED );
    mUpdateItem = 0;
  }
}



void KOAgendaView::showDates( const QDate &start, const QDate &end )
{
//  kDebug(5850) <<"KOAgendaView::selectDates";

  mSelectedDates.clear();

  QDate d = start;
  while (d <= end) {
    mSelectedDates.append(d);
    d = d.addDays( 1 );
  }

  // and update the view
  fillAgenda();
}


void KOAgendaView::showIncidences( const Incidence::List &incidences )
{
  // we must check if they are not filtered; if they are, remove the filter
  CalFilter *filter = calendar()->filter();
  bool wehaveall = true;
  if ( filter )
    for ( Incidence::List::ConstIterator it = incidences.constBegin();
        it != incidences.constEnd(); ++it )
      if ( !( wehaveall = filter->filterIncidence( *it ) ) )
        break;

  if ( !wehaveall )
    calendar()->setFilter( 0 );

  KDateTime start = incidences.first()->dtStart().toTimeSpec( KOPrefs::instance()->timeSpec() ),
            end = incidences.first()->dtEnd().toTimeSpec( KOPrefs::instance()->timeSpec() );
  Incidence *first = incidences.first();
  for ( Incidence::List::ConstIterator it = incidences.constBegin();
        it != incidences.constEnd(); ++it ) {
    if ( ( *it )->dtStart().toTimeSpec( KOPrefs::instance()->timeSpec() ) < start )
      first = *it;
    start = qMin( start, ( *it )->dtStart().toTimeSpec( KOPrefs::instance()->timeSpec() ) );
    end = qMax( start, ( *it )->dtEnd().toTimeSpec( KOPrefs::instance()->timeSpec() ) );
  }

  end.toTimeSpec( start );    // allow direct comparison of dates
  if ( start.date().daysTo( end.date() ) + 1 <= currentDateCount() )
    showDates( start.date(), end.date() );
  else
    showDates( start.date(), start.date().addDays( currentDateCount() - 1 ) );

  mAgenda->selectItemByUID( first->uid() );
}

void KOAgendaView::insertIncidence( Incidence *incidence, const QDate &curDate,
                                    int curCol )
{
  if ( !filterByResource( incidence ) )
    return;

  // FIXME: Use a visitor here, or some other method to get rid of the dynamic_cast's
  Event *event = dynamic_cast<Event *>( incidence );
  Todo  *todo  = dynamic_cast<Todo  *>( incidence );

  if ( curCol < 0 ) {
    curCol = mSelectedDates.indexOf( curDate );
  }
  // The date for the event is not displayed, just ignore it
  if ( curCol < 0 || curCol > int( mSelectedDates.size() ) )
    return;

  int beginX;
  int endX;
  if ( event ) {
    beginX = curDate.daysTo( incidence->dtStart().toTimeSpec( KOPrefs::instance()->timeSpec() ).date() ) + curCol;
    endX = curDate.daysTo( event->dtEnd().toTimeSpec( KOPrefs::instance()->timeSpec() ).date() ) + curCol;
  } else if ( todo ) {
    if ( ! todo->hasDueDate() ) return;  // todo shall not be displayed if it has no date
    beginX = curDate.daysTo( todo->dtDue().date() ) + curCol;
    endX = beginX;
  } else {
    return;
  }
  if ( todo && todo->isOverdue() ) {
    mAllDayAgenda->insertAllDayItem( incidence, curDate, curCol, curCol );
  } else if ( incidence->allDay() ) {
// FIXME: This breaks with recurring multi-day events!
    if ( incidence->recurrence()->recurs() ) {
      mAllDayAgenda->insertAllDayItem( incidence, curDate, curCol, curCol );
    } else {
      // Insert multi-day events only on the first day, otherwise it will
      // appear multiple times
      if ( ( beginX <= 0 && curCol == 0 ) || beginX == curCol ) {
        mAllDayAgenda->insertAllDayItem( incidence, curDate, beginX, endX );
      }
    }
  } else if ( event && event->isMultiDay( KOPrefs::instance()->timeSpec() ) ) {
    int startY = mAgenda->timeToY( event->dtStart().toTimeSpec( KOPrefs::instance()->timeSpec() ).time() );
    QTime endtime( event->dtEnd().toTimeSpec( KOPrefs::instance()->timeSpec() ).time() );
    if ( endtime == QTime( 0, 0, 0 ) ) endtime = QTime( 23, 59, 59 );
    int endY = mAgenda->timeToY( endtime ) - 1;
    if ( (beginX <= 0 && curCol == 0) || beginX == curCol ) {
      mAgenda->insertMultiItem( event, curDate, beginX, endX, startY, endY );
    }
    if ( beginX == curCol ) {
      mMaxY[curCol] = mAgenda->timeToY( QTime(23,59) );
      if ( startY < mMinY[curCol] ) mMinY[curCol] = startY;
    } else if ( endX == curCol ) {
      mMinY[curCol] = mAgenda->timeToY( QTime(0,0) );
      if ( endY > mMaxY[curCol] ) mMaxY[curCol] = endY;
    } else {
      mMinY[curCol] = mAgenda->timeToY( QTime(0,0) );
      mMaxY[curCol] = mAgenda->timeToY( QTime(23,59) );
    }
  } else {
    int startY = 0, endY = 0;
    if ( event ) {
      startY = mAgenda->timeToY( incidence->dtStart().toTimeSpec( KOPrefs::instance()->timeSpec() ).time() );
      QTime endtime( event->dtEnd().toTimeSpec( KOPrefs::instance()->timeSpec() ).time() );
      if ( endtime == QTime( 0, 0, 0 ) ) endtime = QTime( 23, 59, 59 );
      endY = mAgenda->timeToY( endtime ) - 1;
    }
    if ( todo ) {
      QTime t = todo->dtDue().toTimeSpec( KOPrefs::instance()->timeSpec() ).time();
      endY = mAgenda->timeToY( t ) - 1;
      startY = mAgenda->timeToY( t.addSecs( -1800 ) );
    }
    if ( endY < startY ) endY = startY;
    mAgenda->insertItem( incidence, curDate, curCol, startY, endY );
    if ( startY < mMinY[curCol] ) mMinY[curCol] = startY;
    if ( endY > mMaxY[curCol] ) mMaxY[curCol] = endY;
  }
}

void KOAgendaView::changeIncidenceDisplayAdded( Incidence *incidence )
{
  Todo *todo = dynamic_cast<Todo *>(incidence);
  CalFilter *filter = calendar()->filter();
  if ( filter && !filter->filterIncidence( incidence ) ||
     ( todo && !KOPrefs::instance()->showAllDayTodo() ) )
    return;

  QDate f = mSelectedDates.first();
  QDate l = mSelectedDates.last();

  QDate startDt = incidence->dtStart().toTimeSpec( KOPrefs::instance()->timeSpec() ).date();

  if ( incidence->recurs() ) {
    DateList::ConstIterator dit;
    QDate curDate;
    for( dit = mSelectedDates.begin(); dit != mSelectedDates.end(); ++dit ) {
      curDate = *dit;
// FIXME: This breaks with recurring multi-day events!
      if ( incidence->recursOn( curDate, KOPrefs::instance()->timeSpec() ) ) {
        insertIncidence( incidence, curDate );
      }
    }
    return;
  }

  QDate endDt;
  if ( incidence->type() == "Event" )
    endDt = (static_cast<Event *>(incidence))->dtEnd().toTimeSpec( KOPrefs::instance()->timeSpec() ).date();
  if ( todo ) {
    endDt = todo->isOverdue() ? QDate::currentDate()
                              : todo->dtDue().date();

    if ( endDt >= f && endDt <= l ) {
      insertIncidence( incidence, endDt );
      return;
    }
  }
  if ( startDt >= f && startDt <= l ) {
    insertIncidence( incidence, startDt );
  }
}

void KOAgendaView::changeIncidenceDisplay( Incidence *incidence, int mode )
{
  switch ( mode ) {
    case KOGlobals::INCIDENCEADDED: {
        //  Add an event. No need to recreate the whole view!
        // recreating everything even causes troubles: dropping to the day matrix
        // recreates the agenda items, but the evaluation is still in an agendaItems' code,
        // which was deleted in the mean time. Thus KOrg crashes...
      changeIncidenceDisplayAdded( incidence );
      break;
    }
    case KOGlobals::INCIDENCEEDITED: {
      if ( !mAllowAgendaUpdate ) {
        updateEventIndicators();
      } else {
        removeIncidence( incidence );
        updateEventIndicators();
        changeIncidenceDisplayAdded( incidence );
      }
      break;
    }
    case KOGlobals::INCIDENCEDELETED: {
      mAgenda->removeIncidence( incidence );
      mAllDayAgenda->removeIncidence( incidence );
      updateEventIndicators();
      break;
    }
    default:
      updateView();
  }
}

void KOAgendaView::fillAgenda( const QDate & )
{
  fillAgenda();
}

void KOAgendaView::fillAgenda()
{
  /* Remember the uids of the selected items. In case one of the
   * items was deleted and re-added, we want to reselect it. */
  const QString &selectedAgendaUid = mAgenda->lastSelectedUid();
  const QString &selectedAllDayAgendaUid = mAllDayAgenda->lastSelectedUid();

  enableAgendaUpdate( true );
  clearView();

  mAllDayAgenda->changeColumns(mSelectedDates.count());
  mAgenda->changeColumns(mSelectedDates.count());
  mEventIndicatorTop->changeColumns(mSelectedDates.count());
  mEventIndicatorBottom->changeColumns(mSelectedDates.count());

  createDayLabels();
  setHolidayMasks();

  mMinY.resize(mSelectedDates.count());
  mMaxY.resize(mSelectedDates.count());

  Event::List dayEvents;

  // ToDo items shall be displayed for the day they are due, but only shown today if they are already overdue.
  // Therefore, get all of them.
  Todo::List todos  = calendar()->todos();

  mAgenda->setDateList(mSelectedDates);

  QDate today = QDate::currentDate();

  bool somethingReselected = false;
  DateList::ConstIterator dit;
  int curCol = 0;
  for( dit = mSelectedDates.begin(); dit != mSelectedDates.end(); ++dit ) {
    QDate currentDate = *dit;
//    kDebug(5850) <<"KOAgendaView::fillAgenda():" << currentDate.toString();

    dayEvents = calendar()->events(currentDate, KOPrefs::instance()->timeSpec(),
                                   EventSortStartDate,
                                   SortDirectionAscending);

    // Default values, which can never be reached
    mMinY[curCol] = mAgenda->timeToY(QTime(23,59)) + 1;
    mMaxY[curCol] = mAgenda->timeToY(QTime(0,0)) - 1;

    int numEvent;
    for(numEvent=0;numEvent<dayEvents.count();++numEvent) {
      Event *event = dayEvents.at(numEvent);
//      kDebug(5850) <<" Event:" << event->summary();
      insertIncidence( event, currentDate, curCol );
      if( event->uid() == selectedAgendaUid && !selectedAgendaUid.isNull() ) {
        mAgenda->selectItemByUID( event->uid() );
        somethingReselected = true;
      }
      if( event->uid() == selectedAllDayAgendaUid && !selectedAllDayAgendaUid.isNull() ) {
        mAllDayAgenda->selectItemByUID( event->uid() );
        somethingReselected = true;
      }

    }
//    if (numEvent == 0) kDebug(5850) <<" No events";


    // ---------- [display Todos --------------
    if ( KOPrefs::instance()->showAllDayTodo() ) {
      int numTodo;
      for (numTodo = 0; numTodo < todos.count(); ++numTodo) {
        Todo *todo = todos.at(numTodo);

        if ( ! todo->hasDueDate() ) continue;  // todo shall not be displayed if it has no date

        // ToDo items shall be displayed for the day they are due, but only showed today if they are already overdue.
        // Already completed items can be displayed on their original due date
        bool overdue = todo->isOverdue();

        if ( (( todo->dtDue().date() == currentDate) && !overdue) ||
             (( currentDate == today) && overdue) ||
             ( todo->recursOn( currentDate, KOPrefs::instance()->timeSpec() ) ) ) {
          if ( todo->allDay() || overdue ) {  // Todo has no due-time set or is already overdue
            //kDebug(5850) <<"todo without time:" << todo->dtDueDateStr() <<";" << todo->summary();

            mAllDayAgenda->insertAllDayItem(todo, currentDate, curCol, curCol);
          } else {
            //kDebug(5850) <<"todo with time:" << todo->dtDueStr() <<";" << todo->summary();

            int endY = mAgenda->timeToY(todo->dtDue().toTimeSpec( KOPrefs::instance()->timeSpec() ).time()) - 1;
            int startY = endY - 1;

            mAgenda->insertItem(todo,currentDate,curCol,startY,endY);

            if (startY < mMinY[curCol]) mMinY[curCol] = startY;
            if (endY > mMaxY[curCol]) mMaxY[curCol] = endY;
          }
        }
      }
    }
    // ---------- display Todos] --------------

    ++curCol;
  }

  mAgenda->checkScrollBoundaries();
  updateEventIndicators();

//  mAgenda->viewport()->update();
//  mAllDayAgenda->viewport()->update();

// make invalid
  deleteSelectedDateTime();

  if( !somethingReselected ) {
    emit incidenceSelected( 0 );
  }

//  kDebug(5850) <<"Fill Agenda done";
}

void KOAgendaView::clearView()
{
//  kDebug(5850) <<"ClearView";
  mAllDayAgenda->clear();
  mAgenda->clear();
}

CalPrinter::PrintType KOAgendaView::printType()
{
  if ( currentDateCount() == 1 ) return CalPrinter::Day;
  else return CalPrinter::Week;
}

void KOAgendaView::updateEventIndicatorTop( int newY )
{
  for(int i = 0; i < mMinY.size(); ++i ) {
    mEventIndicatorTop->enableColumn( i, newY >= mMinY[i] );
  }
  mEventIndicatorTop->update();
}

void KOAgendaView::updateEventIndicatorBottom( int newY )
{
  for(int i = 0; i < mMaxY.size(); ++i ) {
    mEventIndicatorBottom->enableColumn( i, newY <= mMaxY[i] );
  }
  mEventIndicatorBottom->update();
}

void KOAgendaView::slotTodoDropped( Todo *todo, const QPoint &gpos, bool allDay )
{
  if ( gpos.x()<0 || gpos.y()<0 ) return;
  QDate day = mSelectedDates[gpos.x()];
  QTime time = mAgenda->gyToTime( gpos.y() );
  KDateTime newTime( day, time, KOPrefs::instance()->timeSpec() );
  newTime.setDateOnly( allDay );

  if ( todo ) {
    Todo *existingTodo = calendar()->todo( todo->uid() );
    if ( existingTodo ) {
      kDebug(5850) <<"Drop existing Todo";
      Todo *oldTodo = existingTodo->clone();
      if ( mChanger && mChanger->beginChange( existingTodo ) ) {
        existingTodo->setDtDue( newTime );
        existingTodo->setAllDay( allDay );
        existingTodo->setHasDueDate( true );
        mChanger->changeIncidence( oldTodo, existingTodo );
        mChanger->endChange( existingTodo );
      } else {
        KMessageBox::sorry( this, i18n("Unable to modify this to-do, "
                            "because it cannot be locked.") );
      }
      delete oldTodo;
    } else {
      kDebug(5850) <<"Drop new Todo";
      todo->setDtDue( newTime );
      todo->setAllDay( allDay );
      todo->setHasDueDate( true );
      if ( !mChanger->addIncidence( todo, this ) ) {
        KODialogManager::errorSaveIncidence( this, todo );
      }
    }
  }
}

void KOAgendaView::startDrag( Incidence *incidence )
{
#ifndef KORG_NODND
  DndFactory factory( calendar() );
  QDrag *drag = factory.createDrag( incidence, this );
  if ( drag->start() ) {
    kDebug(5850) <<"KOAgendaView::startDrag(): Delete drag source";
  }
#endif
}

void KOAgendaView::readSettings()
{
  readSettings(KOGlobals::self()->config());
}

void KOAgendaView::readSettings(KConfig *config)
{
//  kDebug(5850) <<"KOAgendaView::readSettings()";

  KConfigGroup group = config->group("Views");

#ifndef KORG_NOSPLITTER
  QList<int> sizes = group.readEntry("Separator AgendaView",QList<int>());
  if (sizes.count() == 2) {
    mSplitterAgenda->setSizes(sizes);
  }
#endif

  updateConfig();
}

void KOAgendaView::writeSettings(KConfig *config)
{
//  kDebug(5850) <<"KOAgendaView::writeSettings()";

  KConfigGroup group = config->group("Views");

#ifndef KORG_NOSPLITTER
  QList<int> list = mSplitterAgenda->sizes();
  group.writeEntry("Separator AgendaView",list);
#endif
}

void KOAgendaView::setHolidayMasks()
{
  mHolidayMask.resize( mSelectedDates.count() + 1 );

  for( int i = 0; i < mSelectedDates.count(); ++i ) {
    mHolidayMask[i] = !KOGlobals::self()->isWorkDay( mSelectedDates[ i ] );
  }

  // Store the information about the day before the visible area (needed for
  // overnight working hours) in the last bit of the mask:
  bool showDay = !KOGlobals::self()->isWorkDay( mSelectedDates[ 0 ].addDays( -1 ) );
  mHolidayMask[ mSelectedDates.count() ] = showDay;

  mAgenda->setHolidayMask( &mHolidayMask );
  mAllDayAgenda->setHolidayMask( &mHolidayMask );
}

void KOAgendaView::setContentsPos( int y )
{
  mAgenda->setContentsPos( 0, y );
}

void KOAgendaView::setExpandedButton( bool expanded )
{
  if ( !mExpandButton ) return;

  if ( expanded ) {
    mExpandButton->setIcon( QIcon(mExpandedPixmap) );
  } else {
    mExpandButton->setIcon( QIcon(mNotExpandedPixmap) );
  }
}

void KOAgendaView::clearSelection()
{
  mAgenda->deselectItem();
  mAllDayAgenda->deselectItem();
}

void KOAgendaView::newTimeSpanSelectedAllDay( const QPoint &start, const QPoint &end )
{
  newTimeSpanSelected( start, end );
  mTimeSpanInAllDay = true;
}

void KOAgendaView::newTimeSpanSelected( const QPoint &start, const QPoint &end )
{
  if (!mSelectedDates.count()) return;

  mTimeSpanInAllDay = false;

  QDate dayStart = mSelectedDates[start.x()];
  QDate dayEnd = mSelectedDates[end.x()];

  QTime timeStart = mAgenda->gyToTime(start.y());
  QTime timeEnd = mAgenda->gyToTime( end.y() + 1 );

  QDateTime dtStart(dayStart,timeStart);
  QDateTime dtEnd(dayEnd,timeEnd);

  mTimeSpanBegin = dtStart;
  mTimeSpanEnd = dtEnd;
}

void KOAgendaView::deleteSelectedDateTime()
{
  mTimeSpanBegin.setDate(QDate());
  mTimeSpanEnd.setDate(QDate());
  mTimeSpanInAllDay = false;
}

void KOAgendaView::setTypeAheadReceiver( QObject *o )
{
  mAgenda->setTypeAheadReceiver( o );
  mAllDayAgenda->setTypeAheadReceiver( o );
}

void KOAgendaView::finishTypeAhead()
{
  mAgenda->finishTypeAhead();
  mAllDayAgenda->finishTypeAhead();
}

void KOAgendaView::removeIncidence( Incidence *incidence )
{
  mAgenda->removeIncidence( incidence );
  mAllDayAgenda->removeIncidence( incidence );
}

void KOAgendaView::updateEventIndicators()
{
  mMinY = mAgenda->minContentsY();
  mMaxY = mAgenda->maxContentsY();

  mAgenda->checkScrollBoundaries();
  updateEventIndicatorTop( mAgenda->visibleContentsYMin() );
  updateEventIndicatorBottom( mAgenda->visibleContentsYMax() );
}

void KOAgendaView::setIncidenceChanger( IncidenceChangerBase *changer )
{
  mChanger = changer;
  mAgenda->setIncidenceChanger( changer );
  mAllDayAgenda->setIncidenceChanger( changer );
}

void KOAgendaView::clearTimeSpanSelection()
{
  mAgenda->clearSelection();
  mAllDayAgenda->clearSelection();
  deleteSelectedDateTime();
}

void KOAgendaView::setResource(KCal::ResourceCalendar * res, const QString & subResource)
{
  mResource = res;
  mSubResource = subResource;
}

bool KOAgendaView::filterByResource(Incidence * incidence)
{
  if ( !mResource )
    return true;
  CalendarResources *calRes = dynamic_cast<CalendarResources*>( calendar() );
  if ( !calRes )
    return true;
  if ( calRes->resource( incidence ) != mResource )
    return false;
  if ( !mSubResource.isEmpty() ) {
    if ( mResource->subresourceIdentifier( incidence ) != mSubResource )
      return false;
  }
  return true;
}
