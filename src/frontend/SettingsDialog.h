/*
 *  MatraKi Keyboard
 *  Settings — Onyx Design System
 *  Copyright (C) 2016-2018 Muhammad Mominul Huque <mominul2082@gmail.com>
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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class MatraKiToggle;

class AutoCorrectDialog;

namespace Ui {
class SettingsDialog;
}

/**
 * Settings hosts every configurable option of the MatraKi engine, grouped
 * into three onyx cards: phonetic & AI typing, general interface and the
 * fixed keyboard layout behaviour.
 */
class SettingsDialog : public QDialog {
  Q_OBJECT

public:
  explicit SettingsDialog(QWidget *parent = nullptr);
  ~SettingsDialog() override;

  /* Re-read every value from the persistent settings store. */
  void updateSettings();

  /* Cancel live appearance previews when the dialog is dismissed. */
  void reject() override;

public slots:
  void applyTheme();

protected:
  void showEvent(QShowEvent *event) override;
  void closeEvent(QCloseEvent *event) override;
  bool eventFilter(QObject *watched, QEvent *event) override;

private slots:

  void on_btnCloseIcon_clicked();
  void handleThemeModeChanged(int index);
  void saveSettings();

private:
  Ui::SettingsDialog *ui;
  AutoCorrectDialog *autoCorrectDialog;

  MatraKiToggle *m_suggestionPhonetic = nullptr;
  MatraKiToggle *m_includeEnglish = nullptr;
  MatraKiToggle *m_enterClose = nullptr;
  MatraKiToggle *m_updateCheck = nullptr;
  MatraKiToggle *m_suggestionFixed = nullptr;
  MatraKiToggle *m_autoVowel = nullptr;
  MatraKiToggle *m_autoChandra = nullptr;
  MatraKiToggle *m_oldReph = nullptr;
  MatraKiToggle *m_karJoining = nullptr;
  MatraKiToggle *m_numberpad = nullptr;

  /* Build the switch widgets and drop them into their placeholders. */
  void buildToggles();

  /* Attach a switch to a placeholder slot from the designer file. */
  MatraKiToggle *installToggle(QWidget *slot, bool checked);

  void syncToggle(MatraKiToggle *toggle, bool value);
};

#endif // SETTINGSDIALOG_H
