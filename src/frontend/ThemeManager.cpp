/*
 *  MatraKi Keyboard — Next-Gen AI Smart Bangla Keyboard
 *  Theme Manager: Light / Dark Mode & Automatic System Detection
 *  Copyright (C) 2026 Md Sojib Hossain
 */

#include <QApplication>
#include <QPalette>
#include <QProcess>
#include <QTimer>
#include "ThemeManager.h"
#include "Settings.h"

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

  return true; // Default fallback to dark tech
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

QString ThemeManager::iconSettings() const {
  return isDark() ? ":/images/outline_settings_black_24dp.png"
                  : ":/images/outline_settings_darkicon_24dp.png";
}

QString ThemeManager::iconKeyboard() const {
  return isDark() ? ":/images/outline_keyboard_black_24dp.png"
                  : ":/images/outline_keyboard_darkicon_24dp.png";
}

QString ThemeManager::iconKeyboardHide() const {
  return isDark() ? ":/images/outline_keyboard_hide_black_24dp.png"
                  : ":/images/outline_keyboard_hide_darkicon_24dp.png";
}

QString ThemeManager::iconClose() const {
  return isDark() ? ":/images/outline_close_black_24dp.png"
                  : ":/images/outline_close_darkicon_24dp.png";
}

QString ThemeManager::iconDescription() const {
  return isDark() ? ":/images/outline_description_black_18dp.png"
                  : ":/images/outline_description_darkicon_18dp.png";
}

QString ThemeManager::topBarStyle() const {
  if (isDark()) {
    return QStringLiteral(
      "#centralwidget {"
      "    background-color: rgba(9, 13, 22, 0.95);"
      "    border: 1.5px solid qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0ea5e9, stop:0.5 #6366f1, stop:1 #38bdf8);"
      "    border-radius: 18px;"
      "}"
      "QPushButton {"
      "    background-color: transparent;"
      "    border: none;"
      "    border-radius: 13px;"
      "    padding: 2px;"
      "}"
      "QPushButton:hover {"
      "    background-color: rgba(56, 189, 248, 0.20);"
      "}"
      "QPushButton:pressed {"
      "    background-color: rgba(14, 165, 233, 0.38);"
      "}"
      "QPushButton:checked {"
      "    background-color: rgba(14, 165, 233, 0.28);"
      "}"
      "#buttonShutdown:hover {"
      "    background-color: rgba(239, 68, 68, 0.75);"
      "}"
      "QToolTip {"
      "    background-color: #0f172a;"
      "    color: #f8fafc;"
      "    border: 1px solid #38bdf8;"
      "    border-radius: 6px;"
      "    padding: 4px 8px;"
      "    font-size: 11px;"
      "}"
    );
  } else {
    return QStringLiteral(
      "#centralwidget {"
      "    background-color: rgba(255, 255, 255, 0.96);"
      "    border: 1.5px solid qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0ea5e9, stop:0.5 #3b82f6, stop:1 #0284c7);"
      "    border-radius: 18px;"
      "}"
      "QPushButton {"
      "    background-color: transparent;"
      "    border: none;"
      "    border-radius: 13px;"
      "    padding: 2px;"
      "}"
      "QPushButton:hover {"
      "    background-color: rgba(14, 165, 233, 0.14);"
      "}"
      "QPushButton:pressed {"
      "    background-color: rgba(14, 165, 233, 0.28);"
      "}"
      "QPushButton:checked {"
      "    background-color: rgba(14, 165, 233, 0.22);"
      "}"
      "#buttonShutdown:hover {"
      "    background-color: rgba(239, 68, 68, 0.85);"
      "}"
      "QToolTip {"
      "    background-color: #ffffff;"
      "    color: #0f172a;"
      "    border: 1px solid #0284c7;"
      "    border-radius: 6px;"
      "    padding: 4px 8px;"
      "    font-size: 11px;"
      "}"
    );
  }
}

