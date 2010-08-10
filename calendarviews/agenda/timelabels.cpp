/*
  This file is part of KOrganizer.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2007 Bruno Virlet <bruno@virlet.org>

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

#include "timelabels.h"
#include "timelabelszone.h"
#include "timescaleconfigdialog.h"
#include "prefs.h"
#include "agenda.h"
#include "eventview.h"

#include <KIcon>

#include <QScrollArea>
#include <QFrame>
#include <QAction>
#include <QMenu>
#include <QPainter>

using namespace EventViews;

TimeLabels::TimeLabels( const KDateTime::Spec &spec, int rows, EventView *eventView,
                        TimeLabelsZone *parent, Qt::WFlags f )
  : QFrame( parent, f ), mEventView( eventView )
{
  mTimeLabelsZone = parent;
  mSpec = spec;
  mRows = rows;
  mMiniWidth = 0;

  mCellHeight = mEventView->preferences()->hourSize() * 4;

  setFrameStyle( Plain );

  setBackgroundRole( QPalette::Background );

  mMousePos = new QFrame( this );
  mMousePos->setLineWidth( 1 );
  mMousePos->setFrameStyle( QFrame::HLine | QFrame::Plain );
  mMousePos->setFixedSize( width(), 1 );
  colorMousePos();
  mAgenda = 0;

  if ( mSpec.isValid() ) {
    setToolTip( i18n( "Timezone:" ) + mSpec.timeZone().name() );
  }

  updateConfig();
 }

void TimeLabels::mousePosChanged( const QPoint &pos )
{
  colorMousePos();
  mMousePos->move( 0, pos.y() );

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

void TimeLabels::colorMousePos()
{
  QPalette pal;
  pal.setColor( QPalette::Window, // for Oxygen
                mEventView->preferences()->agendaMarcusBainsLineLineColor() );
  pal.setColor( QPalette::WindowText, // for Plastique
                mEventView->preferences()->agendaMarcusBainsLineLineColor() );
  mMousePos->setPalette( pal );
}

void TimeLabels::setCellHeight( double height )
{
  mCellHeight = height;
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
  setFont( mEventView->preferences()->agendaTimeLabelsFont() );

  QString test = "20";
  if ( KGlobal::locale()->use12Clock() ) {
    test = "12";
  }
  mMiniWidth = fontMetrics().width( test );
  if ( KGlobal::locale()->use12Clock() ) {
    test = "pm";
  } else {
    test = "00";
  }
  QFont sFont = font();
  sFont.setPointSize( sFont.pointSize() / 2 );
  QFontMetrics fmS( sFont );
  mMiniWidth += fmS.width( test ) + frameWidth() * 2 + 4 ;
  // update geometry restrictions based on new settings
  setFixedWidth( mMiniWidth );

  /** Can happen if all resources are disabled */
  if ( !mAgenda ) {
     return;
  }

  // update HourSize
  mCellHeight = mEventView->preferences()->hourSize() * 4;
  // If the agenda is zoomed out so that more than 24 would be shown,
  // the agenda only shows 24 hours, so we need to take the cell height
  // from the agenda, which is larger than the configured one!
  if ( mCellHeight < 4 * mAgenda->gridSpacingY() ) {
       mCellHeight = 4 * mAgenda->gridSpacingY();
  }
  repaint();
}

/**  */
void TimeLabels::setAgenda( Agenda *agenda )
{
  mAgenda = agenda;

  connect( mAgenda, SIGNAL(mousePosSignal(const QPoint &)),
           this, SLOT(mousePosChanged(const QPoint &)) );
  connect( mAgenda, SIGNAL(enterAgenda()), this, SLOT(showMousePos()) );
  connect( mAgenda, SIGNAL(leaveAgenda()), this, SLOT(hideMousePos()) );
  connect( mAgenda, SIGNAL(gridSpacingYChanged(double)),
           this, SLOT(setCellHeight(double)) );
}

