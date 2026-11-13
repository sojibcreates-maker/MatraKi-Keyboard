#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <ctime>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <sstream>
#include <iomanip>

namespace fcitx {

struct SmartCandidate {
    std::string display;
    std::string commitText;
    bool isExclusive; // If true, only show smart candidates
};

// Safe and lightweight in-keyboard math evaluator (supports +, -, *, /, %, ^, parentheses, and percentages)
class MathEvaluator {
public:
    static bool evaluate(const std::string &expr, double &result) {
        size_t idx = 0;
        return parseExpression(expr, idx, result) && idx == expr.length();
    }

private:
    static void skipWhitespace(const std::string &expr, size_t &idx) {
        while (idx < expr.length() && (expr[idx] == ' ' || expr[idx] == '\t')) idx++;
    }

    static bool parseNumber(const std::string &expr, size_t &idx, double &num) {
        skipWhitespace(expr, idx);
        if (idx >= expr.length()) return false;
        size_t start = idx;
        if (expr[idx] == '+' || expr[idx] == '-') idx++;
        bool hasDigits = false;
        while (idx < expr.length() && std::isdigit(expr[idx])) { hasDigits = true; idx++; }
        if (idx < expr.length() && expr[idx] == '.') {
            idx++;
            while (idx < expr.length() && std::isdigit(expr[idx])) { hasDigits = true; idx++; }
        }
        if (!hasDigits) return false;
        try {
            num = std::stod(expr.substr(start, idx - start));
            if (idx < expr.length() && expr[idx] == '%') {
                idx++;
                num /= 100.0;
            }
            return true;
        } catch (...) { return false; }
    }

    static bool parseBase(const std::string &expr, size_t &idx, double &res) {
        skipWhitespace(expr, idx);
        if (idx >= expr.length()) return false;
        if (expr[idx] == '(') {
            idx++;
            if (!parseExpression(expr, idx, res)) return false;
            skipWhitespace(expr, idx);
            if (idx >= expr.length() || expr[idx] != ')') return false;
            idx++;
            return true;
        }
        return parseNumber(expr, idx, res);
    }

    static bool parseFactor(const std::string &expr, size_t &idx, double &res) {
        if (!parseBase(expr, idx, res)) return false;
        skipWhitespace(expr, idx);
        if (idx < expr.length() && expr[idx] == '^') {
            idx++;
            double expVal = 0;
            if (!parseFactor(expr, idx, expVal)) return false;
            res = std::pow(res, expVal);
        }
        return true;
    }

    static bool parseTerm(const std::string &expr, size_t &idx, double &res) {
        if (!parseFactor(expr, idx, res)) return false;
        while (true) {
            skipWhitespace(expr, idx);
            if (idx >= expr.length()) break;
            char op = expr[idx];
            if (op != '*' && op != '/' && op != '%') break;
            idx++;
            double nextFactor = 0;
            if (!parseFactor(expr, idx, nextFactor)) return false;
            if (op == '*') res *= nextFactor;
            else if (op == '/') {
                if (std::abs(nextFactor) < 1e-9) return false;
                res /= nextFactor;
            } else if (op == '%') {
                if (std::abs(nextFactor) < 1e-9) return false;
                res = std::fmod(res, nextFactor);
            }
        }
        return true;
    }

    static bool parseExpression(const std::string &expr, size_t &idx, double &res) {
        if (!parseTerm(expr, idx, res)) return false;
        while (true) {
            skipWhitespace(expr, idx);
            if (idx >= expr.length()) break;
            char op = expr[idx];
            if (op != '+' && op != '-') break;
            idx++;
            double nextTerm = 0;
            if (!parseTerm(expr, idx, nextTerm)) return false;
            if (op == '+') res += nextTerm;
            else if (op == '-') res -= nextTerm;
        }
        return true;
    }
};

// Hybrid English mode detector
class HybridEnglish {
public:
    static bool isEnglishWord(const std::string &raw, std::string &formatted) {
        if (raw.empty()) return false;

        static const std::unordered_map<std::string, std::string> techWords = {
            {"facebook", "Facebook"}, {"youtube", "YouTube"}, {"google", "Google"},
            {"gmail", "Gmail"}, {"email", "Email"}, {"wifi", "WiFi"}, {"ai", "AI"},
            {"python", "Python"}, {"linux", "Linux"}, {"deepin", "Deepin"},
            {"chrome", "Chrome"}, {"windows", "Windows"}, {"intel", "Intel"},
            {"nvidia", "Nvidia"}, {"amd", "AMD"}, {"github", "GitHub"},
            {"whatsapp", "WhatsApp"}, {"telegram", "Telegram"}, {"instagram", "Instagram"},
            {"messenger", "Messenger"}, {"tiktok", "TikTok"}, {"linkedin", "LinkedIn"},
            {"chatgpt", "ChatGPT"}, {"gemini", "Gemini"}, {"claude", "Claude"},
            {"openai", "OpenAI"}, {"deepseek", "DeepSeek"}, {"copilot", "Copilot"},
            {"flutter", "Flutter"}, {"react", "React"}, {"nextjs", "Next.js"},
            {"vue", "Vue"}, {"angular", "Angular"}, {"svelte", "Svelte"},
            {"docker", "Docker"}, {"kubernetes", "Kubernetes"}, {"aws", "AWS"},
            {"azure", "Azure"}, {"tailwind", "Tailwind"}, {"typescript", "TypeScript"},
            {"javascript", "JavaScript"}, {"nodejs", "Node.js"}, {"rust", "Rust"},
            {"golang", "Go"}, {"sqlite", "SQLite"}, {"mysql", "MySQL"},
            {"postgres", "PostgreSQL"}, {"redis", "Redis"}, {"figma", "Figma"},
            {"upwork", "Upwork"}, {"fiverr", "Fiverr"}, {"zoom", "Zoom"},
            {"android", "Android"}, {"iphone", "iPhone"}, {"bkash", "bKash"},
            {"nagad", "Nagad"}, {"daraz", "Daraz"}, {"pathao", "Pathao"},
            {"matraki", "MatraKi"}, {"ubuntu", "Ubuntu"}, {"debian", "Debian"}
        };

        std::string lower = raw;
        std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c){ return std::tolower(c); });

        auto it = techWords.find(lower);
        if (it != techWords.end()) {
            formatted = it->second;
            return true;
        }

        return false;
    }
};

// Typo tolerance and phonetic auto-corrections
class TypoCorrector {
public:
    static std::string getCorrection(const std::string &raw) {
        static const std::unordered_map<std::string, std::string> typos = {
            {"bhalobada", "ভালোবাসা"},
            {"valobasa", "ভালোবাসা"},
            {"valobasha", "ভালোবাসা"},
            {"bhalobasha", "ভালোবাসা"},
            {"valobashi", "ভালোবাসি"},
            {"bhalobashi", "ভালোবাসি"},
            {"dhonobad", "ধন্যবাদ"},
            {"donnobad", "ধন্যবাদ"},
            {"dhonnobaad", "ধন্যবাদ"},
            {"dhonnobad", "ধন্যবাদ"},
            {"shondor", "সুন্দর"},
            {"sundor", "সুন্দর"},
            {"somossa", "সমস্যা"},
            {"somosa", "সমস্যা"},
            {"shomossa", "সমস্যা"},
            {"shorir", "শরীর"},
            {"shobuj", "সবুজ"},
            {"shobbai", "সবাই"},
            {"sobai", "সবাই"},
            {"shobai", "সবাই"},
            {"shobaike", "সবাইকে"},
            {"sobaike", "সবাইকে"},
            {"bepar", "ব্যাপার"},
            {"oviman", "অভিমান"},
            {"shikkhok", "শিক্ষক"},
            {"protidim", "প্রতিদিন"},
            {"bides", "বিদেশ"},
            {"dakter", "ডাক্তার"},
            {"onurod", "অনুরোধ"},
            {"porikha", "পরীক্ষা"},
            {"porikkha", "পরীক্ষা"},
            {"shurute", "শুরুতে"},
            {"sathe", "সাথে"},
            {"kisu", "কিছু"},
            {"sobkisu", "সবকিছু"},
            {"shobkichu", "সবকিছু"},
            {"kmn", "কেমন"},
            {"aso", "আছো"},
            {"acho", "আছো"},
            {"achen", "আছেন"},
            {"kortesi", "করছি"},
            {"kortecho", "করছো"},
            {"jaiteci", "যাচ্ছি"},
            {"ashtesi", "আসছি"},
            {"boltesi", "বলছি"},
            {"dekhtesi", "দেখছি"},
            {"khaitesi", "খাচ্ছি"},
            {"khaisi", "খেয়েছি"},
            {"bolsi", "বলেছি"},
            {"dekhsi", "দেখেছি"},
            {"paisi", "পেয়েছি"},
            {"hoise", "হয়েছে"},
            {"gechi", "গিয়েছি"},
            {"gese", "গিয়েছে"},
            {"kire", "কিরে"},
            {"kiree", "কিরে"},
            {"thikase", "ঠিক আছে"},
            {"thikache", "ঠিক আছে"},
            {"ghumacchi", "ঘুমাচ্ছি"},
            {"ghumaiteci", "ঘুমাচ্ছি"},
            {"ghurtesi", "ঘুরছি"},
            {"shokal", "সকাল"},
            {"dupur", "দুপুর"},
            {"bikal", "বিকেল"},
            {"shondha", "সন্ধ্যা"},
            {"sondha", "সন্ধ্যা"},
            {"raat", "রাত"},
            {"ratri", "রাত্রি"},
            {"pls", "দয়া করে"},
            {"plz", "দয়া করে"},
            {"thnx", "ধন্যবাদ"},
            {"tks", "ধন্যবাদ"},
            {"ty", "ধন্যবাদ"},
            {"sry", "দুঃখিত"},
            {"srry", "দুঃখিত"}
        };

        std::string lower = raw;
        std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c){ return std::tolower(c); });

        auto it = typos.find(lower);
        if (it != typos.end()) {
            return it->second;
        }
        return "";
    }
};

