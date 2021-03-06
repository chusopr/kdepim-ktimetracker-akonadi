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

#include "addtagdialog.h"
#include "mailcommon/tagwidget.h"

#include <KLocale>
#include <KLineEdit>

#include <Nepomuk2/Tag>

#include <QVBoxLayout>

AddTagDialog::AddTagDialog(QWidget *parent)
  : KDialog(parent)
{
  setModal( true );
  setCaption( i18n( "Add Tag" ) );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );
  showButtonSeparator( true );
  QVBoxLayout *lay = new QVBoxLayout( mainWidget() );
  mTagWidget = new MailCommon::TagWidget(QList<KActionCollection*>(),this);
  lay->addWidget(mTagWidget);
  connect( this, SIGNAL(okClicked()), SLOT(slotOk()) );
}

AddTagDialog::~AddTagDialog()
{
}

void AddTagDialog::slotOk()
{
  const QString name(mTagWidget->tagNameLineEdit()->text());
  Nepomuk2::Tag nepomukTag( name );
  nepomukTag.setLabel( name );

  MailCommon::Tag::Ptr tag = MailCommon::Tag::fromNepomuk( nepomukTag );
  mTagWidget->recordTagSettings(tag);
  MailCommon::Tag::SaveFlags saveFlags = mTagWidget->saveFlags();
  tag->saveToNepomuk( saveFlags );

  mLabel = name;
  mNepomukUrl = tag->nepomukResourceUri.toString();

  accept();
}

QString AddTagDialog::label() const
{
  return mLabel;
}

QString AddTagDialog::nepomukUrl() const
{
  return mNepomukUrl;
}

#include "addtagdialog.moc"
