/*
 *  MatraKi Keyboard
 *  Frameless dialog chrome — Onyx Design System
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

#ifndef MATRAKIWINDOWCHROME_H
#define MATRAKIWINDOWCHROME_H

#include <QAbstractButton>
#include <QApplication>
#include <QDialog>
#include <QEvent>
#include <QMouseEvent>
#include <QPoint>
#include <QWidget>

/**
 * The Onyx dialogs draw their own header and close button, so the native
 * window title bar is unnecessary (and clashes with the paper/onyx surfaces).
 * This filter makes the header strip behave like a title bar: press and drag
 * anywhere in the top band moves the window, while real controls still get
 * their normal clicks.
 */
class MatraKiWindowDrag : public QObject {
public:
  explicit MatraKiWindowDrag(QDialog *dialog) : QObject(dialog), m_dialog(dialog) {
    qApp->installEventFilter(this);
  }

protected:
  bool eventFilter(QObject *watched, QEvent *event) override {
    QWidget *widget = qobject_cast<QWidget *>(watched);
    if (!widget || widget->window() != m_dialog) return false;

    switch (event->type()) {
      case QEvent::MouseButtonPress: {
        auto *mouse = static_cast<QMouseEvent *>(event);
        if (mouse->button() != Qt::LeftButton) return false;
        if (qobject_cast<QAbstractButton *>(watched)) return false;
        if (m_dialog->mapFromGlobal(mouse->globalPos()).y() > kHeaderHeight) return false;
        m_pressed = true;
        m_dragging = false;
        m_pressGlobal = mouse->globalPos();
        m_windowStart = m_dialog->pos();
        return true;
      }
      case QEvent::MouseMove: {
        if (!m_pressed) return false;
        auto *mouse = static_cast<QMouseEvent *>(event);
        const QPoint delta = mouse->globalPos() - m_pressGlobal;
        if (!m_dragging && delta.manhattanLength() > 3) m_dragging = true;
        if (m_dragging) m_dialog->move(m_windowStart + delta);
        return m_dragging;
      }
      case QEvent::MouseButtonRelease: {
        if (!m_pressed) return false;
        m_pressed = false;
        const bool moved = m_dragging;
        m_dragging = false;
        return moved;
      }
      default:
        break;
    }
    return false;
  }

private:
  static constexpr int kHeaderHeight = 64;
  QDialog *m_dialog;
  QPoint m_pressGlobal;
  QPoint m_windowStart;
  bool m_pressed = false;
  bool m_dragging = false;
};

/* Strip the native title bar; each dialog draws its own header controls. */
inline void makeOnyxDialog(QDialog *dialog) {
  dialog->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
  new MatraKiWindowDrag(dialog);
}

#endif // MATRAKIWINDOWCHROME_H
