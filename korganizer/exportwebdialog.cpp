// $Id$
//
// ExportWebDialog.cpp - Implementation of web page export dialog.
//

#include "exportwebdialog.h"
#include "exportwebdialog.moc"

#include <qlayout.h>
#include <qhgroupbox.h>
#include <qvgroupbox.h>
#include <qvbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qhbox.h>
#include <qfiledialog.h>
#include <qtextstream.h>

#include <klocale.h>
#include <kfiledialog.h>
#include <ktempfile.h>
#include <kurl.h>
#include <kio/job.h>

#include "calobject.h"
#include "kdated.h"

ExportWebDialog::ExportWebDialog (CalObject *cal, QWidget *parent,
                                  const char *name) :
  KDialogBase(Tabbed,i18n("Export calendar as web page"),
              Help|Default|User1|Cancel,User1,parent,name,false,false,
              i18n("Export")),
  mCalendar(cal)
{
  setupGeneralPage();
  setupTodoPage();
  setupAdvancedPage();
  
  QObject::connect(this,SIGNAL(user1Clicked()),this,SLOT(exportWebPage()));
}

ExportWebDialog::~ExportWebDialog()
{
}

void ExportWebDialog::setupGeneralPage()
{
  mGeneralPage = addPage(i18n("General"));

  QVBoxLayout *topLayout = new QVBoxLayout(mGeneralPage, 10);
  
  QGroupBox *rangeGroup = new QHGroupBox(i18n("Date Range"),mGeneralPage);
  topLayout->addWidget(rangeGroup);
  
  mFromDate = new KDateEdit(rangeGroup);
  mFromDate->setDate(QDate::currentDate());
  
  mToDate = new KDateEdit(rangeGroup);
  mToDate->setDate(QDate::currentDate());

  QButtonGroup *typeGroup = new QVButtonGroup(i18n("View Type"),mGeneralPage);
  topLayout->addWidget(typeGroup);
  
  
  // For now we just support the todo view. Other view types will follow
  // shortly.
//  new QRadioButton(i18n("Day"), typeGroup);
//  new QRadioButton(i18n("Week"), typeGroup);
//  new QRadioButton(i18n("Month"), typeGroup);

  mCbEvent = new QCheckBox(i18n("Event List"), typeGroup);
  // Implement event export to enable this button
  mCbEvent->setEnabled(false);  
  mCbTodo = new QCheckBox(i18n("To-Do List"), typeGroup);
  mCbTodo->setChecked(true);  

  QGroupBox *destGroup = new QVGroupBox(i18n("Destination"),mGeneralPage);
  topLayout->addWidget(destGroup);

  new QLabel(i18n("Output File:"),destGroup);

  QHBox *outputFileLayout = new QHBox(destGroup);
  QString str = QDir::homeDirPath() + "/calendar.html";
  mOutputFileEdit = new QLineEdit(str,outputFileLayout);
  QPushButton *browseButton = new QPushButton(i18n("Browse"),outputFileLayout);
  QObject::connect(browseButton, SIGNAL(clicked()),
                   this, SLOT(browseOutputFile()));
  
  topLayout->addStretch(1);
}

void ExportWebDialog::setupTodoPage()
{
  mTodoPage = addPage(i18n("To-Do"));
  
  QVBoxLayout *topLayout = new QVBoxLayout(mTodoPage, 10);
  
  mCbDueDates = new QCheckBox (i18n("Due Dates"),mTodoPage);
  topLayout->addWidget(mCbDueDates);
  
  topLayout->addStretch(1);
}

void ExportWebDialog::setupAdvancedPage()
{
  mAdvancedPage = addPage(i18n("Advanced"));
  
  QVBoxLayout *topLayout = new QVBoxLayout(mAdvancedPage, 10);
  
  mCbHtmlFragment = new QCheckBox (i18n("Only generate HTML fragment"),
                                   mAdvancedPage);
  topLayout->addWidget(mCbHtmlFragment);  
  
  QPushButton *colorsButton = new QPushButton(i18n("Colors"),mAdvancedPage);
  topLayout->addWidget(colorsButton);
  
  // Implement the functionality to enable this buttons.
  mCbHtmlFragment->setEnabled(false);
  colorsButton->setEnabled(false);
  
  topLayout->addStretch(1);
}

void ExportWebDialog::browseOutputFile()
{
// KFileDialog seems to be broken, use QFileDIalog instead
#if 0
  qDebug("ExportWebDialog::browseOutputFile()");
  KURL u = KFileDialog::getSaveURL();
//  KURL url = KFileDialog::getSaveURL(QString::null, QString::null, this,
//                                     i18n("Output File"));
  qDebug("ExportWebDialog::browseOutputFile() 1");
  QString str = u.path();
  qDebug("ExportWebDialog::browseOutputFile() 2");
  if(!str.isEmpty())
    qDebug("ExportWebDialog::browseOutputFile() 3");
    mOutputFileEdit->setText(str);
  qDebug("ExportWebDialog::browseOutputFile() done");
#endif

  QString str = QFileDialog::getSaveFileName();
  if(!str.isEmpty())
    mOutputFileEdit->setText(str);
}

