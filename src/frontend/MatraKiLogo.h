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

#ifndef MATRAKILOGO_H
#define MATRAKILOGO_H

#include <QColor>
#include <QPixmap>
#include <QWidget>

class QWidget;

/**
 * MatraKiLogo draws the MatraKi brand mark: a rounded green square holding
 * the Bengali letter "ম" with the signature red status dot in the top right
 * corner. It is resolution independent so the exact same mark can be used at
 * 24 px inside a dialog header and at 74 px inside the About page.
 */
class MatraKiLogo : public QWidget {
  Q_OBJECT

public:
  explicit MatraKiLogo(QWidget *parent = nullptr);

  /* Fixed size of the mark in device independent pixels. */
  void setMarkSize(int size);
  int markSize() const { return m_size; }

  void setBrandColor(const QColor &color);
  QColor brandColor() const { return m_brand; }

  void setDotColor(const QColor &color);
  QColor dotColor() const { return m_dot; }

  /* Optional thin ring around the mark — used for the focused toolbar logo. */
  void setRingColor(const QColor &color);
  void setRingWidth(int width);

  QSize sizeHint() const override;
  QSize minimumSizeHint() const override;

  /* Re-colour every brand mark found below @root. */
  static void applyTo(QWidget *root, const QColor &brand, const QColor &dot);

  /**
   * Render the brand mark into a pixmap. Handy for buttons and labels that
   * only accept a pixmap.
   */
  static QPixmap render(int size, const QColor &brand, const QColor &dot,
                        const QColor &ring = QColor(), int ringWidth = 0);

  /* Paint the mark into @target (the rounded square area). */
  static void paint(QPainter &painter, const QRectF &target, const QColor &brand,
                    const QColor &dot, const QColor &ring = QColor(), int ringWidth = 0);

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  int m_size = 36;
  QColor m_brand = QColor("#17835F");
  QColor m_dot = QColor("#F42A41");
  QColor m_ring;
  int m_ringWidth = 0;
};

#endif // MATRAKILOGO_H
