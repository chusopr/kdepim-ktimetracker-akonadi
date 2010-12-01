
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>

class KBreadcrumbNavigationFactory;

class MainWindow : public QWidget
{
  Q_OBJECT
public:
  MainWindow(QWidget *parent = 0);

private:
  KBreadcrumbNavigationFactory *m_bnf;

};

#endif

