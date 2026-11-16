/*
 *  MatraKi Keyboard
 *  Vector Icon Set — Emerald Design System
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

#include "MatraKiIcons.h"

#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>

namespace {

/* All glyphs are authored on a 24 x 24 logical canvas. */
constexpr qreal kCanvas = 24.0;

void strokePen(QPainter &p, qreal width) {
  /* Keep the tint that was selected by the caller. */
  QPen pen(p.pen().color(), width);
  pen.setCapStyle(Qt::RoundCap);
  pen.setJoinStyle(Qt::RoundJoin);
  p.setPen(pen);
  p.setBrush(Qt::NoBrush);
}

QPainterPath squiggle(qreal x1, qreal y, qreal x2) {
  QPainterPath path;
  qreal step = (x2 - x1) / 2.0;
  path.moveTo(x1, y);
  path.cubicTo(x1 + step * 0.4, y - 2.4, x1 + step * 1.1, y + 2.4, x1 + step, y);
  path.cubicTo(x1 + step * 1.6, y - 2.4, x1 + step * 2.2, y + 2.4, x2, y);
  return path;
}

void drawGear(QPainter &p) {
  p.drawEllipse(QPointF(12, 12), 3.3, 3.3);
  for (int i = 0; i < 8; ++i) {
    const qreal a = i * M_PI / 4.0;
    const QPointF outer = QPointF(12 + 7.4 * std::cos(a), 12 + 7.4 * std::sin(a));
    const QPointF inner = QPointF(12 + 5.2 * std::cos(a), 12 + 5.2 * std::sin(a));
    p.drawLine(inner, outer);
  }
}

void drawKeyboardBody(QPainter &p, qreal top) {
  const qreal h = 24.0 - top - 2.0;
  p.drawRoundedRect(QRectF(2.5, top, 19.0, h), 2.4, 2.4);
}

void drawKeyDots(QPainter &p, qreal top) {
  const QColor ink = p.pen().color();
  p.save();
  p.setPen(Qt::NoPen);
  p.setBrush(ink);
  const qreal row1 = top + 3.4;
  const qreal row2 = top + 7.0;
  for (int i = 0; i < 4; ++i) {
    p.drawEllipse(QPointF(6.2 + i * 3.1, row1), 0.95, 0.95);
  }
  for (int i = 0; i < 3; ++i) {
    p.drawEllipse(QPointF(6.2 + i * 3.1, row2), 0.95, 0.95);
  }
  p.drawRoundedRect(QRectF(15.2, row2 - 1.0, 5.4, 2.0), 1.0, 1.0);
  p.restore();
}

void polyline(QPainter &p, const QVector<QPointF> &pts) {
  QPainterPath path;
  path.moveTo(pts.first());
  for (int i = 1; i < pts.size(); ++i) path.lineTo(pts.at(i));
  p.drawPath(path);
}

void drawStar(QPainter &p, const QPointF &c, qreal r, qreal inner) {
  QPainterPath path;
  for (int i = 0; i < 4; ++i) {
    const qreal a = i * M_PI / 2.0 - M_PI / 2.0;
    const QPointF tip(c.x() + r * std::cos(a), c.y() + r * std::sin(a));
    const QPointF notch(c.x() + inner * std::cos(a + M_PI / 4.0),
                        c.y() + inner * std::sin(a + M_PI / 4.0));
    if (i == 0) path.moveTo(tip);
    else path.lineTo(tip);
    const qreal b = a + M_PI / 2.0;
    path.quadTo(QPointF(c.x() + inner * 0.55 * std::cos(b), c.y() + inner * 0.55 * std::sin(b)),
                QPointF(c.x() + inner * std::cos(b), c.y() + inner * std::sin(b)));
  }
  path.closeSubpath();
  p.drawPath(path);
}

} // namespace