QString ThemeManager::settingsDialogStyle() const {
  if (isDark()) {
    return QStringLiteral(
      "QDialog#SettingsDialog {"
      "    background-color: #090d16;"
      "    color: #f8fafc;"
      "    font-family: 'Noto Sans', 'Segoe UI', sans-serif;"
      "}"
      "QFrame#headerFrame {"
      "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0f172a, stop:0.5 #162033, stop:1 #0f172a);"
      "    border: 1px solid rgba(56, 189, 248, 0.35);"
      "    border-radius: 12px;"
      "}"
      "QLabel#lblHeaderTitle {"
      "    color: #ffffff;"
      "    font-size: 18px;"
      "    font-weight: 700;"
      "}"
      "QLabel#lblHeaderSub {"
      "    color: #94a3b8;"
      "    font-size: 12px;"
      "}"
      "QLabel#lblBadge {"
      "    background-color: rgba(16, 185, 129, 0.18);"
      "    color: #34d399;"
      "    border: 1px solid #10b981;"
      "    border-radius: 12px;"
      "    padding: 4px 12px;"
      "    font-size: 11px;"
      "    font-weight: bold;"
      "}"
      "QGroupBox {"
      "    background-color: #111827;"
      "    border: 1px solid #1e293b;"
      "    border-radius: 12px;"
      "    margin-top: 12px;"
      "    padding: 18px 14px 12px 14px;"
      "    font-size: 13px;"
      "    font-weight: bold;"
      "    color: #38bdf8;"
      "}"
      "QGroupBox::title {"
      "    subcontrol-origin: margin;"
      "    subcontrol-position: top left;"
      "    left: 14px;"
      "    padding: 2px 10px;"
      "    background-color: #0f172a;"
      "    border: 1px solid #38bdf8;"
      "    border-radius: 6px;"
      "    color: #38bdf8;"
      "}"
      "QGroupBox#FixedLayout { color: #f43f5e; }"
      "QGroupBox#FixedLayout::title { border: 1px solid #f43f5e; color: #fb7185; }"
      "QLabel { color: #e2e8f0; font-size: 13px; }"
      "QPushButton {"
      "    background-color: #1e293b;"
      "    color: #94a3b8;"
      "    border: 1px solid #334155;"
      "    border-radius: 14px;"
      "    padding: 5px 12px;"
      "    min-width: 90px;"
      "    max-width: 105px;"
      "    min-height: 20px;"
      "    font-size: 12px;"
      "    font-weight: bold;"
      "    outline: none;"
      "}"
      "QPushButton:hover { border-color: #38bdf8; color: #f8fafc; }"
      "QPushButton:checked {"
      "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0284c7, stop:1 #38bdf8);"
      "    color: #030712; font-weight: 800; border: 1px solid #38bdf8;"
      "}"
      "QPushButton:disabled { background-color: #0f172a; color: #475569; border: 1px solid #1e293b; }"
      "QPushButton#btnACUpdate {"
      "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #7c3aed);"
      "    color: #ffffff; border: 1px solid #60a5fa; border-radius: 8px; padding: 5px 12px;"
      "}"
      "QPushButton#btnACUpdate:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1d4ed8, stop:1 #6d28d9); }"
      "QComboBox {"
      "    background-color: #1e293b; color: #f8fafc; border: 1px solid #38bdf8; border-radius: 8px;"
      "    padding: 4px 10px; min-width: 95px; max-width: 150px; font-size: 12px; font-weight: bold;"
      "}"
      "QComboBox::drop-down { border: none; width: 20px; }"
      "QComboBox QAbstractItemView {"
      "    background-color: #0f172a; color: #f8fafc; selection-background-color: #1d4ed8; selection-color: #ffffff; border: 1px solid #38bdf8;"
      "}"
      "QDialogButtonBox QPushButton {"
      "    min-width: 120px; max-width: 160px; min-height: 28px; border-radius: 8px;"
      "    background-color: #1e293b; color: #e2e8f0; border: 1px solid #475569; font-weight: 600;"
      "}"
      "QDialogButtonBox QPushButton:hover { background-color: #0284c7; color: #ffffff; border-color: #38bdf8; }"
    );
  } else {
    return QStringLiteral(
      "QDialog#SettingsDialog {"
      "    background-color: #f8fafc;"
      "    color: #0f172a;"
      "    font-family: 'Noto Sans', 'Segoe UI', sans-serif;"
      "}"
      "QFrame#headerFrame {"
      "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ffffff, stop:0.5 #f1f5f9, stop:1 #ffffff);"
      "    border: 1px solid #cbd5e1;"
      "    border-radius: 12px;"
      "}"
      "QLabel#lblHeaderTitle {"
      "    color: #0f172a;"
      "    font-size: 18px;"
      "    font-weight: 700;"
      "}"
      "QLabel#lblHeaderSub {"
      "    color: #475569;"
      "    font-size: 12px;"
      "}"
      "QLabel#lblBadge {"
      "    background-color: rgba(5, 150, 105, 0.12);"
      "    color: #059669;"
      "    border: 1px solid #10b981;"
      "    border-radius: 12px;"
      "    padding: 4px 12px;"
      "    font-size: 11px;"
      "    font-weight: bold;"
      "}"
      "QGroupBox {"
      "    background-color: #ffffff;"
      "    border: 1px solid #e2e8f0;"
      "    border-radius: 12px;"
      "    margin-top: 12px;"
      "    padding: 18px 14px 12px 14px;"
      "    font-size: 13px;"
      "    font-weight: bold;"
      "    color: #0284c7;"
      "}"
      "QGroupBox::title {"
      "    subcontrol-origin: margin;"
      "    subcontrol-position: top left;"
      "    left: 14px;"
      "    padding: 2px 10px;"
      "    background-color: #ffffff;"
      "    border: 1px solid #0284c7;"
      "    border-radius: 6px;"
      "    color: #0284c7;"
      "}"
      "QGroupBox#FixedLayout { color: #e11d48; }"
      "QGroupBox#FixedLayout::title { border: 1px solid #e11d48; color: #e11d48; }"
      "QLabel { color: #1e293b; font-size: 13px; }"
      "QPushButton {"
      "    background-color: #f1f5f9;"
      "    color: #64748b;"
      "    border: 1px solid #cbd5e1;"
      "    border-radius: 14px;"
      "    padding: 5px 12px;"
      "    min-width: 90px;"
      "    max-width: 105px;"
      "    min-height: 20px;"
      "    font-size: 12px;"
      "    font-weight: bold;"
      "    outline: none;"
      "}"
      "QPushButton:hover { border-color: #0284c7; color: #0f172a; background-color: #e2e8f0; }"
      "QPushButton:checked {"
      "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0284c7, stop:1 #0ea5e9);"
      "    color: #ffffff; font-weight: 800; border: 1px solid #0284c7;"
      "}"
      "QPushButton:disabled { background-color: #f8fafc; color: #94a3b8; border: 1px solid #e2e8f0; }"
      "QPushButton#btnACUpdate {"
      "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0284c7, stop:1 #6366f1);"
      "    color: #ffffff; border: 1px solid #0284c7; border-radius: 8px; padding: 5px 12px;"
      "}"
      "QPushButton#btnACUpdate:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0369a1, stop:1 #4f46e5); }"
      "QComboBox {"
      "    background-color: #ffffff; color: #0f172a; border: 1px solid #cbd5e1; border-radius: 8px;"
      "    padding: 4px 10px; min-width: 95px; max-width: 150px; font-size: 12px; font-weight: bold;"
      "}"
      "QComboBox::drop-down { border: none; width: 20px; }"
      "QComboBox QAbstractItemView {"
      "    background-color: #ffffff; color: #0f172a; selection-background-color: #0284c7; selection-color: #ffffff; border: 1px solid #cbd5e1;"
      "}"
      "QDialogButtonBox QPushButton {"
      "    min-width: 120px; max-width: 160px; min-height: 28px; border-radius: 8px;"
      "    background-color: #f1f5f9; color: #1e293b; border: 1px solid #cbd5e1; font-weight: 600;"
      "}"
      "QDialogButtonBox QPushButton:hover { background-color: #0284c7; color: #ffffff; border-color: #0284c7; }"
    );
  }
}