// Transliteration Vocabulary Dictionary (20,500+ words dataset + Smart Agglutinative Suffix Stemmer)
class TransliterationDict {
public:
    static bool getTransliteration(const std::string &raw, std::string &outBn) {
        static std::unordered_map<std::string, std::string> dict;
        static bool loaded = false;

        if (!loaded) {
            loaded = true;
            dict.reserve(65536);
            std::string home = std::getenv("HOME") ? std::getenv("HOME") : "";
            if (!home.empty()) {
                std::string path = home + "/.local/share/matraki-keyboard/transliteration.json";
                std::ifstream in(path);
                if (in.is_open()) {
                    std::string line;
                    while (std::getline(in, line)) {
                        size_t c = line.find(':');
                        if (c != std::string::npos) {
                            size_t q1 = line.find('"', 0);
                            size_t q2 = line.find('"', q1 + 1);
                            size_t q3 = line.find('"', c + 1);
                            size_t q4 = line.find('"', q3 + 1);
                            if (q1 != std::string::npos && q2 != std::string::npos &&
                                q3 != std::string::npos && q4 != std::string::npos) {
                                std::string key = line.substr(q1 + 1, q2 - q1 - 1);
                                std::string val = line.substr(q3 + 1, q4 - q3 - 1);
                                dict[std::move(key)] = std::move(val);
                            }
                        }
                    }
                }
            }
        }

        std::string lower = raw;
        std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c){ return std::tolower(c); });

        auto it = dict.find(lower);
        if (it != dict.end()) {
            outBn = it->second;
            return true;
        }

        // Smart Agglutinative Suffix Stemmer for Bengali nouns (বিভক্তি ও নির্দেশক বিশ্লেষক)
        if (lower.size() >= 6) {
            static const std::pair<const char*, const char*> suffixes[] = {
                {"guloke", "গুলোকে"}, {"gulote", "গুলোতে"}, {"derke", "দেরকে"},
                {"gulor", "গুলোর"}, {"gulo", "গুলো"}, {"gula", "গুলা"},
                {"take", "টাকে"}, {"tike", "টিকে"}, {"tate", "টাতে"},
                {"der", "দের"}, {"tar", "টার"}, {"tir", "টির"},
                {"ta", "টা"}, {"ti", "টি"}, {"ke", "কে"}
            };
            for (const auto &suf : suffixes) {
                size_t sufLen = std::char_traits<char>::length(suf.first);
                if (lower.size() >= sufLen + 4 &&
                    lower.compare(lower.size() - sufLen, sufLen, suf.first) == 0) {
                    std::string stem = lower.substr(0, lower.size() - sufLen);
                    auto stemIt = dict.find(stem);
                    if (stemIt != dict.end() && stemIt->second.find(' ') == std::string::npos) {
                        outBn = stemIt->second + suf.second;
                        return true;
                    }
                }
            }
        }
        return false;
    }
};

class SmartFeatures {
public:
    static std::string englishDigitsToBengali(const std::string &input) {
        static const char* b_digits[] = {"০", "১", "২", "৩", "৪", "৫", "৬", "৭", "৮", "৯"};
        std::string res;
        for (char c : input) {
            if (c >= '0' && c <= '9') {
                res += b_digits[c - '0'];
            } else {
                res += c;
            }
        }
        return res;
    }

