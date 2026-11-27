/*
 *  MatraKi Keyboard
 *  Onyx Menu — icon rows, trailing meta and inline switches
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

#include "MatraKiMenu.h"

#include <QApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QHBoxLayout>
#include <QWidgetAction>

#include "MatraKiIcons.h"
#include "MatraKiToggle.h"
#include "ThemeManager.h"

namespace {

QColor paletteColor(const QString &token) {
  const MatraKiPalette p = ThemeManager::instance().palette();
  if (token == "text") return p.text;
  if (token == "muted") return p.textMuted;
  if (token == "icon") return p.icon;
  if (token == "accent") return p.accent;
  if (token == "accentSoft") return p.accentSoft;
  if (token == "surface") return p.surface;
  if (token == "border") return p.border;
  if (token == "danger") return p.danger;
  return p.text;
}

QLabel *ensureLabel(QWidget *host, int pixelSize, Qt::Alignment align) {
  QLabel *label = host->findChild<QLabel *>();
  if (!label) {
    label = new QLabel(host);
    QHBoxLayout *box = new QHBoxLayout();
    box->setContentsMargins(0, 0, 0, 0);
    box->addWidget(label);
    host->setLayout(box);
  }
  label->setAlignment(align);
  QFont f(label->font());
  f.setPixelSize(pixelSize);
  label->setFont(f);
  return label;
}

} // namespace

/* ------------------------------------------------------------------ */
/*  MatraKiMenuRow                                                    */
/* ------------------------------------------------------------------ */

MatraKiMenuRow::MatraKiMenuRow(QWidget *parent) : QWidget(parent) {
  setAttribute(Qt::WA_TranslucentBackground, true);
  setFocusPolicy(Qt::StrongFocus);
  setCursor(Qt::PointingHandCursor);

  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setContentsMargins(10, 0, 10, 0);
  layout->setSpacing(10);

  m_iconHolder = new QWidget(this);
  m_iconHolder->setAttribute(Qt::WA_TransparentForMouseEvents, true);
  m_iconHolder->setFixedWidth(20);
  m_iconHolder->setFixedHeight(20);
  layout->addWidget(m_iconHolder, 0, Qt::AlignVCenter);

  m_labelHolder = new QWidget(this);
  m_labelHolder->setAttribute(Qt::WA_TransparentForMouseEvents, true);
  layout->addWidget(m_labelHolder, 1, Qt::AlignVCenter);

  m_metaHolder = new QWidget(this);
  m_metaHolder->setAttribute(Qt::WA_TransparentForMouseEvents, true);
  m_metaHolder->setFixedWidth(0);
  layout->addWidget(m_metaHolder, 0, Qt::AlignVCenter);

  setFixedHeight(38);
}

void MatraKiMenuRow::setIcon(const QPixmap &pixmap) {
  QLabel *label = ensureLabel(m_iconHolder, 14, Qt::AlignCenter);
  label->setPixmap(pixmap);
  label->setFixedSize(20, 20);
  label->setScaledContents(false);
}

void MatraKiMenuRow::setLabel(const QString &text) {
  QLabel *label = ensureLabel(m_labelHolder, 14, Qt::AlignLeft | Qt::AlignVCenter);
  label->setText(text);
  label->setStyleSheet(QStringLiteral("color: %1; background: transparent;")
                           .arg(paletteColor("text").name()));
}

void MatraKiMenuRow::setMeta(const QString &meta) {
  if (meta.isEmpty()) {
    m_metaHolder->setFixedWidth(0);
    return;
  }
  QLabel *label = ensureLabel(m_metaHolder, 12, Qt::AlignRight | Qt::AlignVCenter);
  label->setText(meta);
  label->setStyleSheet(QStringLiteral("color: %1; background: transparent;")
                           .arg(paletteColor("muted").name()));
  m_metaHolder->setFixedWidth(label->fontMetrics().horizontalAdvance(meta) + 2);
}

void MatraKiMenuRow::setTrailingWidget(QWidget *widget) {
  if (m_trailing) {
    m_trailing->setParent(nullptr);
    m_trailing = nullptr;
  }
  if (!widget) return;
  m_trailing = widget;
  widget->setParent(this);
  QHBoxLayout *layout = qobject_cast<QHBoxLayout *>(this->layout());
  if (layout) layout->addWidget(widget, 0, Qt::AlignVCenter);
}

void MatraKiMenuRow::setCheckable(bool checkable) {
  m_checkable = checkable;
  update();
}

void MatraKiMenuRow::setChecked(bool checked) {
  if (m_checked == checked) return;
  m_checked = checked;
  update();
}

void MatraKiMenuRow::setRowWidth(int width) {
  m_rowWidth = width;
  setFixedWidth(width);
}