void ExportWebDialog::exportWebPage()
{
  KTempFile tmpFile;
//  tmpFile.setAutoDelete(true);
  QTextStream *ts = tmpFile.textStream();
  
  // Write HTML header
  *ts << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\" ";
  *ts << "\"http://www.w3.org/TR/REC-html40/loose.dtd\">\n";
  
  *ts << "<HTML><HEAD>" << endl;
  *ts << "  <TITLE>KOrganizer To-Do List</TITLE>\n";
  *ts << "  <style type=\"text/css\">\n";
  *ts << "    body { background-color:white; color:black }\n";
  *ts << "    td { text-align:center; background-color:#eee }\n";
  *ts << "    th { text-align:center; background-color:#228; color:white }\n";
  *ts << "    td.sum { text-align:left }\n";
  *ts << "    td.sumdone { text-align:left; background-color:#ccc }\n";
  *ts << "    td.done { background-color:#ccc }\n";
  *ts << "    td.subhead { text-align:center; background-color:#ccf }\n";
  *ts << "    td.space { background-color:white }\n";
  *ts <<   "</style>\n";
  *ts << "</HEAD><BODY>\n";

  // TO DO: Write KOrganizer header
  // (Heading, Calendar-Owner, Calendar-Date, ...)  
  *ts << "<H1>KOrganizer To-Do List</H1>\n";

  // Write HTML page content
  createHtmlTodoList(ts);

  // Write KOrganizer trailer
  *ts << "<P>This page was created by <A HREF=\"http://"
      << "devel-home.kde.org/~korganiz\">KOrganizer</A></P>\n";
  
  // Write HTML trailer
  *ts << "</BODY></HTML>\n";

  // Copy temporary file to final destination
  QString srcStr = tmpFile.name();
  KURL::encode(srcStr);
  KURL src(srcStr);
  
  QString destStr = mOutputFileEdit->text();
  KURL::encode(destStr);
  KURL dest(destStr);
  
  KIO::Job *job = KIO::move(src,dest);
  connect(job,SIGNAL(result(KIO::Job *)),SLOT(slotResult(KIO::Job *)));
}

void ExportWebDialog::slotResult(KIO::Job *job)
{
  if (job->error())
  {
    job->showErrorDialog();
  } else {
    accept();
  }
}

void ExportWebDialog::createHtmlTodoList (QTextStream *ts)
{
  KOEvent *ev,*subev;
  
  QList<KOEvent> todoList = mCalendar->getTodoList();  
  
  *ts << "<TABLE BORDER=0 CELLPADDING=3 CELLSPACING=3>\n";
  *ts << "  <TR>\n";
  *ts << "    <TH CLASS=sum>Task</TH> \n";
  *ts << "    <TH>Priority</TH>\n";
  *ts << "    <TH>Status</TH>\n";
  if (mCbDueDates->isChecked()) {
    *ts << "    <TH>Due Date</TH>\n";
  }

  // Create top-level list.
  for(ev=todoList.first();ev;ev=todoList.next()) {
    if (!ev->getRelatedTo()) createHtmlTodo(ts,ev);
  }

  // Create sub-level lists
  for(ev=todoList.first();ev;ev=todoList.next()) {
    QList<KOEvent> relations = ev->getRelations();
    if (relations.count()) {
      // Generate sub-task list of event ev
      *ts << "  <TR>\n";
      *ts << "    <TD CLASS=subhead COLSPAN=";
      if (mCbDueDates->isChecked()) *ts << "4";
      else *ts << "3";
      *ts << "><A NAME=\"sub" << ev->getVUID() << "\"></A>"
          << "Sub-Tasks of: <A HREF=\"#"
          << ev->getVUID() << "\"><B>" << ev->getSummary() << "</B></A></TD>\n";
      *ts << "  </TR>\n";
      
      for(subev=relations.first();subev;subev=relations.next()) {
        createHtmlTodo(ts,subev);
      }
    }
  }

  *ts << "</TABLE>\n";
}

void ExportWebDialog::createHtmlTodo (QTextStream *ts,KOEvent *todo)
{
  bool completed = todo->getStatus() == KOEvent::COMPLETED;
  QList<KOEvent> relations = todo->getRelations();

  *ts << "<TR>\n";

  *ts << "  <TD CLASS=sum";
  if (completed) *ts << "done";
  *ts << ">\n";
  *ts << "    <A NAME=\"" << todo->getVUID() << "\"></A>\n";
  *ts << "    <B>" << todo->getSummary() << "</B>\n";
  if (!todo->getDescription().isEmpty()) {
    *ts << "    <P>" << todo->getDescription() << "</P>\n";
  }
  if (relations.count()) {
    *ts << "    <DIV ALIGN=right><A HREF=\"#sub" << todo->getVUID()
        << "\">Sub-Tasks</A></DIV>\n";
  }

  *ts << "  </TD";
  if (completed) *ts << " CLASS=done";
  *ts << ">\n";

  *ts << "  <TD";
  if (completed) *ts << " CLASS=done";
  *ts << ">\n";
  *ts << "    " << todo->getPriority() << "\n";
  *ts << "  </TD>\n";

  *ts << "  <TD";
  if (completed) *ts << " CLASS=done";
  *ts << ">\n";
  *ts << "    " << (completed ? "Done" : "Open")
      << "\n";
  *ts << "  </TD>\n";

  if (mCbDueDates->isChecked()) {
    *ts << "  <TD";
    if (completed) *ts << " CLASS=done";
    *ts << ">\n";
    if (todo->hasDueDate()) {
      *ts << "    " << todo->getDtDueDateStr() << "\n";
    } else {
      *ts << "    &nbsp;\n";
    }
    *ts << "  </TD>\n";
  }

  *ts << "</TR>\n";
}
