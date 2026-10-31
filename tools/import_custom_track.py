#!/usr/bin/env python3

import argparse
import hashlib
import math
import re
import shutil
import struct
import subprocess
import sys
import tempfile
import zlib
from dataclasses import dataclass
from pathlib import Path
from typing import Optional

RAW_SECTOR = 2352
FORM1_OFFSET = 24
FORM1_SIZE = 2048
PVD_LBA = 16
PVD_ROOT_OFFSET = 156
BIGFILE_ENTRY_COUNT = 608

# Race tracks and battle arenas (DINGO_CANYON .. LAB_BASEMENT), 8 bigfile
# entries each: {1P, 2P, 4P, relic} x {vrm, lev}.
LEVEL_SLOT_COUNT = 25
FIRST_BATTLE_SLOT = 18
FILES_PER_SLOT = 8
LOD_COUNT = 4
# PS1_TrackROM based patches put their level here, played on Dingo Canyon.
TRACKROM_LEV_ENTRY = 221
TRACKROM_VRM_ENTRY = 222
TRACKROM_SLOT = 0

PACKAGE_MAGIC = b"CTRT"
PACKAGE_VERSION = 1
PACKAGE_FLAG_BATTLE = 1 << 0
PACKAGE_FLAG_NO_AI_PATHS = 1 << 1
PACKAGE_HEADER_STRUCT = struct.Struct("<4sHHhH64s64s6h" + "II" * 3)
NAME_DISPLAY_LEN = 32

# Minimap: the track's ground seen from above, white, north up, with a white
# border round it that smooths its jagged triangle edges. The game draws it
# like the stock maps (track select layers, race HUD with the karts' icons).
MAP_W, MAP_H = 104, 80  # (HUD pixels) the map fits in this, aspect kept: the stock maps' size
MAP_SCALE = 2           # image pixels per HUD pixel
MAP_SAMPLES = 4         # coverage samples per image pixel, each way
MAP_BORDER = 2          # (HUD pixels) the white border
MAP_ROAD_NEAR = 700     # ground this close to the checkpoint path is the track

# Level layout (include/namespace_Level.h); pointers are offsets into the level body
LEVEL_MESH_INFO = 0x0
LEVEL_RESTART_COUNT = 0x148
LEVEL_RESTART_POINTS = 0x14C
CHECKPOINT_NODE_SIZE = 0xC
VERTEX_SIZE = 0x10
QUADBLOCK_SIZE = 0x5C
QUAD_GROUND = 0x1000
QUAD_TRIGGER = 0x40
QUAD_KILL_PLANE = 0x200
# QuadBlock.index[9] -> its high detail triangles (the last 4 only on quads)
QUAD_TRIANGLES = ((8, 6, 7), (7, 3, 8), (1, 7, 6), (2, 6, 8), (0, 4, 5), (4, 6, 5), (6, 4, 1), (5, 6, 2))

SLOT_NAMES = (
    "Dingo Canyon", "Dragon Mines", "Blizzard Bluff", "Crash Cove", "Tiger Temple",
    "Papu's Pyramid", "Roo's Tubes", "Hot Air Skyway", "Sewer Speedway", "Mystery Caves",
    "Cortex Castle", "N. Gin Labs", "Polar Pass", "Oxide Station", "Coco Park",
    "Tiny Arena", "Slide Coliseum", "Turbo Track", "Nitro Court", "Rampage Ruins",
    "Parking Lot", "Skull Rock", "The North Bowl", "Rocky Road", "Lab Basement",
)

# Levels left over from shared base ROMs: they show up next to the real track
# in many unrelated track patches.
KNOWN_LEFTOVER_LEVS = {
    "0d8b78c424bc63bc6edd93d34b364439", "47c74da07befac5792b52a42b2f031eb", "5b01eabe8da3098327b1ecff7d9786eb",
    "6668d23f61580f66816ebf3a4265d974", "d9f685c211ada441ae10b7f5de943794",
}

