// kmfilterrulesedit.cpp
// Author: Marc Mutz <Marc@Mutz.com>
// This code is under GPL

#include "kmsearchpatternedit.h"
//#include "kmfilter.h"
//#include "kmfilterdlg.h"

#include <klocale.h>
#include <kdebug.h>
#include <kbuttonbox.h>

#include <qstring.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qbuttongroup.h>

#include <assert.h>

static QStringList sFilterFieldList, sFilterFuncList;

//=============================================================================
//
// class KMSearchRuleWidget
//
//=============================================================================

KMSearchRuleWidget::KMSearchRuleWidget(QWidget *parent, KMSearchRule *aRule, const char *name)
  : QHBox(parent,name)
{
  kdDebug() << "KMSearchRuleWidget::KMSearchRuleWidget" << endl;
  initLists(); // sFilter{Func,Field}List are local to KMSearchRuleWidget
  initWidget();
  
  if ( aRule )
    setRule(aRule);
  else
    reset();
}

void KMSearchRuleWidget::initWidget()
{
  kdDebug() << "KMSearchRuleWidget::initWidget" << endl;
  //  QHBoxLayout *l = new QHBoxLayout(this,0,4); // with margin 0 and spacing 4
  
  setSpacing(4);

  mRuleField = new QComboBox( true, this, "mRuleField" );
  mRuleFunc = new QComboBox( false, this, "mRuleFunc" );
  mRuleValue = new QLineEdit( this, "mRuleValue" );
  
  mRuleFunc->insertStringList(sFilterFuncList);
  mRuleFunc->adjustSize();

  mRuleField->insertStringList(sFilterFieldList);
  mRuleField->adjustSize();
  
  connect( mRuleField, SIGNAL(textChanged(const QString &)),
	   this, SIGNAL(fieldChanged(const QString &)) );
  connect( mRuleValue, SIGNAL(textChanged(const QString &)),
	   this, SIGNAL(contentsChanged(const QString &)) );
}

void KMSearchRuleWidget::setRule(KMSearchRule *aRule)
{
  assert ( aRule );
  kdDebug() << "KMSearchRuleWidget::setRule:" << endl;
  kdDebug() << aRule->asString() << endl;

  kdDebug() << aRule->function() << endl;

  //--------------set the field
  int i = indexOfRuleField( aRule->field() );
  
  kdDebug() << aRule->function() << endl;

  if ( i<0 ) { // not found -> user defined field
    mRuleField->changeItem( aRule->field(), 0 );
    i=0;
  } else // found in the list of predefined fields
    mRuleField->changeItem( " ", 0 );
  
  mRuleField->setCurrentItem( i );

  kdDebug() << aRule->function() << endl;

  //--------------set function and contents
  mRuleFunc->setCurrentItem( (int)aRule->function() );
  mRuleValue->setText( aRule->contents() );

  kdDebug() << "KMSearchRule::setRule: left" << endl;
}

KMSearchRule* KMSearchRuleWidget::rule() const
{
  KMSearchRule *r = new KMSearchRule;

  kdDebug() << "r->init( " << mRuleField->currentText() << ", "
	    << mRuleFunc->currentItem() << ", "
	    << mRuleValue->text() << " );" << endl;

  r->init( mRuleField->currentText(),
	   (KMSearchRule::Function)mRuleFunc->currentItem(),
	   mRuleValue->text() );

  return r;
}

void KMSearchRuleWidget::reset()
{
  kdDebug() << "KMSearchRuleWidget::reset" << endl;

  mRuleField->changeItem( " ", 0 );
  mRuleField->setCurrentItem( 0 );

  mRuleFunc->setCurrentItem( 0 );

  mRuleValue->clear();
}

QString KMSearchRuleWidget::ruleFieldToEnglish(const QString & i18nVal) const
{
  kdDebug() << "ruleFieldToEnglish: 18nVal = \"" << i18nVal << "\"" << endl;
  if (i18nVal == i18n("<message>")) return QString("<message>");
  if (i18nVal == i18n("<body>")) return QString("<body>");
  if (i18nVal == i18n("<any header>")) return QString("<any header>");
  if (i18nVal == i18n("<To or Cc>")) return QString("<To or Cc>");
  return i18nVal;
}

int KMSearchRuleWidget::indexOfRuleField(const QString aName) const
{
  int i;

  if ( aName.isEmpty() ) return -1;

  for (i=sFilterFieldList.count()-1; i>=0; i--) {
    if (*(sFilterFieldList.at(i))==i18n(aName)) break;
  }
  return i;
}

