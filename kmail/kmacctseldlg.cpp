// kmacctseldlg.cpp

#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <kapp.h>
#include <qlayout.h>

#include "kmacctseldlg.h"
#include "kmglobal.h"
#include <klocale.h>

//-----------------------------------------------------------------------------
KMAcctSelDlg::KMAcctSelDlg(QWidget* parent, const char* name):
  KMAcctSelDlgInherited(parent, name, TRUE)
{
  QPushButton *cancel;
  QRadioButton *rbtn;

  initMetaObject();

  setFixedSize(230,180);
  setCaption(name);
  mSelBtn = 0;

  grp = new QButtonGroup(i18n("Account Type"), this);
  grp->resize(230, 140);
  connect(grp, SIGNAL(clicked(int)), SLOT(buttonClicked(int)));

  ok = new QPushButton(i18n("OK"), this);
  ok->adjustSize();
  ok->setMinimumSize(ok->sizeHint());
  ok->resize(100, ok->size().height());
  ok->move(10, 175-ok->size().height());
  ok->setEnabled(FALSE);
  connect(ok, SIGNAL(clicked()), SLOT(accept()));

  cancel = new QPushButton(i18n("Cancel"), this);
  cancel->adjustSize();
  cancel->setMinimumSize(cancel->sizeHint());
  cancel->resize(100, cancel->size().height());
  cancel->move(120, 175-cancel->size().height());
  connect(cancel, SIGNAL(clicked()), SLOT(reject()));

  rbtn = new QRadioButton(i18n("Local Mailbox"), grp);
  rbtn->adjustSize();
  rbtn->move(30,30);

  rbtn = new QRadioButton(i18n("Pop3"), grp);
  rbtn->adjustSize();
  rbtn->move(30,60);

  rbtn = new QRadioButton(i18n("Experimental Pop3"), grp);
  rbtn->adjustSize();
  rbtn->move(30,90);
}


//-----------------------------------------------------------------------------
void KMAcctSelDlg::buttonClicked(int id)
{
  mSelBtn = id;
  ok->setEnabled(TRUE);
}


//-----------------------------------------------------------------------------
#include "kmacctseldlg.moc"
