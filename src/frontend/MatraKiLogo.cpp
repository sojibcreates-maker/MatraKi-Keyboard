/*
 *  MatraKi Keyboard
 *  Brand Mark — Emerald Design System
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

#include "MatraKiLogo.h"

#include <QApplication>
#include <QFontMetricsF>
#include <QPainter>
#include <QPainterPath>

MatraKiLogo::MatraKiLogo(QWidget *parent) : QWidget(parent) {
  setAttribute(Qt::WA_TransparentForMouseEvents, true);
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  setMarkSize(m_size);
}

void MatraKiLogo::setMarkSize(int size) {
  m_size = qMax(12, size);
  const int extra = (m_ringWidth > 0) ? (m_ringWidth * 2 + 4) : 0;
  setFixedSize(m_size + extra, m_size + extra);
  update();
}

void MatraKiLogo::setBrandColor(const QColor &color) {
  if (m_brand == color) return;
  m_brand = color;
  update();
}

void MatraKiLogo::setDotColor(const QColor &color) {
  if (m_dot == color) return;
  m_dot = color;
  update();
}

void MatraKiLogo::setRingColor(const QColor &color) {
  if (m_ring == color) return;
  m_ring = color;
  update();
}

void MatraKiLogo::setRingWidth(int width) {
  m_ringWidth = qMax(0, width);
  const int extra = (m_ringWidth > 0) ? (m_ringWidth * 2 + 4) : 0;
  setFixedSize(m_size + extra, m_size + extra);
  update();
}

QSize MatraKiLogo::sizeHint() const {
  const int extra = (m_ringWidth > 0) ? (m_ringWidth * 2 + 4) : 0;
  return QSize(m_size + extra, m_size + extra);
}

QSize MatraKiLogo::minimumSizeHint() const {
  return sizeHint();
}

void MatraKiLogo::applyTo(QWidget *root, const QColor &brand, const QColor &dot) {
  if (!root) return;
  const QList<MatraKiLogo *> logos = root->findChildren<MatraKiLogo *>();
  for (MatraKiLogo *logo : logos) {
    logo->setBrandColor(brand);
    logo->setDotColor(dot);
  }
}

QPixmap MatraKiLogo::render(int size, const QColor &brand, const QColor &dot,
                             const QColor &ring, int ringWidth) {
  if (size <= 0) return QPixmap();

  const int extra = (ringWidth > 0) ? (ringWidth * 2 + 4) : 0;
  const int side = size + extra;

  QPixmap pm(side, side);
  pm.setDevicePixelRatio(qApp->devicePixelRatio() > 1.0 ? qApp->devicePixelRatio() : 1.0);
  pm.fill(Qt::transparent);

  QPainter p(&pm);
  p.setRenderHint(QPainter::Antialiasing, true);
  p.setRenderHint(QPainter::TextAntialiasing, true);
  paint(p, QRectF(extra, extra, size, size), brand, dot, ring, ringWidth);
  p.end();

  return pm;
}

void MatraKiLogo::paint(QPainter &p, const QRectF &box, const QColor &brand, const QColor &dot,
                         const QColor &ring, int ringWidth) {
  p.setRenderHint(QPainter::Antialiasing, true);
  p.setRenderHint(QPainter::SmoothPixmapTransform, true);
  p.setRenderHint(QPainter::TextAntialiasing, true);

  const qreal s = box.width();
  const qreal radius = s * 0.30;

  if (ringWidth > 0 && ring.isValid()) {
    p.setPen(QPen(ring, ringWidth));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(box.adjusted(-ringWidth, -ringWidth, ringWidth, ringWidth),
                      radius + ringWidth, radius + ringWidth);
  }

  QPixmap master(QStringLiteral(":/images/MatraKi.png"));
  if (!master.isNull()) {
    p.drawPixmap(box.toRect(), master.scaled(box.size().toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    return;
  }

  /* Fallback: keycap vector drawing matching official brand icon */
  /* 1. Rounded green tile */
  p.setPen(Qt::NoPen);
  p.setBrush(brand);
  p.drawRoundedRect(box, radius, radius);

  /* 2. Key dimensions */
  const qreal kw = s * 0.63;
  const qreal kh = s * 0.59;
  const qreal kx = box.left() + (s - kw) * 0.5;
  const qreal ky = box.top() + s * 0.18;
  const qreal kr = s * 0.16;

  /* 3. Mint shadow key */
  p.setBrush(QColor(184, 213, 201));
  p.drawRoundedRect(QRectF(kx, ky + s * 0.05, kw, kh), kr, kr);

  /* 4. White keycap surface */
  p.setBrush(Qt::white);
  p.drawRoundedRect(QRectF(kx, ky, kw, kh), kr, kr);

  /* 5. Bengali "ম" mark in brand green */
  QFont font;
  font.setStyleHint(QFont::SansSerif);
  font.setFamilies({QStringLiteral("Noto Sans Bengali"), QStringLiteral("Hind Siliguri"),
                    QStringLiteral("SolaimanLipi"), QStringLiteral("Kalpurush"),
                    QStringLiteral("Nirmala UI"), QStringLiteral("Segoe UI")});
  font.setPixelSize(qMax(8, qRound(s * 0.34)));
  font.setBold(true);
  p.setFont(font);
  p.setPen(brand);
  p.drawText(QRectF(kx, ky, kw, kh), Qt::AlignCenter, QStringLiteral("ম"));

  /* 6. Signature red status dot on keycap */
  const qreal dotR = s * 0.035;
  p.setBrush(dot);
  p.setPen(Qt::NoPen);
  p.drawEllipse(QPointF(kx + kw - s * 0.10, ky + s * 0.10), dotR, dotR);
}

void MatraKiLogo::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);

  QPainter p(this);
  const int extra = (m_ringWidth > 0) ? (m_ringWidth * 2 + 4) : 0;
  paint(p, QRectF(extra, extra, m_size, m_size), m_brand, m_dot, m_ring, m_ringWidth);
}
