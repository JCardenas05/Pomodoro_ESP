import unicodedata

def normalize_text(text):
    if not isinstance(text, str):
        return text
    text = text.replace("ñ", "n").replace("Ñ", "N")
    text = unicodedata.normalize("NFKD", text).encode("ASCII", "ignore").decode("utf-8")
    return text