    static std::string formatDoubleClean(double val) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2) << val;
        std::string s = ss.str();
        while (!s.empty() && s.back() == '0') s.pop_back();
        if (!s.empty() && s.back() == '.') s.pop_back();
        return s;
    }

    static std::string getBengaliDate() {
        std::time_t t = std::time(nullptr);
        std::tm* now = std::localtime(&t);
        int day = now->tm_mday;
        int month = now->tm_mon; // 0-11
        int year = now->tm_year + 1900;

        static const char* b_months[] = {
            "জানুয়ারি", "ফেব্রুয়ারি", "মার্চ", "এপ্রিল", "মে", "জুন",
            "জুলাই", "আগস্ট", "সেপ্টেম্বর", "অক্টোবর", "নভেম্বর", "ডিসেম্বর"
        };

        return englishDigitsToBengali(std::to_string(day)) + "ই " + b_months[month] + ", " + englishDigitsToBengali(std::to_string(year));
    }

    static std::string getBengaliTime() {
        std::time_t t = std::time(nullptr);
        std::tm* now = std::localtime(&t);
        int hour = now->tm_hour;
        int min = now->tm_min;
        bool pm = (hour >= 12);
        if (hour > 12) hour -= 12;
        if (hour == 0) hour = 12;

        std::string s_hour = (hour < 10 ? "0" : "") + std::to_string(hour);
        std::string s_min = (min < 10 ? "0" : "") + std::to_string(min);

        return englishDigitsToBengali(s_hour) + ":" + englishDigitsToBengali(s_min) + (pm ? " অপরাহ্ন" : " পূর্বাহ্ন");
    }

    static std::string numberToBengaliWords(long long n) {
        if (n < 0 || n > 999999999) return "";
        if (n == 0) return "শূন্য";

        static const char* units[] = {
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
        };

        std::string res;
        long long crore = n / 10000000;
        n %= 10000000;
        long long lakh = n / 100000;
        n %= 100000;
        long long thousand = n / 1000;
        n %= 1000;
        long long hundred = n / 100;
        n %= 100;
        long long rem = n;

        if (crore > 0 && crore < 100) res += std::string(units[crore]) + " কোটি ";
        if (lakh > 0) res += std::string(units[lakh]) + " লাখ ";
        if (thousand > 0) res += std::string(units[thousand]) + " হাজার ";
        if (hundred > 0) res += std::string(units[hundred]) + " শত ";
        if (rem > 0) res += std::string(units[rem]) + " ";

        while (!res.empty() && res.back() == ' ') res.pop_back();
        return res;
    }

    struct SnippetOption {
        std::string display;
        std::string commitText;
    };

    static const std::vector<std::pair<std::string, std::vector<SnippetOption>>>& getAllBuiltinSnippets() {
        static const std::vector<std::pair<std::string, std::vector<SnippetOption>>> builtin = {
            {"!salam", {
                {"আসসালামু আলাইকুম ওয়া রাহমাতুল্লাহি ওয়া বারাকাতুহু (পূর্ণাঙ্গ)", "আসসালামু আলাইকুম ওয়া রাহমাতুল্লাহি ওয়া বারাকাতুহু"},
                {"আসসালামু আলাইকুম (সংক্ষিপ্ত)", "আসসালামু আলাইকুম"},
                {"السَّلَامُ عَلَيْكُمْ وَرَحْمَةُ ٱللَّهِ وَبَرَكَاتُهُ (আরবি পূর্ণাঙ্গ)", "السَّلَامُ عَلَيْكُمْ وَرَحْمَةُ ٱللَّهِ وَبَرَكَاتُهُ"},
                {"السَّلَامُ عَلَيْكُمْ (আরবি)", "السَّلَامُ عَلَيْكُمْ"}
            }},
            {"!olikum", {
                {"ওয়া আলাইকুমুস সালাম ওয়া রাহমাতুল্লাহি ওয়া বারাকাতুহু (পূর্ণাঙ্গ)", "ওয়া আলাইকুমুস সালাম ওয়া রাহমাতুল্লাহি ওয়া বারাকাতুহু"},
                {"ওয়া আলাইকুমুস সালাম (সংক্ষিপ্ত)", "ওয়া আলাইকুমুস সালাম"},
                {"وَعَلَيْكُمُ ٱلسَّلَامُ وَرَحْمَةُ ٱللَّهِ وَبَرَكَاتُهُ (আরবি পূর্ণাঙ্গ)", "وَعَلَيْكُمُ ٱلسَّلَامُ وَرَحْمَةُ ٱللَّهِ وَبَرَكَاتُهُ"},
                {"وَعَلَيْكُمُ ٱلسَّلَامُ (আরবি)", "وَعَلَيْكُمُ ٱلسَّلَامُ"}
            }},
            {"!walaikum", {
                {"ওয়া আলাইকুমুস সালাম ওয়া রাহমাতুল্লাহি ওয়া বারাকাতুহু (পূর্ণাঙ্গ)", "ওয়া আলাইকুমুস সালাম ওয়া রাহমাতুল্লাহি ওয়া বারাকাতুহু"},
                {"ওয়া আলাইকুমুস সালাম (সংক্ষিপ্ত)", "ওয়া আলাইকুমুস সালাম"},
                {"وَعَلَيْكُمُ ٱلسَّلَامُ وَرَحْمَةُ ٱللَّهِ وَبَرَكَاتُهُ (আরবি পূর্ণাঙ্গ)", "وَعَلَيْكُمُ ٱلسَّلَامُ وَرَحْمَةُ ٱللَّهِ وَبَرَكَاتُهُ"},
                {"وَعَلَيْكُمُ ٱلسَّلَامُ (আরবি)", "وَعَلَيْكُمُ ٱلسَّلَامُ"}
            }},
            {"!bismillah", {
                {"বিসমিল্লাহির রাহমানির রাহিম", "বিসমিল্লাহির রাহমানির রাহিম"},
                {"بِسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ (আরবি)", "بِسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ"}
            }},
            {"!inshaallah", {
                {"ইনশাআল্লাহ", "ইনশাআল্লাহ"},
                {"إِنْ شَاءَ ٱللَّٰهُ (আরবি)", "إِنْ شَاءَ ٱللَّٰهُ"},
                {"ইনশাআল্লাহ, সব ঠিক হয়ে যাবে।", "ইনশাআল্লাহ, সব ঠিক হয়ে যাবে।"}
            }},
            {"!alhamdulillah", {
                {"আলহামদুলিল্লাহ", "আলহামদুলিল্লাহ"},
                {"ٱلْحَمْدُ لِلَّهِ (আরবি)", "ٱلْحَمْدُ لِلَّهِ"},
                {"আলহামদুলিল্লাহ, আমি ভালো আছি।", "আলহামদুলিল্লাহ, আমি ভালো আছি।"}
            }},
            {"!subhanallah", {
                {"সুবহানআল্লাহ", "সুবহানআল্লাহ"},
                {"سُبْحَانَ ٱللَّٰهِ (আরবি)", "سُبْحَانَ ٱللَّٰهِ"}
            }},
            {"!mashaallah", {
                {"মাশাআল্লাহ", "মাশাআল্লাহ"},
                {"مَا شَاءَ ٱللَّٰهُ (আরবি)", "مَا شَاءَ ٱللَّٰهُ"},
                {"মাশাআল্লাহ, অনেক সুন্দর!", "মাশাআল্লাহ, অনেক সুন্দর!"}
            }},
            {"!jazakallah", {
                {"জাযাকাল্লাহু খাইরান", "জাযাকাল্লাহু খাইরান"},
                {"جَزَاكَ ٱللَّٰهُ خَيْرًا (আরবি)", "جَزَاكَ ٱللَّٰهُ خَيْرًا"}
            }},
            {"!astagfirullah", {
                {"আস্তাগফিরুল্লাহ", "আস্তাগফিরুল্লাহ"},
                {"أَسْتَغْفِرُ ٱللَّٰهَ (আরবি)", "أَسْتَغْفِرُ ٱللَّٰهَ"}
            }},
            {"!shokal", {
                {"শুভ সকাল! কেমন আছেন?", "শুভ সকাল! কেমন আছেন?"},
                {"শুভ সকাল! দিনটি আপনার ভালো কাটুক।", "শুভ সকাল! দিনটি আপনার ভালো কাটুক।"}
            }},
            {"!gm", {
                {"শুভ সকাল! দিনটি আপনার ভালো কাটুক।", "শুভ সকাল! দিনটি আপনার ভালো কাটুক।"}
            }},
            {"!rat", {
                {"শুভ রাত্রি! ভালো থাকবেন।", "শুভ রাত্রি! ভালো থাকবেন।"},
                {"শুভ রাত্রি! মিষ্টি স্বপ্ন।", "শুভ রাত্রি! মিষ্টি স্বপ্ন।"}
            }},
            {"!gn", {
                {"শুভ রাত্রি! ভালো থাকবেন।", "শুভ রাত্রি! ভালো থাকবেন।"}
            }},
            {"!dhonnobad", {
                {"আপনাকে অনেক অনেক ধন্যবাদ!", "আপনাকে অনেক অনেক ধন্যবাদ!"},
                {"ধন্যবাদ ভাই!", "ধন্যবাদ ভাই!"}
            }},
            {"!thx", {
                {"আপনাকে অনেক অনেক ধন্যবাদ!", "আপনাকে অনেক অনেক ধন্যবাদ!"},
                {"ধন্যবাদ ভাই!", "ধন্যবাদ ভাই!"}
            }},
            {"!ty", {
                {"আপনাকে অনেক অনেক ধন্যবাদ!", "আপনাকে অনেক অনেক ধন্যবাদ!"}
            }},
            {"!welcome", {
                {"আপনাকে স্বাগতম!", "আপনাকে স্বাগতম!"},
                {"মোস্ট ওয়েলকাম!", "মোস্ট ওয়েলকাম!"}
            }},
            {"!np", {
                {"কোনো সমস্যা নেই, আপনার স্বাগতম!", "কোনো সমস্যা নেই, আপনার স্বাগতম!"}
            }},
            {"!kemon", {
                {"কেমন আছেন? আশা করি ভালো আছেন।", "কেমন আছেন? আশা করি ভালো আছেন।"},
                {"কেমন আছো? দিনকাল কেমন কাটছে?", "কেমন আছো? দিনকাল কেমন কাটছে?"}
            }},
            {"!valo", {
                {"আলহামদুলিল্লাহ, আমি ভালো আছি। আপনি কেমন আছেন?", "আলহামদুলিল্লাহ, আমি ভালো আছি। আপনি কেমন আছেন?"},
                {"আমি ভালো আছি, আপনি কেমন আছেন?", "আমি ভালো আছি, আপনি কেমন আছেন?"}
            }},
            {"!khobor", {
                {"কী খবর? দিনকাল কেমন চলছে?", "কী খবর? দিনকাল কেমন চলছে?"},
                {"কী অবস্থা? কী করছেন?", "কী অবস্থা? কী করছেন?"}
            }},
            {"!busy", {
                {"আমি এখন একটু ব্যস্ত আছি, ফ্রি হয়ে কথা বলছি।", "আমি এখন একটু ব্যস্ত আছি, ফ্রি হয়ে কথা বলছি।"},
                {"একটু কাজে আছি, পরে নক দিচ্ছি।", "একটু কাজে আছি, পরে নক দিচ্ছি।"}
            }},
            {"!brb", {
                {"একটু পরেই ফিরছি, অপেক্ষা করুন।", "একটু পরেই ফিরছি, অপেক্ষা করুন।"}
            }},
            {"!omw", {
                {"আমি রাস্তায় আছি, কিছুক্ষণের মধ্যেই পৌঁছাচ্ছি।", "আমি রাস্তায় আছি, কিছুক্ষণের মধ্যেই পৌঁছাচ্ছি।"}
            }},
            {"!wait", {
                {"একটু অপেক্ষা করুন, আমি দেখছি।", "একটু অপেক্ষা করুন, আমি দেখছি।"}
            }},
            {"!meeting", {
                {"আমি এখন একটি মিটিংয়ে আছি, শেষ করে জানাচ্ছি।", "আমি এখন একটি মিটিংয়ে আছি, শেষ করে জানাচ্ছি।"}
            }},
            {"!call", {
                {"ফ্রি হলে আমাকে একটা কল দিয়েন।", "ফ্রি হলে আমাকে একটা কল দিয়েন।"},
                {"একটু কল দেওয়া যাবে? জরুরি কথা ছিল।", "একটু কল দেওয়া যাবে? জরুরি কথা ছিল।"}
            }},
            {"!ok", {
                {"ঠিক আছে, কোনো সমস্যা নেই।", "ঠিক আছে, কোনো সমস্যা নেই।"},
                {"ওকে, ঠিক আছে।", "ওকে, ঠিক আছে।"}
            }},
            {"!biday", {
                {"আজকের মতো বিদায়, ভালো থাকবেন।", "আজকের মতো বিদায়, ভালো থাকবেন।"},
                {"বিদায়, আবার কথা হবে ইনশাআল্লাহ।", "বিদায়, আবার কথা হবে ইনশাআল্লাহ।"}
            }},
            {"!allahhafez", {
                {"আল্লাহ হাফেজ, ভালো থাকবেন।", "আল্লাহ হাফেজ, ভালো থাকবেন।"}
            }},
            {"!ameen", {
                {"আমিন, সুম্মা আমিন।", "আমিন, সুম্মা আমিন।"}
            }},
            {"!jumma", {
                {"জুম্মা মোবারক! আপনার দিনটি বরকতময় হোক।", "জুম্মা মোবারক! আপনার দিনটি বরকতময় হোক।"}
            }},
            {"!eid", {
                {"ঈদ মোবারক! আপনার এবং আপনার পরিবারের জন্য অনেক শুভকামনা।", "ঈদ মোবারক! আপনার এবং আপনার পরিবারের জন্য অনেক শুভকামনা।"},
                {"তাকাব্বালাল্লাহু মিন্না ওয়া মিনকুম", "তাকাব্বালাল্লাহু মিন্না ওয়া মিনকুম"}
            }},
            {"!ramadan", {
                {"রমজান মোবারক! আল্লাহ আমাদের সবার রোজা ও ইবাদত কবুল করুন।", "রমজান মোবারক! আল্লাহ আমাদের সবার রোজা ও ইবাদত কবুল করুন।"}
            }},
            {"!noboborsho", {
                {"শুভ নববর্ষ! নতুন বছর আপনার জীবনে সুখ ও সমৃদ্ধি বয়ে আনুক।", "শুভ নববর্ষ! নতুন বছর আপনার জীবনে সুখ ও সমৃদ্ধি বয়ে আনুক।"}
            }},
            {"!dua", {
                {"আল্লাহ আপনার মঙ্গল করুন, আমিন।", "আল্লাহ আপনার মঙ্গল করুন, আমিন।"},
                {"অনেক দোয়া ও শুভকামনা রইল!", "অনেক দোয়া ও শুভকামনা রইল!"}
            }},
            {"!getwell", {
                {"দ্রুত সুস্থ হয়ে উঠুন, আপনার জন্য অনেক দোয়া রইল।", "দ্রুত সুস্থ হয়ে উঠুন, আপনার জন্য অনেক দোয়া রইল।"}
            }},
            {"!sorry", {
                {"আমি আন্তরিকভাবে দুঃখিত।", "আমি আন্তরিকভাবে দুঃখিত।"},
                {"ক্ষমা করবেন, অনিচ্ছাকৃত ভুলের জন্য দুঃখিত।", "ক্ষমা করবেন, অনিচ্ছাকৃত ভুলের জন্য দুঃখিত।"}
            }},
            {"!congrats", {
                {"আপনাকে জানাই আন্তরিক অভিনন্দন ও শুভেচ্ছা!", "আপনাকে জানাই আন্তরিক অভিনন্দন ও শুভেচ্ছা!"}
            }},
            {"!hbd", {
                {"শুভ জন্মদিন! আপনার আগামী দিনগুলো আনন্দময় হোক।", "শুভ জন্মদিন! আপনার আগামী দিনগুলো আনন্দময় হোক।"}
            }},
            {"!dupur", {
                {"শুভ দুপুর! কেমন আছেন?", "শুভ দুপুর! কেমন আছেন?"}
            }},
            {"!bikal", {
                {"শুভ বিকেল! কেমন কাটছে দিনটি?", "শুভ বিকেল! কেমন কাটছে দিনটি?"}
            }},
            {"!shondha", {
                {"শুভ সন্ধ্যা! আশা করি ভালো আছেন।", "শুভ সন্ধ্যা! আশা করি ভালো আছেন।"}
            }},
            {"!takecare", {
                {"নিজের যত্ন নেবেন, ভালো থাকবেন।", "নিজের যত্ন নেবেন, ভালো থাকবেন।"}
            }},
            {"!tc", {
                {"টেক কেয়ার!", "টেক কেয়ার!"}
            }},
            {"!best", {
                {"আপনার উজ্জ্বল ভবিষ্যৎ ও সফলতার জন্য অনেক শুভকামনা!", "আপনার উজ্জ্বল ভবিষ্যৎ ও সফলতার জন্য অনেক শুভকামনা!"}
            }},
            {"!goodluck", {
                {"শুভকামনা রইল!", "শুভকামনা রইল!"}
            }},
            {"!proud", {
                {"আপনার অর্জনে আমরা অত্যন্ত গর্বিত!", "আপনার অর্জনে আমরা অত্যন্ত গর্বিত!"}
            }},
            {"!en", {
                {"Dear [Recipient],\n\nI hope this email finds you well.\nI am writing to discuss [State purpose].\n\nBest regards,\n[Your Name] (!en)", "Dear [Recipient],\n\nI hope this email finds you well.\nI am writing to discuss [State purpose].\n\nBest regards,\n[Your Name]"}
            }},
            {"!an", {
                {"তারিখ: [তারিখ]\nবরাবর, ব্যবস্থাপনা পরিচালক\nবিষয়: [পদের নাম]-এর জন্য আবেদনপত্র।\n\nমহোদয়, আপনার প্রতিষ্ঠানে [পদের নাম] পদের জন্য আবেদন করছি।\n\nবিনীত,\n[আপনার নাম] (!an)", "তারিখ: [তারিখ]\nবরাবর, ব্যবস্থাপনা পরিচালক\nবিষয়: [পদের নাম]-এর জন্য আবেদনপত্র।\n\nমহোদয়, আপনার প্রতিষ্ঠানে [পদের নাম] পদের জন্য আবেদন করছি।\n\nবিনীত,\n[আপনার নাম]"}
            }},
            {"!bn", {
                {"সুপ্রিয় মহোদয়,\nআসসালামু আলাইকুম। আশা করি ভালো আছেন।\nআমার বার্তাটি হলো [এখানে বার্তা লিখুন]।\n\nধন্যবাদান্তে,\n[আপনার নাম] (!bn)", "সুপ্রিয় মহোদয়,\nআসসালামু আলাইকুম। আশা করি ভালো আছেন।\nআমার বার্তাটি হলো [এখানে বার্তা লিখুন]।\n\nধন্যবাদান্তে,\n[আপনার নাম]"}
            }},
            {"!app", {
                {"বিষয়: চাকুরীর জন্য আবেদনপত্র।\n\nমহোদয়, আপনার স্বনামধন্য প্রতিষ্ঠানে চাকুরীর আবেদন করছি। জীবনবৃত্তান্ত সংযুক্ত করা হলো।\n\nবিনীত,\n[আপনার নাম] (!app)", "বিষয়: চাকুরীর জন্য আবেদনপত্র।\n\nমহোদয়, আপনার স্বনামধন্য প্রতিষ্ঠানে চাকুরীর আবেদন করছি। জীবনবৃত্তান্ত সংযুক্ত করা হলো।\n\nবিনীত,\n[আপনার নাম]"}
            }},
            {"!job", {
                {"Subject: Application for [Job Title] - [Your Name]\n\nDear Hiring Manager,\nI am writing to apply for the [Job Title] position at [Company Name].\n\nSincerely,\n[Your Name] (!job)", "Subject: Application for [Job Title] - [Your Name]\n\nDear Hiring Manager,\nI am writing to apply for the [Job Title] position at [Company Name].\n\nSincerely,\n[Your Name]"}
            }},
            {"!emailbn", {
                {"সুপ্রিয় মহোদয়,\nআসসালামু আলাইকুম। আশা করি ভালো আছেন।\nআমার বার্তাটি হলো [এখানে আপনার মূল বার্তা লিখুন]।\nএ বিষয়ে আপনার সদয় মতামত ও পরবর্তী নির্দেশনা প্রত্যাশা করছি।\n\nধন্যবাদ ও শুভকামনাসহ,\n[আপনার নাম]", "সুপ্রিয় মহোদয়,\nআসসালামু আলাইকুম। আশা করি ভালো আছেন।\nআমার বার্তাটি হলো [এখানে আপনার মূল বার্তা লিখুন]।\nএ বিষয়ে আপনার সদয় মতামত ও পরবর্তী নির্দেশনা প্রত্যাশা করছি।\n\nধন্যবাদ ও শুভকামনাসহ,\n[আপনার নাম]"}
            }},
            {"!mailformal", {
                {"শ্রদ্ধেয় মহোদয়,\nআসসালামু আলাইকুম। আশা করি ভালো আছেন।\n\nবিনীত নিবেদন এই যে, [এখানে বিষয়ের বিস্তারিত লিখুন]।\nবিষয়টি আপনার সদয় বিবেচনার জন্য বিনীত অনুরোধ জানাচ্ছি।\n\nবিনীত,\n[আপনার নাম]", "শ্রদ্ধেয় মহোদয়,\nআসসালামু আলাইকুম। আশা করি ভালো আছেন।\n\nবিনীত নিবেদন এই যে, [এখানে বিষয়ের বিস্তারিত লিখুন]।\nবিষয়টি আপনার সদয় বিবেচনার জন্য বিনীত অনুরোধ জানাচ্ছি।\n\nবিনীত,\n[আপনার নাম]"}
            }},
            {"!emailen", {
                {"Dear [Recipient],\n\nI hope this email finds you well.\nI am writing to discuss [State purpose].\nPlease let me know if you need any further information.\n\nBest regards,\n[Your Name]", "Dear [Recipient],\n\nI hope this email finds you well.\nI am writing to discuss [State purpose].\nPlease let me know if you need any further information.\n\nBest regards,\n[Your Name]"}
            }},
            {"!mailen", {
                {"Dear [Recipient],\n\nI hope you are doing well.\nI would like to bring to your attention [mention topic].\nPlease let me know your availability for a brief discussion.\n\nKind regards,\n[Your Name]", "Dear [Recipient],\n\nI hope you are doing well.\nI would like to bring to your attention [mention topic].\nPlease let me know your availability for a brief discussion.\n\nKind regards,\n[Your Name]"}
            }},
            {"!leavemail", {
                {"বিষয়: ছুটির জন্য আবেদনপত্র।\n\nমহোদয়,\nআমার ব্যক্তিগত/পারিবারিক কারণে আগামী [শুরুর তারিখ] হতে [শেষের তারিখ] পর্যন্ত মোট [দিন] দিন কর্মস্থলে উপস্থিত থাকা সম্ভব হবে না।\nঅতএব, আমাকে উক্ত দিনসমূহের ছুটি মঞ্জুর করতে আপনার সদয় মর্জি হয়।\n\nবিনীত নিবেদক,\n[আপনার নাম]", "বিষয়: ছুটির জন্য আবেদনপত্র।\n\nমহোদয়,\nআমার ব্যক্তিগত/পারিবারিক কারণে আগামী [শুরুর তারিখ] হতে [শেষের তারিখ] পর্যন্ত মোট [দিন] দিন কর্মস্থলে উপস্থিত থাকা সম্ভব হবে না।\nঅতএব, আমাকে উক্ত দিনসমূহের ছুটি মঞ্জুর করতে আপনার সদয় মর্জি হয়।\n\nবিনীত নিবেদক,\n[আপনার নাম]"}
            }},
            {"!leaveen", {
                {"Subject: Leave Application - [Your Name]\n\nDear [Manager],\nI am writing to formally request leave from [Start Date] to [End Date] due to [reason]. All pending tasks are up to date.\n\nSincerely,\n[Your Name]", "Subject: Leave Application - [Your Name]\n\nDear [Manager],\nI am writing to formally request leave from [Start Date] to [End Date] due to [reason]. All pending tasks are up to date.\n\nSincerely,\n[Your Name]"}
            }},
            {"!followup", {
                {"Subject: Following up on [Subject]\n\nDear [Recipient],\nI am following up on my previous email sent on [Date] regarding [Topic]. Please let me know if you require any additional details.\n\nBest regards,\n[Your Name]", "Subject: Following up on [Subject]\n\nDear [Recipient],\nI am following up on my previous email sent on [Date] regarding [Topic]. Please let me know if you require any additional details.\n\nBest regards,\n[Your Name]"}
            }},
            {"!followupbn", {
                {"বিষয়: পূর্ববর্তী যোগাযোগের ফলো-আপ প্রসঙ্গে।\n\nসুপ্রিয় মহোদয়,\nগত [তারিখ]-এ প্রেরিত আমার ইমেইলের সূত্র ধরে আপনাকে বিনীতভাবে স্মরণ করিয়ে দিচ্ছি। বিষয়টি আপনার সদয় বিবেচনায় রয়েছে বলে আশা করি।\n\nধন্যবাদান্তে,\n[আপনার নাম]", "বিষয়: পূর্ববর্তী যোগাযোগের ফলো-আপ প্রসঙ্গে।\n\nসুপ্রিয় মহোদয়,\nগত [তারিখ]-এ প্রেরিত আমার ইমেইলের সূত্র ধরে আপনাকে বিনীতভাবে স্মরণ করিয়ে দিচ্ছি। বিষয়টি আপনার সদয় বিবেচনায় রয়েছে বলে আশা করি।\n\nধন্যবাদান্তে,\n[আপনার নাম]"}
            }},
            {"!resignation", {
                {"Subject: Resignation Notice - [Your Name]\n\nDear [Manager],\nPlease accept this email as notification that I am resigning from my position as [Job Title]. My last working day will be [Last Date]. Thank you for the support.\n\nSincerely,\n[Your Name]", "Subject: Resignation Notice - [Your Name]\n\nDear [Manager],\nPlease accept this email as notification that I am resigning from my position as [Job Title]. My last working day will be [Last Date]. Thank you for the support.\n\nSincerely,\n[Your Name]"}
            }},
            {"!applybn", {
                {"তারিখ: [তারিখ]\nবরাবর, ব্যবস্থাপনা পরিচালক\nবিষয়: [পদের নাম]-এর জন্য আবেদনপত্র।\n\nমহোদয়,\nআপনার স্বনামধন্য প্রতিষ্ঠানে [পদের নাম] পদে কিছুসংখ্যক দক্ষ কর্মী নিয়োগ করা হবে জেনে আমি উক্ত পদের জন্য আবেদন করছি। আমার জীবনবৃত্তান্ত সদয় বিবেচনার জন্য পেশ করলাম।\n\nবিনীত নিবেদক,\n[আপনার নাম]", "তারিখ: [তারিখ]\nবরাবর, ব্যবস্থাপনা পরিচালক\nবিষয়: [পদের নাম]-এর জন্য আবেদনপত্র।\n\nমহোদয়,\nআপনার স্বনামধন্য প্রতিষ্ঠানে [পদের নাম] পদে কিছুসংখ্যক দক্ষ কর্মী নিয়োগ করা হবে জেনে আমি উক্ত পদের জন্য আবেদন করছি। আমার জীবনবৃত্তান্ত সদয় বিবেচনার জন্য পেশ করলাম।\n\nবিনীত নিবেদক,\n[আপনার নাম]"}
            }},
            {"!jobapply", {
                {"বিষয়: [পদের নাম] পদের জন্য আবেদনপত্র।\n\nমহোদয়,\nআপনার প্রতিষ্ঠানে [পদের নাম] পদে জনবল নিয়োগের নিমিত্তে আমি একজন আগ্রহী ও যোগ্য প্রার্থী হিসেবে আবেদন করছি। জীবনবৃত্তান্তে বিস্তারিত সংযুক্ত করা হলো।\n\nবিনীত,\n[আপনার নাম]", "বিষয়: [পদের নাম] পদের জন্য আবেদনপত্র।\n\nমহোদয়,\nআপনার প্রতিষ্ঠানে [পদের নাম] পদে জনবল নিয়োগের নিমিত্তে আমি একজন আগ্রহী ও যোগ্য প্রার্থী হিসেবে আবেদন করছি। জীবনবৃত্তান্তে বিস্তারিত সংযুক্ত করা হলো।\n\nবিনীত,\n[আপনার নাম]"}
            }},
            {"!applyen", {
                {"Subject: Application for [Job Title] - [Your Name]\n\nDear Hiring Manager,\nI am writing to formally apply for the [Job Title] position currently open at [Company Name]. With my proven expertise in [Your Skills], I am confident in my ability to make a valuable contribution.\n\nSincerely,\n[Your Full Name]", "Subject: Application for [Job Title] - [Your Name]\n\nDear Hiring Manager,\nI am writing to formally apply for the [Job Title] position currently open at [Company Name]. With my proven expertise in [Your Skills], I am confident in my ability to make a valuable contribution.\n\nSincerely,\n[Your Full Name]"}
            }},
            {"!coverletter", {
                {"Dear Hiring Manager,\n\nI am writing to express my enthusiastic interest in the [Job Title] role at [Company Name]. In my previous experience, I successfully [describe achievement]. I look forward to discussing how my skills align with your team's goals.\n\nWarm regards,\n[Your Full Name]", "Dear Hiring Manager,\n\nI am writing to express my enthusiastic interest in the [Job Title] role at [Company Name]. In my previous experience, I successfully [describe achievement]. I look forward to discussing how my skills align with your team's goals.\n\nWarm regards,\n[Your Full Name]"}
            }},
            {"!techapply", {
                {"Subject: Application: [Job Title] - [Your Full Name] (Software Engineer)\n\nDear Tech Lead,\nI am excited to apply for the [Job Title] role at [Company Name]. With hands-on experience in [Your Tech Stack], clean code architecture, and API systems, I believe I can deliver immediate impact.\n\nBest regards,\n[Your Full Name]", "Subject: Application: [Job Title] - [Your Full Name] (Software Engineer)\n\nDear Tech Lead,\nI am excited to apply for the [Job Title] role at [Company Name]. With hands-on experience in [Your Tech Stack], clean code architecture, and API systems, I believe I can deliver immediate impact.\n\nBest regards,\n[Your Full Name]"}
            }},
            {"!internapply", {
                {"Subject: Internship Application - [Position] - [Your Name]\n\nDear Hiring Team,\nI am currently a student pursuing [Degree] at [University]. I am eager to apply for the [Internship Role] opportunity to learn from industry leaders and contribute proactively.\n\nSincerely,\n[Your Name]", "Subject: Internship Application - [Position] - [Your Name]\n\nDear Hiring Team,\nI am currently a student pursuing [Degree] at [University]. I am eager to apply for the [Internship Role] opportunity to learn from industry leaders and contribute proactively.\n\nSincerely,\n[Your Name]"}
            }},
            {"!cvbn", {
                {"জীবনবৃত্তান্ত (Curriculum Vitae)\n১. ব্যক্তিগত তথ্যাবলি: নাম, পিতা, মাতা, বর্তমান ও স্থায়ী ঠিকানা, মোবাইল, ইমেইল।\n২. ক্যারিয়ার অবজেক্টিভ: ...\n৩. শিক্ষাগত যোগ্যতা: স্নাতক, এইচএসসি, এসএসসি।\n৪. কর্মঅভিজ্ঞতা ও দক্ষতা।", "জীবনবৃত্তান্ত (Curriculum Vitae)\n১. ব্যক্তিগত তথ্যাবলি: নাম, পিতা, মাতা, বর্তমান ও স্থায়ী ঠিকানা, মোবাইল, ইমেইল।\n২. ক্যারিয়ার অবজেক্টিভ: ...\n৩. শিক্ষাগত যোগ্যতা: স্নাতক, এইচএসসি, এসএসসি।\n৪. কর্মঅভিজ্ঞতা ও দক্ষতা।"}
            }},
            {"!cven", {
                {"CURRICULUM VITAE\n[FULL NAME] | Phone | Email | LinkedIn | GitHub\n1. Professional Summary\n2. Technical & Core Skills\n3. Work Experience\n4. Education & Certifications\n5. Projects & References", "CURRICULUM VITAE\n[FULL NAME] | Phone | Email | LinkedIn | GitHub\n1. Professional Summary\n2. Technical & Core Skills\n3. Work Experience\n4. Education & Certifications\n5. Projects & References"}
            }},
            {"!resume", {
                {"[FULL NAME] - RESUME\nPhone | Email | LinkedIn | GitHub\nSummary | Experience | Education | Skills", "[FULL NAME] - RESUME\nPhone | Email | LinkedIn | GitHub\nSummary | Experience | Education | Skills"}
            }},
            {"!mail", {
                {"শ্রদ্ধেয় মহোদয়,\nআসসালামু আলাইকুম। আশা করি ভালো আছেন।\n\nবিনীত,", "শ্রদ্ধেয় মহোদয়,\nআসসালামু আলাইকুম। আশা করি ভালো আছেন।\n\nবিনীত,"}
            }},
            {"!email", {
                {"সুপ্রিয় সুধী,\nশুভেচ্ছা গ্রহণ করবেন।", "সুপ্রিয় সুধী,\nশুভেচ্ছা গ্রহণ করবেন।"}
            }},
            {"!regards", {
                {"ধন্যবাদ ও আন্তরিক শুভকামনাসহ,\nমো. সজীব হোসেন", "ধন্যবাদ ও আন্তরিক শুভকামনাসহ,\nমো. সজীব হোসেন"}
            }},
            {"!sir", {
                {"শ্রদ্ধেয় স্যার,\nআসসালামু আলাইকুম।", "শ্রদ্ধেয় স্যার,\nআসসালামু আলাইকুম।"}
            }},
            {"!madam", {
                {"শ্রদ্ধেয়া ম্যাডাম,\nআসসালামু আলাইকুম।", "শ্রদ্ধেয়া ম্যাডাম,\nআসসালামু আলাইকুম।"}
            }},
            {"!apply", {
                {"তারিখ: [তারিখ]\nবরাবর, ব্যবস্থাপনা পরিচালক\nবিষয়: চাকুরীর জন্য আবেদনপত্র।\n\nমহোদয়, বিনীত নিবেদন এই যে, আপনার প্রতিষ্ঠানে [পদের নাম] পদের জন্য আবেদন করছি।", "তারিখ: [তারিখ]\nবরাবর, ব্যবস্থাপনা পরিচালক\nবিষয়: চাকুরীর জন্য আবেদনপত্র।\n\nমহোদয়, বিনীত নিবেদন এই যে, আপনার প্রতিষ্ঠানে [পদের নাম] পদের জন্য আবেদন করছি।"}
            }},
            {"!chuti", {
                {"বিষয়: ছুটির জন্য আবেদনপত্র।\n\nমহোদয়, বিনীত নিবেদন এই যে, আমার ব্যক্তিগত কারণে ছুটির আবেদন করছি।", "বিষয়: ছুটির জন্য আবেদনপত্র।\n\nমহোদয়, বিনীত নিবেদন এই যে, আমার ব্যক্তিগত কারণে ছুটির আবেদন করছি।"}
            }},
            {"!cv", {
                {"জীবনবৃত্তান্ত (Curriculum Vitae)\nব্যক্তিগত বিবরণ | শিক্ষা | অভিজ্ঞতা | দক্ষতা", "জীবনবৃত্তান্ত (Curriculum Vitae)\nব্যক্তিগত বিবরণ | শিক্ষা | অভিজ্ঞতা | দক্ষতা"}
            }},
            {"!intro", {
                {"আমার নাম মো. সজীব হোসেন। আমি একজন প্রফেশনাল সফটওয়্যার ও ওয়েব ডেভেলপার।", "আমার নাম মো. সজীব হোসেন। আমি একজন প্রফেশনাল সফটওয়্যার ও ওয়েব ডেভেলপার।"}
            }},
            {"!meeting", {
                {"জরুরি মিটিং সংক্রান্ত বিজ্ঞপ্তি: অনুগ্রহ করে নির্ধারিত সময়ে উপস্থিত থাকবেন।", "জরুরি মিটিং সংক্রান্ত বিজ্ঞপ্তি: অনুগ্রহ করে নির্ধারিত সময়ে উপস্থিত থাকবেন।"}
            }},
            {"!agenda", {
                {"আজকের মিটিংয়ের আলোচ্য বিষয়সূচি:", "আজকের মিটিংয়ের আলোচ্য বিষয়সূচি:"}
            }},
            {"!bangladesh", {
                {"গণপ্রজাতন্ত্রী বাংলাদেশ", "গণপ্রজাতন্ত্রী বাংলাদেশ"}
            }},
            {"!joybangla", {
                {"জয় বাংলা!", "জয় বাংলা!"}
            }},
            {"!amarshonar", {
                {"আমার সোনার বাংলা, আমি তোমায় ভালোবাসি।", "আমার সোনার বাংলা, আমি তোমায় ভালোবাসি।"}
            }},
            {"!swadhinota", {
                {"২৬শে মার্চ — মহান স্বাধীনতা ও জাতীয় দিবস। সকল শহীদদের প্রতি বিনম্র শ্রদ্ধা।", "২৬শে মার্চ — মহান স্বাধীনতা ও জাতীয় দিবস। সকল শহীদদের প্রতি বিনম্র শ্রদ্ধা।"}
            }},
            {"!bijoy", {
                {"১৬ই ডিসেম্বর — মহান বিজয় দিবস। বিজয়ের শুভেচ্ছা!", "১৬ই ডিসেম্বর — মহান বিজয় দিবস। বিজয়ের শুভেচ্ছা!"}
            }},
            {"!ekushey", {
                {"২১শে ফেব্রুয়ারি — মহান শহীদ দিবস ও আন্তর্জাতিক মাতৃভাষা দিবস।", "২১শে ফেব্রুয়ারি — মহান শহীদ দিবস ও আন্তর্জাতিক মাতৃভাষা দিবস।"}
            }},
            {"!thikana", {
                {"ঠিকানা: ঢাকা, বাংলাদেশ।", "ঠিকানা: ঢাকা, বাংলাদেশ।"}
            }},
            {"!bkash", {
                {"বিকাশ (Personal): 01XXXXXXXXX", "বিকাশ (Personal): 01XXXXXXXXX"}
            }},
            {"!nagad", {
                {"নগদ (Personal): 01XXXXXXXXX", "নগদ (Personal): 01XXXXXXXXX"}
            }},
            {"!rocket", {
                {"রকেট (Personal): 01XXXXXXXXX", "রকেট (Personal): 01XXXXXXXXX"}
            }},
            {"!lorem", {
                {"বাংলা আমাদের অহংকার, আমাদের আবেগ ও মাতৃভাষা। বাংলা ভাষায় শুদ্ধ ও সাবলীলভাবে লেখার জন্য মাত্রা-কী কীবোর্ড একটি যুগান্তকারী সংযোজন।", "বাংলা আমাদের অহংকার, আমাদের আবেগ ও মাতৃভাষা। বাংলা ভাষায় শুদ্ধ ও সাবলীলভাবে লেখার জন্য মাত্রা-কী কীবোর্ড একটি যুগান্তকারী সংযোজন।"}
            }},
            {"!dev", {
                {"MatraKi Keyboard — Developed by Md Sojib Hossain", "MatraKi Keyboard — Developed by Md Sojib Hossain"}
            }},
            {"!matraki", {
                {"মাত্রা-কী কীবোর্ড: দ্রুতগতির, নির্ভুল ও আধুনিক বাংলা টাইপিং কিবোর্ড।", "মাত্রা-কী কীবোর্ড: দ্রুতগতির, নির্ভুল ও আধুনিক বাংলা টাইপিং কিবোর্ড।"}
            }},
            {"!site", {
                {"https://sojibcreates-maker.github.io/MatraKi-Keyboard/", "https://sojibcreates-maker.github.io/MatraKi-Keyboard/"}
            }},
            {"!repo", {
                {"https://github.com/sojibcreates-maker/MatraKi-Keyboard", "https://github.com/sojibcreates-maker/MatraKi-Keyboard"}
            }}
        };
        return builtin;
    }

    static bool matchSnippetsByPrefix(const std::string &trigger, std::vector<SnippetOption> &options) {
        const auto &all = getAllBuiltinSnippets();
        // Exact match first
        for (const auto &entry : all) {
            if (entry.first == trigger) {
                options = entry.second;
                return true;
            }
        }
        // Prefix match (e.g., "!sa" -> "!salam", "!ol" -> "!olikum", or "!" -> top snippets)
        if (trigger == "!") {
            options.push_back({"আসসালামু আলাইকুম ওয়া রাহমাতুল্লাহি ওয়া বারাকাতুহু (!salam)", "আসসালামু আলাইকুম ওয়া রাহমাতুল্লাহি ওয়া বারাকাতুহু"});
            options.push_back({"ওয়া আলাইকুমুস সালাম ওয়া রাহমাতুল্লাহি ওয়া বারাকাতুহু (!olikum)", "ওয়া আলাইকুমুস সালাম ওয়া রাহমাতুল্লাহি ওয়া বারাকাতুহু"});
            options.push_back({"বিসমিল্লাহির রাহমানির রাহিম (!bismillah)", "বিসমিল্লাহির রাহমানির রাহিম"});
            options.push_back({"আলহামদুলিল্লাহ, আমি ভালো আছি। (!alham)", "আলহামদুলিল্লাহ, আমি ভালো আছি।"});
            options.push_back({"ইনশাআল্লাহ (!insha)", "ইনশাআল্লাহ"});
            return true;
        }
        if (trigger.size() >= 2) {
            for (const auto &entry : all) {
                if (entry.first.rfind(trigger, 0) == 0) {
                    for (const auto &opt : entry.second) {
                        options.push_back(opt);
                    }
                }
            }
            if (!options.empty()) return true;
        }
        return false;
    }

    static const std::map<std::string, std::string>& getCustomSnippets() {
        static std::map<std::string, std::string> snippets;
        static bool loaded = false;
        if (!loaded) {
            loaded = true;
            std::string home = std::getenv("HOME") ? std::getenv("HOME") : "";
            if (!home.empty()) {
                std::string path = home + "/.local/share/matraki-keyboard/snippets.json";
                std::ifstream in(path);
                if (in.is_open()) {
                    std::string line;
                    while (std::getline(in, line)) {
                        size_t c = line.find(':');
                        if (c != std::string::npos) {
                            size_t q1 = line.find('"', 0);
                            size_t q2 = line.find('"', q1 + 1);
                            size_t q3 = line.find('"', c + 1);
                            size_t q4 = line.find('"', q3 + 1);
                            if (q1 != std::string::npos && q2 != std::string::npos &&
                                q3 != std::string::npos && q4 != std::string::npos) {
                                snippets[line.substr(q1 + 1, q2 - q1 - 1)] = line.substr(q3 + 1, q4 - q3 - 1);
                            }
                        }
                    }
                }
            }
        }
        return snippets;
    }

    // Smart Unit Converter (starts with '=')
    static bool tryUnitConversion(const std::string &expr, std::vector<SmartCandidate> &results) {
        std::string lower = expr;
        std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c){ return std::tolower(c); });
        // Extract leading number and trailing unit
        size_t i = 0;
        while (i < lower.size() && (std::isdigit(lower[i]) || lower[i] == '.')) i++;
        if (i == 0 || i == lower.size()) return false;
        double val = 0;
        try { val = std::stod(lower.substr(0, i)); } catch (...) { return false; }
        std::string unit = lower.substr(i);
        std::string bnVal = englishDigitsToBengali(formatDoubleClean(val));

        if (unit == "km") {
            std::string miles = englishDigitsToBengali(formatDoubleClean(val * 0.621371));
            std::string meters = englishDigitsToBengali(formatDoubleClean(val * 1000.0));
            results.push_back({bnVal + " কিমি = " + miles + " মাইল", miles + " মাইল", true});
            results.push_back({bnVal + " কিমি = " + meters + " মিটার", meters + " মিটার", true});
            results.push_back({bnVal + " কিলোমিটার", bnVal + " কিলোমিটার", true});
            return true;
        }
        if (unit == "mi" || unit == "mile" || unit == "miles") {
            std::string km = englishDigitsToBengali(formatDoubleClean(val * 1.60934));
            results.push_back({bnVal + " মাইল = " + km + " কিমি", km + " কিলোমিটার", true});
            return true;
        }
        if (unit == "m" || unit == "meter") {
            std::string ft = englishDigitsToBengali(formatDoubleClean(val * 3.28084));
            std::string cm = englishDigitsToBengali(formatDoubleClean(val * 100.0));
            results.push_back({bnVal + " মিটার = " + ft + " ফুট", ft + " ফুট", true});
            results.push_back({bnVal + " মিটার = " + cm + " সেমি", cm + " সেন্টিমিটার", true});
            return true;
        }
        if (unit == "cm") {
            std::string inch = englishDigitsToBengali(formatDoubleClean(val * 0.393701));
            results.push_back({bnVal + " সেমি = " + inch + " ইঞ্চি", inch + " ইঞ্চি", true});
            return true;
        }
        if (unit == "in" || unit == "inch") {
            std::string cm = englishDigitsToBengali(formatDoubleClean(val * 2.54));
            results.push_back({bnVal + " ইঞ্চি = " + cm + " সেমি", cm + " সেন্টিমিটার", true});
            return true;
        }
        if (unit == "kg") {
            std::string gm = englishDigitsToBengali(formatDoubleClean(val * 1000.0));
            std::string lbs = englishDigitsToBengali(formatDoubleClean(val * 2.20462));
            results.push_back({bnVal + " কেজি = " + gm + " গ্রাম", gm + " গ্রাম", true});
            results.push_back({bnVal + " কেজি = " + lbs + " পাউন্ড", lbs + " পাউন্ড", true});
            results.push_back({bnVal + " কেজি", bnVal + " কেজি", true});
            return true;
        }
        if (unit == "g" || unit == "gm" || unit == "gram") {
            std::string kg = englishDigitsToBengali(formatDoubleClean(val / 1000.0));
            results.push_back({bnVal + " গ্রাম = " + kg + " কেজি", kg + " কেজি", true});
            results.push_back({bnVal + " গ্রাম", bnVal + " গ্রাম", true});
            return true;
        }
        if (unit == "lb" || unit == "lbs") {
            std::string kg = englishDigitsToBengali(formatDoubleClean(val * 0.453592));
            results.push_back({bnVal + " পাউন্ড = " + kg + " কেজি", kg + " কেজি", true});
            return true;
        }
        if (unit == "l" || unit == "liter" || unit == "litre") {
            std::string ml = englishDigitsToBengali(formatDoubleClean(val * 1000.0));
            results.push_back({bnVal + " লিটার = " + ml + " মিলি", ml + " মিলিলিটার", true});
            results.push_back({bnVal + " লিটার", bnVal + " লিটার", true});
            return true;
        }
        if (unit == "c") {
            std::string fahr = englishDigitsToBengali(formatDoubleClean((val * 9.0 / 5.0) + 32.0));
            results.push_back({bnVal + "°C = " + fahr + "°F", fahr + "° ফারেনহাইট", true});
            results.push_back({bnVal + "° সেলসিয়াস", bnVal + "° সেলসিয়াস", true});
            return true;
        }
        if (unit == "f") {
            std::string cels = englishDigitsToBengali(formatDoubleClean((val - 32.0) * 5.0 / 9.0));
            results.push_back({bnVal + "°F = " + cels + "°C", cels + "° সেলসিয়াস", true});
            results.push_back({bnVal + "° ফারেনহাইট", bnVal + "° ফারেনহাইট", true});
            return true;
        }
        if (unit == "ft" || unit == "foot") {
            std::string cm = englishDigitsToBengali(formatDoubleClean(val * 30.48));
            std::string inch = englishDigitsToBengali(formatDoubleClean(val * 12.0));
            results.push_back({bnVal + " ফুট = " + cm + " সেমি", cm + " সেন্টিমিটার", true});
            results.push_back({bnVal + " ফুট = " + inch + " ইঞ্চি", inch + " ইঞ্চি", true});
            return true;
        }
        if (unit == "acre") {
            std::string bigha = englishDigitsToBengali(formatDoubleClean(val * 3.0303));
            std::string shotok = englishDigitsToBengali(formatDoubleClean(val * 100.0));
            results.push_back({bnVal + " একর = " + shotok + " শতক (" + bigha + " বিঘা)", shotok + " শতক", true});
            return true;
        }
        if (unit == "bigha") {
            std::string shotok = englishDigitsToBengali(formatDoubleClean(val * 33.0));
            std::string katha = englishDigitsToBengali(formatDoubleClean(val * 20.0));
            results.push_back({bnVal + " বিঘা = " + shotok + " শতক (" + katha + " কাঠা)", shotok + " শতক", true});
            return true;
        }
        if (unit == "katha") {
            std::string shotok = englishDigitsToBengali(formatDoubleClean(val * 1.65));
            std::string sqft = englishDigitsToBengali(formatDoubleClean(val * 720.0));
            results.push_back({bnVal + " কাঠা = " + shotok + " শতক (" + sqft + " বর্গফুট)", shotok + " শতক", true});
            return true;
        }
        if (unit == "shotok") {
            std::string sqft = englishDigitsToBengali(formatDoubleClean(val * 435.6));
            results.push_back({bnVal + " শতক = " + sqft + " বর্গফুট", sqft + " বর্গফুট", true});
            return true;
        }
        // Speed conversion
        if (unit == "kmh" || unit == "km/h") {
            std::string mps = englishDigitsToBengali(formatDoubleClean(val / 3.6));
            std::string mph = englishDigitsToBengali(formatDoubleClean(val * 0.621371));
            results.push_back({bnVal + " কিমি/ঘণ্টা = " + mps + " মি/সেকেন্ড (" + mph + " মাইল/ঘণ্টা)", mps + " মি/সেকেন্ড", true});
            return true;
        }
        if (unit == "mps" || unit == "m/s") {
            std::string kmh = englishDigitsToBengali(formatDoubleClean(val * 3.6));
            results.push_back({bnVal + " মি/সেকেন্ড = " + kmh + " কিমি/ঘণ্টা", kmh + " কিমি/ঘণ্টা", true});
            return true;
        }
        // Digital Storage conversion
        if (unit == "mb") {
            std::string gb = englishDigitsToBengali(formatDoubleClean(val / 1024.0));
            std::string kb = englishDigitsToBengali(formatDoubleClean(val * 1024.0));
            results.push_back({bnVal + " MB = " + gb + " GB (" + kb + " KB)", gb + " জিবি", true});
            return true;
        }
        if (unit == "gb") {
            std::string mb = englishDigitsToBengali(formatDoubleClean(val * 1024.0));
            std::string tb = englishDigitsToBengali(formatDoubleClean(val / 1024.0));
            results.push_back({bnVal + " GB = " + mb + " MB (" + tb + " TB)", mb + " এমবি", true});
            return true;
        }
        if (unit == "tb") {
            std::string gb = englishDigitsToBengali(formatDoubleClean(val * 1024.0));
            results.push_back({bnVal + " TB = " + gb + " GB", gb + " জিবি", true});
            return true;
        }
        // Foreign Currency conversion (approximate BDT rates)
        if (unit == "usd" || unit == "$") {
            std::string bdt = englishDigitsToBengali(formatDoubleClean(val * 122.0));
            results.push_back({bnVal + " USD = ৳" + bdt + " টাকা (আনুমানিক)", bdt + " টাকা", true});
            return true;
        }
        if (unit == "eur" || unit == "euro") {
            std::string bdt = englishDigitsToBengali(formatDoubleClean(val * 132.0));
            results.push_back({bnVal + " EUR = ৳" + bdt + " টাকা (আনুমানিক)", bdt + " টাকা", true});
            return true;
        }
        if (unit == "sar" || unit == "riyal") {
            std::string bdt = englishDigitsToBengali(formatDoubleClean(val * 32.5));
            results.push_back({bnVal + " SAR = ৳" + bdt + " টাকা (আনুমানিক)", bdt + " টাকা", true});
            return true;
        }
        if (unit == "aed" || unit == "dirham") {
            std::string bdt = englishDigitsToBengali(formatDoubleClean(val * 33.2));
            results.push_back({bnVal + " AED = ৳" + bdt + " টাকা (আনুমানিক)", bdt + " টাকা", true});
            return true;
        }
        return false;
    }

    static std::vector<SmartCandidate> processInput(const std::string &raw) {
        std::vector<SmartCandidate> results;
        if (raw.empty()) return results;

        std::string lower = raw;
        std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c){ return std::tolower(c); });

        // 1. In-Keyboard Math Calculator & Smart Unit Converter (starts with '=')
        if (raw.length() > 1 && raw[0] == '=') {
            std::string expr = raw.substr(1);
            if (tryUnitConversion(expr, results)) {
                return results;
            }
            double val = 0;
            if (MathEvaluator::evaluate(expr, val)) {
                std::string enStr;
                if (std::floor(val) == val && std::abs(val) < 1e15) {
                    enStr = std::to_string(static_cast<long long>(val));
                } else {
                    enStr = formatDoubleClean(val);
                }
                std::string bnStr = englishDigitsToBengali(enStr);
                results.push_back({bnStr + " (গণনা)", bnStr, true});
                results.push_back({enStr + " (English)", enStr, true});
                try {
                    long long intVal = std::stoll(enStr);
                    std::string words = numberToBengaliWords(intVal);
                    if (!words.empty()) {
                        results.push_back({words, words, true});
                    }
                } catch (...) {}
                return results;
            }
        }

        // 2. Snippets / Text expansion with Instant Prefix Matching (starts with '!' or '@')
        if (lower[0] == '!' || lower[0] == '@') {
            if (lower == "!tarikh" || lower == "!date" || lower == "@today" || lower == "@date") {
                std::string d = getBengaliDate();
                results.push_back({d + " (তারিখ)", d, true});
                return results;
            }
            if (lower == "!somoy" || lower == "!time" || lower == "@time" || lower == "@now") {
                std::string t = getBengaliTime();
                results.push_back({t + " (সময়)", t, true});
                return results;
            }

            std::vector<SnippetOption> options;
            if (matchSnippetsByPrefix(lower, options)) {
                for (const auto &opt : options) {
                    results.push_back({opt.display, opt.commitText, true});
                }
                return results;
            }

            const auto &snippets = getCustomSnippets();
            for (const auto &kv : snippets) {
                if (kv.first.rfind(lower, 0) == 0) {
                    results.push_back({kv.second + " (" + kv.first + ")", kv.second, true});
                }
            }
            if (!results.empty()) return results;
        }

        // 3. Emoji & Symbol integration with Prefix Matching (starts with ':')
        if (lower[0] == ':' && lower.size() >= 2) {
            static const std::vector<std::pair<std::string, std::pair<std::string, std::string>>> emojis = {
                {":hasi", {"😂", "হাসি"}},
                {":smile", {"😊", "হাসি"}},
                {":laugh", {"🤣", "হাসি"}},
                {":love", {"❤️", "ভালোবাসা"}},
                {":heart", {"❤️", "ভালোবাসা"}},
                {":prem", {"❤️", "প্রেম"}},
                {":bhalobasha", {"🥰", "ভালোবাসা"}},
                {":kanna", {"😭", "কান্না"}},
                {":cry", {"😭", "কান্না"}},
                {":sad", {"😢", "দুঃখ"}},
                {":like", {"👍", "লাইক"}},
                {":dislike", {"👎", "ডিসলাইক"}},
                {":ok", {"👌", "ওকে"}},
                {":clap", {"👏", "তালি"}},
                {":100", {"💯", "শতভাগ"}},
                {":check", {"✅", "সঠিক"}},
                {":cross", {"❌", "ভুল"}},
                {":fire", {"🔥", "আগুন"}},
                {":sparkles", {"✨", "ঝলমলে"}},
                {":rocket", {"🚀", "রকেট"}},
                {":star", {"⭐", "তারা"}},
                {":dua", {"🤲", "দোয়া"}},
                {":pray", {"🤲", "প্রার্থনা"}},
                {":namaskar", {"🙏", "নমস্কার"}},
                {":bd", {"🇧🇩", "বাংলাদেশ"}},
                {":bangladesh", {"🇧🇩", "বাংলাদেশ"}},
                {":shanto", {"😌", "শান্ত"}},
                {":wow", {"😮", "অবাক"}},
                {":chokh", {"👀", "চোখ"}},
                {":ful", {"🌸", "ফুল"}},
                {":rose", {"🌹", "গোলাপ"}},
                {":surjo", {"☀️", "সূর্য"}},
                {":chad", {"🌙", "চাঁদ"}},
                {":chashma", {"😎", "কুল"}},
                {":cool", {"😎", "কুল"}},
                {":party", {"🎉", "পার্টি"}},
                {":cake", {"🎂", "কেক"}},
                {":gift", {"🎁", "উপহার"}},
                {":coffee", {"☕", "কফি"}},
                {":tea", {"🍵", "চা"}},
                {":cha", {"🍵", "চা"}},
                {":book", {"📚", "বই"}},
                {":laptop", {"💻", "ল্যাপটপ"}},
                {":gari", {"🚗", "গাড়ি"}},
                {":biman", {"✈️", "বিমান"}},
                {":bhalo", {"👌", "ভালো"}},
                {":raag", {"😡", "রাগ"}},
                {":angry", {"😡", "রাগ"}},
                {":monkharap", {"😞", "মনখারাপ"}},
                {":salute", {"🫡", "স্যালুট"}},
                {":handheart", {"🫶", "হাত-হৃদয়"}},
                {":mindblown", {"🤯", "অবাক"}},
                {":thinking", {"🤔", "চিন্তা"}},
                {":shrug", {"🤷", "কী জানি"}},
                {":pleading", {"🥺", "অনুরোধ"}},
                {":sweat", {"😅", "ঘাম"}},
                {":hug", {"🤗", "আলিঙ্গন"}},
                {":wink", {"😉", "চোখ টেপা"}},
                {":punch", {"👊", "ঘুষি"}},
                {":victory", {"✌️", "বিজয়"}},
                {":point", {"👉", "ইশারা"}},
                {":muscle", {"💪", "শক্তি"}},
                {":trophy", {"🏆", "ট্রফি"}},
                {":medal", {"🥇", "পদক"}},
                {":crown", {"👑", "মুকুট"}},
                {":bulb", {"💡", "আইডিয়া"}},
                {":pin", {"📌", "পিন"}},
                {":bell", {"🔔", "ঘণ্টা"}},
                {":phone", {"📱", "ফোন"}},
                {":mobile", {"📱", "মোবাইল"}},
                {":computer", {"🖥️", "কম্পিউটার"}},
                {":code", {"💻", "কোডিং"}},
                {":rain", {"🌧️", "বৃষ্টি"}},
                {":tree", {"🌳", "গাছ"}},
                {":money", {"💵", "টাকা"}},
                {":card", {"💳", "কার্ড"}},
                {":target", {"🎯", "লক্ষ্য"}},
                {":gem", {"💎", "হীরা"}},
                {":taka", {"৳", "টাকা"}}
            };

            for (const auto &em : emojis) {
                if (em.first.rfind(lower, 0) == 0) {
                    results.push_back({em.second.first + " (" + em.second.second + ")", em.second.first, true});
                }
            }
            if (!results.empty()) return results;
        }

        // 4. Smart Bangla Ordinals & Date Suffixes (all 31 days 1la..31she + ranks 1m..20m)
        static const std::unordered_map<std::string, std::pair<std::string, std::string>> ordinals = {
            {"1la", {"১লা", "পহেলা"}}, {"2ra", {"২রা", "দোসরা"}}, {"3ra", {"৩রা", "তেসরা"}}, {"4tha", {"৪ঠা", "চৌঠা"}},
            {"5i", {"৫ই", "পাঁচই"}}, {"6i", {"৬ই", "ছয়ই"}}, {"7i", {"৭ই", "সাতই"}}, {"8i", {"৮ই", "আটই"}},
            {"9i", {"৯ই", "নয়ই"}}, {"10i", {"১০ই", "দশই"}}, {"11i", {"১১ই", "এগারোই"}}, {"12i", {"১২ই", "বারোই"}},
            {"13i", {"১৩ই", "তেরোই"}}, {"14i", {"১৪ই", "চৌদ্দই"}}, {"15i", {"১৫ই", "পনেরোই"}}, {"16i", {"১৬ই", "ষোলই"}},
            {"17i", {"১৭ই", "সতেরোই"}}, {"18i", {"১৮ই", "আঠারোই"}},
            {"19she", {"১৯শে", "উনিশে"}}, {"20she", {"২০শে", "বিশে"}}, {"21she", {"২১শে", "একুশে"}},
            {"22she", {"২২শে", "বাইশে"}}, {"23she", {"২৩শে", "তেইশে"}}, {"24she", {"২৪শে", "চব্বিশে"}},
            {"25she", {"২৫শে", "পঁচিশে"}}, {"26she", {"২৬শে", "ছাব্বিশে"}}, {"27she", {"২৭শে", "সাতাশে"}},
            {"28she", {"২৮শে", "আটাশে"}}, {"29she", {"২৯শে", "উনত্রিশে"}}, {"30she", {"৩০শে", "ত্রিশে"}},
            {"31she", {"৩১শে", "একত্রিশে"}},
            {"1m", {"১ম", "প্রথম"}}, {"2yo", {"২য়", "দ্বিতীয়"}}, {"3yo", {"৩য়", "তৃতীয়"}},
            {"4rtho", {"৪র্থ", "চতুর্থ"}}, {"5m", {"৫ম", "পঞ্চম"}}, {"6stho", {"৬ষ্ঠ", "ষষ্ঠ"}},
            {"7m", {"৭ম", "সপ্তম"}}, {"8m", {"৮ম", "অষ্টম"}}, {"9m", {"৯ম", "নবম"}}, {"10m", {"১০ম", "দশম"}},
            {"11sh", {"১১শ", "একাদশ"}}, {"12sh", {"১২শ", "দ্বাদশ"}}, {"13sh", {"১৩শ", "ত্রয়োদশ"}},
            {"14sh", {"১৪শ", "চতুর্দশ"}}, {"15sh", {"১৫শ", "পঞ্চদশ"}}, {"16sh", {"১৬শ", "ষোড়শ"}},
            {"17sh", {"১৭শ", "সপ্তদশ"}}, {"18sh", {"১৮শ", "অষ্টাদশ"}}, {"19sh", {"১৯শ", "ঊনবিংশ"}},
            {"20sh", {"২০শ", "বিংশ"}}
        };
        auto ordIt = ordinals.find(lower);
        if (ordIt != ordinals.end()) {
            results.push_back({ordIt->second.first, ordIt->second.first, false});
            results.push_back({ordIt->second.second, ordIt->second.second, false});
            return results;
        }

        // 5. Smart Hybrid English Mode
        std::string formattedEnglish;
        if (HybridEnglish::isEnglishWord(raw, formattedEnglish)) {
            bool isStrictEnglish = (raw.length() >= 2 && std::isupper(raw[0]));
            results.push_back({formattedEnglish + " (English)", formattedEnglish, isStrictEnglish});
            if (isStrictEnglish) {
                return results;
            }
        }

        // 6. Typo Auto-Fix (Fuzzy & Phonetic Correction)
        std::string typoFix = TypoCorrector::getCorrection(raw);
        if (!typoFix.empty()) {
            results.push_back({typoFix + " (সংশোধন)", typoFix, false});
        }

        // 7. Currency / Taka conversion: e.g. "500tk", "500taka", or "tk"
        if (lower == "tk" || lower == "taka") {
            results.push_back({"টাকা", "টাকা", false});
            results.push_back({"৳", "৳", false});
            return results;
        }

        if ((lower.size() > 2 && lower.substr(lower.size() - 2) == "tk") ||
            (lower.size() > 4 && lower.substr(lower.size() - 4) == "taka")) {
            size_t sufLen = (lower.substr(lower.size() - 2) == "tk") ? 2 : 4;
            std::string numPart = lower.substr(0, lower.size() - sufLen);
            bool allDigits = !numPart.empty();
            for (char c : numPart) {
                if (!std::isdigit(c)) { allDigits = false; break; }
            }
            if (allDigits) {
                std::string bnNum = englishDigitsToBengali(numPart);
                results.push_back({bnNum + " টাকা", bnNum + " টাকা", true});
                results.push_back({"৳" + bnNum, "৳" + bnNum, true});
                try {
                    long long val = std::stoll(numPart);
                    std::string inWords = numberToBengaliWords(val);
                    if (!inWords.empty()) {
                        results.push_back({inWords + " টাকা", inWords + " টাকা", true});
                    }
                } catch (...) {}
                return results;
            }
        }

        // 8. Pure Numbers conversion: e.g. "123"
        bool allDigits = true;
        for (char c : lower) {
            if (!std::isdigit(c)) { allDigits = false; break; }
        }
        if (allDigits && lower.size() >= 1 && lower.size() <= 9) {
            std::string bnNum = englishDigitsToBengali(lower);
            results.push_back({bnNum, bnNum, false});
            results.push_back({lower, lower, false});
            try {
                long long val = std::stoll(lower);
                std::string inWords = numberToBengaliWords(val);
                if (!inWords.empty()) {
                    results.push_back({inWords, inWords, false});
                }
            } catch (...) {}
            return results;
        }

        // 9. Transliteration Vocabulary (from 13,560+ words dataset)
        std::string translitWord;
        if (TransliterationDict::getTransliteration(lower, translitWord)) {
            results.push_back({translitWord, translitWord, false});
        }

        return results;
    }
};

} // namespace fcitx
