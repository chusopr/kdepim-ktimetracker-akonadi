/*  -*- c++ -*-
    csshelper.cpp

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2003 Marc Mutz <mutz@kde.org>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KMail is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config.h>

#include "csshelper.h"

#include "kmkernel.h"

#include <kconfig.h>
#include <kglobalsettings.h>
#include <kdebug.h>

#include <qcolor.h>
#include <qfont.h>
#include <qstring.h>
#include <qapplication.h>

#include <cassert>

namespace KMail {

  class CSSHelper::Private {
    friend class CSSHelper;
  public:
    Private() {}
    ~Private() {}

    bool operator==( const Private & other ) const;
    bool operator!=( const Private & other ) const {
      return !operator==( other );
    }

    void readColorConfig();

    // returns CSS rules specific to the print media type
    QString printCssDefinitions() const;

    // returns CSS rules specific to the screen media type
    QString screenCssDefinitions( const CSSHelper * helper, bool fixed ) const;

    // returns CSS rules common to both screen and print media types
    QString commonCssDefinitions() const;

    QFont bodyFont( bool fixed, bool print=false ) const {
      return fixed ? mFixedFont : print ? mPrintFont : mBodyFont ;
    }
    int fontSize( bool fixed, bool print=false ) const {
      return bodyFont( fixed, print ).pointSize();
    }

  private:
    QFont mBodyFont, mPrintFont, mFixedFont;
    QFont mQuoteFont[3];
    QColor mQuoteColor[3];
    bool mBackingPixmapOn;
    QString mBackingPixmapStr;
    QColor c1, c2, c3, c4;
    // colors for PGP (Frame, Header, Body)
    QColor cPgpOk1F, cPgpOk1H, cPgpOk1B,
      cPgpOk0F, cPgpOk0H, cPgpOk0B,
      cPgpWarnF, cPgpWarnH, cPgpWarnB,
      cPgpErrF, cPgpErrH, cPgpErrB,
      cPgpEncrF, cPgpEncrH, cPgpEncrB;
    // color of frame of warning preceding the source of HTML messages
    QColor cHtmlWarning;
  };

  bool CSSHelper::Private::operator==( const Private & other ) const {
    for ( int i = 0 ; i < 3 ; ++i )
      if ( mQuoteFont[i] != other.mQuoteFont[i] ||
	   mQuoteColor[i] != other.mQuoteColor[i] )
	return false;
    return // eeek!
      mBodyFont == other.mBodyFont &&
      mPrintFont == other.mPrintFont &&
      mFixedFont == other.mFixedFont &&
      mBackingPixmapOn == other.mBackingPixmapOn &&
      mBackingPixmapStr == other.mBackingPixmapStr &&
      c1 == other.c1 && c2 == other.c2 && c3 == other.c3 && c4 == other.c4 &&
      cHtmlWarning == other.cHtmlWarning &&
      cPgpOk1F == other.cPgpOk1F && cPgpOk1H == other.cPgpOk1H && cPgpOk1B == other.cPgpOk1B &&
      cPgpOk0F == other.cPgpOk0F && cPgpOk0H == other.cPgpOk0H && cPgpOk0B == other.cPgpOk0B &&
      cPgpWarnF == other.cPgpWarnF && cPgpWarnH == other.cPgpWarnH && cPgpWarnB == other.cPgpWarnB &&
      cPgpErrF == other.cPgpErrF && cPgpErrH == other.cPgpErrH && cPgpErrB == other.cPgpErrB &&
      cPgpEncrF == other.cPgpEncrF && cPgpEncrH == other.cPgpEncrH && cPgpEncrB == other.cPgpEncrB ;
    }

  namespace {
    // some QColor manipulators that hide the ugly QColor API w.r.t. HSV:
    inline QColor darker( const QColor & c ) {
      int h, s, v;
      c.hsv( &h, &s, &v );
      return QColor( h, s, v*4/5, QColor::Hsv );
    }

    inline QColor desaturate( const QColor & c ) {
      int h, s, v;
      c.hsv( &h, &s, &v );
      return QColor( h, s/8, v, QColor::Hsv );
    }

    inline QColor fixValue( const QColor & c, int newV ) {
      int h, s, v;
      c.hsv( &h, &s, &v );
      return QColor( h, s, newV );
    }

    inline int getValueOf( const QColor & c ) {
      int h, s, v;
      c.hsv( &h, &s, &v );
      return v;
    }
  }

  void CSSHelper::Private::readColorConfig() {
    KConfig * config = KMKernel::config();

    KConfigGroup reader( config, "Reader" );
    KConfigGroup fonts( config, "Fonts" );
    KConfigGroup pixmaps( config, "Pixmaps" );

    c1 = QApplication::palette().active().text();
    c2 = KGlobalSettings::linkColor();
    c3 = KGlobalSettings::visitedLinkColor();
    c4 = QApplication::palette().active().base();
    cHtmlWarning = QColor( 0xFF, 0x40, 0x40 ); // warning frame color: light red

    // The default colors are also defined in configuredialog.cpp
    cPgpEncrH = QColor( 0x00, 0x80, 0xFF ); // light blue
    cPgpOk1H  = QColor( 0x40, 0xFF, 0x40 ); // light green
    cPgpOk0H  = QColor( 0xFF, 0xFF, 0x40 ); // light yellow
    cPgpWarnH = QColor( 0xFF, 0xFF, 0x40 ); // light yellow
    cPgpErrH  = Qt::red;

    for ( int i = 0 ; i < 3 ; ++i )
      mQuoteColor[i] = QColor( 0x00, 0x80 - i * 0x10, 0x00 ); // shades of green

    if ( !reader.readBoolEntry( "defaultColors", true ) ) {
      c1 = reader.readColorEntry("ForegroundColor",&c1);
      c2 = reader.readColorEntry("LinkColor",&c2);
      c3 = reader.readColorEntry("FollowedColor",&c3);
      c4 = reader.readColorEntry("BackgroundColor",&c4);
      cPgpEncrH = reader.readColorEntry( "PGPMessageEncr", &cPgpEncrH );
      cPgpOk1H  = reader.readColorEntry( "PGPMessageOkKeyOk", &cPgpOk1H );
      cPgpOk0H  = reader.readColorEntry( "PGPMessageOkKeyBad", &cPgpOk0H );
      cPgpWarnH = reader.readColorEntry( "PGPMessageWarn", &cPgpWarnH );
      cPgpErrH  = reader.readColorEntry( "PGPMessageErr", &cPgpErrH );
      cHtmlWarning = reader.readColorEntry( "HTMLWarningColor", &cHtmlWarning );
      for ( int i = 0 ; i < 3 ; ++i ) {
	const QString key = "QuotedText" + QString::number( i+1 );
	mQuoteColor[i] = reader.readColorEntry( key, &mQuoteColor[i] );
      }
    }

    // determine the frame and body color for PGP messages from the header color
    // if the header color equals the background color then the other colors are
    // also set to the background color (-> old style PGP message viewing)
    // else
    // the brightness of the frame is set to 4/5 of the brightness of the header
    // and in case of a light background color
    // the saturation of the body is set to 1/8 of the saturation of the header
    // while in case of a dark background color
    // the value of the body is set to the value of the background color

    // Check whether the user uses a light color scheme
    const int vBG = getValueOf( c4 );
    const bool lightBG = vBG >= 128;
    if ( cPgpOk1H == c4 ) {
      cPgpOk1F = c4;
      cPgpOk1B = c4;
    } else {
      cPgpOk1F= darker( cPgpOk1H );
      cPgpOk1B = lightBG ? desaturate( cPgpOk1H ) : fixValue( cPgpOk1H, vBG );
    }
    if ( cPgpOk0H == c4 ) {
      cPgpOk0F = c4;
      cPgpOk0B = c4;
    } else {
      cPgpOk0F = darker( cPgpOk0H );
      cPgpOk0B = lightBG ? desaturate( cPgpOk0H ) : fixValue( cPgpOk0H, vBG );
    }
    if ( cPgpWarnH == c4 ) {
      cPgpWarnF = c4;
      cPgpWarnB = c4;
    } else {
      cPgpWarnF = darker( cPgpWarnH );
      cPgpWarnB = lightBG ? desaturate( cPgpWarnH ) : fixValue( cPgpWarnH, vBG );
    }
    if ( cPgpErrH == c4 ) {
      cPgpErrF = c4;
      cPgpErrB = c4;
    } else {
      cPgpErrF = darker( cPgpErrH );
      cPgpErrB = lightBG ? desaturate( cPgpErrH ) : fixValue( cPgpErrH, vBG );
    }
    if ( cPgpEncrH == c4 ) {
      cPgpEncrF = c4;
      cPgpEncrB = c4;
    } else {
      cPgpEncrF = darker( cPgpEncrH );
      cPgpEncrB = lightBG ? desaturate( cPgpEncrH ) : fixValue( cPgpEncrH, vBG );
    }

    QFont defaultFont = KGlobalSettings::generalFont();
    if ( fonts.readBoolEntry( "defaultFonts", true ) ) {
      mBodyFont = mPrintFont = defaultFont;
      mFixedFont = KGlobalSettings::fixedFont();
      defaultFont.setItalic( true );
      for ( int i = 0 ; i < 3 ; ++i )
	mQuoteFont[i] = defaultFont;
    } else {
      mBodyFont = fonts.readFontEntry(  "body-font",  &defaultFont);
      mPrintFont = fonts.readFontEntry( "print-font", &defaultFont);
      mFixedFont = fonts.readFontEntry( "fixed-font", &defaultFont);
      defaultFont.setItalic( true );
      for ( int i = 0 ; i < 3 ; ++i ) {
	const QString key = QString( "quote%1-font" ).arg( i+1 );
	mQuoteFont[i] = fonts.readFontEntry( key, &defaultFont );
      }
    }

    mBackingPixmapStr = pixmaps.readPathEntry("Readerwin");
    mBackingPixmapOn = !mBackingPixmapStr.isEmpty();
  }

  CSSHelper::CSSHelper( const QPaintDeviceMetrics & pdm, QObject * parent, const char * name )
    : ConfigManager( parent, name ),
      d( 0 ), s( 0 ), mMetrics( pdm )
  {
    d = new Private();
    d->readColorConfig();
  }

  CSSHelper::~CSSHelper() {
    kdWarning( hasPendingChanges(), 5006 )
      << "CSSHelper: There were uncommitted changes!" << endl;
    delete d; d = 0;
    delete s; s = 0;
  }

  void CSSHelper::commit() {
    // not yet implemented
  }

  void CSSHelper::rollback() {
    delete s; s = 0;
  }

  bool CSSHelper::hasPendingChanges() const {
    assert( d );
    return s && *s != *d ;
  }

  QString CSSHelper::cssDefinitions( bool fixed ) const {
    assert( d );
    return
      d->commonCssDefinitions()
      +
      "@media screen {\n\n"
      +
      d->screenCssDefinitions( this, fixed )
      +
      "}\n"
      "@media print {\n\n"
      +
      d->printCssDefinitions()
      +
      "}\n";
  }

  QString CSSHelper::htmlHead( bool fixed ) const {
    return
      "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n"
      "<html><head><title></title>\n"
      "<style type=\"text/css\">\n\n"
      +
      cssDefinitions( fixed )
      +
      "</style></head>\n"
      +
      ( fixed ? "<body class\"fixedfont\">\n" : "<body>\n" );
  }

  QString CSSHelper::quoteFontTag( int level ) const {
    return QString( "<div class=\"quotelevel%1\">" ).arg( level % 3 + 1 );
  }

  QString CSSHelper::nonQuotedFontTag() const {
    return "<div class=\"noquote\">";
  }

  QFont CSSHelper::bodyFont( bool fixed, bool print ) const {
    assert( d );
    return d->bodyFont( fixed, print );
  }

  namespace {
    int pointsToPixel( const QPaintDeviceMetrics & metrics, int pointSize ) {
      return ( pointSize * metrics.logicalDpiY() + 36 ) / 72 ;
    }
  }

  QString CSSHelper::Private::printCssDefinitions() const {
    const QString headerFont = QString( "  font-family: \"%1\";\n"
					"  font-size: %2pt;\n" )
                           .arg( mPrintFont.family() )
                           .arg( mPrintFont.pointSize() );
    const QColorGroup & cg = QApplication::palette().active();

    QString quoteCSS;
    if ( mPrintFont.italic() )
      quoteCSS += "  font-style: italic;\n";
    if ( mPrintFont.bold() )
      quoteCSS += "  font-weight: bold;\n";
    if ( !quoteCSS.isEmpty() )
      quoteCSS = "div.noquote {\n" + quoteCSS + "}\n\n";

    return
      QString( "body {\n"
	       "  font-family: \"%1\";\n"
	       "  font-size: %2pt;\n"
	       "  color: #000000;\n"
	       "  background-color: #ffffff\n"
	       "}\n\n" )
#if QT_VERSION >= 0x030200
      .arg( mPrintFont.family(),
	    QString::number( mPrintFont.pointSize() ) )
#else
      .arg( mPrintFont.family() )
      .arg( mPrintFont.pointSize() )
#endif
      +
      QString( "tr.textAtmH,\n"
	       "tr.rfc822H,\n"
	       "tr.encrH,\n"
	       "tr.signOkKeyOkH,\n"
	       "tr.signOkKeyBadH,\n"
	       "tr.signWarnH,\n"
	       "tr.signErrH,\n"
	       "div.header {\n"
	       "%1"
	       "}\n\n"

	       "div.fancy.header > div {\n"
	       "  background-color: %2;\n"
	       "  color: %3;\n"
	       "  padding: 4px;\n"
	       "  border: solid %3 1px;\n"
	       "}\n\n"

	       "div.fancy.header > div a[href] { color: %3; }\n\n"

	       "div.fancy.header table {\n"
	       "  background-color: %2;\n"
	       "  color: %3;\n"
	       "  border-bottom: solid %3 1px;\n"
	       "  border-left: solid %3 1px;\n"
	       "  border-right: solid %3 1px;\n"
	       "}\n\n"

	       "div.htmlWarn {\n"
	       "  border: 2px solid #ffffff;\n"
	       "}\n\n" )
#if QT_VERSION >= 0x030200
      .arg( headerFont,
	    cg.background().name(),
	    cg.foreground().name() )
#else
      .arg( headerFont )
      .arg(cg.background().name())
      .arg(cg.background().name())
      .arg(cg.foreground().name())
      .arg(cg.foreground().name())
      .arg(cg.foreground().name())
      .arg(cg.foreground().name())
      .arg(cg.foreground().name())
      .arg(cg.foreground().name())
      .arg(cg.foreground().name())
#endif
      + quoteCSS;
  }

  QString CSSHelper::Private::screenCssDefinitions( const CSSHelper * helper, bool fixed ) const {
    const QString fgColor = c1.name();
    const QString bgColor = c4.name();
    const QString linkColor = c2.name();
    const QString headerFont = QString("  font-family: \"%1\";\n"
				       "  font-size: %2px\n;")
      .arg( mBodyFont.family() )
      .arg( pointsToPixel( helper->mMetrics, mBodyFont.pointSize() ) );
    const QString background = ( mBackingPixmapOn
                         ? QString( "  background-image:url(file://%1);\n" )
                           .arg( mBackingPixmapStr )
                         : QString( "  background-color: %1;\n" )
                           .arg( bgColor ) );
    const QString bodyFontSize = QString::number( pointsToPixel( helper->mMetrics, fontSize( fixed ) ) ) + "px" ;
    const QColorGroup & cg = QApplication::palette().active();

    QString quoteCSS;
    if ( bodyFont( fixed ).italic() )
      quoteCSS += "  font-style: italic;\n";
    if ( bodyFont( fixed ).bold() )
      quoteCSS += "  font-weight: bold;\n";
    if ( !quoteCSS.isEmpty() )
      quoteCSS = "div.noquote {\n" + quoteCSS + "}\n\n";

    for ( int i = 0 ; i < 3 ; ++i ) {
      quoteCSS += QString( "div.quotelevel%1 {\n"
			   "  color: %2;\n" )
#if QT_VERSION >= 0x030200
	.arg( QString::number(i+1), mQuoteColor[i].name() );
#else
        .arg(i+1).arg( mQuoteColor[i].name() );
#endif
      if ( mQuoteFont[i].italic() )
	quoteCSS += "  font-style: italic;\n";
      if ( mQuoteFont[i].bold() )
	quoteCSS += "  font-weight: bold;\n";
      quoteCSS += "}\n\n";
    }

    return
      QString( "body {\n"
	       "  font-family: \"%1\";\n"
	       "  font-size: %2;\n"
	       "  color: %3;\n"
	       "%4"
	       "}\n\n" )
#if QT_VERSION >= 0x030200
      .arg( bodyFont( fixed ).family(),
	    bodyFontSize,
	    fgColor,
	    background )
#else
      .arg( bodyFont( fixed ).family() )
      .arg( bodyFontSize )
      .arg( fgColor )
      .arg( background )
#endif
      +
      QString( "a {\n"
	       "  color: %1;\n"
	       "  text-decoration: none;\n"
	       "}\n\n"

	       "table.textAtm { background-color: %2; }\n\n"

	       "tr.textAtmH {\n"
	       "  background-color: %3;\n"
	       "%4"
	       "}\n\n"

	       "tr.textAtmB {\n"
	       "  background-color: %3;\n"
	       "}\n\n"

	       "table.rfc822 {\n"
	       "  background-color: %3;\n"
	       "}\n\n"

	       "tr.rfc822H {\n"
	       "%4"
	       "}\n\n" )
#if QT_VERSION >= 0x030200
      .arg( linkColor, fgColor, bgColor, headerFont )
#else
      .arg( linkColor )
      .arg( fgColor )
      .arg( bgColor )
      .arg( bgColor )
      .arg( bgColor )
      .arg( headerFont )
      .arg( headerFont )
#endif
      +
      QString( "table.encr {\n"
	       "  background-color: %1;\n"
	       "}\n\n"

	       "tr.encrH {\n"
	       "  background-color: %2;\n"
	       "%3"
	       "}\n\n"

	       "tr.encrB { background-color: %4; }\n\n" )
#if QT_VERSION >= 0x030200
      .arg( cPgpEncrF.name(),
	    cPgpEncrH.name(),
	    headerFont,
	    cPgpEncrB.name() )
#else
      .arg( cPgpEncrF.name() )
      .arg( cPgpEncrH.name() )
      .arg( headerFont )
      .arg( cPgpEncrB.name() )
#endif
      +
      QString( "table.signOkKeyOk {\n"
	       "  background-color: %1;\n"
	       "}\n\n"

	       "tr.signOkKeyOkH {\n"
	       "  background-color: %2;\n"
	       "%3"
	       "}\n\n"

	       "tr.signOkKeyOkB { background-color: %4; }\n\n" )
#if QT_VERSION >= 0x030200
      .arg( cPgpOk1F.name(),
	    cPgpOk1H.name(),
	    headerFont,
	    cPgpOk1B.name() )
#else
      .arg( cPgpOk1F.name() )
      .arg( cPgpOk1H.name() )
      .arg( headerFont )
      .arg( cPgpOk1B.name() )
#endif
      +
      QString( "table.signOkKeyBad {\n"
	       "  background-color: %1;\n"
	       "}\n\n"

	       "tr.signOkKeyBadH {\n"
	       "  background-color: %2;\n"
	       "%3"
	       "}\n\n"

	       "tr.signOkKeyBadB { background-color: %4; }\n\n" )
#if QT_VERSION >= 0x030200
      .arg( cPgpOk0F.name(),
	    cPgpOk0H.name(),
	    headerFont,
	    cPgpOk0B.name() )
#else
      .arg( cPgpOk0F.name() )
      .arg( cPgpOk0H.name() )
      .arg( headerFont )
      .arg( cPgpOk0B.name() )
#endif
      +
      QString( "table.signWarn {\n"
	       "  background-color: %1;\n"
	       "}\n\n"

	       "tr.signWarnH {\n"
	       "  background-color: %2;\n"
	       "%3"
	       "}\n\n"

	       "tr.signWarnB { background-color: %4; }\n\n" )
#if QT_VERSION >= 0x030200
      .arg( cPgpWarnF.name(),
	    cPgpWarnH.name(),
	    headerFont,
	    cPgpWarnB.name() )
#else
      .arg( cPgpWarnF.name() )
      .arg( cPgpWarnH.name() )
      .arg( headerFont )
      .arg( cPgpWarnB.name() )
#endif
      +
      QString( "table.signErr {\n"
	       "  background-color: %1;\n"
	       "}\n\n"

	       "tr.signErrH {\n"
	       "  background-color: %2;\n"
	       "%3"
	       "}\n\n"

	       "tr.signErrB { background-color: %4; }\n\n" )
#if QT_VERSION >= 0x030200
      .arg( cPgpErrF.name(),
	    cPgpErrH.name(),
	    headerFont,
	    cPgpErrB.name() )
#else
      .arg( cPgpErrF.name() )
      .arg( cPgpErrH.name() )
      .arg( headerFont )
      .arg( cPgpErrB.name() )
#endif
      +
      QString( "div.htmlWarn {\n"
	       "  border: 2px solid %1;\n"
	       "}\n\n" )
      .arg( cHtmlWarning.name() )
      +
      QString( "div.header {\n"
	       "%1"
	       "}\n\n"

	       "div.fancy.header > div {\n"
	       "  background-color: %2;\n"
	       "  color: %3;\n"
	       "  border: solid %4 1px;\n"
	       "}\n\n"

	       "div.fancy.header > div a[href] { color: %3; }\n\n"

	       "div.fancy.header > div a[href]:hover { text-decoration: underline; }\n\n"

	       "div.fancy.header table {\n"
	       "  background-color: %5;\n"
	       "  color: %4;\n"
	       "  border-bottom: solid %4 1px;\n"
	       "  border-left: solid %4 1px;\n"
	       "  border-right: solid %4 1px;\n"
	       "}\n\n" )
      .arg( headerFont )
#if QT_VERSION >= 0x030200
      .arg( cg.highlight().name(),
	    cg.highlightedText().name(),
	    cg.foreground().name(),
	    cg.background().name() )
#else
      .arg(cg.highlight().name())
      .arg(cg.highlightedText().name())
      .arg(cg.highlightedText().name())
      .arg(cg.foreground().name())
      .arg(cg.foreground().name())
      .arg(cg.foreground().name())
      .arg(cg.foreground().name())
      .arg(cg.foreground().name())
      .arg(cg.background().name())
#endif
      + quoteCSS;
  }

  QString CSSHelper::Private::commonCssDefinitions() const {
    return
      "div.header {\n"
      "  margin-bottom: 10pt;\n"
      "}\n\n"

      "table.textAtm {\n"
      "  margin-top: 10pt;\n"
      "  margin-bottom: 10pt;\n"
      "}\n\n"

      "tr.textAtmH,\n"
      "tr.textAtmB,\n"
      "tr.rfc822B {\n"
      "  font-weight: normal;\n"
      "}\n\n"

      "tr.rfc822H,\n"
      "tr.encrH,\n"
      "tr.signOkKeyOkH,\n"
      "tr.signOkKeyBadH,\n"
      "tr.signWarnH,\n"
      "tr.signErrH {\n"
      "  font-weight: bold;\n"
      "}\n\n"

      "tr.textAtmH td,\n"
      "tr.textAtmB td {\n"
      "  padding: 3px;\n"
      "}\n\n"

      "table.rfc822 {\n"
      "  width: 100%;\n"
      "  border: solid 1px black;\n"
      "  margin-top: 10pt;\n"
      "  margin-bottom: 10pt;\n"
      "}\n\n"

      "table.textAtm,\n"
      "table.encr,\n"
      "table.signWarn,\n"
      "table.signErr,\n"
      "table.signOkKeyBad,\n"
      "table.signOkKeyOk,\n"
      "div.fancy.header table {\n"
      "  width: 100%;\n"
      "  border-width: 0px;\n"
      "}\n\n"

      "div.htmlWarn {\n"
      "  margin: 0px 5%;\n"
      "  padding: 10px;\n"
      "  text-align: left;\n"
      "}\n\n"

      "div.fancy.header > div {\n"
      "  font-weight: bold;\n"
      "  padding: 4px;\n"
      "}\n\n"

      "div.fancy.header table {\n"
      "  padding: 2px;\n" // ### khtml bug: this is ignored
      "  align: left\n"
      "}\n\n"

      "div.fancy.header table th {\n"
      "  padding: 0px;\n"
      "  white-space: nowrap;\n"
      "  border-spacing: 0px;\n"
      "  text-align: left;\n"
      "  vertical-align: top;\n"
      "}\n\n"

      "div.fancy.header table td {\n"
      "  padding: 0px;\n"
      "  border-spacing: 0px;\n"
      "  text-align: left;\n"
      "  text-valign: top;\n"
      "  width: 100%;\n"
      "}\n\n"
      ;
  }

} // namespace KMail