# Characters the in-game font can draw.
FONT_UNSUPPORTED = re.compile(r"[^A-Z0-9 !%'+,\-./:<=>?]")


@dataclass
class IsoEntry:
    lba: int
    size: int
    flags: int


@dataclass
class TrackCandidate:
    slot: int
    lod: int
    vrm: bytes
    lev: bytes


class RawIso:
    def __init__(self, path: Path):
        self.path = path
        self.fp = path.open("rb")
        if path.stat().st_size % RAW_SECTOR:
            raise ValueError(f"{path} is not a raw MODE2/2352 image")
        self.files = {}
        self._parse_filesystem()

    def close(self):
        self.fp.close()

    def read_sector(self, lba: int) -> bytes:
        self.fp.seek(lba * RAW_SECTOR + FORM1_OFFSET)
        data = self.fp.read(FORM1_SIZE)
        if len(data) != FORM1_SIZE:
            raise ValueError(f"Short sector read at LBA {lba}")
        return data

    @staticmethod
    def _parse_record(data: bytes, off: int):
        length = data[off]
        if length == 0:
            return None, off
        rec = data[off:off + length]
        if len(rec) < 34:
            raise ValueError("Invalid ISO directory record")
        lba = struct.unpack_from("<I", rec, 2)[0]
        size = struct.unpack_from("<I", rec, 10)[0]
        flags = rec[25]
        name_len = rec[32]
        return (rec[33:33 + name_len], IsoEntry(lba, size, flags)), off + length

    def _read_entry(self, entry: IsoEntry, offset=0, size=None) -> bytes:
        if size is None:
            size = entry.size - offset
        if offset < 0 or size < 0 or offset + size > entry.size:
            raise ValueError("ISO read out of range")
        out = bytearray()
        sector_index = offset // FORM1_SIZE
        sector_offset = offset % FORM1_SIZE
        remaining = size
        while remaining:
            sector = self.read_sector(entry.lba + sector_index)
            take = min(remaining, FORM1_SIZE - sector_offset)
            out += sector[sector_offset:sector_offset + take]
            remaining -= take
            sector_index += 1
            sector_offset = 0
        return bytes(out)

    def _read_directory(self, entry: IsoEntry, prefix: str):
        data = self._read_entry(entry)
        off = 0
        while off < len(data):
            if data[off] == 0:
                off = ((off // FORM1_SIZE) + 1) * FORM1_SIZE
                continue
            parsed, next_off = self._parse_record(data, off)
            if parsed is None:
                break
            name_raw, child = parsed
            off = next_off
            if name_raw in (b"\x00", b"\x01"):
                continue
            name = name_raw.decode("ascii", errors="replace").split(";", 1)[0]
            full = f"{prefix}/{name}" if prefix else name
            self.files[full.upper()] = child
            if child.flags & 0x02:
                self._read_directory(child, full)

    def _parse_filesystem(self):
        pvd = self.read_sector(PVD_LBA)
        if pvd[0] != 1 or pvd[1:6] != b"CD001":
            raise ValueError("Invalid ISO9660 PVD")
        parsed, _ = self._parse_record(pvd, PVD_ROOT_OFFSET)
        if parsed is None:
            raise ValueError("Missing ISO root")
        _, root = parsed
        self._read_directory(root, "")

    def read_range(self, path: str, offset: int, size: int) -> bytes:
        return self._read_entry(self.files[path.upper()], offset, size)


class BigFile:
    def __init__(self, iso: RawIso):
        header = iso.read_range("BIGFILE.BIG", 0, 8 + BIGFILE_ENTRY_COUNT * 8)
        self.iso = iso
        self.cdpos, self.count = struct.unpack_from("<II", header, 0)
        if self.count != BIGFILE_ENTRY_COUNT:
            raise ValueError(f"Unsupported BIGFILE entry count: {self.count}")
        self.entries = [struct.unpack_from("<II", header, 8 + i * 8) for i in range(self.count)]

    def read_entry(self, index: int) -> bytes:
        sector, size = self.entries[index]
        return self.iso.read_range("BIGFILE.BIG", sector * FORM1_SIZE, size) if size else b""

    def changed(self, other, index: int) -> bool:
        if self.entries[index][1] != other.entries[index][1]:
            return True
        return self.read_entry(index) != other.read_entry(index)


def find_xdelta3(explicit: Optional[str]) -> str:
    candidates = []
    if explicit:
        candidates.append(explicit)
    script_dir = Path(__file__).resolve().parent
    candidates += [str(script_dir / "xdelta3.exe"), str(script_dir / "xdelta_temp.exe"), str(script_dir / "xdelta3")]
    found = shutil.which("xdelta3") or shutil.which("xdelta3.exe")
    if found:
        candidates.append(found)
    for candidate in candidates:
        if candidate and Path(candidate).is_file():
            return candidate
    raise RuntimeError("xdelta3 was not found. Pass --xdelta3 <path>.")


def fixed_string(text: str, size: int) -> bytes:
    raw = text.encode("utf-8", errors="replace")[:size - 1]
    return raw + b"\0" * (size - len(raw))


def font_text(text: str) -> str:
    """Upper case, only characters the in-game font has."""
    text = text.upper().replace("’", "'").replace("–", "-").replace("&", "+")
    text = re.sub(r"\s*[(\[]\s*", " - ", text).replace(")", " ").replace("]", " ")
    text = FONT_UNSUPPORTED.sub(" ", text)
    return " ".join(text.split())[:NAME_DISPLAY_LEN].strip()


def normalize_name(stem: str) -> str:
    name = stem
    for suffix in (".xdelta", "_xdelta", ".vcdiff", "_vcdiff"):
        if name.lower().endswith(suffix):
            name = name[:-len(suffix)]
    name = name.replace("_", " ").replace("-", " ")
    return " ".join(name.split()) or "Custom Track"


def read_u32(data: bytes, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def lev_length(lev: bytes) -> int:
    """Real length of [u32 mapOffset][Level ...][u32 mapBytes][pointer map], or 0 if malformed.

    Slot replacement patches keep the stock file size, so the new level is
    usually followed by padding or leftovers of the original.
    """
    if len(lev) < 0x200:
        return 0
    map_offset = read_u32(lev, 0)
    if 4 + map_offset + 4 > len(lev):
        return 0
    map_bytes = read_u32(lev, 4 + map_offset)
    end = 4 + map_offset + 4 + map_bytes
    return end if end <= len(lev) and map_bytes % 4 == 0 else 0


def lev_has_ai_paths(lev: bytes) -> bool:
    body = lev[4:]
    nav_table = read_u32(body, 0x188)  # Level.LevNavTable
    return 0 < nav_table < len(body) - 12 and any(struct.unpack_from("<III", body, nav_table))


def ground_triangles(lev: bytes):
    """(x, z) corners of the level's drivable ground triangles."""
    body = lev[4:]
    mesh = read_u32(body, LEVEL_MESH_INFO)
    num_quads, num_verts, _, quads, verts = struct.unpack_from("<5I", body, mesh)
    if quads + num_quads * QUADBLOCK_SIZE > len(body) or verts + num_verts * VERTEX_SIZE > len(body):
        return []
    pos = [struct.unpack_from("<3h", body, verts + i * VERTEX_SIZE) for i in range(num_verts)]
    triangles = []
    for q in range(num_quads):
        base = quads + q * QUADBLOCK_SIZE
        index = struct.unpack_from("<9H", body, base)
        flags = struct.unpack_from("<H", body, base + 0x12)[0]
        if (flags & QUAD_GROUND) == 0 or (flags & (QUAD_TRIGGER | QUAD_KILL_PLANE)) != 0 or max(index) >= num_verts:
            continue
        for k, tri in enumerate(QUAD_TRIANGLES):
            if k >= 4 and index[2] == index[3]:
                break
            a, b, c = (index[i] for i in tri)
            if a != b and b != c and a != c:
                triangles.append(tuple((pos[i][0], pos[i][2]) for i in (a, b, c)))
    return triangles


def checkpoint_path(lev: bytes):
    """(x, z) of the checkpoints in driving order, closed, or [] (battle arenas)."""
    body = lev[4:]
    count, points = struct.unpack_from("<iI", body, LEVEL_RESTART_COUNT)
    if count < 3 or points + count * CHECKPOINT_NODE_SIZE > len(body):
        return []
    nodes = [struct.unpack_from("<3hH4B", body, points + i * CHECKPOINT_NODE_SIZE) for i in range(count)]
    path, i, seen = [], 0, set()
    while i < count and i not in seen:
        seen.add(i)
        path.append((nodes[i][0], nodes[i][2]))
        i = nodes[i][4]  # nextIndex_forward
    return path + path[:1] if len(path) >= 3 else []


def distance_to_path(x, z, path):
    best = float("inf")
    for (ax, az), (bx, bz) in zip(path, path[1:]):
        dx, dz = bx - ax, bz - az
        length = dx * dx + dz * dz
        t = 0.0 if length == 0 else max(0.0, min(1.0, ((x - ax) * dx + (z - az) * dz) / length))
        px, pz = ax + t * dx - x, az + t * dz - z
        best = min(best, px * px + pz * pz)
    return math.sqrt(best)


def png_gray_alpha(width: int, height: int, alpha_rows) -> bytes:
    def chunk(kind, data):
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data) & 0xFFFFFFFF)
    raw = b"".join(b"\0" + bytes(v for a in row for v in (255, a)) for row in alpha_rows)
    return (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 4, 0, 0, 0)) +
            chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b""))


