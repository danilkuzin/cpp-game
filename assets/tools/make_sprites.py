#!/usr/bin/env python3
"""Generates the game's BMP sprites from ASCII art.

The art lives here as text so it stays reviewable in diffs; the .bmp files are
build products that happen to be committed (SDL_LoadBMP needs them at runtime).

    python3 assets/tools/make_sprites.py        # rewrites assets/*.bmp

Sprites are 16x16 and are drawn at 50x50 with SDL_SCALEMODE_NEAREST.
'.' is the magenta color key (255, 0, 255): TextureManager makes it
transparent, so entities and items keep the terrain visible underneath.
Terrain tiles are fully opaque and must never contain '.'.

Generated with CLAUDE
"""

import os
import struct

SIZE = 16
KEY = (255, 0, 255)

PALETTE = {
    ".": KEY,
    "k": (20, 18, 26),  # outline
    "z": (26, 26, 32),  # shadow / void
    "w": (240, 240, 240),  # white
    "W": (222, 211, 180),  # bone
    "g": (184, 188, 196),  # steel light
    "G": (125, 131, 143),  # steel mid
    "d": (86, 91, 102),  # steel dark
    "r": (192, 57, 43),  # red
    "R": (231, 76, 60),  # red bright
    "y": (232, 195, 60),  # gold
    "Y": (245, 224, 138),  # gold pale
    "n": (122, 82, 48),  # wood
    "N": (163, 111, 66),  # wood light
    "b": (59, 110, 165),  # blue
    "B": (106, 160, 216),  # blue light
    "e": (74, 143, 60),  # green
    "E": (111, 191, 85),  # green light
    "v": (47, 91, 40),  # green dark
    "p": (125, 74, 158),  # purple
    "P": (169, 115, 201),  # purple light
    "f": (216, 160, 122),  # flesh
    "F": (176, 122, 88),  # flesh shade
    "u": (196, 120, 70),  # copper
    "U": (230, 160, 100),  # copper light
    "x": (58, 58, 64),  # chitin
    "X": (90, 90, 98),  # chitin light
    "a": (51, 80, 47),  # grass dark
    "A": (67, 102, 60),  # grass light
    "h": (107, 83, 64),  # dirt
    "H": (125, 99, 80),  # dirt light
    "q": (76, 76, 85),  # stone
    "Q": (94, 94, 105),  # stone light
    "t": (47, 109, 122),  # water
    "T": (61, 138, 153),  # water light
}


def Humanoid(skin, skin_dark, cloth, cloth_light):
    """Shared body plan for the bipedal creatures, recolored per species."""
    template = [
        ".....OOOOOO.....",
        "....OSSSSSSO....",
        "....OSDSSDSO....",
        "....OSSSSSSO....",
        "....OSDDDDSO....",
        ".....OSSSSO.....",
        "...OOOOOOOOOO...",
        "..OSOCCCCCCOSO..",
        "..OSOCCLLCCOSO..",
        "..OSOCCCCCCOSO..",
        "..OOOCCCCCCOOO..",
        "....OCCCCCCO....",
        "....OSSOOSSO....",
        "....OSSOOSSO....",
        "...OSSOOOOSSO...",
        "...OOOO..OOOO...",
    ]
    mapping = {"O": "k", "S": skin, "D": skin_dark, "C": cloth, "L": cloth_light}
    return [row.translate(str.maketrans(mapping)) for row in template]


