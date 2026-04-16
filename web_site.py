import ast
import subprocess
import os
import re
from functools import lru_cache
from flask import Flask, request, render_template, jsonify
from translation.code_trans import to_Tel
from translation.data.tel_data import data_type
from translation.translate import variable as telugu_variable
from langdetect import detect
from langdetect.lang_detect_exception import LangDetectException
from easygoogletranslate import EasyGoogleTranslate

if not hasattr(ast, "Str"):
    ast.Str = ast.Constant
if not hasattr(ast, "Num"):
    ast.Num = ast.Constant
if not hasattr(ast, "NameConstant"):
    ast.NameConstant = ast.Constant
if not hasattr(ast, "Ellipsis"):
    ast.Ellipsis = ast.Constant

app = Flask(__name__)

lang = False


@app.route("/", methods=["GET", "POST"])
def index():
    if request.method == "POST":
        query: str | None = request.form.get("query")
        if query:
            query = query.strip()
            if not query:
                return jsonify({"error": "Error"})
            global lang
            lang = detect_telugu(query)
            if query == "help":
                return jsonify({"output": "సింటాక్స్ లోపం"})
            return jsonify({"output": translate(query)})
        return jsonify({"error": "Error"})
    return render_template("index.html")


SYNTAX_ERROR = "syntax error"
MISSING_BINARY_ERROR = (
    "Translation engine binary not found. Build cdecl-bin-telugu/cdecl-telugu "
    "and cdecl-bin-code/cdecl first."
)

current_directory: str = os.getcwd()


def fallback_parse_c_declaration(query: str) -> dict[str, str | int] | None:
    """Parse a subset of C declarations for fallback translation."""
    cleaned = query.strip().rstrip(";")
    if not cleaned:
        return None

    type_pattern = (
        r"(?:unsigned|signed|long|short|const|volatile|register|static|extern|auto|"
        r"void|char|int|float|double|struct\s+\w+|union\s+\w+|enum\s+\w+)"
    )
    type_expr = rf"(?:{type_pattern})(?:\s+(?:{type_pattern}))*"

    fn_match = re.fullmatch(
        rf"(?P<type>{type_expr})\s+(?P<name>[A-Za-z_]\w*)\s*\(\s*\)",
        cleaned,
    )
    if fn_match:
        return {
            "kind": "function",
            "type": " ".join(fn_match.group("type").split()),
            "name": fn_match.group("name"),
        }

    arr_match = re.fullmatch(
        rf"(?P<type>{type_expr})\s+(?P<name>[A-Za-z_]\w*)\s*\[\s*(?P<size>\d*)\s*\]",
        cleaned,
    )
    if arr_match:
        return {
            "kind": "array",
            "type": " ".join(arr_match.group("type").split()),
            "name": arr_match.group("name"),
            "size": arr_match.group("size"),
        }

    ptr_match = re.fullmatch(
        rf"(?P<type>{type_expr})\s+(?P<stars>\*+)\s*(?P<name>[A-Za-z_]\w*)",
        cleaned,
    )
    if ptr_match:
        return {
            "kind": "pointer",
            "type": " ".join(ptr_match.group("type").split()),
            "name": ptr_match.group("name"),
            "pointer_depth": len(ptr_match.group("stars")),
        }

    var_match = re.fullmatch(
        rf"(?P<type>{type_expr})\s+(?P<name>[A-Za-z_]\w*)",
        cleaned,
    )
    if var_match:
        return {
            "kind": "variable",
            "type": " ".join(var_match.group("type").split()),
            "name": var_match.group("name"),
        }

    return None


def fallback_type_to_telugu(c_type: str) -> str:
    parts = c_type.split()
    translated: list[str] = []
    i = 0
    while i < len(parts):
        token = parts[i]
        if token in ("struct", "union", "enum", "class") and i + 1 < len(parts):
            translated.append(data_type.get(token, token))
            translated.append(telugu_variable(parts[i + 1]))
            i += 2
            continue
        translated.append(data_type.get(token, token))
        i += 1
    return " ".join(translated)


