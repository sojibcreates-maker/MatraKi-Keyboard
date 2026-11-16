/*
 *  MatraKi Keyboard — Next-Gen AI Smart Bangla Keyboard
 *  Theme Manager: Emerald Design System, Light / Dark & System Detection
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

#include <QApplication>
#include <QPalette>
#include <QProcess>
#include <QTimer>
#include "ThemeManager.h"
#include "Settings.h"

namespace {

/**
 * Tiny token formatter — lets every stylesheet below be written with
 * readable "[@token]" placeholders instead of a maze of %1, %2, %3.
 */
struct Tokens {
  QHash<QString, QString> m;

  Tokens &put(const QString &key, const QString &value) {
    m.insert(key, value);
    return *this;
  }

  QString format(const QString &tpl) const {
    QString out = tpl;
    for (auto it = m.constBegin(); it != m.constEnd(); ++it) {
      out.replace(QString("[@%1]").arg(it.key()), it.value());
      out.replace(QString("[%1]").arg(it.key()), it.value());
    }
    return out;
  }
};

} // namespace

ThemeManager::ThemeManager(QObject *parent) : QObject(parent) {
  lastDetectedDark = detectSystemDark();

  // Watch for OS theme changes every 3.5 seconds
  pollTimer = new QTimer(this);
  connect(pollTimer, &QTimer::timeout, this, &ThemeManager::checkSystemThemeChange);
  pollTimer->start(3500);
}

ThemeManager &ThemeManager::instance() {
  static ThemeManager inst;
  return inst;
}

ThemeManager::Mode ThemeManager::currentMode() const {
  QString m = gSettings ? gSettings->getThemeMode().toLower() : "auto";
  if (m == "dark") return Mode::Dark;
  if (m == "light") return Mode::Light;
  return Mode::Auto;
}

QString ThemeManager::currentModeString() const {
  if (!gSettings) return "auto";
  return gSettings->getThemeMode().toLower();
}

void ThemeManager::setMode(Mode mode) {
  QString s = "auto";
  if (mode == Mode::Dark) s = "dark";
  else if (mode == Mode::Light) s = "light";
  setModeString(s);
}

void ThemeManager::setModeString(const QString &modeStr) {
  if (gSettings) {
    gSettings->setThemeMode(modeStr.toLower());
  }
  emit themeChanged(isDark());
}

bool ThemeManager::detectSystemDark() const {
  // Method 1: Freedesktop Desktop Portal via dbus-send (Standard for modern Wayland & X11 on Deepin, GNOME, KDE)
  {
    QProcess proc;
    proc.start("dbus-send", {
      "--session",
      "--print-reply=literal",
      "--dest=org.freedesktop.portal.Desktop",
      "/org/freedesktop/portal/desktop",
      "org.freedesktop.portal.Settings.Read",
      "string:org.freedesktop.appearance",
      "string:color-scheme"
    });
    if (proc.waitForFinished(300)) {
      QString out = proc.readAllStandardOutput().trimmed();
      // uint32 1 means dark mode, 2 means light mode
      if (out.contains("uint32 1")) {
        return true;
      } else if (out.contains("uint32 2")) {
        return false;
      }
    }
  }

  // Method 2: GNOME / Deepin Desktop settings via gsettings
  {
    QProcess proc;
    proc.start("gsettings", {"get", "org.gnome.desktop.interface", "color-scheme"});
    if (proc.waitForFinished(200)) {
      QString out = proc.readAllStandardOutput().trimmed().toLower();
      if (out.contains("dark")) return true;
      if (out.contains("light") || out.contains("default")) {
        // Continue to deeper check
      }
    }
  }

  // Method 3: Deepin theme name
  {
    QProcess proc;
    proc.start("gsettings", {"get", "com.deepin.xsettings", "theme-name"});
    if (proc.waitForFinished(200)) {
      QString out = proc.readAllStandardOutput().trimmed().toLower();
      if (out.contains("dark") || out.contains("black")) return true;
    }
  }

  // Method 4: GTK Theme name
  {
    QProcess proc;
    proc.start("gsettings", {"get", "org.gnome.desktop.interface", "gtk-theme"});
    if (proc.waitForFinished(200)) {
      QString out = proc.readAllStandardOutput().trimmed().toLower();
      if (out.contains("dark") || out.contains("black")) return true;
    }
  }

  // Method 5: Fallback to Qt Window palette luminance
  if (qApp) {
    int lightness = qApp->palette().color(QPalette::Window).lightness();
    return lightness < 128;
  }

  return true; // Default fallback to dark
}

bool ThemeManager::isDark() const {
  Mode m = currentMode();
  if (m == Mode::Dark) return true;
  if (m == Mode::Light) return false;
  return detectSystemDark();
}

QString ThemeManager::detectedModeName() const {
  bool d = detectSystemDark();
  return d ? "Dark Mode" : "Light Mode";
}

void ThemeManager::checkSystemThemeChange() {
  if (currentMode() != Mode::Auto) return;
  bool nowDark = detectSystemDark();
  if (nowDark != lastDetectedDark) {
    lastDetectedDark = nowDark;
    emit themeChanged(nowDark);
  }
}

