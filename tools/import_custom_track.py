#!/usr/bin/env python3

import argparse
import hashlib
import re
import shutil
import struct
import subprocess
import sys
import tempfile
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

BI_LANGUAGEFILE = 234
ENGLISH_LANGUAGE_INDEX = 1
LNG_FIRST_TRACK_NAME = 0x6E  # LNG_DINGO_CANYON; race tracks only

PACKAGE_MAGIC = b"CTRT"
PACKAGE_VERSION = 1
PACKAGE_FLAG_BATTLE = 1 << 0
PACKAGE_FLAG_NO_AI_PATHS = 1 << 1
PACKAGE_HEADER_STRUCT = struct.Struct("<4sHHhH64s64s" + "II" * 2)
NAME_DISPLAY_LEN = 32

SLOT_NAMES = (
    "Dingo Canyon", "Dragon Mines", "Blizzard Bluff", "Crash Cove", "Tiger Temple",
    "Papu's Pyramid", "Roo's Tubes", "Hot Air Skyway", "Sewer Speedway", "Mystery Caves",
    "Cortex Castle", "N. Gin Labs", "Polar Pass", "Oxide Station", "Coco Park",
    "Tiny Arena", "Slide Coliseum", "Turbo Track", "Nitro Court", "Rampage Ruins",
    "Parking Lot", "Skull Rock", "The North Bowl", "Rocky Road", "Lab Basement",
)

# Levels left over from a shared base ROM: they show up in many unrelated
# track patches next to the real track.
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


def slugify(text: str) -> str:
    return re.sub(r"[^a-z0-9]+", "_", text.lower()).strip("_")


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


def parse_lng_strings(raw: bytes):
    if len(raw) < 8:
        return []
    num_strings, pointer_array_offset = struct.unpack_from("<ii", raw, 0)
    if num_strings < 0 or pointer_array_offset < 8 or pointer_array_offset + num_strings * 4 > len(raw):
        return []
    strings = []
    for index in range(num_strings):
        string_offset = read_u32(raw, pointer_array_offset + index * 4)
        string_end = raw.find(b"\0", string_offset) if string_offset < len(raw) else -1
        strings.append(raw[string_offset:string_end].decode("latin-1", errors="replace") if string_end >= 0 else "")
    return strings


def patched_track_names(base_big: BigFile, patched_big: BigFile):
    """Race track names the patch renamed in the English language file."""
    base_strings = parse_lng_strings(base_big.read_entry(BI_LANGUAGEFILE + ENGLISH_LANGUAGE_INDEX))
    patched_strings = parse_lng_strings(patched_big.read_entry(BI_LANGUAGEFILE + ENGLISH_LANGUAGE_INDEX))
    names = {}
    for slot in range(FIRST_BATTLE_SLOT):
        index = LNG_FIRST_TRACK_NAME + slot
        if index >= len(base_strings) or index >= len(patched_strings):
            continue
        name = " ".join(patched_strings[index].split())
        if name and name != base_strings[index]:
            names[slot] = name
    return names


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

    tracks = []
    seen = set()
    for candidate in candidates:
        digest = hashlib.md5(candidate.lev).hexdigest()
        if digest in seen or digest in KNOWN_LEFTOVER_LEVS or not candidate.vrm:
            continue
        seen.add(digest)
        tracks.append(candidate)
    return tracks


def build_package(track: TrackCandidate, name: str, author: str) -> bytes:
    flags = 0
    if track.slot >= FIRST_BATTLE_SLOT:
        flags |= PACKAGE_FLAG_BATTLE
    if not lev_has_ai_paths(track.lev):
        flags |= PACKAGE_FLAG_NO_AI_PATHS

    vrm_offset = PACKAGE_HEADER_STRUCT.size
    lev_offset = (vrm_offset + len(track.vrm) + 3) & ~3
    header = PACKAGE_HEADER_STRUCT.pack(
        PACKAGE_MAGIC, PACKAGE_VERSION, PACKAGE_HEADER_STRUCT.size, track.slot, flags,
        fixed_string(name, 64), fixed_string(author, 64),
        vrm_offset, len(track.vrm), lev_offset, len(track.lev))
    return header + track.vrm + b"\0" * (lev_offset - vrm_offset - len(track.vrm)) + track.lev


def output_paths(output: Path, tracks):
    if len(tracks) == 1:
        return [output if output.suffix.lower() == ".ctrt" else output / "custom_track.ctrt"]
    directory = output.parent if output.suffix.lower() == ".ctrt" else output
    prefix = output.stem + "_" if output.suffix.lower() == ".ctrt" else ""
    paths = []
    for track in tracks:
        suffix = slugify(SLOT_NAMES[track.slot])
        if sum(t.slot == track.slot for t in tracks) > 1:
            suffix += f"_{track.lod + 1}"
        paths.append(directory / f"{prefix}{suffix}.ctrt")
    return paths


def main() -> int:
    parser = argparse.ArgumentParser(description="Import a CTR custom track XDelta as a High Octane .ctrt package")
    parser.add_argument("base", type=Path, help="clean NTSC-U raw ctr-u.bin")
    parser.add_argument("patch", type=Path, help="custom track .xdelta patch")
    parser.add_argument("output", type=Path,
                        help="output .ctrt package; patches holding several tracks use this as a filename prefix or output directory")
    parser.add_argument("--name", help="display name override; otherwise uses the patched track name or the patch file name")
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
            renamed = patched_track_names(base_big, patched_big)
        finally:
            base_iso.close()
            patched_iso.close()

    paths = output_paths(args.output, tracks)
    for track, path in zip(tracks, paths):
        if args.name:
            name = args.name
        elif track.slot in renamed:
            name = renamed[track.slot]
        else:
            name = normalize_name(args.patch.stem)
            if len(tracks) > 1:
                name += f" ({SLOT_NAMES[track.slot]})"
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
