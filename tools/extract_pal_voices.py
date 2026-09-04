#!/usr/bin/env python3
"""Extract CTR PAL voice packs without transcoding.

The output mirrors the PAL disc XA layout expected by ctr-native:
  pal-voices/XA/ITL.XNF
  pal-voices/XA/ITL/EXTRA/Sxx.XA
  pal-voices/XA/ITL/GAME/Sxx.XA

.XNF files are copied as ISO9660 data. .XA files are copied as original
2352-byte raw CD sectors, preserving the Mode2 XA subheaders and ADPCM data.
"""

from __future__ import annotations

import argparse
import math
import shutil
import struct
import sys
from dataclasses import dataclass
from pathlib import Path

RAW_SECTOR = 2352
FORM1_OFFSET = 24
FORM1_SIZE = 2048
PVD_LBA = 16
PVD_ROOT_OFFSET = 156
SYNC = b"\x00" + (b"\xff" * 10) + b"\x00"
LANGUAGES = ("ENG", "FRN", "GRM", "ITL", "SPN", "DCH")


@dataclass(frozen=True)
class Record:
    lba: int
    size: int
    flags: int
    name: str

    @property
    def is_dir(self) -> bool:
        return bool(self.flags & 0x02)


class RawPsxDisc:
    def __init__(self, path: Path):
        self.path = path
        self.fp = path.open("rb")
        size = path.stat().st_size
        if size % RAW_SECTOR:
            raise ValueError(f"{path} is not a raw 2352-byte-sector BIN")
        self.sectors = size // RAW_SECTOR
        pvd = self.read_raw_sector(PVD_LBA)
        data = pvd[FORM1_OFFSET : FORM1_OFFSET + FORM1_SIZE]
        if data[0] != 1 or data[1:6] != b"CD001":
            raise ValueError("ISO9660 primary volume descriptor not found at LBA 16")
        self.root = self._parse_record(data[PVD_ROOT_OFFSET:])

    def close(self) -> None:
        self.fp.close()

    def __enter__(self) -> "RawPsxDisc":
        return self

    def __exit__(self, *_exc) -> None:
        self.close()

    def read_raw_sector(self, lba: int) -> bytes:
        if lba < 0 or lba >= self.sectors:
            raise ValueError(f"LBA out of range: {lba}")
        self.fp.seek(lba * RAW_SECTOR)
        sector = self.fp.read(RAW_SECTOR)
        if len(sector) != RAW_SECTOR or sector[:12] != SYNC:
            raise ValueError(f"invalid raw CD sector at LBA {lba}")
        return sector

    @staticmethod
    def _parse_record(src: bytes) -> Record:
        if not src or src[0] < 34:
            raise ValueError("invalid ISO9660 directory record")
        length = src[0]
        src = src[:length]
        lba = struct.unpack_from("<I", src, 2)[0]
        size = struct.unpack_from("<I", src, 10)[0]
        flags = src[25]
        name_len = src[32]
        raw_name = src[33 : 33 + name_len]
        if raw_name == b"\x00":
            name = "."
        elif raw_name == b"\x01":
            name = ".."
        else:
            name = raw_name.decode("ascii", errors="strict").split(";", 1)[0]
        return Record(lba, size, flags, name)

    def read_form1_file(self, record: Record) -> bytes:
        remaining = record.size
        out = bytearray()
        for index in range(math.ceil(record.size / FORM1_SIZE)):
            sector = self.read_raw_sector(record.lba + index)
            payload = sector[FORM1_OFFSET : FORM1_OFFSET + FORM1_SIZE]
            take = min(remaining, FORM1_SIZE)
            out += payload[:take]
            remaining -= take
        if remaining != 0:
            raise ValueError(f"short Form1 file: {record.name}")
        return bytes(out)

    def list_dir(self, record: Record) -> list[Record]:
        if not record.is_dir:
            raise ValueError(f"not a directory: {record.name}")
        data = self.read_form1_file(record)
        result: list[Record] = []
        offset = 0
        while offset < len(data):
            length = data[offset]
            if length == 0:
                offset = ((offset // FORM1_SIZE) + 1) * FORM1_SIZE
                continue
            if offset + length > len(data):
                break
            result.append(self._parse_record(data[offset : offset + length]))
            offset += length
        return result

    def find(self, path: str) -> Record:
        current = self.root
        for component in (p for p in path.replace("\\", "/").split("/") if p):
            matches = [r for r in self.list_dir(current) if r.name.upper() == component.upper()]
            if not matches:
                raise FileNotFoundError(path)
            current = matches[0]
        return current

    def copy_raw_xa(self, record: Record, output: Path) -> int:
        # CTR's XA ISO records use the logical 2048-byte file length. The actual
        # audio must retain the complete raw Mode2 sectors, so copy the extent
        # as 2352-byte sectors instead of reading the ISO user-data payload.
        sector_count = math.ceil(record.size / FORM1_SIZE)
        output.parent.mkdir(parents=True, exist_ok=True)
        with output.open("wb") as dst:
            for index in range(sector_count):
                dst.write(self.read_raw_sector(record.lba + index))
        return sector_count


def extract_language(disc: RawPsxDisc, code: str, output_root: Path) -> tuple[int, int]:
    xa_root = output_root / "XA"
    xnf = disc.find(f"XA/{code}.XNF")
    xnf_data = disc.read_form1_file(xnf)
    if len(xnf_data) < 0x44 or xnf_data[:4] != b"XINF":
        raise ValueError(f"{code}.XNF is not a CTR XINF manifest")
    xa_root.mkdir(parents=True, exist_ok=True)
    (xa_root / f"{code}.XNF").write_bytes(xnf_data)

    file_count = 0
    sector_count = 0
    for category in ("EXTRA", "GAME"):
        directory = disc.find(f"XA/{code}/{category}")
        entries = sorted(
            (r for r in disc.list_dir(directory) if not r.is_dir and r.name.upper().endswith(".XA")),
            key=lambda r: r.name.upper(),
        )
        if not entries:
            raise ValueError(f"no XA files found in {code}/{category}")
        for entry in entries:
            sector_count += disc.copy_raw_xa(entry, xa_root / code / category / entry.name.upper())
            file_count += 1
    return file_count, sector_count


def main() -> int:
    parser = argparse.ArgumentParser(description="Extract CTR PAL voices as original raw XA sectors (no transcoding).")
    parser.add_argument("disc", type=Path, help="raw PAL Crash Team Racing .bin (2352-byte sectors)")
    parser.add_argument(
        "--language",
        action="append",
        type=lambda value: value.upper(),
        choices=LANGUAGES,
        help="language to extract; repeat for multiple languages (default: all six PAL languages)",
    )
    parser.add_argument("--output", type=Path, default=Path("pal-voices"), help="voice-pack output directory (default: pal-voices)")
    args = parser.parse_args()

    languages = tuple(dict.fromkeys(args.language or LANGUAGES))
    if not args.disc.is_file():
        parser.error(f"disc image not found: {args.disc}")

    args.output.mkdir(parents=True, exist_ok=True)
    with RawPsxDisc(args.disc) as disc:
        # Reject unrelated images early.
        for required in ("XA/ENG.XNF", "XA/ITL.XNF", "XA/FRN.XNF"):
            disc.find(required)
        print(f"Source: {args.disc}")
        print(f"Output: {args.output}")
        for code in languages:
            files, sectors = extract_language(disc, code, args.output)
            raw_bytes = sectors * RAW_SECTOR
            print(f"{code}: {files} XA files, {sectors} raw sectors, {raw_bytes / (1024 * 1024):.1f} MiB")

    print("Done. Copy this directory to ux0:data/ctr/mods/pal-voices on the Vita.")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, FileNotFoundError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        raise SystemExit(1)
