/* Dialog for handling the properties of a mail folder
 */
#ifndef __KMFOLDERDIA
#define __KMFOLDERDIA

#include <qdialog.h>

class KMAccount;
class KMAcctMgr;
class KMAcctFolder;
class QPushButton;
class QLineEdit;
class QListBox;

#define KMFolderDialogInherited QDialog

class KMFolderDialog : public QDialog
{
  Q_OBJECT

public:
  KMFolderDialog(KMAcctFolder* folder, QWidget *parent=0, 
		 const char *name=0);

private slots:
  void doAccept();
  void doAdd();
  void doAccountHighlighted(int);
  void doAccountSelected(int);
  void doAssocHighlighted(int);
  void doAssocSelected(int);
  void doRemove();

protected:
  QLineEdit *nameEdit;
  QPushButton *addButton,*removeButton;
  QListBox *assocList,*accountList;
  KMAcctFolder* folder;
};

#endif /*__KMFOLDERDIA*/