QString ThemeManager::aboutDialogStyle() const {
  if (isDark()) {
    return QStringLiteral(
      "QDialog#AboutDialog {"
      "    background-color: #090d16;"
      "    color: #f8fafc;"
      "    font-family: 'Noto Sans', 'Segoe UI', sans-serif;"
      "}"
      "QFrame#frameHeader {"
      "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0f172a, stop:0.5 #162033, stop:1 #0f172a);"
      "    border: 1.5px solid #38bdf8; border-radius: 14px;"
      "}"
      "QFrame#frameFeatures { background-color: #111827; border: 1px solid #1e293b; border-radius: 12px; }"
      "QFrame#frameShortcuts { background-color: #0f172a; border: 1px solid rgba(56, 189, 248, 0.35); border-radius: 10px; }"
      "QFrame#frameDev {"
      "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(2, 132, 199, 0.20), stop:1 rgba(14, 165, 233, 0.14));"
      "    border: 1px solid rgba(56, 189, 248, 0.45); border-radius: 12px;"
      "}"
      "QFrame#frameDonation {"
      "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(225, 29, 72, 0.22), stop:1 rgba(244, 63, 94, 0.16));"
      "    border: 1px solid rgba(244, 63, 94, 0.55); border-radius: 12px;"
      "}"
      "QPushButton {"
      "    background-color: #1e293b; color: #f8fafc; border: 1px solid #38bdf8; border-radius: 8px; padding: 6px 14px; font-weight: bold;"
      "}"
      "QPushButton:hover { background-color: #0284c7; }"
    );
  } else {
    return QStringLiteral(
      "QDialog#AboutDialog {"
      "    background-color: #f8fafc;"
      "    color: #0f172a;"
      "    font-family: 'Noto Sans', 'Segoe UI', sans-serif;"
      "}"
      "QFrame#frameHeader {"
      "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ffffff, stop:0.5 #f1f5f9, stop:1 #ffffff);"
      "    border: 1.5px solid #0284c7; border-radius: 14px;"
      "}"
      "QFrame#frameFeatures { background-color: #ffffff; border: 1px solid #e2e8f0; border-radius: 12px; }"
      "QFrame#frameShortcuts { background-color: #f1f5f9; border: 1px solid #cbd5e1; border-radius: 10px; }"
      "QFrame#frameDev {"
      "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(2, 132, 199, 0.10), stop:1 rgba(14, 165, 233, 0.08));"
      "    border: 1px solid #0284c7; border-radius: 12px;"
      "}"
      "QFrame#frameDonation {"
      "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(225, 29, 72, 0.10), stop:1 rgba(244, 63, 94, 0.06));"
      "    border: 1px solid #f43f5e; border-radius: 12px;"
      "}"
      "QPushButton {"
      "    background-color: #ffffff; color: #0f172a; border: 1px solid #0284c7; border-radius: 8px; padding: 6px 14px; font-weight: bold;"
      "}"
      "QPushButton:hover { background-color: #0284c7; color: #ffffff; }"
    );
  }
}

