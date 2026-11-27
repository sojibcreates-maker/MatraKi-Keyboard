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

#include <QCloseEvent>
#include <QComboBox>
#include <QDesktopServices>
#include <QEvent>
#include <QListView>
#include <QPushButton>
#include <QShowEvent>
#include <QSignalBlocker>
#include <QStyledItemDelegate>
#include <QUrl>

#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include "Settings.h"
#include "AutoCorrectDialog.h"
#include "MatraKiIcons.h"
#include "MatraKiLogo.h"
#include "MatraKiToggle.h"
#include "MatraKiUpdater.h"
#include "MatraKiWindowChrome.h"
#include "ThemeManager.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog) {
  ui->setupUi(this);

  setAttribute(Qt::WA_TranslucentBackground, true);
  setWindowIcon(QIcon(":/images/MatraKi.png"));
  makeOnyxDialog(this);
  ui->btnCloseIcon->setObjectName("iconOnly");

  ui->gridLayout->setColumnStretch(0, 1);
  ui->gridLayout_2->setColumnStretch(0, 1);
  ui->gridLayout_3->setColumnStretch(0, 1);

  ui->btnSave->setObjectName("primary");
  ui->btnCancel->setObjectName("ghost");
  ui->btnACUpdate->setObjectName("ghost");

  autoCorrectDialog = new AutoCorrectDialog(this);

  auto setupCombo = [this](QComboBox *cb) {
    if (!cb) return;
    cb->setItemDelegate(new QStyledItemDelegate(cb));
    QWidget *container = cb->view() ? cb->view()->window() : nullptr;
    if (container) {
      container->setAttribute(Qt::WA_TranslucentBackground, true);
      container->installEventFilter(this);
    }
  };
  setupCombo(ui->cmbOrientation);
  setupCombo(ui->cmbThemeMode);

  ui->cmbOrientation->insertItems(0, {"Horizontal", "Vertical"});
  ui->cmbThemeMode->insertItems(0, {
    "Auto (System Detect)",
    "Dark Onyx",
    "Light Paper"
  });

  buildToggles();

  connect(ui->btnSave, &QPushButton::clicked, this, &SettingsDialog::saveSettings);
  connect(ui->btnCancel, &QPushButton::clicked, this, [this]() {
    /* The appearance switch previews live; cancelling must put the
       previously saved mode back. */
    ThemeManager::instance().setModeString(gSettings->getThemeMode());
    updateSettings();
    close();
  });
  connect(ui->btnACUpdate, &QPushButton::clicked, [=]() {
    autoCorrectDialog->open();
  });
  connect(ui->btnWebsite, &QPushButton::clicked, [=]() {
    QDesktopServices::openUrl(QUrl("https://sojibcreates-maker.github.io/MatraKi-Keyboard/"));
  });
  connect(ui->btnCheckUpdates, &QPushButton::clicked, [=]() {
    MatraKiUpdater::check(this, false);
  });

  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &SettingsDialog::applyTheme);
  connect(ui->cmbThemeMode, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &SettingsDialog::handleThemeModeChanged);

  updateSettings();
  applyTheme();
}

SettingsDialog::~SettingsDialog() {
  delete autoCorrectDialog;
  delete ui;
}

MatraKiToggle *SettingsDialog::installToggle(QWidget *slot, bool checked) {
  MatraKiToggle *toggle = new MatraKiToggle(slot);
  toggle->setChecked(checked);
  if (slot && slot->layout()) {
    slot->layout()->addWidget(toggle);
    slot->layout()->setAlignment(toggle, Qt::AlignRight | Qt::AlignVCenter);
  }
  return toggle;
}

void SettingsDialog::buildToggles() {
  m_suggestionPhonetic = installToggle(ui->toggleSlotSuggestion, gSettings->getShowCWPhonetic());
  m_includeEnglish = installToggle(ui->toggleSlotEnglish, gSettings->getIncludeEnglishPrevWin());
  m_enterClose = installToggle(ui->toggleSlotEnterClose, gSettings->getEnterKeyClosesPrevWin());
  m_updateCheck = installToggle(ui->toggleSlotUpdateCheck, gSettings->getUpdateCheck());
  m_suggestionFixed = installToggle(ui->toggleSlotSuggestionFixed, gSettings->getShowPrevWinFixed());
  m_autoVowel = installToggle(ui->toggleSlotAutoVowel, gSettings->getAutoVowelFormFixed());
  m_autoChandra = installToggle(ui->toggleSlotAutoChandra, gSettings->getAutoChandraPosFixed());
  m_oldReph = installToggle(ui->toggleSlotOldReph, gSettings->getOldReph());
  m_karJoining = installToggle(ui->toggleSlotKarJoining, gSettings->getTraditionalKarFixed());
  m_numberpad = installToggle(ui->toggleSlotNumberpad, gSettings->getNumberPadFixed());

  /* The English suggestion switch only makes sense while suggestions are
     shown at all. */
  connect(m_suggestionPhonetic, &MatraKiToggle::toggled, this, [this](bool checked) {
    m_includeEnglish->setEnabled(checked);
    ui->btnACUpdate->setEnabled(checked);
  });
}

