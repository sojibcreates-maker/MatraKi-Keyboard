#!/usr/bin/env python3
import sys
import os
from PyQt5.QtWidgets import QApplication
from PyQt5.QtGui import QImage, QPainter, QColor, QFont, QPen, QBrush, QLinearGradient
from PyQt5.QtCore import Qt, QRectF

app = QApplication(sys.argv)

def create_chart(is_dark=True, is_guide=False):
    w, h = 920, 530
    img = QImage(w, h, QImage.Format_ARGB32_Premultiplied)
    
    if is_dark:
        bg_col = QColor("#0A0B0C")
        card_bg = QColor("#121416")
        card_border = QColor("#2C3033")
        rule_col = QColor("#23272A")
        cell_top = QColor("#212427")
        cell_bottom = QColor("#17191B")
        cell_border = QColor("#383C3F")
        cell_shadow = QColor("#070809")
        cell_highlight = QColor("#3C4043")
        text_bn = QColor("#F2EEE6")
        text_en = QColor("#A3A69F")
        text_en_case = QColor("#D8BC80")
        head_text = QColor("#D2B372")
        muted_text = QColor("#8E9089")
        val_text = QColor("#F2EEE6")
    else:
        bg_col = QColor("#EDE9E0")
        card_bg = QColor("#FAF8F3")
        card_border = QColor("#D8D1C2")
        rule_col = QColor("#E6E0D5")
        cell_top = QColor("#FFFDF9")
        cell_bottom = QColor("#F3EFE6")
        cell_border = QColor("#D6CFC0")
        cell_shadow = QColor("#C9BFAE")
        cell_highlight = QColor("#FFFFFF")
        text_bn = QColor("#1B1B19")
        text_en = QColor("#7A756A")
        text_en_case = QColor("#8A6A2F")
        head_text = QColor("#8A6A2F")
        muted_text = QColor("#6E6A60")
        val_text = QColor("#1B1B19")

    img.fill(bg_col)
    p = QPainter(img)
    p.setRenderHint(QPainter.Antialiasing, True)
    p.setRenderHint(QPainter.TextAntialiasing, True)

    def draw_card(x, y, w, h, title, sub_title=""):
        p.setPen(QPen(card_border, 1.0))
        p.setBrush(QBrush(card_bg))
        p.drawRoundedRect(QRectF(x, y, w, h), 12, 12)

        # Brass registration mark + spaced wordmark.
        p.setPen(Qt.NoPen)
        p.setBrush(head_text)
        p.drawRoundedRect(QRectF(x + 14, y + 13, 4, 4), 1, 1)

        f_title = QFont("Noto Sans", 8, QFont.Bold)
        f_title.setLetterSpacing(QFont.AbsoluteSpacing, 1.2)
        p.setFont(f_title)
        p.setPen(head_text)
        p.drawText(QRectF(x + 25, y + 7, w - 38, 18),
                   Qt.AlignVCenter | Qt.AlignLeft, title.upper())

        if sub_title:
            f_sub = QFont("Noto Sans", 8)
            p.setFont(f_sub)
            p.setPen(muted_text)
            p.drawText(QRectF(x + 25, y + 7, w - 38, 18),
                       Qt.AlignVCenter | Qt.AlignRight, sub_title)

        p.setPen(QPen(rule_col, 1.0))
        p.drawLine(QRectF(x + 14, y + 31, w - 28, 0).topLeft(),
                   QRectF(x + 14, y + 31, w - 28, 0).topRight())

    def draw_key_cell(x, y, w, h, bn_text, en_text, en_color):
        # Keycap underside — the classic depth cue.
        p.setPen(Qt.NoPen)
        p.setBrush(cell_shadow)
        p.drawRoundedRect(QRectF(x, y + 2.2, w, h), 5, 5)

        # Cap face with a soft vertical sheen.
        grad = QLinearGradient(x, y, x, y + h)
        grad.setColorAt(0.0, cell_top)
        grad.setColorAt(1.0, cell_bottom)
        p.setBrush(QBrush(grad))
        p.setPen(QPen(cell_border, 1.0))
        p.drawRoundedRect(QRectF(x, y, w, h), 5, 5)

        # Top-edge highlight.
        p.setPen(QPen(cell_highlight, 1.0))
        p.drawLine(QRectF(x + 5, y + 1.4, w - 10, 0).topLeft(),
                   QRectF(x + 5, y + 1.4, w - 10, 0).topRight())

        f_bn = QFont("Noto Sans Bengali", 11, QFont.Bold)
        p.setFont(f_bn)
        p.setPen(text_bn)
        p.drawText(QRectF(x + 9, y, w - 18, h), Qt.AlignVCenter | Qt.AlignLeft, bn_text)

        f_en = QFont("Noto Sans", 8, QFont.Bold)
        p.setFont(f_en)
        p.setPen(en_color)
        p.drawText(QRectF(x + 9, y, w - 18, h), Qt.AlignVCenter | Qt.AlignRight, en_text)

    if not is_guide:
        # === KEYMAP VIEW ===
        # 1. Vowels & Kar
        draw_card(12, 10, 442, 158, "Vowels & Kar  •  স্বরবর্ণ ও কার", "(Case-Sensitive)")
        vowels = [
            ("অ", "o"), ("আ  া", "a"), ("ই  ি", "i"), ("ঈ  ী", "I / ii"),
            ("উ  ু", "u"), ("ঊ  ূ", "U / uu"), ("ঋ  ৃ", "rri"), ("এ  ে", "e"),
            ("ঐ  ৈ", "OI"), ("ও  ো", "O"), ("ঔ  ৌ", "OU"), ("অ্যা", "aZ")
        ]
        for idx, (bn, en) in enumerate(vowels):
            col = idx % 4
            row = idx // 4
            is_case = any(c.isupper() for c in en)
            c = text_en_case if is_case else text_en
            draw_key_cell(22 + col * 106, 38 + row * 38, 100, 32, bn, en, c)

        # 2. Special Marks & Numbers
        draw_card(466, 10, 442, 158, "Special Marks & Numbers  •  বিশেষ চিহ্ন ও সংখ্যা")
        specials = [
            ("ং", "ng"), ("ঃ", ":"), ("ঁ", "^"), ("ৎ", "t`"),
            ("্ (হসন্ত)", ",,"), ("্য (য-ফলা)", "Z"), ("্র (র-ফলা)", "r"), ("র্ (রেফ)", "rr"),
            ("। (দাড়ি)", "."), ("॥", ".."), ("৳ (টাকা)", "$"), ("০ - ৯", "0 - 9")
        ]
        for idx, (bn, en) in enumerate(specials):
            col = idx % 4
            row = idx // 4
            is_case = any(c.isupper() for c in en)
            c = text_en_case if is_case else text_en
            draw_key_cell(476 + col * 106, 38 + row * 38, 100, 32, bn, en, c)

        # 3. Consonants
        draw_card(12, 178, 544, 342, "Consonants  •  ব্যঞ্জনবর্ণ", "(Gold = Case-Sensitive)")
        consonants = [
            ("ক", "k"), ("খ", "kh"), ("গ", "g"), ("ঘ", "gh"), ("ঙ", "Ng"),
            ("চ", "c"), ("ছ", "ch"), ("জ", "j"), ("ঝ", "jh"), ("ঞ", "NG"),
            ("ট", "T"), ("ঠ", "Th"), ("ড", "D"), ("ঢ", "Dh"), ("ণ", "N"),
            ("ত", "t"), ("থ", "th"), ("দ", "d"), ("ধ", "dh"), ("ন", "n"),
            ("প", "p"), ("ফ", "ph / f"), ("ব", "b"), ("ভ", "bh / v"), ("ম", "m"),
            ("য", "z"), ("র", "r"), ("ল", "l"), ("শ", "sh / S"), ("ষ", "Sh"),
            ("স", "s"), ("হ", "h"), ("ড়", "R"), ("ঢ়", "Rh"), ("য়", "y / Y")
        ]
        for idx, (bn, en) in enumerate(consonants):
            col = idx % 5
            row = idx // 5
            is_case = any(c.isupper() for c in en)
            c = text_en_case if is_case else text_en
            draw_key_cell(22 + col * 106, 206 + row * 43, 100, 35, bn, en, c)

        # 4. Juktakkhor
        draw_card(566, 178, 342, 342, "Juktakkhor  •  প্রয়োজনীয় যুক্তবর্ণ")
        juktakkhor = [
            ("ক্ষ", "kkh"), ("জ্ঞ", "gg"), ("ঞ্চ", "nc"),
            ("ঞ্জ", "nj"), ("ণ্ড", "ND"), ("ন্ত", "nt"),
            ("ন্দ", "nd"), ("ন্ধ", "ndh"), ("ন্ব", "nb"),
            ("ন্ম", "nm"), ("ম্প", "mp"), ("ম্ভ", "mbh"),
            ("ল্ল", "ll"), ("স্ত", "st"), ("স্থ", "sth"),
            ("স্প", "sp"), ("স্ফ", "sph"), ("স্ব", "sb / sw"),
            ("স্ম", "sm"), ("হ্ম", "hm"), ("হ্ণ", "hN")
        ]
        for idx, (bn, en) in enumerate(juktakkhor):
            col = idx % 3
            row = idx // 3
            is_case = any(c.isupper() for c in en)
            c = text_en_case if is_case else text_en
            draw_key_cell(576 + col * 106, 206 + row * 43, 100, 35, bn, en, c)

    else:
        # === AI SMART SHORTCUTS ===
        # 1. Islamic & Chat Snippets
        draw_card(12, 10, 442, 248, "Islamic & Chat Snippets  •  Type ! + keyword")
        snippets = [
            ("!salam", "আসসালামু আলাইকুম ওয়া রাহমাতুল্লাহি ওয়া বারাকাতুহু"),
            ("!olikum", "ওয়া আলাইকুমুস সালাম ওয়া রাহমাতুল্লাহি ওয়া বারাকাতুহু"),
            ("!bismillah", "বিসমিল্লাহির রাহমানির রাহিম (Bangla & Arabic)"),
            ("!alhamdulillah", "আলহামদুলিল্লাহ  •  !subhanallah  •  !mashallah"),
            ("!inshaallah", "ইনশাআল্লাহ  •  !jazakallah  •  !astagfirullah"),
            ("!shuvo / !hbd", "শুভ সকাল / শুভ জন্মদিন / অভিনন্দন বার্তা")
        ]
        for idx, (cmd, desc) in enumerate(snippets):
            y = 38 + idx * 34
            p.setPen(Qt.NoPen)
            p.setBrush(cell_shadow)
            p.drawRoundedRect(QRectF(22, y + 1.8, 422, 28), 5, 5)
            grad = QLinearGradient(22, y, 22, y + 28)
            grad.setColorAt(0.0, cell_top)
            grad.setColorAt(1.0, cell_bottom)
            p.setPen(QPen(cell_border, 1.0))
            p.setBrush(QBrush(grad))
            p.drawRoundedRect(QRectF(22, y, 422, 28), 5, 5)
            
            p.setFont(QFont("Noto Sans", 9, QFont.Bold))
            p.setPen(text_en)
            p.drawText(QRectF(30, y, 110, 28), Qt.AlignVCenter | Qt.AlignLeft, cmd)
            
            p.setFont(QFont("Noto Sans Bengali", 9, QFont.Bold))
            p.setPen(val_text)
            p.drawText(QRectF(142, y, 294, 28), Qt.AlignVCenter | Qt.AlignLeft, desc)

        # 2. Live Math & Unit Converter
        draw_card(466, 10, 442, 248, "Live Math & Unit Converter  •  Type = + expression")
        units = [
            ("=500*15%", "= ৭৫   (Supports +, -, *, /, ^, %)"),
            ("=10km / =5mi", "= ৬.২১ মাইল (১০ কি.মি.)   •   = ৮.০৫ কি.মি."),
            ("=5kg / =500g", "= ১১.০২ পাউন্ড (৫ কেজি)   •   = ১৭.৬৪ আউন্স"),
            ("=100c / =98.6f", "= ২১২° ফারেনহাইট   •   = ৩৭° সেলসিয়াস"),
            ("=5ft / =12in", "= ১৫২.৪ সে.মি. (৬০ ইঞ্চি)   •   = ৩০.৪৮ সে.মি."),
            ("=1bigha / =5katha", "= ৩৩ শতক (২০ কাঠা)   •   = ৮.২৫ শতক")
        ]
        for idx, (cmd, desc) in enumerate(units):
            y = 38 + idx * 34
            p.setPen(Qt.NoPen)
            p.setBrush(cell_shadow)
            p.drawRoundedRect(QRectF(476, y + 1.8, 422, 28), 5, 5)
            grad = QLinearGradient(476, y, 476, y + 28)
            grad.setColorAt(0.0, cell_top)
            grad.setColorAt(1.0, cell_bottom)
            p.setPen(QPen(cell_border, 1.0))
            p.setBrush(QBrush(grad))
            p.drawRoundedRect(QRectF(476, y, 422, 28), 5, 5)
            
            p.setFont(QFont("Noto Sans", 9, QFont.Bold))
            p.setPen(text_en)
            p.drawText(QRectF(484, y, 125, 28), Qt.AlignVCenter | Qt.AlignLeft, cmd)
            
            p.setFont(QFont("Noto Sans Bengali", 9, QFont.Bold))
            p.setPen(val_text)
            p.drawText(QRectF(612, y, 280, 28), Qt.AlignVCenter | Qt.AlignLeft, desc)

        # 3. Smart Dates, Ordinals & Shortcuts
        draw_card(12, 268, 896, 252, "Smart Dates, Ordinals, Currency & Typing Shortcuts")
        tips = [
            ("1la / 2ra / 4tha / 21she", "১লা (পহেলা), ২রা, ৪ঠা, ২১শে — Type number + la, ra, tha, e, or she for Bangla date ordinals"),
            ("1m / 2yo / 3yo / 4rtho", "১ম (প্রথম), ২য় (দ্বিতীয়), ৩য় (তৃতীয়), ৪র্থ — Type number + m, yo, or rtho for rank suffixes"),
            ("500taka / 500tk", "৫০০ টাকা  •  ৳৫০০  •  পাঁচ শত টাকা — Instant currency & number-to-words conversion"),
            ("@today / @time / @date", "আজকের বাংলা ও ইংরেজি তারিখ এবং বর্তমান সময় (Live Date & Time stamps)"),
            (":smile / :heart / :thumbsup", "smile  •  heart  •  thumbsup  •  party  •  fire — Type : + name for the instant emoji picker"),
            ("Double Space  •  1-9 Keys", "Press Space twice quickly for Bangla Dari (। )  •  Press 1-9 to quick-pick !snippets")
        ]
        for idx, (cmd, desc) in enumerate(tips):
            y = 298 + idx * 34
            p.setPen(Qt.NoPen)
            p.setBrush(cell_shadow)
            p.drawRoundedRect(QRectF(22, y + 1.8, 876, 28), 5, 5)
            grad = QLinearGradient(22, y, 22, y + 28)
            grad.setColorAt(0.0, cell_top)
            grad.setColorAt(1.0, cell_bottom)
            p.setPen(QPen(cell_border, 1.0))
            p.setBrush(QBrush(grad))
            p.drawRoundedRect(QRectF(22, y, 876, 28), 5, 5)
            
            p.setFont(QFont("Noto Sans", 9, QFont.Bold))
            p.setPen(text_en_case)
            p.drawText(QRectF(32, y, 195, 28), Qt.AlignVCenter | Qt.AlignLeft, cmd)
            
            p.setFont(QFont("Noto Sans Bengali", 9))
            p.setPen(val_text)
            p.drawText(QRectF(235, y, 655, 28), Qt.AlignVCenter | Qt.AlignLeft, desc)

    p.end()
    return img

out_dir = "/home/sojiblabs/MatraKi-Keyboard/src/frontend/images"
os.makedirs(out_dir, exist_ok=True)

# Generate Dark & Light versions
img_km_dark = create_chart(is_dark=True, is_guide=False)
img_km_dark.save(os.path.join(out_dir, "avro_keymap_dark.png"), "PNG")

img_km_light = create_chart(is_dark=False, is_guide=False)
img_km_light.save(os.path.join(out_dir, "avro_keymap_light.png"), "PNG")

img_sg_dark = create_chart(is_dark=True, is_guide=True)
img_sg_dark.save(os.path.join(out_dir, "avro_smart_guide_dark.png"), "PNG")

img_sg_light = create_chart(is_dark=False, is_guide=True)
img_sg_light.save(os.path.join(out_dir, "avro_smart_guide_light.png"), "PNG")

# Also update avro_keymap_modern.png and avro_smart_guide.png as default dark fallback
img_km_dark.save(os.path.join(out_dir, "avro_keymap_modern.png"), "PNG")
img_sg_dark.save(os.path.join(out_dir, "avro_smart_guide.png"), "PNG")

print("Generated Onyx layout charts.")