def build_minimap(lev: bytes):
    """(png, (x0, x1, z0, z1, w, h)) or None: the image, the world range it
    spans and its size in HUD pixels."""
    triangles = ground_triangles(lev)
    path = checkpoint_path(lev)
    if path:
        near = [t for t in triangles
                if distance_to_path(sum(p[0] for p in t) / 3, sum(p[1] for p in t) / 3, path) < MAP_ROAD_NEAR]
        triangles = near or triangles
    if not triangles:
        return None

    xs = [p[0] for t in triangles for p in t]
    zs = [p[1] for t in triangles for p in t]
    span_x, span_z = max(max(xs) - min(xs), 1), max(max(zs) - min(zs), 1)
    fit = min((MAP_W - 2 * MAP_BORDER - 2) / span_x, (MAP_H - 2 * MAP_BORDER - 2) / span_z)
    w = int(math.ceil(span_x * fit)) + 2 * MAP_BORDER + 2
    h = int(math.ceil(span_z * fit)) + 2 * MAP_BORDER + 2
    h += h & 1  # (the game's UIMap halves the height)
    cx, cz = (min(xs) + max(xs)) / 2, (min(zs) + max(zs)) / 2
    x0, x1 = cx - w / 2 / fit, cx + w / 2 / fit
    z0, z1 = cz - h / 2 / fit, cz + h / 2 / fit

    # coverage grid: one bit per sample, a Python int per row (north = +z up)
    sw, sh = w * MAP_SCALE * MAP_SAMPLES, h * MAP_SCALE * MAP_SAMPLES
    rows = [0] * sh
    for tri in triangles:
        pts = [((x - x0) / (x1 - x0) * sw, (z1 - z) / (z1 - z0) * sh) for x, z in tri]
        top = max(0, int(math.floor(min(p[1] for p in pts))))
        bottom = min(sh - 1, int(math.ceil(max(p[1] for p in pts))))
        for y in range(top, bottom + 1):
            yc = y + 0.5
            crossings = []
            for (ax, ay), (bx, by) in zip(pts, pts[1:] + pts[:1]):
                if (ay <= yc < by) or (by <= yc < ay):
                    crossings.append(ax + (yc - ay) * (bx - ax) / (by - ay))
            if len(crossings) >= 2:
                left = max(0, int(math.floor(min(crossings) + 0.5)))
                right = min(sw - 1, int(math.floor(max(crossings) - 0.5)))
                if right >= left:
                    rows[y] |= ((1 << (right - left + 1)) - 1) << left

    # the white border: the shape grown by a disk
    radius = MAP_BORDER * MAP_SCALE * MAP_SAMPLES
    full = (1 << sw) - 1
    half_widths = [int(math.sqrt(radius * radius - dy * dy)) for dy in range(radius + 1)]
    spread = {}
    for hw in set(half_widths):
        out = []
        for row in rows:
            grown, done, step = row, 0, 1
            while done < hw:
                step = min(step, hw - done)
                grown |= (grown << step) | (grown >> step)
                done += step
                step *= 2
            out.append(grown & full)
        spread[hw] = out
    grown_rows = []
    for y in range(sh):
        row = 0
        for dy in range(-radius, radius + 1):
            if 0 <= y + dy < sh:
                row |= spread[half_widths[abs(dy)]][y + dy]
        grown_rows.append(row)

    # each image pixel's alpha: its samples covered
    block = (1 << MAP_SAMPLES) - 1
    samples = MAP_SAMPLES * MAP_SAMPLES
    alpha_rows = []
    for y in range(h * MAP_SCALE):
        sub = grown_rows[y * MAP_SAMPLES:(y + 1) * MAP_SAMPLES]
        alpha_rows.append([sum(bin((r >> (x * MAP_SAMPLES)) & block).count("1") for r in sub) * 255 // samples
                           for x in range(w * MAP_SCALE)])
    bounds = tuple(int(round(v)) for v in (x0, x1, z0, z1)) + (w, h)
    if any(not -32768 <= v <= 32767 for v in bounds):
        return None  # (the game keeps the range in 16 bits)
    return png_gray_alpha(w * MAP_SCALE, h * MAP_SCALE, alpha_rows), bounds


