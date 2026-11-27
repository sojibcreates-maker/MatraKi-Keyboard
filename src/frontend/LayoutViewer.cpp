/*
 *  MatraKi Keyboard
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

#include <QCloseEvent>
#include <QGuiApplication>
#include <QScreen>
#include <QShowEvent>
#include <zstd.h>
#include <vector>
#include "LayoutViewer.h"
#include "Settings.h"
#include "AboutLayoutDialog.h"
#include "MatraKiIcons.h"
#include "MatraKiLogo.h"
#include "base.hpp"
#include "ui_LayoutViewer.h"
#include "MatraKiWindowChrome.h"
#include "ThemeManager.h"

LayoutViewer::LayoutViewer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LayoutViewer) {
  ui->setupUi(this);
  this->setWindowIcon(QIcon(":/images/MatraKi.png"));
  aboutDialog = new AboutLayoutDialog(this);
  ui->labelImage->setAlignment(Qt::AlignCenter);
  this->setAttribute(Qt::WA_TranslucentBackground, true);
  makeOnyxDialog(this);
  ui->btnCloseIcon->setObjectName("iconOnly");
  QPoint savedPos = gSettings->getLayoutViewerWindowPosition();
  if (savedPos != QPoint(0, 0)) {
    this->move(savedPos);
  }

  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &LayoutViewer::applyTheme);
  applyTheme();
}

LayoutViewer::~LayoutViewer() {
  delete aboutDialog;
  delete ui;
}

void LayoutViewer::applyTheme() {
  const MatraKiPalette p = ThemeManager::instance().palette();

  setStyleSheet(ThemeManager::instance().layoutViewerStyle());

  ui->btnCloseIcon->setIcon(QIcon(MatraKiIcons::pixmap(MatraKiIcons::Close, p.icon, 16)));
  ui->logoSlot->setPixmap(MatraKiLogo::render(34, p.brand, p.danger));
  ui->viewNormal->setIcon(QIcon(MatraKiIcons::pixmap(MatraKiIcons::Keyboard, p.accent, 16)));
  ui->viewAltGr->setIcon(QIcon(MatraKiIcons::pixmap(MatraKiIcons::Sparkles, p.accent, 16)));
  ui->buttonAboutLayout->setObjectName("primary");
  ui->buttonAboutLayout->setIcon(QIcon(MatraKiIcons::pixmap(MatraKiIcons::Info, p.onBrand, 16)));

  if (desc.name == "Avro Phonetic") {
    if (ui->viewAltGr->isChecked()) {
      on_viewAltGr_clicked();
    } else {
      on_viewNormal_clicked();
    }
  }
}

void LayoutViewer::showEvent(QShowEvent *event) {
  QDialog::showEvent(event);
  applyTheme();
  /* Opening the viewer directly (preview builds, first run) must still
     show the current layout instead of an empty stage. */
  if (desc.name.isEmpty() && image0.isEmpty() && image.isNull()) {
    refreshLayoutViewer();
  }
}

void LayoutViewer::refreshLayoutViewer() {
  if (!gLayout) return;

  image0.clear();
  image1.clear();
  ui->viewAltGr->setEnabled(false);
  ui->viewNormal->setEnabled(false);
  ui->labelImage->setText("");

  desc = gLayout->getDesc();
  this->setWindowTitle("MatraKi - " + desc.name + " Layout Viewer");
  ui->lblHeaderTitle->setText("MatraKi Layout Viewer  •  " + desc.name);

  if (desc.name == "Avro Phonetic") {
    ui->viewNormal->setText("Phonetic KeyMap");
    ui->viewAltGr->setText("AI Smart Shortcuts");
    ui->viewNormal->setEnabled(true);
    ui->viewAltGr->setEnabled(true);
    on_viewNormal_clicked();
  } else {
    ui->viewNormal->setText("Normal View");
    ui->viewAltGr->setText("AltGr View");
    if (desc.image0.size() != 0) {
      image0 = decodeAndDecompress(desc.image0);
      if (desc.image1.size() != 0) {
        image1 = decodeAndDecompress(desc.image1);
        ui->viewAltGr->setEnabled(true);
      }
      ui->viewNormal->setEnabled(true);
      on_viewNormal_clicked();
    } else {
      ui->labelImage->setText("No layout preview image available.");
      this->resize(560, 200);
    }
  }

  // This refreshes Layout Info Dialog
  aboutDialog->refresh();
}

void LayoutViewer::showLayoutInfoDialog() {
  aboutDialog->refresh();
  aboutDialog->show();
}

void LayoutViewer::closeEvent(QCloseEvent *event) {
  gSettings->setLayoutViewerWindowPosition(this->pos());
  event->accept();
}

void LayoutViewer::on_buttonAboutLayout_clicked() {
  showLayoutInfoDialog();
}

void LayoutViewer::setChart(const QPixmap &source) {
  if (source.isNull()) {
    ui->labelImage->setText("No layout preview image available.");
    return;
  }

  /* Keep the whole chart reachable on short screens (laptops, 720p) by
     scaling it down instead of clipping the bottom rows. */
  const QScreen *target = screen() ? screen() : QGuiApplication::primaryScreen();
  QPixmap pix = source;
  if (target) {
    const QRect avail = target->availableGeometry();
    const int maxW = qMax(640, avail.width() - 80);
    const int maxH = qMax(360, avail.height() - 190);
    if (pix.width() > maxW || pix.height() > maxH) {
      pix = pix.scaled(maxW, maxH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
  }

  ui->labelImage->setPixmap(pix);
  ui->labelImage->adjustSize();
  this->resize(qMax(680, pix.width() + 44), pix.height() + 112);
}

void LayoutViewer::on_viewNormal_clicked() {
  if (desc.name == "Avro Phonetic") {
    const bool dark = ThemeManager::instance().isDark();
    const QString imgPath = dark ? QStringLiteral(":/images/avro_keymap_dark.png")
                                 : QStringLiteral(":/images/avro_keymap_light.png");
    setChart(QPixmap(imgPath));
  } else if (image.loadFromData(image0)) {
    setChart(QPixmap::fromImage(image));
  }
  ui->viewNormal->setChecked(true);
}

void LayoutViewer::on_viewAltGr_clicked() {
  if (desc.name == "Avro Phonetic") {
    const bool dark = ThemeManager::instance().isDark();
    const QString imgPath = dark ? QStringLiteral(":/images/avro_smart_guide_dark.png")
                                 : QStringLiteral(":/images/avro_smart_guide_light.png");
    setChart(QPixmap(imgPath));
  } else if (image.loadFromData(image1)) {
    setChart(QPixmap::fromImage(image));
  }
  ui->viewAltGr->setChecked(true);
}

QByteArray LayoutViewer::decodeAndDecompress(QByteArray &data) {
  const std::string decoded = base91::decode(std::string(data.data(), data.size()));
  if (decoded.empty()) return QByteArray();

  const unsigned long long cap = ZSTD_getFrameContentSize(decoded.data(), decoded.size());
  if (cap == ZSTD_CONTENTSIZE_ERROR || cap == ZSTD_CONTENTSIZE_UNKNOWN || cap == 0) {
    return QByteArray();
  }

  std::vector<char> buffer(static_cast<size_t>(cap));
  const size_t decompressed = ZSTD_decompress(buffer.data(), buffer.size(),
                                              decoded.data(), decoded.size());
  if (ZSTD_isError(decompressed)) return QByteArray();

  return QByteArray(buffer.data(), static_cast<int>(decompressed));
}

void LayoutViewer::on_btnCloseIcon_clicked() {
  close();
}