QPixmap MatraKiIcons::pixmap(Glyph glyph, const QColor &color, int size) {
  if (glyph == None || size <= 0) return QPixmap();

  QPixmap pm(size, size);
  pm.setDevicePixelRatio(qApp->devicePixelRatio() > 1.0 ? qApp->devicePixelRatio() : 1.0);
  pm.fill(Qt::transparent);

  QPainter p(&pm);
  p.setRenderHint(QPainter::Antialiasing, true);
  p.setRenderHint(QPainter::SmoothPixmapTransform, true);
  p.scale(size / kCanvas, size / kCanvas);
  p.setPen(QPen(color));
  p.setBrush(Qt::NoBrush);

  switch (glyph) {
  case Keyboard:
    strokePen(p, 1.7);
    drawKeyboardBody(p, 6.0);
    drawKeyDots(p, 6.0);
    break;

  case KeyboardShow:
    strokePen(p, 1.7);
    drawKeyboardBody(p, 10.5);
    drawKeyDots(p, 10.5);
    {
      QPainterPath up;
      up.moveTo(8.0, 7.4);
      up.lineTo(12.0, 3.6);
      up.lineTo(16.0, 7.4);
      p.drawPath(up);
    }
    break;

  case KeyboardHide:
    strokePen(p, 1.7);
    drawKeyboardBody(p, 10.5);
    drawKeyDots(p, 10.5);
    {
      QPainterPath down;
      down.moveTo(8.0, 3.6);
      down.lineTo(12.0, 7.4);
      down.lineTo(16.0, 3.6);
      p.drawPath(down);
    }
    break;

  case ChevronDown: {
    strokePen(p, 1.9);
    QPainterPath path;
    path.moveTo(6.5, 9.5);
    path.lineTo(12, 15.0);
    path.lineTo(17.5, 9.5);
    p.drawPath(path);
    break;
  }

  case ChevronRight: {
    strokePen(p, 1.9);
    QPainterPath path;
    path.moveTo(9.5, 6.5);
    path.lineTo(15.0, 12);
    path.lineTo(9.5, 17.5);
    p.drawPath(path);
    break;
  }

  case Settings:
    strokePen(p, 1.7);
    drawGear(p);
    break;

  case Close: {
    strokePen(p, 1.9);
    p.drawLine(QPointF(6.5, 6.5), QPointF(17.5, 17.5));
    p.drawLine(QPointF(17.5, 6.5), QPointF(6.5, 17.5));
    break;
  }

  case Info:
    strokePen(p, 1.7);
    p.drawEllipse(QPointF(12, 12), 8.8, 8.8);
    p.drawLine(QPointF(12, 11.0), QPointF(12, 16.0));
    {
      p.save();
      p.setPen(Qt::NoPen);
      p.setBrush(p.pen().color());
      p.drawEllipse(QPointF(12, 7.9), 1.05, 1.05);
      p.restore();
    }
    break;

  case World:
    strokePen(p, 1.6);
    p.drawEllipse(QPointF(12, 12), 8.8, 8.8);
    p.drawEllipse(QPointF(12, 12), 3.9, 8.8);
    p.drawLine(QPointF(3.2, 12), QPointF(20.8, 12));
    break;

  case SpellCheck:
    strokePen(p, 1.7);
    p.drawPath(squiggle(3.5, 8.5, 13.5));
    p.drawPath(squiggle(3.5, 13.0, 10.5));
    {
      QPainterPath check;
      check.moveTo(13.0, 18.0);
      check.lineTo(16.0, 21.0);
      check.lineTo(21.5, 13.5);
      p.drawPath(check);
    }
    break;
  case Heart: {
    strokePen(p, 1.7);
    QPainterPath heart;
    heart.moveTo(12, 20.6);
    heart.cubicTo(12, 20.6, 3.2, 15.6, 3.2, 9.6);
    heart.cubicTo(3.2, 6.1, 6.6, 4.1, 9.1, 6.1);
    heart.cubicTo(10.5, 7.3, 11.5, 8.7, 12, 10.1);
    heart.cubicTo(12.5, 8.7, 13.5, 7.3, 14.9, 6.1);
    heart.cubicTo(17.4, 4.1, 20.8, 6.1, 20.8, 9.6);
    heart.cubicTo(20.8, 15.6, 12, 20.6, 12, 20.6);
    heart.closeSubpath();
    p.drawPath(heart);
    break;
  }

  case Refresh: {
    strokePen(p, 1.7);
    p.drawArc(QRectF(4.2, 4.2, 15.6, 15.6), 55 * 16, 285 * 16);
    QPainterPath head;
    head.moveTo(15.9, 2.6);
    head.lineTo(19.6, 6.2);
    head.lineTo(14.4, 8.2);
    head.closeSubpath();
    p.drawPath(head);
    break;
  }

  case Search:
    strokePen(p, 1.8);
    p.drawEllipse(QPointF(10.5, 10.5), 6.2, 6.2);
    p.drawLine(QPointF(15.4, 15.4), QPointF(20.5, 20.5));
    break;

  case Sun:
    strokePen(p, 1.7);
    p.drawEllipse(QPointF(12, 12), 4.4, 4.4);
    for (int i = 0; i < 8; ++i) {
      const qreal a = i * M_PI / 4.0;
      p.drawLine(QPointF(12 + 6.4 * std::cos(a), 12 + 6.4 * std::sin(a)),
                 QPointF(12 + 8.8 * std::cos(a), 12 + 8.8 * std::sin(a)));
    }
    break;

  case Moon: {
    strokePen(p, 1.7);
    QPainterPath moon;
    moon.moveTo(18.8, 14.9);
    moon.cubicTo(17.4, 15.6, 15.6, 15.4, 14.3, 14.5);
    moon.cubicTo(11.4, 12.5, 10.9, 8.4, 13.0, 5.6);
    moon.cubicTo(15.1, 2.9, 19.0, 2.6, 21.3, 4.8);
    moon.cubicTo(19.4, 5.4, 17.4, 6.5, 15.9, 8.0);
    moon.cubicTo(13.0, 11.0, 13.0, 15.4, 15.8, 18.1);
    moon.cubicTo(16.7, 19.1, 17.8, 19.9, 18.8, 20.3);
    moon.cubicTo(18.0, 20.5, 16.9, 20.4, 15.8, 20.0);
    moon.closeSubpath();
    p.drawPath(moon);
    break;
  }

  case Plus:
    strokePen(p, 1.9);
    p.drawLine(QPointF(12, 5.5), QPointF(12, 18.5));
    p.drawLine(QPointF(5.5, 12), QPointF(18.5, 12));
    break;

  case Trash:
    strokePen(p, 1.7);
    p.drawLine(QPointF(4.2, 7.0), QPointF(19.8, 7.0));
    p.drawLine(QPointF(9.4, 4.4), QPointF(14.6, 4.4));
    polyline(p, {QPointF(6.4, 7.0), QPointF(7.4, 19.6), QPointF(16.6, 19.6), QPointF(17.6, 7.0)});
    p.drawLine(QPointF(10.4, 10.4), QPointF(10.7, 16.4));
    p.drawLine(QPointF(13.6, 10.4), QPointF(13.3, 16.4));
    break;

  case Grid:
    strokePen(p, 1.7);
    p.drawRoundedRect(QRectF(3.6, 3.6, 7.2, 7.2), 1.6, 1.6);
    p.drawRoundedRect(QRectF(13.2, 3.6, 7.2, 7.2), 1.6, 1.6);
    p.drawRoundedRect(QRectF(3.6, 13.2, 7.2, 7.2), 1.6, 1.6);
    p.drawRoundedRect(QRectF(13.2, 13.2, 7.2, 7.2), 1.6, 1.6);
    break;

  case Download:
    strokePen(p, 1.8);
    p.drawLine(QPointF(12, 3.6), QPointF(12, 14.4));
    {
      QPainterPath head;
      head.moveTo(7.4, 10.2);
      head.lineTo(12, 14.8);
      head.lineTo(16.6, 10.2);
      p.drawPath(head);
    }
    polyline(p, {QPointF(4.4, 15.6), QPointF(4.4, 19.4), QPointF(19.6, 19.4),
                  QPointF(19.6, 15.6)});
    break;

  case External:
    strokePen(p, 1.7);
    polyline(p, {QPointF(11.0, 4.6), QPointF(4.6, 4.6), QPointF(4.6, 19.4), QPointF(19.4, 19.4),
                  QPointF(19.4, 13.0)});
    p.drawLine(QPointF(13.4, 4.6), QPointF(19.4, 4.6));
    p.drawLine(QPointF(19.4, 4.6), QPointF(19.4, 10.6));
    p.drawLine(QPointF(11.0, 13.0), QPointF(19.4, 4.6));
    break;

  case Sparkles:
    strokePen(p, 1.6);
    drawStar(p, QPointF(9.5, 9.5), 6.4, 2.6);
    drawStar(p, QPointF(17.0, 16.8), 3.9, 1.6);
    break;

  case Calculator:
    strokePen(p, 1.6);
    p.drawRoundedRect(QRectF(4.6, 2.8, 14.8, 18.4), 2.2, 2.2);
    p.drawLine(QPointF(8.0, 7.2), QPointF(16.0, 7.2));
    {
      p.save();
      p.setPen(Qt::NoPen);
      p.setBrush(p.pen().color());
      p.drawEllipse(QPointF(8.8, 12.0), 1.1, 1.1);
      p.drawEllipse(QPointF(12.0, 12.0), 1.1, 1.1);
      p.drawEllipse(QPointF(15.2, 12.0), 1.1, 1.1);
      p.drawEllipse(QPointF(8.8, 16.4), 1.1, 1.1);
      p.drawEllipse(QPointF(12.0, 16.4), 1.1, 1.1);
      p.drawEllipse(QPointF(15.2, 16.4), 1.1, 1.1);
      p.restore();
    }
    break;

  case Template:
    strokePen(p, 1.6);
    p.drawRoundedRect(QRectF(4.0, 3.2, 16.0, 17.6), 2.2, 2.2);
    p.drawLine(QPointF(7.6, 8.2), QPointF(16.4, 8.2));
    p.drawLine(QPointF(7.6, 12.0), QPointF(16.4, 12.0));
    p.drawLine(QPointF(7.6, 15.8), QPointF(13.0, 15.8));
    break;

  case Clock:
    strokePen(p, 1.7);
    p.drawEllipse(QPointF(12, 12), 8.8, 8.8);
    p.drawLine(QPointF(12, 7.0), QPointF(12, 12.4));
    p.drawLine(QPointF(12, 12.4), QPointF(15.8, 14.6));
    break;

  case Wand:
    strokePen(p, 1.7);
    p.drawLine(QPointF(4.4, 19.6), QPointF(13.2, 10.8));
    p.drawLine(QPointF(11.0, 13.0), QPointF(13.6, 10.4));
    p.drawLine(QPointF(15.0, 9.0), QPointF(17.6, 6.4));
    p.drawLine(QPointF(17.6, 6.4), QPointF(20.2, 3.8));
    p.drawLine(QPointF(13.4, 4.6), QPointF(15.0, 4.6));
    p.drawLine(QPointF(14.2, 3.4), QPointF(14.2, 5.8));
    break;

  case Check:
    strokePen(p, 2.0);
    polyline(p, {QPointF(4.6, 12.6), QPointF(9.6, 17.6), QPointF(19.4, 6.6)});
    break;

  case None:
  default:
    break;
  }

  p.end();
  return pm;
}

