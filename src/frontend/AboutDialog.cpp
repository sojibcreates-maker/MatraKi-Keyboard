/*
 *  MatraKi Keyboard
 *  About — Emerald Design System
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

#include <QDesktopServices>
#include <QShowEvent>
#include <QUrl>

#include "AboutDialog.h"
#include "ui_AboutDialog.h"
#include "MatraKiLogo.h"
#include "ThemeManager.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog) {
  ui->setupUi(this);

  this->setAttribute(Qt::WA_TranslucentBackground, true);
  this->setWindowIcon(QIcon(":/images/MatraKi.png"));

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

  ui->logoSlot->setPixmap(MatraKiLogo::render(58, p.brand, p.danger));

  /* ---- Title block ---- */
  ui->labelTitle->setText(
      QStringLiteral("<span style=\"font-size:21px; font-weight:700; color:%1;\">MatraKi</span>")
          .arg(p.text));
  ui->labelTagline->setText(
      QStringLiteral("<span style=\"font-size:13px; font-weight:600; color:%1;\">AI Smart Bangla Keyboard</span>")
          .arg(p.accent));
  ui->labelVer->setText(
      QStringLiteral("<span style=\"font-size:11px; color:%1;\">"
                     "✨ v1.1.0 (AI Smart Edition) &nbsp;•&nbsp; 38,000+ Trained Words &nbsp;•&nbsp; "
                     "Harper AI Grammar &nbsp;•&nbsp; Light &amp; Dark Mode</span>")
          .arg(p.textMuted));

  /* ---- Feature list ---- */
  ui->labelDesc->setText(
      QStringLiteral(
          "<div style=\"margin:0 0 8px 0;\"><span style=\"font-size:12px; color:%2;\">"
          "<b style=\"color:%1;\">MatraKi</b> একটি উন্নত, অত্যন্ত দ্রুত বাংলা ইনপুট মেথড — "
          "লিনাক্স ডেস্কটপের জন্য তৈরি। দ্রুত রাস্ট ফনেটিক ইঞ্জিন, হার্পার AI ব্যাকরণ যাচাই এবং "
          "বুদ্ধিমান C++ ভাষা স্তরের সমন্বয়ে বাংলা লেখা হয় দ্রুত, সহজ ও নির্ভুল।</span></div>"
          "<div style=\"margin:6px 0 2px 0;\"><span style=\"font-size:12px; font-weight:700; color:%1;\">"
          "🧠 AI ফনেটিক ও ভাষা বুদ্ধিমত্তা:</span></div>"
          "<div style=\"margin:0 0 6px 10px;\"><span style=\"font-size:11px; color:%2;\">"
          "• <b style=\"color:%3;\">বিশাল শব্দভাণ্ডার ও সাফিক্স ইঞ্জিন:</b> "
          "<b style=\"color:%1;\">38,000+</b> প্রশিক্ষিত বাংলা শব্দ, <b style=\"color:%1;\">20,535</b> "
          "অটো-কারেক্ট প্যাটার্ন এবং স্মার্ট সাফিক্স স্টেমিং "
          "(<i>-গুলো, -দের, -টা, -টি, -কে, -তে</i>)।<br/>"
          "• <b style=\"color:%3;\">হার্পার AI ব্যাকরণ ও বানান যাচাই:</b> রিয়েল-টাইম কনটেক্সচুয়াল "
          "ব্যাকরণ যাচাই, নত্ত্ববিদ্ধি ও শত্ত্ববিদ্ধি সঠিকতা।<br/>"
          "• <b style=\"color:%3;\">কনটেক্স্ট-অ্যাওয়ার প্রিডিকশন:</b> স্পেস চাপার পরেই প্রাকৃতিক "
          "পরবর্তী শব্দ সাজেস্ট করে এবং কর্তা-সম্মত ক্রিয়ার রূপ বাছাই করে "
          "(<i>আমি করব, তুমি করবে, আপনি করবেন</i>)।</span></div>"
          "<div style=\"margin:6px 0 2px 0;\"><span style=\"font-size:12px; font-weight:700; color:%4;\">"
          "⚡ স্মার্ট প্রোডাক্টিভিটি ও তাৎক্ষণিক রূপান্তর:</span></div>"
          "<div style=\"margin:0 0 2px 10px;\"><span style=\"font-size:11px; color:%2;\">"
          "• <b style=\"color:%3;\">লাইভ ক্যালকুলেটর ও ইউনিট কনভার্টার (<span style=\"color:%1;\">=</span>):</b> "
          "গণিত (<code style=\"color:%1;\">=500*15%</code>, <code style=\"color:%1;\">=2^10</code>) ও "
          "একক (<code style=\"color:%1;\">=10km</code>, <code style=\"color:%1;\">=5kg</code>, "
          "<code style=\"color:%1;\">=100c</code>, <code style=\"color:%1;\">=1bigha</code>)।<br/>"
          "• <b style=\"color:%3;\">ইনস্ট্যান্ট চ্যাট ও ইমেইল টেমপ্লেট (<span style=\"color:%4;\">!</span>):</b> "
          "অভিবাদন (<code style=\"color:%4;\">!salam</code>, <code style=\"color:%4;\">!shuvo</code>) ও "
          "টেমপ্লেট (<code style=\"color:%4;\">!jobbn</code>, <code style=\"color:%4;\">!cvbn</code>)।<br/>"
          "• <b style=\"color:%3;\">স্মার্ট তারিখ, কারেন্সি ও ইমোজি:</b> "
          "<code style=\"color:%4;\">1la</code>, <code style=\"color:%4;\">21she</code>, "
          "<code style=\"color:%4;\">500tk</code>, <code style=\"color:%4;\">@today</code>, "
          "<code style=\"color:%4;\">:smile</code>।</span></div>")
          .arg(p.accent, p.textMuted, p.text, p.danger));

  /* ---- Shortcuts ---- */
  ui->lblShortcuts->setText(
      QStringLiteral("<div align=\"center\">"
                     "<span style=\"font-size:11px; color:%1;\">⌨️ <b style=\"color:%2;\">শর্টকাট:</b> &nbsp; "
                     "<b style=\"color:%2;\">Ctrl+Space</b> বাংলা/ইংরেজি &nbsp;•&nbsp; "
                     "<b style=\"color:%2;\">Double Space</b> দাড়ি (। ) &nbsp;•&nbsp; "
                     "<b style=\"color:%2;\">Tab / Arrows</b> সিলেক্ট &nbsp;•&nbsp; "
                     "<b style=\"color:%2;\">1–9</b> স্নিপেট</span></div>")
          .arg(p.textMuted, p.accent));

  /* ---- Developer ---- */
  ui->lblDeveloper->setText(
      QStringLiteral("<span style=\"font-size:12px; color:%1;\">👨‍💻 ডেভেলপার: </span>"
                     "<span style=\"font-size:12px; font-weight:700; color:%2;\">Md Sojib Hossain</span>")
          .arg(p.textMuted, p.text));
  ui->lblFacebook->setText(
      QStringLiteral("<a href=\"https://facebook.com/hossain.sojibs\">"
                     "<span style=\"font-size:12px; font-weight:600; color:%1; text-decoration:none;\">"
                     "🌐 ফেসবুক: facebook.com/hossain.sojibs</span></a>")
          .arg(p.accent));

  /* ---- Donation ---- */
  ui->lblDonation->setText(
      QStringLiteral("<a href=\"https://www.supportkori.com/sojib\">"
                     "<span style=\"font-size:12px; font-weight:700; color:%1; text-decoration:none;\">"
                     "💖 মাত্রা-কী-কে সহায়তা করুন: supportkori.com/sojib (ক্লিক করে অনুবাদিতে টাকা পাঠান)</span></a>")
          .arg(p.danger));

  /* ---- Licence ---- */
  ui->lblLicense->setText(
      QStringLiteral("<span style=\"font-size:11px; color:%1;\">ওপেন-সোর্স সফটওয়্যার — "
                     "<a href=\"https://www.gnu.org/licenses/gpl-3.0.html\">"
                     "<span style=\"color:%2; text-decoration:underline;\">GNU General Public License v3.0</span></a>"
                     " &nbsp;•&nbsp; বাংলা কম্পিউটিংয়ের প্রেমে তৈরি।</span>")
          .arg(p.textMuted, p.accent));
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
