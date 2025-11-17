import subprocess
import os
from functools import lru_cache
from flask import Flask, request, render_template, jsonify
from translation.code_trans import to_Tel
from langdetect import detect
from easygoogletranslate import EasyGoogleTranslate

app = Flask(__name__)

lang = bool


@app.route("/", methods=["GET", "POST"])
def index():
    if request.method == "POST":
        query: str | None = request.form.get("query")
        if query:
            global lang
            lang = detect_telugu(query)
            if query == "help":
                return jsonify({"output": "సింటాక్స్ లోపం"})
            return jsonify({"output": translate(query.strip())})
        return jsonify({"error": "Error"})
    return render_template("index.html")


SYNTAX_ERROR = "syntax error"

current_directory: str = os.getcwd()


@lru_cache(None)
def translate(query: str) -> str:
    command = []
    if lang:
        query, command = tel_to_code(query)
    else:
        query, command = code_to_tel(query)
    print(f"translate : {translate.cache_info()}")
    print(query)

    queries: list[str] = [query, f"explain {query};", f"declare {query};"]

    translated_text = None
    with subprocess.Popen(
        command, stdin=subprocess.PIPE, stdout=subprocess.PIPE
    ) as process:
        output, _ = process.communicate(input="\n".join(queries).encode())
        for line in output.splitlines():
            line = line.decode()
            if line and line != SYNTAX_ERROR:
                print(line)
                if lang:
                    translated_text = line
                else:
                    translated_text = to_Tel(line)
                break

    return translated_text or to_Tel(SYNTAX_ERROR)


def detect_telugu(text: str) -> bool:
    language = detect(text)
    return language == "te"


def code_to_tel(query: str) -> str:
    command = [os.path.join(current_directory, "cdecl-bin-telugu/cdecl-telugu")]
    storage_classes: list[str] = ["auto", "extern", "static", "register"]
    q_l: list[str] = query.split()
    if q_l[0] in ("declare", "cast"):
        return to_Tel(SYNTAX_ERROR)
    if len(q_l) < 3 and q_l[0] in storage_classes:
        query = f"{q_l[0]} int {q_l[1]}"
    return query, command


def tel_to_code(text):
    command = [os.path.join(current_directory, "cdecl-bin-code/cdecl")]
    translator = EasyGoogleTranslate(
        source_language="te", target_language="en", timeout=10
    )
    eng = translator.translate(text).lower().split()
    print(eng)
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