MatraKiPalette ThemeManager::palette() const {
  MatraKiPalette p;
  p.dark = isDark();

  if (p.dark) {
    p.bg = QStringLiteral("#0C1310");
    p.surface = QStringLiteral("#16211C");
    p.surfaceAlt = QStringLiteral("#1D2C26");
    p.border = QStringLiteral("#2E4038");
    p.borderSoft = QStringLiteral("#243229");
    p.text = QStringLiteral("#E6EFEA");
    p.textMuted = QStringLiteral("#8FA399");
    p.icon = QStringLiteral("#B4C5BC");
    p.accent = QStringLiteral("#5FD0A6");
    p.accentSoft = QStringLiteral("#1F3A2F");
    p.brand = QStringLiteral("#17835F");
    p.brandHover = QStringLiteral("#1B9670");
    p.onBrand = QStringLiteral("#FFFFFF");
    p.danger = QStringLiteral("#F42A41");
    p.dangerSoft = QStringLiteral("#2A1A1B");
    p.shadow = QStringLiteral("rgba(0, 0, 0, 0.45)");
  } else {
    p.bg = QStringLiteral("#DCE5E0");
    p.surface = QStringLiteral("#FFFFFF");
    p.surfaceAlt = QStringLiteral("#F0F5F2");
    p.border = QStringLiteral("#CBD7D1");
    p.borderSoft = QStringLiteral("#DDE6E1");
    p.text = QStringLiteral("#17231E");
    p.textMuted = QStringLiteral("#6B7C73");
    p.icon = QStringLiteral("#4A5A52");
    p.accent = QStringLiteral("#0B5D45");
    p.accentSoft = QStringLiteral("#E3F1EB");
    p.brand = QStringLiteral("#0B5D45");
    p.brandHover = QStringLiteral("#084A37");
    p.onBrand = QStringLiteral("#FFFFFF");
    p.danger = QStringLiteral("#F42A41");
    p.dangerSoft = QStringLiteral("#FCE7E8");
    p.shadow = QStringLiteral("rgba(11, 26, 20, 0.16)");
  }

  return p;
}

QString ThemeManager::fontStack() {
  return QStringLiteral("'Tiro Bangla', 'Noto Sans Bengali', 'Segoe UI', 'Hind Siliguri', "
                       "'Noto Sans', 'Nirmala UI', 'DejaVu Sans', sans-serif");
}

QString ThemeManager::bengaliFontStack() {
  return QStringLiteral("'Tiro Bangla', 'Noto Sans Bengali', 'Hind Siliguri', 'Kalpurush', "
                       "'SolaimanLipi', 'Nirmala UI', sans-serif");
}

/* ------------------------------------------------------------------ */
/*  Shared building blocks                                            */
/* ------------------------------------------------------------------ */

