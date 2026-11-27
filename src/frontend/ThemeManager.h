/*
 *  MatraKi Keyboard — Next-Gen AI Smart Bangla Keyboard
 *  Theme Manager: Onyx Design System, Light / Dark & System Detection
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

#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QString>

class QColor;
class QWidget;
class QTimer;

/**
 * The complete colour contract of the redesigned interface.
 *
 * Every surface of MatraKi — toolbar, menus, dialogs, cards, inputs and
 * tables — is painted from this single struct, so Light and Dark stay in
 * perfect sync and new widgets can never drift away from the artwork.
 */
struct MatraKiPalette {
  bool dark = true;

  QString bg;          /* window / stage background           */
  QString surface;     /* cards, toolbar, menus               */
  QString surfaceAlt;  /* elevated tiles, inputs, code blocks */
  QString border;      /* hairlines                           */
  QString borderSoft;  /* faint separators                    */
  QString text;        /* primary text                        */
  QString textMuted;   /* secondary text                      */
  QString icon;        /* icon tint                           */
  QString accent;      /* brass accent (text & icons)         */
  QString accentSoft;  /* selected / hovered row background   */
  QString brand;       /* primary fill (logo, buttons)        */
  QString brandHover;  /* primary fill hover                  */
  QString onBrand;     /* text on brand fill                  */
  QString danger;      /* destructive accent                  */
  QString dangerSoft;  /* destructive background              */
  QString shadow;      /* drop shadow colour                  */

  /* Keycap surfaces — the physical-keyboard language used by the
     layout charts, the brand mark and the preview tiles. */
  QString keyBg;       /* keycap top face                     */
  QString keyBorder;   /* keycap rim                          */
  QString keyText;     /* Bangla / primary legend             */
  QString keyMuted;    /* secondary legend                    */
  QString keyAccent;   /* brass legend                        */
  QString keyShadow;   /* keycap underside (depth)            */
  QString keyHighlight;/* top-edge highlight                  */
};

class ThemeManager : public QObject {
  Q_OBJECT

public:
  enum class Mode {
    Auto,  /* Detect from OS */
    Dark,  /* Force Dark Onyx */
    Light  /* Force Light Paper */
  };

  static ThemeManager &instance();

  Mode currentMode() const;
  QString currentModeString() const;
  void setMode(Mode mode);
  void setModeString(const QString &modeStr);

  bool isDark() const;
  bool detectSystemDark() const;
  QString detectedModeName() const;

  /* Active colour contract. */
  MatraKiPalette palette() const;

  /* Font stacks shared by every window. Body copy is set in a clean
     grotesque, while display titles use the classic Tiro serif. */
  static QString fontStack();
  static QString displayFontStack();
  static QString bengaliFontStack();

  /* Stylesheets — Onyx Design System. */
  QString topBarStyle() const;
  QString settingsDialogStyle() const;
  QString aboutDialogStyle() const;
  QString aboutLayoutDialogStyle() const;
  QString layoutViewerStyle() const;
  QString autoCorrectDialogStyle() const;
  QString harperDialogStyle() const;
  QString popupMenuStyle() const;
  QString iconMenuStyle() const;
  QString messageBoxStyle() const;
  QString updateDialogStyle() const;

  /* Shared building blocks reused by the dialog stylesheets. */
  QString cardStyle() const;
  QString primaryButtonStyle() const;
  QString inputStyle() const;

signals:
  void themeChanged(bool isDark);

private:
  explicit ThemeManager(QObject *parent = nullptr);
  ~ThemeManager() override = default;

  mutable bool lastDetectedDark = true;
  QTimer *pollTimer = nullptr;

  void checkSystemThemeChange();
};

#endif // THEMEMANAGER_H
