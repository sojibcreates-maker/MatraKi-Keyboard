/*
 *  MatraKi Keyboard
 *  About Dialog — Onyx Design System
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

#include "AboutDialog.h"
#include "ui_AboutDialog.h"

#include <QDesktopServices>
#include <QShowEvent>
#include <QUrl>

#include "MatraKiIcons.h"
#include "MatraKiLogo.h"
#include "MatraKiWindowChrome.h"
#include "ThemeManager.h"

static const QString LICENSE_URL =
    QStringLiteral("https://www.gnu.org/licenses/gpl-3.0.html");
static const QString DONATE_URL =
    QStringLiteral("https://www.supportkori.com/sojib");
static const QString FACEBOOK_URL =
    QStringLiteral("https://facebook.com/hossain.sojibs");

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog) {
  ui->setupUi(this);

  setAttribute(Qt::WA_TranslucentBackground, true);
  setWindowIcon(QIcon(":/images/MatraKi.png"));
  makeOnyxDialog(this);

  ui->btnClose->setObjectName("btnClose");
  ui->btnLicense->setObjectName("btnLicense");
  ui->btnDonate->setObjectName("btnDonate");

  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](bool) {
    applyTheme();
  });

  applyTheme();
}

AboutDialog::~AboutDialog() {
  delete ui;
}

void AboutDialog::showEvent(QShowEvent *event) {
  QDialog::showEvent(event);
  applyTheme();
}

void AboutDialog::applyTheme() {
  const MatraKiPalette p = ThemeManager::instance().palette();

  setStyleSheet(ThemeManager::instance().aboutDialogStyle());

  ui->logoSm->setPixmap(MatraKiLogo::render(28, p.brand, p.danger));
  ui->heroLogo->setPixmap(MatraKiLogo::render(44, p.brand, p.danger));
  ui->btnClose->setIcon(QIcon(MatraKiIcons::pixmap(MatraKiIcons::Close, p.icon, 18)));

  const QString kbdStyle = QStringLiteral(
      "font-family:ui-monospace,Consolas,monospace; font-size:11.5px; "
      "background:%1; border:1px solid %2; border-radius:5px; padding:1.5px 7px; color:%3;")
      .arg(p.surfaceAlt, p.border, p.accent);

  const QString greenHead = p.accent;
  const QString boldGreen = p.accent;

  /* Tagline */
  ui->lblTagline->setText(
      QStringLiteral("<span style=\"color:%1;\"><b style=\"color:%2;\">Type Smarter. Write Better.</b> "
                     "— Next-gen phonetic &amp; fixed-layout input suite for Linux</span>")
          .arg(p.textMuted, boldGreen));

  /* Section 1: AI Phonetic & Language Intelligence */
  ui->sec1Body->setText(
      QStringLiteral(
          "<div style=\"font-size:13px; line-height:1.65; color:%1;\">"
          "<p style=\"margin-bottom:8px;\">• <b>Massive trained vocabulary</b> — high-frequency Bangla words with smart agglutinative suffix stemming ("
          "<span style=\"%2\">করেছি</span> <span style=\"%2\">যাবে</span> <span style=\"%2\">করবে</span>).</p>"
          "<p style=\"margin-bottom:8px;\">• <b>Context-aware next-word prediction</b> — natural follow-ups right after pressing Space, ranked by subject pronoun concordance ("
          "<span style=\"%2\">আমি করছি</span>, <span style=\"%2\">তুমি করছ</span>).</p>"
          "<p>• <b>Bangla Academy spelling &amp; grammar engine</b> — auto-corrects common Nattabidhi, Shattabidhi and vowel-length errors.</p>"
          "</div>")
          .arg(p.text, kbdStyle));

  /* Section 2: Smart Productivity Tools */
  ui->sec2Body->setText(
      QStringLiteral(
          "<div style=\"font-size:13px; line-height:1.65; color:%1;\">"
          "<p style=\"margin-bottom:8px;\">• <b>Live calculator &amp; unit converter</b> — evaluate math inline and convert units instantly ("
          "<span style=\"%2\">=10km_→_km</span>).</p>"
          "<p style=\"margin-bottom:8px;\">• <b>Instant chat &amp; Islamic snippets</b> — type <span style=\"%2\">islam</span>, "
          "<span style=\"%2\">bsmillah</span>, <span style=\"%2\">in1</span>, <span style=\"%2\">i1shuvo</span> with smart prefix matching.</p>"
          "<p>• <b>Smart dates, ordinals, currency &amp; emojis</b> — <span style=\"%2\">1la</span>, <span style=\"%2\">21she</span>, "
          "<span style=\"%2\">500tk</span>, <span style=\"%2\">@today</span>, <span style=\"%2\">:smile</span>.</p>"
          "</div>")
          .arg(p.text, kbdStyle));

  /* Shortcut bar */
  ui->lblShortcuts->setText(
      QStringLiteral(
          "<div style=\"font-size:12px; color:%1;\">"
          "<b style=\"color:%2;\">Shortcuts:</b> &nbsp; "
          "<span style=\"%3\">Ctrl+Space</span> Toggle Bangla/English &nbsp;·&nbsp; "
          "<b style=\"color:%2;\">Double Space</b> দাঁড়ি (।) &nbsp;·&nbsp; "
          "<span style=\"%3\">Tab</span>/<span style=\"%3\">Arrows</span> Select &nbsp;·&nbsp; "
          "<span style=\"%3\">1–9</span> Pick snippet"
          "</div>")
          .arg(p.textMuted, p.text, kbdStyle));

  /* Dev bar */
  ui->lblDev->setText(
      QStringLiteral("<span style=\"font-size:12.5px; color:%1;\">Developer&nbsp; <b style=\"color:%2;\">Md Sojib Hossain</b></span>")
          .arg(p.textMuted, p.text));

  ui->lblFacebook->setText(
      QStringLiteral("<span style=\"font-size:12.5px; color:%1;\">Facebook: "
                     "<a style=\"color:%2; font-weight:600; text-decoration:underline;\" href=\"%3\">facebook.com/hossain.sojibs</a></span>")
          .arg(p.textMuted, p.accent, FACEBOOK_URL));
}

void AboutDialog::on_btnClose_clicked() {
  close();
}

void AboutDialog::on_btnLicense_clicked() {
  QDesktopServices::openUrl(QUrl(LICENSE_URL));
}

void AboutDialog::on_btnDonate_clicked() {
  QDesktopServices::openUrl(QUrl(DONATE_URL));
}
