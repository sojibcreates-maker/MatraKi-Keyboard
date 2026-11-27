/*
 *  MatraKi Keyboard
 *  Vector Icon Set — Onyx Design System
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

#ifndef MATRAKIICONS_H
#define MATRAKIICONS_H

#include <QColor>
#include <QPixmap>
#include <QString>
#include <QVector>
#include <QPair>

/**
 * MatraKiIcons renders the whole icon set of the redesigned interface as
 * vector art. Icons are stroked on a 24x24 logical canvas, tinted with the
 * palette colour at paint time and finally rasterised to a transparent
 * pixmap. This keeps every glyph crisp at any device pixel ratio and makes
 * the icon colour follow the active Light / Dark theme automatically.
 */
class MatraKiIcons {
public:
  enum Glyph {
    None = 0,
    Keyboard,
    KeyboardShow,
    KeyboardHide,
    ChevronDown,
    ChevronRight,
    Settings,
    Close,
    Info,
    World,
    SpellCheck,
    Heart,
    Refresh,
    Search,
    Sun,
    Moon,
    Plus,
    Trash,
    Grid,
    Download,
    External,
    Sparkles,
    Calculator,
    Template,
    Clock,
    Wand,
    Check
  };

  /* Render @glyph tinted with @color at @size device pixels. */
  static QPixmap pixmap(Glyph glyph, const QColor &color, int size = 20);

  /**
   * Compose several glyphs side by side into a single pixmap. Each entry is
   * a (glyph, pixel height) pair; the widths are derived from the glyph
   * aspect ratio. Used for buttons that show e.g. a keyboard plus a chevron.
   */
  static QPixmap combined(const QVector<QPair<Glyph, int> > &parts, const QColor &color,
                          int gap = 2);

  /* Human readable name — used for accessible names and tooltips. */
  static QString name(Glyph glyph);

  /* Bengali label for menu rows / badges. */
  static QString bengaliName(Glyph glyph);
};

#endif // MATRAKIICONS_H
