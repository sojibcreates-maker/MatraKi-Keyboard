/*
 *  MatraKi Keyboard — Next-Gen AI Smart Bangla Keyboard
 *  Theme Manager: Light / Dark Mode & Automatic System Detection
 *  Copyright (C) 2026 Md Sojib Hossain
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */

#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QString>
#include <QColor>

class QWidget;
class QTimer;

class ThemeManager : public QObject {
  Q_OBJECT

public:
  enum class Mode {
    Auto,  // Detect from OS
    Dark,  // Force Dark Tech
    Light  // Force Light Classic Minimalist
  };

  static ThemeManager &instance();

  Mode currentMode() const;
  QString currentModeString() const;
  void setMode(Mode mode);
  void setModeString(const QString &modeStr);

  bool isDark() const;
  bool detectSystemDark() const;
  QString detectedModeName() const;

  // Modern Professional Minimalist Tech Stylesheets
  QString topBarStyle() const;
  QString settingsDialogStyle() const;
  QString aboutDialogStyle() const;
  QString aboutFileStyle() const;
  QString layoutViewerStyle() const;
  QString autoCorrectDialogStyle() const;
  QString popupMenuStyle() const;
  QString iconMenuStyle() const;
  QString messageBoxStyle() const;
  QString updateDialogStyle() const;

  // Icon path helpers for TopBar
  QString iconSettings() const;
  QString iconKeyboard() const;
  QString iconKeyboardHide() const;
  QString iconClose() const;
  QString iconDescription() const;

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