def find_tracks(base_big: BigFile, patched_big: BigFile):
    candidates = []
    for slot in range(LEVEL_SLOT_COUNT):
        for lod in range(LOD_COUNT):
            vrm_index = slot * FILES_PER_SLOT + lod * 2
            lev_index = vrm_index + 1
            if not base_big.changed(patched_big, lev_index):
                continue
            lev = patched_big.read_entry(lev_index)
            length = lev_length(lev)
            if length:
                candidates.append(TrackCandidate(slot, lod, patched_big.read_entry(vrm_index), lev[:length]))

    if base_big.changed(patched_big, TRACKROM_LEV_ENTRY):
        lev = patched_big.read_entry(TRACKROM_LEV_ENTRY)
        length = lev_length(lev)
        if length:
            candidates.append(TrackCandidate(TRACKROM_SLOT, -1, patched_big.read_entry(TRACKROM_VRM_ENTRY), lev[:length]))

    # Custom track ROMs run a 1P-only engine: of a slot's changed LODs only the
    # first is the track (the others are copies or leftovers).
    tracks = []
    seen_slots, seen_levs = set(), set()
    for candidate in candidates:
        digest = hashlib.md5(candidate.lev).hexdigest()
        key = (candidate.slot, candidate.lod < 0)
        if digest in KNOWN_LEFTOVER_LEVS or digest in seen_levs or key in seen_slots or not candidate.vrm:
            continue
        seen_levs.add(digest)
        seen_slots.add(key)
        tracks.append(candidate)
    return tracks