QString ThemeManager::aboutFileStyle() const {
  if (isDark()) {
    return QStringLiteral(
      "QDialog#AboutFile { background-color: #090d16; color: #f8fafc; font-family: 'Noto Sans', sans-serif; }"
      "QLabel { color: #38bdf8; font-size: 12.5px; font-weight: bold; }"
      "QLineEdit, QPlainTextEdit { background-color: #111827; color: #f1f5f9; border: 1px solid #1e293b; border-radius: 8px; padding: 6px 10px; font-size: 12.5px; }"
      "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1d4ed8, stop:1 #0ea5e9); color: #ffffff; border: 1px solid #38bdf8; border-radius: 8px; padding: 6px 22px; font-size: 12.5px; font-weight: bold; }"
      "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1e40af, stop:1 #0284c7); }"
    );
  } else {
    return QStringLiteral(
      "QDialog#AboutFile { background-color: #f8fafc; color: #0f172a; font-family: 'Noto Sans', sans-serif; }"
      "QLabel { color: #0284c7; font-size: 12.5px; font-weight: bold; }"
      "QLineEdit, QPlainTextEdit { background-color: #ffffff; color: #0f172a; border: 1px solid #cbd5e1; border-radius: 8px; padding: 6px 10px; font-size: 12.5px; }"
      "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0284c7, stop:1 #0ea5e9); color: #ffffff; border: 1px solid #0284c7; border-radius: 8px; padding: 6px 22px; font-size: 12.5px; font-weight: bold; }"
      "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0369a1, stop:1 #0284c7); }"
    );
  }
}

