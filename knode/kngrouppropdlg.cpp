/*
    kngrouppropdlg.cpp

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

#include <qgroupbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qvbox.h>

#include <klocale.h>

#include "utilities.h"
#include "kngroup.h"
#include "kngrouppropdlg.h"
#include "knconfig.h"
#include "knmime.h"


KNGroupPropDlg::KNGroupPropDlg(KNGroup *group, QWidget *parent, const char *name )
  : KDialogBase(Tabbed, i18n("Properties of %1").arg(group->groupname()),
                Ok|Cancel|Help, Ok, parent, name),
    g_rp(group), n_ickChanged(false)
{

  // General tab ===============================================

  QWidget *page = addPage(i18n("&General"));
  QGridLayout *pageL=new QGridLayout(page,  4, 2, 5,5);

  // nickname
  n_ick=new QLineEdit(page);
  if (g_rp->hasName())
    n_ick->setText(g_rp->name());
  QLabel *l=new QLabel(n_ick, i18n("&Nickname:"), page);
  pageL->addWidget(l,0,0);
  pageL->addWidget(n_ick,0,1);

  // group name & description
  QGroupBox *gb=new QGroupBox(i18n("Description"), page);
  pageL->addMultiCellWidget(gb,1,1,0,1);
  QGridLayout *grpL=new QGridLayout(gb, 4, 3, 15, 5);

  grpL->addRowSpacing(0, fontMetrics().lineSpacing()-9);

  l=new QLabel(i18n("Name:"), gb);
  grpL->addWidget(l,1,0);
  l=new QLabel(group->groupname(),gb);
  grpL->addWidget(l,1,2);

  l=new QLabel(i18n("Description:"), gb);
  grpL->addWidget(l,2,0);
  l=new QLabel(g_rp->description(),gb);
  grpL->addWidget(l,2,2);

  l=new QLabel(i18n("Status:"), gb);
  grpL->addWidget(l,3,0);
  QString status;
  switch (g_rp->status()) {
    case KNGroup::unknown:  status=i18n("unknown");
                            break;
    case KNGroup::readOnly: status=i18n("posting forbidden");
                            break;
    case KNGroup::postingAllowed:  status=i18n("posting allowed");
                                   break;
    case KNGroup::moderated:       status=i18n("moderated");
                                   break;
  }
  l=new QLabel(status,gb);
  grpL->addWidget(l,3,2);

  grpL->addColSpacing(1,20);
  grpL->setColStretch(2,1);

  // statistics
  gb=new QGroupBox(i18n("Statistics"), page);
  pageL->addMultiCellWidget(gb,2,2,0,1);
  grpL=new QGridLayout(gb, 6, 3, 15, 5);

  grpL->addRowSpacing(0, fontMetrics().lineSpacing()-9);

  l=new QLabel(i18n("Articles:"), gb);
  grpL->addWidget(l,1,0);
  l=new QLabel(QString::number(g_rp->count()),gb);
  grpL->addWidget(l,1,2);

  l=new QLabel(i18n("Unread articles:"), gb);
  grpL->addWidget(l,2,0);
  l=new QLabel(QString::number(g_rp->count()-g_rp->readCount()),gb);
  grpL->addWidget(l,2,2);

  l=new QLabel(i18n("New articles:"), gb);
  grpL->addWidget(l,3,0);
  l=new QLabel(QString::number(g_rp->newCount()),gb);
  grpL->addWidget(l,3,2);

  l=new QLabel(i18n("Threads with unread articles:"), gb);
  grpL->addWidget(l,4,0);
  l=new QLabel(QString::number(g_rp->statThrWithUnread()),gb);
  grpL->addWidget(l,4,2);

  l=new QLabel(i18n("Threads with new articles:"), gb);
  grpL->addWidget(l,5,0);
  l=new QLabel(QString::number(g_rp->statThrWithNew()),gb);
  grpL->addWidget(l,5,2);

  grpL->addColSpacing(1,20);
  grpL->setColStretch(2,1);
    
  pageL->setRowStretch(3,2);

  // Specfic Identity tab =========================================
  i_dWidget=new KNConfig::IdentityWidget(g_rp->identity(), addVBoxPage(i18n("&Identity")));


  // Specific Settings tab ========================================
  page=addPage(i18n("Se&ttings"));
  pageL=new QGridLayout(page, 2,2, 5,5);

  u_seCharset=new QCheckBox(i18n("&Use different default charset:"), page);
  u_seCharset->setChecked(g_rp->useCharset());
  pageL->addWidget(u_seCharset, 0,0);

  c_harset=new QComboBox(false, page);
  c_harset->insertStringList(KNMimeBase::availableCharsets());
  c_harset->setCurrentItem( KNMimeBase::indexForCharset(g_rp->defaultCharset()) );

  c_harset->setEnabled(g_rp->useCharset());
  connect(u_seCharset, SIGNAL(toggled(bool)), c_harset, SLOT(setEnabled(bool)));
  pageL->addWidget(c_harset, 0,1);

  pageL->setRowStretch(1,1);


  restoreWindowSize("groupPropDLG", this, sizeHint());
}



KNGroupPropDlg::~KNGroupPropDlg()
{
  saveWindowSize("groupPropDLG", size());
}



void KNGroupPropDlg::slotOk()
{
  if( !(g_rp->name()==n_ick->text()) ) {
    g_rp->setName(n_ick->text());
    n_ickChanged=true;
  }

  i_dWidget->apply();

  g_rp->setUseCharset(u_seCharset->isChecked());
  g_rp->setDefaultCharset(c_harset->currentText().latin1());

  accept();
}