SPRITES = {
    # ---------------------------------------------------------------- terrain
    "tile_grass": [
        "aaAaaaaaAaaaaaAa",
        "aaAaaAaaAaaaaaAa",
        "aaaaaAaaaaaAaaaa",
        "AaaaaAaaaaaAaaaA",
        "AaaaaaaaAaaaaaaA",
        "aaaAaaaaAaaaAaaa",
        "aaaAaaaaaaaaAaaa",
        "aaaaaaaAaaaaaaaa",
        "aAaaaaaAaaaAaaaa",
        "aAaaaaaaaaaAaaaa",
        "aaaaAaaaaaaaaaAa",
        "aaaaAaaAaaaaaaAa",
        "aaAaaaaAaaaaAaaa",
        "aaAaaaaaaaaaAaaa",
        "aaaaaaAaaaAaaaaa",
        "aaaaaaAaaaAaaaaa",
    ],
    "tile_ground": [
        "hhhhhhhhhhhhhhhh",
        "hhHhhhhhhhhzhhhh",
        "hhHhhhhzhhhhhhhh",
        "hhhhhhhzhhhhhHhh",
        "hhhhhhhhhhhhhHhh",
        "hzhhhhhhhHhhhhhh",
        "hzhhhhhhhHhhhhzh",
        "hhhhhhhhhhhhhhzh",
        "hhhhHhhhhhhhhhhh",
        "hhhhHhhhhhzhhhhh",
        "hhhhhhhhhhzhhhHh",
        "hhzhhhhhhhhhhhHh",
        "hhzhhhHhhhhhhhhh",
        "hhhhhhHhhhhhhhhh",
        "hhhhhhhhhhHhhhhh",
        "hhhhhhhhhhHhhhhh",
    ],
    "tile_stone": [
        "zzzzzzzzzzzzzzzz",
        "zQQQQQQzQQQQQQQz",
        "zqqqqqqzqqqqqqqz",
        "zqqqqqqzqqqqqqqz",
        "zqqqqqqzqqqqqqqz",
        "zzzzzzzzzzzzzzzz",
        "zQQQQzQQQQQQQQQz",
        "zqqqqzqqqqqqqqqz",
        "zqqqqzqqqqqqqqqz",
        "zqqqqzqqqqqqqqqz",
        "zzzzzzzzzzzzzzzz",
        "zQQQQQQQQQzQQQQz",
        "zqqqqqqqqqzqqqqz",
        "zqqqqqqqqqzqqqqz",
        "zqqqqqqqqqzqqqqz",
        "zzzzzzzzzzzzzzzz",
    ],
    "tile_tree": [
        "aaaaaaaaaaaaaaaa",
        "aaaaaavvvvaaaaaa",
        "aaaavvveevvvaaaa",
        "aaavveeeeeevvaaa",
        "aavveeeeeeeevvaa",
        "aavveeeeEeeevvaa",
        "avvveeeeEeeevvva",
        "avvveeeeeeeevvva",
        "aavveeeeeeeevvaa",
        "aaavveeeeeevvaaa",
        "aaaavvvnnvvvaaaa",
        "aaaaaaannaaaaaaa",
        "aaaaaaannaaaaaaa",
        "aaaaaaannaaaaaaa",
        "aaaaaannnnaaaaaa",
        "aaaaaaaaaaaaaaaa",
    ],
    "tile_trap": [
        "hhhhhhhhhhhhhhhh",
        "hhhhhhhhhhhhhhhh",
        "hhzzzzzzzzzzzzhh",
        "hzzzzzzzzzzzzzzh",
        "hzzgzzgzzgzzgzzh",
        "hzzgzzgzzgzzgzzh",
        "hzzGzzGzzGzzGzzh",
        "hzzzzzzzzzzzzzzh",
        "hzzzzzzzzzzzzzzh",
        "hzzgzzgzzgzzgzzh",
        "hzzgzzgzzgzzgzzh",
        "hzzGzzGzzGzzGzzh",
        "hzzzzzzzzzzzzzzh",
        "hhzzzzzzzzzzzzhh",
        "hhhhhhhhhhhhhhhh",
        "hhhhhhhhhhhhhhhh",
    ],
    "tile_live": [
        "tttttttttttttttt",
        "ttTTTTtttttttttt",
        "tttttttttTTTTttt",
        "tttttttttttttttt",
        "tTTTtttttttTTTtt",
        "tttttttttttttttt",
        "ttttttTTTTtttttt",
        "tttttttttttttttt",
        "ttTTtttttttTTTTt",
        "tttttttttttttttt",
        "ttttttttTTTttttt",
        "tttttttttttttttt",
        "tTTTTttttttttttt",
        "ttttttttttTTTttt",
        "tttttttttttttttt",
        "tttttttttttttttt",
    ],
    "tile_stairs_down": [
        "qqqqqqqqqqqqqqqq",
        "qzzzzzzzzzzzzzzq",
        "qzQQQQQQQQQQQQzq",
        "qzQqqqqqqqqqqQzq",
        "qzzzzzzzzzzzzzzq",
        "qzzQQQQQQQQQQzzq",
        "qzzQqqqqqqqqQzzq",
        "qzzzzzzzzzzzzzzq",
        "qzzzQQQQQQQQzzzq",
        "qzzzQqqqqqqQzzzq",
        "qzzzzzzzzzzzzzzq",
        "qzzzzQQQQQQzzzzq",
        "qzzzzQqqqqQzzzzq",
        "qzzzzzzzzzzzzzzq",
        "qzzzzzzzzzzzzzzq",
        "qqqqqqqqqqqqqqqq",
    ],
    "tile_stairs_up": [
        "qqqqqqqqqqqqqqqq",
        "qzzzzzwwwwzzzzzq",
        "qzzzzzwQQwzzzzzq",
        "qzzzzwwwwwwzzzzq",
        "qzzzzwQQQQwzzzzq",
        "qzzzwwwwwwwwzzzq",
        "qzzzwQQQQQQwzzzq",
        "qzzwwwwwwwwwwzzq",
        "qzzwQQQQQQQQwzzq",
        "qzwwwwwwwwwwwwzq",
        "qzwQQQQQQQQQQwzq",
        "qwwwwwwwwwwwwwwq",
        "qwQQQQQQQQQQQQwq",
        "qwwwwwwwwwwwwwwq",
        "qzzzzzzzzzzzzzzq",
        "qqqqqqqqqqqqqqqq",
    ],
    # --------------------------------------------------------------- creatures
    "hero": [
        ".....kkkkkk.....",
        "....kyyyyyyk....",
        "....kyYYYYyk....",
        "....kffffffk....",
        "....kfkffkfk....",
        "....kffffffk....",
        ".....kfffk......",
        "...kkkbbbbkkk...",
        "..kgkbbbbbbkgk..",
        "..kgkbBBBBbkgk..",
        "..kkkbbbbbbkkk..",
        "....kbbbbbbk....",
        "....knnkknnk....",
        "....knnkknnk....",
        "...kNNkkkkNNk...",
        "...kkkk..kkkk...",
    ],
    "orc": Humanoid("e", "v", "n", "N"),
    "troll": Humanoid("v", "z", "h", "H"),
    # Not built from Humanoid(): a bare skull is what makes it read as undead.
    "skeleton": [
        "................",
        ".....kkkkkk.....",
        "....kWWWWWWk....",
        "....kWkWWkWk....",
        "....kWkWWkWk....",
        "....kWWWWWWk....",
        "....kWkkkkWk....",
        ".....kWWWWk.....",
        "...kkkkWWkkkk...",
        "..kWkWWWWWWkWk..",
        "..kWkWkWWkWkWk..",
        "..kWkWWWWWWkWk..",
        "..kkkWkWWkWkkk..",
        "....kWWWWWWk....",
        "...kWWkkkkWWk...",
        "...kkkk..kkkk...",
    ],
    "shopkeeper": Humanoid("f", "F", "p", "P"),
    # Small, low and long-tailed, so it never reads as a smaller wolf.
    "rat": [
        "................",
        "................",
        "................",
        "................",
        "................",
        "...kk.......kkk.",
        "...knk.....kk.k.",
        "..kknkkkkkkk..k.",
        ".kwnnnnnnnnk..k.",
        ".kwFnnnnnnnk..k.",
        ".kknnnnnnnnk.kk.",
        "..kkkkkkkkkk....",
        "..kk..kk..kk....",
        "..kk..kk..kk....",
        "................",
        "................",
    ],
    "wolf": [
        "................",
        "................",
        ".kk.........kk..",
        ".kGk.......kGk..",
        "kkGGkkkkkkkGGk..",
        "kGGGGGGGGGGGGGk.",
        "kGGdGGGGGGGwdk..",
        "kGGGGGGGGGGGGk..",
        ".kGGGGGGGGGGk...",
        "..kkkkkkkkkkk...",
        "..kGk.kGk.kGk...",
        "..kGk.kGk.kGk...",
        "..kGk.kGk.kGk...",
        ".kkkk.kkk.kkkk..",
        "................",
        "................",
    ],
    "giant_spider": [
        "................",
        "k..............k",
        ".k....kkkk....k.",
        "..k..kxxxxk..k..",
        "...kkxXxxXxkk...",
        "....kxxxxxxk....",
        "...kkxxxxxxkk...",
        "..k.kxxxxxxk.k..",
        ".k..kxRxxRxk..k.",
        "k...kxxxxxxk...k",
        ".k.kkxxxxxxkk.k.",
        "..k.kxxxxxxk.k..",
        "...k.kxxxxk.k...",
        "..k...kkkk...k..",
        ".k...........k..",
        "k.............k.",
    ],
    "giant_snake": [
        "................",
        "....kkkkkkk.....",
        "...keeeeeeek....",
        "..keeEeeEeeek...",
        "..keRekekeReek..",
        "..keeeeeeeeeek..",
        "...keeeeeeeek...",
        "....kkkkkeeek...",
        "..kkkkkkkkeeek..",
        ".keeeeeeeeeeek..",
        "keeEeeEeeEeeek..",
        "keeeeeeeeeekkk..",
        ".keeeeeeeeek....",
        "..kkkkkkkkkk....",
        "................",
        "................",
    ],
    # ------------------------------------------------------------------- items
    "sword": [
        "........kk......",
        ".......kggk.....",
        "......kggGk.....",
        "......kggGk.....",
        "......kggGk.....",
        "......kggGk.....",
        "......kggGk.....",
        "......kggGk.....",
        "....kkkggGkkk...",
        "....kyyyyyyyk...",
        "....kkkyyykkk...",
        "......knnk......",
        "......knnk......",
        "......knNk......",
        ".....kkyykk.....",
        ".....kkkkkk.....",
    ],
    "axe": [
        "................",
        "...kkkk..kkkk...",
        "..kggggkkggggk..",
        ".kgggggkkgggggk.",
        ".kgggGGnnGGgggk.",
        ".kggGGGnnGGGggk.",
        "..kGGGGnnGGGGk..",
        "...kkkGnnGkkk...",
        "......knnk......",
        "......knnk......",
        "......knNk......",
        "......knnk......",
        "......knnk......",
        "......knNk......",
        ".....kknnkk.....",
        ".....kkkkkk.....",
    ],
    "helm": [
        "................",
        "................",
        "......kkkk......",
        "....kkggggkk....",
        "...kggggggggk...",
        "..kgggggggggggk.",
        "..kggGGGGGGggGk.",
        "..kgkkkggkkkgGk.",
        "..kgkzzkkzzkgGk.",
        "..kggGGGGGGggGk.",
        "..kgGGkGGkGGgGk.",
        "..kgGGkGGkGGgGk.",
        "...kGGGGGGGGGk..",
        "....kkGGGGkkk...",
        "......kkkk......",
        "................",
    ],
    "bodyarmor": [
        "................",
        ".kkk........kkk.",
        "kggkkkkkkkkkkggk",
        "kgGkgGGGGGGgkGgk",
        "kggkgGGGGGGgkggk",
        ".kkkgGGGGGGgkkk.",
        "...kgddddddgk...",
        "...kgGGGGGGgk...",
        "...kgGGGGGGgk...",
        "...kgddddddgk...",
        "...kgGGGGGGgk...",
        "...kgGGGGGGgk...",
        "...kggggggggk...",
        "...kgGGGGGGgk...",
        "...kkkkkkkkkk...",
        "................",
    ],
    "ammo": [
        "................",
        "..k....k....k...",
        ".kwk..kwk..kwk..",
        "kwkwkkwkwkkwkwk.",
        ".knk..knk..knk..",
        ".knk..knk..knk..",
        ".knk..knk..knk..",
        ".knk..knk..knk..",
        ".knk..knk..knk..",
        ".knk..knk..knk..",
        "kkNkkkkNkkkkNkk.",
        "kwNwkkwNwkkwNwk.",
        "kwNwkkwNwkkwNwk.",
        ".kNk..kNk..kNk..",
        ".kkk..kkk..kkk..",
        "................",
    ],
    "cross": [
        "................",
        "......kkkk......",
        ".....kyYYyk.....",
        ".....kyYYyk.....",
        "..kkkkyYYykkkk..",
        ".kyyyyyYYyyyyyk.",
        ".kyYYYYYYYYYYyk.",
        ".kkkkkyYYykkkkk.",
        ".....kyYYyk.....",
        ".....kyYYyk.....",
        ".....kyYYyk.....",
        ".....kyYYyk.....",
        ".....kyYYyk.....",
        "....kkyYYykk....",
        "....kyyyyyyk....",
        ".....kkkkkk.....",
    ],
    "magic_item": [
        "......kkkk......",
        "......kGGk......",
        "......kGGk......",
        ".....kkGGkk.....",
        "....kkpppkk.....",
        "...kkpppppkk....",
        "..kkppppppPkk...",
        "..kpppPpppPpk...",
        "..kppppppppkw...",
        "..kpPppppppk.w..",
        "..kpppppPppkw...",
        "..kkppppppkk....",
        "...kkppppkk.....",
        "....kkkkkk......",
        "................",
        "................",
    ],
    "shield": [
        "......kkkk......",
        ".....kggggk.....",
        "....kggGGggk....",
        "...kggGwwGggk...",
        "...kggGwwGggk...",
        "...kggGGGGggk...",
        "...kgGGnnGGgk...",
        "...kgGGnnGGgk...",
        "...kgGGnnGGgk...",
        "....kGGnnGGk....",
        "....kGGnnGGk....",
        ".....kGnnGk.....",
        ".....kGnnGk.....",
        "......knnk......",
        ".......kk.......",
        "................",
    ],
    "ring": [
        "................",
        "................",
        "................",
        "......kkkk......",
        ".....kyyyyk.....",
        "....kyyYYyyk....",
        "....kyY..Yyk....",
        "....kyY..Yyk....",
        "....kyY..Yyk....",
        "....kyyYYyyk....",
        ".....kyyyyk.....",
        "......kkkk......",
        "................",
        "................",
        "................",
        "................",
    ],
    "copper": [
        "................",
        "................",
        ".....kkkkkk.....",
        "....kuUUUUuk....",
        "...kuUUUUUUuk...",
        "...kuUwwwwUuk...",
        "...kuUUUUUUuk...",
        "...kkuuuuuukk...",
        "...kuUUUUUUuk...",
        "...kuUwwwwUuk...",
        "...kuUUUUUUuk...",
        "...kkuuuuuukk...",
        "...kuUUUUUUuk...",
        "...kuuuuuuuuk...",
        "....kkkkkkkk....",
        "................",
    ],
}


