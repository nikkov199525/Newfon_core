"""Regression checks against the core's phoneme output; NVDA is not needed."""
import ctypes
import os
from pathlib import Path
import subprocess
import sys
import unittest

CORE = Path(__file__).resolve().parents[1]


class Numbers(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        build = CORE / "build" / "tests"
        build.mkdir(parents=True, exist_ok=True)
        library = build / ("numerics.dll" if sys.platform == "win32" else "numerics.so")
        compiler = os.environ.get("CC", "x86_64-w64-mingw32-clang" if sys.platform == "win32" else "cc")
        command = [compiler, "-shared", "-Wall", "-Wextra", "-Werror",
                   "-Wno-unused-parameter", "-Wno-sign-compare"]
        if sys.platform != "win32":
            command.append("-fPIC")
        command += ["-I", str(CORE / "include"), "-I", str(CORE / "src"),
                    str(CORE / "tests" / "numerics_capture.c")]
        command += [str(CORE / "src" / name) for name in ("numerics.c", "transcription.c", "sink.c")]
        subprocess.run(command + ["-o", str(library)], check=True)
        cls.library = ctypes.CDLL(str(library))
        cls.library.transcribe.argtypes = [ctypes.c_char_p, ctypes.c_int,
                                          ctypes.POINTER(ctypes.c_ubyte), ctypes.c_size_t, ctypes.c_int]
        cls.library.transcribe.restype = ctypes.c_size_t

    def transcription(self, text, flags=3, cancel=False):
        buffer = (ctypes.c_ubyte * 100000)()
        length = self.library.transcribe(text.encode("koi8-r"), flags, buffer, len(buffer), cancel)
        # Compare words and phonemes, ignoring buffer padding and repeated spaces.
        tokens = bytearray()
        for code in buffer[:length]:
            tokens.extend([43, code, 43] if 42 <= code <= 52 else [code])
        return [word for word in bytes(tokens).split(bytes([43])) if word]

    def assertReading(self, number, words, flags=3):
        # The historical numeric tables simplify these consonant clusters.
        expected = words.replace("ннадцать", "нацать").replace("дцать", "цать")
        self.assertEqual(self.transcription(number, flags), self.transcription(expected, flags), number)

    def test_small_cardinals_and_thousands(self):
        for number, words in [
            ("0", "но+ль"), ("1", "оди+н"), ("12", "двена+дцать"),
            ("123", "сто+ два+дцать три+"),
            ("1001", "одна+ ты+сяча оди+н"),
            ("22000", "два+дцать две+ ты+сячи"),
            ("11000", "оди+ннадцать ты+сяч"),
            ("1000001", "оди+н милио+н оди+н"),
            ("2000000000", "два+ милиа+рда"),
            ("5000000000000", "пя+ть трилио+нов"),
        ]:
            with self.subTest(number=number):
                self.assertReading(number, words)

    def test_large_scale_declensions(self):
        # Sample early, middle and final names, as well as the requested 10^50.
        for power, name in [(15, "квадрилио+н"), (18, "квинтилио+н"),
                            (27, "октилио+н"), (51, "сексдецилио+н"),
                            (90, "новемвигинтилио+н"), (150, "новемквадрагинтилио+н")]:
            for count, spoken, suffix in [(1, "оди+н", ""), (2, "два+", "а"),
                                           (4, "четы+ре", "а"), (5, "пя+ть", "ов"),
                                           (11, "оди+ннадцать", "ов"),
                                           (21, "два+дцать оди+н", ""),
                                           (22, "два+дцать два+", "а"),
                                           (111, "сто+ оди+ннадцать", "ов")]:
                with self.subTest(power=power, count=count):
                    self.assertReading(str(count) + "0" * power, spoken + " " + name + suffix)
        self.assertReading("1" + "0" * 50, "сто+ квиндецилио+нов")

    def test_every_named_period_has_one_stress(self):
        # Each 10^(3*n) consists of "one" plus exactly one scale name.
        for period in range(2, 51):
            with self.subTest(period=period):
                words = self.transcription("1" + "0" * (3 * period))
                self.assertEqual(len(words), 2)
                self.assertEqual(words[0], self.transcription("1")[0])
                self.assertEqual(words[1].count(53), 1)

    def test_zero_counts(self):
        for count in range(2, 126):
            suffix = "но+ль" if count % 10 == 1 and count % 100 != 11 else (
                "нуля+" if count % 10 in (2, 3, 4) and count % 100 not in (12, 13, 14) else "нуле+й")
            with self.subTest(count=count):
                self.assertEqual(self.transcription("0" * count),
                                 self.transcription(str(count)) + self.transcription(suffix))
        self.assertReading("0" * 10000, "де+сять ты+сяч нуле+й")
        self.assertReading("00000123", "пя+ть нуле+й сто+ два+дцать три+")
        self.assertReading("0000, 5", "четы+ре нуля+, пя+ть")

    def test_internal_zeroes_keep_place_value(self):
        self.assertReading("1000000000000001", "оди+н квадрилио+н оди+н")
        self.assertReading("2000000000000000000000030", "два+ септилио+на три+дцать")
        self.assertReading("1000000000000000000000000000", "оди+н октилио+н")

    def test_fractions_and_flags(self):
        self.assertReading("0.0001", "но+ль це+лых одна+ десятиты+сячная")
        self.assertReading("0,0001", "но+ль це+лых одна+ десятиты+сячная")
        self.assertReading("1.5", "одна+ це+лая пя+ть деся+тых")
        self.assertReading("2,25", "две+ це+лых два+дцать пя+ть со+тых")
        self.assertReading("2.5", "два+. пя+ть", flags=0)
        self.assertReading("2,5", "два+, пя+ть", flags=0)
        # The denominator table still ends at 14 places, independently of cardinals.
        words = self.transcription("0." + "0" * 13 + "1")
        # Historical table: стотриллионная, including its unreduced first о.
        self.assertEqual(words[-1], bytes([35, 27, 1, 27, 13, 5, 17, 5, 1, 53, 16, 16, 2, 10, 2]))
        self.assertTrue(self.transcription("0." + "1" * 200))

    def test_maximum_length_and_digit_fallback(self):
        self.assertEqual(len(self.transcription("9" * 153)), 203)
        self.assertEqual(self.transcription("1" + "0" * 153),
                         self.transcription("1") + self.transcription("0") * 153)

    def test_cancellation(self):
        for text in ["9" * 153, "9" * 10000, "0" * 10000 + "123"]:
            with self.subTest(length=len(text)):
                self.assertLess(len(self.transcription(text, cancel=True)), len(self.transcription(text)))


if __name__ == "__main__":
    unittest.main()
