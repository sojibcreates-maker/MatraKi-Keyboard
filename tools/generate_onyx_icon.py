#!/usr/bin/env python3
"""Render the Onyx / Brass MatraKi app icon.

The mark is a miniature mechanical keycap: an anodised brass tile, an
ivory cap face with a soft sheen, and the Bengali "ম" legend. Run with:

    QT_QPA_PLATFORM=offscreen python3 tools/generate_onyx_icon.py
"""

import os
import sys

from PyQt5.QtCore import QPointF, QRectF, Qt
from PyQt5.QtGui import (QColor, QFont, QFontDatabase, QImage, QLinearGradient,
                         QPainter, QPen)
from PyQt5.QtWidgets import QApplication

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, "src", "frontend", "images")
FONT = os.path.join(ROOT, "src", "frontend", "fonts", "TiroBangla-Regular.ttf")

app = QApplication(sys.argv)
if os.path.exists(FONT):
    QFontDatabase.addApplicationFont(FONT)


def render(size: int) -> QImage:
    scale = 4
    s = size * scale
    img = QImage(s, s, QImage.Format_ARGB32_Premultiplied)
    img.fill(Qt.transparent)

    p = QPainter(img)
    p.setRenderHint(QPainter.Antialiasing, True)
    p.setRenderHint(QPainter.TextAntialiasing, True)

    box = QRectF(0, 0, s, s)
    radius = s * 0.235

    tile = QLinearGradient(box.topLeft(), box.bottomLeft())
    tile.setColorAt(0.0, QColor("#DCC288"))
    tile.setColorAt(1.0, QColor("#B08E4A"))
    p.setPen(QPen(QColor("#8E6F31"), s * 0.006))
    p.setBrush(tile)
    p.drawRoundedRect(box.adjusted(s * 0.02, s * 0.02, -s * 0.02, -s * 0.02),
                      radius, radius)

    # Cap geometry.
    kw, kh = s * 0.60, s * 0.56
    kx = (s - kw) / 2
    ky = s * 0.215
    kr = s * 0.125

    # Underside.
    p.setPen(Qt.NoPen)
    p.setBrush(QColor("#6F521F"))
    p.drawRoundedRect(QRectF(kx, ky + s * 0.052, kw, kh), kr, kr)

    # Ivory face.
    cap = QLinearGradient(QPointF(kx, ky), QPointF(kx, ky + kh))
    cap.setColorAt(0.0, QColor("#FFFDF7"))
    cap.setColorAt(1.0, QColor("#E7DFCE"))
    p.setBrush(cap)
    p.setPen(QPen(QColor(0, 0, 0, 30), s * 0.012))
    p.drawRoundedRect(QRectF(kx, ky, kw, kh), kr, kr)

    # Top-edge highlight.
    p.setPen(QPen(QColor(255, 255, 255, 200), s * 0.014))
    p.drawLine(QPointF(kx + kr, ky + s * 0.017),
               QPointF(kx + kw - kr, ky + s * 0.017))

    # "ম" legend.
    font = QFont("Tiro Bangla")
    font.setPixelSize(int(s * 0.36))
    font.setBold(True)
    p.setFont(font)
    p.setPen(QColor("#7A5A22"))
    p.drawText(QRectF(kx, ky - s * 0.01, kw, kh), Qt.AlignCenter, "ম")

    # Status lamp.
    dot_r = s * 0.035
    p.setBrush(QColor("#C4442F"))
    p.setPen(QPen(QColor(255, 255, 255, 120), s * 0.012))
    p.drawEllipse(QPointF(kx + kw - s * 0.085, ky + s * 0.095), dot_r, dot_r)

    p.end()
    return img.scaled(size, size, Qt.KeepAspectRatio, Qt.SmoothTransformation)


for name, size in (("MatraKi.png", 512), ("icon.png", 128)):
    render(size).save(os.path.join(OUT, name), "PNG")
    print("wrote", name)