def WriteBmp(path, rows):
    """Writes a 24-bit uncompressed BMP (the only format SDL_LoadBMP needs here)."""
    stride = (SIZE * 3 + 3) // 4 * 4
    pixels = bytearray()
    for row in reversed(rows):  # BMP rows are bottom-up
        line = bytearray()
        for char in row:
            red, green, blue = PALETTE[char]
            line += bytes((blue, green, red))
        line += b"\0" * (stride - len(line))
        pixels += line

    offset = 14 + 40
    header = struct.pack("<2sIHHI", b"BM", offset + len(pixels), 0, 0, offset)
    info = struct.pack("<IiiHHIIiiII", 40, SIZE, SIZE, 1, 24, 0, len(pixels), 2835,
                       2835, 0, 0)
    with open(path, "wb") as handle:
        handle.write(header + info + pixels)


def Main():
    assets = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    for name, rows in sorted(SPRITES.items()):
        assert len(rows) == SIZE, f"{name}: {len(rows)} rows, expected {SIZE}"
        for index, row in enumerate(rows):
            assert len(row) == SIZE, f"{name} row {index}: {len(row)} px -> {row!r}"
            unknown = set(row) - set(PALETTE)
            assert not unknown, f"{name} row {index}: unknown chars {unknown}"
        if name.startswith("tile_"):
            assert not any("." in row for row in rows), f"{name}: terrain must be opaque"
        WriteBmp(os.path.join(assets, f"{name}.bmp"), rows)
        print(f"wrote {name}.bmp")


if __name__ == "__main__":
    Main()
