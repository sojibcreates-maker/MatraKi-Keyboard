/*
 *  MatraKi Keyboard
 *  About current keyboard layout — Onyx Design System
 *  Copyright (C) 2016 Muhammad Mominul Huque <mominul2082@gmail.com>
 *  Copyright (C) 2026 Md Sojib Hossain
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ABOUTLAYOUTDIALOG_H
#define ABOUTLAYOUTDIALOG_H

#include <QDialog>

namespace Ui {
class AboutLayoutDialog;
}

/**
 * AboutLayoutDialog shows the details of the keyboard layout that is
 * currently active: name, version, engine statistics, maintainer and the
 * long form description.
 */
class AboutLayoutDialog : public QDialog {
  Q_OBJECT

public:
  explicit AboutLayoutDialog(QWidget *parent = nullptr);
  ~AboutLayoutDialog() override;

  /* Reload the information from the active layout. */
  void refresh();

protected:
  void showEvent(QShowEvent *event) override;

private slots:
  void on_btnClose_clicked();
  void on_btnCloseIcon_clicked();

private:
  Ui::AboutLayoutDialog *ui;

  void applyTheme();
  void loadLayoutInfo();
};

#endif // ABOUTLAYOUTDIALOG_H
