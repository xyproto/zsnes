#ifndef UI_H
#define UI_H

#include <QDialog>
#include "debugger/ui_debugger.h"

class DebuggerDialog : public QDialog
{
  Q_OBJECT

  private:
  Ui::Debugger ui;

  static DebuggerDialog *singleton;

  DebuggerDialog(QWidget *parent);
  ~DebuggerDialog();

  private slots:

  public:
  static void showDebuggerDialog(QWidget *parent);
};

#endif
