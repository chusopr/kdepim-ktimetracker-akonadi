#ifndef SELECTFIELDS_H 
#define SELECTFIELDS_H 

#include <qdialog.h>
#include <qstringlist.h>
#include <qwidget.h>

class QListBox;
class QLineEdit;
class QComboBox;

class SelectFields : public QDialog
{
  Q_OBJECT

public:
  SelectFields( QStringList oldFields,
	        QWidget *parent = 0, 
		const char *name = 0, 
		bool modal = false );
  virtual QStringList chosenFields();

public slots:
  virtual void select();
  virtual void unselect();
  virtual void addCustom();
  virtual void showFields( int );
 
private:
  QStringList currentField;
  QListBox *lbUnSelected;
  QListBox *lbSelected;
  QLineEdit *leCustomField;
  QComboBox *cbUnselected;
};

#endif // PABWIDGET_H 