/** This is called in response to repaint() */
void TimeLabels::paintEvent( QPaintEvent * )
{
  QPainter p( this );

  int beginning;

  int cw = width();
  int ch = height();
  int cx = 0;
  int cy = 0;

  if ( !mSpec.isValid() ) {
    beginning = 0;
  } else {
    beginning = ( mSpec.timeZone().currentOffset() -
                  mEventView->preferences()->timeSpec().timeZone().currentOffset() ) / ( 60 * 60 );
  }

  // bug:  the parameters cx and cw are the areas that need to be
  //       redrawn, not the area of the widget.  unfortunately, this
  //       code assumes the latter...

  // now, for a workaround...
  cx = frameWidth() * 2;
  cw = width();
  // end of workaround
  int cell = ( (int)( cy / mCellHeight ) ) + beginning;  // the hour we start drawing with
  double y = ( cell - beginning ) * mCellHeight;
  QFontMetrics fm = fontMetrics();
  QString hour;
  int timeHeight = fm.ascent();
  QFont hourFont = mEventView->preferences()->agendaTimeLabelsFont();
  p.setFont( font() );

  //TODO: rewrite this using KLocale's time formats. "am/pm" doesn't make sense
  // in some locale's
  QString suffix;
  if ( !KGlobal::locale()->use12Clock() ) {
    suffix = "00";
  } else {
    suffix = "am";
    if ( cell > 11 ) {
      suffix = "pm";
    }
  }

  // We adjust the size of the hour font to keep it reasonable
  if ( timeHeight >  mCellHeight ) {
    timeHeight = int( mCellHeight - 1 );
    int pointS = hourFont.pointSize();
    while ( pointS > 4 ) { // TODO: use smallestReadableFont() when added to kdelibs
      hourFont.setPointSize( pointS );
      fm = QFontMetrics( hourFont );
      if ( fm.ascent() < mCellHeight ) {
        break;
      }
      --pointS;
    }
    fm = QFontMetrics( hourFont );
    timeHeight = fm.ascent();
  }
  //timeHeight -= (timeHeight/4-2);
  QFont suffixFont = hourFont;
  suffixFont.setPointSize( suffixFont.pointSize() / 2 );
  QFontMetrics fmS( suffixFont );
  int startW = mMiniWidth - frameWidth() - 2 ;
  int tw2 = fmS.width( suffix );
  int divTimeHeight = ( timeHeight - 1 ) / 2 - 1;
  //testline
  //p->drawLine(0,0,0,contentsHeight());
  while ( y < cy + ch + mCellHeight ) {
    QPen pen;
    if ( cell < 0 || cell >= 24 ) {
      pen.setColor( QColor( 150, 150, 150 ) );
    } else {
      pen.setColor( palette().color( QPalette::WindowText ) );
    }
    p.setPen( pen );

    // hour, full line
    p.drawLine( cx, int( y ), cw + 2, int( y ) );

    hour.setNum( cell % 24 );
    // handle different timezones
    if ( cell < 0 ) {
      hour.setNum( cell + 24 );
    }
    // handle 24h and am/pm time formats
    if ( KGlobal::locale()->use12Clock() ) {
      if ( cell == 12 ) {
        suffix = "pm";
      }
      if ( cell == 0 ) {
        hour.setNum( 12 );
      }
      if ( cell > 12 ) {
        hour.setNum( cell - 12 );
      }
    }

    // center and draw the time label
    int timeWidth = fm.width( hour );
    int offset = startW - timeWidth - tw2 -1 ;
    p.setFont( hourFont );
    p.drawText( offset, int( y + timeHeight ), hour );
    p.setFont( suffixFont );
    offset = startW - tw2;
    p.drawText( offset, int( y + timeHeight - divTimeHeight ), suffix );

    // increment indices
    y += mCellHeight;
    cell++;
  }
}

QSize TimeLabels::sizeHint() const
{
  return QSize( mMiniWidth, mRows * mCellHeight );
}

void TimeLabels::contextMenuEvent( QContextMenuEvent *event )
{
  Q_UNUSED( event );

  QMenu popup( this );
  QAction *editTimeZones =
    popup.addAction( KIcon( "document-properties" ), i18n( "&Add Timezones..." ) );
  QAction *removeTimeZone =
    popup.addAction( KIcon( "edit-delete" ),
                     i18n( "&Remove Timezone %1", mSpec.timeZone().name() ) );
  if ( !mSpec.isValid() ||
       !mEventView->preferences()->timeScaleTimezones().count() ||
       mSpec == mEventView->preferences()->timeSpec() ) {
    removeTimeZone->setEnabled( false );
  }

  QAction *activatedAction = popup.exec( QCursor::pos() );
  if ( activatedAction == editTimeZones ) {
    QPointer<TimeScaleConfigDialog> dialog = new TimeScaleConfigDialog( mEventView->preferences(), this );
    if ( dialog->exec() == QDialog::Accepted ) {
      mTimeLabelsZone->reset();
    }
    delete dialog;
  } else if ( activatedAction == removeTimeZone ) {
    QStringList list = mEventView->preferences()->timeScaleTimezones();
    list.removeAll( mSpec.timeZone().name() );
    mEventView->preferences()->setTimeScaleTimezones( list );
    mTimeLabelsZone->reset();
    hide();
    deleteLater();
  }
}

KDateTime::Spec TimeLabels::timeSpec()
{
  return mSpec;
}

QString TimeLabels::header() const
{
  return mSpec.timeZone().name();
}

QString TimeLabels::headerToolTip() const
{
  KTimeZone tz = mSpec.timeZone();

  QString toolTip;
  toolTip += "<qt>";
  toolTip += i18n( "Timezone: %1", tz.name() );
  if ( !tz.countryCode().isEmpty() ) {
    toolTip += "<br/>";
    toolTip += i18n( "Country Code: %1", tz.countryCode() );
  }
  if ( !tz.abbreviations().isEmpty() ) {
    toolTip += "<br/>";
    toolTip += i18n( "Abbreviations:" );
    foreach ( const QByteArray &a, tz.abbreviations() ) {
      toolTip += "<br/>";
      toolTip += "&nbsp;" + QString::fromLocal8Bit( a );
    }
  }
  if ( !tz.comment().isEmpty() ) {
    toolTip += "<br/>";
    toolTip += i18n( "Comment:<br/>%1", tz.comment() );
  }
  toolTip += "</qt>";

  return toolTip;
}

#include "timelabels.moc"
