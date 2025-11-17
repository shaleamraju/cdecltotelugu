import re
from google.transliteration import transliterate_text
from .data.tel_data import letters, data_type


def replace_words_with_values(string: str, dictionary: dict[str, str]) -> str:
    pattern = re.compile(
        rf"\b({'|'.join(map(re.escape, dictionary.keys()))})\b")
    replaced_string = pattern.sub(lambda x: dictionary[x.group()], string)
    return replaced_string


def user_def_var(text: str) -> str:
    matches = re.findall(r"\bస్ట్రక్ట్‌\s+(\w+)", text)
    if matches:
        for i in matches:
            text = text.replace(
                f"స్ట్రక్ట్‌ {i}", f"స్ట్రక్ట్‌ {variable(i)}", 1)
    matches1 = re.findall(r"\bయూనియన్\s+(\w+)", text)
    if matches1:
        for i in matches1:
            text = text.replace(f"యూనియన్ {i}", f"యూనియన్ {variable(i)}", 1)
    matches2 = re.findall(r"\bఇనమ్‌\s+(\w+)", text)
    if matches2:
        for i in matches2:
            text = text.replace(f"ఇనమ్‌ {i}", f"ఇనమ్‌ {variable(i)}", 1)
    matches3 = re.findall(r"\bక్లాస్‌\s+(\w+)", text)
    if matches3:
        for i in matches3:
            text = text.replace(f"క్లాస్‌ {i}", f"క్లాస్‌ {variable(i)}", 1)
    return text


def typec_eng(text: str) -> str | None:
    pattern = r"\((.*?)\)"
    matches = re.search(pattern, text)
    return matches.group(1).strip() if matches else None


def arg_var(telugu_text: str) -> str:
    if "(" in telugu_text:
        pattern = r"ఫంక్షన్\s*(?:\([^)]*\))?\s*రిటర్నింగ్‌"
        matches = re.findall(pattern, telugu_text)
        pattern1 = r"బ్లాక్\s*(?:\([^)]*\))?\s*రిటర్నింగ్‌"
        matches1 = re.findall(pattern1, telugu_text)
        matches = matches + matches1
        for index, fun in enumerate(matches):
            type_w: str | None = typec_eng(fun)
            if type_w:
                splt: list[str] = type_w.split(",")
                for index1, args in enumerate(splt):
                    word: str = args.strip()
                    if re.match(r"^[a-zA-Z0-9]+$", word):
                        transliterated_word: str = variable(word)
                        splt[index1] = transliterated_word
                if matches[index].startswith("బ్లాక్"):
                    telugu_text = telugu_text.replace(
                        matches[index], f"బ్లాక్({','.join(splt)})రిటర్నింగ్", 1
                    )
                else:
                    telugu_text = telugu_text.replace(
                        matches[index], f"ఫంక్షన్({','.join(splt)})రిటర్నింగ్", 1
                    )
    return telugu_text


def replace_last_occurrence(text: str, old_word: str, new_word: str) -> str:
    words: list[str] = text.rsplit(old_word, 1)
    replaced_text: str = new_word.join(words)
    return replaced_text


def variable(text: str) -> str:
    result: str = ""
    if len(text) > 3:
        text = transliterate_text(text, lang_code="te")
        for char in text:
            if char in letters:
                result += letters[char]
            else:
                result += char
        return result
    emptstr: str = ""
    for i in text:
        emptstr += letters[i.upper()]
    return emptstr


def transt1(text: str) -> str:
    text = re.sub(r"\$ 3", "గా", text)
    text = re.sub(r"\$ ", "కి ", text)
    text = re.sub(r"\$  ", "కి ", text)
    text = re.sub(r"కి  3", "గా", text)
    text = re.sub(r"కి 7", "గా", text)
    text = re.sub(r"కి  శ్రేణి", " యొక్క శ్రేణి", text)
    text = re.sub(r"కి శ్రేణి", " యొక్క శ్రేణి", text)
    telugu_text: str = replace_words_with_values(text, data_type)
    return telugu_text
