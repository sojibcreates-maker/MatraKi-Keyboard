/*
 *  MatraKi Keyboard
 *  Software Update Dialog — Emerald Design System
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
 */

#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class UpdateDialog;
}

class UpdateDialog : public QDialog {
  Q_OBJECT

public:
  explicit UpdateDialog(QWidget *parent = nullptr);
  ~UpdateDialog() override;

  void setUpdateInfo(bool updateAvailable, const QString &latestVer,
                     const QString &releaseTitle, const QString &changelog,
                     const QString &releaseUrl);

protected:
  void showEvent(QShowEvent *event) override;

private slots:
  void on_btnCloseIcon_clicked();
  void on_btnLater_clicked();
  void on_btnGitHub_clicked();
  void on_btnAction_clicked();

private:
  Ui::UpdateDialog *ui;
  bool m_updateAvailable = false;
  QString m_releaseUrl;

  void applyTheme();
};

#endif // UPDATEDIALOG_H
