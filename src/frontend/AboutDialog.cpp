/*
 *  MatraKi Keyboard
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

#include <QDesktopServices>
#include <QUrl>
#include "AboutDialog.h"
#include "ui_AboutDialog.h"
#include "ThemeManager.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog) {
  ui->setupUi(this);

  this->setFixedSize(this->width(), this->height());

  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &AboutDialog::applyTheme);
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
  setStyleSheet(ThemeManager::instance().aboutDialogStyle());
  bool dark = ThemeManager::instance().isDark();

  if (dark) {
    ui->labelTitle->setText(
      "<html><head/><body><p style=\"margin:0;\">"
      "<span style=\"font-size:19pt; font-weight:800; color:#ffffff;\">MatraKi</span>"
      "<span style=\"font-size:12.5pt; font-weight:700; color:#38bdf8;\">  •  AI Smart Bangla Keyboard</span>"
      "</p></body></html>"
    );
    ui->labelTagline->setText(
      "<html><head/><body><p style=\"margin:0;\">"
      "<span style=\"font-size:10.5pt; font-weight:700; color:#f43f5e;\">Type Smarter. Write Better.</span>"
      "<span style=\"font-size:9.5pt; color:#cbd5e1;\"> — Next-Gen Phonetic &amp; Fixed Layout Input Suite</span>"
      "</p></body></html>"
    );
    ui->labelVer->setText(
      "<html><head/><body><p style=\"margin:2px 0 0 0;\">"
      "<span style=\"color:#34d399; font-size:9pt; font-weight:700;\">✨ v1.1.0 (AI Smart Edition)  •  15,580+ Trained Words  •  Harper AI Grammar  •  Light &amp; Dark Mode</span>"
      "</p></body></html>"
    );
    ui->labelDesc->setText(
      "<html><head/><body>"
      "<p style=\"margin:0 0 6px 0;\"><span style=\"font-size:9.2pt; color:#e2e8f0;\"><b style=\"color:#38bdf8;\">MatraKi</b> is an advanced, high-performance Bangla input method engineered for Linux desktops. Combining a blazingly fast Rust phonetic core with Harper AI grammar checking and an intelligent C++ language layer, MatraKi makes writing Bengali faster, smarter, and completely effortless.</span></p>"
      "<p style=\"margin:4px 0 2px 0;\"><span style=\"font-size:9.5pt; font-weight:700; color:#38bdf8;\">🧠 AI Phonetic &amp; Language Intelligence:</span></p>"
      "<p style=\"margin:0 0 6px 8px;\"><span style=\"font-size:8.8pt; color:#cbd5e1;\">"
      "• <b style=\"color:#f8fafc;\">Massive Vocabulary &amp; Suffix Engine:</b> Pre-trained with <b style=\"color:#34d399;\">15,580+</b> Bengali words, <b style=\"color:#34d399;\">20,535</b> auto-correct patterns, and smart agglutinative suffix stemming (<i>-গুলো, -দের, -টা, -টি, -কে, -তে</i>).<br/>"
      "• <b style=\"color:#f8fafc;\">Harper AI Grammar &amp; Spell Check:</b> Built-in automattic/harper integration for real-time contextual grammar verification, Nattvabidhi, and Shattvabidhi accuracy.<br/>"
      "• <b style=\"color:#f8fafc;\">Context-Aware Predictions:</b> Follow-up bigram suggestions right after Space and concordant verb endings (<i>আমি করব, তুমি করবে, আপনি করবেন</i>)."
      "</span></p>"
      "<p style=\"margin:4px 0 2px 0;\"><span style=\"font-size:9.5pt; font-weight:700; color:#f43f5e;\">⚡ Smart Productivity &amp; Instant Conversions:</span></p>"
      "<p style=\"margin:0 0 2px 8px;\"><span style=\"font-size:8.8pt; color:#cbd5e1;\">"
      "• <b style=\"color:#f8fafc;\">Live Calculator &amp; Unit Converter (<span style=\"color:#34d399;\">=</span>):</b> Math (<code style=\"color:#38bdf8;\">=500*15%</code>, <code style=\"color:#38bdf8;\">=2^10</code>) and units (<code style=\"color:#38bdf8;\">=10km</code>, <code style=\"color:#38bdf8;\">=5kg</code>, <code style=\"color:#38bdf8;\">=100c</code>, <code style=\"color:#38bdf8;\">=98.6f</code>, <code style=\"color:#38bdf8;\">=1bigha</code>).<br/>"
      "• <b style=\"color:#f8fafc;\">Instant Chat &amp; Email Templates (<span style=\"color:#fde047;\">!</span>):</b> Greetings (<code style=\"color:#fde047;\">!salam</code>, <code style=\"color:#fde047;\">!shuvo</code>) and templates (<code style=\"color:#fde047;\">!jobbn</code>, <code style=\"color:#fde047;\">!joben</code>, <code style=\"color:#fde047;\">!cvbn</code>, <code style=\"color:#fde047;\">!cven</code>).<br/>"
      "• <b style=\"color:#f8fafc;\">Smart Dates, Currency &amp; Emojis:</b> <code style=\"color:#fb7185;\">1la</code>, <code style=\"color:#fb7185;\">21she</code>, <code style=\"color:#fb7185;\">500tk</code>, <code style=\"color:#fb7185;\">@today</code>, <code style=\"color:#fb7185;\">:smile</code>."
      "</span></p>"
      "</body></html>"
    );
    ui->lblShortcuts->setText(
      "<html><head/><body><p align=\"center\" style=\"margin:0;\">"
      "<span style=\"font-size:8.5pt; color:#94a3b8;\">⌨️ <b style=\"color:#38bdf8;\">Shortcuts:</b>  "
      "<b style=\"color:#f8fafc;\">Ctrl+Space</b> Toggle Bangla/English  •  "
      "<b style=\"color:#f8fafc;\">Double Space</b> Dari (। )  •  "
      "<b style=\"color:#f8fafc;\">Tab / Arrows</b> Select  •  "
      "<b style=\"color:#f8fafc;\">1–9</b> Pick Snippet</span>"
      "</p></body></html>"
    );
    ui->lblDeveloper->setText(
      "<html><head/><body><p style=\"margin:0;\">"
      "<span style=\"font-size:9.5pt; color:#94a3b8;\">👨‍💻 Developer: </span>"
      "<span style=\"font-size:10pt; font-weight:700; color:#ffffff;\">Md Sojib Hossain</span>"
      "</p></body></html>"
    );
  } else {
    ui->labelTitle->setText(
      "<html><head/><body><p style=\"margin:0;\">"
      "<span style=\"font-size:19pt; font-weight:800; color:#0f172a;\">MatraKi</span>"
      "<span style=\"font-size:12.5pt; font-weight:700; color:#0284c7;\">  •  AI Smart Bangla Keyboard</span>"
      "</p></body></html>"
    );
    ui->labelTagline->setText(
      "<html><head/><body><p style=\"margin:0;\">"
      "<span style=\"font-size:10.5pt; font-weight:700; color:#e11d48;\">Type Smarter. Write Better.</span>"
      "<span style=\"font-size:9.5pt; color:#475569;\"> — Next-Gen Phonetic &amp; Fixed Layout Input Suite</span>"
      "</p></body></html>"
    );
    ui->labelVer->setText(
      "<html><head/><body><p style=\"margin:2px 0 0 0;\">"
      "<span style=\"color:#059669; font-size:9pt; font-weight:700;\">✨ v1.1.0 (AI Smart Edition)  •  15,580+ Trained Words  •  Harper AI Grammar  •  Light &amp; Dark Mode</span>"
      "</p></body></html>"
    );
    ui->labelDesc->setText(
      "<html><head/><body>"
      "<p style=\"margin:0 0 6px 0;\"><span style=\"font-size:9.2pt; color:#334155;\"><b style=\"color:#0284c7;\">MatraKi</b> is an advanced, high-performance Bangla input method engineered for Linux desktops. Combining a blazingly fast Rust phonetic core with Harper AI grammar checking and an intelligent C++ language layer, MatraKi makes writing Bengali faster, smarter, and completely effortless.</span></p>"
      "<p style=\"margin:4px 0 2px 0;\"><span style=\"font-size:9.5pt; font-weight:700; color:#0284c7;\">🧠 AI Phonetic &amp; Language Intelligence:</span></p>"
      "<p style=\"margin:0 0 6px 8px;\"><span style=\"font-size:8.8pt; color:#475569;\">"
      "• <b style=\"color:#0f172a;\">Massive Vocabulary &amp; Suffix Engine:</b> Pre-trained with <b style=\"color:#059669;\">15,580+</b> Bengali words, <b style=\"color:#059669;\">20,535</b> auto-correct patterns, and smart agglutinative suffix stemming (<i>-গুলো, -দের, -টা, -টি, -কে, -তে</i>).<br/>"
      "• <b style=\"color:#0f172a;\">Harper AI Grammar &amp; Spell Check:</b> Built-in automattic/harper integration for real-time contextual grammar verification, Nattvabidhi, and Shattvabidhi accuracy.<br/>"
      "• <b style=\"color:#0f172a;\">Context-Aware Predictions:</b> Follow-up bigram suggestions right after Space and concordant verb endings (<i>আমি করব, তুমি করবে, আপনি করবেন</i>)."
      "</span></p>"
      "<p style=\"margin:4px 0 2px 0;\"><span style=\"font-size:9.5pt; font-weight:700; color:#e11d48;\">⚡ Smart Productivity &amp; Instant Conversions:</span></p>"
      "<p style=\"margin:0 0 2px 8px;\"><span style=\"font-size:8.8pt; color:#475569;\">"
      "• <b style=\"color:#0f172a;\">Live Calculator &amp; Unit Converter (<span style=\"color:#059669;\">=</span>):</b> Math (<code style=\"color:#0284c7;\">=500*15%</code>, <code style=\"color:#0284c7;\">=2^10</code>) and units (<code style=\"color:#0284c7;\">=10km</code>, <code style=\"color:#0284c7;\">=5kg</code>, <code style=\"color:#0284c7;\">=100c</code>, <code style=\"color:#0284c7;\">=98.6f</code>, <code style=\"color:#0284c7;\">=1bigha</code>).<br/>"
      "• <b style=\"color:#0f172a;\">Instant Chat &amp; Email Templates (<span style=\"color:#d97706;\">!</span>):</b> Greetings (<code style=\"color:#d97706;\">!salam</code>, <code style=\"color:#d97706;\">!shuvo</code>) and templates (<code style=\"color:#d97706;\">!jobbn</code>, <code style=\"color:#d97706;\">!joben</code>, <code style=\"color:#d97706;\">!cvbn</code>, <code style=\"color:#d97706;\">!cven</code>).<br/>"
      "• <b style=\"color:#0f172a;\">Smart Dates, Currency &amp; Emojis:</b> <code style=\"color:#e11d48;\">1la</code>, <code style=\"color:#e11d48;\">21she</code>, <code style=\"color:#e11d48;\">500tk</code>, <code style=\"color:#e11d48;\">@today</code>, <code style=\"color:#e11d48;\">:smile</code>."
      "</span></p>"
      "</body></html>"
    );
    ui->lblShortcuts->setText(
      "<html><head/><body><p align=\"center\" style=\"margin:0;\">"
      "<span style=\"font-size:8.5pt; color:#64748b;\">⌨️ <b style=\"color:#0284c7;\">Shortcuts:</b>  "
      "<b style=\"color:#0f172a;\">Ctrl+Space</b> Toggle Bangla/English  •  "
      "<b style=\"color:#0f172a;\">Double Space</b> Dari (। )  •  "
      "<b style=\"color:#0f172a;\">Tab / Arrows</b> Select  •  "
      "<b style=\"color:#0f172a;\">1–9</b> Pick Snippet</span>"
      "</p></body></html>"
    );
    ui->lblDeveloper->setText(
      "<html><head/><body><p style=\"margin:0;\">"
      "<span style=\"font-size:9.5pt; color:#64748b;\">👨‍💻 Developer: </span>"
      "<span style=\"font-size:10pt; font-weight:700; color:#0f172a;\">Md Sojib Hossain</span>"
      "</p></body></html>"
    );
  }
}

void AboutDialog::on_lblFacebook_linkActivated(const QString &link) {
  QDesktopServices::openUrl(QUrl(link));
}

void AboutDialog::on_lblDonation_linkActivated(const QString &link) {
  QDesktopServices::openUrl(QUrl(link));
}

void AboutDialog::on_lblLicense_linkActivated(const QString &link) {
  QDesktopServices::openUrl(QUrl(link));
}
