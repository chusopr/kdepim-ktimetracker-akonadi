/*
    knconfigwidgets.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2000 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include "knconfig.h"

#include <qlayout.h>
#include <qbuttongroup.h>
#include <qvbox.h>

#include <klocale.h>
#include <knumvalidator.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kcolordialog.h>
#include <kfontdialog.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <krun.h>
#include <kuserprofile.h>
#include <kopenwith.h>
#include <kcharsets.h>
#include <kdebug.h>

#include "knaccountmanager.h"
#include "kngroupmanager.h"
#include "knglobals.h"
#include "knnntpaccount.h"
#include "utilities.h"
#include "knarticlewidget.h"
#include "knfiltermanager.h"
#include "knarticlefilter.h"
#include "knmime.h"


KNConfig::IdentityWidget::IdentityWidget(Identity *d, QWidget *p, const char *n) : BaseWidget(p, n), d_ata(d)
{
  QGridLayout *topL=new QGridLayout(this,  9, 3, 5,5);

  n_ame=new QLineEdit(this);
  QLabel *l=new QLabel(n_ame, i18n("&Name:"), this);
  topL->addWidget(l, 0,0);
  topL->addMultiCellWidget(n_ame, 0,0, 1,2);
  n_ame->setText(d_ata->n_ame);

  o_rga=new QLineEdit(this);
  l=new QLabel(o_rga, i18n("Organi&zation:"), this);
  topL->addWidget(l, 1,0);
  topL->addMultiCellWidget(o_rga, 1,1, 1,2);
  o_rga->setText(d_ata->o_rga);

  e_mail=new QLineEdit(this);
  l=new QLabel(e_mail, i18n("Email A&ddress:"), this);
  topL->addWidget(l, 2,0);
  topL->addMultiCellWidget(e_mail, 2,2, 1,2);
  e_mail->setText(d_ata->e_mail);

  r_eplyTo=new QLineEdit(this);
  l=new QLabel(r_eplyTo, i18n("&Reply-To Address:"), this);
  topL->addWidget(l, 3,0);
  topL->addMultiCellWidget(r_eplyTo, 3,3, 1,2);
  r_eplyTo->setText(d_ata->r_eplyTo);

  QButtonGroup *buttonGroup = new QButtonGroup(this);
  connect( buttonGroup, SIGNAL(clicked(int)),
           this, SLOT(slotSignatureType(int)) );
  buttonGroup->hide();

  s_igFile = new QRadioButton( i18n("&Use a signature from file"), this );
  buttonGroup->insert(s_igFile);
  topL->addMultiCellWidget(s_igFile, 4, 4, 0, 2);

  s_ig = new QLineEdit(this);
  f_ileName = new QLabel(s_ig, i18n("Signature &File:"), this);
  topL->addWidget(f_ileName, 5, 0 );
  topL->addWidget(s_ig, 5, 1 );
  s_ig->setText(d_ata->s_igPath);

  c_hooseBtn = new QPushButton( i18n("Choo&se..."), this);
  connect(c_hooseBtn, SIGNAL(clicked()),
          this, SLOT(slotSignatureChoose()));
  topL->addWidget(c_hooseBtn, 5, 2 );
  e_ditBtn = new QPushButton( i18n("&Edit File"), this);
  connect(e_ditBtn, SIGNAL(clicked()),
          this, SLOT(slotSignatureEdit()));
  topL->addWidget(e_ditBtn, 6, 2);

  s_igEdit = new QRadioButton( i18n("Specify signature &below"), this);
  buttonGroup->insert(s_igEdit);
  topL->addMultiCellWidget(s_igEdit, 7, 7, 0, 2);

  s_igEditor = new QMultiLineEdit(this);
  topL->addMultiCellWidget(s_igEditor, 8, 8, 0, 2);
  s_igEditor->setText(d_ata->s_igText);

  topL->setColStretch(1,1);
  topL->setRowStretch(5,1);
  topL->setResizeMode(QLayout::Minimum);

  slotSignatureType(d_ata->useSigFile()? 0:1);
}


KNConfig::IdentityWidget::~IdentityWidget()
{
}


void KNConfig::IdentityWidget::apply()
{
  if(!d_irty)
    return;

  d_ata->n_ame=n_ame->text();
  d_ata->o_rga=o_rga->text();
  d_ata->e_mail=e_mail->text();
  d_ata->r_eplyTo=r_eplyTo->text();
  d_ata->u_seSigFile=s_igFile->isChecked();
  d_ata->s_igPath=s_ig->text();
  d_ata->s_igText=s_igEditor->text();

  if(d_ata->isGlobal())
    d_ata->save();
}


void KNConfig::IdentityWidget::slotSignatureType(int type)
{
  bool sigFromFile = (type==0);

  s_igFile->setChecked(sigFromFile);
  f_ileName->setEnabled(sigFromFile);
  s_ig->setEnabled(sigFromFile);
  c_hooseBtn->setEnabled(sigFromFile);
  e_ditBtn->setEnabled(sigFromFile);
  s_igEdit->setChecked(!sigFromFile);
  s_igEditor->setEnabled(!sigFromFile);

  if (sigFromFile)
    f_ileName->setFocus();
  else
    s_igEditor->setFocus();
}


void KNConfig::IdentityWidget::slotSignatureChoose()
{
  QString tmp=KFileDialog::getOpenFileName(s_ig->text(),QString::null,this,i18n("Choose Signature"));
  if(!tmp.isEmpty()) s_ig->setText(tmp);
}


void KNConfig::IdentityWidget::slotSignatureEdit()
{
  QString fileName = s_ig->text().stripWhiteSpace();

  if (fileName.isEmpty()) {
    KMessageBox::sorry(this, i18n("You must specify a filename!"));
    return;
  }

  QFileInfo fileInfo( fileName );
  if (fileInfo.isDir()) {
    KMessageBox::sorry(this, i18n("You have specified a directory!"));
    return;
  }

  KService::Ptr offer = KServiceTypeProfile::preferredService("text/plain", true);
  KURL::List  lst(fileName);

  if (offer)
    KRun::run(*offer, lst);
  else {
    KFileOpenWithHandler *openhandler = new KFileOpenWithHandler();
    openhandler->displayOpenWithDialog(lst);
  }
}



//==========================================================================================


KNConfig::NntpAccountListWidget::NntpAccountListWidget(QWidget *p, const char *n)
  : BaseWidget(p, n), p_ixmap(UserIcon("server")), a_ccManager(knGlobals.accManager)
{
  QGridLayout *topL=new QGridLayout(this, 6,2, 5,5);

  // account listbox
  l_box=new KNDialogListBox(false, this);
  connect(l_box, SIGNAL(selected(int)), this, SLOT(slotItemSelected(int)));
  connect(l_box, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()));
  topL->addMultiCellWidget(l_box, 0,4, 0,0);

  // info box
  QGroupBox *gb = new QGroupBox(2,Qt::Vertical,QString::null,this);
  topL->addWidget(gb,5,0);

  s_erverInfo = new QLabel(gb);
  p_ortInfo = new QLabel(gb);

  // buttons
  a_ddBtn=new QPushButton(i18n("&New"), this);
  connect(a_ddBtn, SIGNAL(clicked()), this, SLOT(slotAddBtnClicked()));
  topL->addWidget(a_ddBtn, 0,1);

  d_elBtn=new QPushButton(i18n("&Delete"), this);
  connect(d_elBtn, SIGNAL(clicked()), this, SLOT(slotDelBtnClicked()));
  topL->addWidget(d_elBtn, 1,1);

  e_ditBtn=new QPushButton(i18n("modify something","&Edit"), this);
  connect(e_ditBtn, SIGNAL(clicked()), this, SLOT(slotEditBtnClicked()));
  topL->addWidget(e_ditBtn, 2,1);

  s_ubBtn=new QPushButton(i18n("&Subscribe"), this);
  connect(s_ubBtn, SIGNAL(clicked()), this, SLOT(slotSubBtnClicked()));
  topL->addWidget(s_ubBtn, 3,1);

  topL->setRowStretch(4,1);   // stretch the server listbox


  for(KNNntpAccount *a=a_ccManager->first(); a; a=a_ccManager->next())
    slotAddItem(a);

  // the settings dialog is non-modal, so we have to react to changes
  // made outside of the dialog
  connect(a_ccManager, SIGNAL(accountAdded(KNNntpAccount*)), this, SLOT(slotAddItem(KNNntpAccount*)));
  connect(a_ccManager, SIGNAL(accountRemoved(KNNntpAccount*)), this, SLOT(slotRemoveItem(KNNntpAccount*)));
  connect(a_ccManager, SIGNAL(accountModified(KNNntpAccount*)), this, SLOT(slotUpdateItem(KNNntpAccount*)));

  slotSelectionChanged();     // disable Delete & Edit initially
}


KNConfig::NntpAccountListWidget::~NntpAccountListWidget()
{
}


void KNConfig::NntpAccountListWidget::slotAddItem(KNNntpAccount *a)
{
  LBoxItem *it;
  it=new LBoxItem(a, a->name(), &p_ixmap);
  l_box->insertItem(it);
}


void KNConfig::NntpAccountListWidget::slotRemoveItem(KNNntpAccount *a)
{
  LBoxItem *it;
  for(uint i=0; i<l_box->count(); i++) {
    it=static_cast<LBoxItem*>(l_box->item(i));
    if(it && it->account==a) {
      l_box->removeItem(i);
      break;
    }
  }
  slotSelectionChanged();
}


void KNConfig::NntpAccountListWidget::slotUpdateItem(KNNntpAccount *a)
{
  LBoxItem *it;
  for(uint i=0; i<l_box->count(); i++) {
    it=static_cast<LBoxItem*>(l_box->item(i));
    if(it && it->account==a) {
      it=new LBoxItem(a, a->name(), &p_ixmap);
      l_box->changeItem(it, i);
      break;
    }
  }
  slotSelectionChanged();
}



void KNConfig::NntpAccountListWidget::slotSelectionChanged()
{
  int curr=l_box->currentItem();
  d_elBtn->setEnabled(curr!=-1);
  e_ditBtn->setEnabled(curr!=-1);
  s_ubBtn->setEnabled(curr!=-1);

  LBoxItem *it = static_cast<LBoxItem*>(l_box->item(curr));
  if(it) {
    s_erverInfo->setText(i18n("Server: %1").arg(it->account->server()));
    p_ortInfo->setText(i18n("Port: %1").arg(it->account->port()));
  }
  else {
    s_erverInfo->setText(i18n("Server: "));
    p_ortInfo->setText(i18n("Port: "));
  }
}



void KNConfig::NntpAccountListWidget::slotItemSelected(int)
{
  slotEditBtnClicked();
}



void KNConfig::NntpAccountListWidget::slotAddBtnClicked()
{
  KNNntpAccount *acc = new KNNntpAccount();

  if(acc->editProperties(this))
    a_ccManager->newAccount(acc);
  else
    delete acc;
}



void KNConfig::NntpAccountListWidget::slotDelBtnClicked()
{
  LBoxItem *it = static_cast<LBoxItem*>(l_box->item(l_box->currentItem()));

  if(it)
    a_ccManager->removeAccount(it->account);
}



void KNConfig::NntpAccountListWidget::slotEditBtnClicked()
{
  LBoxItem *it = static_cast<LBoxItem*>(l_box->item(l_box->currentItem()));

  if(it) {
    it->account->editProperties(this);
    slotUpdateItem(it->account);
  }
}


void KNConfig::NntpAccountListWidget::slotSubBtnClicked()
{
  LBoxItem *it = static_cast<LBoxItem*>(l_box->item(l_box->currentItem()));

  if(it)
    knGlobals.grpManager->showGroupDialog(it->account, this);
}


//=======================================================================================


KNConfig::NntpAccountConfDialog::NntpAccountConfDialog(KNNntpAccount *a, QWidget *p, const char *n)
  : KDialogBase(Tabbed, (a->id()!=-1)? i18n("Properties of %1").arg(a->name()):i18n("New Account"),
                Ok|Cancel|Help, Ok, p, n),
    a_ccount(a)
{
  QFrame* page=addPage(i18n("Ser&ver"));
  QGridLayout *topL=new QGridLayout(page, 11, 3, 5);

  n_ame=new QLineEdit(page);
  QLabel *l=new QLabel(n_ame,i18n("&Name:"),page);
  topL->addWidget(l, 0,0);
  n_ame->setText(a->name());
  topL->addMultiCellWidget(n_ame, 0, 0, 1, 2);

  s_erver=new QLineEdit(page);
  l=new QLabel(s_erver,i18n("&Server:"), page);
  s_erver->setText(a->server());
  topL->addWidget(l, 1,0);
  topL->addMultiCellWidget(s_erver, 1, 1, 1, 2);

  p_ort=new QLineEdit(page);
  l=new QLabel(p_ort, i18n("&Port:"), page);
  p_ort->setValidator(new KIntValidator(0,65536,this));
  p_ort->setText(QString::number(a->port()));
  topL->addWidget(l, 2,0);
  topL->addWidget(p_ort, 2,1);

  h_old = new KIntSpinBox(5,1800,5,5,10,page);
  l = new QLabel(h_old,i18n("Hol&d connection for:"), page);
  h_old->setSuffix(i18n(" sec"));
  h_old->setValue(a->hold());
  topL->addWidget(l,3,0);
  topL->addWidget(h_old,3,1);

  t_imeout = new KIntSpinBox(15,600,5,15,10,page);
  l = new QLabel(t_imeout, i18n("&Timeout:"), page);
  t_imeout->setValue(a->timeout());
  t_imeout->setSuffix(i18n(" sec"));
  topL->addWidget(l,4,0);
  topL->addWidget(t_imeout,4,1);

  f_etchDes=new QCheckBox(i18n("&Fetch group descriptions"), page);
  f_etchDes->setChecked(a->fetchDescriptions());
  topL->addMultiCellWidget(f_etchDes, 5,5, 0,3);

  a_uth=new QCheckBox(i18n("Server requires &authentication"), page);
  connect(a_uth, SIGNAL(toggled(bool)), this, SLOT(slotAuthChecked(bool)));
  topL->addMultiCellWidget(a_uth, 6,6, 0,3);

  u_ser=new QLineEdit(page);
  u_serLabel=new QLabel(u_ser,i18n("&User:"), page);
  u_ser->setText(a->user());
  topL->addWidget(u_serLabel, 7,0);
  topL->addMultiCellWidget(u_ser, 7,7, 1,2);

  p_ass=new QLineEdit(page);
  p_assLabel=new QLabel(p_ass, i18n("Pass&word:"), page);
  p_ass->setEchoMode(QLineEdit::Password);
  p_ass->setText(a->pass());
  topL->addWidget(p_assLabel, 8,0);
  topL->addMultiCellWidget(p_ass, 8,8, 1,2);

  slotAuthChecked(a->needsLogon());

  topL->setColStretch(1, 1);
  topL->setColStretch(2, 1);

  // Specfic Identity tab =========================================
  i_dWidget=new KNConfig::IdentityWidget(a->identity(), addVBoxPage(i18n("&Identity")));

  restoreWindowSize("accNewsPropDLG", this, sizeHint());

  setHelp("anc-setting-the-news-account");
}



KNConfig::NntpAccountConfDialog::~NntpAccountConfDialog()
{
  saveWindowSize("accNewsPropDLG", size());
}


void KNConfig::NntpAccountConfDialog::slotOk()
{
  if (n_ame->text().isEmpty() || s_erver->text().isEmpty()) {
    KMessageBox::sorry(this, i18n("Please enter an arbitrary name for the account and the\nhostname of the news server."));
    return;
  }

  a_ccount->setName(n_ame->text());
  a_ccount->setServer(s_erver->text());
  a_ccount->setPort(p_ort->text().toInt());
  a_ccount->setHold(h_old->value());
  a_ccount->setTimeout(t_imeout->value());
  a_ccount->setFetchDescriptions(f_etchDes->isChecked());
  a_ccount->setNeedsLogon(a_uth->isChecked());
  a_ccount->setUser(u_ser->text());
  a_ccount->setPass(p_ass->text());

  i_dWidget->apply();

  accept();
}


void KNConfig::NntpAccountConfDialog::slotAuthChecked(bool b)
{
  a_uth->setChecked(b);
  u_ser->setEnabled(b);
  u_serLabel->setEnabled(b);
  p_ass->setEnabled(b);
  p_assLabel->setEnabled(b);
}


//=============================================================================================


KNConfig::SmtpAccountWidget::SmtpAccountWidget(QWidget *p, const char *n) : BaseWidget(p, n)
{
  QGridLayout *topL=new QGridLayout(this, 5, 3, 5);

  s_erver=new QLineEdit(this);
  QLabel *l=new QLabel(s_erver, i18n("&Server:"), this);
  topL->addWidget(l, 0,0);
  topL->addMultiCellWidget(s_erver, 0, 0, 1, 2);

  p_ort=new QLineEdit(this);
  l=new QLabel(p_ort, i18n("&Port:"), this);
  topL->addWidget(l, 1,0);
  p_ort->setValidator(new KIntValidator(0,65536,this));
  topL->addWidget(p_ort, 1,1);

  h_old = new KIntSpinBox(0,300,5,0,10,this);
  h_old->setSuffix(i18n(" sec"));
  l = new QLabel(h_old, i18n("Hol&d connection for:"), this);
  topL->addWidget(l,2,0);
  topL->addWidget(h_old,2,1);

  t_imeout = new KIntSpinBox(15,300,5,15,10,this);
  t_imeout->setSuffix(i18n(" sec"));
  l = new QLabel(t_imeout, i18n("&Timeout:"), this);
  topL->addWidget(l,3,0);
  topL->addWidget(t_imeout,3,1);

  topL->setColStretch(1,1);
  topL->setColStretch(2,1);

  s_erverInfo=knGlobals.accManager->smtp();

  s_erver->setText(s_erverInfo->server());
  p_ort->setText(QString::number(s_erverInfo->port()));
  h_old->setValue(s_erverInfo->hold());
  t_imeout->setValue(s_erverInfo->timeout());

}



KNConfig::SmtpAccountWidget::~SmtpAccountWidget()
{
}


void KNConfig::SmtpAccountWidget::apply()
{
  s_erverInfo->setServer(s_erver->text());
  s_erverInfo->setPort(p_ort->text().toInt());
  s_erverInfo->setHold(h_old->value());
  s_erverInfo->setTimeout(t_imeout->value());

  KConfig *conf=KGlobal::config();
  conf->setGroup("MAILSERVER");
  s_erverInfo->saveConf(conf);
}


//=============================================================================================


//===================================================================================
// code taken from KMail, Copyright (C) 2000 Espen Sand, espen@kde.org

KNConfig::AppearanceWidget::ColorListItem::ColorListItem( const QString &text, const QColor &color )
  : QListBoxText(text), mColor( color )
{
}


KNConfig::AppearanceWidget::ColorListItem::~ColorListItem()
{
}


void KNConfig::AppearanceWidget::ColorListItem::paint( QPainter *p )
{
  QFontMetrics fm = p->fontMetrics();
  int h = fm.height();

  p->drawText( 30+3*2, fm.ascent() + fm.leading()/2, text() );

  p->setPen( Qt::black );
  p->drawRect( 3, 1, 30, h-1 );
  p->fillRect( 4, 2, 28, h-3, mColor );
}


int KNConfig::AppearanceWidget::ColorListItem::height(const QListBox *lb ) const
{
  return( lb->fontMetrics().lineSpacing()+1 );
}


int KNConfig::AppearanceWidget::ColorListItem::width(const QListBox *lb ) const
{
  return( 30 + lb->fontMetrics().width( text() ) + 6 );
}


//===================================================================================


KNConfig::AppearanceWidget::FontListItem::FontListItem( const QString &name, const QFont &font )
  : QListBoxText(name), f_ont(font)
{
  fontInfo = QString("[%1 %2]").arg(f_ont.family()).arg(f_ont.pointSize());
}


KNConfig::AppearanceWidget::FontListItem::~FontListItem()
{
}


void KNConfig::AppearanceWidget::FontListItem::setFont(const QFont &font)
{
  f_ont = font;
  fontInfo = QString("[%1 %2]").arg(f_ont.family()).arg(f_ont.pointSize());
}


void KNConfig::AppearanceWidget::FontListItem::paint( QPainter *p )
{
  QFont fnt = p->font();
  fnt.setWeight(QFont::Bold);
  p->setFont(fnt);
  int fontInfoWidth = p->fontMetrics().width(fontInfo);
  int h = p->fontMetrics().ascent() + p->fontMetrics().leading()/2;
  p->drawText(2, h, fontInfo );
  fnt.setWeight(QFont::Normal);
  p->setFont(fnt);
  p->drawText(5 + fontInfoWidth, h, text() );
}


int KNConfig::AppearanceWidget::FontListItem::width(const QListBox *lb ) const
{
  return( lb->fontMetrics().width(fontInfo) + lb->fontMetrics().width(text()) + 20 );
}


//===================================================================================


KNConfig::AppearanceWidget::AppearanceWidget(Appearance *d, QWidget *p, const char *n)
  : BaseWidget(p, n), d_ata(d)
{
  QGridLayout *topL=new QGridLayout(this, 9,2, 5,5);

  //long group list
  l_ongCB = new QCheckBox(i18n("&Show long group list"),this);
  topL->addWidget(l_ongCB,0,0);

  //color-list
  c_List = new KNDialogListBox(false, this);
  topL->addMultiCellWidget(c_List,2,4,0,0);
  connect(c_List, SIGNAL(selected(QListBoxItem*)),SLOT(slotColItemSelected(QListBoxItem*)));
  connect(c_List, SIGNAL(selectionChanged()), SLOT(slotColSelectionChanged()));

  c_olorCB = new QCheckBox(i18n("&Use custom colors"),this);
  topL->addWidget(c_olorCB,1,0);
  connect(c_olorCB, SIGNAL(toggled(bool)), this, SLOT(slotColCheckBoxToggled(bool)));

  c_olChngBtn=new QPushButton(i18n("Cha&nge"), this);
  connect(c_olChngBtn, SIGNAL(clicked()), this, SLOT(slotColChangeBtnClicked()));
  topL->addWidget(c_olChngBtn,2,1);

  c_olDefBtn=new QPushButton(i18n("&Defaults"), this);
  connect(c_olDefBtn, SIGNAL(clicked()), this, SLOT(slotColDefaultBtnClicked()));
  topL->addWidget(c_olDefBtn,3,1);


  //font-list
  f_List = new KNDialogListBox(false, this);
  topL->addMultiCellWidget(f_List,6,8,0,0);
  connect(f_List, SIGNAL(selected(QListBoxItem*)),SLOT(slotFontItemSelected(QListBoxItem*)));
  connect(f_List, SIGNAL(selectionChanged()),SLOT(slotFontSelectionChanged()));

  f_ontCB = new QCheckBox(i18n("Use custom &fonts"),this);
  topL->addWidget(f_ontCB,5,0);
  connect(f_ontCB, SIGNAL(toggled(bool)), this, SLOT(slotFontCheckBoxToggled(bool)));

  f_ntChngBtn=new QPushButton(i18n("Chang&e"), this);
  connect(f_ntChngBtn, SIGNAL(clicked()), this, SLOT(slotFontChangeBtnClicked()));
  topL->addWidget(f_ntChngBtn,6,1);

  f_ntDefBtn=new QPushButton(i18n("Defaul&ts"), this);
  connect(f_ntDefBtn, SIGNAL(clicked()), this, SLOT(slotFontDefaultBtnClicked()));
  topL->addWidget(f_ntDefBtn,7,1);

  //init
  l_ongCB->setChecked(d->l_ongGroupList);

  c_olorCB->setChecked(d->u_seColors);
  slotColCheckBoxToggled(d->u_seColors);
  for(int i=0; i<d->colorCount(); i++)
    c_List->insertItem(new ColorListItem(d->colorName(i), d->color(i)));

  f_ontCB->setChecked(d->u_seFonts);
  slotFontCheckBoxToggled(d->u_seFonts);
  for(int i=0; i<d->fontCount(); i++)
    f_List->insertItem(new FontListItem(d->fontName(i), d->font(i)));

}



KNConfig::AppearanceWidget::~AppearanceWidget()
{
}


void KNConfig::AppearanceWidget::apply()
{
  if(!d_irty)
    return;

  d_ata->l_ongGroupList=l_ongCB->isChecked();

  d_ata->u_seColors=c_olorCB->isChecked();
  for(int i=0; i<d_ata->colorCount(); i++)
    d_ata->c_olors[i] = (static_cast<ColorListItem*>(c_List->item(i)))->color();

  d_ata->u_seFonts=f_ontCB->isChecked();
  for(int i=0; i<d_ata->fontCount(); i++)
    d_ata->f_onts[i] = (static_cast<FontListItem*>(f_List->item(i)))->font();

  d_ata->save();
}


void KNConfig::AppearanceWidget::slotColCheckBoxToggled(bool b)
{
  c_List->setEnabled(b);
  c_olDefBtn->setEnabled(b);
  c_olChngBtn->setEnabled(b && (c_List->currentItem()!=-1));
  if (b) c_List->setFocus();
}


// show color dialog for the entry
void KNConfig::AppearanceWidget::slotColItemSelected(QListBoxItem *it)
{
  if (it) {
    ColorListItem *colorItem = static_cast<ColorListItem*>(it);
    QColor col = colorItem->color();
    int result = KColorDialog::getColor(col,this);

    if (result == KColorDialog::Accepted) {
      colorItem->setColor(col);
      c_List->triggerUpdate(false);
    }
  }
}


void KNConfig::AppearanceWidget::slotColDefaultBtnClicked()
{
  ColorListItem *colorItem;
  for(int i=0; i < d_ata->colorCount(); i++) {
    colorItem=static_cast<ColorListItem*>(c_List->item(i));
    colorItem->setColor(d_ata->defaultColor(i));
  }
  c_List->triggerUpdate(true);
  c_List->repaint(true);
}


void KNConfig::AppearanceWidget::slotColChangeBtnClicked()
{
  if(c_List->currentItem()!=-1)
    slotColItemSelected(c_List->item(c_List->currentItem()));
}


void KNConfig::AppearanceWidget::slotColSelectionChanged()
{
  c_olChngBtn->setEnabled(c_List->currentItem()!=-1);
}


void KNConfig::AppearanceWidget::slotFontCheckBoxToggled(bool b)
{
  f_List->setEnabled(b);
  f_ntDefBtn->setEnabled(b);
  f_ntChngBtn->setEnabled(b && (f_List->currentItem()!=-1));
  if (b) f_List->setFocus();
}


// show font dialog for the entry
void KNConfig::AppearanceWidget::slotFontItemSelected(QListBoxItem *it)
{
  if (it) {
    FontListItem *fontItem = static_cast<FontListItem*>(it);
    QFont font = fontItem->font();
    int result = KFontDialog::getFont(font,false,this);

    if (result == KFontDialog::Accepted) {
      fontItem->setFont(font);
      f_List->triggerUpdate(false);
    }
  }
}


void KNConfig::AppearanceWidget::slotFontDefaultBtnClicked()
{
  FontListItem *fontItem;
  for(int i=0; i < d_ata->fontCount(); i++) {
    fontItem=static_cast<FontListItem*>(f_List->item(i));
    fontItem->setFont(d_ata->defaultFont(i));
  }
  f_List->triggerUpdate(false);
}


void KNConfig::AppearanceWidget::slotFontChangeBtnClicked()
{
  if(f_List->currentItem()!=-1)
    slotFontItemSelected(f_List->item(f_List->currentItem()));
}


void KNConfig::AppearanceWidget::slotFontSelectionChanged()
{
  f_ntChngBtn->setEnabled(f_List->currentItem()!=-1);
}


//=============================================================================================


KNConfig::ReadNewsGeneralWidget::ReadNewsGeneralWidget(ReadNewsGeneral *d, QWidget *p, const char *n)
  : BaseWidget(p, n), d_ata(d)
{
  QGroupBox *hgb=new QGroupBox(i18n("Article Handling"), this);
  QGroupBox *lgb=new QGroupBox(i18n("Article List"), this);
  QLabel *l1;

  a_utoCB=new QCheckBox(i18n("Check for new articles a&utomatically"), hgb);
  m_axFetch=new KIntSpinBox(0, 20000, 1, 0, 10, hgb);
  l1=new QLabel(m_axFetch, i18n("&Maximal number of articles to fetch"), hgb);
  m_arkCB=new QCheckBox(i18n("Mar&k article as read after"), hgb);
  m_arkSecs=new KIntSpinBox(0, 9999, 1, 0, 10, hgb);
  connect(m_arkCB, SIGNAL(toggled(bool)), m_arkSecs, SLOT(setEnabled(bool)));
  m_arkSecs->setSuffix(i18n(" sec"));

  e_xpThrCB=new QCheckBox(i18n("Show &whole thread on expanding"), lgb);
  s_coreCB=new QCheckBox(i18n("Show article &score"), lgb);
  l_inesCB=new QCheckBox(i18n("Show &line count"), lgb);

  QVBoxLayout *topL=new QVBoxLayout(this, 5);
  QGridLayout *hgbL=new QGridLayout(hgb, 4,2, 8,5);
  QVBoxLayout *lgbL=new QVBoxLayout(lgb, 8, 5);

  topL->addWidget(hgb);
  topL->addWidget(lgb);
  topL->addStretch(1);

  hgbL->addRowSpacing(0, fontMetrics().lineSpacing()-4);
  hgbL->addWidget(a_utoCB, 1,0);
  hgbL->addWidget(l1, 2, 0);
  hgbL->addWidget(m_axFetch, 2,1);
  hgbL->addWidget(m_arkCB, 3,0);
  hgbL->addWidget(m_arkSecs, 3,1);
  hgbL->setColStretch(0,1);

  lgbL->addSpacing(fontMetrics().lineSpacing()-4);
  lgbL->addWidget(e_xpThrCB);
  lgbL->addWidget(s_coreCB);
  lgbL->addWidget(l_inesCB);

  topL->setResizeMode(QLayout::Minimum);

  //init
  a_utoCB->setChecked(d->a_utoCheck);
  m_axFetch->setValue(d->m_axFetch);
  m_arkCB->setChecked(d->a_utoMark);
  m_arkSecs->setValue(d->m_arkSecs);
  m_arkSecs->setEnabled(d->a_utoMark);
  e_xpThrCB->setChecked(d->t_otalExpand);
  l_inesCB->setChecked(d->s_howLines);
  s_coreCB->setChecked(d->s_howScore);
}


KNConfig::ReadNewsGeneralWidget::~ReadNewsGeneralWidget()
{
}


void KNConfig::ReadNewsGeneralWidget::apply()
{
  if(!d_irty)
    return;

  d_ata->a_utoCheck=a_utoCB->isChecked();
  d_ata->m_axFetch=m_axFetch->value();
  d_ata->a_utoMark=m_arkCB->isChecked();
  d_ata->m_arkSecs=m_arkSecs->value();
  d_ata->t_otalExpand=e_xpThrCB->isChecked();
  d_ata->s_howLines=l_inesCB->isChecked();
  d_ata->s_howScore=s_coreCB->isChecked();

  d_ata->save();
}


//=============================================================================================


KNConfig::ReadNewsViewerWidget::ReadNewsViewerWidget(ReadNewsViewer *d, QWidget *p, const char *n)
  : BaseWidget(p, n), d_ata(d)
{
  QGroupBox *appgb=new QGroupBox(i18n("Appearance"), this);
  QGroupBox *agb=new QGroupBox(i18n("Attachments"), this);
  QGroupBox *bgb=new QGroupBox(i18n("Browser"), this);
  QLabel *l1;

  s_igCB=new QCheckBox(i18n("Show sig&nature"), appgb);
  f_ormatCB=new QCheckBox(i18n("Interpret te&xt format tags"), appgb);

  i_nlineCB=new QCheckBox(i18n("Show attachments &inline if possible"), agb);
  o_penAttCB=new QCheckBox(i18n("Open a&ttachments on click"), agb);
  a_ltAttCB=new QCheckBox(i18n("Show alternati&ve contents as attachments"), agb);

  b_rowser=new QComboBox(bgb);
  b_rowser->insertItem("Konqueror");
  b_rowser->insertItem("Netscape");
  b_rowser->insertItem("Mozilla");
  b_rowser->insertItem("Opera");
  b_rowser->insertItem("Other");
  connect(b_rowser, SIGNAL(activated(int)), SLOT(slotBrowserTypeChanged(int)));
  l1=new QLabel(b_rowser, i18n("Open &links with"), bgb);
  b_rowserCommand = new QLineEdit(bgb);
  c_hooseBrowser= new QPushButton(i18n("Choo&se..."),bgb);
  connect(c_hooseBrowser, SIGNAL(clicked()), SLOT(slotChooseBrowser()));

  QVBoxLayout *topL=new QVBoxLayout(this, 5);
  QVBoxLayout *appgbL=new QVBoxLayout(appgb, 8, 5);
  QVBoxLayout *agbL=new QVBoxLayout(agb, 8, 5);
  QGridLayout *bgbL=new QGridLayout(bgb, 3,3, 8,5);

  topL->addWidget(appgb);
  topL->addWidget(agb);
  topL->addWidget(bgb);
  topL->addStretch(1);

  appgbL->addSpacing(fontMetrics().lineSpacing()-4);
  appgbL->addWidget(s_igCB);
  appgbL->addWidget(f_ormatCB);

  agbL->addSpacing(fontMetrics().lineSpacing()-4);
  agbL->addWidget(i_nlineCB);
  agbL->addWidget(o_penAttCB);
  agbL->addWidget(a_ltAttCB);

  bgbL->addRowSpacing(0, fontMetrics().lineSpacing()-4);
  bgbL->addWidget(l1, 1,0);
  bgbL->addMultiCellWidget(b_rowser,1,1,1,2);
  bgbL->addMultiCellWidget(b_rowserCommand,2,2,0,1);
  bgbL->addWidget(c_hooseBrowser,2,2);
  bgbL->setColStretch(1,1);

  topL->setResizeMode(QLayout::Minimum);

  //init
  s_igCB->setChecked(d->s_howSig);
  f_ormatCB->setChecked(d->i_nterpretFormatTags);
  i_nlineCB->setChecked(d->i_nlineAtt);
  o_penAttCB->setChecked(d->o_penAtt);
  a_ltAttCB->setChecked(d->s_howAlts);
  b_rowser->setCurrentItem((int)(d->b_rowser));
  b_rowserCommand->setText(d->b_rowserCommand);
  b_rowserCommand->setEnabled(d->b_rowser==ReadNewsViewer::BTother);
  c_hooseBrowser->setEnabled(d->b_rowser==ReadNewsViewer::BTother);
}


KNConfig::ReadNewsViewerWidget::~ReadNewsViewerWidget()
{
}


void KNConfig::ReadNewsViewerWidget::apply()
{
  if(!d_irty)
    return;

  d_ata->s_howSig=s_igCB->isChecked();
  d_ata->i_nterpretFormatTags=f_ormatCB->isChecked();
  d_ata->i_nlineAtt=i_nlineCB->isChecked();
  d_ata->o_penAtt=o_penAttCB->isChecked();
  d_ata->s_howAlts=a_ltAttCB->isChecked();
  d_ata->b_rowser=(ReadNewsViewer::browserType)(b_rowser->currentItem());
  d_ata->b_rowserCommand=b_rowserCommand->text();

  d_ata->save();
}


void KNConfig::ReadNewsViewerWidget::slotBrowserTypeChanged(int i)
{
  bool enabled=((ReadNewsViewer::browserType)(i)==ReadNewsViewer::BTother);
  b_rowserCommand->setEnabled(enabled);
  c_hooseBrowser->setEnabled(enabled);
}


void KNConfig::ReadNewsViewerWidget::slotChooseBrowser()
{
  QString path=b_rowserCommand->text().simplifyWhiteSpace();
  if (path.right(3) == " %u")
    path.truncate(path.length()-3);

  path=KFileDialog::getOpenFileName(path, QString::null, this, i18n("Choose Browser"));

  if (!path.isEmpty())
    b_rowserCommand->setText(path+" %u");
}


//=============================================================================================


KNConfig::DisplayedHeadersWidget::DisplayedHeadersWidget(DisplayedHeaders *d, QWidget *p, const char *n)
  : BaseWidget(p, n), s_ave(false), d_ata(d)
{
  QGridLayout *topL=new QGridLayout(this, 7,2, 5,5);

  //listbox
  l_box=new KNDialogListBox(false, this);
  connect(l_box, SIGNAL(selected(int)), this, SLOT(slotItemSelected(int)));
  connect(l_box, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()));
  topL->addMultiCellWidget(l_box, 0,6, 0,0);

  // buttons
  a_ddBtn=new QPushButton(i18n("&New"), this);
  connect(a_ddBtn, SIGNAL(clicked()), this, SLOT(slotAddBtnClicked()));
  topL->addWidget(a_ddBtn, 0,1);

  d_elBtn=new QPushButton(i18n("&Delete"), this);
  connect(d_elBtn, SIGNAL(clicked()), this, SLOT(slotDelBtnClicked()));
  topL->addWidget(d_elBtn, 1,1);

  e_ditBtn=new QPushButton(i18n("modify something","&Edit"), this);
  connect(e_ditBtn, SIGNAL(clicked()), this, SLOT(slotEditBtnClicked()));
  topL->addWidget(e_ditBtn, 2,1);

  u_pBtn=new QPushButton(i18n("&Up"), this);
  connect(u_pBtn, SIGNAL(clicked()), this, SLOT(slotUpBtnClicked()));
  topL->addWidget(u_pBtn, 4,1);

  d_ownBtn=new QPushButton(i18n("Do&wn"), this);
  connect(d_ownBtn, SIGNAL(clicked()), this, SLOT(slotDownBtnClicked()));
  topL->addWidget(d_ownBtn, 5,1);

  topL->addRowSpacing(3,20);        // separate up/down buttons
  topL->setRowStretch(6,1);         // stretch the listbox

  for(KNDisplayedHeader *h=d->h_drList.first(); h; h=d->h_drList.next())
    l_box->insertItem(generateItem(h));

  slotSelectionChanged();     // disable buttons initially
}



KNConfig::DisplayedHeadersWidget::~DisplayedHeadersWidget()
{
  if(s_ave)
    d_ata->save();
}



KNConfig::DisplayedHeadersWidget::HdrItem* KNConfig::DisplayedHeadersWidget::generateItem(KNDisplayedHeader *h)
{
  QString text;
  if(h->hasName()) {
    text=h->translatedName();
    text+=": <";
  } else
    text="<";
  text+=h->header();
  text+=">";
  return new HdrItem(text,h);
}



void KNConfig::DisplayedHeadersWidget::slotItemSelected(int)
{
  slotEditBtnClicked();
}



void KNConfig::DisplayedHeadersWidget::slotSelectionChanged()
{
  int curr = l_box->currentItem();
  d_elBtn->setEnabled(curr!=-1);
  e_ditBtn->setEnabled(curr!=-1);
  u_pBtn->setEnabled(curr>0);
  d_ownBtn->setEnabled((curr!=-1)&&(curr+1!=(int)(l_box->count())));
}



void KNConfig::DisplayedHeadersWidget::slotAddBtnClicked()
{
  KNDisplayedHeader *h=d_ata->createNewHeader();

  DisplayedHeaderConfDialog* dlg=new DisplayedHeaderConfDialog(h, this);
  if(dlg->exec()) {
    l_box->insertItem(generateItem(h));
    h->createTags();
    s_ave=true;
  } else
    d_ata->remove(h);
}



void KNConfig::DisplayedHeadersWidget::slotDelBtnClicked()
{
  if(l_box->currentItem()==-1)
    return;

  if(KMessageBox::questionYesNo(this, i18n("Really delete this header?"))==KMessageBox::Yes) {
    KNDisplayedHeader *h = (static_cast<HdrItem*>(l_box->item(l_box->currentItem())))->hdr;
    d_ata->remove(h);
    l_box->removeItem(l_box->currentItem());
    s_ave=true;
  }
}



void KNConfig::DisplayedHeadersWidget::slotEditBtnClicked()
{
  if (l_box->currentItem()==-1) return;
  KNDisplayedHeader *h = (static_cast<HdrItem*>(l_box->item(l_box->currentItem())))->hdr;

  DisplayedHeaderConfDialog* dlg=new DisplayedHeaderConfDialog(h, this);
  if(dlg->exec()) {
    l_box->changeItem(generateItem(h), l_box->currentItem());
    h->createTags();
    s_ave=true;
  }
}



void KNConfig::DisplayedHeadersWidget::slotUpBtnClicked()
{
  int c=l_box->currentItem();
  if(c==0 || c==-1) return;

  KNDisplayedHeader *h = (static_cast<HdrItem*>(l_box->item(c)))->hdr;

  d_ata->up(h);
  l_box->insertItem(generateItem(h), c-1);
  l_box->removeItem(c+1);
  l_box->setCurrentItem(c-1);
  s_ave=true;
}



void KNConfig::DisplayedHeadersWidget::slotDownBtnClicked()
{
  int c=l_box->currentItem();
  if(c==-1 || c==(int) l_box->count()-1) return;

  KNDisplayedHeader *h = (static_cast<HdrItem*>(l_box->item(c)))->hdr;

  d_ata->down(h);
  l_box->insertItem(generateItem(h), c+2);
  l_box->removeItem(c);
  l_box->setCurrentItem(c+1);
  s_ave=true;
}


//=============================================================================================


KNConfig::DisplayedHeaderConfDialog::DisplayedHeaderConfDialog(KNDisplayedHeader *h, QWidget *p, char *n)
  : KDialogBase(Plain, i18n("Header Properties"),Ok|Cancel|Help, Ok, p, n),
    h_dr(h)
{
  QFrame* page=plainPage();
  QGridLayout *topL=new QGridLayout(page, 2, 2, 0, 5);

  QWidget *nameW = new QWidget(page);
  QGridLayout *nameL=new QGridLayout(nameW, 2, 2, 5);

  h_drC=new QComboBox(true, nameW);
  h_drC->lineEdit()->setMaxLength(64);
  connect(h_drC, SIGNAL(activated(int)), this, SLOT(slotActivated(int)));
  nameL->addWidget(new QLabel(h_drC, i18n("H&eader:"),nameW),0,0);
  nameL->addWidget(h_drC,0,1);

  n_ameE=new QLineEdit(nameW);
  connect(n_ameE, SIGNAL(textChanged(const QString&)), SLOT(slotNameChanged(const QString&)));
  n_ameE->setMaxLength(64);
  nameL->addWidget(new QLabel(n_ameE, i18n("Displayed Na&me:"),nameW),1,0);
  nameL->addWidget(n_ameE,1,1);
  nameL->setColStretch(1,1);

  topL->addMultiCellWidget(nameW,0,0,0,1);

  QGroupBox *ngb=new QGroupBox(i18n("Name"), page);
  QVBoxLayout *ngbL = new QVBoxLayout(ngb, 8, 5);
  ngbL->setAutoAdd(true);
  ngbL->addSpacing(fontMetrics().lineSpacing()-4);
  n_ameCB[0]=new QCheckBox(i18n("&large"), ngb);
  n_ameCB[1]=new QCheckBox(i18n("&bold"), ngb);
  n_ameCB[2]=new QCheckBox(i18n("&italic"), ngb);
  n_ameCB[3]=new QCheckBox(i18n("&underlined"), ngb);
  topL->addWidget(ngb,1,0);

  QGroupBox *vgb=new QGroupBox(i18n("Value"), page);
  QVBoxLayout *vgbL = new QVBoxLayout(vgb, 8, 5);
  vgbL->setAutoAdd(true);
  vgbL->addSpacing(fontMetrics().lineSpacing()-4);
  v_alueCB[0]=new QCheckBox(i18n("l&arge"), vgb);
  v_alueCB[1]=new QCheckBox(i18n("bol&d"), vgb);
  v_alueCB[2]=new QCheckBox(i18n("i&talic"), vgb);
  v_alueCB[3]=new QCheckBox(i18n("u&nderlined"), vgb);
  topL->addWidget(vgb,1,1);

  topL->setColStretch(0,1);
  topL->setColStretch(1,1);

  // preset values...
  h_drC->insertStrList(KNDisplayedHeader::predefs());
  h_drC->lineEdit()->setText(h->header());
  n_ameE->setText(h->translatedName());
  for(int i=0; i<4; i++) {
    n_ameCB[i]->setChecked(h->flag(i));
    v_alueCB[i]->setChecked(h->flag(i+4));
  }

  setFixedHeight(sizeHint().height());
  restoreWindowSize("accReadHdrPropDLG", this, sizeHint());

  setHelp("anc-knode-headers");
}


KNConfig::DisplayedHeaderConfDialog::~DisplayedHeaderConfDialog()
{
  saveWindowSize("accReadHdrPropDLG", size());
}


void KNConfig::DisplayedHeaderConfDialog::slotOk()
{
  h_dr->setHeader(h_drC->currentText());
  h_dr->setTranslatedName(n_ameE->text());
  for(int i=0; i<4; i++) {
    if(h_dr->hasName())
      h_dr->setFlag(i, n_ameCB[i]->isChecked());
    else
      h_dr->setFlag(i,false);
    h_dr->setFlag(i+4, v_alueCB[i]->isChecked());
  }
  accept();
}


// the user selected one of the presets, insert the *translated* string as display name:
void KNConfig::DisplayedHeaderConfDialog::slotActivated(int pos)
{
  n_ameE->setText(i18n(h_drC->text(pos).local8Bit()));  // I think it's save here, the combobox has only english defaults
}


// disable the name format options when the name is empty
void KNConfig::DisplayedHeaderConfDialog::slotNameChanged(const QString& str)
{
  for(int i=0; i<4; i++)
    n_ameCB[i]->setEnabled(!str.isEmpty());
}


//=============================================================================================


KNConfig::FilterListWidget::FilterListWidget(QWidget *p, const char *n)
 : BaseWidget(p,n), f_ilManager(knGlobals.filManager)
{
  QGridLayout *topL=new QGridLayout(this, 6,2, 5,5);

  // == Filters =================================================

  f_lb=new KNDialogListBox(false, this);
  topL->addWidget(new QLabel(f_lb, i18n("&Filters:"),this),0,0);

  connect(f_lb, SIGNAL(selectionChanged()), SLOT(slotSelectionChangedFilter()));
  connect(f_lb, SIGNAL(selected(int)), SLOT(slotItemSelectedFilter(int)));
  topL->addMultiCellWidget(f_lb,1,5,0,0);

  a_ddBtn=new QPushButton(i18n("&New"), this);
  connect(a_ddBtn, SIGNAL(clicked()), this, SLOT(slotAddBtnClicked()));
  topL->addWidget(a_ddBtn,1,1);

  d_elBtn=new QPushButton(i18n("&Delete"), this);
  connect(d_elBtn, SIGNAL(clicked()), this, SLOT(slotDelBtnClicked()));
  topL->addWidget(d_elBtn,2,1);

  e_ditBtn=new QPushButton(i18n("modify something","&Edit"), this);
  connect(e_ditBtn, SIGNAL(clicked()), this, SLOT(slotEditBtnClicked()));
  topL->addWidget(e_ditBtn,3,1);

  c_opyBtn=new QPushButton(i18n("Co&py"), this);
  connect(c_opyBtn, SIGNAL(clicked()), this, SLOT(slotCopyBtnClicked()));
  topL->addWidget(c_opyBtn,4,1);

  // == Menu ====================================================

  m_lb=new KNDialogListBox(false, this);
  topL->addWidget(new QLabel(m_lb, i18n("&Menu:"),this),6,0);

  connect(m_lb, SIGNAL(selectionChanged()), SLOT(slotSelectionChangedMenu()));
  topL->addMultiCellWidget(m_lb,7,11,0,0);

  u_pBtn=new QPushButton(i18n("&Up"), this);
  connect(u_pBtn, SIGNAL(clicked()), this, SLOT(slotUpBtnClicked()));
  topL->addWidget(u_pBtn,7,1);

  d_ownBtn=new QPushButton(i18n("Do&wn"), this);
  connect(d_ownBtn, SIGNAL(clicked()), this, SLOT(slotDownBtnClicked()));
  topL->addWidget(d_ownBtn,8,1);

  s_epAddBtn=new QPushButton(i18n("Add\n&Separator"), this);
  connect(s_epAddBtn, SIGNAL(clicked()), this, SLOT(slotSepAddBtnClicked()));
  topL->addWidget(s_epAddBtn,9,1);

  s_epRemBtn=new QPushButton(i18n("&Remove\nSeparator"), this);
  connect(s_epRemBtn, SIGNAL(clicked()), this, SLOT(slotSepRemBtnClicked()));
  topL->addWidget(s_epRemBtn,10,1);

  topL->setRowStretch(5,1);
  topL->setRowStretch(11,1);

  a_ctive = SmallIcon("filter",16);
  d_isabled = SmallIcon("filter",16,KIcon::DisabledState);

  f_ilManager->startConfig(this);

  slotSelectionChangedFilter();
  slotSelectionChangedMenu();
}


KNConfig::FilterListWidget::~FilterListWidget()
{
  f_ilManager->endConfig();
}



void KNConfig::FilterListWidget::apply()
{
  if(d_irty)
    f_ilManager->commitChanges();
}


void KNConfig::FilterListWidget::addItem(KNArticleFilter *f)
{
  if(f->isEnabled())
    f_lb->insertItem(new LBoxItem(f, f->translatedName(), &a_ctive));
  else
    f_lb->insertItem(new LBoxItem(f, f->translatedName(), &d_isabled));
  slotSelectionChangedFilter();
}


void KNConfig::FilterListWidget::removeItem(KNArticleFilter *f)
{
  int i=findItem(f_lb, f);
  if (i!=-1) f_lb->removeItem(i);
  slotSelectionChangedFilter();
}


void KNConfig::FilterListWidget::updateItem(KNArticleFilter *f)
{
  int i=findItem(f_lb, f);

  if(i!=-1) {
    if(f->isEnabled()) {
      f_lb->changeItem(new LBoxItem(f, f->translatedName(), &a_ctive), i);
      m_lb->changeItem(new LBoxItem(f, f->translatedName()), findItem(m_lb, f));
    } else
      f_lb->changeItem(new LBoxItem(f, f->translatedName(), &d_isabled), i);
  }
  slotSelectionChangedFilter();
}


void KNConfig::FilterListWidget::addMenuItem(KNArticleFilter *f)
{
  if (f) {
    if (findItem(m_lb, f)==-1)
      m_lb->insertItem(new LBoxItem(f, f->translatedName()));
  } else   // separator
    m_lb->insertItem(new LBoxItem(0, "==="));
  slotSelectionChangedMenu();
}


void KNConfig::FilterListWidget::removeMenuItem(KNArticleFilter *f)
{
  int i=findItem(m_lb, f);
  if(i!=-1) m_lb->removeItem(i);
  slotSelectionChangedMenu();
}


QValueList<int> KNConfig::FilterListWidget::menuOrder()
{
  KNArticleFilter *f;
  QValueList<int> lst;

  for(uint i=0; i<m_lb->count(); i++) {
    f= (static_cast<LBoxItem*>(m_lb->item(i)))->filter;
    if(f)
      lst << f->id();
    else
      lst << -1;
  }
 return lst;
}


int KNConfig::FilterListWidget::findItem(QListBox *l, KNArticleFilter *f)
{
  int idx=0;
  bool found=false;
  while(!found && idx < (int) l->count()) {
    found=( (static_cast<LBoxItem*>(l->item(idx)))->filter==f );
    if(!found) idx++;
  }
  if(found) return idx;
  else return -1;
}


void KNConfig::FilterListWidget::slotAddBtnClicked()
{
  f_ilManager->newFilter();
}


void KNConfig::FilterListWidget::slotDelBtnClicked()
{
  if (f_lb->currentItem()!=-1)
    f_ilManager->deleteFilter( (static_cast<LBoxItem*>(f_lb->item(f_lb->currentItem())))->filter );
}


void KNConfig::FilterListWidget::slotEditBtnClicked()
{
  if (f_lb->currentItem()!=-1)
    f_ilManager->editFilter( (static_cast<LBoxItem*>(f_lb->item(f_lb->currentItem())))->filter );
}


void KNConfig::FilterListWidget::slotCopyBtnClicked()
{
  if (f_lb->currentItem()!=-1)
    f_ilManager->copyFilter( (static_cast<LBoxItem*>(f_lb->item(f_lb->currentItem())))->filter );
}


void KNConfig::FilterListWidget::slotUpBtnClicked()
{
  int c=m_lb->currentItem();
  KNArticleFilter *f=0;

  if(c==0 || c==-1) return;
  f=(static_cast<LBoxItem*>(m_lb->item(c)))->filter;
  if(f)
    m_lb->insertItem(new LBoxItem(f, f->translatedName()), c-1);
  else
    m_lb->insertItem(new LBoxItem(0, "==="), c-1);
  m_lb->removeItem(c+1);
  m_lb->setCurrentItem(c-1);
}


void KNConfig::FilterListWidget::slotDownBtnClicked()
{
  int c=m_lb->currentItem();
  KNArticleFilter *f=0;

  if(c==-1 || c+1==(int)m_lb->count()) return;
  f=(static_cast<LBoxItem*>(m_lb->item(c)))->filter;
  if(f)
    m_lb->insertItem(new LBoxItem(f, f->translatedName()), c+2);
  else
    m_lb->insertItem(new LBoxItem(0, "==="), c+2);
  m_lb->removeItem(c);
  m_lb->setCurrentItem(c+1);
}


void KNConfig::FilterListWidget::slotSepAddBtnClicked()
{
  m_lb->insertItem(new LBoxItem(0, "==="), m_lb->currentItem());
  slotSelectionChangedMenu();
}


void KNConfig::FilterListWidget::slotSepRemBtnClicked()
{
  int c=m_lb->currentItem();

  if( (c!=-1) && ( (static_cast<LBoxItem*>(m_lb->item(c)))->filter==0 ) )
     m_lb->removeItem(c);
  slotSelectionChangedMenu();
}


void KNConfig::FilterListWidget::slotItemSelectedFilter(int)
{
  slotEditBtnClicked();
}


void KNConfig::FilterListWidget::slotSelectionChangedFilter()
{
  int curr = f_lb->currentItem();

  d_elBtn->setEnabled(curr!=-1);
  e_ditBtn->setEnabled(curr!=-1);
  c_opyBtn->setEnabled(curr!=-1);
}


void KNConfig::FilterListWidget::slotSelectionChangedMenu()
{
  int curr = m_lb->currentItem();

  u_pBtn->setEnabled(curr>0);
  d_ownBtn->setEnabled((curr!=-1)&&(curr+1!=(int)m_lb->count()));
  s_epRemBtn->setEnabled((curr!=-1) && ( (static_cast<LBoxItem*>(m_lb->item(curr)))->filter==0 ) );
}


//=============================================================================================


KNConfig::PostNewsTechnicalWidget::PostNewsTechnicalWidget(PostNewsTechnical *d, QWidget *p, const char *n)
  : BaseWidget(p, n), d_ata(d)
{
  QVBoxLayout *topL=new QVBoxLayout(this, 5);

  // ==== General =============================================================

  QGroupBox *ggb=new QGroupBox(i18n("General"), this);
  QGridLayout *ggbL=new QGridLayout(ggb, 7,2, 8,5);
  topL->addWidget(ggb);

  ggbL->addRowSpacing(0, fontMetrics().lineSpacing()-4);
  c_harset=new QComboBox(ggb);
  c_harset->insertStringList(d->composerCharsets());
  ggbL->addWidget(new QLabel(c_harset, i18n("Cha&rset"), ggb), 1,0);
  ggbL->addWidget(c_harset, 1,1);

  e_ncoding=new QComboBox(ggb);
  e_ncoding->insertItem("Allow 8-bit");
  e_ncoding->insertItem("7-bit (Quoted-Printable)");
  ggbL->addWidget(new QLabel(e_ncoding, i18n("Enco&ding"), ggb), 2,0);
  ggbL->addWidget(e_ncoding, 2,1);

  u_seOwnCSCB=new QCheckBox(i18n("Use o&wn default charset when replying"), ggb);
  ggbL->addMultiCellWidget(u_seOwnCSCB, 3,3, 0,1);

  a_llow8bitCB=new QCheckBox(i18n("Don't encode &8-bit characters in the header"), ggb);
  connect(a_llow8bitCB, SIGNAL(toggled(bool)), this, SLOT(slotHeadEncToggled(bool)));
  ggbL->addMultiCellWidget(a_llow8bitCB, 4,4, 0,1);

  g_enMIdCB=new QCheckBox(i18n("&Generate Message-Id"), ggb);
  connect(g_enMIdCB, SIGNAL(toggled(bool)), this, SLOT(slotGenMIdCBToggled(bool)));
  ggbL->addMultiCellWidget(g_enMIdCB, 5,5, 0,1);
  h_ost=new QLineEdit(ggb);
  h_ost->setEnabled(false);
  h_ostL=new QLabel(h_ost, i18n("Ho&stname:"), ggb);
  h_ostL->setEnabled(false);
  ggbL->addWidget(h_ostL, 6,0);
  ggbL->addWidget(h_ost, 6,1);
  ggbL->setColStretch(1,1);

  // ==== X-Headers =============================================================

  QGroupBox *xgb=new QGroupBox(i18n("X-Headers"), this);
  topL->addWidget(xgb, 1);
  QGridLayout *xgbL=new QGridLayout(xgb, 6,2, 8,5);

  xgbL->addRowSpacing(0, fontMetrics().lineSpacing()-4);

  l_box=new KNDialogListBox(false, xgb);
  connect(l_box, SIGNAL(selected(int)), SLOT(slotItemSelected(int)));
  connect(l_box, SIGNAL(selectionChanged()), SLOT(slotSelectionChanged()));
  xgbL->addMultiCellWidget(l_box, 1,4, 0,0);

  a_ddBtn=new QPushButton(i18n("&New"), xgb);
  connect(a_ddBtn, SIGNAL(clicked()), SLOT(slotAddBtnClicked()));
  xgbL->addWidget(a_ddBtn, 1,1);

  d_elBtn=new QPushButton(i18n("Dele&te"), xgb);
  connect(d_elBtn, SIGNAL(clicked()), SLOT(slotDelBtnClicked()));
  xgbL->addWidget(d_elBtn, 2,1);

  e_ditBtn=new QPushButton(i18n("modify something","&Edit"), xgb);
  connect(e_ditBtn, SIGNAL(clicked()), SLOT(slotEditBtnClicked()));
  xgbL->addWidget(e_ditBtn, 3,1);

  i_ncUaCB=new QCheckBox(i18n("Don't add the \"&User-Agent\" identification header"), xgb);
  xgbL->addMultiCellWidget(i_ncUaCB, 5,5, 0,1);

  xgbL->setRowStretch(4,1);
  xgbL->setColStretch(0,1);

  //init
  c_harset->setCurrentItem(d->indexForCharset(d->charset()));
  e_ncoding->setCurrentItem(d->a_llow8BitBody? 0:1);
  u_seOwnCSCB->setChecked(d->u_seOwnCharset);
  a_llow8bitCB->blockSignals(true); //avoid warning message on startup
  a_llow8bitCB->setChecked(d->a_llow8BitHeaders);
  a_llow8bitCB->blockSignals(false);
  g_enMIdCB->setChecked(d->g_enerateMID);
  h_ost->setText(d->h_ostname);
  i_ncUaCB->setChecked(d->d_ontIncludeUA);

  for(XHeaders::Iterator it=d->x_headers.begin(); it!=d->x_headers.end(); ++it)
    l_box->insertItem((*it).header());
  slotSelectionChanged();
}


KNConfig::PostNewsTechnicalWidget::~PostNewsTechnicalWidget()
{
}


void KNConfig::PostNewsTechnicalWidget::apply()
{
  if(!d_irty)
    return;

  d_ata->c_harset=c_harset->currentText().latin1();
  d_ata->a_llow8BitBody=(e_ncoding->currentItem()==0);
  d_ata->u_seOwnCharset=u_seOwnCSCB->isChecked();
  d_ata->a_llow8BitHeaders=a_llow8bitCB->isChecked();
  d_ata->g_enerateMID=g_enMIdCB->isChecked();
  d_ata->h_ostname=h_ost->text().latin1();
  d_ata->d_ontIncludeUA=i_ncUaCB->isChecked();
  d_ata->x_headers.clear();
  for(unsigned int idx=0; idx<l_box->count(); idx++)
    d_ata->x_headers.append( XHeader(l_box->text(idx)) );

  d_ata->save();
}



void KNConfig::PostNewsTechnicalWidget::slotHeadEncToggled(bool b)
{
  if (b)
    KMessageBox::information(this,i18n("Please be aware that unencoded 8-bit characters\nare illegal in the header of a usenet message.\nUse this feature with extreme care!"));
}



void KNConfig::PostNewsTechnicalWidget::slotGenMIdCBToggled(bool b)
{
  h_ost->setEnabled(b);
  h_ostL->setEnabled(b);
}



void KNConfig::PostNewsTechnicalWidget::slotSelectionChanged()
{
  d_elBtn->setEnabled(l_box->currentItem()!=-1);
  e_ditBtn->setEnabled(l_box->currentItem()!=-1);
}



void KNConfig::PostNewsTechnicalWidget::slotItemSelected(int)
{
  slotEditBtnClicked();
}



void KNConfig::PostNewsTechnicalWidget::slotAddBtnClicked()
{
  XHeaderConfDialog *dlg=new XHeaderConfDialog(QString::null, this);
  if (dlg->exec())
    l_box->insertItem(dlg->result());

  delete dlg;

  slotSelectionChanged();
}



void KNConfig::PostNewsTechnicalWidget::slotDelBtnClicked()
{
  int c=l_box->currentItem();
  if (c == -1)
    return;

  l_box->removeItem(c);
  slotSelectionChanged();
}



void KNConfig::PostNewsTechnicalWidget::slotEditBtnClicked()
{
  int c=l_box->currentItem();
  if (c == -1)
    return;

  XHeaderConfDialog *dlg=new XHeaderConfDialog(l_box->text(c), this);
  if (dlg->exec())
    l_box->changeItem(dlg->result(),c);

  delete dlg;

  slotSelectionChanged();
}


//===================================================================================================


KNConfig::XHeaderConfDialog::XHeaderConfDialog(const QString &h, QWidget *p, const char *n)
  : KDialogBase(Plain, i18n("X-Headers"),Ok|Cancel, Ok, p, n)
{
  QFrame* page=plainPage();
  QHBoxLayout *topL=new QHBoxLayout(page, 5,8);
  topL->setAutoAdd(true);

  new QLabel("X-", page);
  n_ame=new QLineEdit(page);
  new QLabel(":", page);
  v_alue=new QLineEdit(page);

  int pos=h.find(": ", 2);
  if(pos!=-1) {
    n_ame->setText(h.mid(2, pos-2));
    pos+=2;
    v_alue->setText(h.mid(pos, h.length()-pos));
  }

  setFixedHeight(sizeHint().height());
  restoreWindowSize("XHeaderDlg", this, sizeHint());

  n_ame->setFocus();
}



KNConfig::XHeaderConfDialog::~XHeaderConfDialog()
{
  saveWindowSize("XHeaderDlg", size());
}



QString KNConfig::XHeaderConfDialog::result()
{
  return QString("X-%1: %2").arg(n_ame->text()).arg(v_alue->text());
}


//===================================================================================================


KNConfig::PostNewsComposerWidget::PostNewsComposerWidget(PostNewsComposer *d, QWidget *p, const char *n)
  : BaseWidget(p, n), d_ata(d)
{
  QVBoxLayout *topL=new QVBoxLayout(this, 5);

  // === general ===========================================================

  QGroupBox *generalB=new QGroupBox(i18n("General"), this);
  topL->addWidget(generalB);
  QGridLayout *generalL=new QGridLayout(generalB, 3,3, 8,5);

  generalL->addRowSpacing(0, fontMetrics().lineSpacing()-4);

  w_ordWrapCB=new QCheckBox(i18n("Word &wrap at column:"), generalB);
  generalL->addWidget(w_ordWrapCB,1,0);
  m_axLen=new KIntSpinBox(20, 200, 1, 20, 10, generalB);
  generalL->addWidget(m_axLen,1,2);
  connect(w_ordWrapCB, SIGNAL(toggled(bool)), m_axLen, SLOT(setEnabled(bool)));

  o_wnSigCB=new QCheckBox(i18n("Appe&nd signature automatically"), generalB);
  generalL->addMultiCellWidget(o_wnSigCB,2,2,0,1);

  generalL->setColStretch(1,1);

  // === reply =============================================================

  QGroupBox *replyB=new QGroupBox(i18n("Reply"), this);
  topL->addWidget(replyB);
  QGridLayout *replyL=new QGridLayout(replyB, 6,2, 8,5);

  replyL->addRowSpacing(0, fontMetrics().lineSpacing()-4);

  i_ntro=new QLineEdit(replyB);
  replyL->addMultiCellWidget(new QLabel(i_ntro,i18n("&Introduction Phrase:"), replyB),1,1,0,1);
  replyL->addMultiCellWidget(i_ntro, 2,2,0,1);
  replyL->addMultiCellWidget(new QLabel(i18n("Placeholders: %NAME=name, %EMAIL=email address,\n%DATE=date, %MSID=msgid"), replyB),3,3,0,1);

  r_ewrapCB=new QCheckBox(i18n("Rewrap quoted te&xt automatically"), replyB);
  replyL->addMultiCellWidget(r_ewrapCB, 5,5,0,1);

  a_uthSigCB=new QCheckBox(i18n("Include the a&uthors signature"), replyB);
  replyL->addMultiCellWidget(a_uthSigCB, 6,6,0,1);

  replyL->setColStretch(1,1);

  // === external editor ========================================================

  QGroupBox *editorB=new QGroupBox(i18n("External Editor"), this);
  topL->addWidget(editorB);
  QGridLayout *editorL=new QGridLayout(editorB, 6,3, 8,5);

  editorL->addRowSpacing(0, fontMetrics().lineSpacing()-4);

  e_ditor=new QLineEdit(editorB);
  editorL->addWidget(new QLabel(e_ditor, i18n("Specify Edi&tor:"), editorB),1,0);
  editorL->addWidget(e_ditor,1,1);
  QPushButton *btn = new QPushButton(i18n("Choo&se..."),editorB);
  connect(btn, SIGNAL(clicked()), SLOT(slotChooseEditor()));
  editorL->addWidget(btn,1,2);

  editorL->addMultiCellWidget(new QLabel(i18n("%f will be replaced with the filename to edit."), editorB),2,2,0,2);

  e_xternCB=new QCheckBox(i18n("Start exte&rnal editor automatically"), editorB);
  editorL->addMultiCellWidget(e_xternCB, 3,3,0,2);

  editorL->setColStretch(1,1);

  topL->addStretch(1);

  //init
  w_ordWrapCB->setChecked(d->w_ordWrap);
  m_axLen->setEnabled(d->w_ordWrap);
  a_uthSigCB->setChecked(d->i_ncSig);
  e_xternCB->setChecked(d->u_seExtEditor);
  o_wnSigCB->setChecked(d->a_ppSig);
  r_ewrapCB->setChecked(d->r_ewrap);
  m_axLen->setValue(d->m_axLen);
  i_ntro->setText(d->i_ntro);
  e_ditor->setText(d->e_xternalEditor);
}


KNConfig::PostNewsComposerWidget::~PostNewsComposerWidget()
{
}


void KNConfig::PostNewsComposerWidget::apply()
{
  if(!d_irty)
    return;

  d_ata->w_ordWrap=w_ordWrapCB->isChecked();
  d_ata->m_axLen=m_axLen->value();
  d_ata->r_ewrap=r_ewrapCB->isChecked();
  d_ata->a_ppSig=o_wnSigCB->isChecked();
  d_ata->i_ntro=i_ntro->text();
  d_ata->i_ncSig=a_uthSigCB->isChecked();
  d_ata->e_xternalEditor=e_ditor->text();
  d_ata->u_seExtEditor=e_xternCB->isChecked();

  d_ata->save();
}


void KNConfig::PostNewsComposerWidget::slotChooseEditor()
{
  QString path=e_ditor->text().simplifyWhiteSpace();
  if (path.right(3) == " %f")
    path.truncate(path.length()-3);

  path=KFileDialog::getOpenFileName(path, QString::null, this, i18n("Choose Editor"));

  if (!path.isEmpty())
    e_ditor->setText(path+" %f");
}


//===================================================================================================


KNConfig::PostNewsSpellingWidget::PostNewsSpellingWidget(QWidget *p, const char *n)
  : BaseWidget(p, n)
{
  QVBoxLayout *topL=new QVBoxLayout(this, 5);

  c_onf = new KSpellConfig( this, "spell", 0, false );
  topL->addWidget(c_onf);

  topL->addStretch(1);
}


KNConfig::PostNewsSpellingWidget::~PostNewsSpellingWidget()
{
}


void KNConfig::PostNewsSpellingWidget::apply()
{
  if(d_irty)
     c_onf->writeGlobalSettings();
}


//==============================================================================================================

KNConfig::PrivacyWidget::PrivacyWidget(Privacy *pr, QWidget *p, const char *n) : BaseWidget(p,n), d_ata(pr)
{
//  QVBoxLayout *topL = new QVBoxLayout(this, 2);
  QGridLayout *topL=new QGridLayout(this, 3,3, 8,5);

//  don't use a groupbox as long as we have nothing to group in here (CG)

  // === groups ===========================================================

//  QGroupBox *groupsB=new QGroupBox(this);

//  QGridLayout *groupsL=new QGridLayout(groupsB, 2,3, 8,5);
//  groupsL->addRowSpacing(0, fontMetrics().lineSpacing()-4);

  pgp_version = new QComboBox(this);
  QLabel *l1 = new QLabel(pgp_version,i18n("&PGP version"), this);
  topL->addWidget(l1,0,0);
  topL->addMultiCellWidget(pgp_version,0,0,1,2);
  connect(pgp_version, SIGNAL(activated(int)), SLOT(slotDefaultProg(int)));

  pgp_path = new QLineEdit(this);
  l1 =new QLabel(pgp_path, i18n("Crypt progra&m:"), this);
  topL->addWidget(l1,1,0);
  topL->addWidget(pgp_path,1,1);

  QPushButton *choosePgp = new QPushButton(i18n("Choo&se..."),this);
  topL->addWidget(choosePgp,1,2);
  connect(choosePgp, SIGNAL(clicked()), SLOT(slotChoosePgp()));

  //l1 = new QLabel(i18n("alternate keyring:"), groupsB);
  //groupsL->addWidget(l1,3,0);
  //keyring = new QLineEdit(groupsB);
  //groupsL->addWidget(keyring,3,1);

  //l1 = new QLabel(i18n("keyserver:"), groupsB);
  //groupsL->addWidget(l1,4,0);
  //keyserv = new QLineEdit(groupsB);
  //groupsL->addWidget(keyserv,4,1);

  //l1 = new QLabel(i18n("PGP method"), groupsB);
  //groupsL->addWidget(l1,5,0);
  //enc_style = new QComboBox(groupsB);
  //groupsL->addWidget(enc_style,5,1);

  //keeppasswd = new QCheckBox("keep passphrase in memory",groupsB);
  //groupsL->addMultiCellWidget(keeppasswd, 6,6,0,1);

  topL->setColStretch(1,1);
  topL->setRowStretch(2,1);

//  topL->addWidget(groupsB);
//  topL->addStretch(1);

  // === init ===========================================================
  pgp_version->insertStringList(d_ata->v_ersions);
  pgp_version->setCurrentItem(d_ata->v_ersion);
  pgp_path->setText(d_ata->p_rogpath);
  //keyring->setText(d_ata->k_eyring);
  //keyserv->setText(d_ata->k_eyserv);
  //enc_style->insertStringList(d_ata->e_ncodings);
  //enc_style->setCurrentItem(d_ata->e_ncoding);
  //keeppasswd->setChecked(d_ata->k_eeppasswd);
  //kdDebug(5003) << k_funcinfo << endl;
}


void KNConfig::PrivacyWidget::slotChoosePgp()
{
  QString path = pgp_path->text().simplifyWhiteSpace();

  path = KFileDialog::getOpenFileName(path, QString::null, this, i18n("Choose PGP binary"));

  if (!path.isEmpty())
    pgp_path->setText(path);
}


void KNConfig::PrivacyWidget::slotDefaultProg(int version)
{
   pgp_path->setText(d_ata->defaultProg(version));
}


KNConfig::PrivacyWidget::~PrivacyWidget()
{
}

void KNConfig::PrivacyWidget::apply()
{
  if(!d_irty)
    return;

  d_ata->v_ersion = pgp_version->currentItem();
  //d_ata->e_ncoding = enc_style->currentItem();
  //d_ata->k_eeppasswd = keeppasswd->isChecked();
  //d_ata->k_eyring = keyring->text();
  d_ata->p_rogpath = pgp_path->text();

  d_ata->save();
}


//==============================================================================================================


KNConfig::CleanupWidget::CleanupWidget(Cleanup *d, QWidget *p, const char *n) : BaseWidget(p, n), d_ata(d)
{
  QVBoxLayout *topL=new QVBoxLayout(this, 5);

  // === groups ===========================================================

  QGroupBox *groupsB=new QGroupBox(i18n("Groups"), this);
  topL->addWidget(groupsB);
  QGridLayout *groupsL=new QGridLayout(groupsB, 6,2, 8,5);

  groupsL->addRowSpacing(0, fontMetrics().lineSpacing()-4);

  g_roupCB=new QCheckBox(i18n("&Remove old articles from newsgroups"), groupsB);
  connect(g_roupCB, SIGNAL(toggled(bool)), this, SLOT(slotGroupCBtoggled(bool)));
  groupsL->addMultiCellWidget(g_roupCB,1,1,0,1);

  g_roupDays=new KIntSpinBox(0, 999, 1, 0, 10, groupsB);
  g_roupDays->setSuffix(i18n(" days"));
  g_roupDaysL=new QLabel(g_roupDays,i18n("&Purge groups every"), groupsB);
  groupsL->addWidget(g_roupDaysL,2,0);
  groupsL->addWidget(g_roupDays,2,1,Qt::AlignRight);

  r_eadDays=new KIntSpinBox(0, 999, 1, 0, 10, groupsB);
  r_eadDays->setSuffix(i18n(" days"));
  r_eadDaysL=new QLabel(r_eadDays, i18n("&Keep read articles"), groupsB);
  groupsL->addWidget(r_eadDaysL,3,0);
  groupsL->addWidget(r_eadDays,3,1,Qt::AlignRight);

  u_nreadDays=new KIntSpinBox(0, 999, 1, 0, 10, groupsB);
  u_nreadDays->setSuffix(i18n(" days"));
  u_nreadDaysL=new QLabel(u_nreadDays, i18n("Keep u&nread articles"), groupsB);
  groupsL->addWidget(u_nreadDaysL,4,0);
  groupsL->addWidget(u_nreadDays,4,1,Qt::AlignRight);

  t_hrCB=new QCheckBox(i18n("Preser&ve threads"), groupsB);
  groupsL->addWidget(t_hrCB,5,0);

  groupsL->setColStretch(1,1);

  // === folders =========================================================

  QGroupBox *foldersB=new QGroupBox(i18n("Folders"), this);
  topL->addWidget(foldersB);
  QGridLayout *foldersL=new QGridLayout(foldersB, 3,2, 8,5);

  foldersL->addRowSpacing(0, fontMetrics().lineSpacing()-4);

  f_olderCB=new QCheckBox(i18n("Co&mpact folders"), foldersB);
  connect(f_olderCB, SIGNAL(toggled(bool)), this, SLOT(slotFolderCBtoggled(bool)));
  foldersL->addMultiCellWidget(f_olderCB,1,1,0,1);

  f_olderDays=new KIntSpinBox(0, 999, 1, 0, 10, foldersB);
  f_olderDays->setSuffix(i18n(" days"));
  f_olderDaysL=new QLabel(f_olderDays,i18n("P&urge folders every"), foldersB);
  foldersL->addWidget(f_olderDaysL,2,0);
  foldersL->addWidget(f_olderDays,2,1,Qt::AlignRight);

  foldersL->setColStretch(1,1);

  topL->addStretch(1);

  //init
  f_olderCB->setChecked(d->d_oCompact);
  slotFolderCBtoggled(d->d_oCompact);
  g_roupCB->setChecked(d->d_oExpire);
  slotGroupCBtoggled(d->d_oExpire);
  t_hrCB->setChecked(d->p_reserveThr);
  f_olderDays->setValue(d->c_ompactInterval);
  g_roupDays->setValue(d->e_xpireInterval);
  r_eadDays->setValue(d->r_eadMaxAge);
  u_nreadDays->setValue(d->u_nreadMaxAge);

}


KNConfig::CleanupWidget::~CleanupWidget()
{
}


void KNConfig::CleanupWidget::apply()
{
  if(!d_irty)
    return;

  d_ata->d_oExpire=g_roupCB->isChecked();
  d_ata->e_xpireInterval=g_roupDays->value();
  d_ata->u_nreadMaxAge=u_nreadDays->value();
  d_ata->r_eadMaxAge=r_eadDays->value();
  d_ata->p_reserveThr=t_hrCB->isChecked();
  d_ata->d_oCompact=f_olderCB->isChecked();
  d_ata->c_ompactInterval=f_olderDays->value();

  d_ata->save();
}


void KNConfig::CleanupWidget::slotGroupCBtoggled(bool b)
{
  g_roupDaysL->setEnabled(b);
  g_roupDays->setEnabled(b);
  r_eadDaysL->setEnabled(b);
  r_eadDays->setEnabled(b);
  u_nreadDaysL->setEnabled(b);
  u_nreadDays->setEnabled(b);
  t_hrCB->setEnabled(b);
}


void KNConfig::CleanupWidget::slotFolderCBtoggled(bool b)
{
  f_olderDaysL->setEnabled(b);
  f_olderDays->setEnabled(b);
}


//------------------------
#include "knconfig.moc"
