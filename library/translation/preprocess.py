import re
from . import translate


def args_restruct(text1: str) -> str:
    if "(" in text1:
        list1: list = []
        list2: list = []
        pattern = r"function\s*(?:\([^)]*\))?\s*returning | block\s*(?:\([^)]*\))?\s*returning"
        matches: list = re.findall(pattern, text1)
        for index, word in enumerate(matches):
            list1 = word.split("(")
            if len(list1) == 2:
                list2 = list1[1].split(")")
                splt: list = list2[0].split(",")
                for index1, arg in enumerate(splt):
                    text = data = func = ""
                    data_pattern = r"\bto\s(?!.*\bto\b)(.*)$"
                    func_pattern = r"function returning"
                    data_pattern1 = r"\bof\s(?!.*\bto\b)(.*)$"
                    match: match[str] | None = re.search(data_pattern, arg)
                    if match:
                        result = match.group()
                        text: str = re.sub(data_pattern, "", arg)
                        data: str = f"{' '.join(result.split()[1:])}$"
                    else:
                        match = re.search(data_pattern1, arg)
                        if match:
                            result = match.group()
                            text: str = re.sub(data_pattern1, "", arg)
                            data: str = f"{' '.join(result.split()[1:])}$"

                    match1: match[str] | None = re.search(func_pattern, arg)
                    if match1:
                        result = match1.group()
                        func: str = result
                        text: str = re.sub(
                            func_pattern, f"{func} {data}", text)
                    else:
                        text = f"{data} {text}"
                    if text in ("", " "):
                        text = splt[index1]
                    splt[index1] = text.strip()
                text1 = text1.replace(
                    matches[index], f"{list1[0]}({','.join(splt)}){list2[1]}"
                )
    return text1


def declare_restruct(string: str) -> str:
    pattern = r"function\s*(?:\([^)]*\))?\s*returning"
    match = re.findall(pattern, string)
    match1 = re.search(r"\^(.*?)\$", string)
    if match:
        updated_string: str = string.replace(match1.group()[1:], "", 1)
        string = translate.replace_last_occurrence(
            updated_string, match[-1], f"{match[-1]} {match1.group()[1:]}"
        )
    text: list[str] = string.split("^")
    return f"{text[0]} {text[1]}"


def cast_restruct(string: str) -> str:
    pattern = (
        r"function\s*(?:\([^)]*\))?\s*returning | block\s*(?:\([^)]*\))?\s*returning"
    )
    match = re.search(pattern, string)
    if match:
        extracted_string: str = match.group(0)
        updated_string: str = re.sub(pattern, "", string)
        return extracted_string + " " + updated_string
    return string


def cast_var(string: str) -> str:
    pattern = r"7\s+(\w+)\s+0"
    match = re.search(pattern, string)
    extracted_word: str = ""
    if match:
        extracted_word = match.group(1)
    return extracted_word


def class_restructd(text1: str) -> str:
    match = re.search(r"\^(.*?)\$", text1)
    data: str = f"{match.group(1)}$"
    matches = re.findall(r"\bclass\s+(\w+)", text1)
    text1 = text1.replace(f"^{data}", "^")
    match = re.findall(r"function\s*(?:\([^)]*\))?\s*returning", text1)
    if match:
        updated_string: str = translate.replace_last_occurrence(
            text1, match[-1], "##")
        text1 = updated_string.replace("##", match[-1] + " " + data)
        text1 = text1.replace(
            f"pointer to member of class {matches[-1]}",
            f"class {matches[-1]} member of pointer to",
            1,
        )
        return text1.replace("^ ", "")
    second_class: list[str] = text1.split(matches[-1])
    text1 = f"{second_class[0]}{matches[-1]} {data}{second_class[-1]}"
    text1 = text1.replace(
        f"pointer to member of class {matches[-1]}",
        f"class {matches[-1]} member of pointer to",
        1,
    )
    return text1.replace("^ ", "")


def class_restructc(text: str) -> str:
    matches = re.findall(r"\bclass\s+(\w+)", text)
    text = text.replace(
        f"pointer to member of class {matches[-1]}",
        f"class {matches[-1]} member of pointer to",
        1,
    )
    return text
