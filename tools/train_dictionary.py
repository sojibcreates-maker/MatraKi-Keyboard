#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""MatraKi training round 2: merge-only (never overwrites existing entries)."""
import json, os, re, shutil

BASE = "/home/sojiblabs/.local/share/matraki-keyboard"
F_TR = f"{BASE}/transliteration.json"
F_CS = f"{BASE}/phonetic-candidate-selection.json"
F_AC = f"{BASE}/autocorrect.json"
F_NW = f"{BASE}/phonetic-next-word.json"
F_SN = f"{BASE}/snippets.json"

# ---------------------------------------------------------------------------
# 1. Verb conjugation generator (only linguistically regular patterns)
# ---------------------------------------------------------------------------
# (banglish_suffix, bangla_suffix, uses_high_stem_for_mutating_roots)
FORMS = [
    ("i", "ি", True), ("o", "ো", False), ("e", "ে", False), ("en", "েন", False),
    ("chi", "ছি", True), ("cho", "ছো", True), ("che", "ছে", True), ("chen", "ছেন", True),
    ("echi", "েছি", True), ("echo", "েছো", True), ("eche", "েছে", True), ("echen", "েছেন", True),
    ("lam", "লাম", True), ("le", "লে", True), ("lo", "লো", True), ("len", "লেন", True),
    ("bo", "ব", True), ("be", "বে", True), ("ben", "বেন", True), ("bi", "বি", True),
    ("tam", "তাম", True), ("te", "তে", True), ("to", "তো", True), ("ten", "তেন", True),
    ("un", "ুন", True),
]
PERFECT = {"echi", "echo", "eche", "echen"}

# Non-mutating roots (অ / এ vowel, same stem everywhere): banglish, bangla
FLAT_ROOTS = [
    ("kor", "কর"), ("dhor", "ধর"), ("bol", "বল"), ("chol", "চল"), ("por", "পড়"),
    ("mor", "মর"), ("bhor", "ভর"), ("gor", "গড়"), ("bos", "বস"), ("sor", "সর"),
    ("dekh", "দেখ"), ("fel", "ফেল"), ("khel", "খেল"), ("bech", "বেচ"), ("thel", "ঠেল"),
    ("jol", "জ্বল"), ("hot", "হট"), ("lor", "লড়"), ("nor", "নড়"),
]
# আ-roots: perfect mutates আ->এ (রাখ -> রেখেছি)
A_ROOTS = [
    ("rakh", "রাখ", "rekh", "রেখ"), ("dak", "ডাক", "dek", "ডেক"), ("mar", "মার", "mer", "মের"),
    ("thak", "থাক", "thek", "থেক"), ("nach", "নাচ", "nech", "নেচ"), ("kat", "কাট", "ket", "কেট"),
    ("jan", "জান", "jen", "জেন"), ("hash", "হাস", "hesh", "হেস"), ("kand", "কাঁদ", "kend", "কেঁদ"),
    ("bach", "বাঁচ", "bech", "বেঁচ"), ("jag", "জাগ", "jeg", "জেগ"), ("chalা", "", "", ""),
]
A_ROOTS = [r for r in A_ROOTS if r[1]]
# ও/এ-roots alternating low/high stem (খোল/খুল, লেখ/লিখ)
ALT_ROOTS = [
    ("khol", "খোল", "khul", "খুল"), ("tol", "তোল", "tul", "তুল"), ("ghor", "ঘোর", "ghur", "ঘুর"),
    ("bojh", "বোঝ", "bujh", "বুঝ"), ("shon", "শোন", "shun", "শুন"), ("khoj", "খোঁজ", "khuj", "খুঁজ"),
    ("oth", "ওঠ", "uth", "উঠ"), ("lekh", "লেখ", "likh", "লিখ"), ("shekh", "শেখ", "shikh", "শিখ"),
    ("pher", "ফের", "phir", "ফির"), ("dol", "দোল", "dul", "দুল"), ("bol", None, None, None),
]
ALT_ROOTS = [r for r in ALT_ROOTS if r[1]]