void MatraKiMenuRow::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);

  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing, true);

  const QRectF box = QRectF(this->rect()).adjusted(0, 1, 0, -1);
  const bool active = m_hovered || hasFocus();

  if (active) {
    p.setPen(Qt::NoPen);
    p.setBrush(paletteColor("accentSoft"));
    p.drawRoundedRect(box, 9, 9);
  }

  QLabel *label = m_labelHolder->findChild<QLabel *>();
  if (label) {
    label->setStyleSheet(QStringLiteral("color: %1; background: transparent;")
                             .arg(paletteColor(active ? "accent" : "text").name()));
  }

  /* Check mark for the active layout. */
  if (m_checkable && m_checked) {
    p.setPen(QPen(paletteColor("accent"), 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.setBrush(Qt::NoBrush);
    QPainterPath path;
    path.moveTo(3, 8);
    path.lineTo(6.5, 11.5);
    path.lineTo(13, 3);
    p.drawPath(path.translated(m_metaHolder->x() - 20, m_metaHolder->y() + 9));
  }
}

void MatraKiMenuRow::enterEvent(QEvent *event) {
  m_hovered = true;
  update();
  QWidget::enterEvent(event);
}

void MatraKiMenuRow::leaveEvent(QEvent *event) {
  m_hovered = false;
  update();
  QWidget::leaveEvent(event);
}

void MatraKiMenuRow::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    emit clicked();
    event->accept();
    return;
  }
  QWidget::mouseReleaseEvent(event);
}

void MatraKiMenuRow::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter ||
      event->key() == Qt::Key_Space) {
    emit clicked();
    event->accept();
    return;
  }
  QWidget::keyPressEvent(event);
}

/* ------------------------------------------------------------------ */
/*  MatraKiMenu                                                       */
/* ------------------------------------------------------------------ */

MatraKiMenu::MatraKiMenu(const QString &title, QWidget *parent) : QMenu(title, parent) {
  setAttribute(Qt::WA_TranslucentBackground, true);
}

void MatraKiMenu::setRowWidth(int width) {
  m_rowWidth = width;
  for (MatraKiMenuRow *row : m_rows) row->setRowWidth(width);
}

QAction *MatraKiMenu::addRow(const QString &label, const QPixmap &icon, const QString &meta) {
  MatraKiMenuRow *row = new MatraKiMenuRow(this);
  row->setRowWidth(m_rowWidth);
  row->setIcon(icon);
  row->setLabel(label);
  row->setMeta(meta);

  QWidgetAction *action = new QWidgetAction(this);
  action->setDefaultWidget(row);
  addAction(action);

  connect(row, &MatraKiMenuRow::clicked, this, [this, action]() {
    action->trigger();
    close();
  });

  m_rows.append(row);
  return action;
}

QAction *MatraKiMenu::addCheckableRow(const QString &label, const QPixmap &icon) {
  MatraKiMenuRow *row = new MatraKiMenuRow(this);
  row->setRowWidth(m_rowWidth);
  row->setIcon(icon);
  row->setLabel(label);
  row->setCheckable(true);

  QWidgetAction *action = new QWidgetAction(this);
  action->setDefaultWidget(row);
  action->setCheckable(true);
  addAction(action);

  connect(row, &MatraKiMenuRow::clicked, this, [this, action, row]() {
    action->setChecked(!action->isChecked());
    row->setChecked(action->isChecked());
    action->trigger();
    close();
  });
  connect(action, &QAction::changed, this, [row, action]() {
    row->setChecked(action->isChecked());
  });

  m_rows.append(row);
  return action;
}

QAction *MatraKiMenu::addToggleRow(const QString &label, const QPixmap &icon,
                                    MatraKiToggle *toggle) {
  MatraKiMenuRow *row = new MatraKiMenuRow(this);
  row->setRowWidth(m_rowWidth);
  row->setIcon(icon);
  row->setLabel(label);
  row->setTrailingWidget(toggle);

  QWidgetAction *action = new QWidgetAction(this);
  action->setDefaultWidget(row);
  addAction(action);

  /* Clicking the label toggles the switch, clicking the switch itself is
     handled by the switch widget. */
  connect(row, &MatraKiMenuRow::clicked, this, [toggle]() {
    toggle->toggle();
  });

  m_rows.append(row);
  return action;
}

void MatraKiMenu::addSeparatorRow() {
  /* A plain separator action lets QMenu paint the hairline with the
     "QMenu::separator" rule of the active theme. */
  QAction *separator = addAction(QString());
  separator->setSeparator(true);
}

void MatraKiMenu::clearRows() {
  const QList<QAction *> actions = this->actions();
  for (QAction *action : actions) {
    removeAction(action);
    delete action;
  }
  m_rows.clear();
}

void MatraKiMenu::keyPressEvent(QKeyEvent *event) {
  if ((event->key() == Qt::Key_Down || event->key() == Qt::Key_Up) && !m_rows.isEmpty()) {
    int index = -1;
    for (int i = 0; i < m_rows.size(); ++i) {
      if (m_rows.at(i) == focusWidget()) {
        index = i;
        break;
      }
    }
    const int delta = (event->key() == Qt::Key_Down) ? 1 : -1;
    int next = index + delta;
    if (index < 0) next = (delta > 0) ? 0 : m_rows.size() - 1;
    if (next < 0) next = m_rows.size() - 1;
    if (next >= m_rows.size()) next = 0;
    m_rows.at(next)->setFocus();
    event->accept();
    return;
  }
  QMenu::keyPressEvent(event);
}
