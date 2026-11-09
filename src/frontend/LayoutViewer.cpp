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
#include <zstd.h>
#include "LayoutViewer.h"
#include "Settings.h"
#include "AboutFile.h"
#include "base.hpp"
#include "ui_LayoutViewer.h"

LayoutViewer::LayoutViewer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LayoutViewer) {
  ui->setupUi(this);
  this->setWindowIcon(QIcon(":/images/MatraKi.png"));
  aboutDialog = new AboutFile(this);
  ui->labelImage->setAlignment(Qt::AlignCenter);
  this->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);
  QPoint savedPos = gSettings->getLayoutViewerWindowPosition();
  if (savedPos != QPoint(0, 0)) {
    this->move(savedPos);
  }
}

LayoutViewer::~LayoutViewer() {
  delete aboutDialog;
  delete ui;
}

void LayoutViewer::refreshLayoutViewer() {
  image0.clear();
  image1.clear();
  ui->viewAltGr->setEnabled(false);
  ui->viewNormal->setEnabled(false);
  ui->labelImage->setText("");

  desc = gLayout->getDesc();
  this->setWindowTitle("MatraKi - " + desc.name + " Layout Viewer");
  ui->lblHeaderTitle->setText("MatraKi Layout Viewer  •  " + desc.name);

  if (desc.name == "Avro Phonetic") {
    ui->viewNormal->setText("⌨️ Phonetic KeyMap");
    ui->viewAltGr->setText("✨ AI Smart Shortcuts");
    ui->viewNormal->setEnabled(true);
    ui->viewAltGr->setEnabled(true);
    on_viewNormal_clicked();
  } else {
    ui->viewNormal->setText("⌨️ Normal View");
    ui->viewAltGr->setText("⌨️ AltGr View");
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
  aboutDialog->setDialogType(AboutLayout);
}

void LayoutViewer::showLayoutInfoDialog() {
  aboutDialog->setDialogType(AboutLayout);
  aboutDialog->show();
}

void LayoutViewer::closeEvent(QCloseEvent *event) {
  gSettings->setLayoutViewerWindowPosition(this->pos());
  event->accept();
}

void LayoutViewer::on_buttonAboutLayout_clicked() {
  showLayoutInfoDialog();
}

void LayoutViewer::on_viewNormal_clicked() {
  if (desc.name == "Avro Phonetic") {
    QPixmap pix(":/images/avro_keymap_modern.png");
    ui->labelImage->setPixmap(pix);
    ui->labelImage->adjustSize();
    this->resize(pix.width() + 36, pix.height() + 98);
  } else {
    image.loadFromData(image0);
    ui->labelImage->setPixmap(QPixmap::fromImage(image));
    ui->labelImage->adjustSize();
    this->resize(qMax(680, image.width() + 36), image.height() + 98);
  }
  ui->viewNormal->setChecked(true);
}

void LayoutViewer::on_viewAltGr_clicked() {
  if (desc.name == "Avro Phonetic") {
    QPixmap pix(":/images/avro_smart_guide.png");
    ui->labelImage->setPixmap(pix);
    ui->labelImage->adjustSize();
    this->resize(pix.width() + 36, pix.height() + 98);
  } else {
    image.loadFromData(image1);
    ui->labelImage->setPixmap(QPixmap::fromImage(image));
    ui->labelImage->adjustSize();
    this->resize(qMax(680, image.width() + 36), image.height() + 98);
  }
  ui->viewAltGr->setChecked(true);
}

QByteArray LayoutViewer::decodeAndDecompress(QByteArray &data) {
  std::string decoded = base91::decode(std::string(data.data(), data.size()));
  unsigned long long cap = ZSTD_getDecompressedSize(decoded.data(), decoded.size());
  char *imgData = (char *)malloc(cap);

  size_t decompressed = ZSTD_decompress(imgData, cap, decoded.data(), decoded.size());
  QByteArray img = QByteArray(imgData, decompressed);
  free(imgData);

  return img;
}
