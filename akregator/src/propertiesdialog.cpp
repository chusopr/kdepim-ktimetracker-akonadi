/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "propertiesdialog.h"

#include <kcombobox.h>
#include <klineedit.h>
#include <kpassdlg.h>
#include <klocale.h>
#include <knuminput.h>

#include <qcheckbox.h>
#include <qbuttongroup.h>

using namespace Akregator;

FeedPropertiesWidget::FeedPropertiesWidget(QWidget *parent, const char *name)
        : FeedPropertiesWidgetBase(parent, name)
{
   connect(      upChkbox, SIGNAL(toggled(bool)),
            updateSpinBox, SLOT(setEnabled(bool)) );
}

FeedPropertiesWidget::~FeedPropertiesWidget()
{}

FeedPropertiesDialog::FeedPropertiesDialog(QWidget *parent, const char *name)
        : KDialogBase(KDialogBase::Swallow, Qt::WStyle_DialogBorder, parent, name, true, i18n("Feed Properties"), KDialogBase::Ok|KDialogBase::Cancel)
{
    widget=new FeedPropertiesWidget(this);
    setMainWidget(widget);
}

FeedPropertiesDialog::~FeedPropertiesDialog()
{}

const QString FeedPropertiesDialog::feedName() const
{
   return widget->feedNameEdit->text();
}

const QString FeedPropertiesDialog::url() const
{
   return widget->urlEdit->text();
}

bool FeedPropertiesDialog::autoFetch() const
{
   return widget->upChkbox->isChecked();
}

int FeedPropertiesDialog::fetchInterval() const
{
   return widget->updateSpinBox->value();
}

void FeedPropertiesDialog::setFeedName(const QString& title)
{
   widget->feedNameEdit->setText(title);
}

void FeedPropertiesDialog::setUrl(const QString& url)
{
   widget->urlEdit->setText(url);
}

void FeedPropertiesDialog::setAutoFetch(bool w)
{
   widget->upChkbox->setChecked(w);
   widget->updateSpinBox->setEnabled(w);
}

void FeedPropertiesDialog::setFetchInterval(int i)
{
   widget->updateSpinBox->setValue(i);
}

bool FeedPropertiesDialog::useCustomExpiry() const
{
    return widget->expChkbox->isChecked();     
}

int FeedPropertiesDialog::expiryAge() const
{
    if (widget->expiryComboBox->currentItem() == 0)
        return 0;
    else
        return widget->expirySpinBox->value();
}

void FeedPropertiesDialog::setUseCustomExpiry(bool enabled)
{
    widget->expChkbox->setChecked(enabled);
    if (!enabled)
    {
        widget->expirySpinBox->setEnabled(false);
        widget->expiryComboBox->setEnabled(false);
        widget->expiryComboBox->setCurrentItem(0); // "never"
    }
    else
    {
        widget->expiryComboBox->setEnabled(true);
    }   
}

void FeedPropertiesDialog::setExpiryAge(int days)
{
    if (days == 0)
    {
        widget->expiryComboBox->setCurrentItem(0); // "never"
        widget->expirySpinBox->setEnabled(false);
    }    
    else
    {
        widget->expiryComboBox->setCurrentItem(1); // "days"
        widget->expirySpinBox->setEnabled(true);
    }    
    widget->expirySpinBox->setValue(days);
}
         
void FeedPropertiesDialog::selectFeedName()
{
   widget->feedNameEdit->selectAll();
}

#include "propertiesdialog.moc"
