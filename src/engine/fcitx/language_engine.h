#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <fstream>
#include <cstdlib>

namespace fcitx {

enum class Person {
    None,
    First,              // আমি, আমরা
    SecondFamiliar,     // তুমি, তোমরা
    SecondIntimate,     // তুই, তোরা
    SecondHonorific,    // আপনি, আপনারা
    ThirdNormal,        // সে, তারা, ও, ওরা, এ, এরা
    ThirdHonorific      // তিনি, তাঁরা
};

class LanguageEngine {
public:
    static Person getSubjectPerson(const std::string &word) {
        static const std::unordered_set<std::string> p1 = {
            "আমি", "আমরা", "আমাকে", "আমাদের", "মোরা", "মুই"
        };
        static const std::unordered_set<std::string> p2_fam = {
            "তুমি", "তোমরা", "তোমাকে", "তোমাদের"
        };
        static const std::unordered_set<std::string> p2_int = {
            "তুই", "তোরা", "তোকে", "তোদের"
        };
        static const std::unordered_set<std::string> p2_hon = {
            "আপনি", "আপনারা", "আপনাকে", "আপনাদের"
        };
        static const std::unordered_set<std::string> p3_hon = {
            "তিনি", "তাঁরা", "তাঁকে", "তাঁদের"
        };
        static const std::unordered_set<std::string> p3_norm = {
            "সে", "তারা", "তাহাকে", "তাদের", "ও", "ওরা", "ওকে", "ওদের", "এ", "এরা", "একে", "এদের"
        };

        if (p1.count(word)) return Person::First;
        if (p2_fam.count(word)) return Person::SecondFamiliar;
        if (p2_int.count(word)) return Person::SecondIntimate;
        if (p2_hon.count(word)) return Person::SecondHonorific;
        if (p3_hon.count(word)) return Person::ThirdHonorific;
        if (p3_norm.count(word)) return Person::ThirdNormal;

        return Person::None;
    }

    static int getVerbConcordanceScore(Person person, const std::string &candidate) {
        if (person == Person::None) return 0;

        auto endsWith = [](const std::string &str, const std::string &suffix) {
            if (str.length() < suffix.length()) return false;
            return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
        };

        if (person == Person::First) {
            if (endsWith(candidate, "ব") || endsWith(candidate, "বো") ||
                endsWith(candidate, "ই") || endsWith(candidate, "লাম") ||
                endsWith(candidate, "ছি") || endsWith(candidate, "ছিলাম") ||
                endsWith(candidate, "তাম")) {
                return 10;
            }
            if (endsWith(candidate, "বে") || endsWith(candidate, "েন") || endsWith(candidate, "বেন")) {
                return -5;
            }
        } else if (person == Person::SecondHonorific || person == Person::ThirdHonorific) {
            if (endsWith(candidate, "েন") || endsWith(candidate, "বেন") ||
                endsWith(candidate, "লেন") || endsWith(candidate, "ছেন") ||
                endsWith(candidate, "ছিলেন") || endsWith(candidate, "তেন")) {
                return 10;
            }
            if (endsWith(candidate, "ব") || endsWith(candidate, "লাম") || endsWith(candidate, "লি")) {
                return -5;
            }
        } else if (person == Person::SecondFamiliar) {
            if (endsWith(candidate, "ও") || endsWith(candidate, "বে") ||
                endsWith(candidate, "লে") || endsWith(candidate, "ছ") ||
                endsWith(candidate, "ছো") || endsWith(candidate, "ছিলে") || endsWith(candidate, "তে")) {
                return 10;
            }
            if (endsWith(candidate, "লাম") || endsWith(candidate, "েন") || endsWith(candidate, "লি")) {
                return -5;
            }
        } else if (person == Person::SecondIntimate) {
            if (endsWith(candidate, "িস") || endsWith(candidate, "বি") ||
                endsWith(candidate, "লি") || endsWith(candidate, "ছিস") ||
                endsWith(candidate, "ছিলি") || endsWith(candidate, "তিস")) {
                return 10;
            }
            if (endsWith(candidate, "লাম") || endsWith(candidate, "েন") || endsWith(candidate, "বেন")) {
                return -5;
            }
        } else if (person == Person::ThirdNormal) {
            if (endsWith(candidate, "য়") || endsWith(candidate, "ে") ||
                endsWith(candidate, "বে") || endsWith(candidate, "ল") ||
                endsWith(candidate, "লো") || endsWith(candidate, "ছে") ||
                endsWith(candidate, "ছিল")) {
                return 10;
            }
            if (endsWith(candidate, "লাম") || endsWith(candidate, "েন") || endsWith(candidate, "ব")) {
                return -5;
            }
        }

        return 0;
    }