def build_package(track: TrackCandidate, name: str, author: str) -> bytes:
    flags = 0
    if track.slot >= FIRST_BATTLE_SLOT:
        flags |= PACKAGE_FLAG_BATTLE
    if not lev_has_ai_paths(track.lev):
        flags |= PACKAGE_FLAG_NO_AI_PATHS

    minimap = build_minimap(track.lev)
    map_png, map_bounds = minimap if minimap else (b"", (0, 0, 0, 0, 0, 0))

    vrm_offset = PACKAGE_HEADER_STRUCT.size
    lev_offset = (vrm_offset + len(track.vrm) + 3) & ~3
    map_offset = (lev_offset + len(track.lev) + 3) & ~3
    header = PACKAGE_HEADER_STRUCT.pack(
        PACKAGE_MAGIC, PACKAGE_VERSION, PACKAGE_HEADER_STRUCT.size, track.slot, flags,
        fixed_string(name, 64), fixed_string(author, 64), *map_bounds,
        vrm_offset, len(track.vrm), lev_offset, len(track.lev), map_offset if map_png else 0, len(map_png))
    return (header + track.vrm + b"\0" * (lev_offset - vrm_offset - len(track.vrm)) +
            track.lev + b"\0" * (map_offset - lev_offset - len(track.lev)) + map_png)