QPixmap MatraKiIcons::combined(const QVector<QPair<Glyph, int> > &parts, const QColor &color,
                                int gap) {
  if (parts.isEmpty()) return QPixmap();

  int width = 0;
  int height = 0;
  for (const auto &part : parts) {
    const int h = part.second;
    const int w = qRound(h * (kCanvas / kCanvas));
    width += w;
    height = qMax(height, h);
  }
  width += gap * (parts.size() - 1);

  QPixmap pm(width, height);
  pm.setDevicePixelRatio(qApp->devicePixelRatio() > 1.0 ? qApp->devicePixelRatio() : 1.0);
  pm.fill(Qt::transparent);

  QPainter p(&pm);
  p.setRenderHint(QPainter::Antialiasing, true);
  int x = 0;
  for (const auto &part : parts) {
    const int h = part.second;
    const QPixmap glyph = pixmap(part.first, color, h);
    p.drawPixmap(x, (height - h) / 2, glyph);
    x += h + gap;
  }
  p.end();

  return pm;
}

QString MatraKiIcons::name(Glyph glyph) {  switch (glyph) {
  case Keyboard: return QStringLiteral("Keyboard");
  case KeyboardShow: return QStringLiteral("On-screen keyboard");
  case KeyboardHide: return QStringLiteral("Hide keyboard");
  case ChevronDown: return QStringLiteral("Chevron down");
  case ChevronRight: return QStringLiteral("Chevron right");
  case Settings: return QStringLiteral("Settings");
  case Close: return QStringLiteral("Close");
  case Info: return QStringLiteral("Information");
  case World: return QStringLiteral("Website");
  case SpellCheck: return QStringLiteral("Spell check");
  case Heart: return QStringLiteral("Support");
  case Refresh: return QStringLiteral("Refresh");
  case Search: return QStringLiteral("Search");
  case Sun: return QStringLiteral("Light theme");
  case Moon: return QStringLiteral("Dark theme");
  case Plus: return QStringLiteral("Add");
  case Trash: return QStringLiteral("Remove");
  case Grid: return QStringLiteral("Grid");
  case Download: return QStringLiteral("Download");
  case External: return QStringLiteral("Open externally");
  case Sparkles: return QStringLiteral("Smart");
  case Calculator: return QStringLiteral("Calculator");
  case Template: return QStringLiteral("Template");
  case Clock: return QStringLiteral("Clock");
  case Wand: return QStringLiteral("Wizard");
  case Check: return QStringLiteral("Done");
  case None:
  default: return QString();
  }
}

