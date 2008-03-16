#include "ui_configurationdialog.h"
#include "configurationdialog.h"
#include "configurationdialogadd.h"

#include <kdebug.h>
#include <QTimer>

namespace feedsync
{

ConfigurationDialog::ConfigurationDialog( QWidget *parent)
{
    kDebug();

    QTimer::singleShot( 0, this, SLOT(slotDelayedInit()) );
}

ConfigurationDialog::~ConfigurationDialog()
{
    kDebug();
    delete ui;
}

void ConfigurationDialog::refresh()
{
    kDebug();

    // Clear
    ui->list_readerList->clear();

    // Read configuration
    KConfig config("akregator_feedsyncrc");
    QList<QTreeWidgetItem *> items;
    foreach ( const QString& groupname, config.groupList() ) {
        if (groupname.left(15)=="FeedSyncSource_") {
            kDebug() << groupname;
            KConfigGroup generalGroup( &config, groupname );
            QStringList line;
            line.append( generalGroup.readEntry( "AggregatorType", QString() ) );
            line.append( generalGroup.readEntry( "Identifier", QString() ) );
            line.append( groupname );
            items.append( new QTreeWidgetItem((QTreeWidget*)0,line) );
        }
        ui->list_readerList->insertTopLevelItems(0, items);
    }
}

// SLOT

void ConfigurationDialog::slotChildEnd()
{
    kDebug();
    refresh();
}

void ConfigurationDialog::slotFinished()
{
    kDebug();
    deleteLater();
}

void ConfigurationDialog::slotButtonUpdateClicked()
{
    kDebug();
    ConfigurationDialogAdd * addDlg = new ConfigurationDialogAdd();
    addDlg->show();
    connect( addDlg, SIGNAL( finished() ), this, SLOT( slotChildEnd() ) );
}

void ConfigurationDialog::slotButtonAddClicked()
{
    kDebug();
    ConfigurationDialogAdd * addDlg = new ConfigurationDialogAdd();
    addDlg->show();
    connect( addDlg, SIGNAL( finished() ), this, SLOT( slotChildEnd() ) );
}

void ConfigurationDialog::slotButtonRemoveClicked()
{
    // kDebug();

    QList<QTreeWidgetItem *> m_items = ui->list_readerList->selectedItems();
    if (m_items.count()>0) {
        kDebug() << m_items.at(0)->text(2);
        KConfig config("akregator_feedsyncrc");
        config.deleteGroup(m_items.at(0)->text(2));
    }

    refresh();
}

void ConfigurationDialog::slotDelayedInit()
{
    kDebug();

    // UI setup
    QWidget *widget = new QWidget( this );
    ui = new Ui::ConfigurationDialog();
    ui->setupUi(widget);
    setMainWidget( widget );

    setCaption( i18n("Online reader") );

    // Read config file
    refresh();

    // Init
    ui->list_readerList->setColumnCount(2);
    QStringList deleteTags;
        deleteTags.append( i18n("Never") );
        deleteTags.append( i18n("Always") );
        deleteTags.append( i18n("Ask") );
    ui->cb_deleteFeeds->addItems(deleteTags);
    QStringList title;
        title.append( i18n("Type") );
        title.append( i18n("Description") );
    ui->list_readerList->setHeaderLabels(title);

    // Slots
    connect( ui->b_add, SIGNAL( clicked() ), this, SLOT( slotButtonAddClicked() ) );
    connect( ui->b_update, SIGNAL( clicked() ), this, SLOT( slotButtonUpdateClicked() ) );
    connect( ui->b_remove, SIGNAL( clicked() ), this, SLOT( slotButtonRemoveClicked() ) );
    connect( this, SIGNAL( finished() ), this, SLOT( slotFinished() ) );
}

}