QString ThemeManager::layoutViewerStyle() const {
  if (isDark()) {
    return QStringLiteral(
      "QDialog#LayoutViewer {"
      "    background-color: #090d16;"
      "    color: #f8fafc;"
      "    font-family: 'Noto Sans', 'Segoe UI', sans-serif;"
      "}"
      "QFrame#headerFrame {"
      "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0f172a, stop:0.5 #162033, stop:1 #0f172a);"
      "    border: 1.5px solid #38bdf8; border-radius: 12px;"
      "}"
      "QLabel#lblHeaderTitle { color: #ffffff; font-size: 16px; font-weight: bold; }"
      "QLabel#lblHeaderSub { color: #94a3b8; font-size: 11.5px; }"
      "QPushButton {"
      "    background-color: #1e293b; color: #cbd5e1; border: 1px solid #334155; border-radius: 14px;"
      "    padding: 6px 16px; font-size: 12px; font-weight: bold; outline: none;"
      "}"
      "QPushButton:hover { border-color: #38bdf8; color: #ffffff; }"
      "QPushButton:checked {"
      "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0284c7, stop:1 #0ea5e9);"
      "    color: #ffffff; border: 1px solid #38bdf8;"
      "}"
      "QPushButton:disabled { background-color: #0f172a; color: #475569; border-color: #1e293b; }"
      "QComboBox {"
      "    background-color: #1e293b; color: #f8fafc; border: 1px solid #38bdf8; border-radius: 8px; padding: 5px 12px;"
      "}"
    );
  } else {
    return QStringLiteral(
      "QDialog#LayoutViewer {"
      "    background-color: #f8fafc;"
      "    color: #0f172a;"
      "    font-family: 'Noto Sans', 'Segoe UI', sans-serif;"
      "}"
      "QFrame#headerFrame {"
      "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ffffff, stop:0.5 #f1f5f9, stop:1 #ffffff);"
      "    border: 1.5px solid #0284c7; border-radius: 12px;"
      "}"
      "QLabel#lblHeaderTitle { color: #0f172a; font-size: 16px; font-weight: bold; }"
      "QLabel#lblHeaderSub { color: #475569; font-size: 11.5px; }"
      "QPushButton {"
      "    background-color: #f1f5f9; color: #475569; border: 1px solid #cbd5e1; border-radius: 14px;"
      "    padding: 6px 16px; font-size: 12px; font-weight: bold; outline: none;"
      "}"
      "QPushButton:hover { border-color: #0284c7; color: #0f172a; background-color: #e2e8f0; }"
      "QPushButton:checked {"
      "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0284c7, stop:1 #0ea5e9);"
      "    color: #ffffff; border: 1px solid #0284c7;"
      "}"
      "QPushButton:disabled { background-color: #f8fafc; color: #94a3b8; border-color: #e2e8f0; }"
      "QComboBox {"
      "    background-color: #ffffff; color: #0f172a; border: 1px solid #cbd5e1; border-radius: 8px; padding: 5px 12px;"
      "}"
    );
  }
}