def gen_verbs():
    out = {}
    for bn_l, bg in FLAT_ROOTS:
        for s_l, s_b, _ in FORMS:
            out[bn_l + s_l] = bg + s_b
    for lo_l, lo_b, pf_l, pf_b in A_ROOTS:
        for s_l, s_b, _ in FORMS:
            if s_l in PERFECT:
                out[pf_l + s_l] = pf_b + s_b
            else:
                out[lo_l + s_l] = lo_b + s_b
    for lo_l, lo_b, hi_l, hi_b in ALT_ROOTS:
        for s_l, s_b, high in FORMS:
            if high:
                out[hi_l + s_l] = hi_b + s_b
            else:
                out[lo_l + s_l] = lo_b + s_b
    return out

# ---------------------------------------------------------------------------
# 1b. Noun suffix generator (explicitly generates -ta, -ti, -gulo, -ke, -der, -er/-r, -te/-e)
# ---------------------------------------------------------------------------
# Vowel-ending nouns (take -র, -য়, -তে, -টা, -টি, -গুলো, -কে, -দের)
VOWEL_NOUNS = [
    ("kotha", "কথা"), ("basha", "বাসা"), ("bari", "বাড়ি"), ("gari", "গাড়ি"),
    ("shomossa", "সমস্যা"), ("somossa", "সমস্যা"), ("porikkha", "পরীক্ষা"),
    ("chinta", "চিন্তা"), ("bhabna", "ভাবনা"), ("vabna", "ভাবনা"),
    ("asha", "আশা"), ("bhalobasha", "ভালোবাসা"), ("valobasha", "ভালোবাসা"),
    ("shikkha", "শিক্ষা"), ("chhatro", "ছাত্র"), ("chhatri", "ছাত্রী"),
    ("bondhu", "বন্ধু"), ("manush", None), ("chele", "ছেলে"), ("meye", "মেয়ে"),
    ("baccha", "বাচ্চা"), ("shishu", "শিশু"), ("baba", "বাবা"), ("maa", "মা"),
    ("bhai", "ভাই"), ("vai", "ভাই"), ("apu", "আপু"), ("dada", "দাদা"), ("didi", "দিদি"),
    ("mama", "মামা"), ("chacha", "চাচা"), ("khala", "খালা"), ("dadu", "দাদু"),
    ("rasta", "রাস্তা"), ("taka", "টাকা"), ("poisha", "পয়সা"), ("khata", "খাতা"),
    ("boi", "বই"), ("chobi", "ছবি"), ("gan", None), ("kobita", "কবিতা"),
    ("golpo", "গল্প"), ("natok", None), ("cinema", "সিনেমা"), ("khela", "খেলা"),
    ("bela", "বেলা"), ("shondha", "সন্ধ্যা"), ("sondha", "সন্ধ্যা"),
    ("brishti", "বৃষ্টি"), ("abohawa", "আবহাওয়া"), ("dhaka", "ঢাকা"),
    ("khulna", "খুলনা"), ("cumilla", "কুমিল্লা"), ("comilla", "কুমিল্লা"),
    ("rajshahi", "রাজশাহী"), ("bangla", "বাংলা"), ("duniya", "দুনিয়া"),
    ("projukti", "প্রযুক্তি"), ("chakri", "চাকরি"), ("byabsha", "ব্যবসা"),
]
VOWEL_NOUNS = [(k, v) for k, v in VOWEL_NOUNS if v]

