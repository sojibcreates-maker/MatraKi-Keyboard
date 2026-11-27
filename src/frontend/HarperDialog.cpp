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

#include "HarperDialog.h"

#include <QDesktopServices>
#include <QLabel>
#include <QProcess>
#include <QTemporaryFile>
#include <QTextEdit>
#include <QTextStream>
#include <QUrl>
#include <QVBoxLayout>

#include "MatraKiIcons.h"
#include "MatraKiLogo.h"
#include "MatraKiWindowChrome.h"
#include "ThemeManager.h"
#include "ui_HarperDialog.h"

static const QString HARPER_WEB_STUDIO =
    QStringLiteral("https://sojibcreates-maker.github.io/MatraKi-Keyboard/#demo");

HarperDialog::HarperDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HarperDialog) {
  ui->setupUi(this);

  setAttribute(Qt::WA_TranslucentBackground, true);
  setWindowIcon(QIcon(":/images/MatraKi.png"));
  makeOnyxDialog(this);
  ui->btnCloseIcon->setObjectName("iconOnly");

  ui->btnCheck->setObjectName("primary");
  ui->txtOutput->setObjectName("output");
  ui->txtOutput->setLineWrapMode(QTextEdit::WidgetWidth);
  ui->txtOutput->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  connect(ui->btnCheck, &QPushButton::clicked, this, &HarperDialog::runCheck);
  connect(ui->btnSample, &QPushButton::clicked, this, &HarperDialog::loadSample);
  connect(ui->btnWeb, &QPushButton::clicked, this, &HarperDialog::openWebStudio);

  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](bool) {
    applyTheme();
  });

  applyTheme();
  runCheck();
}

HarperDialog::~HarperDialog() {
  delete ui;
}

void HarperDialog::applyTheme() {
  const MatraKiPalette p = ThemeManager::instance().palette();
  setStyleSheet(ThemeManager::instance().harperDialogStyle());

  ui->btnCloseIcon->setIcon(QIcon(MatraKiIcons::pixmap(MatraKiIcons::Close, p.icon, 16)));
  ui->logoSlot->setPixmap(MatraKiLogo::render(34, p.brand, p.danger));

  ui->btnCheck->setIcon(QIcon(MatraKiIcons::pixmap(MatraKiIcons::SpellCheck, p.onBrand, 18)));
  ui->btnSample->setIcon(QIcon(MatraKiIcons::pixmap(MatraKiIcons::Sparkles, p.text, 18)));
  ui->btnWeb->setIcon(QIcon(MatraKiIcons::pixmap(MatraKiIcons::World, p.text, 18)));
}

void HarperDialog::showEvent(QShowEvent *event) {
  QDialog::showEvent(event);
  applyTheme();
}

void HarperDialog::setHarperEnabled(bool enabled) {
  if (m_enabled == enabled) return;
  m_enabled = enabled;
  emit harperEnabledChanged(enabled);
}

void HarperDialog::loadSample() {
  ui->txtInput->setText(QStringLiteral(
      "This is an test with a apple and alot of issues in monday. "
      "I want to to say that their is no problem."));
  runCheck();
}

void HarperDialog::openWebStudio() {
  QDesktopServices::openUrl(QUrl(HARPER_WEB_STUDIO));
}

void HarperDialog::runCheck() {
  const MatraKiPalette p = ThemeManager::instance().palette();

  if (!m_enabled) {
    ui->txtOutput->setHtml(
        QStringLiteral("<p style='color:%1; font-size:13px;'>⏸ হার্পার ব্যাকরণ ইঞ্জিন বন্ধ আছে।</p>"
                       "<p style='color:%2;'>লোগো মেনু থেকে 'Harper grammar &amp; spell check' "
                       "সুইচটি চালু করে আবার চেষ্টা করুন।</p>")
            .arg(p.danger, p.textMuted));
    return;
  }

  const QString text = ui->txtInput->toPlainText().trimmed();
  if (text.isEmpty()) {
    ui->txtOutput->setPlainText(QStringLiteral("যাচাই করার জন্য কিছু লেখা লিখুন।"));
    return;
  }

  QTemporaryFile tempFile;
  if (!tempFile.open()) {
    ui->txtOutput->setPlainText(QStringLiteral("অস্থায়ী ফাইল তৈরি করা যায়নি।"));
    return;
  }
  {
    QTextStream out(&tempFile);
    out << text << "\n";
  }
  tempFile.close();

  QProcess proc;
  proc.start(QStringLiteral("harper-cli"),
             QStringList() << QStringLiteral("--no-color") << QStringLiteral("lint")
                           << tempFile.fileName());
  proc.waitForFinished(5000);

  const QString output = QString::fromUtf8(proc.readAllStandardOutput());
  const QString errOutput = QString::fromUtf8(proc.readAllStandardError());
  const QString combined = (output + QStringLiteral("\n") + errOutput).trimmed();

  if (proc.exitCode() == 0 && !combined.contains(QStringLiteral("Advice:"))) {
    ui->txtOutput->setHtml(
        QStringLiteral("<p style='color:%1; font-weight:600; font-size:13px;'>"
                       "✓ কোনো ব্যাকরণ বা বানান ভুল পাওয়া যায়নি!</p>"
                       "<p style='color:%2;'>আপনার লেখাটি সম্পূর্ণ সঠিক এবং সব হার্পার নিয়ম "
                       "পাস করেছে।</p>")
            .arg(p.accent, p.textMuted));
  } else {
    ui->txtOutput->setPlainText(combined);
  }
}

void HarperDialog::on_btnCloseIcon_clicked() {
  close();
}
