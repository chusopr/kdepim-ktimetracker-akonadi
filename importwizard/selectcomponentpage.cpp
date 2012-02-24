/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "selectcomponentpage.h"
#include "ui_selectcomponentpage.h"

SelectComponentPage::SelectComponentPage(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::SelectComponentPage)
{
  ui->setupUi(this);
  connect( ui->everything, SIGNAL( clicked ( bool ) ), this, SLOT( slotEverythingClicked( bool ) ) );
}

SelectComponentPage::~SelectComponentPage()
{
  delete ui;
}

void SelectComponentPage::slotEverythingClicked( bool clicked )
{
  ui->addressbooks->setEnabled( !clicked );
  ui->filters->setEnabled( !clicked );
  ui->mails->setEnabled( !clicked );
  ui->settings->setEnabled( !clicked );
}

#include "selectcomponentpage.moc"