# Consonant-ending nouns (take -ের, -ে, -টা, -টি, -গুলো, -কে, -দের)
CONS_NOUNS = [
    ("manush", "মানুষ"), ("din", "দিন"), ("rat", "রাত"), ("raat", "রাত"),
    ("mash", "মাস"), ("bochor", "বছর"), ("shokal", "সকাল"), ("sokal", "সকাল"),
    ("dupur", "দুপুর"), ("bikel", "বিকেল"), ("shomoy", "সময়"), ("somoy", "সময়"),
    ("kaj", "কাজ"), ("ghor", "ঘর"), ("dokan", "দোকান"), ("bazar", "বাজার"),
    ("shahar", "শহর"), ("shohor", "শহর"), ("gram", "গ্রাম"), ("desh", "দেশ"),
    ("bangladesh", "বাংলাদেশ"), ("bishwobidyaloy", "বিশ্ববিদ্যালয়"),
    ("kolej", "কলেজ"), ("school", "স্কুল"), ("office", "অফিস"),
    ("hashpatal", "হাসপাতাল"), ("masjid", "মসজিদ"), ("protishthan", "প্রতিষ্ঠান"),
    ("onushthan", "অনুষ্ঠান"), ("poribar", "পরিবার"), ("shorir", "শরীর"),
    ("mon", "মন"), ("jibon", "জীবন"), ("moron", "মরণ"), ("hat", "হাত"),
    ("chokh", "চোখ"), ("mukh", "মুখ"), ("matha", None), ("kan", "কান"),
    ("kolom", "কলম"), ("টেবিল", None), ("mobile", "মোবাইল"), ("computer", "কম্পিউটার"),
    ("internet", "ইন্টারনেট"), ("keyboard", "কীবোর্ড"), ("software", "সফটওয়্যার"),
    ("shikkhok", "শিক্ষক"), ("daktar", "ডাক্তার"), ("doctor", "ডাক্তার"),
    ("pulis", "পুলিশ"), ("police", "পুলিশ"), ("sorkar", "সরকার"), ("shorkar", "সরকার"),
    ("beton", "বেতন"), ("dam", "দাম"), ("khabar", "খাবার"), ("oshudh", "ওষুধ"),
]
CONS_NOUNS = [(k, v) for k, v in CONS_NOUNS if v and re.fullmatch(r"[a-z]+", k)]

def gen_nouns():
    out = {}
    common_suf = [
        ("ta", "টা"), ("ti", "টি"), ("gulo", "গুলো"), ("gula", "গুলা"),
        ("ke", "কে"), ("der", "দের"), ("derke", "দেরকে"),
        ("take", "টাকে"), ("tar", "টার"), ("tir", "টির"), ("gulor", "গুলোর"),
    ]
    for en, bn in VOWEL_NOUNS:
        out[en] = bn
        out[en + "r"] = bn + "র"
        out[en + "te"] = bn + "তে"
        for s_en, s_bn in common_suf:
            out[en + s_en] = bn + s_bn
    for en, bn in CONS_NOUNS:
        out[en] = bn
        out[en + "er"] = bn + "ের"
        out[en + "e"] = bn + "ে"
        for s_en, s_bn in common_suf:
            out[en + s_en] = bn + s_bn
    return out

