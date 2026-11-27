/*
 *  MatraKi Keyboard
 *  Brand Mark — Onyx Design System
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
#include <QLinearGradient>
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

  const qreal dpr = qApp->devicePixelRatio() > 1.0 ? qApp->devicePixelRatio() : 1.0;
  QPixmap pm(qRound(side * dpr), qRound(side * dpr));
  pm.setDevicePixelRatio(dpr);
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
  const qreal radius = s * 0.28;

  if (ringWidth > 0 && ring.isValid()) {
    p.setPen(QPen(ring, ringWidth));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(box.adjusted(-ringWidth, -ringWidth, ringWidth, ringWidth),
                      radius + ringWidth, radius + ringWidth);
  }

  /* The mark is drawn as a miniature keycap: an anodised tile with a
     brushed top face, an ivory cap and the Bengali "ম" cut into it.
     Using the live palette keeps it in step with both themes. */
  QLinearGradient tile(box.topLeft(), box.bottomLeft());
  tile.setColorAt(0.0, brand.lighter(114));
  tile.setColorAt(1.0, brand.darker(116));

  p.setPen(QPen(brand.darker(150), qMax(1.0, s * 0.02)));
  p.setBrush(tile);
  p.drawRoundedRect(box, radius, radius);

  /* Key dimensions. */
  const qreal kw = s * 0.60;
  const qreal kh = s * 0.56;
  const qreal kx = box.left() + (s - kw) * 0.5;
  const qreal ky = box.top() + s * 0.21;
  const qreal kr = s * 0.13;

  /* Underside of the cap — the classic keyboard depth cue. */
  p.setPen(Qt::NoPen);
  p.setBrush(brand.darker(185));
  p.drawRoundedRect(QRectF(kx, ky + s * 0.055, kw, kh), kr, kr);

  /* Ivory cap face with a very soft vertical sheen. */
  QLinearGradient cap(QPointF(kx, ky), QPointF(kx, ky + kh));
  cap.setColorAt(0.0, QColor("#FFFDF7"));
  cap.setColorAt(1.0, QColor("#E9E2D3"));
  p.setBrush(cap);
  p.setPen(QPen(QColor(0, 0, 0, 26), qMax(1.0, s * 0.012)));
  p.drawRoundedRect(QRectF(kx, ky, kw, kh), kr, kr);

  /* Top-edge highlight. */
  p.setPen(QPen(QColor(255, 255, 255, 190), qMax(1.0, s * 0.018)));
  p.drawLine(QPointF(kx + kr, ky + s * 0.018),
             QPointF(kx + kw - kr, ky + s * 0.018));

  /* Bengali "ম" legend. */
  QFont font;
  font.setStyleHint(QFont::Serif);
  font.setFamilies({QStringLiteral("Tiro Bangla"), QStringLiteral("Noto Serif Bengali"),
                    QStringLiteral("Noto Sans Bengali"), QStringLiteral("Hind Siliguri"),
                    QStringLiteral("SolaimanLipi"), QStringLiteral("Kalpurush"),
                    QStringLiteral("Nirmala UI")});
  font.setPixelSize(qMax(8, qRound(s * 0.36)));
  font.setBold(true);
  p.setFont(font);
  p.setPen(brand.darker(108));
  p.drawText(QRectF(kx, ky - s * 0.01, kw, kh), Qt::AlignCenter, QStringLiteral("ম"));

  /* Signature dot, like the status lamp on a mechanical keyboard. */
  const qreal dotR = s * 0.036;
  p.setBrush(dot);
  p.setPen(QPen(QColor(255, 255, 255, 110), qMax(1.0, s * 0.012)));
  p.drawEllipse(QPointF(kx + kw - s * 0.085, ky + s * 0.095), dotR, dotR);
}

void MatraKiLogo::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);

  QPainter p(this);
  const int extra = (m_ringWidth > 0) ? (m_ringWidth * 2 + 4) : 0;
  paint(p, QRectF(extra, extra, m_size, m_size), m_brand, m_dot, m_ring, m_ringWidth);
}