    static std::string getGrammarRuleFix(const std::string &word) {
        static const std::unordered_map<std::string, std::string> fixes = {
            // ণ-ত্ব ও ষ-ত্ব বিধান এবং প্রমিত বাংলা বানান
            {"বন্টন", "বণ্টন"},
            {"ঘন্টা", "ঘণ্টা"},
            {"লন্ঠন", "লণ্ঠন"},
            {"কান্ড", "কাণ্ড"},
            {"ঠান্ডা", "ঠান্ডা"},
            {"পরিস্কার", "পরিষ্কার"},
            {"আবিস্কার", "আবিষ্কার"},
            {"তিস্কার", "তিরস্কার"},
            {"পুরষ্কার", "পুরস্কার"},
            {"নমষ্কার", "নমস্কার"},
            {"অনুষ্টান", "অনুষ্ঠান"},
            {"প্রতিষ্টান", "প্রতিষ্ঠান"},
            {"সান্তনা", "সান্ত্বনা"},
            {"ইতিমধ্যে", "ইতোমধ্যে"},
            {"উপরোক্ত", "উপর্যুক্ত"},
            {"অদ্যবধি", "অদ্যাবধি"},
            {"শ্রদ্ধাঞ্জলী", "শ্রদ্ধাঞ্জলি"},
            {"অঞ্জলী", "অঞ্জলি"},
            {"গীতাঞ্জলী", "গীতাঞ্জলি"},
            {"পুস্পাঞ্জলী", "পুষ্পাঞ্জলি"},
            {"দুরাবস্থা", "দুরবস্থা"},
            {"নিরাপরাধ", "নিরপরাধ"},
            {"নিরহংকারী", "নিরহংকার"},
            {"অহোরাত্রি", "অহোরাত্র"},
            {"সখ্যতা", "সখ্য"},
            {"দারিদ্রতা", "দারিদ্র্য"},
            {"উৎকর্ষতা", "উৎকর্ষ"},
            {"দৈন্যতা", "দৈন্য"},
            {"বাহুল্যতা", "বাহুল্য"},
            {"একত্রিত", "একত্র"},
            {"শারিরীক", "শারীরিক"},
            {"মানষিক", "মানসিক"},
            {"বাৎসরিক", "বার্ষিক"},
            {"পৌরহিত্য", "পৌরোহিত্য"},
            {"সমীচিন", "সমীচীন"},
            {"মরিচীকা", "মরীচিকা"},
            {"পিপিলিকা", "পিপীলিকা"},
            {"বিভিষীকা", "বিভীষিকা"},
            {"শিরচ্ছেদ", "শিরশ্ছেদ"},
            {"মনকষ্ট", "মনঃকষ্ট"},
            {"নিশ্বাস", "নিঃশ্বাস"},
            {"দুস্কৃতি", "দুষ্কৃতি"},
            {"নিস্পাপ", "নিষ্পাপ"},
            {"নিস্প্রয়োজন", "নিষ্প্রয়োজন"},
            {"ভৌগলিক", "ভৌগোলিক"},
            {"ঐক্যমত", "ঐকমত্য"},
            {"স্বাতন্ত্রতা", "স্বাতন্ত্র্য"},
            {"সৌন্দর্য্য", "সৌন্দর্য"},
            {"কার্য্য", "কার্য"},
            {"সূর্য্য", "সূর্য"},
            {"ধৈর্য্য", "ধৈর্য"},

            // চলিত রূপ অগ্রাধিকার (গুরুচণ্ডালী দোষ রোধ)
            {"করিব", "করব"},
            {"যাইব", "যাব"},
            {"খাইব", "খাব"},
            {"বলিব", "বলব"},
            {"দেখিব", "দেখব"},
            {"থাকিব", "থাকব"},
            {"আসিব", "আসব"},
            {"লিখিব", "লিখব"},
            {"খাইতেছি", "খাচ্ছি"},
            {"যাইতেছি", "যাচ্ছি"},
            {"করিতেছি", "করছি"},
            {"বলিতেছি", "বলছি"},
            {"দেখিতেছি", "দেখছি"},
            {"দেখিতেছ", "দেখছ"},
            {"আসিতেছি", "আসছি"},
            {"তাহারা", "তারা"},
            {"উহারা", "ওরা"},
            {"তাহাকে", "তাকে"},
            {"দিয়া", "দিয়ে"},
            {"হইয়া", "হয়ে"},
            {"করিয়া", "করে"},
            {"বলিয়া", "বলে"},
            {"যাইয়া", "গিয়ে"}
        };

        auto it = fixes.find(word);
        if (it != fixes.end()) {
            return it->second;
        }
        return "";
    }

