/*
 *  MatraKi Keyboard
 *  Harper Grammar & Spell Checker dialog — Onyx Design System
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

#ifndef HARPERDIALOG_H
#define HARPERDIALOG_H

#include <QDialog>

class QLabel;
class QPushButton;
class QTextEdit;

namespace Ui {
class HarperDialog;
}

/**
 * HarperDialog hosts the Automattic Harper™ grammar and spell checker.
 */
class HarperDialog : public QDialog {
  Q_OBJECT

public:
  explicit HarperDialog(QWidget *parent = nullptr);
  ~HarperDialog() override;

  /* Enable or disable the Harper grammar engine from the logo menu. */
  void setHarperEnabled(bool enabled);
  bool isHarperEnabled() const { return m_enabled; }

signals:
  /* Emitted whenever the user flips the switch inside the logo menu. */
  void harperEnabledChanged(bool enabled);

private slots:

  void on_btnCloseIcon_clicked();
  void runCheck();
  void loadSample();
  void openWebStudio();

protected:
  void showEvent(QShowEvent *event) override;

private:
  Ui::HarperDialog *ui;
  bool m_enabled = true;

  void applyTheme();
};

#endif // HARPERDIALOG_H