def fallback_translate_c_to_telugu(query: str) -> str | None:
    parsed = fallback_parse_c_declaration(query)
    if not parsed:
        return None

    name = str(parsed["name"])
    name_tel = telugu_variable(name)
    c_type_tel = fallback_type_to_telugu(str(parsed["type"]))
    kind = str(parsed["kind"])

    if kind == "variable":
        return f"ప్రకటించండి {name_tel} ను {c_type_tel} గా"

    if kind == "pointer":
        pointer_depth = int(parsed["pointer_depth"])
        if pointer_depth == 1:
            pointer_label = "పాయింటర్"
        else:
            pointer_label = f"{pointer_depth} స్థాయి పాయింటర్"
        return f"ప్రకటించండి {name_tel} ను {c_type_tel} కి {pointer_label} గా"

    if kind == "array":
        size = str(parsed.get("size", ""))
        if size:
            return f"ప్రకటించండి {name_tel} ను {c_type_tel} యొక్క {size} అంశాల శ్రేణిగా"
        return f"ప్రకటించండి {name_tel} ను {c_type_tel} యొక్క శ్రేణిగా"

    if kind == "function":
        return f"ప్రకటించండి {name_tel} ను {c_type_tel} ను రిటర్న్ చేసే ఫంక్షన్‌గా"

    return None


@lru_cache(None)
def translate(query: str) -> str:
    command = []
    if lang:
        query, command = tel_to_code(query)
    else:
        query, command = code_to_tel(query)

    if not command:
        return to_Tel(query)

    print(f"translate : {translate.cache_info()}")
    print(query)

    queries: list[str] = [query, f"explain {query};", f"declare {query};"]

    translated_text = None
    try:
        with subprocess.Popen(
            command, stdin=subprocess.PIPE, stdout=subprocess.PIPE
        ) as process:
            output, _ = process.communicate(input="\n".join(queries).encode())
            for line in output.splitlines():
                line = line.decode()
                if line and line != SYNTAX_ERROR:
                    print(line)
                    if lang:
                        translated_text = fallback_translate_c_to_telugu(line) or line
                    else:
                        translated_text = to_Tel(line)
                    break
    except OSError:
        fallback = fallback_translate_c_to_telugu(query)
        if fallback:
            return fallback
        return MISSING_BINARY_ERROR

    return translated_text or to_Tel(SYNTAX_ERROR)


def detect_telugu(text: str) -> bool:
    try:
        language = detect(text)
        return language == "te"
    except LangDetectException:
        return False


def code_to_tel(query: str) -> tuple[str, list[str]]:
    if not query:
        return SYNTAX_ERROR, []

    command = [os.path.join(current_directory, "cdecl-bin-telugu/cdecl-telugu")]
    storage_classes: list[str] = ["auto", "extern", "static", "register"]
    q_l: list[str] = query.split()
    if q_l[0] in ("declare", "cast"):
        return SYNTAX_ERROR, []
    if len(q_l) < 3 and q_l[0] in storage_classes:
        query = f"{q_l[0]} int {q_l[1]}"
    return query, command


def tel_to_code(text):
    command = [os.path.join(current_directory, "cdecl-bin-code/cdecl")]
    translator = EasyGoogleTranslate(
        source_language="te", target_language="en", timeout=10
    )
    eng = translator.translate(text).lower().split()
    if not eng:
        return SYNTAX_ERROR, []
    print(eng)

    # Normalize common transliteration artifacts to C declaration tokens.
    eng = ["*" if word in ("star", "asterisk") else word for word in eng]

    if "a" in eng:
        i = 3
        print(eng[:i],eng[i:])
        eng = eng[:i] + [w for w in eng[i:] if w != "a"]
    if eng[-1] == "car":
        eng[-1]="char"
    formatted_eng = " ".join(eng)
    return formatted_eng, command


if __name__ == "__main__":
    app.run(host="127.0.0.1", port=5000, debug=True)