    // Load next-word predictions from ~/.local/share/matraki-keyboard/phonetic-next-word.json + built-in fallback
    static std::vector<std::string> getNextWordPredictions(const std::string &prevWord) {
        static std::unordered_map<std::string, std::vector<std::string>> bigrams;
        static bool loaded = false;

        if (!loaded) {
            loaded = true;
            bigrams = {
                {"কেমন", {"আছেন", "আছো", "আছিস"}},
                {"শুভ", {"সকাল", "রাত্রি", "কামনা", "নববর্ষ", "জন্মদিন"}},
                {"ধন্যবাদ", {"আপনাকে", "তোমাকে", "ভাই", "অনেক"}},
                {"অনেক", {"ধন্যবাদ", "ভালো", "সুন্দর", "দিন"}},
                {"খুব", {"ভালো", "সুন্দর", "দ্রুত", "সহজ"}},
                {"আমি", {"ভালো", "এখন", "তোমাকে", "যাব", "করব"}},
                {"তুমি", {"কেমন", "কোথায়", "কী", "কবে"}},
                {"আপনি", {"কেমন", "আছেন", "কোথায়", "কী", "কখন"}},
                {"সে", {"এখন", "কোথায়", "কাল", "যাবে"}},
                {"তিনি", {"একজন", "এখন", "বললেন", "করবেন"}},
                {"কী", {"খবর", "অবস্থা", "করছেন", "করছো", "ব্যাপার"}},
                {"বাংলাদেশ", {"আমার", "একটি", "সরকার", "চিরজীবী"}},
                {"ইনশাআল্লাহ", {"ভালো", "হবে", "সব", "দেখা"}},
                {"আসসালামু", {"আলাইকুম"}},
                {"আলহামদুলিল্লাহ", {"ভালো", "আমি", "সব"}}
            };

            const char *homeEnv = std::getenv("HOME");
            if (homeEnv) {
                std::string path = std::string(homeEnv) + "/.local/share/matraki-keyboard/phonetic-next-word.json";
                std::ifstream in(path);
                if (in.is_open()) {
                    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
                    size_t pos = 0;
                    while (pos < content.size()) {
                        size_t k1 = content.find('"', pos);
                        if (k1 == std::string::npos) break;
                        size_t k2 = content.find('"', k1 + 1);
                        if (k2 == std::string::npos) break;
                        std::string key = content.substr(k1 + 1, k2 - k1 - 1);

                        size_t braceOpen = content.find('{', k2);
                        if (braceOpen == std::string::npos) break;
                        size_t braceClose = content.find('}', braceOpen);
                        if (braceClose == std::string::npos) break;

                        std::string inner = content.substr(braceOpen + 1, braceClose - braceOpen - 1);
                        std::vector<std::pair<int, std::string>> scored;
                        size_t ipos = 0;
                        while (ipos < inner.size()) {
                            size_t nk1 = inner.find('"', ipos);
                            if (nk1 == std::string::npos) break;
                            size_t nk2 = inner.find('"', nk1 + 1);
                            if (nk2 == std::string::npos) break;
                            std::string nword = inner.substr(nk1 + 1, nk2 - nk1 - 1);
                            size_t colon = inner.find(':', nk2);
                            if (colon == std::string::npos) break;
                            size_t comma = inner.find(',', colon);
                            std::string numStr = (comma == std::string::npos)
                                ? inner.substr(colon + 1)
                                : inner.substr(colon + 1, comma - colon - 1);
                            int weight = 1;
                            try { weight = std::stoi(numStr); } catch (...) {}
                            scored.push_back({weight, nword});
                            if (comma == std::string::npos) break;
                            ipos = comma + 1;
                        }
                        std::sort(scored.begin(), scored.end(), [](const auto &a, const auto &b) {
                            return a.first > b.first;
                        });
                        std::vector<std::string> words;
                        for (size_t i = 0; i < scored.size() && i < 6; ++i) {
                            words.push_back(scored[i].second);
                        }
                        if (!words.empty()) {
                            bigrams[key] = std::move(words);
                        }
                        pos = braceClose + 1;
                    }
                }
            }
        }

        auto it = bigrams.find(prevWord);
        if (it != bigrams.end()) {
            return it->second;
        }
        return {};
    }
};

} // namespace fcitx
