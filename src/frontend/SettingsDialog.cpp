/*
 *  MatraKi Keyboard
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

#include <QPushButton>
#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include "Settings.h"
#include "AutoCorrectDialog.h"
#include "ThemeManager.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog) {
  ui->setupUi(this);
  this->setWindowIcon(QIcon(":/images/MatraKi.png"));
  this->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);
  ui->gridLayout->setColumnStretch(0, 1);
  ui->gridLayout_2->setColumnStretch(0, 1);
  ui->gridLayout_3->setColumnStretch(0, 1);
  autoCorrectDialog = new AutoCorrectDialog(this);

  ui->cmbOrientation->insertItems(0, {"Horizontal", "Vertical"});
  ui->cmbThemeMode->insertItems(0, {
    "Auto (System Detect)",
    "Dark Tech Mode",
    "Light Classic Mode"
  });

  if (QPushButton *okBtn = ui->buttonBox->button(QDialogButtonBox::Ok)) {
    okBtn->setText("✓ Save Settings");
    okBtn->setCursor(Qt::PointingHandCursor);
  }
  if (QPushButton *cancelBtn = ui->buttonBox->button(QDialogButtonBox::Cancel)) {
    cancelBtn->setText("✕ Cancel");
    cancelBtn->setCursor(Qt::PointingHandCursor);
  }

  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &SettingsDialog::applyTheme);

  implementSignals();
  updateSettings();
  applyTheme();
}

SettingsDialog::~SettingsDialog() {
  delete autoCorrectDialog;
  delete ui;
}

void SettingsDialog::applyTheme() {
  this->setStyleSheet(ThemeManager::instance().settingsDialogStyle());
  ui->lblThemeDetected->setText("Detected: " + ThemeManager::instance().detectedModeName());
}

void SettingsDialog::showEvent(QShowEvent *event) {
  QDialog::showEvent(event);
  applyTheme();
}

void SettingsDialog::handleThemeModeChanged(int index) {
  QString mode = "auto";
  if (index == 1) mode = "dark";
  else if (index == 2) mode = "light";
  ThemeManager::instance().setModeString(mode);
  ui->lblThemeDetected->setText("Detected: " + ThemeManager::instance().detectedModeName());
}

void SettingsDialog::implementSignals() {
  auto setToggleText = [](QPushButton *btn, bool checked) {
    btn->setText(checked ? "✓ ON" : "OFF");
  };

  // Phonetic Keyboard Layout Group.
  connect(ui->btnSuggestionPhonetic, &QPushButton::toggled, [=](bool checked) {
    setToggleText(ui->btnSuggestionPhonetic, checked);
    // Control other Preview window related settings.
    ui->btnIncludeEnglishPrevWin->setEnabled(checked);
    ui->btnACUpdate->setEnabled(checked);
  });
  connect(ui->btnEnterClosePW, &QPushButton::toggled, [=](bool checked) {
    setToggleText(ui->btnEnterClosePW, checked);
  });
  connect(ui->btnIncludeEnglishPrevWin, &QPushButton::toggled, [=](bool checked) {
    setToggleText(ui->btnIncludeEnglishPrevWin, checked);
  });
  connect(ui->btnACUpdate, &QPushButton::clicked, [=]() {
    autoCorrectDialog->open();
  });

  // Fixed Keyboard Layout Group.
  connect(ui->btnSuggestionFixed, &QPushButton::toggled, [=](bool checked) {
    setToggleText(ui->btnSuggestionFixed, checked);
  });
  connect(ui->btnAutoVowel, &QPushButton::toggled, [=](bool checked) {
    setToggleText(ui->btnAutoVowel, checked);
  });
  connect(ui->btnKarJoining, &QPushButton::toggled, [=](bool checked) {
    setToggleText(ui->btnKarJoining, checked);
  });
  connect(ui->btnAutoChandra, &QPushButton::toggled, [=](bool checked) {
    setToggleText(ui->btnAutoChandra, checked);
  });
  connect(ui->btnOldReph, &QPushButton::toggled, [=](bool checked) {
    setToggleText(ui->btnOldReph, checked);
  });
  connect(ui->btnNumberpad, &QPushButton::toggled, [=](bool checked) {
    setToggleText(ui->btnNumberpad, checked);
  });

  // General
  connect(ui->btnCheckUpdate, &QPushButton::toggled, [=](bool checked) {
    setToggleText(ui->btnCheckUpdate, checked);
  });
  connect(ui->cmbThemeMode, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &SettingsDialog::handleThemeModeChanged);
}

void SettingsDialog::updateSettings() {
  auto syncToggle = [](QPushButton *btn, bool val) {
    btn->setChecked(val);
    btn->setText(val ? "✓ ON" : "OFF");
  };

  // Phonetic Keyboard Layout Group.
  syncToggle(ui->btnEnterClosePW, gSettings->getEnterKeyClosesPrevWin());
  syncToggle(ui->btnSuggestionPhonetic, gSettings->getShowCWPhonetic());
  ui->cmbOrientation->setCurrentIndex(gSettings->getCandidateWinHorizontal() ? 0 : 1);
  syncToggle(ui->btnIncludeEnglishPrevWin, gSettings->getIncludeEnglishPrevWin());

  // Fixed Keyboard Layout Group.
  syncToggle(ui->btnSuggestionFixed, gSettings->getShowPrevWinFixed());
  syncToggle(ui->btnAutoVowel, gSettings->getAutoVowelFormFixed());
  syncToggle(ui->btnAutoChandra, gSettings->getAutoChandraPosFixed());
  syncToggle(ui->btnOldReph, gSettings->getOldReph());
  syncToggle(ui->btnKarJoining, gSettings->getTraditionalKarFixed());
  syncToggle(ui->btnNumberpad, gSettings->getNumberPadFixed());

  syncToggle(ui->btnCheckUpdate, gSettings->getUpdateCheck());

  ui->cmbThemeMode->blockSignals(true);
  QString tMode = gSettings->getThemeMode().toLower();
  if (tMode == "dark") ui->cmbThemeMode->setCurrentIndex(1);
  else if (tMode == "light") ui->cmbThemeMode->setCurrentIndex(2);
  else ui->cmbThemeMode->setCurrentIndex(0);
  ui->cmbThemeMode->blockSignals(false);
  ui->lblThemeDetected->setText("Detected: " + ThemeManager::instance().detectedModeName());
  applyTheme();
}

void SettingsDialog::on_buttonBox_accepted() {
  // Phonetic Keyboard Layout Group.
  gSettings->setEnterKeyClosesPrevWin(ui->btnEnterClosePW->isChecked());
  gSettings->setShowCWPhonetic(ui->btnSuggestionPhonetic->isChecked());
  gSettings->setCandidateWinHorizontal((ui->cmbOrientation->currentIndex() == 0));
  gSettings->setIncludeEnglishPrevWin(ui->btnIncludeEnglishPrevWin->isChecked());

  // Fixed Keyboard Layout Group.
  gSettings->setShowPrevWinFixed(ui->btnSuggestionFixed->isChecked());
  gSettings->setAutoVowelFormFixed(ui->btnAutoVowel->isChecked());
  gSettings->setAutoChandraPosFixed(ui->btnAutoChandra->isChecked());
  gSettings->setOldReph(ui->btnOldReph->isChecked());
  gSettings->setTraditionalKarFixed(ui->btnKarJoining->isChecked());
  gSettings->setNumberPadFixed(ui->btnNumberpad->isChecked());

  gSettings->setUpdateCheck(ui->btnCheckUpdate->isChecked());

  int tIndex = ui->cmbThemeMode->currentIndex();
  QString mode = (tIndex == 1) ? "dark" : (tIndex == 2 ? "light" : "auto");
  gSettings->setThemeMode(mode);
  ThemeManager::instance().setModeString(mode);
}

void SettingsDialog::on_buttonBox_rejected() {
  SettingsDialog::close();
}