QString ThemeManager::cardStyle() const {
  const MatraKiPalette p = palette();
  Tokens t;
  t.put("bg", p.bg)
      .put("surface", p.surface)
      .put("surfaceAlt", p.surfaceAlt)
      .put("border", p.border)
      .put("borderSoft", p.borderSoft)
      .put("text", p.text)
      .put("muted", p.textMuted)
      .put("icon", p.icon)
      .put("accent", p.accent)
      .put("accentSoft", p.accentSoft)
      .put("brand", p.brand)
      .put("brandHover", p.brandHover)
      .put("onBrand", p.onBrand)
      .put("danger", p.danger)
      .put("dangerSoft", p.dangerSoft)
      .put("font", fontStack())
      .put("bengali", bengaliFontStack());

  return t.format(QStringLiteral(R"(
QFrame#root {
    background-color: [@bg];
    border: 1px solid [@border];
    border-radius: 16px;
}
QFrame#card {
    background-color: [@surface];
    border: 1px solid [@border];
    border-radius: 14px;
}
QFrame#cardFlat {
    background-color: [@surfaceAlt];
    border: 1px solid [@border];
    border-radius: 12px;
}
QFrame#tile {
    background-color: [@surfaceAlt];
    border: none;
    border-radius: 12px;
}
QFrame#divider {
    background-color: [@borderSoft];
    border: none;
    border-radius: 1px;
}
QFrame#headerBar {
    background-color: transparent;
    border: none;
    border-bottom: 1px solid [@border];
    border-radius: 0px;
}
QFrame#accentBar {
    background-color: [@accentSoft];
    border: none;
    border-radius: 10px;
}
QLabel {
    color: [@text];
    font-family: [@font];
    font-size: 13px;
}
QLabel#title {
    color: [@text];
    font-size: 20px;
    font-weight: 600;
}
QLabel#subtitle {
    color: [@muted];
    font-size: 12px;
}
QLabel#fieldLabel {
    color: [@muted];
    font-size: 11px;
    font-weight: 600;
}
QLabel#sectionTitle {
    color: [@text];
    font-size: 14px;
    font-weight: 600;
}
QLabel#statValue {
    color: [@text];
    font-size: 19px;
    font-weight: 600;
}
QLabel#statCaption {
    color: [@muted];
    font-size: 11px;
}
QLabel#chip {
    color: [@accent];
    background-color: [@accentSoft];
    border-radius: 11px;
    padding: 3px 10px;
    font-size: 11px;
    font-weight: 600;
}
QLabel#badge {
    color: [@accent];
    background-color: [@accentSoft];
    border-radius: 12px;
    padding: 5px 12px;
    font-size: 11px;
    font-weight: 600;
}
QLabel#muted {
    color: [@muted];
}
QLabel#danger {
    color: [@danger];
}
QPushButton {
    color: [@text];
    background-color: [@surfaceAlt];
    border: 1px solid [@border];
    border-radius: 10px;
    padding: 8px 16px;
    font-size: 13px;
    font-weight: 500;
    outline: none;
}
QPushButton:hover {
    border-color: [@accent];
    color: [@accent];
}
QPushButton:pressed {
    background-color: [@accentSoft];
}
QPushButton:disabled {
    color: [@muted];
    border-color: [@borderSoft];
    background-color: transparent;
}
QPushButton#primary {
    color: [@onBrand];
    background-color: [@brand];
    border: 1px solid [@brand];
    border-radius: 10px;
    padding: 9px 22px;
    font-size: 13px;
    font-weight: 600;
}
QPushButton#primary:hover {
    background-color: [@brandHover];
    border-color: [@brandHover];
    color: [@onBrand];
}
QPushButton#primary:pressed {
    background-color: [@brandHover];
}
QPushButton#ghost {
    color: [@muted];
    background-color: transparent;
    border: 1px solid transparent;
    border-radius: 10px;
    padding: 9px 18px;
    font-size: 13px;
    font-weight: 500;
}
QPushButton#ghost:hover {
    color: [@accent];
    background-color: [@accentSoft];
}
QPushButton#segment {
    color: [@muted];
    background-color: transparent;
    border: 1px solid [@border];
    border-radius: 999px;
    padding: 7px 16px;
    font-size: 12px;
    font-weight: 600;
}
QPushButton#segment:hover {
    color: [@accent];
    border-color: [@accent];
}
QPushButton#segment:checked {
    color: [@accent];
    background-color: [@accentSoft];
    border-color: [@accent];
}
QPushButton#danger {
    color: [@danger];
    background-color: [@dangerSoft];
    border: 1px solid [@danger];
    border-radius: 10px;
    padding: 8px 16px;
    font-size: 13px;
    font-weight: 500;
}
QPushButton#danger:hover {
    color: [@onBrand];
    background-color: [@danger];
}
QLineEdit, QPlainTextEdit, QTextEdit, QSpinBox, QDoubleSpinBox {
    color: [@text];
    background-color: [@surfaceAlt];
    border: 1px solid [@border];
    border-radius: 10px;
    padding: 9px 12px;
    font-size: 13px;
    selection-background-color: [@accent];
    selection-color: [@onBrand];
    outline: none;
}
QLineEdit:focus, QPlainTextEdit:focus, QTextEdit:focus {
    border-color: [@accent];
}
QLineEdit::placeholder {
    color: [@muted];
}
QComboBox {
    color: [@text];
    background-color: [@surfaceAlt];
    border: 1px solid [@border];
    border-radius: 10px;
    padding: 8px 12px;
    font-size: 13px;
    outline: none;
}
QComboBox:hover { border-color: [@accent]; }
QComboBox::drop-down { border: none; width: 24px; }
QComboBox QAbstractItemView {
    color: [@text];
    background-color: [@surface];
    border: 1px solid [@border];
    border-radius: 10px;
    selection-background-color: [@accentSoft];
    selection-color: [@accent];
    padding: 4px;
    outline: none;
}
QTreeWidget, QTreeView, QListWidget, QTableWidget {
    color: [@text];
    background-color: [@surface];
    border: 1px solid [@border];
    border-radius: 12px;
    alternate-background-color: [@surfaceAlt];
    font-size: 12px;
    selection-background-color: [@accentSoft];
    selection-color: [@text];
    outline: none;
}
QHeaderView::section {
    color: [@muted];
    background-color: [@surfaceAlt];
    border: none;
    border-bottom: 1px solid [@border];
    padding: 8px;
    font-size: 11px;
    font-weight: 700;
}
QTreeWidget::item { padding: 4px 2px; }
QTreeWidget::item:selected { color: [@accent]; }
QScrollBar:vertical {
    background: transparent;
    width: 10px;
    margin: 2px;
}
QScrollBar::handle:vertical {
    background: [@border];
    border-radius: 5px;
    min-height: 28px;
}
QScrollBar::handle:vertical:hover { background: [@icon]; }
QScrollBar::add-line, QScrollBar::sub-line { height: 0px; width: 0px; }
QScrollBar:horizontal {
    background: transparent;
    height: 10px;
    margin: 2px;
}
QScrollBar::handle:horizontal {
    background: [@border];
    border-radius: 5px;
    min-width: 28px;
}
QToolTip {
    color: [@text];
    background-color: [@surface];
    border: 1px solid [@border];
    border-radius: 8px;
    padding: 6px 10px;
    font-size: 11px;
}
)"));
}

QString ThemeManager::primaryButtonStyle() const {
  const MatraKiPalette p = palette();
  return QStringLiteral(
      "QPushButton#primary { color: %1; background-color: %2; border: 1px solid %2; "
      "border-radius: 10px; padding: 9px 22px; font-size: 13px; font-weight: 600; }\n"
      "QPushButton#primary:hover { background-color: %3; border-color: %3; }")
      .arg(p.onBrand, p.brand, p.brandHover);
}

QString ThemeManager::inputStyle() const {
  const MatraKiPalette p = palette();
  return QStringLiteral(
      "QLineEdit, QPlainTextEdit, QTextEdit { color: %1; background-color: %2; "
      "border: 1px solid %3; border-radius: 10px; padding: 9px 12px; font-size: 13px; }")
      .arg(p.text, p.surfaceAlt, p.border);
}

/* ------------------------------------------------------------------ */
/*  Toolbar                                                           */
/* ------------------------------------------------------------------ */

QString ThemeManager::topBarStyle() const {
  const MatraKiPalette p = palette();
  return QStringLiteral(
      "#centralwidget {"
      "    background-color: %1;"
      "    border: 1px solid %2;"
      "    border-radius: 24px;"
      "}"
      "QPushButton {"
      "    background-color: transparent;"
      "    border: none;"
      "    border-radius: 999px;"
      "    padding: 0px;"
      "}"
      "QPushButton:hover {"
      "    background-color: %3;"
      "}"
      "QPushButton:pressed {"
      "    background-color: %3;"
      "}"
      "QPushButton:checked {"
      "    background-color: %3;"
      "}"
      "#buttonShutdown:hover {"
      "    background-color: %4;"
      "}"
      "#buttonIcon:focus {"
      "    background-color: %3;"
      "}"
      "QToolTip {"
      "    color: %5;"
      "    background-color: %6;"
      "    border: 1px solid %2;"
      "    border-radius: 8px;"
      "    padding: 6px 10px;"
      "    font-size: 11px;"
      "}")
      .arg(p.surface, p.border, p.accentSoft, p.danger, p.text, p.surfaceAlt);
}