# ---------------------------------------------------------------------------
# 2. Curated everyday vocabulary (chat, work, study, tech, health, places)
# ---------------------------------------------------------------------------
CURATED = {
    # greetings / chat
    "assalamualaikum": "আসসালামু আলাইকুম", "walaikumassalam": "ওয়া আলাইকুমুস সালাম",
    "dhonnobad": "ধন্যবাদ", "dhonyobad": "ধন্যবাদ", "shuvechha": "শুভেচ্ছা", "shubhechha": "শুভেচ্ছা",
    "ovinondon": "অভিনন্দন", "abhinandan": "অভিনন্দন", "shuvokamona": "শুভকামনা", "shubhokamona": "শুভকামনা",
    "dukkhito": "দুঃখিত", "dukhito": "দুঃখিত", "maf": "মাফ", "khoma": "ক্ষমা", "kkhoma": "ক্ষমা",
    "thikache": "ঠিক আছে", "thikase": "ঠিক আছে", "accha": "আচ্ছা", "acha": "আচ্ছা", "hum": "হুম", "hmm": "হুম",
    "na": "না", "hya": "হ্যাঁ", "ha": "হ্যাঁ", "haa": "হ্যাঁ", "jee": "জি", "ji": "জি",
    "bhai": "ভাই", "vai": "ভাই", "apu": "আপু", "bon": "বোন", "dada": "দাদা", "didi": "দিদি",
    "bondhu": "বন্ধু", "bondhura": "বন্ধুরা", "bandhobi": "বান্ধবী", "priyo": "প্রিয়", "prio": "প্রিয়",
    "shotti": "সত্যি", "sotti": "সত্যি", "shottii": "সত্যি", "mittha": "মিথ্যা", "mitha": "মিথ্যা",
    "ekhon": "এখন", "akhon": "এখন", "ekhoni": "এখনই", "akhoni": "এখনই", "pore": "পরে", "age": "আগে",
    "ajke": "আজকে", "aj": "আজ", "kal": "কাল", "kalke": "কালকে", "porshu": "পরশু", "gotokal": "গতকাল",
    "shokal": "সকাল", "sokal": "সকাল", "dupur": "দুপুর", "bikel": "বিকেল", "shondha": "সন্ধ্যা", "sondha": "সন্ধ্যা",
    "rat": "রাত", "raat": "রাত", "ratri": "রাত্রি", "shopta": "সপ্তাহ", "shoptaho": "সপ্তাহ", "mash": "মাস", "bochor": "বছর",
    "shombar": "সোমবার", "mongolbar": "মঙ্গলবার", "budhbar": "বুধবার", "brihoshpotibar": "বৃহস্পতিবার",
    "shukrobar": "শুক্রবার", "shonibar": "শনিবার", "robibar": "রবিবার",
    # feelings
    "khushi": "খুশি", "khusi": "খুশি", "anondo": "আনন্দ", "dukkho": "দুঃখ", "dukho": "দুঃখ",
    "kosto": "কষ্ট", "koshto": "কষ্ট", "rag": "রাগ", "raag": "রাগ", "voy": "ভয়", "bhoy": "ভয়",
    "chinta": "চিন্তা", "tension": "টেনশন", "monkharap": "মন খারাপ", "mon": "মন", "valolage": "ভালো লাগে",
    "bhalolage": "ভালো লাগে", "valobashi": "ভালোবাসি", "bhalobashi": "ভালোবাসি", "miss": "মিস",
    # family / people
    "abba": "আব্বা", "abbu": "আব্বু", "amma": "আম্মা", "ammu": "আম্মু", "baba": "বাবা", "maa": "মা",
    "nana": "নানা", "nani": "নানি", "dadu": "দাদু", "dadi": "দাদি", "chacha": "চাচা", "chachi": "চাচি",
    "mama": "মামা", "mami": "মামি", "khala": "খালা", "khalu": "খালু", "fupu": "ফুপু", "phupu": "ফুপু",
    "chele": "ছেলে", "meye": "মেয়ে", "baccha": "বাচ্চা", "shishu": "শিশু", "manush": "মানুষ", "lok": "লোক",
    "sthri": "স্ত্রী", "shami": "স্বামী", "poribar": "পরিবার", "attiyo": "আত্মীয়", "protibeshi": "প্রতিবেশী",
    # study / work
    "porashuna": "পড়াশোনা", "porashona": "পড়াশোনা", "class": "ক্লাস", "porikkha": "পরীক্ষা",
    "fol": "ফল", "result": "রেজাল্ট", "bishwobidyaloy": "বিশ্ববিদ্যালয়", "biswabidyaloy": "বিশ্ববিদ্যালয়",
    "kolej": "কলেজ", "school": "স্কুল", "iskul": "স্কুল", "shikkhok": "শিক্ষক", "shikkhika": "শিক্ষিকা",
    "chhatro": "ছাত্র", "chatro": "ছাত্র", "chatri": "ছাত্রী", "boi": "বই", "khata": "খাতা", "kolom": "কলম",
    "office": "অফিস", "chakri": "চাকরি", "kaj": "কাজ", "kajer": "কাজের", "beton": "বেতন", "meeting": "মিটিং",
    "proshashon": "প্রশাসন", "protishthan": "প্রতিষ্ঠান", "byabsha": "ব্যবসা", "baboshay": "ব্যবসায়",
    "onushthan": "অনুষ্ঠান", "shomoy": "সময়", "somoy": "সময়", "shujog": "সুযোগ", "sujog": "সুযোগ",
    "shohojogita": "সহযোগিতা", "sohojogita": "সহযোগিতা", "shahajjo": "সাহায্য", "sahajjo": "সাহায্য",
    "dorkar": "দরকার", "proyojon": "প্রয়োজন", "joruri": "জরুরি", "gurutto": "গুরুত্ব", "guruttopurno": "গুরুত্বপূর্ণ",
    "obossoi": "অবশ্যই", "oboshshoi": "অবশ্যই", "hoyto": "হয়তো", "shombhoboto": "সম্ভবত", "nishchoi": "নিশ্চয়ই",
    "projukti": "প্রযুক্তি", "computer": "কম্পিউটার", "mobile": "মোবাইল", "internet": "ইন্টারনেট",
    "software": "সফটওয়্যার", "keyboard": "কীবোর্ড", "website": "ওয়েবসাইট", "password": "পাসওয়ার্ড",
    "download": "ডাউনলোড", "upload": "আপলোড", "message": "মেসেজ", "msg": "মেসেজ", "phone": "ফোন",
    # daily life / food / health
    "khabar": "খাবার", "bhat": "ভাত", "vat": "ভাত", "mach": "মাছ", "mangsho": "মাংস", "dal": "ডাল",
    "pani": "পানি", "cha": "চা", "nasta": "নাস্তা", "rutи": "", "ruti": "রুটি", "dim": "ডিম", "dudh": "দুধ",
    "bazar": "বাজার", "dokan": "দোকান", "dam": "দাম", "taka": "টাকা", "poisha": "পয়সা",
    "bari": "বাড়ি", "basha": "বাসা", "ghor": "ঘর", "rasta": "রাস্তা", "gari": "গাড়ি", "bus": "বাস",
    "ostho": "অসুস্থ", "oshustho": "অসুস্থ", "shustho": "সুস্থ", "jor": "জ্বর", "jwor": "জ্বর",
    "doctor": "ডাক্তার", "daktar": "ডাক্তার", "oshudh": "ওষুধ", "osudh": "ওষুধ", "hashpatal": "হাসপাতাল",
    "brishti": "বৃষ্টি", "rod": "রোদ", "thanda": "ঠান্ডা", "gorom": "গরম", "abohawa": "আবহাওয়া",
    # adjectives / adverbs
    "bhalo": "ভালো", "valo": "ভালো", "kharap": "খারাপ", "sundor": "সুন্দর", "shundor": "সুন্দর",
    "boro": "বড়", "choto": "ছোট", "notun": "নতুন", "puraton": "পুরাতন", "purono": "পুরোনো",
    "shohoj": "সহজ", "sohoj": "সহজ", "kothin": "কঠিন", "druto": "দ্রুত", "taratari": "তাড়াতাড়ি",
    "aste": "আস্তে", "beshi": "বেশি", "besi": "বেশি", "kom": "কম", "onek": "অনেক", "ektu": "একটু",
    "shob": "সব", "sob": "সব", "shobkichu": "সবকিছু", "kichu": "কিছু", "kisu": "কিছু", "kichui": "কিছুই",
    "abar": "আবার", "aro": "আরও", "arо": "", "shudhu": "শুধু", "sudhu": "শুধু", "kintu": "কিন্তু",
    "tobe": "তবে", "jodi": "যদি", "tahole": "তাহলে", "tai": "তাই", "karon": "কারণ", "jonno": "জন্য",
    "sathe": "সাথে", "shathe": "সাথে", "moddhe": "মধ্যে", "upore": "উপরে", "niche": "নিচে", "pashe": "পাশে",
    "ekshathe": "একসাথে", "eksathe": "একসাথে", "protidin": "প্রতিদিন", "majhe": "মাঝে", "shesh": "শেষ",
    "ghum": "ঘুম", "ghumabo": "ঘুমাব", "ghumacchi": "ঘুমাচ্ছি", "ghumaccho": "ঘুমাচ্ছো",
    "ghumiye": "ঘুমিয়ে", "ghumiyechi": "ঘুমিয়েছি", "purano": "পুরানো",
    # places
    "dhaka": "ঢাকা", "chottogram": "চট্টগ্রাম", "chittagong": "চট্টগ্রাম", "rajshahi": "রাজশাহী",
    "khulna": "খুলনা", "sylhet": "সিলেট", "barishal": "বরিশাল", "rangpur": "রংপুর", "mymensingh": "ময়মনসিংহ",
    "comilla": "কুমিল্লা", "cumilla": "কুমিল্লা", "bangladesh": "বাংলাদেশ", "bangla": "বাংলা",
    # religion / culture
    "namaz": "নামাজ", "namaj": "নামাজ", "roja": "রোজা", "eid": "ঈদ", "jumma": "জুমা", "masjid": "মসজিদ",
    "quran": "কুরআন", "dua": "দোয়া", "doa": "দোয়া", "puja": "পূজা", "pohela": "পহেলা", "boishakh": "বৈশাখ",
    "matraki": "মাত্রা-কী",
}
CURATED = {k: v for k, v in CURATED.items() if v and re.fullmatch(r"[a-z]+", k)}