QString MatraKiIcons::bengaliName(Glyph glyph) {
  switch (glyph) {
  case Keyboard: return QStringLiteral("কীবোর্ড লেআউট");
  case KeyboardShow: return QStringLiteral("অন-স্ক্রিন কীবোর্ড");
  case KeyboardHide: return QStringLiteral("কীবোর্ড লুকান");
  case ChevronDown: return QStringLiteral("নিচে");
  case ChevronRight: return QStringLiteral("ডানে");
  case Settings: return QStringLiteral("সেটিংস");
  case Close: return QStringLiteral("বন্ধ করুন");
  case Info: return QStringLiteral("তথ্য");
  case World: return QStringLiteral("ওয়েবসাইট");
  case SpellCheck: return QStringLiteral("ব্যাকরণ ও বানান যাচাই");
  case Heart: return QStringLiteral("সহায়তা ও ডোনেশন");
  case Refresh: return QStringLiteral("রিফ্রেশ");
  case Search: return QStringLiteral("খুঁজুন");
  case Sun: return QStringLiteral("লাইট থিম");
  case Moon: return QStringLiteral("ডার্ক থিম");
  case Plus: return QStringLiteral("যোগ করুন");
  case Trash: return QStringLiteral("মুছুন");
  case Grid: return QStringLiteral("গ্রিড");
  case Download: return QStringLiteral("ডাউনলোড");
  case External: return QStringLiteral("বাইরে খুলুন");
  case Sparkles: return QStringLiteral("স্মার্ট");
  case Calculator: return QStringLiteral("ক্যালকুলেটর");
  case Template: return QStringLiteral("টেমপ্লেট");
  case Clock: return QStringLiteral("সময়");
  case Wand: return QStringLiteral("উইজার্ড");
  case Check: return QStringLiteral("সম্পন্ন");
  case None:
  default: return QString();
  }
}