/* ------------------------------------------------------------------ */
/*  Menus                                                              */
/* ------------------------------------------------------------------ */

QString ThemeManager::popupMenuStyle() const {
  const MatraKiPalette p = palette();
  return QStringLiteral(
      "QMenu {"
      "    background-color: %1;"
      "    color: %2;"
      "    border: 1px solid %3;"
      "    border-radius: 14px;"
      "    padding: 6px;"
      "}"
      "QMenu::item {"
      "    padding: 9px 12px;"
      "    border-radius: 9px;"
      "    color: %2;"
      "}"
      "QMenu::item:selected {"
      "    background-color: %4;"
      "    color: %5;"
      "}"
      "QMenu::item:checked {"
      "    color: %5;"
      "    font-weight: 600;"
      "}"
      "QMenu::separator {"
      "    height: 1px;"
      "    background-color: %3;"
      "    margin: 5px 6px;"
      "}")
      .arg(p.surface, p.text, p.border, p.accentSoft, p.accent);
}

QString ThemeManager::iconMenuStyle() const {
  return popupMenuStyle();
}

/* ------------------------------------------------------------------ */
/*  Settings                                                          */
/* ------------------------------------------------------------------ */

QString ThemeManager::settingsDialogStyle() const {
  const MatraKiPalette p = palette();
  Tokens t;
  t.put("bg", p.bg)
      .put("surface", p.surface)
      .put("surfaceAlt", p.surfaceAlt)
      .put("border", p.border)
      .put("borderSoft", p.borderSoft)
      .put("text", p.text)
      .put("muted", p.textMuted)
      .put("icon", p.icon)
      .put("accent", p.accent)
      .put("accentSoft", p.accentSoft)
      .put("brand", p.brand)
      .put("brandHover", p.brandHover)
      .put("onBrand", p.onBrand)
      .put("danger", p.danger)
      .put("dangerSoft", p.dangerSoft)
      .put("font", fontStack());

  return t.format(QStringLiteral(R"(
QDialog#SettingsDialog {
    background-color: transparent;
    color: [@text];
    font-family: [@font];
}
QFrame#root {
    background-color: [@bg];
    border: 1px solid [@border];
    border-radius: 16px;
}
QFrame#card, QFrame#cardPhonetic, QFrame#cardGeneral, QFrame#cardFixed, QFrame#cardSupport {
    background-color: [@surface];
    border: 1px solid [@border];
    border-radius: 14px;
}
QFrame#tile {
    background-color: [@surfaceAlt];
    border: none;
    border-radius: 12px;
}
QLabel { color: [@text]; font-size: 13px; }
QLabel#title {
    color: [@text];
    font-size: 19px;
    font-weight: 600;
}
QLabel#subtitle { color: [@muted]; font-size: 12px; }
QLabel#fieldLabel { color: [@muted]; font-size: 11px; font-weight: 600; }
QLabel#sectionTitle,
QLabel#lblSectionPhonetic,
QLabel#lblSectionGeneral,
QLabel#lblSectionFixed,
QLabel#lblSectionSupport {
    color: [@text];
    font-size: 13px;
    font-weight: 600;
}
QLabel#sectionCaption { color: [@muted]; font-size: 11px; }
QLabel#badge {
    color: [@accent];
    background-color: [@accentSoft];
    border-radius: 12px;
    padding: 5px 12px;
    font-size: 11px;
    font-weight: 600;
}
QLabel#hint { color: [@muted]; font-size: 11px; }
QPushButton {
    color: [@text];
    background-color: [@surfaceAlt];
    border: 1px solid [@border];
    border-radius: 10px;
    padding: 8px 16px;
    font-size: 13px;
    font-weight: 500;
    outline: none;
}
QPushButton:hover { border-color: [@accent]; color: [@accent]; }
QPushButton#primary {
    color: [@onBrand];
    background-color: [@brand];
    border: 1px solid [@brand];
    border-radius: 10px;
    padding: 10px 24px;
    font-size: 13px;
    font-weight: 600;
}
QPushButton#primary:hover { background-color: [@brandHover]; border-color: [@brandHover]; }
QPushButton#ghost {
    color: [@muted];
    background-color: transparent;
    border: 1px solid transparent;
    border-radius: 10px;
    padding: 10px 20px;
    font-size: 13px;
    font-weight: 500;
}
QPushButton#ghost:hover { color: [@accent]; background-color: [@accentSoft]; }
QComboBox {
    color: [@text];
    background-color: [@surfaceAlt];
    border: 1px solid [@border];
    border-radius: 10px;
    padding: 8px 12px;
    font-size: 13px;
    outline: none;
}
QComboBox:hover { border-color: [@accent]; }
QComboBox::drop-down { border: none; width: 24px; }
QComboBox QAbstractItemView {
    color: [@text];
    background-color: [@surface];
    border: 1px solid [@border];
    border-radius: 10px;
    selection-background-color: [@accentSoft];
    selection-color: [@accent];
    padding: 4px;
    outline: none;
}
QToolTip {
    color: [@text];
    background-color: [@surface];
    border: 1px solid [@border];
    border-radius: 8px;
    padding: 6px 10px;
    font-size: 11px;
}
QScrollArea { background: transparent; border: none; }
QScrollArea > QWidget > QWidget { background: transparent; }
QScrollBar:vertical { background: transparent; width: 10px; margin: 2px; }
QScrollBar::handle:vertical { background: [@border]; border-radius: 5px; min-height: 28px; }
QScrollBar::handle:vertical:hover { background: [@icon]; }
QScrollBar::add-line, QScrollBar::sub-line { height: 0px; width: 0px; }
)"));
}

