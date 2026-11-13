/**
 * MatraKi v1.0.0 (AI Smart Edition) — Complete Interactive Live Web Keyboard Engine
 * Mirrors every feature of the C++17 & Rust desktop engine:
 *  1. 20,535+ Pre-Trained Words + Avro Phonetic Algorithmic Transliterator
 *  2. Smart Agglutinative Suffix Stemmer (-গুলো, -গুলা, -দের, -দেরকে, -টা, -টি, -টাকে, -টার, -টির, -কে, -তে, -ের)
 *  3. Context-Aware Next-Word Prediction (130+ bigram contexts) + Subject-Verb Concordance Ranking
 *  4. Bangla Academy Spelling & Grammar Auto-Fix (ণ-ত্ব, ষ-ত্ব, প্রমিত বানান ও চলিত রূপ)
 *  5. Live In-Keyboard Math Calculator (=500*15%, =2^10, =(120+80)*3)
 *  6. Smart Unit Converter (=km, =mi, =m, =cm, =in, =ft, =kg, =gm, =lb, =l, =c, =f, =acre, =bigha, =katha, =shotok)
 *  7. 42+ Instant Chat & Islamic Snippets (!salam, !olikum, !bismillah, !alham, !insha, !shokal, !eid, !jumma...)
 *  8. Live Bengali Date & Time (@today, @now, !date, !time)
 *  9. All 31 Bengali Date Ordinals (1la..31she) & 20 Ranks (1m..20sh)
 * 10. Currency & Number-to-Bengali-Words Converter (500tk, 1250taka, 0..99,99,99,999)
 * 11. 45+ Emoji & Symbol Picker (:smile, :hasi, :love, :fire, :bd, :dua, :taka...)
 * 12. Hybrid English Mode (Facebook, Google, YouTube, GitHub, Linux, Deepin, Python, ChatGPT, Gemini, bKash, MatraKi)
 * 13. Double-Space to Bengali Dari (। )
 * 14. 6 Switchable Keyboard Layouts (Avro Phonetic, Probhat, National/Jatiya, Munir Optima, Borno, Avro Easy) + On-Screen Virtual Keyboard
 */

