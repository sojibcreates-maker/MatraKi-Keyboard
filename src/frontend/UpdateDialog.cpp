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

#include "UpdateDialog.h"
#include "ui_UpdateDialog.h"

#include <QDesktopServices>
#include <QUrl>

#include "MatraKiIcons.h"
#include "MatraKiLogo.h"
#include "ThemeManager.h"

static const QString DEFAULT_UPDATE_PAGE =
    QStringLiteral("https://sojibcreates-maker.github.io/MatraKi-Keyboard/#downloads");
static const QString DEFAULT_GITHUB_RELEASES =
    QStringLiteral("https://github.com/sojibcreates-maker/MatraKi-Keyboard/releases/latest");

UpdateDialog::UpdateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateDialog) {
  ui->setupUi(this);

  setAttribute(Qt::WA_TranslucentBackground, true);
  setWindowIcon(QIcon(":/images/MatraKi.png"));
  setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint |
                 Qt::WindowStaysOnTopHint);

  ui->btnCloseIcon->setObjectName("iconOnly");
  ui->btnAction->setObjectName("primary");

  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](bool) {
    applyTheme();
  });

  applyTheme();
}

UpdateDialog::~UpdateDialog() {
  delete ui;
}

void UpdateDialog::showEvent(QShowEvent *event) {
  QDialog::showEvent(event);
  applyTheme();
}

void UpdateDialog::applyTheme() {
  const MatraKiPalette p = ThemeManager::instance().palette();

  setStyleSheet(ThemeManager::instance().updateDialogStyle());

  ui->logoSlot->setPixmap(MatraKiLogo::render(26, p.brand, p.danger));
  ui->btnCloseIcon->setIcon(QIcon(MatraKiIcons::pixmap(MatraKiIcons::Close, p.icon, 18)));

  if (m_updateAvailable) {
    ui->iconBadge->setPixmap(MatraKiIcons::pixmap(MatraKiIcons::Refresh, p.accent, 24));
    ui->iconBadge->setStyleSheet(QStringLiteral("background-color: %1; border-radius: 19px;").arg(p.accentSoft));
  } else {
    ui->iconBadge->setPixmap(MatraKiIcons::pixmap(MatraKiIcons::Check, p.brand, 22));
    ui->iconBadge->setStyleSheet(QStringLiteral("background-color: %1; border-radius: 19px;").arg(p.accentSoft));
  }
}

void UpdateDialog::setUpdateInfo(bool updateAvailable, const QString &latestVer,
                                 const QString &releaseTitle, const QString &changelog,
                                 const QString &releaseUrl) {
  m_updateAvailable = updateAvailable;
  m_releaseUrl = releaseUrl.isEmpty() ? DEFAULT_GITHUB_RELEASES : releaseUrl;

  const MatraKiPalette p = ThemeManager::instance().palette();

  if (updateAvailable) {
    setWindowTitle("MatraKi · নতুন আপডেট পাওয়া গেছে!");
    ui->lblSub->setText("নতুন আপডেট পাওয়া গেছে");
    ui->lblHeadline->setText(QString("নতুন আপডেট পাওয়া গেছে · v%1").arg(latestVer));
    ui->lblVersionLine->setText(QString("ইনস্টলকৃত: v%1  •  সর্বশেষ: v%2 (%3)")
                               .arg(QStringLiteral(PROJECT_VERSION), latestVer, releaseTitle));
    ui->lblHighlightsTitle->setText("রিলিজ হাইলাইটস (Release Highlights):");

    QString summary = changelog.trimmed();
    if (summary.isEmpty()) {
      summary = "Next-generation AI-smart Bangla phonetic & fixed-layout suite, engineered natively for Linux.";
    } else if (summary.length() > 240) {
      summary = summary.left(240) + "...";
    }
    ui->lblHighlightsBody->setText(summary);

    ui->btnLater->setText("পরে");
    ui->btnAction->setText("ডাউনলোড আপডেট");
  } else {
    setWindowTitle("MatraKi · সফটওয়্যার আপডেট");
    ui->lblSub->setText("সফটওয়্যার আপডেট");
    ui->lblHeadline->setText(QString("সর্বশেষ সংস্করণ ইনস্টল আছে  •  v%1").arg(QStringLiteral(PROJECT_VERSION)));
    ui->lblVersionLine->setText(QString("ইনস্টলকৃত: v%1 — Harper AI Edition (সর্বশেষ সংস্করণ)").arg(QStringLiteral(PROJECT_VERSION)));
    ui->lblHighlightsTitle->setText("রিলিজ হাইলাইটস:");
    ui->lblHighlightsBody->setText(
        "• 38,000+ প্রি-ট্রেনড শব্দ ও অটো-কারেক্ট নিয়ম\n"
        "• স্মার্ট সাফিক্স স্টেমার, নেক্সট-ওয়ার্ড AI ও লাইভ ম্যাথ (=)\n"
        "• ১০০+ রেডি স্নিপেট (!salam, !bismillah, ইমেইল ও সিভি)");

    ui->btnLater->setText("বন্ধ করুন");
    ui->btnAction->setText("অফিসিয়াল ওয়েবসাইট");
  }

  applyTheme();
}

void UpdateDialog::on_btnCloseIcon_clicked() {
  close();
}

void UpdateDialog::on_btnLater_clicked() {
  close();
}

void UpdateDialog::on_btnGitHub_clicked() {
  QDesktopServices::openUrl(QUrl(m_releaseUrl));
}

void UpdateDialog::on_btnAction_clicked() {
  QDesktopServices::openUrl(QUrl(DEFAULT_UPDATE_PAGE));
}