/* ------------------------------------------------------------------ */
/*  About layout dialog                                               */
/* ------------------------------------------------------------------ */

QString ThemeManager::aboutLayoutDialogStyle() const {
  const MatraKiPalette p = palette();
  Tokens t;
  t.put("bg", p.bg)
      .put("surface", p.surface)
      .put("surfaceAlt", p.surfaceAlt)
      .put("border", p.border)
      .put("borderSoft", p.borderSoft)
      .put("text", p.text)
      .put("muted", p.textMuted)
      .put("icon", p.icon)
      .put("accent", p.accent)
      .put("accentSoft", p.accentSoft)
      .put("brand", p.brand)
      .put("brandHover", p.brandHover)
      .put("onBrand", p.onBrand)
      .put("danger", p.danger)
      .put("font", fontStack());

  return t.format(QStringLiteral(R"(
QDialog#AboutLayoutDialog {
    background-color: transparent;
    color: [@text];
    font-family: [@font];
}
QFrame#root {
    background-color: [@surface];
    border: 1px solid [@border];
    border-radius: 16px;
}
QFrame#headerBar {
    background-color: transparent;
    border: none;
    border-bottom: 1px solid [@border];
}
QFrame#tile, QFrame#statTileOne, QFrame#statTileTwo {
    background-color: [@surfaceAlt];
    border: none;
    border-radius: 12px;
}
QFrame#divider, QFrame#dividerOne, QFrame#dividerTwo {
    background-color: [@borderSoft];
    border: none;
}
QLabel { color: [@text]; font-size: 13px; }
QLabel#dialogTitle { color: [@text]; font-size: 14px; font-weight: 600; }
QLabel#fieldLabel,
QLabel#lblNameCaption,
QLabel#lblAuthorCaption,
QLabel#lblDescriptionCaption { color: [@muted]; font-size: 12px; }
QLabel#layoutName {
    color: [@text];
    font-size: 22px;
    font-weight: 500;
}
QLabel#chip {
    color: [@accent];
    background-color: [@accentSoft];
    border-radius: 11px;
    padding: 3px 10px;
    font-size: 12px;
    font-weight: 600;
}
QLabel#statValue,
QLabel#lblStatOneValue,
QLabel#lblStatTwoValue {
    color: [@text];
    font-size: 20px;
    font-weight: 500;
}
QLabel#statCaption,
QLabel#lblStatOneCaption,
QLabel#lblStatTwoCaption { color: [@muted]; font-size: 12px; }
QLabel#bodyText, QLabel#lblAuthor, QLabel#lblDescription { color: [@text]; font-size: 13px; }
QPushButton {
    color: [@text];
    background-color: [@surfaceAlt];
    border: 1px solid [@border];
    border-radius: 10px;
    padding: 8px 16px;
    font-size: 13px;
    font-weight: 500;
    outline: none;
}
QPushButton:hover { border-color: [@accent]; color: [@accent]; }
QPushButton#primary {
    color: [@onBrand];
    background-color: [@brand];
    border: 1px solid [@brand];
    border-radius: 10px;
    padding: 10px 24px;
    font-size: 13px;
    font-weight: 500;
}
QPushButton#primary:hover { background-color: [@brandHover]; border-color: [@brandHover]; }
QPushButton#iconOnly {
    background-color: transparent;
    border: none;
    border-radius: 999px;
    padding: 4px;
}
QPushButton#iconOnly:hover { background-color: [@accentSoft]; }
QToolTip {
    color: [@text];
    background-color: [@surface];
    border: 1px solid [@border];
    border-radius: 8px;
    padding: 6px 10px;
    font-size: 11px;
}
)"));
}

/* ------------------------------------------------------------------ */
/*  About dialog                                                      */
/* ------------------------------------------------------------------ */

