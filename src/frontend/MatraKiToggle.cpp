/*
 *  MatraKi Keyboard
 *  Switch Control — Emerald Design System
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

#include "MatraKiToggle.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>

MatraKiToggle::MatraKiToggle(QWidget *parent) : QWidget(parent) {
  setCursor(Qt::PointingHandCursor);
  setFocusPolicy(Qt::StrongFocus);
  setAttribute(Qt::WA_TranslucentBackground);
  setFixedSize(m_trackW, m_trackH);
}

void MatraKiToggle::setChecked(bool checked) {
  if (m_checked == checked) return;
  m_checked = checked;
  update();
  emit toggled(m_checked);
}

void MatraKiToggle::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);

  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing, true);

  const QRectF track(0, 0, m_trackW, m_trackH);
  const qreal r = m_trackH / 2.0;

  p.setPen(Qt::NoPen);
  p.setBrush(m_checked ? m_on : m_off);
  p.drawRoundedRect(track, r, r);

  if (hasFocus()) {
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(m_focus, 1.6));
    p.drawRoundedRect(track.adjusted(-2.4, -2.4, 2.4, 2.4), r + 2.4, r + 2.4);
  }

  const qreal knob = m_trackH - 6.0;
  const qreal inset = 3.0;
  const qreal x = m_checked ? (m_trackW - knob - inset) : inset;
  p.setBrush(QColor("#FFFFFF"));
  p.setPen(Qt::NoPen);
  p.drawEllipse(QRectF(x, inset, knob, knob));
}

void MatraKiToggle::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    toggle();
    event->accept();
    return;
  }
  QWidget::mousePressEvent(event);
}

void MatraKiToggle::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Return ||
      event->key() == Qt::Key_Enter) {
    toggle();
    event->accept();
    return;
  }
  QWidget::keyPressEvent(event);
}