void KMSearchRuleWidget::initLists() const
{
  kdDebug() << "KMSearchRuleWidget:: initLists" << endl;
  //---------- initialize list of filter operators
  if ( sFilterFuncList.isEmpty() )
  {
    // also see KMSearchRule::matches() and KMSearchRule::Function
    // you change the following strings!
    sFilterFuncList.append(i18n("equals"));
    sFilterFuncList.append(i18n("doesn't equal"));
    sFilterFuncList.append(i18n("contains"));
    sFilterFuncList.append(i18n("doesn't contain"));
    sFilterFuncList.append(i18n("matches regular expr."));
    sFilterFuncList.append(i18n("doesn't match reg. expr."));
  }

  //---------- initialize list of filter operators
  if ( sFilterFieldList.isEmpty() )
  {
    sFilterFieldList.append(" ");
    // also see KMSearchRule::matches() and ruleFieldToEnglish() if
    // you change the following i18n-ized strings!
    sFilterFieldList.append(i18n("<message>"));
    sFilterFieldList.append(i18n("<body>"));
    sFilterFieldList.append(i18n("<any header>"));
    sFilterFieldList.append(i18n("<To or Cc>"));
    // these others only represent meassage headers and you can add to
    // them as you like
    sFilterFieldList.append("Subject");
    sFilterFieldList.append("From");
    sFilterFieldList.append("To");
    sFilterFieldList.append("Cc");
    sFilterFieldList.append("Reply-To");
    sFilterFieldList.append("Organization");
    sFilterFieldList.append("Resent-From");
    sFilterFieldList.append("X-Loop");
  }
}

//=============================================================================
//
// class KMFilterActionWidgetLister (the filter action editor)
//
//=============================================================================

KMSearchRuleWidgetLister::KMSearchRuleWidgetLister( QWidget *parent, const char* name )
  : KWidgetLister( 1, FILTER_MAX_RULES, parent, name )
{
  kdDebug() << "KMSearchRuleWidgetLister::KMSearchRuleWidgetLister" << endl;
  mRuleList = 0;
}

KMSearchRuleWidgetLister::~KMSearchRuleWidgetLister()
{
}

void KMSearchRuleWidgetLister::setRuleList( QList<KMSearchRule> *aList )
{
  assert ( aList );
  kdDebug() << "KMSearchRuleWidgetLister::setRuleList called with a list containing "
	    << aList->count() << " items" << endl;

  if ( mRuleList )
    regenerateRuleListFromWidgets();

  mRuleList = aList;

  if ( mWidgetList.first() ) // move this below next 'if'?
    mWidgetList.first()->blockSignals(TRUE);

  if ( aList->count() == 0 ) {
    slotClear();
    mWidgetList.first()->blockSignals(FALSE);
    return;
  }

  int superfluousItems = (int)mRuleList->count() - mMaxWidgets ;
  if ( superfluousItems > 0 ) {
    kdDebug() << "KMSearchRuleWidgetLister: Clipping rule list to "
	      << mMaxWidgets << " items!" << endl;

    for ( ; superfluousItems ; superfluousItems-- )
      mRuleList->removeLast();
  }

  // set the right number of widgets
  setNumberOfShownWidgetsTo( QMAX((int)mRuleList->count(),mMinWidgets) );

  // load the actions into the widgets
  QListIterator<KMSearchRule> rIt( *mRuleList );
  QListIterator<QWidget> wIt( mWidgetList );
  for ( rIt.toFirst(), wIt.toFirst() ;
	rIt.current() && wIt.current() ; ++rIt, ++wIt ) {
    kdDebug() << "about to call setRule for rule:\n" 
	      << (*rIt)->asString() << endl;
    ((KMSearchRuleWidget*)(*wIt))->setRule( (*rIt) );
  }

  assert( mWidgetList.first() );
  mWidgetList.first()->blockSignals(FALSE);
}

void KMSearchRuleWidgetLister::reset()
{
  if ( mRuleList )
    regenerateRuleListFromWidgets();

  mRuleList = 0;
  slotClear();
}

QWidget* KMSearchRuleWidgetLister::createWidget( QWidget *parent )
{
  return new KMSearchRuleWidget(parent);
}

void KMSearchRuleWidgetLister::clearWidget( QWidget *aWidget )
{
  if ( aWidget )
    ((KMSearchRuleWidget*)aWidget)->reset();
}