QString ThemeManager::aboutDialogStyle() const {
  const MatraKiPalette p = palette();
  Tokens t;
  t.put("bg", p.bg)
      .put("surface", p.surface)
      .put("surfaceAlt", p.surfaceAlt)
      .put("border", p.border)
      .put("borderSoft", p.borderSoft)
      .put("text", p.text)
      .put("muted", p.textMuted)
      .put("icon", p.icon)
      .put("accent", p.accent)
      .put("accentSoft", p.accentSoft)
      .put("brand", p.brand)
      .put("brandHover", p.brandHover)
      .put("onBrand", p.onBrand)
      .put("danger", p.danger)
      .put("font", fontStack());

  return t.format(QStringLiteral(R"(
QDialog#AboutDialog {
    background-color: transparent;
    color: [@text];
    font-family: [@font];
}
QFrame#root {
    background-color: [@bg];
    border: 1px solid [@border];
    border-radius: 24px;
}
QFrame#headerBar {
    border-bottom: 1px solid [@borderSoft];
}
QLabel#lblTitle {
    color: [@text];
    font-size: 15px;
    font-weight: 600;
}
QLabel#lblDot {
    color: [@muted];
    font-size: 15px;
}
QLabel#lblSub {
    color: [@muted];
    font-size: 15px;
    font-weight: 500;
}
QPushButton {
    color: [@muted];
    background-color: transparent;
    border: none;
    border-radius: 999px;
    padding: 8px 18px;
    font-size: 13px;
    font-weight: 600;
    outline: none;
}
QPushButton:hover {
    color: [@text];
    background-color: [@surfaceAlt];
}
QPushButton#btnClose, QPushButton#iconOnly {
    color: [@muted];
    background-color: transparent;
    border: none;
    border-radius: 14px;
    padding: 0px;
}
QPushButton#btnClose:hover, QPushButton#iconOnly:hover {
    color: [@text];
    background-color: [@surfaceAlt];
}
QScrollArea, QScrollArea#scrollArea {
    background-color: transparent;
    border: none;
}
QScrollArea > QWidget > QWidget {
    background: transparent;
}
QWidget#scrollWidget {
    background-color: transparent;
}
QFrame#heroCard {
    background-color: [@surface];
    border: 1px solid [@border];
    border-radius: 18px;
}
QLabel#lblAppName {
    color: [@text];
    font-size: 19px;
    font-weight: 700;
}
QLabel#lblBadge {
    color: [@accent];
    background-color: [@accentSoft];
    border-radius: 999px;
    padding: 3px 10px;
    font-size: 11px;
    font-weight: 700;
}
QLabel#lblTagline {
    font-size: 12.5px;
    line-height: 1.4;
}
QFrame#statCard1, QFrame#statCard2, QFrame#statCard3 {
    background-color: [@bg];
    border: 1px solid [@border];
    border-radius: 12px;
}
QLabel#statVal1, QLabel#statVal2, QLabel#statVal3 {
    color: [@text];
    font-size: 15px;
    font-weight: 700;
}
QLabel#statCap1, QLabel#statCap2, QLabel#statCap3 {
    color: [@muted];
    font-size: 11px;
}
QFrame#sec1, QFrame#sec2 {
    border-top: 1px solid [@borderSoft];
}
QLabel#sec1Title, QLabel#sec2Title {
    color: [@accent];
    font-size: 13px;
    font-weight: 700;
}
QLabel#sec1Body, QLabel#sec2Body {
    color: [@text];
    font-size: 13px;
    line-height: 1.65;
}
QFrame#shortcutBar {
    background-color: [@surface];
    border: 1px solid [@border];
    border-radius: 14px;
}
QFrame#devBar {
    border-top: 1px solid [@borderSoft];
}
QLabel#lblDev {
    color: [@muted];
    font-size: 12.5px;
}
QFrame#footerBar {
    border-top: 1px solid [@borderSoft];
}
QLabel#lblSupport {
    color: [@muted];
    font-size: 12.5px;
}
QPushButton#btnLicense, QPushButton#ghost {
    color: [@muted];
    background-color: transparent;
    border: none;
    border-radius: 999px;
    padding: 9px 20px;
    font-size: 13px;
    font-weight: 600;
    outline: none;
}
QPushButton#btnLicense:hover, QPushButton#ghost:hover {
    color: [@text];
    background-color: [@surfaceAlt];
}
QPushButton#btnDonate, QPushButton#primary {
    color: [@onBrand];
    background-color: [@brand];
    border: 1px solid [@brand];
    border-radius: 999px;
    padding: 9px 22px;
    font-size: 13px;
    font-weight: 600;
    outline: none;
}
QPushButton#btnDonate:hover, QPushButton#primary:hover {
    background-color: [@brandHover];
    border-color: [@brandHover];
}
QScrollBar:vertical {
    background: transparent;
    width: 8px;
    margin: 4px 2px;
}
QScrollBar::handle:vertical {
    background: [@border];
    border-radius: 4px;
    min-height: 24px;
}
QScrollBar::handle:vertical:hover {
    background: [@icon];
}
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0px;
}
)"));
}

/* ------------------------------------------------------------------ */
/*  Layout viewer                                                     */
/* ------------------------------------------------------------------ */

QString ThemeManager::layoutViewerStyle() const {
  const MatraKiPalette p = palette();
  Tokens t;
  t.put("bg", p.bg)
      .put("surface", p.surface)
      .put("surfaceAlt", p.surfaceAlt)
      .put("border", p.border)
      .put("borderSoft", p.borderSoft)
      .put("text", p.text)
      .put("muted", p.textMuted)
      .put("icon", p.icon)
      .put("accent", p.accent)
      .put("accentSoft", p.accentSoft)
      .put("brand", p.brand)
      .put("brandHover", p.brandHover)
      .put("onBrand", p.onBrand)
      .put("danger", p.danger)
      .put("font", fontStack());

  return t.format(QStringLiteral(R"(
QDialog#LayoutViewer {
    background-color: transparent;
    color: [@text];
    font-family: [@font];
}
QFrame#root {
    background-color: [@bg];
    border: 1px solid [@border];
    border-radius: 16px;
}
QFrame#card, QFrame#imageCard {
    background-color: [@surface];
    border: 1px solid [@border];
    border-radius: 14px;
}
QLabel { color: [@text]; font-size: 13px; }
QLabel#title {
    color: [@text];
    font-size: 16px;
    font-weight: 600;
}
QLabel#subtitle { color: [@muted]; font-size: 12px; }
QLabel#chip {
    color: [@accent];
    background-color: [@accentSoft];
    border-radius: 11px;
    padding: 3px 10px;
    font-size: 11px;
    font-weight: 600;
}
QPushButton {
    color: [@muted];
    background-color: [@surface];
    border: 1px solid [@border];
    border-radius: 999px;
    padding: 7px 16px;
    font-size: 12px;
    font-weight: 600;
    outline: none;
}
QPushButton:hover { color: [@accent]; border-color: [@accent]; }
QPushButton:checked {
    color: [@accent];
    background-color: [@accentSoft];
    border-color: [@accent];
}
QPushButton:disabled { color: [@muted]; border-color: [@borderSoft]; }
QPushButton#primary {
    color: [@onBrand];
    background-color: [@brand];
    border: 1px solid [@brand];
    border-radius: 999px;
    padding: 7px 16px;
    font-size: 12px;
    font-weight: 600;
}
QPushButton#primary:hover { background-color: [@brandHover]; border-color: [@brandHover]; }
QToolTip {
    color: [@text];
    background-color: [@surface];
    border: 1px solid [@border];
    border-radius: 8px;
    padding: 6px 10px;
    font-size: 11px;
}
)"));
}

