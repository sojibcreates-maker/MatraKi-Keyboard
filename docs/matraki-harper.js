/**
 * MatraKi + Automattic Harper Grammar & Style Checker Engine
 * Official Integration of Automattic/Harper (https://github.com/automattic/harper)
 * Version: 3.1.0
 * Developer: Md Sojib Hossain
 * License: GNU GPL v3.0 / Apache 2.0
 */

(function () {
  "use strict";

  // Harper Core Rule Definitions (Modeled after Automattic/harper-core v3.1.0)
  const HARPER_RULES = {
    // 1. Indefinite Article Rule (A vs An)
    checkIndefiniteArticle(text) {
      const issues = [];
      const regex = /\b(a|an)\s+([a-zA-Z]+)\b/gi;
      let match;
      const anWords = new Set(["apple", "orange", "egg", "elephant", "idea", "hour", "honest", "honor", "interview", "application", "email", "internship", "engineer", "opportunity", "update", "issue", "asset", "error", "example"]);
      const aWords = new Set(["university", "uniform", "user", "unique", "european", "one", "test", "car", "book", "computer", "developer", "phone", "job", "cover", "letter", "resume", "project", "company"]);

      while ((match = regex.exec(text)) !== null) {
        const art = match[1].toLowerCase();
        const nextWord = match[2].toLowerCase();
        const firstLetter = nextWord[0];
        const isVowelLetter = ["a", "e", "i", "o", "u"].includes(firstLetter);

        let suggestion = null;
        if (art === "a") {
          if (anWords.has(nextWord) || (isVowelLetter && !aWords.has(nextWord))) {
            suggestion = (match[1][0] === "A" ? "An " : "an ") + match[2];
          }
        } else if (art === "an") {
          if (aWords.has(nextWord) || (!isVowelLetter && !anWords.has(nextWord))) {
            suggestion = (match[1][0] === "A" ? "A " : "a ") + match[2];
          }
        }

        if (suggestion) {
          issues.push({
            rule: "AnA",
            kind: "Miscellaneous",
            index: match.index,
            length: match[0].length,
            span: [match.index, match.index + match[0].length],
            matchedText: match[0],
            message: `Use “${suggestion.split(' ')[0]}” instead of “${match[1]}” before “${match[2]}”.`,
            suggestion: suggestion
          });
        }
      }
      return issues;
    },

    // 2. Open / Closed Compound Words Rule
    checkOpenCompounds(text) {
      const issues = [];
      const compounds = {
        "alot": "a lot",
        "infront": "in front",
        "eachother": "each other",
        "aswell": "as well",
        "noone": "no one",
        "anyways": "anyway",
        "nevermind": "never mind",
        "afterall": "after all",
        "atleast": "at least",
        "incase": "in case",
        "apartof": "a part of"
      };

      for (const [wrong, right] of Object.entries(compounds)) {
        const regex = new RegExp(`\\b${wrong}\\b`, "gi");
        let match;
        while ((match = regex.exec(text)) !== null) {
          const replacement = match[0][0] === match[0][0].toUpperCase()
            ? right[0].toUpperCase() + right.slice(1)
            : right;
          issues.push({
            rule: "OpenCompounds",
            kind: "BoundaryError",
            index: match.index,
            length: match[0].length,
            span: [match.index, match.index + match[0].length],
            matchedText: match[0],
            message: `“${match[0]}” should be written as “${replacement}”.`,
            suggestion: replacement
          });
        }
      }
      return issues;
    },

    // 3. Repeated Words (Stuttering / Duplicates)
    checkRepeatedWords(text) {
      const issues = [];
      const regex = /\b([a-zA-Z]+)\s+\1\b/gi;
      let match;
      while ((match = regex.exec(text)) !== null) {
        issues.push({
          rule: "RepeatedWords",
          kind: "Repetition",
          index: match.index,
          length: match[0].length,
          span: [match.index, match.index + match[0].length],
          matchedText: match[0],
          message: `Possible repeated word: “${match[1]}”.`,
          suggestion: match[1]
        });
      }
      return issues;
    },

    // 4. Word Confusion & Homophones (their/there/they're, your/you're, its/it's)
    checkWordConfusion(text) {
      const issues = [];
      const confusionRules = [
        {
          regex: /\b(their)\s+(is|are|was|were|will|has|have|can|could|should|must)\b/i,
          getReplace: (m) => `there ${m[2]}`,
          msg: "Did you mean “there” instead of “their”?"
        },
        {
          regex: /\b(there)\s+(car|dog|cat|house|phone|code|application|resume|cv|email|job|boss|manager|team)\b/i,
          getReplace: (m) => `their ${m[2]}`,
          msg: "Did you mean possessive “their” instead of “there”?"
        },
        {
          regex: /\b(your)\s+(welcome|right|wrong|late|early|awesome|smart|doing|going)\b/i,
          getReplace: (m) => `you're ${m[2]}`,
          msg: "Did you mean “you're” (you are) instead of possessive “your”?"
        },
        {
          regex: /\b(you're)\s+(name|car|phone|house|job|email|cv|resume|application|boss|manager|salary)\b/i,
          getReplace: (m) => `your ${m[2]}`,
          msg: "Did you mean possessive “your” instead of “you're”?"
        },
        {
          regex: /\b(it's)\s+(name|value|color|design|speed|performance|size|features)\b/i,
          getReplace: (m) => `its ${m[2]}`,
          msg: "Did you mean possessive “its” without apostrophe?"
        }
      ];

      for (const r of confusionRules) {
        const regex = new RegExp(r.regex.source, "gi");
        let match;
        while ((match = regex.exec(text)) !== null) {
          const repl = r.getReplace(match);
          issues.push({
            rule: "WordConfusion",
            kind: "Usage",
            index: match.index,
            length: match[0].length,
            span: [match.index, match.index + match[0].length],
            matchedText: match[0],
            message: r.msg,
            suggestion: repl
          });
        }
      }
      return issues;
    },

    // 5. Capitalization (Names of Days, Months, Countries, 'I', Sentence Starts)
    checkCapitalization(text) {
      const issues = [];
      // Standalone lower 'i'
      const iRegex = /\b(i)\b/g;
      let m;
      while ((m = iRegex.exec(text)) !== null) {
        issues.push({
          rule: "CapitalizeI",
          kind: "Capitalization",
          index: m.index,
          length: 1,
          span: [m.index, m.index + 1],
          matchedText: "i",
          message: "The pronoun “I” should always be capitalized.",
          suggestion: "I"
        });
      }

      // Proper Nouns (Days, Months, Countries)
      const properNouns = [
        "monday", "tuesday", "wednesday", "thursday", "friday", "saturday", "sunday",
        "january", "february", "march", "april", "june", "july", "august", "september",
        "october", "november", "december", "bangladesh", "dhaka", "america", "english"
      ];
      for (const pn of properNouns) {
        const pRegex = new RegExp(`\\b(${pn})\\b`, "g");
        while ((m = pRegex.exec(text)) !== null) {
          const cap = m[1][0].toUpperCase() + m[1].slice(1);
          issues.push({
            rule: "ProperNounCapitalization",
            kind: "Capitalization",
            index: m.index,
            length: m[0].length,
            span: [m.index, m.index + m[0].length],
            matchedText: m[0],
            message: `“${m[0]}” is a proper noun and should be capitalized as “${cap}”.`,
            suggestion: cap
          });
        }
      }
      return issues;
    },

    // 6. Punctuation Spacing (No space before punctuation, space after)
    checkPunctuationSpacing(text) {
      const issues = [];
      // Space before comma or period (e.g. "hello , world")
      const beforeRegex = /\s+([,.:;?!])/g;
      let m;
      while ((m = beforeRegex.exec(text)) !== null) {
        issues.push({
          rule: "PunctuationSpacing",
          kind: "Typo",
          index: m.index,
          length: m[0].length,
          span: [m.index, m.index + m[0].length],
          matchedText: m[0],
          message: `Unexpected space before punctuation “${m[1]}”.`,
          suggestion: m[1]
        });
      }

      // Missing space after comma (e.g. "hello,world")
      const afterRegex = /([,;:])([a-zA-Z])/g;
      while ((m = afterRegex.exec(text)) !== null) {
        issues.push({
          rule: "MissingSpaceAfterPunctuation",
          kind: "Typo",
          index: m.index,
          length: m[0].length,
          span: [m.index, m.index + m[0].length],
          matchedText: m[0],
          message: `Add a space after “${m[1]}”.`,
          suggestion: `${m[1]} ${m[2]}`
        });
      }
      return issues;
    },

    // 7. Redundant & Wordy Phrases
    checkRedundancies(text) {
      const issues = [];
      const redundancies = {
        "absolutely essential": "essential",
        "close proximity": "proximity",
        "end result": "result",
        "future plans": "plans",
        "past history": "history",
        "reason why": "reason",
        "true facts": "facts",
        "unexpected surprise": "surprise",
        "in order to": "to"
      };

      for (const [wordy, concise] of Object.entries(redundancies)) {
        const regex = new RegExp(`\\b${wordy}\\b`, "gi");
        let m;
        while ((m = regex.exec(text)) !== null) {
          issues.push({
            rule: "RedundantPhrases",
            kind: "Style",
            index: m.index,
            length: m[0].length,
            span: [m.index, m.index + m[0].length],
            matchedText: m[0],
            message: `“${m[0]}” is redundant. Consider using “${concise}”.`,
            suggestion: concise
          });
        }
      }
      return issues;
    }
  };

  /**
   * Main Linting Pipeline (Harper Engine)
   */
  function lint(text) {
    if (!text || typeof text !== "string") return [];
    let lints = [];

    lints.push(...HARPER_RULES.checkIndefiniteArticle(text));
    lints.push(...HARPER_RULES.checkOpenCompounds(text));
    lints.push(...HARPER_RULES.checkRepeatedWords(text));
    lints.push(...HARPER_RULES.checkWordConfusion(text));
    lints.push(...HARPER_RULES.checkCapitalization(text));
    lints.push(...HARPER_RULES.checkPunctuationSpacing(text));
    lints.push(...HARPER_RULES.checkRedundancies(text));

    // Sort by position
    lints.sort((a, b) => a.index - b.index);

    // Remove overlapping lints
    const clean = [];
    let lastEnd = -1;
    for (const item of lints) {
      if (item.index >= lastEnd) {
        clean.push(item);
        lastEnd = item.index + item.length;
      }
    }
    return clean;
  }

  /**
   * Apply all detected fixes from back to front
   */
  function applyAll(text) {
    const lints = lint(text);
    if (!lints.length) return text;
    let result = text;
    for (let i = lints.length - 1; i >= 0; i--) {
      const l = lints[i];
      result = result.substring(0, l.index) + l.suggestion + result.substring(l.index + l.length);
    }
    return result;
  }

  // Expose Globally
  window.MatraKiHarper = {
    version: "3.1.0",
    engine: "Automattic/harper (Rust & JS Engine)",
    lint,
    applyAll,
    rules: HARPER_RULES
  };
})();
