/**
 * syntaxhighlighter.cpp
 *
 * Copyright (c) 2003 Trolltech AS
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "syntaxhighlighter.h"
#include <klocale.h>
#include <qcolor.h>
#include <qregexp.h>
#include <qsyntaxhighlighter.h>
#include <qtimer.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kspell.h>
#include <kmkernel.h>
#include <kapplication.h>

static int dummy, dummy2, dummy3;
static int *Okay = &dummy;
static int *NotOkay = &dummy2;
static int *Ignore = &dummy3;

namespace KMail {

MessageHighlighter::MessageHighlighter( QTextEdit *textEdit, SyntaxMode mode )
    : QSyntaxHighlighter( textEdit ), sMode( mode )
{
  KConfig *config = KMKernel::config();

  // block defines the lifetime of KConfigGroupSaver
  KConfigGroupSaver saver(config, "Reader");
  QColor defaultColor1( 0x00, 0x80, 0x00 ); // defaults from kmreaderwin.cpp
  QColor defaultColor2( 0x00, 0x70, 0x00 );
  QColor defaultColor3( 0x00, 0x60, 0x00 );
  col1 = QColor(kapp->palette().active().text());
  col2 = config->readColorEntry( "QuotedText3", &defaultColor3 );
  col3 = config->readColorEntry( "QuotedText2", &defaultColor2 );
  col4 = config->readColorEntry( "QuotedText1", &defaultColor1 );
  col5 = col1;
}

MessageHighlighter::~MessageHighlighter()
{
}

int MessageHighlighter::highlightParagraph( const QString &text, int )
{
    QString simplified = text;
    simplified = simplified.replace( QRegExp( "\\s" ), "" ).replace( QRegExp( "\\|" ), ">" );
    if ( simplified.startsWith( ">>>>" ) )
	setFormat( 0, text.length(), col1 );
    else if	( simplified.startsWith( ">>>" ) || simplified.startsWith( "> >	>" ) )
	setFormat( 0, text.length(), col2 );
    else if	( simplified.startsWith( ">>" )	|| simplified.startsWith( "> >"	) )
	setFormat( 0, text.length(), col3 );
    else if	( simplified.startsWith( ">" ) )
	setFormat( 0, text.length(), col4 );
    else
	setFormat( 0, text.length(), col5 );
    return 0;
}

SpellChecker::SpellChecker( QTextEdit *textEdit )
: MessageHighlighter( textEdit ), alwaysEndsWithSpace( TRUE )
{
  KConfig *config = KMKernel::config();
  KConfigGroupSaver saver(config, "Reader");
  QColor c = QColor("red");
  mColor = config->readColorEntry("NewMessage", &c);
}

SpellChecker::~SpellChecker()
{
}

int SpellChecker::highlightParagraph( const QString& text,
				      int endStateOfLastPara )
{
    // leave #includes, diffs, and quoted replies alone
    QString diffAndCo( ">|" );

    bool isCode = diffAndCo.find(text[0]) != -1;

    if ( !text.endsWith(" ") )
	alwaysEndsWithSpace = FALSE;

    MessageHighlighter::highlightParagraph( text, endStateOfLastPara );

    if ( !isCode ) {
        int para, index;
	textEdit()->getCursorPosition( &para, &index );
	QString paraText = textEdit()->text( para );
	int len = text.length();
	if ( alwaysEndsWithSpace )
	    len--;

	currentPos = 0;
	currentWord = "";
	for ( int i = 0; i < len; i++ ) {
	    if ( text[i].isSpace() || text[i] == '-' ) {
		flushCurrentWord();
		currentPos = i + 1;
	    } else {
		currentWord += text[i];
	    }
	}
	if ( !text[len - 1].isLetter() || 
	     index + 1 != text.length() || 
	     text != paraText)
	    flushCurrentWord();
    }
    return endStateOfLastPara;
}

QStringList SpellChecker::personalWords()
{
    QStringList l;
    l.append( "KMail" );
    l.append( "KOrganizer" );
    l.append( "KHTML" );
    l.append( "KIO" );
    l.append( "KJS" );
    l.append( "Konqueror" );
    l.append( "KSpell" );
    l.append( "Kontact" );
    return l;
}

void SpellChecker::flushCurrentWord()
{
    while ( currentWord[0].isPunct() ) {
	currentWord = currentWord.mid( 1 );
	currentPos++;
    }

    QChar ch;
    while ( (ch = currentWord[(int) currentWord.length() - 1]).isPunct() &&
	     ch != '(' && ch != '@' )
	currentWord.truncate( currentWord.length() - 1 );

    if ( !currentWord.isEmpty() ) {
	if ( isMisspelled(currentWord) )
	    setFormat( currentPos, currentWord.length(), mColor );
//	    setMisspelled( currentPos, currentWord.length(), true );
    }
    currentWord = "";
}

QDict<int> DictSpellChecker::dict( 50021 );
QObject *DictSpellChecker::sDictionaryMonitor = 0;

DictSpellChecker::DictSpellChecker( QTextEdit *textEdit )
    : SpellChecker( textEdit )
{
    mAutoReady = false;
    mWordCount = 0;
    mErrorCount = 0;
    mActive = true;
    mAutomatic = true;
    textEdit->installEventFilter( this );
    mInitialMove = true;
    mRehighlightRequested = false;
    mSpell = 0;
    mSpellKey = spellKey();
    if (!sDictionaryMonitor)
	sDictionaryMonitor = new QObject();
    slotDictionaryChanged();
    startTimer(2*1000);
}

DictSpellChecker::~DictSpellChecker()
{
    delete mSpell;
}

void DictSpellChecker::slotSpellReady( KSpell *spell )
{
    connect( sDictionaryMonitor, SIGNAL( destroyed() ),
	     this, SLOT( slotDictionaryChanged() ));
    mSpell = spell;
    QStringList l = SpellChecker::personalWords();
    for ( QStringList::Iterator it = l.begin(); it != l.end(); ++it ) {
        mSpell->addPersonal( *it );
    }
    connect( spell, SIGNAL( misspelling (const QString &, const QStringList &, unsigned int) ),
	     this, SLOT( slotMisspelling (const QString &, const QStringList &, unsigned int)));
    if (!mRehighlightRequested) {
	mRehighlightRequested = true;
	QTimer::singleShot(0, this, SLOT(slotRehighlight()));
    }
}

bool DictSpellChecker::isMisspelled( const QString& word )
{
    // Normally isMisspelled would look up a dictionary and return
    // true or false, but kspell is asynchronous and slow so things
    // get tricky...

    // For auto detection ignore signature and reply prefix
    if (!mAutoReady)
	mAutoIgnoreDict.replace( word, Ignore );

    // "dict" is used as a cache to store the results of KSpell
    if (!dict.isEmpty() && dict[word] == NotOkay) {
	if (mAutoReady && (mAutoDict[word] != NotOkay)) {
	    if (!mAutoIgnoreDict[word]) {
		++mErrorCount;
		if (mAutoDict[word] != Okay)
		    ++mWordCount;
	    }
	    mAutoDict.replace( word, NotOkay );
	}
	return true && mActive;
    }
    if (!dict.isEmpty() && dict[word] == Okay) {
	if (mAutoReady && !mAutoDict[word]) {
	    mAutoDict.replace( word, Okay );
	    if (!mAutoIgnoreDict[word])
		++mWordCount;
	}
	return false;
    }

    // there is no 'spelt correctly' signal so default to Okay
    dict.replace( word, Okay );
    
    // yes I tried checkWord, the docs lie and it didn't give useful signals :-(
    if (mSpell)
	mSpell->check( word, false );
    return false;
}

void DictSpellChecker::slotMisspelling (const QString & originalword, const QStringList & suggestions, unsigned int)
{
    kdDebug(5006) << suggestions.join(" ").latin1() << endl;
    dict.replace( originalword, NotOkay );

    // this is slow but since kspell is async this will have to do for now
    if (!mRehighlightRequested) {
	mRehighlightRequested = true;
	QTimer::singleShot(0, this, SLOT(slotRehighlight()));
    }
}

void DictSpellChecker::dictionaryChanged()
{
    QObject *oldMonitor = sDictionaryMonitor;
    sDictionaryMonitor = new QObject();
    dict.clear();
    delete oldMonitor;
}

void DictSpellChecker::slotRehighlight()
{
    mRehighlightRequested = false;
    rehighlight();
    QTimer::singleShot(0, this, SLOT(slotAutoDetection()));
}

void DictSpellChecker::slotDictionaryChanged()
{
    delete mSpell;
    mSpell = 0;
    mWordCount = 0;
    mErrorCount = 0;
    mAutoDict.clear();
    new KSpell(0, i18n("Incremental Spellcheck - KMail"), this,
			 SLOT(slotSpellReady(KSpell*)));
}

QString DictSpellChecker::spellKey()
{
    //Note: Yes polling with timerEvent and reading directly from kglobals
    //Note: is evil. It would be nice if there was some kind of inter-process
    //Note: signal emitted when spelling configuration options are changed.
    KConfig *config = KGlobal::config();
    KConfigGroupSaver cs(config,"KSpell");
    config->reparseConfiguration();
    QString key;
    key += QString::number(config->readNumEntry("KSpell_NoRootAffix", 0));
    key += '/';
    key += QString::number(config->readNumEntry("KSpell_RunTogether", 0));
    key += '/';
    key += config->readEntry("KSpell_Dictionary", "");
    key += '/';
    key += QString::number(config->readNumEntry("KSpell_DictFromList", FALSE));
    key += '/';
    key += QString::number(config->readNumEntry ("KSpell_Encoding", KS_E_ASCII));
    key += '/';
    key += QString::number(config->readNumEntry ("KSpell_Client", KS_CLIENT_ISPELL));
    return key;
}


// Automatic spell checking support
// In auto spell checking mode disable as-you-type spell checking
// iff more than one third of words are spelt incorrectly.
//
// Words in the signature and reply prefix are ignored.
// Only unique words are counted.
void DictSpellChecker::slotAutoDetection()
{
    if (!mAutoReady)
	return;
    bool savedActive = mActive;
    if (mAutomatic) {
	if (mActive && (mErrorCount * 3 >= mWordCount))
	    mActive = false;
	else if (!mActive && (mErrorCount * 3 < mWordCount))
	    mActive = true;
    }
    if (mActive != savedActive && !mRehighlightRequested) {
	emit activeChanged( mActive );
	mRehighlightRequested = true;
	QTimer::singleShot(100, this, SLOT(slotRehighlight()));
    }
}

bool DictSpellChecker::eventFilter(QObject* o, QEvent* e)
{
    //TODO mouse moves, pgup, home ctrl etc.
    if (o == textEdit() && (e->type() == QEvent::FocusIn)) {
	if (mSpell && mSpellKey != spellKey()) {
	    mSpellKey = spellKey();
	    DictSpellChecker::dictionaryChanged();
	}
    }
    
    if (o == textEdit() && (e->type() == QEvent::KeyPress)) {
	QKeyEvent *k = (QKeyEvent*)e;
	mAutoReady = true;
	if (k->key() == Key_Enter ||
	    k->key() == Key_Return || 
	    k->key() == Key_Up || 
	    k->key() == Key_Down || 
	    k->key() == Key_Left || 
	    k->key() == Key_Right) {
	    if (mInitialMove) {
		if (!mRehighlightRequested) {
		    mRehighlightRequested = true;
		    QTimer::singleShot(0, this, SLOT(slotRehighlight()));
		}
		mInitialMove = false;
	    }
	} else {
	    mInitialMove = true;
	}
	if (k->key() == Key_Space ||
	    k->key() == Key_Enter ||
	    k->key() == Key_Return)
	    QTimer::singleShot(0, this, SLOT(slotAutoDetection()));
    }
    return false;
}

} //namespace KMail

#include "syntaxhighlighter.moc"