/* ------------------------------------------------------------------ */
/*  Auto-correct dictionary                                           */
/* ------------------------------------------------------------------ */

QString ThemeManager::autoCorrectDialogStyle() const {
  const MatraKiPalette p = palette();
  Tokens t;
  t.put("bg", p.bg)
      .put("surface", p.surface)
      .put("surfaceAlt", p.surfaceAlt)
      .put("border", p.border)
      .put("borderSoft", p.borderSoft)
      .put("text", p.text)
      .put("muted", p.textMuted)
      .put("icon", p.icon)
      .put("accent", p.accent)
      .put("accentSoft", p.accentSoft)
      .put("brand", p.brand)
      .put("brandHover", p.brandHover)
      .put("onBrand", p.onBrand)
      .put("danger", p.danger)
      .put("font", fontStack())
      .put("mono", QStringLiteral("'SF Mono', 'DejaVu Sans Mono', 'Menlo', 'Consolas', monospace"));

  return t.format(QStringLiteral(R"(
QDialog#AutoCorrectDialog {
    background-color: transparent;
    color: [@text];
    font-family: [@font];
}
QFrame#root {
    background-color: [@bg];
    border: 1px solid [@border];
    border-radius: 16px;
}
QFrame#card, QFrame#entryCard {
    background-color: [@surface];
    border: 1px solid [@border];
    border-radius: 14px;
}
QFrame#tile {
    background-color: [@surfaceAlt];
    border: none;
    border-radius: 12px;
}
QLabel { color: [@text]; font-size: 13px; }
QLabel#title {
    color: [@text];
    font-size: 17px;
    font-weight: 600;
}
QLabel#subtitle { color: [@muted]; font-size: 12px; }
QLabel#fieldLabel { color: [@muted]; font-size: 11px; font-weight: 600; }
QLabel#label, QLabel#label_2 { color: [@muted]; font-size: 11px; font-weight: 600; }
QLabel#lblEntryTitle {
    color: [@text];
    font-size: 13px;
    font-weight: 600;
}
QLabel#preview, QLabel#lblPreviewR, QLabel#lblPreviewW {
    color: [@text];
    background-color: [@surfaceAlt];
    border-radius: 10px;
    padding: 8px 12px;
    font-size: 14px;
}
QLabel#counter { color: [@muted]; font-size: 11px; }
QLineEdit {
    color: [@text];
    background-color: [@surfaceAlt];
    border: 1px solid [@border];
    border-radius: 10px;
    padding: 9px 12px;
    font-size: 13px;
    outline: none;
}
QLineEdit:focus { border-color: [@accent]; }
QLineEdit::placeholder { color: [@muted]; }
QTreeWidget {
    color: [@text];
    background-color: [@surface];
    border: 1px solid [@border];
    border-radius: 12px;
    alternate-background-color: [@surfaceAlt];
    font-size: 12px;
    selection-background-color: [@accentSoft];
    selection-color: [@text];
    outline: none;
}
QHeaderView::section {
    color: [@muted];
    background-color: [@surfaceAlt];
    border: none;
    border-bottom: 1px solid [@border];
    padding: 8px;
    font-size: 11px;
    font-weight: 700;
}
QPushButton {
    color: [@text];
    background-color: [@surfaceAlt];
    border: 1px solid [@border];
    border-radius: 10px;
    padding: 8px 16px;
    font-size: 13px;
    font-weight: 500;
    outline: none;
}
QPushButton:hover { border-color: [@accent]; color: [@accent]; }
QPushButton#primary {
    color: [@onBrand];
    background-color: [@brand];
    border: 1px solid [@brand];
    border-radius: 10px;
    padding: 9px 20px;
    font-size: 13px;
    font-weight: 600;
}
QPushButton#primary:hover { background-color: [@brandHover]; border-color: [@brandHover]; }
QPushButton#ghost {
    color: [@muted];
    background-color: transparent;
    border: 1px solid transparent;
    border-radius: 10px;
    padding: 9px 18px;
    font-size: 13px;
}
QPushButton#ghost:hover { color: [@accent]; background-color: [@accentSoft]; }
QScrollBar:vertical { background: transparent; width: 10px; margin: 2px; }
QScrollBar::handle:vertical { background: [@border]; border-radius: 5px; min-height: 28px; }
QScrollBar::handle:vertical:hover { background: [@icon]; }
QScrollBar::add-line, QScrollBar::sub-line { height: 0px; width: 0px; }
QToolTip {
    color: [@text];
    background-color: [@surface];
    border: 1px solid [@border];
    border-radius: 8px;
    padding: 6px 10px;
    font-size: 11px;
}
)"));
}

/* ------------------------------------------------------------------ */
/*  Harper grammar dialog                                             */
/* ------------------------------------------------------------------ */