QString ThemeManager::autoCorrectDialogStyle() const {
  if (isDark()) {
    return QStringLiteral(
      "QDialog { background-color: #090d16; color: #f8fafc; font-family: 'Noto Sans', sans-serif; }"
      "QFrame#headerFrame { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0f172a, stop:0.5 #162033, stop:1 #0f172a); border: 1.5px solid #38bdf8; border-radius: 10px; }"
      "QLabel#lblHeaderTitle { color: #ffffff; font-size: 16px; font-weight: bold; }"
      "QLabel#lblHeaderSub { color: #94a3b8; font-size: 11px; }"
      "QLabel { color: #e2e8f0; font-size: 12px; font-weight: bold; }"
      "QLabel#lblEntries { color: #38bdf8; font-size: 12px; font-weight: bold; padding: 3px; }"
      "QLineEdit { background-color: #111827; color: #f8fafc; border: 1px solid #38bdf8; border-radius: 7px; padding: 6px 10px; font-size: 13px; }"
      "QGroupBox { background-color: #111827; border: 1px solid #1e293b; border-radius: 8px; margin-top: 10px; padding: 14px 10px 8px 10px; color: #38bdf8; font-weight: bold; }"
      "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 10px; padding: 1px 8px; background-color: #0f172a; border: 1px solid #38bdf8; border-radius: 5px; }"
      "QTreeWidget { background-color: #111827; color: #f1f5f9; border: 1px solid #1e293b; border-radius: 8px; alternate-background-color: #0f172a; font-size: 12.5px; }"
      "QHeaderView::section { background-color: #1e293b; color: #38bdf8; padding: 6px; border: none; font-weight: bold; }"
      "QPushButton { background-color: #1e293b; color: #e2e8f0; border: 1px solid #38bdf8; border-radius: 7px; padding: 6px 16px; font-weight: bold; }"
      "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0284c7, stop:1 #38bdf8); color: #030712; }"
      "QPushButton:disabled { background-color: #0f172a; color: #475569; border-color: #1e293b; }"
    );
  } else {
    return QStringLiteral(
      "QDialog { background-color: #f8fafc; color: #0f172a; font-family: 'Noto Sans', sans-serif; }"
      "QFrame#headerFrame { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ffffff, stop:0.5 #f1f5f9, stop:1 #ffffff); border: 1.5px solid #0284c7; border-radius: 10px; }"
      "QLabel#lblHeaderTitle { color: #0f172a; font-size: 16px; font-weight: bold; }"
      "QLabel#lblHeaderSub { color: #475569; font-size: 11px; }"
      "QLabel { color: #1e293b; font-size: 12px; font-weight: bold; }"
      "QLabel#lblEntries { color: #0284c7; font-size: 12px; font-weight: bold; padding: 3px; }"
      "QLineEdit { background-color: #ffffff; color: #0f172a; border: 1px solid #cbd5e1; border-radius: 7px; padding: 6px 10px; font-size: 13px; }"
      "QGroupBox { background-color: #ffffff; border: 1px solid #e2e8f0; border-radius: 8px; margin-top: 10px; padding: 14px 10px 8px 10px; color: #0284c7; font-weight: bold; }"
      "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 10px; padding: 1px 8px; background-color: #ffffff; border: 1px solid #0284c7; border-radius: 5px; }"
      "QTreeWidget { background-color: #ffffff; color: #1e293b; border: 1px solid #e2e8f0; border-radius: 8px; alternate-background-color: #f8fafc; font-size: 12.5px; }"
      "QHeaderView::section { background-color: #f1f5f9; color: #0284c7; padding: 6px; border: none; font-weight: bold; }"
      "QPushButton { background-color: #f1f5f9; color: #1e293b; border: 1px solid #cbd5e1; border-radius: 7px; padding: 6px 16px; font-weight: bold; }"
      "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0284c7, stop:1 #0ea5e9); color: #ffffff; border-color: #0284c7; }"
      "QPushButton:disabled { background-color: #f8fafc; color: #94a3b8; border-color: #e2e8f0; }"
    );
  }
}

