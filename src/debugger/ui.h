#ifndef UI_H
#define UI_H

#include <QMainWindow>
#include <QCloseEvent>

#ifdef __UNIXSDL__ //I hate this hack -Nach
#include "debugger/ui_debugger.h"
#else
#include "ui_debugger.h"
#endif

class QtDebugger : public QMainWindow
{
  Q_OBJECT

  private:
  Ui::Debugger ui;

  static QtDebugger *singleton;

  QtDebugger(QWidget *parent);
  ~QtDebugger();

  void closeEvent(QCloseEvent *event);

  private slots:

  public:
  static void showQtDebugger(QWidget *parent);
  static void destroyQtDebugger();
};

#endif