QString ThemeManager::harperDialogStyle() const {
  const MatraKiPalette p = palette();
  Tokens t;
  t.put("bg", p.bg)
      .put("surface", p.surface)
      .put("surfaceAlt", p.surfaceAlt)
      .put("border", p.border)
      .put("borderSoft", p.borderSoft)
      .put("text", p.text)
      .put("muted", p.textMuted)
      .put("icon", p.icon)
      .put("accent", p.accent)
      .put("accentSoft", p.accentSoft)
      .put("brand", p.brand)
      .put("brandHover", p.brandHover)
      .put("onBrand", p.onBrand)
      .put("danger", p.danger)
      .put("dangerSoft", p.dangerSoft)
      .put("font", fontStack())
      .put("mono", QStringLiteral("'SF Mono', 'DejaVu Sans Mono', 'Menlo', 'Consolas', monospace"));

  return t.format(QStringLiteral(R"(
QDialog#HarperDialog {
    background-color: transparent;
    color: [@text];
    font-family: [@font];
}
QFrame#root {
    background-color: [@bg];
    border: 1px solid [@border];
    border-radius: 16px;
}
QFrame#card, QFrame#inputCard {
    background-color: [@surface];
    border: 1px solid [@border];
    border-radius: 14px;
}
QFrame#codeBlock {
    background-color: [@surfaceAlt];
    border: 1px solid [@border];
    border-radius: 12px;
}
QLabel { color: [@text]; font-size: 13px; }
QLabel#title {
    color: [@text];
    font-size: 17px;
    font-weight: 600;
}
QLabel#subtitle { color: [@muted]; font-size: 12px; }
QLabel#sectionTitle {
    color: [@accent];
    font-size: 12px;
    font-weight: 700;
}
QTextEdit {
    color: [@text];
    background-color: [@surfaceAlt];
    border: 1px solid [@border];
    border-radius: 12px;
    padding: 10px;
    font-size: 13px;
    selection-background-color: [@accent];
    selection-color: [@onBrand];
    outline: none;
}
QTextEdit#output {
    font-family: [@mono];
    font-size: 12px;
}
QPushButton {
    color: [@text];
    background-color: [@surfaceAlt];
    border: 1px solid [@border];
    border-radius: 10px;
    padding: 9px 16px;
    font-size: 13px;
    font-weight: 500;
    outline: none;
}
QPushButton:hover { border-color: [@accent]; color: [@accent]; }
QPushButton#primary {
    color: [@onBrand];
    background-color: [@brand];
    border: 1px solid [@brand];
    border-radius: 10px;
    padding: 9px 20px;
    font-size: 13px;
    font-weight: 600;
}
QPushButton#primary:hover { background-color: [@brandHover]; border-color: [@brandHover]; }
QScrollBar:vertical { background: transparent; width: 10px; margin: 2px; }
QScrollBar::handle:vertical { background: [@border]; border-radius: 5px; min-height: 28px; }
QScrollBar::handle:vertical:hover { background: [@icon]; }
QScrollBar::add-line, QScrollBar::sub-line { height: 0px; width: 0px; }
QToolTip {
    color: [@text];
    background-color: [@surface];
    border: 1px solid [@border];
    border-radius: 8px;
    padding: 6px 10px;
    font-size: 11px;
}
)"));
}

/* ------------------------------------------------------------------ */
/*  Message boxes & update dialog                                     */
/* ------------------------------------------------------------------ */

QString ThemeManager::messageBoxStyle() const {
  const MatraKiPalette p = palette();
  return QStringLiteral(
      "QMessageBox { background-color: %1; color: %2; font-family: %6; }"
      "QLabel { color: %2; font-size: 13px; }"
      "QPushButton { color: %2; background-color: %3; border: 1px solid %4; border-radius: 10px; "
      "padding: 9px 18px; font-size: 13px; font-weight: 500; }"
      "QPushButton:hover { color: %5; border-color: %5; }")
      .arg(p.surface, p.text, p.surfaceAlt, p.border, p.accent, fontStack());
}

QString ThemeManager::updateDialogStyle() const {
  const MatraKiPalette p = palette();
  Tokens t;
  t.put("bg", p.bg)
      .put("surface", p.surface)
      .put("surfaceAlt", p.surfaceAlt)
      .put("border", p.border)
      .put("borderSoft", p.borderSoft)
      .put("text", p.text)
      .put("muted", p.textMuted)
      .put("icon", p.icon)
      .put("accent", p.accent)
      .put("accentSoft", p.accentSoft)
      .put("brand", p.brand)
      .put("brandHover", p.brandHover)
      .put("onBrand", p.onBrand)
      .put("danger", p.danger)
      .put("font", fontStack());

  return t.format(QStringLiteral(R"(
QDialog#UpdateDialog {
    background-color: transparent;
    color: [@text];
    font-family: [@font];
}
QFrame#root {
    background-color: [@bg];
    border: 1px solid [@border];
    border-radius: 24px;
}
QFrame#headerBar {
    border-bottom: 1px solid [@borderSoft];
}
QFrame#updateCard {
    background-color: [@surface];
    border: 1px solid [@border];
    border-radius: 18px;
}
QLabel#lblTitle {
    color: [@text];
    font-size: 15px;
    font-weight: 600;
}
QLabel#lblDot {
    color: [@muted];
    font-size: 15px;
}
QLabel#lblSub {
    color: [@muted];
    font-size: 13.5px;
    font-weight: 500;
}
QLabel#lblHeadline {
    color: [@text];
    font-size: 15px;
    font-weight: 700;
}
QLabel#lblVersionLine {
    color: [@muted];
    font-size: 12px;
}
QLabel#lblHighlightsTitle {
    color: [@brand];
    font-size: 12px;
    font-weight: 700;
}
QLabel#lblHighlightsBody {
    color: [@text];
    font-size: 12px;
    line-height: 1.55;
}
QPushButton {
    color: [@muted];
    background-color: transparent;
    border: none;
    border-radius: 999px;
    padding: 8px 18px;
    font-size: 12.5px;
    font-weight: 600;
    outline: none;
}
QPushButton:hover {
    color: [@text];
    background-color: [@surfaceAlt];
}
QPushButton#primary {
    color: [@onBrand];
    background-color: [@brand];
    border: 1px solid [@brand];
    border-radius: 999px;
    padding: 8px 22px;
    font-size: 13px;
    font-weight: 600;
}
QPushButton#primary:hover {
    background-color: [@brandHover];
    border-color: [@brandHover];
}
QPushButton#iconOnly {
    background-color: transparent;
    border: none;
    border-radius: 999px;
    padding: 0px;
}
QPushButton#iconOnly:hover {
    background-color: [@surfaceAlt];
}
)"));
}