# ---------------------------------------------------------------------------
# 3. Next-word bigrams (context -> {next: weight})
# ---------------------------------------------------------------------------
BIGRAMS = {
    "আজ": {"আমি": 20, "অনেক": 18, "কী": 16, "সকালে": 15, "রাতে": 14, "থেকে": 13},
    "আজকে": {"আমি": 20, "অনেক": 16, "কী": 15, "আবহাওয়া": 12},
    "কাল": {"সকালে": 20, "থেকে": 18, "আমি": 16, "দেখা": 14, "রাতে": 13},
    "দয়া": {"করে": 30},
    "মন": {"খারাপ": 25, "ভালো": 22, "দিয়ে": 15, "থেকে": 14},
    "ভালো": {"আছি": 25, "আছেন": 22, "থাকবেন": 20, "লাগছে": 18, "লাগে": 16, "হয়েছে": 14},
    "খুশি": {"হলাম": 22, "হয়েছি": 20, "হবো": 15},
    "অভিনন্দন": {"ও": 18, "আপনাকে": 20, "তোমাকে": 16},
    "শুভেচ্ছা": {"ও": 15, "রইল": 22, "জানাই": 20},
    "দুঃখিত": {"ভাই": 18, "আমি": 15, "দেরি": 14},
    "ঠিক": {"আছে": 30, "আছি": 15, "হবে": 18, "করে": 14},
    "আসসালামু": {"আলাইকুম": 30},
    "ওয়া": {"আলাইকুমুস": 25, "রাহমাতুল্লাহি": 20},
    "কোথায়": {"আছো": 22, "আছেন": 20, "যাচ্ছো": 16, "যাবেন": 15},
    "কখন": {"আসবে": 20, "আসবেন": 18, "যাবে": 16, "হবে": 15},
    "কেন": {"এমন": 18, "করলে": 16, "বললে": 14},
    "আমার": {"নাম": 22, "মনে": 18, "কাছে": 16, "বাড়ি": 14, "জন্য": 15},
    "তোমার": {"জন্য": 22, "নাম": 18, "কথা": 16, "সাথে": 15},
    "আপনার": {"জন্য": 22, "নাম": 18, "সাথে": 16, "কাছে": 15},
    "অফিসে": {"আছি": 22, "যাচ্ছি": 20, "যাব": 15},
    "বাসায়": {"আছি": 22, "যাচ্ছি": 18, "ফিরছি": 16},
    "কথা": {"বলব": 20, "হবে": 18, "বলো": 15, "বলেন": 14},
    "একটু": {"পরে": 22, "অপেক্ষা": 18, "বেশি": 14, "কম": 12},
    "সব": {"ঠিক": 22, "কিছু": 18, "সময়": 16},
    "আল্লাহ": {"হাফেজ": 25, "ভরসা": 20, "আপনার": 15},
    "জুম্মা": {"মোবারক": 30},
    "ঈদ": {"মোবারক": 30},
    "রমজান": {"মোবারক": 28, "কারিম": 20},
    "নতুন": {"বছর": 20, "কিছু": 15},
    "শুভ": {"জন্মদিন": 22, "সকাল": 20, "রাত্রি": 18, "সন্ধ্যা": 15, "বিবাহ": 12},
}

