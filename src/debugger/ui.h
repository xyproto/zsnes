/*
Copyright (C) 1997-2007 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes
https://zsnes.bountysource.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

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

  private slots:
  void on_pauseButton_clicked();

  public:
  static void showQtDebugger(QWidget *parent);
  static void destroyQtDebugger();
};

#endif