def output_paths(output: Path, count: int):
    if count == 1:
        return [output if output.suffix.lower() == ".ctrt" else output / "custom_track.ctrt"]
    directory = output.parent if output.suffix.lower() == ".ctrt" else output
    stem = output.stem if output.suffix.lower() == ".ctrt" else "custom_track"
    return [directory / (f"{stem}.ctrt" if n == 0 else f"{stem}_{n + 1}.ctrt") for n in range(count)]


def main() -> int:
    parser = argparse.ArgumentParser(description="Import a CTR custom track XDelta as a High Octane .ctrt package")
    parser.add_argument("base", type=Path, help="clean NTSC-U raw ctr-u.bin")
    parser.add_argument("patch", type=Path, help="custom track .xdelta patch")
    parser.add_argument("output", type=Path,
                        help="output .ctrt package; patches holding several tracks use this as a filename prefix or output directory")
    parser.add_argument("--name", help="display name; otherwise the patch file name")
    parser.add_argument("--author", default="", help="track author")
    parser.add_argument("--xdelta3", help="path to xdelta3 executable")
    args = parser.parse_args()

    xdelta = find_xdelta3(args.xdelta3)
    if not args.base.is_file() or not args.patch.is_file():
        raise RuntimeError("base image or patch does not exist")

    with tempfile.TemporaryDirectory(prefix="ctr-track-") as temp_dir:
        patched_path = Path(temp_dir) / "patched.bin"
        subprocess.run([xdelta, "-d", "-s", str(args.base), str(args.patch), str(patched_path)], check=True)

        base_iso = RawIso(args.base)
        patched_iso = RawIso(patched_path)
        try:
            base_big = BigFile(base_iso)
            patched_big = BigFile(patched_iso)
            tracks = find_tracks(base_big, patched_big)
            if not tracks:
                raise RuntimeError("no changed track or battle arena levels were found")
        finally:
            base_iso.close()
            patched_iso.close()

    paths = output_paths(args.output, len(tracks))
    for n, (track, path) in enumerate(zip(tracks, paths)):
        name = args.name or normalize_name(args.patch.stem)
        if n:
            name += f" {n + 1}"  # (a patch holding several tracks)
        name = font_text(name) or "CUSTOM TRACK"

        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(build_package(track, name, args.author))
        no_ai = track.slot < FIRST_BATTLE_SLOT and not lev_has_ai_paths(track.lev)
        mode_note = " (no AI paths: not offered in Arcade)" if no_ai else ""
        print(f"Wrote {path}: '{name}' replacing {SLOT_NAMES[track.slot]}{mode_note}")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (RuntimeError, ValueError, subprocess.CalledProcessError) as error:
        print(f"error: {error}", file=sys.stderr)
        sys.exit(1)
