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

#include "AboutLayoutDialog.h"
#include "ui_AboutLayoutDialog.h"

#include <QPushButton>

#include "Layout.h"
#include "MatraKiIcons.h"
#include "MatraKiLogo.h"
#include "MatraKiWindowChrome.h"
#include "ThemeManager.h"

AboutLayoutDialog::AboutLayoutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutLayoutDialog) {
  ui->setupUi(this);

  setAttribute(Qt::WA_TranslucentBackground, true);
  setWindowIcon(QIcon(":/images/MatraKi.png"));
  makeOnyxDialog(this);

  ui->btnCloseIcon->setObjectName("iconOnly");

  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](bool) {
    applyTheme();
  });

  applyTheme();
  loadLayoutInfo();
}

AboutLayoutDialog::~AboutLayoutDialog() {
  delete ui;
}

void AboutLayoutDialog::showEvent(QShowEvent *event) {
  QDialog::showEvent(event);
  applyTheme();
  loadLayoutInfo();
}

void AboutLayoutDialog::applyTheme() {
  const MatraKiPalette p = ThemeManager::instance().palette();

  setStyleSheet(ThemeManager::instance().aboutLayoutDialogStyle());

  ui->logoSlot->setPixmap(MatraKiLogo::render(26, p.brand, p.danger));
  ui->btnCloseIcon->setIcon(QIcon(MatraKiIcons::pixmap(MatraKiIcons::Close, p.icon, 18)));
}

void AboutLayoutDialog::refresh() {
  loadLayoutInfo();
}

void AboutLayoutDialog::loadLayoutInfo() {
  LayoutDesc desc = gLayout->getDesc();

  ui->lblLayoutName->setText(desc.name);
  ui->lblVersionChip->setText(QStringLiteral("Version %1").arg(desc.ver));

  if (desc.name == "Avro Phonetic") {
    ui->lblStatOneValue->setText(QStringLiteral("15,580+"));
    ui->lblStatOneCaption->setText(QStringLiteral("Trained words"));
    ui->lblStatTwoValue->setText(QStringLiteral("38,592"));
    ui->lblStatTwoCaption->setText(QStringLiteral("Auto-correct entries"));
    ui->lblAuthor->setText(QStringLiteral("MatraKi AI Edition / Mehdi Hasan"));
    ui->lblDescription->setText(
        QStringLiteral("MatraKi Enhanced Avro Phonetic Engine — powered by 15,580+ trained "
                       "words, 38,592 auto-correct entries, context-aware next-word predictions, "
                       "Bangla Academy spelling rules, live math/unit converter (=) and instant "
                       "chat snippets (!)."));
  } else {
    ui->lblStatOneValue->setText(desc.ver);
    ui->lblStatOneCaption->setText(QStringLiteral("Layout version"));
    ui->lblStatTwoValue->setText(desc.type == Layout_Fixed ? QStringLiteral("Fixed")
                                                           : QStringLiteral("Phonetic"));
    ui->lblStatTwoCaption->setText(QStringLiteral("Layout type"));
    ui->lblAuthor->setText(desc.devName);
    ui->lblDescription->setText(desc.devComment);
  }
}

void AboutLayoutDialog::on_btnClose_clicked() {
  close();
}

void AboutLayoutDialog::on_btnCloseIcon_clicked() {
  close();
}
