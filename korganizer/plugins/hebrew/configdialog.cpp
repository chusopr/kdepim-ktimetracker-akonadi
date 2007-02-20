/*
    This file is part of KOrganizer.
    Copyright (c) 2003 Jonathan Singer <jsinger@leeta.net>

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
*/
#include "configdialog.h"
#include "configdialog.moc"
#include <QFrame>
#include <klocale.h>
#include <QLayout>
#include <kapplication.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kstandarddirs.h>

ConfigDialog::ConfigDialog(QWidget * parent)
  :KDialog( parent)
{
  QFrame *topFrame = new QFrame(this);
  setMainWidget( topFrame );
  setCaption( i18n("Configure Holidays") );
  setButtons( Ok|Cancel );
  setDefaultButton( Ok );
  setModal( true );
  QVBoxLayout *topLayout = new QVBoxLayout(topFrame);
  topLayout->setMargin( 0 );
  topLayout->setSpacing( spacingHint() );

  israel_box = new QCheckBox(topFrame);
  israel_box->setText(i18n("Use Israeli holidays"));
  topLayout->addWidget(israel_box);

  parsha_box = new QCheckBox(topFrame);
  parsha_box->setText(i18n("Show weekly parsha"));
  topLayout->addWidget(parsha_box);

  omer_box = new QCheckBox(topFrame);
  omer_box->setText(i18n("Show day of Omer"));
  topLayout->addWidget(omer_box);

  chol_box = new QCheckBox(topFrame);
  chol_box->setText(i18n("Show Chol HaMoed"));
  topLayout->addWidget(chol_box);
  connect( this, SIGNAL( okClicked() ),this,SLOT( slotOk() ) );
  load();
}

ConfigDialog::~ConfigDialog()
{
}

void ConfigDialog::load()
{
  KConfig config("korganizerrc", KConfig::NoGlobals );

  config.setGroup("Calendar/Hebrew Calendar Plugin");
  israel_box->setChecked(config.
                         readEntry("Israel",
                                       (KGlobal::locale()->
                                        country() == QLatin1String(".il"))));
  parsha_box->setChecked(config.readEntry("Parsha", true));
  chol_box->setChecked(config.readEntry("Chol_HaMoed", true));
  omer_box->setChecked(config.readEntry("Omer", true));

}

void ConfigDialog::save()
{
  KConfig config("korganizerrc", KConfig::NoGlobals); // Open read-write, no kdeglobals

  config.setGroup("Calendar/Hebrew Calendar Plugin");
  config.writeEntry("Israel", israel_box->isChecked());
  config.writeEntry("Parsha", parsha_box->isChecked());
  config.writeEntry("Chol_HaMoed", chol_box->isChecked());
  config.writeEntry("Omer", omer_box->isChecked());
  config.sync();
}

void ConfigDialog::slotOk()
{
  save();

  accept();
}
