#ifndef KLISTBOXDIALOG_H
#define KLISTBOXDIALOG_H

#include <kdialogbase.h>

class QLabel;
class QListBox;

class KListBoxDialog : public KDialogBase
{
    Q_OBJECT

public:
    KListBoxDialog( QString& _selectedString,
                    const QString& caption,
                    const QString& labelText,
                    QWidget*    parent = 0,
                    const char* name   = 0,
                    bool        modal  = TRUE );
    ~KListBoxDialog();

    QLabel*   descriptionLA;
    QListBox* entriesLB;

private slots:
    void highlighted( const QString& );

protected:
    QString& selectedString;
};

#endif
