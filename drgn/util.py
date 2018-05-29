# Copyright 2018 - Omar Sandoval
# SPDX-License-Identifier: GPL-3.0+

from typing import Dict, Iterable, List, TextIO


def escape_character(c: int, escape_single_quote: bool = False,
                     escape_double_quote: bool = False,
                     escape_backslash: bool = False) -> str:
    if c == 0:
        return r'\0'
    elif c == 7:
        return r'\a'
    elif c == 8:
        return r'\b'
    elif c == 9:
        return r'\t'
    elif c == 10:
        return r'\n'
    elif c == 11:
        return r'\v'
    elif c == 12:
        return r'\f'
    elif c == 13:
        return r'\r'
    elif escape_double_quote and c == 34:
        return r'\"'
    elif escape_single_quote and c == 39:
        return r"\'"
    elif escape_backslash and c == 92:
        return r'\\'
    elif 32 <= c <= 126:
        return chr(c)
    else:
        return f'\\x{c:02x}'


def escape_string(buffer: Iterable[int], escape_single_quote: bool = False,
                  escape_double_quote: bool = False,
                  escape_backslash: bool = False) -> str:
    return ''.join(escape_character(c, escape_single_quote=escape_single_quote,
                                    escape_double_quote=escape_double_quote,
                                    escape_backslash=escape_backslash)
                   for c in buffer)


def c_string(buffer: Iterable[int]) -> str:
    parts = ['"']
    parts.extend(escape_character(c, escape_double_quote=True,
                                  escape_backslash=True)
                 for c in buffer)
    parts.append('"')
    return ''.join(parts)


def parse_symbol_file(file: TextIO) -> Dict[str, List[int]]:
    symbols: Dict[str, List[int]] = {}
    for line in file:
        fields = line.split()
        name = fields[2]
        if fields[0] == '(null)':
            address = 0
        else:
            address = int(fields[0], 16)
        try:
            symbols[name].append(address)
        except KeyError:
            symbols[name] = [address]
    return symbols