void SettingsDialog::applyTheme() {
  const MatraKiPalette p = ThemeManager::instance().palette();

  this->setStyleSheet(ThemeManager::instance().settingsDialogStyle());

  ui->btnCloseIcon->setIcon(QIcon(MatraKiIcons::pixmap(MatraKiIcons::Close, p.icon, 16)));
  ui->logoSlot->setPixmap(MatraKiLogo::render(44, p.brand, p.danger));
  ui->iconPhonetic->setPixmap(MatraKiIcons::pixmap(MatraKiIcons::Sparkles, p.accent, 18));
  ui->iconGeneral->setPixmap(MatraKiIcons::pixmap(MatraKiIcons::Settings, p.accent, 18));
  ui->iconFixed->setPixmap(MatraKiIcons::pixmap(MatraKiIcons::Grid, p.accent, 18));
  ui->iconSupport->setPixmap(MatraKiIcons::pixmap(MatraKiIcons::Info, p.accent, 18));

  ui->btnSave->setIcon(QIcon(MatraKiIcons::pixmap(MatraKiIcons::Check, p.onBrand, 16)));
  ui->btnACUpdate->setIcon(QIcon(MatraKiIcons::pixmap(MatraKiIcons::Search, p.accent, 16)));
  ui->btnWebsite->setIcon(QIcon(MatraKiIcons::pixmap(MatraKiIcons::World, p.accent, 16)));
  ui->btnCheckUpdates->setIcon(QIcon(MatraKiIcons::pixmap(MatraKiIcons::Refresh, p.accent, 16)));

  ui->lblVersion->setText(QStringLiteral("v%1").arg(QStringLiteral(PROJECT_VERSION)));

  const QList<MatraKiToggle *> toggles = findChildren<MatraKiToggle *>();
  for (MatraKiToggle *toggle : toggles) {
    toggle->setOnColor(p.accent);
    toggle->setOffColor(p.dark ? p.border : p.keyBorder);
    toggle->setFocusColor(p.accent);
  }

  ui->lblThemeDetected->setText("System: " + ThemeManager::instance().detectedModeName());

  auto refreshCombo = [&p](QComboBox *cb) {
    if (!cb) return;
    QWidget *container = cb->view() ? cb->view()->window() : nullptr;
    if (container) {
      container->setAttribute(Qt::WA_TranslucentBackground, true);
      container->setStyleSheet(QStringLiteral(
        "QFrame { background-color: %1; border: 1px solid %2; border-radius: 8px; margin: 0; padding: 0; }"
      ).arg(p.surface, p.border));
      if (container->layout()) {
        container->layout()->setContentsMargins(0, 0, 0, 0);
        container->layout()->setSpacing(0);
      }
    }
    if (cb->view()) {
      QPalette vpal = cb->view()->palette();
      vpal.setColor(QPalette::Text, QColor(p.text));
      vpal.setColor(QPalette::WindowText, QColor(p.text));
      vpal.setColor(QPalette::Base, QColor(p.surface));
      vpal.setColor(QPalette::Window, QColor(p.surface));
      cb->view()->setPalette(vpal);
    }
  };
  refreshCombo(ui->cmbOrientation);
  refreshCombo(ui->cmbThemeMode);
}

void SettingsDialog::showEvent(QShowEvent *event) {
  QDialog::showEvent(event);
  applyTheme();
}

void SettingsDialog::reject() {
  ThemeManager::instance().setModeString(gSettings->getThemeMode());
  QDialog::reject();
}

void SettingsDialog::closeEvent(QCloseEvent *event) {
  /* The appearance preview is applied immediately, but only Save commits
     it. Closing with the window button therefore rolls back to the saved
     mode — exactly like Cancel. */
  ThemeManager::instance().setModeString(gSettings->getThemeMode());
  QDialog::closeEvent(event);
}