QString ThemeManager::popupMenuStyle() const {
  if (isDark()) {
    return QStringLiteral(
      "QMenu {"
      "    background-color: #0f172a; color: #f1f5f9;"
      "    border: 1.5px solid #38bdf8; border-radius: 10px; padding: 6px; font-weight: 500;"
      "}"
      "QMenu::item { padding: 7px 28px 7px 14px; border-radius: 6px; }"
      "QMenu::item:selected { background-color: rgba(56, 189, 248, 0.25); color: #38bdf8; }"
      "QMenu::item:checked { font-weight: bold; color: #00d2ff; }"
      "QMenu::separator { height: 1px; background: rgba(255, 255, 255, 0.12); margin: 4px 6px; }"
    );
  } else {
    return QStringLiteral(
      "QMenu {"
      "    background-color: #ffffff; color: #0f172a;"
      "    border: 1.5px solid #0284c7; border-radius: 10px; padding: 6px; font-weight: 500;"
      "}"
      "QMenu::item { padding: 7px 28px 7px 14px; border-radius: 6px; }"
      "QMenu::item:selected { background-color: rgba(2, 132, 199, 0.15); color: #0284c7; font-weight: 600; }"
      "QMenu::item:checked { font-weight: bold; color: #0284c7; }"
      "QMenu::separator { height: 1px; background: #e2e8f0; margin: 4px 6px; }"
    );
  }
}

QString ThemeManager::iconMenuStyle() const {
  if (isDark()) {
    return QStringLiteral(
      "QMenu {"
      "    background-color: #0f172a; color: #f1f5f9;"
      "    border: 1.5px solid #6366f1; border-radius: 10px; padding: 6px; font-weight: 500;"
      "}"
      "QMenu::item { padding: 7px 28px 7px 14px; border-radius: 6px; }"
      "QMenu::item:selected { background-color: rgba(99, 102, 241, 0.25); color: #a5b4fc; }"
      "QMenu::separator { height: 1px; background: rgba(255, 255, 255, 0.12); margin: 4px 6px; }"
    );
  } else {
    return QStringLiteral(
      "QMenu {"
      "    background-color: #ffffff; color: #0f172a;"
      "    border: 1.5px solid #4f46e5; border-radius: 10px; padding: 6px; font-weight: 500;"
      "}"
      "QMenu::item { padding: 7px 28px 7px 14px; border-radius: 6px; }"
      "QMenu::item:selected { background-color: rgba(79, 70, 229, 0.12); color: #4338ca; font-weight: 600; }"
      "QMenu::separator { height: 1px; background: #e2e8f0; margin: 4px 6px; }"
    );
  }
}

QString ThemeManager::messageBoxStyle() const {
  if (isDark()) {
    return QStringLiteral(
      "QMessageBox { background-color: #0f172a; color: #f8fafc; font-family: 'Noto Sans', sans-serif; }"
      "QLabel { color: #e2e8f0; font-size: 13px; }"
      "QPushButton { background: #1e293b; color: #38bdf8; border: 1px solid #38bdf8; border-radius: 6px; padding: 6px 18px; font-weight: bold; }"
      "QPushButton:hover { background: #0284c7; color: #ffffff; }"
    );
  } else {
    return QStringLiteral(
      "QMessageBox { background-color: #ffffff; color: #0f172a; font-family: 'Noto Sans', sans-serif; }"
      "QLabel { color: #1e293b; font-size: 13px; }"
      "QPushButton { background: #f1f5f9; color: #0284c7; border: 1px solid #0284c7; border-radius: 6px; padding: 6px 18px; font-weight: bold; }"
      "QPushButton:hover { background: #0284c7; color: #ffffff; }"
    );
  }
}

QString ThemeManager::updateDialogStyle() const {
  if (isDark()) {
    return QStringLiteral(
      "QDialog { background-color: #0b1120; color: #f8fafc; font-family: 'Noto Sans', sans-serif; }"
      "QLabel { color: #e2e8f0; font-size: 13px; }"
      "QPushButton { background: #1e293b; color: #38bdf8; border: 1px solid #38bdf8; border-radius: 8px; padding: 8px 18px; font-weight: bold; }"
      "QPushButton:hover { background: #0284c7; color: #ffffff; }"
    );
  } else {
    return QStringLiteral(
      "QDialog { background-color: #ffffff; color: #0f172a; font-family: 'Noto Sans', sans-serif; }"
      "QLabel { color: #1e293b; font-size: 13px; }"
      "QPushButton { background: #f1f5f9; color: #0284c7; border: 1px solid #0284c7; border-radius: 8px; padding: 8px 18px; font-weight: bold; }"
      "QPushButton:hover { background: #0284c7; color: #ffffff; }"
    );
  }
}