(function () {
  "use strict";

  const BN_DIGITS = ["০", "১", "২", "৩", "৪", "৫", "৬", "৭", "৮", "৯"];
  const BN_MONTHS = [
    "জানুয়ারি", "ফেব্রুয়ারি", "মার্চ", "এপ্রিল", "মে", "জুন",
    "জুলাই", "আগস্ট", "সেপ্টেম্বর", "অক্টোবর", "নভেম্বর", "ডিসেম্বর"
  ];

  const BN_UNITS_0_99 = [
    "", "এক", "দুই", "তিন", "চার", "পাঁচ", "ছয়", "সাত", "আট", "নয়",
    "দশ", "এগারো", "বারো", "তেরো", "চৌদ্দ", "পনেরো", "ষোলো", "সতেরো", "আঠারো", "উনিশ",
    "বিশ", "একুশ", "বাইশ", "তেইশ", "চব্বিশ", "পঁচিশ", "ছাব্বিশ", "সাতাশ", "আটাশ", "উনত্রিশ",
    "ত্রিশ", "একত্রিশ", "বত্রিশ", "তেত্রিশ", "চৌত্রিশ", "পঁয়ত্রিশ", "ছত্রিশ", "সাঁইত্রিশ", "আটত্রিশ", "উনচল্লিশ",
    "চল্লিশ", "একচল্লিশ", "বিয়াল্লিশ", "তেতাল্লিশ", "চুয়াল্লিশ", "পঁয়তাল্লিশ", "ছেচল্লিশ", "সাতচল্লিশ", "আটচল্লিশ", "উনপঞ্চাশ",
    "পঞ্চাশ", "একান্ন", "বায়ান্ন", "তিপ্পান্ন", "চুয়ান্ন", "পঞ্চান্ন", "ছাপ্পান্ন", "সাতান্ন", "আটান্ন", "উনষাট",
    "ষাট", "একষট্টি", "বাষট্টি", "তেষট্টি", "চৌষট্টি", "পঁয়ষট্টি", "ছেষট্টি", "সাতষট্টি", "আটষট্টি", "উনসত্তর",
    "সত্তর", "একাত্তর", "বাহাত্তর", "তিয়াত্তর", "চুয়াত্তর", "পঁচাত্তর", "ছিয়াত্তর", "সাতাত্তর", "আটাত্তর", "ঊনআশি",
    "আশি", "একাশি", "বিরাশি", "তিরাশি", "চুরাশি", "পঁচাশি", "ছিয়াশি", "সাতাশি", "অষ্টআশি", "ঊননব্বই",
    "নব্বই", "একানব্বই", "বিরানব্বই", "তিরানব্বই", "চুরানব্বই", "পঁচানব্বই", "ছিয়ানব্বই", "সাতানব্বই", "আটানব্বই", "নিরানব্বই"
  ];

  function toBnDigits(str) {
    return String(str).replace(/[0-9]/g, (d) => BN_DIGITS[d.charCodeAt(0) - 48]);
  }

  function formatClean(val) {
    if (!isFinite(val)) return "";
    let s = Number(val).toFixed(2);
    s = s.replace(/\.?0+$/, "");
    return s;
  }

  function numberToBnWords(n) {
    n = Math.floor(Number(n));
    if (!isFinite(n) || n < 0 || n > 999999999) return "";
    if (n === 0) return "শূন্য";
    let res = [];
    const crore = Math.floor(n / 10000000); n %= 10000000;
    const lakh = Math.floor(n / 100000); n %= 100000;
    const thousand = Math.floor(n / 1000); n %= 1000;
    const hundred = Math.floor(n / 100); n %= 100;
    const rem = n;
    if (crore > 0 && crore < 100) res.push(BN_UNITS_0_99[crore] + " কোটি");
    if (lakh > 0) res.push(BN_UNITS_0_99[lakh] + " লাখ");
    if (thousand > 0) res.push(BN_UNITS_0_99[thousand] + " হাজার");
    if (hundred > 0) res.push(BN_UNITS_0_99[hundred] + " শত");
    if (rem > 0) res.push(BN_UNITS_0_99[rem]);
    return res.join(" ");
  }

  function getBengaliDate() {
    const now = new Date();
    return toBnDigits(now.getDate()) + "ই " + BN_MONTHS[now.getMonth()] + ", " + toBnDigits(now.getFullYear());
  }

  function getBengaliTime() {
    const now = new Date();
    let h = now.getHours();
    const m = now.getMinutes();
    const pm = h >= 12;
    if (h > 12) h -= 12;
    if (h === 0) h = 12;
    const sh = (h < 10 ? "0" : "") + h;
    const sm = (m < 10 ? "0" : "") + m;
    return toBnDigits(sh) + ":" + toBnDigits(sm) + (pm ? " অপরাহ্ন" : " পূর্বাহ্ন");
  }

  // Safe Math Evaluator (+, -, *, /, %, ^, parentheses, percentages)
  function evaluateMath(expr) {
    let s = expr.trim();
    if (!s || !/^[0-9+\-*/%^().\s]+$/.test(s)) return null;
    // Transform trailing number% into (number/100)
    s = s.replace(/(\d+(?:\.\d+)?)%/g, "($1/100)");
    s = s.replace(/\^/g, "**");
    try {
      const fn = new Function("return (" + s + ");");
      const val = fn();
      if (typeof val === "number" && isFinite(val)) return val;
    } catch (e) {}
    return null;
  }

  // 16 Unit Conversions (=km, =mi, =m, =cm, =in, =ft, =kg, =gm, =lb, =l, =c, =f, =acre, =bigha, =katha, =shotok)
  function tryUnitConversion(expr) {
    const m = expr.trim().toLowerCase().match(/^(\d+(?:\.\d+)?)\s*([a-z]+)$/);
    if (!m) return null;
    const val = parseFloat(m[1]);
    const u = m[2];
    if (!isFinite(val)) return null;
    const bnV = toBnDigits(formatClean(val));
    const out = [];
    const add = (disp, commit) => out.push({ display: disp, commitText: commit, isExclusive: true });

    if (u === "km") {
      const mi = toBnDigits(formatClean(val * 0.621371));
      const mt = toBnDigits(formatClean(val * 1000));
      add(`${bnV} কিমি = ${mi} মাইল`, `${mi} মাইল`);
      add(`${bnV} কিমি = ${mt} মিটার`, `${mt} মিটার`);
      add(`${bnV} কিলোমিটার`, `${bnV} কিলোমিটার`);
    } else if (u === "mi" || u === "mile" || u === "miles") {
      const km = toBnDigits(formatClean(val * 1.60934));
      add(`${bnV} মাইল = ${km} কিমি`, `${km} কিলোমিটার`);
    } else if (u === "m" || u === "meter") {
      const ft = toBnDigits(formatClean(val * 3.28084));
      const cm = toBnDigits(formatClean(val * 100));
      add(`${bnV} মিটার = ${ft} ফুট`, `${ft} ফুট`);
      add(`${bnV} মিটার = ${cm} সেমি`, `${cm} সেন্টিমিটার`);
    } else if (u === "cm") {
      const inc = toBnDigits(formatClean(val * 0.393701));
      add(`${bnV} সেমি = ${inc} ইঞ্চি`, `${inc} ইঞ্চি`);
    } else if (u === "in" || u === "inch") {
      const cm = toBnDigits(formatClean(val * 2.54));
      add(`${bnV} ইঞ্চি = ${cm} সেমি`, `${cm} সেন্টিমিটার`);
    } else if (u === "ft" || u === "foot") {
      const cm = toBnDigits(formatClean(val * 30.48));
      const inc = toBnDigits(formatClean(val * 12));
      add(`${bnV} ফুট = ${cm} সেমি`, `${cm} সেন্টিমিটার`);
      add(`${bnV} ফুট = ${inc} ইঞ্চি`, `${inc} ইঞ্চি`);
    } else if (u === "kg") {
      const gm = toBnDigits(formatClean(val * 1000));
      const lbs = toBnDigits(formatClean(val * 2.20462));
      add(`${bnV} কেজি = ${gm} গ্রাম`, `${gm} গ্রাম`);
      add(`${bnV} কেজি = ${lbs} পাউন্ড`, `${lbs} পাউন্ড`);
      add(`${bnV} কেজি`, `${bnV} কেজি`);
    } else if (u === "g" || u === "gm" || u === "gram") {
      const kg = toBnDigits(formatClean(val / 1000));
      add(`${bnV} গ্রাম = ${kg} কেজি`, `${kg} কেজি`);
      add(`${bnV} গ্রাম`, `${bnV} গ্রাম`);
    } else if (u === "lb" || u === "lbs") {
      const kg = toBnDigits(formatClean(val * 0.453592));
      add(`${bnV} পাউন্ড = ${kg} কেজি`, `${kg} কেজি`);
    } else if (u === "l" || u === "liter" || u === "litre") {
      const ml = toBnDigits(formatClean(val * 1000));
      add(`${bnV} লিটার = ${ml} মিলি`, `${ml} মিলিলিটার`);
      add(`${bnV} লিটার`, `${bnV} লিটার`);
    } else if (u === "c") {
      const f = toBnDigits(formatClean((val * 9 / 5) + 32));
      add(`${bnV}°C = ${f}°F`, `${f}° ফারেনহাইট`);
      add(`${bnV}° সেলসিয়াস`, `${bnV}° সেলসিয়াস`);
    } else if (u === "f") {
      const c = toBnDigits(formatClean((val - 32) * 5 / 9));
      add(`${bnV}°F = ${c}°C`, `${c}° সেলসিয়াস`);
      add(`${bnV}° ফারেনহাইট`, `${bnV}° ফারেনহাইট`);
    } else if (u === "acre") {
      const bigha = toBnDigits(formatClean(val * 3.0303));
      const shotok = toBnDigits(formatClean(val * 100));
      add(`${bnV} একর = ${shotok} শতক (${bigha} বিঘা)`, `${shotok} শতক`);
    } else if (u === "bigha") {
      const shotok = toBnDigits(formatClean(val * 33));
      const katha = toBnDigits(formatClean(val * 20));
      add(`${bnV} বিঘা = ${shotok} শতক (${katha} কাঠা)`, `${shotok} শতক`);
    } else if (u === "katha") {
      const shotok = toBnDigits(formatClean(val * 1.65));
      const sqft = toBnDigits(formatClean(val * 720));
      add(`${bnV} কাঠা = ${shotok} শতক (${sqft} বর্গফুট)`, `${shotok} শতক`);
    } else if (u === "shotok") {
      const sqft = toBnDigits(formatClean(val * 435.6));
      add(`${bnV} শতক = ${sqft} বর্গফুট`, `${sqft} বর্গফুট`);
    } else if (u === "kmh" || u === "km/h") {
      const mps = toBnDigits(formatClean(val / 3.6));
      const mph = toBnDigits(formatClean(val * 0.621371));
      add(`${bnV} কিমি/ঘণ্টা = ${mps} মি/সেকেন্ড (${mph} মাইল/ঘণ্টা)`, `${mps} মি/সেকেন্ড`);
    } else if (u === "mps" || u === "m/s") {
      const kmh = toBnDigits(formatClean(val * 3.6));
      add(`${bnV} মি/সেকেন্ড = ${kmh} কিমি/ঘণ্টা`, `${kmh} কিমি/ঘণ্টা`);
    } else if (u === "mb") {
      const gb = toBnDigits(formatClean(val / 1024));
      const kb = toBnDigits(formatClean(val * 1024));
      add(`${bnV} MB = ${gb} GB (${kb} KB)`, `${gb} জিবি`);
    } else if (u === "gb") {
      const mb = toBnDigits(formatClean(val * 1024));
      const tb = toBnDigits(formatClean(val / 1024));
      add(`${bnV} GB = ${mb} MB (${tb} TB)`, `${mb} এমবি`);
    } else if (u === "tb") {
      const gb = toBnDigits(formatClean(val * 1024));
      add(`${bnV} TB = ${gb} GB`, `${gb} জিবি`);
    } else if (u === "usd" || u === "$") {
      const bdt = toBnDigits(formatClean(val * 122));
      add(`${bnV} USD = ৳${bdt} টাকা (আনুমানিক)`, `${bdt} টাকা`);
    } else if (u === "eur" || u === "euro") {
      const bdt = toBnDigits(formatClean(val * 132));
      add(`${bnV} EUR = ৳${bdt} টাকা (আনুমানিক)`, `${bdt} টাকা`);
    } else if (u === "sar" || u === "riyal") {
      const bdt = toBnDigits(formatClean(val * 32.5));
      add(`${bnV} SAR = ৳${bdt} টাকা (আনুমানিক)`, `${bdt} টাকা`);
    } else if (u === "aed" || u === "dirham") {
      const bdt = toBnDigits(formatClean(val * 33.2));
      add(`${bnV} AED = ৳${bdt} টাকা (আনুমানিক)`, `${bdt} টাকা`);
    }
    return out.length ? out : null;
  }

  // Built-in Snippets (!salam, !olikum, !bismillah, etc.)
  const BUILTIN_SNIPPETS = {
    "!salam": [
      ["আসসালামু আলাইকুম ওয়া রাহমাতুল্লাহি ওয়া বারাকাতুহু (পূর্ণাঙ্গ)", "আসসালামু আলাইকুম ওয়া রাহমাতুল্লাহি ওয়া বারাকাতুহু"],
      ["আসসালামু আলাইকুম (সংক্ষিপ্ত)", "আসসালামু আলাইকুম"],
      ["السَّلَامُ عَلَيْكُمْ وَرَحْمَةُ ٱللَّهِ وَبَرَكَاتُهُ (আরবি)", "السَّلَامُ عَلَيْكُمْ وَرَحْمَةُ ٱللَّهِ وَبَرَكَاتُهُ"]
    ],
    "!olikum": [
      ["ওয়া আলাইকুমুস সালাম ওয়া রাহমাতুল্লাহি ওয়া বারাকাতুহু (পূর্ণাঙ্গ)", "ওয়া আলাইকুমুস সালাম ওয়া রাহমাতুল্লাহি ওয়া বারাকাতুহু"],
      ["ওয়া আলাইকুমুস সালাম (সংক্ষিপ্ত)", "ওয়া আলাইকুমুস সালাম"],
      ["وَعَلَيْكُمُ ٱلسَّلَامُ وَرَحْمَةُ ٱللَّهِ وَبَرَكَاتُهُ (আরবি)", "وَعَلَيْكُمُ ٱلسَّلَامُ وَرَحْمَةُ ٱللَّهِ وَبَرَكَاتُهُ"]
    ],
    "!walaikum": [
      ["ওয়া আলাইকুমুস সালাম ওয়া রাহমাতুল্লাহি ওয়া বারাকাতুহু", "ওয়া আলাইকুমুস সালাম ওয়া রাহমাতুল্লাহি ওয়া বারাকাতুহু"],
      ["ওয়া আলাইকুমুস সালাম", "ওয়া আলাইকুমুস সালাম"]
    ],
    "!bismillah": [
      ["বিসমিল্লাহির রাহমানির রাহিম", "বিসমিল্লাহির রাহমানির রাহিম"],
      ["بِسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ (আরবি)", "بِسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ"]
    ],
    "!inshaallah": [
      ["ইনশাআল্লাহ", "ইনশাআল্লাহ"],
      ["إِنْ شَاءَ ٱللَّٰهُ (আরবি)", "إِنْ شَاءَ ٱللَّٰهُ"],
      ["ইনশাআল্লাহ, সব ঠিক হয়ে যাবে।", "ইনশাআল্লাহ, সব ঠিক হয়ে যাবে।"]
    ],
    "!alhamdulillah": [
      ["আলহামদুলিল্লাহ", "আলহামদুলিল্লাহ"],
      ["ٱلْحَمْدُ لِلَّهِ (আরবি)", "ٱلْحَمْدُ لِلَّهِ"],
      ["আলহামদুলিল্লাহ, আমি ভালো আছি।", "আলহামদুলিল্লাহ, আমি ভালো আছি।"]
    ],
    "!subhanallah": [
      ["সুবহানআল্লাহ", "সুবহানআল্লাহ"],
      ["سُبْحَانَ ٱللَّٰهِ (আরবি)", "سُبْحَانَ ٱللَّٰهِ"]
    ],
    "!mashaallah": [
      ["মাশাআল্লাহ", "মাশাআল্লাহ"],
      ["مَا شَاءَ ٱللَّٰهُ (আরবি)", "مَا شَاءَ ٱللَّٰهُ"],
      ["মাশাআল্লাহ, অনেক সুন্দর!", "মাশাআল্লাহ, অনেক সুন্দর!"]
    ],
    "!jazakallah": [
      ["জাযাকাল্লাহু খাইরান", "জাযাকাল্লাহু খাইরান"],
      ["جَزَاكَ ٱللَّٰهُ خَيْرًا (আরবি)", "جَزَاكَ ٱللَّٰهُ خَيْرًا"]
    ],
    "!astagfirullah": [
      ["আস্তাগফিরুল্লাহ", "আস্তাগফিরুল্লাহ"],
      ["أَسْتَغْفِرُ ٱللَّٰهَ (আরবি)", "أَسْتَغْفِرُ ٱللَّٰهَ"]
    ],
    "!shokal": [
      ["শুভ সকাল! কেমন আছেন?", "শুভ সকাল! কেমন আছেন?"],
      ["শুভ সকাল! দিনটি আপনার ভালো কাটুক।", "শুভ সকাল! দিনটি আপনার ভালো কাটুক।"]
    ],
    "!gm": [["শুভ সকাল! দিনটি আপনার ভালো কাটুক।", "শুভ সকাল! দিনটি আপনার ভালো কাটুক।"]],
    "!rat": [
      ["শুভ রাত্রি! ভালো থাকবেন।", "শুভ রাত্রি! ভালো থাকবেন।"],
      ["শুভ রাত্রি! মিষ্টি স্বপ্ন।", "শুভ রাত্রি! মিষ্টি স্বপ্ন।"]
    ],
    "!gn": [["শুভ রাত্রি! ভালো থাকবেন।", "শুভ রাত্রি! ভালো থাকবেন।"]],
    "!dhonnobad": [
      ["আপনাকে অনেক অনেক ধন্যবাদ!", "আপনাকে অনেক অনেক ধন্যবাদ!"],
      ["ধন্যবাদ ভাই!", "ধন্যবাদ ভাই!"]
    ],
    "!thx": [["আপনাকে অনেক অনেক ধন্যবাদ!", "আপনাকে অনেক অনেক ধন্যবাদ!"]],
    "!ty": [["আপনাকে অনেক অনেক ধন্যবাদ!", "আপনাকে অনেক অনেক ধন্যবাদ!"]],
    "!welcome": [["আপনাকে স্বাগতম!", "আপনাকে স্বাগতম!"], ["মোস্ট ওয়েলকাম!", "মোস্ট ওয়েলকাম!"]],
    "!np": [["কোনো সমস্যা নেই, আপনার স্বাগতম!", "কোনো সমস্যা নেই, আপনার স্বাগতম!"]],
    "!kemon": [
      ["কেমন আছেন? আশা করি ভালো আছেন।", "কেমন আছেন? আশা করি ভালো আছেন।"],
      ["কেমন আছো? দিনকাল কেমন কাটছে?", "কেমন আছো? দিনকাল কেমন কাটছে?"]
    ],
    "!valo": [["আলহামদুলিল্লাহ, আমি ভালো আছি। আপনি কেমন আছেন?", "আলহামদুলিল্লাহ, আমি ভালো আছি। আপনি কেমন আছেন?"]],
    "!khobor": [["কী খবর? দিনকাল কেমন চলছে?", "কী খবর? দিনকাল কেমন চলছে?"]],
    "!busy": [["আমি এখন একটু ব্যস্ত আছি, ফ্রি হয়ে কথা বলছি।", "আমি এখন একটু ব্যস্ত আছি, ফ্রি হয়ে কথা বলছি।"]],
    "!brb": [["একটু পরেই ফিরছি, অপেক্ষা করুন।", "একটু পরেই ফিরছি, অপেক্ষা করুন।"]],
    "!omw": [["আমি রাস্তায় আছি, কিছুক্ষণের মধ্যেই পৌঁছাচ্ছি।", "আমি রাস্তায় আছি, কিছুক্ষণের মধ্যেই পৌঁছাচ্ছি।"]],
    "!wait": [["একটু অপেক্ষা করুন, আমি দেখছি।", "একটু অপেক্ষা করুন, আমি দেখছি।"]],
    "!meeting": [["আমি এখন একটি মিটিংয়ে আছি, শেষ করে জানাচ্ছি।", "আমি এখন একটি মিটিংয়ে আছি, শেষ করে জানাচ্ছি।"]],
    "!call": [["ফ্রি হলে আমাকে একটা কল দিয়েন।", "ফ্রি হলে আমাকে একটা কল দিয়েন।"]],
    "!ok": [["ঠিক আছে, কোনো সমস্যা নেই।", "ঠিক আছে, কোনো সমস্যা নেই।"]],
    "!biday": [["আজকের মতো বিদায়, ভালো থাকবেন।", "আজকের মতো বিদায়, ভালো থাকবেন।"]],
    "!allahhafez": [["আল্লাহ হাফেজ, ভালো থাকবেন।", "আল্লাহ হাফেজ, ভালো থাকবেন।"]],
    "!ameen": [["আমিন, সুম্মা আমিন।", "আমিন, সুম্মা আমিন।"]],
    "!jumma": [["জুম্মা মোবারক! আপনার দিনটি বরকতময় হোক।", "জুম্মা মোবারক! আপনার দিনটি বরকতময় হোক।"]],
    "!eid": [
      ["ঈদ মোবারক! আপনার এবং আপনার পরিবারের জন্য অনেক শুভকামনা।", "ঈদ মোবারক! আপনার এবং আপনার পরিবারের জন্য অনেক শুভকামনা।"],
      ["তাকাব্বালাল্লাহু মিন্না ওয়া মিনকুম", "তাকাব্বালাল্লাহু মিন্না ওয়া মিনকুম"]
    ],
    "!ramadan": [["রমজান মোবারক! আল্লাহ আমাদের সবার রোজা ও ইবাদত কবুল করুন।", "রমজান মোবারক! আল্লাহ আমাদের সবার রোজা ও ইবাদত কবুল করুন।"]],
    "!noboborsho": [["শুভ নববর্ষ! নতুন বছর আপনার জীবনে সুখ ও সমৃদ্ধি বয়ে আনুক।", "শুভ নববর্ষ! নতুন বছর আপনার জীবনে সুখ ও সমৃদ্ধি বয়ে আনুক।"]],
    "!dua": [["আল্লাহ আপনার মঙ্গল করুন, আমিন।", "আল্লাহ আপনার মঙ্গল করুন, আমিন।"]],
    "!getwell": [["দ্রুত সুস্থ হয়ে উঠুন, আপনার জন্য অনেক দোয়া রইল।", "দ্রুত সুস্থ হয়ে উঠুন, আপনার জন্য অনেক দোয়া রইল।"]],
    "!sorry": [["আমি আন্তরিকভাবে দুঃখিত।", "আমি আন্তরিকভাবে দুঃখিত।"]],
    "!congrats": [["আপনাকে জানাই আন্তরিক অভিনন্দন ও শুভেচ্ছা!", "আপনাকে জানাই আন্তরিক অভিনন্দন ও শুভেচ্ছা!"]],
    "!hbd": [["শুভ জন্মদিন! আপনার আগামী দিনগুলো আনন্দময় হোক।", "শুভ জন্মদিন! আপনার আগামী দিনগুলো আনন্দময় হোক।"]],
    "!thikana": [["ঢাকা, বাংলাদেশ", "ঢাকা, বাংলাদেশ"]]
  };

  // 45+ Emojis (:)
  const EMOJIS = [
    [":hasi", "😂", "হাসি"], [":smile", "😊", "হাসি"], [":laugh", "🤣", "হাসি"],
    [":love", "❤️", "ভালোবাসা"], [":heart", "❤️", "ভালোবাসা"], [":prem", "❤️", "প্রেম"],
    [":bhalobasha", "🥰", "ভালোবাসা"], [":kanna", "😭", "কান্না"], [":cry", "😭", "কান্না"],
    [":sad", "😢", "দুঃখ"], [":like", "👍", "লাইক"], [":dislike", "👎", "ডিসলাইক"],
    [":ok", "👌", "ওকে"], [":clap", "👏", "তালি"], [":100", "💯", "শতভাগ"],
    [":check", "✅", "সঠিক"], [":cross", "❌", "ভুল"], [":fire", "🔥", "আগুন"],
    [":sparkles", "✨", "ঝলমলে"], [":rocket", "🚀", "রকেট"], [":star", "⭐", "তারা"],
    [":dua", "🤲", "দোয়া"], [":pray", "🤲", "প্রার্থনা"], [":namaskar", "🙏", "নমস্কার"],
    [":bd", "🇧🇩", "বাংলাদেশ"], [":bangladesh", "🇧🇩", "বাংলাদেশ"], [":shanto", "😌", "শান্ত"],
    [":wow", "😮", "অবাক"], [":chokh", "👀", "চোখ"], [":ful", "🌸", "ফুল"],
    [":rose", "🌹", "গোলাপ"], [":surjo", "☀️", "সূর্য"], [":chad", "🌙", "চাঁদ"],
    [":chashma", "😎", "কুল"], [":cool", "😎", "কুল"], [":party", "🎉", "পার্টি"],
    [":cake", "🎂", "কেক"], [":gift", "🎁", "উপহার"], [":coffee", "☕", "কফি"],
    [":tea", "🍵", "চা"], [":cha", "🍵", "চা"], [":book", "📚", "বই"],
    [":laptop", "💻", "ল্যাপটপ"], [":gari", "🚗", "গাড়ি"], [":biman", "✈️", "বিমান"],
    [":bhalo", "👌", "ভালো"], [":raag", "😡", "রাগ"], [":angry", "😡", "রাগ"],
    [":monkharap", "😞", "মনখারাপ"], [":salute", "🫡", "স্যালুট"], [":handheart", "🫶", "হাত-হৃদয়"],
    [":mindblown", "🤯", "অবাক"], [":thinking", "🤔", "চিন্তা"], [":shrug", "🤷", "কী জানি"],
    [":pleading", "🥺", "অনুরোধ"], [":sweat", "😅", "ঘাম"], [":hug", "🤗", "আলিঙ্গন"],
    [":wink", "😉", "চোখ টেপা"], [":punch", "👊", "ঘুষি"], [":victory", "✌️", "বিজয়"],
    [":point", "👉", "ইশারা"], [":muscle", "💪", "শক্তি"], [":trophy", "🏆", "ট্রফি"],
    [":medal", "🥇", "পদক"], [":crown", "👑", "মুকুট"], [":bulb", "💡", "আইডিয়া"],
    [":pin", "📌", "পিন"], [":bell", "🔔", "ঘণ্টা"], [":phone", "📱", "ফোন"],
    [":mobile", "📱", "মোবাইল"], [":computer", "🖥️", "কম্পিউটার"], [":code", "💻", "কোডিং"],
    [":rain", "🌧️", "বৃষ্টি"], [":tree", "🌳", "গাছ"], [":money", "💵", "টাকা"],
    [":card", "💳", "কার্ড"], [":target", "🎯", "লক্ষ্য"], [":gem", "💎", "হীরা"],
    [":taka", "৳", "টাকা"]
  ];

  // All 31 Date Ordinals (1la..31she) & 20 Ranks (1m..20sh)
  const ORDINALS = {
    "1la": ["১লা", "পহেলা"], "2ra": ["২রা", "দোসরা"], "3ra": ["৩রা", "তেসরা"], "4tha": ["৪ঠা", "চৌঠা"],
    "5i": ["৫ই", "পাঁচই"], "6i": ["৬ই", "ছয়ই"], "7i": ["৭ই", "সাতই"], "8i": ["৮ই", "আটই"],
    "9i": ["৯ই", "নয়ই"], "10i": ["১০ই", "দশই"], "11i": ["১১ই", "এগারোই"], "12i": ["১২ই", "বারোই"],
    "13i": ["১৩ই", "তেরোই"], "14i": ["১৪ই", "চৌদ্দই"], "15i": ["১৫ই", "পনেরোই"], "16i": ["১৬ই", "ষোলই"],
    "17i": ["১৭ই", "সতেরোই"], "18i": ["১৮ই", "আঠারোই"],
    "19she": ["১৯শে", "উনিশে"], "20she": ["২০শে", "বিশে"], "21she": ["২১শে", "একুশে"],
    "22she": ["২২শে", "বাইশে"], "23she": ["২৩শে", "তেইশে"], "24she": ["২৪শে", "চব্বিশে"],
    "25she": ["২৫শে", "পঁচিশে"], "26she": ["২৬শে", "ছাব্বিশে"], "27she": ["২৭শে", "সাতাশে"],
    "28she": ["২৮শে", "আটাশে"], "29she": ["২৯শে", "উনত্রিশে"], "30she": ["৩০শে", "ত্রিশে"],
    "31she": ["৩১শে", "একত্রিশে"],
    "1m": ["১ম", "প্রথম"], "2yo": ["২য়", "দ্বিতীয়"], "3yo": ["৩য়", "তৃতীয়"],
    "4rtho": ["৪র্থ", "চতুর্থ"], "5m": ["৫ম", "পঞ্চম"], "6stho": ["৬ষ্ঠ", "ষষ্ঠ"],
    "7m": ["৭ম", "সপ্তম"], "8m": ["৮ম", "অষ্টম"], "9m": ["৯ম", "নবম"], "10m": ["১০ম", "দশম"],
    "11sh": ["১১শ", "একাদশ"], "12sh": ["১২শ", "দ্বাদশ"], "13sh": ["১৩শ", "ত্রয়োদশ"],
    "14sh": ["১৪শ", "চতুর্দশ"], "15sh": ["১৫শ", "পঞ্চদশ"], "16sh": ["১৬শ", "ষোড়শ"],
    "17sh": ["১৭শ", "সপ্তদশ"], "18sh": ["১৮শ", "অষ্টাদশ"], "19sh": ["১৯শ", "ঊনবিংশ"],
    "20sh": ["২০শ", "বিংশ"]
  };

  // Hybrid English tech words
  const TECH_WORDS = {
    facebook: "Facebook", youtube: "YouTube", google: "Google", gmail: "Gmail",
    email: "Email", wifi: "WiFi", ai: "AI", python: "Python", linux: "Linux",
    deepin: "Deepin", chrome: "Chrome", windows: "Windows", intel: "Intel",
    nvidia: "Nvidia", amd: "AMD", github: "GitHub", whatsapp: "WhatsApp",
    telegram: "Telegram", instagram: "Instagram", messenger: "Messenger",
    tiktok: "TikTok", linkedin: "LinkedIn", chatgpt: "ChatGPT", gemini: "Gemini",
    claude: "Claude", openai: "OpenAI", deepseek: "DeepSeek", copilot: "Copilot",
    flutter: "Flutter", react: "React", nextjs: "Next.js", vue: "Vue",
    angular: "Angular", svelte: "Svelte", docker: "Docker", kubernetes: "Kubernetes",
    aws: "AWS", azure: "Azure", tailwind: "Tailwind", typescript: "TypeScript",
    javascript: "JavaScript", nodejs: "Node.js", rust: "Rust", golang: "Go",
    sqlite: "SQLite", mysql: "MySQL", postgres: "PostgreSQL", redis: "Redis",
    figma: "Figma", upwork: "Upwork", fiverr: "Fiverr", zoom: "Zoom",
    android: "Android", iphone: "iPhone", bkash: "bKash", nagad: "Nagad",
    daraz: "Daraz", pathao: "Pathao", matraki: "MatraKi", ubuntu: "Ubuntu", debian: "Debian"
  };

  // Bangla Academy Grammar & Spelling Auto-Fix
  const GRAMMAR_FIXES = {
    "বন্টন": "বণ্টন", "ঘন্টা": "ঘণ্টা", "লন্ঠন": "লণ্ঠন", "কান্ড": "কাণ্ড",
    "পরিস্কার": "পরিষ্কার", "আবিস্কার": "আবিষ্কার", "তিস্কার": "তিরস্কার",
    "পুরষ্কার": "পুরস্কার", "নমষ্কার": "নমস্কার", "অনুষ্টান": "অনুষ্ঠান",
    "প্রতিষ্টান": "প্রতিষ্ঠান", "সান্তনা": "সান্ত্বনা", "ইতিমধ্যে": "ইতোমধ্যে",
    "উপরোক্ত": "উপর্যুক্ত", "অদ্যবধি": "অদ্যাবধি", "শ্রদ্ধাঞ্জলী": "শ্রদ্ধাঞ্জলি",
    "অঞ্জলী": "অঞ্জলি", "গীতাঞ্জলী": "গীতাঞ্জলি", "পুস্পাঞ্জলী": "পুষ্পাঞ্জলি",
    "দুরাবস্থা": "দুরবস্থা", "নিরাপরাধ": "নিরপরাধ", "সখ্যতা": "সখ্য",
    "দারিদ্রতা": "দারিদ্র্য", "উৎকর্ষতা": "উৎকর্ষ", "দৈন্যতা": "দৈন্য",
    "বাহুল্যতা": "বাহুল্য", "একত্রিত": "একত্র", "শারিরীক": "শারীরিক",
    "মানষিক": "মানসিক", "বাৎসরিক": "বার্ষিক", "সমীচিন": "সমীচীন",
    "মরিচীকা": "মরীচিকা", "পিপিলিকা": "পিপীলিকা", "বিভিষীকা": "বিভীষিকা",
    "শিরচ্ছেদ": "শিরশ্ছেদ", "মনকষ্ট": "মনঃকষ্ট", "নিশ্বাস": "নিঃশ্বাস",
    "দুস্কৃতি": "দুষ্কৃতি", "নিস্পাপ": "নিষ্পাপ", "ভৌগলিক": "ভৌগোলিক",
    "ঐক্যমত": "ঐকমত্য", "স্বাতন্ত্রতা": "স্বাতন্ত্র্য", "সৌন্দর্য্য": "সৌন্দর্য",
    "কার্য্য": "কার্য", "সূর্য্য": "সূর্য", "ধৈর্য্য": "ধৈর্য",
    "করিব": "করব", "যাইব": "যাব", "খাইব": "খাব", "বলিব": "বলব",
    "খাইতেছি": "খাচ্ছি", "যাইতেছি": "যাচ্ছি", "করিতেছি": "করছি", "বলিতেছি": "বলছি"
  };

  // Agglutinative Suffix Stemmer
  const SUFFIXES = [
    ["guloke", "গুলোকে"], ["gulote", "গুলোতে"], ["derke", "দেরকে"],
    ["gulor", "গুলোর"], ["gulo", "গুলো"], ["gula", "গুলা"],
    ["take", "টাকে"], ["tike", "টিকে"], ["tate", "টাতে"],
    ["der", "দের"], ["tar", "টার"], ["tir", "টির"],
    ["ta", "টা"], ["ti", "টি"], ["ke", "কে"]
  ];

  function lookupDictWithStemmer(raw) {
    const dict = window.MATRAKI_DICT || {};
    const lower = raw.toLowerCase();
    if (dict[lower]) return { word: dict[lower], stemmed: false };

    if (lower.length >= 6) {
      for (const [sufEn, sufBn] of SUFFIXES) {
        if (lower.length >= sufEn.length + 3 && lower.endsWith(sufEn)) {
          const stem = lower.slice(0, -sufEn.length);
          if (dict[stem] && !dict[stem].includes(" ")) {
            return { word: dict[stem] + sufBn, stemmed: true, sufBn };
          }
        }
      }
      // Also check -er / -r / -te / -e on stems
      if (lower.endsWith("er") && dict[lower.slice(0, -2)]) {
        return { word: dict[lower.slice(0, -2)] + "ের", stemmed: true };
      }
      if (lower.endsWith("te") && dict[lower.slice(0, -2)]) {
        return { word: dict[lower.slice(0, -2)] + "তে", stemmed: true };
      }
    }
    return null;
  }

  // Algorithmic Avro Phonetic Transliterator (for arbitrary words not in dictionary)
  function avroPhoneticConvert(input) {
    if (!input) return "";
    const dictHit = lookupDictWithStemmer(input);
    if (dictHit) return dictHit.word;

    const cons = [
      ["kSh", "ক্ষ"], ["kh", "খ"], ["gh", "ঘ"], ["Ng", "ঙ"], ["ng", "ং"],
      ["chh", "ছ"], ["ch", "চ"], ["jh", "ঝ"], ["NG", "ঞ"],
      ["Th", "ঠ"], ["Dh", "ঢ"], ["th", "থ"], ["dh", "ধ"],
      ["ph", "ফ"], ["bh", "ভ"], ["sh", "শ"], ["Sh", "ষ"],
      ["rh", "ঢ়"], ["kk", "ক্ক"], ["nt", "ন্ত"], ["nd", "ন্দ"],
      ["st", "স্ত"], ["sp", "স্প"], ["sk", "স্ক"], ["tr", "ত্র"],
      ["pr", "প্র"], ["kr", "ক্র"], ["gr", "গ্র"], ["br", "ব্র"],
      ["k", "ক"], ["g", "গ"], ["c", "চ"], ["j", "জ"],
      ["T", "ট"], ["D", "ড"], ["N", "ণ"], ["t", "ত"], ["d", "দ"], ["n", "ন"],
      ["p", "প"], ["f", "ফ"], ["b", "ব"], ["v", "ভ"], ["m", "ম"],
      ["z", "য"], ["r", "র"], ["l", "ল"], ["S", "ষ"], ["s", "স"], ["h", "হ"],
      ["R", "ড়"], ["y", "য়"], ["w", "ও"], ["^", "ঁ"], [":", "ঃ"]
    ];
    const vowels = [
      ["rri", "ঋ", "ৃ"], ["OU", "ঔ", "ৌ"], ["OI", "ঐ", "ৈ"],
      ["oo", "উ", "ু"], ["ee", "ঈ", "ী"], ["aa", "আ", "া"],
      ["a", "আ", "া"], ["i", "ই", "ি"], ["I", "ঈ", "ী"],
      ["u", "উ", "ু"], ["U", "ঊ", "ূ"], ["e", "এ", "ে"],
      ["O", "ও", "ো"], ["o", "অ", ""]
    ];

    let out = "";
    let i = 0;
    let prevWasCons = false;

    while (i < input.length) {
      const ch = input[i];
      if (ch >= "0" && ch <= "9") {
        out += BN_DIGITS[ch.charCodeAt(0) - 48];
        prevWasCons = false;
        i++;
        continue;
      }

      let matchedV = false;
      for (const [pat, fullV, karV] of vowels) {
        if (input.startsWith(pat, i)) {
          out += prevWasCons ? karV : fullV;
          prevWasCons = false;
          i += pat.length;
          matchedV = true;
          break;
        }
      }
      if (matchedV) continue;

      let matchedC = false;
      for (const [pat, bnC] of cons) {
        if (input.startsWith(pat, i)) {
          out += bnC;
          prevWasCons = (bnC !== "ং" && bnC !== "ঁ" && bnC !== "ঃ");
          i += pat.length;
          matchedC = true;
          break;
        }
      }
      if (matchedC) continue;

      out += ch;
      prevWasCons = false;
      i++;
    }
    return out;
  }

  // Fixed Layout Keymaps (Probhat, National/Jatiya, Munir, Borno, Avro Easy)
  const FIXED_LAYOUTS = {
    probhat: {
      name: "Probhat",
      normal: {
        "`":"‍","1":"১","2":"২","3":"৩","4":"৪","5":"৫","6":"৬","7":"৭","8":"৮","9":"৯","0":"০","-":"-","=":"=",
        "q":"দ","w":"ূ","e":"ী","r":"র","t":"ট","y":"এ","u":"ু","i":"ি","o":"ও","p":"প","[":"ে","]":"ো",
        "a":"া","s":"স","d":"ড","f":"ত","g":"গ","h":"হ","j":"জ","k":"ক","l":"ল",";":";","'":"'",
        "z":"য়","x":"শ","c":"চ","v":"আ","b":"ব","n":"ন","m":"ম",",":",",".": "।","/":"্"
      },
      shift: {
        "~":"‍","!":"!","@":"@","#":"#","$":"৳","%":"%","^":"^","&":"ঞ","*":"ৎ","(":"(",")":")","_":"_","+":"+",
        "Q":"ধ","W":"ঊ","E":"ঈ","R":"ড়","T":"ঠ","Y":"ঐ","U":"উ","I":"ই","O":"ঔ","P":"ফ","{":"ৈ","}":"ৌ",
        "A":"অ","S":"ষ","D":"ঢ","F":"থ","G":"ঘ","H":"ঃ","J":"ঝ","K":"খ","L":"ং",":":":","\"":"\"",
        "Z":"য","X":"ঢ়","C":"ছ","V":"ঋ","B":"ভ","N":"ণ","M":"ঁ","<":"ৃ",">":"ঁ","?":"?"
      }
    },
    national: {
      name: "National (Jatiya)",
      normal: {
        "1":"১","2":"২","3":"৩","4":"৪","5":"৫","6":"৬","7":"৭","8":"৮","9":"৯","0":"০",
        "q":"ঙ","w":"য","e":"ড","r":"প","t":"ট","y":"চ","u":"জ","i":"হ","o":"গ","p":"ড়",
        "a":"ৃ","s":"ু","d":"ি","f":"া","g":"্","h":"ব","j":"ক","k":"ত","l":"দ",
        "z":"্র","x":"ও","c":"ে","v":"র","b":"ন","n":"স","m":"ম",".": "।"
      },
      shift: {
        "Q":"ং","W":"য়","E":"ঢ","R":"ফ","T":"ঠ","Y":"ছ","U":"ঝ","I":"ঞ","O":"ঘ","P":"ঢ়",
        "A":"র্","S":"ূ","D":"ী","F":"অ","G":"।","H":"ভ","J":"খ","K":"থ","L":"ধ",
        "Z":"্য","X":"ঔ","C":"ৈ","V":"ল","B":"ণ","N":"শ","M":"ষ"
      }
    },
    borno: {
      name: "Borno",
      normal: {
        "1":"১","2":"২","3":"৩","4":"৪","5":"৫","6":"৬","7":"৭","8":"৮","9":"৯","0":"০",
        "q":"ঙ","w":"য","e":"ড","r":"প","t":"ট","y":"চ","u":"জ","i":"হ","o":"গ","p":"ড়",
        "a":"ৃ","s":"ু","d":"ি","f":"া","g":"্","h":"ব","j":"ক","k":"ত","l":"দ",
        "z":"্র","x":"ও","c":"ে","v":"র","b":"ন","n":"স","m":"ম",".": "।"
      },
      shift: {
        "Q":"ং","W":"য়","E":"ঢ","R":"ফ","T":"ঠ","Y":"ছ","U":"ঝ","I":"ঞ","O":"ঘ","P":"ঢ়",
        "A":"র্","S":"ূ","D":"ী","F":"অ","G":"।","H":"ভ","J":"খ","K":"থ","L":"ধ",
        "Z":"্য","X":"ঔ","C":"ৈ","V":"ল","B":"ণ","N":"শ","M":"ষ"
      }
    },
    avro_easy: {
      name: "Avro Easy",
      normal: {
        "1":"১","2":"২","3":"৩","4":"৪","5":"৫","6":"৬","7":"৭","8":"৮","9":"৯","0":"০",
        "q":"ঙ","w":"য","e":"ড","r":"প","t":"ট","y":"চ","u":"জ","i":"হ","o":"গ","p":"ড়",
        "a":"ৃ","s":"ু","d":"ি","f":"া","g":"্","h":"ব","j":"ক","k":"ত","l":"দ",
        "z":"্র","x":"ও","c":"ে","v":"র","b":"ন","n":"স","m":"ম",".": "।"
      },
      shift: {
        "Q":"ং","W":"য়","E":"ঢ","R":"ফ","T":"ঠ","Y":"ছ","U":"ঝ","I":"ঞ","O":"ঘ","P":"ঢ়",
        "A":"র্","S":"ূ","D":"ী","F":"অ","G":"।","H":"ভ","J":"খ","K":"থ","L":"ধ",
        "Z":"্য","X":"ঔ","C":"ৈ","V":"ল","B":"ণ","N":"শ","M":"ষ"
      }
    },
    munir: {
      name: "Munir Optima",
      normal: {
        "1":"১","2":"২","3":"৩","4":"৪","5":"৫","6":"৬","7":"৭","8":"৮","9":"৯","0":"০",
        "q":"স","w":"শ","e":"ে","r":"র","t":"ত","y":"য","u":"ু","i":"ি","o":"ো","p":"প",
        "a":"া","s":"ষ","d":"দ","f":"ফ","g":"গ","h":"হ","j":"জ","k":"ক","l":"ল",
        "z":"ঞ","x":"ং","c":"চ","v":"ভ","b":"ব","n":"ন","m":"ম",".": "।"
      },
      shift: {
        "Q":"ৎ","W":"ঢ়","E":"ৈ","R":"ড়","T":"ট","Y":"য়","U":"ূ","I":"ী","O":"ৌ","P":"ঠ",
        "A":"অ","S":"আ","D":"ধ","F":"ড","G":"ঘ","H":"ঃ","J":"ঝ","K":"খ","L":"ঁ",
        "Z":"ঋ","X":"ঙ","C":"ছ","V":"ঢ","B":"ণ","N":"ই","M":"উ"
      }
    }
  };

  // Subject-Verb Concordance Helper
  function getSubjectPerson(word) {
    if (["আমি", "আমরা", "আমাকে", "আমাদের"].includes(word)) return "First";
    if (["তুমি", "তোমরা", "তোমাকে", "তোমাদের"].includes(word)) return "SecondFamiliar";
    if (["তুই", "তোরা", "তোকে", "তোদের"].includes(word)) return "SecondIntimate";
    if (["আপনি", "আপনারা", "আপনাকে", "আপনাদের", "তিনি", "তাঁরা"].includes(word)) return "Honorific";
    if (["সে", "তারা", "ও", "ওরা"].includes(word)) return "Third";
    return null;
  }

  // Full Candidate Generator (mirrors MatraKiState::updateUI + SmartFeatures::processInput)
  function getSmartCandidates(raw, prevWord) {
    const results = [];
    const seen = new Set();
    const pushCand = (display, commitText, badge = "", isExclusive = false) => {
      if (!commitText || seen.has(commitText)) return;
      seen.add(commitText);
      results.push({ display, commitText, badge, isExclusive });
    };

    if (!raw) {
      // Next-Word Prediction mode when raw buffer is empty after Space
      if (prevWord && window.MATRAKI_NEXT_WORD && window.MATRAKI_NEXT_WORD[prevWord]) {
        for (const nextW of window.MATRAKI_NEXT_WORD[prevWord]) {
          pushCand(`${nextW} ➜`, nextW, "Next Word");
        }
      }
      return results;
    }

    const lower = raw.toLowerCase();

    // 1. Live Math Calculator & Unit Converter (=)
    if (raw.length > 1 && raw[0] === "=") {
      const expr = raw.slice(1);
      const unitRes = tryUnitConversion(expr);
      if (unitRes) return unitRes;

      const mathVal = evaluateMath(expr);
      if (mathVal !== null) {
        const enStr = Number.isInteger(mathVal) ? String(mathVal) : formatClean(mathVal);
        const bnStr = toBnDigits(enStr);
        pushCand(`${bnStr} (গণনা)`, bnStr, "Math", true);
        pushCand(`${enStr} (English)`, enStr, "EN", true);
        if (Number.isInteger(mathVal) && mathVal >= 0 && mathVal <= 999999999) {
          const words = numberToBnWords(mathVal);
          if (words) pushCand(`${words} (কথায়)`, words, "Words", true);
        }
        return results;
      }
    }

    // 2. Instant Snippets & Date/Time (! or @)
    if (lower[0] === "!" || lower[0] === "@") {
      if (["!tarikh", "!date", "@today", "@date"].includes(lower)) {
        const d = getBengaliDate();
        pushCand(`${d} (তারিখ)`, d, "Date", true);
        return results;
      }
      if (["!somoy", "!time", "@time", "@now"].includes(lower)) {
        const t = getBengaliTime();
        pushCand(`${t} (সময়)`, t, "Time", true);
        return results;
      }
      if (lower === "!") {
        pushCand("আসসালামু আলাইকুম ওয়া রাহমাতুল্লাহি ওয়া বারাকাতুহু (!salam)", "আসসালামু আলাইকুম ওয়া রাহমাতুল্লাহি ওয়া বারাকাতুহু", "Snippet", true);
        pushCand("ওয়া আলাইকুমুস সালাম (!olikum)", "ওয়া আলাইকুমুস সালাম", "Snippet", true);
        pushCand("বিসমিল্লাহির রাহমানির রাহিম (!bismillah)", "বিসমিল্লাহির রাহমানির রাহিম", "Snippet", true);
        pushCand("আলহামদুলিল্লাহ, আমি ভালো আছি। (!alham)", "আলহামদুলিল্লাহ, আমি ভালো আছি।", "Snippet", true);
        return results;
      }
      const SNIPPET_TITLES = {
        "!en": "Professional English Email",
        "!an": "চাকুরীর আবেদনপত্র (বাংলা)",
        "!bn": "অফিসিয়াল ইমেইল (বাংলা)",
        "!app": "চাকুরীর আবেদনপত্র",
        "!job": "Job Application Letter",
        "!applybn": "চাকুরীর আবেদনপত্র (বাংলা)",
        "!jobapply": "চাকুরীর আবেদনপত্র (বাংলা)",
        "!applyen": "Job Application & Cover Letter (English)",
        "!coverletter": "Professional Cover Letter",
        "!techapply": "Software Engineer Job Application",
        "!internapply": "Internship Application (English)",
        "!cvbn": "পূর্ণাঙ্গ বাংলা জীবনবৃত্তান্ত (CV)",
        "!cv": "পূর্ণাঙ্গ জীবনবৃত্তান্ত (CV)",
        "!cven": "Executive English CV / Resume",
        "!resume": "Executive Resume (English)",
        "!cvprofile": "CV: Professional Summary Block",
        "!cvedu": "CV: Education Table",
        "!cvexp": "CV: Work Experience Block",
        "!cvskills": "CV: Technical & Core Skills",
        "!cvref": "CV: References Block",
        "!emailbn": "অফিসিয়াল ইমেইল (বাংলা)",
        "!mailformal": "আনুষ্ঠানিক পত্র/ইমেইল (বাংলা)",
        "!emailen": "Professional Business Email",
        "!mailen": "Business Email (English)",
        "!leavemail": "ছুটির আবেদনপত্র (বাংলা)",
        "!leaveen": "Leave Application Email",
        "!followup": "Follow-up Email (English)",
        "!followupbn": "ফলো-আপ ইমেইল (বাংলা)",
        "!resignation": "Resignation Notice Email",
        "!resignbn": "পদত্যাগপত্র (বাংলা)",
        "!clientmail": "Project Proposal Email",
        "!meetingbn": "জরুরি মিটিং নোটিশ",
        "!meeting": "জরুরি মিটিং নোটিশ",
        "!agenda": "মিটিং আলোচ্যসূচি",
        "!bangladesh": "গণপ্রজাতন্ত্রী বাংলাদেশ",
        "!joybangla": "জয় বাংলা!",
        "!amarshonar": "আমার সোনার বাংলা",
        "!swadhinota": "মহান স্বাধীনতা ও জাতীয় দিবস",
        "!bijoy": "মহান বিজয় দিবস",
        "!ekushey": "আন্তর্জাতিক মাতৃভাষা দিবস",
        "!bkash": "বিকাশ (Personal)",
        "!nagad": "নগদ (Personal)",
        "!rocket": "রকেট (Personal)",
        "!cellfin": "সেলফিন / ব্যাংক",
        "!bank": "ব্যাংক বিবরণী",
        "!thikana": "ঠিকানা: ঢাকা, বাংলাদেশ",
        "!lorem": "বাংলা ডেমো টেক্সট",
        "!dev": "MatraKi Developer Info",
        "!matraki": "মাত্রা-কী কীবোর্ড",
        "!site": "MatraKi Website Link",
        "!repo": "GitHub Repository"
      };

      for (const [trig, list] of Object.entries(BUILTIN_SNIPPETS)) {
        if (trig.startsWith(lower)) {
          for (const [disp, com] of list) {
            pushCand(disp, com, trig, true);
          }
        }
      }
      const customSnips = window.MATRAKI_CUSTOM_SNIPPETS || {};
      for (const [trig, com] of Object.entries(customSnips)) {
        if (trig.startsWith(lower)) {
          let disp = com;
          if (SNIPPET_TITLES[trig]) {
            disp = `${SNIPPET_TITLES[trig]} (${trig})`;
          } else if (com.includes("\n")) {
            const first = com.split("\n")[0].trim();
            disp = `${first.slice(0, 34)}... (${trig})`;
          } else if (com.length > 40) {
            disp = `${com.slice(0, 36)}... (${trig})`;
          } else {
            disp = `${com} (${trig})`;
          }
          pushCand(disp, com, trig, true);
        }
      }
      if (results.length) return results;
    }

    // 3. Emoji Picker (:)
    if (lower[0] === ":" && lower.length >= 2) {
      for (const [trig, em, bnName] of EMOJIS) {
        if (trig.startsWith(lower)) {
          pushCand(`${em} (${bnName})`, em, trig, true);
        }
      }
      if (results.length) return results;
    }

    // 4. All 31 Date Ordinals & 20 Ranks
    if (ORDINALS[lower]) {
      const [numOrd, wordOrd] = ORDINALS[lower];
      pushCand(numOrd, numOrd, "Ordinal");
      pushCand(wordOrd, wordOrd, "কথায়");
      return results;
    }

    // 5. Currency (500tk / 500taka / tk)
    if (lower === "tk" || lower === "taka") {
      pushCand("টাকা", "টাকা", "Currency");
      pushCand("৳", "৳", "Symbol");
      return results;
    }
    const tkMatch = lower.match(/^(\d+)(tk|taka)$/);
    if (tkMatch) {
      const numPart = tkMatch[1];
      const bnNum = toBnDigits(numPart);
      pushCand(`${bnNum} টাকা`, `${bnNum} টাকা`, "Currency", true);
      pushCand(`৳${bnNum}`, `৳${bnNum}`, "Symbol", true);
      const words = numberToBnWords(parseInt(numPart, 10));
      if (words) pushCand(`${words} টাকা`, `${words} টাকা`, "কথায়", true);
      return results;
    }

    // 6. Pure Numbers -> Bengali Digits & Words
    if (/^\d{1,9}$/.test(lower)) {
      const bnNum = toBnDigits(lower);
      pushCand(bnNum, bnNum, "সংখ্যা");
      const words = numberToBnWords(parseInt(lower, 10));
      if (words) pushCand(words, words, "কথায়");
      pushCand(lower, lower, "EN");
      return results;
    }

    // 7. Hybrid English Mode
    if (TECH_WORDS[lower]) {
      const strictEn = raw.length >= 2 && raw[0] >= "A" && raw[0] <= "Z";
      if (strictEn) {
        pushCand(`${TECH_WORDS[lower]} (English)`, TECH_WORDS[lower], "Hybrid EN", true);
      }
    }

    // 7. Instant Template Keywords without '!' (e.g. typing "en", "an", "email", "apply", "cv", "job")
    const allSnips = window.MATRAKI_CUSTOM_SNIPPETS || {};
    if (lower === "en" || lower === "emailen") {
      if (allSnips["!emailen"]) pushCand("Professional Business Email (!emailen)", allSnips["!emailen"], "Template");
      if (allSnips["!applyen"]) pushCand("Job Application Letter (!applyen)", allSnips["!applyen"], "Template");
      if (allSnips["!cven"]) pushCand("Executive English CV (!cven)", allSnips["!cven"], "Template");
    } else if (lower === "an" || lower === "apply" || lower === "job") {
      if (allSnips["!applybn"]) pushCand("চাকুরীর আবেদনপত্র (বাংলা) (!applybn)", allSnips["!applybn"], "Template");
      if (allSnips["!applyen"]) pushCand("Job Application Letter (!applyen)", allSnips["!applyen"], "Template");
    } else if (lower === "bn" || lower === "emailbn") {
      if (allSnips["!emailbn"]) pushCand("অফিসিয়াল ইমেইল (বাংলা) (!emailbn)", allSnips["!emailbn"], "Template");
      if (allSnips["!applybn"]) pushCand("চাকুরীর আবেদনপত্র (বাংলা) (!applybn)", allSnips["!applybn"], "Template");
    } else if (lower === "cv" || lower === "resume") {
      if (allSnips["!cvbn"]) pushCand("পূর্ণাঙ্গ বাংলা জীবনবৃত্তান্ত (!cvbn)", allSnips["!cvbn"], "Template");
      if (allSnips["!cven"]) pushCand("Executive English CV (!cven)", allSnips["!cven"], "Template");
    } else if (lower === "email" || lower === "mail") {
      if (allSnips["!emailbn"]) pushCand("অফিসিয়াল ইমেইল (বাংলা) (!emailbn)", allSnips["!emailbn"], "Template");
      if (allSnips["!emailen"]) pushCand("Business Email (English) (!emailen)", allSnips["!emailen"], "Template");
    }

    // 8. Trained Dictionary + Smart Agglutinative Suffix Stemmer + Subject-Verb Concordance
    const dictRes = lookupDictWithStemmer(raw);
    if (dictRes) {
      // Check Bangla Academy Grammar Fix
      if (GRAMMAR_FIXES[dictRes.word]) {
        const fixed = GRAMMAR_FIXES[dictRes.word];
        pushCand(`${fixed} ✓`, fixed, "প্রমিত বানান");
      }
      const subj = prevWord ? getSubjectPerson(prevWord) : null;
      let badgeLabel = dictRes.stemmed ? `Stemmer (${dictRes.sufBn || "বিভক্তি"})` : "AI Dict";
      if (subj && !dictRes.stemmed) {
        const w = dictRes.word;
        if (
          (subj === "First" && (w.endsWith("ি") || w.endsWith("ব") || w.endsWith("লাম") || w.endsWith("ছি"))) ||
          (subj === "SecondFamiliar" && (w.endsWith("ো") || w.endsWith("বে") || w.endsWith("লে") || w.endsWith("ছ"))) ||
          (subj === "SecondIntimate" && (w.endsWith("িস") || w.endsWith("বি") || w.endsWith("লি") || w.endsWith("ছিস"))) ||
          (subj === "Honorific" && (w.endsWith("েন") || w.endsWith("বেন") || w.endsWith("লেন") || w.endsWith("ছেন"))) ||
          (subj === "Third" && (w.endsWith("ে") || w.endsWith("বে") || w.endsWith("ল") || w.endsWith("ছে")))
        ) {
          badgeLabel = `Subject-Verb AI (${prevWord})`;
        }
      }
      pushCand(dictRes.word, dictRes.word, badgeLabel);
    }

    // 9. Algorithmic Avro Phonetic candidate
    const phoneticWord = avroPhoneticConvert(raw);
    if (phoneticWord) {
      if (GRAMMAR_FIXES[phoneticWord]) {
        const fixed = GRAMMAR_FIXES[phoneticWord];
        pushCand(`${fixed} ✓`, fixed, "প্রমিত বানান");
      }
      pushCand(phoneticWord, phoneticWord, "Phonetic");
    }

    // 10. Prefix suggestions from trained dictionary (up to 4 related words)
    const dict = window.MATRAKI_DICT || {};
    if (lower.length >= 3) {
      let count = 0;
      for (const k of Object.keys(dict)) {
        if (k.length > lower.length && k.startsWith(lower)) {
          pushCand(dict[k], dict[k], k);
          if (++count >= 4) break;
        }
      }
    }

    // 11. Hybrid English fallback option
    if (TECH_WORDS[lower]) {
      pushCand(`${TECH_WORDS[lower]} (EN)`, TECH_WORDS[lower], "EN");
    } else {
      pushCand(raw, raw, "EN");
    }

    return results;
  }

  // Expose API globally for the Interactive Live Keyboard Studio
  window.MatraKiLiveEngine = {
    getSmartCandidates,
    avroPhoneticConvert,
    FIXED_LAYOUTS,
    toBnDigits,
    numberToBnWords,
    getBengaliDate,
    getBengaliTime,
    GRAMMAR_FIXES
  };
})();
