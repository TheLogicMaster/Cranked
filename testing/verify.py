#!/usr/bin/python3
import sys
from abc import ABC, abstractmethod, abstractproperty
from PIL import Image
from io import BytesIO
import os
import zlib
import struct
import json
from typing import Any
import enum
import argparse
from diff_match_patch import diff_match_patch
import deepdiff
import PyQt6
from PyQt6.QtWidgets import QApplication, QDialog, QWidget

PALETTE = (
    (0x32, 0x2F, 0x28, 0xFF),
    (0xB1, 0xAE, 0xA7, 0xFF)
)


class BinaryReader:
    def __init__(self, source: str|BytesIO):
        if type(source) is str:
            with open(source, 'rb') as f:
                self.data = BytesIO(f.read())
        else:
            self.data = source

    def read(self, count: int = -1):
        read = self.data.read(count)
        self.advance(count)
        return read

    def decompress(self, count: int = -1):
        return BinaryReader(BytesIO(zlib.decompress(self.read(count))))

    def u8(self) -> int:
        return struct.unpack("<B", self.read(1))[0]

    def u16(self) -> int:
        return struct.unpack("<H", self.read(2))[0]

    def u32(self) -> int:
        return struct.unpack("<L", self.read(4))[0]

    def i8(self) -> int:
        return struct.unpack("<b", self.read(1))[0]

    def i16(self) -> int:
        return struct.unpack("<h", self.read(2))[0]

    def i32(self) -> int:
        return struct.unpack("<l", self.read(4))[0]

    def string(self):
        value = bytearray()
        while True:
            read = self.read(1)[0]
            if not read:
                return str(value, 'utf-8')
            value.append(read)

    def seek(self, offset: int):
        self.data.seek(offset)

    def advance(self, offset: int):
        self.data.seek(offset, os.SEEK_CUR)

    def align(self, alignment: int):
        self.advance((alignment - self.tell() % alignment) % alignment)

    def tell(self):
        return self.data.tell()


def read_image(file_path: str):
    reader = BinaryReader(file_path)

    flags = reader.u32()
    compressed = bool(flags & 0x80000000)
    if compressed:
        reader.advance(16)
        reader = reader.decompress()

    width = reader.u16()
    height = reader.u16()
    stride = reader.u16()
    clip_left = reader.u16()
    clip_right = reader.u16()
    clip_top = reader.u16()
    clip_bottom = reader.u16()
    flags = reader.u32()
    alpha = bool(flags & 0x3)
    total_width = clip_left + width + clip_right
    total_height = clip_top + height + clip_bottom

    def read() -> list[list[bool]]:
        rows = []
        for y in range(height):
            row = []
            rows.append(row)
            x = 0
            for i in range(stride):
                byte = reader.u8()
                for j in range(8):
                    row.append(bool(byte & 1 << 7 - j))
                    x += 1
                    if x >= width:
                        break
        return rows

    pixels = read()
    mask = read() if alpha else [[True for _ in range(width)] for _ in range(height)]

    image = Image.new("RGBA", (total_width, total_height), (0, 0, 0, 0))
    for y in range(height):
        for x in range(width):
            if mask[y][x]:
                image.putpixel((x + clip_left, y + clip_top), PALETTE[int(pixels[y][x])])
    return image


class Result:
    def __init__(self, name: str, result_type: str, options: dict[str, Any], expected, actual):
        self.name = name
        self.type = result_type
        self.options = options
        self.expected = expected
        self.actual = actual

        self.result = False
        self.summary = ''
        self.result_image: Image.Image | None = None

        if (validator := getattr(self, f'_validate_{self.type}', None)) is not None:
            validator()
        else:
            self._validate_exact()

    def _validate_exact(self):
        diff = deepdiff.DeepDiff(self.expected, self.actual)
        self.result = len(diff.affected_paths) == 0
        self.summary = diff.pretty()

    def _validate_number(self):
        return self._validate_exact()

    def _validate_int(self):
        return self._validate_exact()

    def _validate_bool(self):
        return self._validate_exact()

    def _validate_text(self):
        return self._validate_exact()

    def _validate_image(self):
        return self._validate_exact()


class Test:
    def __init__(self, name: str, duration: float):
        self.name = name
        self.duration = duration
        self.results: list[Result] = []


def read_tests(expected_dir: str, actual_dir: str) -> list[Test]:
    tests = []
    for test_name in next(os.walk(expected_dir))[1]:
        with open(os.path.join(expected_dir, test_name, 'results.json'), 'r') as f:
            info = json.load(f)
        actual_info = None
        actual_results = {}
        try:
            with open(os.path.join(actual_dir, test_name, 'results.json'), 'r') as f:
                actual_info = json.load(f)
                if 'results' in actual_info:
                    actual_results = actual_info['results']
        except IOError: pass
        test = Test(test_name, info['duration'])
        for name, expected in info['results'].items():
            actual = actual_results[name] if name in actual_results else None
            actual_value = actual['value'] if 'value' in actual else None
            options = info['options'] if 'options' in expected else dict()
            test.results.append(Result(name, expected['type'], options, expected['value'], actual_value))
        tests.append(test)
    return tests


def main():
    tests = read_tests(sys.argv[1], sys.argv[2])
    for test in tests:
        for result in test.results:
            print(f'{test.name} [{result.name}]: {"pass" if result.result else "fail"}: {result.summary}')


if __name__ == '__main__':
    main()
