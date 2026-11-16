/*
 *  MatraKi Keyboard
 *  Emerald Menu — icon rows, trailing meta and inline switches
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

#ifndef MATRAKIMENU_H
#define MATRAKIMENU_H

#include <QMenu>
#include <QWidget>

class MatraKiToggle;

/**
 * A single row of the Emerald menu.
 *
 * The row paints itself: icon on the left, label in the middle and an
 * optional trailing element (muted caption or a switch) on the right.
 */
class MatraKiMenuRow : public QWidget {
  Q_OBJECT

public:
  explicit MatraKiMenuRow(QWidget *parent = nullptr);

  void setIcon(const QPixmap &pixmap);
  void setLabel(const QString &label);
  void setMeta(const QString &meta);
  void setTrailingWidget(QWidget *widget);

  void setCheckable(bool checkable);
  void setChecked(bool checked);
  bool isChecked() const { return m_checked; }

  void setRowWidth(int width);

signals:
  void clicked();

protected:
  void paintEvent(QPaintEvent *event) override;
  void enterEvent(QEvent *event) override;
  void leaveEvent(QEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;

private:
  QWidget *m_iconHolder = nullptr;
  QWidget *m_labelHolder = nullptr;
  QWidget *m_metaHolder = nullptr;
  QWidget *m_trailing = nullptr;
  bool m_hovered = false;
  bool m_checkable = false;
  bool m_checked = false;
  int m_rowWidth = 284;
};

/**
 * MatraKiMenu builds the Emerald popup menus used by the floating toolbar.
 */
class MatraKiMenu : public QMenu {
  Q_OBJECT

public:
  explicit MatraKiMenu(const QString &title, QWidget *parent = nullptr);

  /* Add a row with an icon, a label and an optional muted trailing caption. */
  QAction *addRow(const QString &label, const QPixmap &icon, const QString &meta = QString());

  /* Add a checkable row (used by the layout picker). */
  QAction *addCheckableRow(const QString &label, const QPixmap &icon);

  /* Add a row that embeds a switch on the right hand side. */
  QAction *addToggleRow(const QString &label, const QPixmap &icon, MatraKiToggle *toggle);

  /* Add a hairline separator between groups. */
  void addSeparatorRow();

  /* Remove every row (and its backing action) from the menu. */
  void clearRows();

  void setRowWidth(int width);

protected:
  void keyPressEvent(QKeyEvent *event) override;

private:
  int m_rowWidth = 284;
  QList<MatraKiMenuRow *> m_rows;
};

#endif // MATRAKIMENU_H