void KMSearchRuleWidgetLister::regenerateRuleListFromWidgets()
{
  kdDebug() << "KMSearchRuleWidgetLister::regenerateRuleListFromWidgets" << endl;
  if ( !mRuleList ) return;

  mRuleList->clear();

  QListIterator<QWidget> it( mWidgetList );
  for ( it.toFirst() ; it.current() ; ++it ) {
    KMSearchRule *r = ((KMSearchRuleWidget*)(*it))->rule();
    if ( r )
      mRuleList->append( r );
  }
}




//=============================================================================
//
// class KMSearchPatternEdit
//
//=============================================================================

KMSearchPatternEdit::KMSearchPatternEdit(QWidget *parent, const char *name )
  : QGroupBox( 1/*columns*/, Horizontal, parent, name )
{
  kdDebug() << "KMSearchPatternEdit::KMSearchPatternEdit" << endl;
  setTitle( i18n("Search Criteria") );
  initLayout();
}

KMSearchPatternEdit::KMSearchPatternEdit(const QString & title, QWidget *parent, const char *name )
  : QGroupBox( 1/*column*/, Horizontal, title, parent, name )
{
  kdDebug() << "KMSearchPatternEdit::KMSearchPatternEdit" << endl;
  initLayout();
}

KMSearchPatternEdit::~KMSearchPatternEdit()
{
}

void KMSearchPatternEdit::initLayout()
{
  kdDebug() << "KMSearchPatternEdit::initLayout" << endl;

  //------------the radio buttons	
  mAllRBtn = new QRadioButton( i18n("Match all of the following"), this, "mAllRBtn" );
  mAnyRBtn = new QRadioButton( i18n("Match any of the following"), this, "mAnyRBtn" );
  
  mAllRBtn->setChecked(TRUE);
  mAnyRBtn->setChecked(FALSE);

  QButtonGroup *bg = new QButtonGroup( this );
  bg->hide();
  bg->insert( mAllRBtn, (int)KMSearchPattern::OpAnd );
  bg->insert( mAnyRBtn, (int)KMSearchPattern::OpOr );

  //------------the list of KMSearchRuleWidget's
  mRuleLister = new KMSearchRuleWidgetLister( this );
  mRuleLister->slotClear();

  //------------connect a few signals
  connect( bg, SIGNAL(clicked(int)),
	   this, SLOT(slotRadioClicked(int)) );

  KMSearchRuleWidget *srw = (KMSearchRuleWidget*)mRuleLister->mWidgetList.first();
  if ( srw ) {
    connect( srw, SIGNAL(fieldChanged(const QString &)),
	     this, SLOT(slotAutoNameHack()) );
    connect( srw, SIGNAL(contentsChanged(const QString &)),
	     this, SLOT(slotAutoNameHack()) );
  } else
    kdDebug() << "KMSearchPatternEdit: no first KMSearchRuleWidget, though slotClear() has been called!" << endl;
}

void KMSearchPatternEdit::setSearchPattern( KMSearchPattern *aPattern )
{
  assert( aPattern );
  kdDebug() << "KMSearchPatternEdit::setSearchPattern called with a pattern containing "
	    << aPattern->count() << " rules" << endl;

  blockSignals(TRUE);

  mRuleLister->setRuleList( aPattern );
  
  mPattern = aPattern;

  if ( mPattern->op() == KMSearchPattern::OpOr )
    mAnyRBtn->setChecked(TRUE);
  else
    mAllRBtn->setChecked(TRUE);

  setEnabled( TRUE );

  blockSignals(FALSE);
}

void KMSearchPatternEdit::reset()
{
  mRuleLister->reset();
  mAllRBtn->setChecked( TRUE );
  setEnabled( FALSE );
}

void KMSearchPatternEdit::slotRadioClicked(int aIdx)
{
  dumpObjectInfo();
  kdDebug() << "signals blocked: " << signalsBlocked() << endl;
  mRuleLister->dumpObjectInfo();
  kdDebug() << "signals blocked: " << mRuleLister->signalsBlocked() << endl;
  mRuleLister->mWidgetList.first()->dumpObjectInfo();
  kdDebug() << "signals blocked: " << mRuleLister->mWidgetList.first()->signalsBlocked() << endl;
  kdDebug() << "KMSearchPatternEdit::slotRadioClicked: aIdx=" << aIdx << endl;
  if ( mPattern ) 
    mPattern->setOp( (KMSearchPattern::Operator)aIdx );
}

void KMSearchPatternEdit::slotAutoNameHack()
{
  mRuleLister->regenerateRuleListFromWidgets();
  emit maybeNameChanged();
}
#include "kmsearchpatternedit.moc"