SNIPPETS = {
    "!shuvo": "শুভ সকাল! দিনটি আপনার ভালো কাটুক।",
    "!mashallah": "মাশাআল্লাহ",
    "!ameen": "আমিন",
    "!allahhafez": "আল্লাহ হাফেজ",
    "!ramadan": "রমজান মোবারক!",
    "!noboborsho": "শুভ নববর্ষ!",
    "!getwell": "দ্রুত সুস্থ হয়ে উঠুন, অনেক দোয়া রইল।",
    "!wait": "একটু অপেক্ষা করুন, আমি দেখছি।",
}

def load(p):
    with open(p, encoding="utf-8") as f:
        return json.load(f)

def save(p, d):
    tmp = p + ".tmp"
    with open(tmp, "w", encoding="utf-8") as f:
        json.dump(d, f, ensure_ascii=False, indent=2)
    os.replace(tmp, p)

def main():
    for p in (F_TR, F_CS, F_AC, F_NW, F_SN):
        shutil.copy2(p, p + ".pre-v1.bak")

    tr, cs, ac, nw, sn = load(F_TR), load(F_CS), load(F_AC), load(F_NW), load(F_SN)

    # Bug fix: remove corrupted mixed-script keys (e.g. "shobaiকে")
    bad_re = re.compile(r"[a-zA-Z]+[\u0980-\u09FF]")
    removed = 0
    for d in (tr, cs, ac):
        for k in [k for k in d if bad_re.search(k)]:
            del d[k]; removed += 1

    new_words = {**gen_verbs(), **gen_nouns(), **CURATED}
    stats = {}
    for name, d in (("transliteration", tr), ("candidate-selection", cs), ("autocorrect", ac)):
        before = len(d)
        for k, v in new_words.items():
            d.setdefault(k, v)        # merge only: never overwrite existing / user-learned
        stats[name] = (before, len(d))

    nw_before = len(nw)
    for ctx, nexts in BIGRAMS.items():
        cur = nw.setdefault(ctx, {})
        for w, wt in nexts.items():
            cur[w] = max(cur.get(w, 0), wt)
    sn_before = len(sn)
    for k, v in SNIPPETS.items():
        sn.setdefault(k, v)

    save(F_TR, tr); save(F_CS, cs); save(F_AC, ac); save(F_NW, nw); save(F_SN, sn)
    shutil.copy2(F_CS, F_CS + ".bak")

    print(f"removed corrupt keys: {removed}")
    print(f"generated verb forms: {len(gen_verbs())}, noun forms: {len(gen_nouns())}, curated words: {len(CURATED)}")
    for n, (b, a) in stats.items():
        print(f"{n}: {b} -> {a} (+{a-b})")
    print(f"next-word contexts: {nw_before} -> {len(nw)}")
    print(f"snippets: {sn_before} -> {len(sn)}")

if __name__ == "__main__":
    main()
