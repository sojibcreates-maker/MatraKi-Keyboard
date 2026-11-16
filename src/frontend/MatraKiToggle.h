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

#ifndef MATRAKITOGGLE_H
#define MATRAKITOGGLE_H

#include <QColor>
#include <QWidget>

/**
 * MatraKiToggle is the pill switch used across the redesigned interface —
 * inside the logo menu for the Harper grammar toggle and inside the
 * settings page for every on/off option.
 */
class MatraKiToggle : public QWidget {
  Q_OBJECT
  Q_PROPERTY(bool checked READ isChecked WRITE setChecked NOTIFY toggled)

public:
  explicit MatraKiToggle(QWidget *parent = nullptr);

  bool isChecked() const { return m_checked; }

  /* Track colours follow the active theme. */
  void setOnColor(const QColor &color) { m_on = color; update(); }
  void setOffColor(const QColor &color) { m_off = color; update(); }
  void setFocusColor(const QColor &color) { m_focus = color; update(); }

public slots:
  void setChecked(bool checked);
  void toggle() { setChecked(!m_checked); }

signals:
  void toggled(bool checked);

protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;

private:
  bool m_checked = false;
  int m_trackW = 38;
  int m_trackH = 22;
  QColor m_on = QColor("#17835F");
  QColor m_off = QColor("#2E4038");
  QColor m_focus = QColor("#5FD0A6");
};

#endif // MATRAKITOGGLE_H