bool SettingsDialog::eventFilter(QObject *watched, QEvent *event) {
  if (event->type() == QEvent::Show) {
    const MatraKiPalette p = ThemeManager::instance().palette();
    QWidget *w = qobject_cast<QWidget *>(watched);
    if (w && w->inherits("QFrame")) {
      w->setAttribute(Qt::WA_TranslucentBackground, true);
      w->setStyleSheet(QStringLiteral(
        "QFrame { background-color: %1; border: 1px solid %2; border-radius: 8px; margin: 0; padding: 0; }"
      ).arg(p.surface, p.border));
      if (w->layout()) {
        w->layout()->setContentsMargins(0, 0, 0, 0);
        w->layout()->setSpacing(0);
      }
    }
  }
  return QDialog::eventFilter(watched, event);
}

void SettingsDialog::handleThemeModeChanged(int index) {
  QString mode = "auto";
  if (index == 1) mode = "dark";
  else if (index == 2) mode = "light";
  ThemeManager::instance().setModeString(mode);
  ui->lblThemeDetected->setText("System: " + ThemeManager::instance().detectedModeName());
  applyTheme();
}

void SettingsDialog::syncToggle(MatraKiToggle *toggle, bool value) {
  const QSignalBlocker blocker(toggle);
  toggle->setChecked(value);
}

void SettingsDialog::updateSettings() {
  /* Phonetic keyboard layout group. */
  syncToggle(m_enterClose, gSettings->getEnterKeyClosesPrevWin());
  syncToggle(m_suggestionPhonetic, gSettings->getShowCWPhonetic());
  ui->cmbOrientation->setCurrentIndex(gSettings->getCandidateWinHorizontal() ? 0 : 1);
  syncToggle(m_includeEnglish, gSettings->getIncludeEnglishPrevWin());

  /* Fixed keyboard layout group. */
  syncToggle(m_suggestionFixed, gSettings->getShowPrevWinFixed());
  syncToggle(m_autoVowel, gSettings->getAutoVowelFormFixed());
  syncToggle(m_autoChandra, gSettings->getAutoChandraPosFixed());
  syncToggle(m_oldReph, gSettings->getOldReph());
  syncToggle(m_karJoining, gSettings->getTraditionalKarFixed());
  syncToggle(m_numberpad, gSettings->getNumberPadFixed());

  syncToggle(m_updateCheck, gSettings->getUpdateCheck());

  m_includeEnglish->setEnabled(m_suggestionPhonetic->isChecked());
  ui->btnACUpdate->setEnabled(m_suggestionPhonetic->isChecked());

  ui->cmbThemeMode->blockSignals(true);
  QString tMode = gSettings->getThemeMode().toLower();
  if (tMode == "dark") ui->cmbThemeMode->setCurrentIndex(1);
  else if (tMode == "light") ui->cmbThemeMode->setCurrentIndex(2);
  else ui->cmbThemeMode->setCurrentIndex(0);
  ui->cmbThemeMode->blockSignals(false);

  ui->lblThemeDetected->setText("System: " + ThemeManager::instance().detectedModeName());
  applyTheme();
}

void SettingsDialog::saveSettings() {
  /* Phonetic keyboard layout group. */
  gSettings->setEnterKeyClosesPrevWin(m_enterClose->isChecked());
  gSettings->setShowCWPhonetic(m_suggestionPhonetic->isChecked());
  gSettings->setCandidateWinHorizontal((ui->cmbOrientation->currentIndex() == 0));
  gSettings->setIncludeEnglishPrevWin(m_includeEnglish->isChecked());

  /* Fixed keyboard layout group. */
  gSettings->setShowPrevWinFixed(m_suggestionFixed->isChecked());
  gSettings->setAutoVowelFormFixed(m_autoVowel->isChecked());
  gSettings->setAutoChandraPosFixed(m_autoChandra->isChecked());
  gSettings->setOldReph(m_oldReph->isChecked());
  gSettings->setTraditionalKarFixed(m_karJoining->isChecked());
  gSettings->setNumberPadFixed(m_numberpad->isChecked());

  gSettings->setUpdateCheck(m_updateCheck->isChecked());

  int tIndex = ui->cmbThemeMode->currentIndex();
  QString mode = (tIndex == 1) ? "dark" : (tIndex == 2 ? "light" : "auto");
  gSettings->setThemeMode(mode);
  ThemeManager::instance().setModeString(mode);

  close();
}

void SettingsDialog::on_btnCloseIcon_clicked() {
  close();
}
