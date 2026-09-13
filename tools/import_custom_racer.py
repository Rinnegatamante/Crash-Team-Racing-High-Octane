#!/usr/bin/env python3

import argparse
import heapq
import hashlib
import math
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
BI_RACERMODELHI = 242
RACER_COUNT = 16
BI_SHAREDMPKVRM = 258

PACKAGE_MAGIC = b"CTRR"
PACKAGE_VERSION = 2
MAX_VOICE_FILES = 32
PACKAGE_HEADER_STRUCT = struct.Struct("<IHHhhI32s64s64s" + "II" * 3 + "I" + "B3xII" * MAX_VOICE_FILES)
ASSET_MODEL_HI = 0
ASSET_SHARED_VRM = 1
ASSET_VOICE_XNF = 2

MODEL_FILE_HEADER_SIZE = 4
MODEL_HEADER_SIZE = 0x40
MODEL_ANIM_SIZE = 0x18
MODEL_FRAME_SIZE = 0x1C
TEMPLATE_ANIMATION_NEIGHBORS = 4


@dataclass
class IsoEntry:
    lba: int
    size: int
    flags: int


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

    def read_raw_extent(self, path: str) -> bytes:
        entry = self.files[path.upper()]
        sector_count = (entry.size + FORM1_SIZE - 1) // FORM1_SIZE
        self.fp.seek(entry.lba * RAW_SECTOR)
        data = self.fp.read(sector_count * RAW_SECTOR)
        if len(data) != sector_count * RAW_SECTOR:
            raise ValueError(f"Short raw extent read for {path}")
        return data


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
        return self.iso.read_range("BIGFILE.BIG", sector * FORM1_SIZE, size)

    def changed(self, other, index: int) -> bool:
        if self.entries[index][1] != other.entries[index][1]:
            return True
        return hashlib.sha256(self.read_entry(index)).digest() != hashlib.sha256(other.read_entry(index)).digest()


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


def normalize_name(stem: str) -> str:
    name = stem
    for suffix in (".xdelta", "_xdelta"):
        if name.lower().endswith(suffix):
            name = name[:-len(suffix)]
    name = name.replace("_", " ").replace("-", " ")
    return " ".join(name.split())[:63] or "Custom Racer"


def align(value: int, alignment: int) -> int:
    return (value + alignment - 1) & ~(alignment - 1)


def xa_payload(raw: bytes) -> bytes:
    if len(raw) % RAW_SECTOR:
        return raw
    return b"".join(raw[i + 16:i + RAW_SECTOR] for i in range(0, len(raw), RAW_SECTOR))


def read_u32(data: bytes, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def read_s32(data: bytes, offset: int) -> int:
    return struct.unpack_from("<i", data, offset)[0]


def sign_extend(value: int, bits: int) -> int:
    mask = (1 << bits) - 1
    value &= mask
    sign = 1 << (bits - 1)
    return (value ^ sign) - sign


def signed_byte(value: int) -> int:
    return ((value + 128) & 0xFF) - 128


def packed_word(data: bytes, offset: int) -> int:
    chunk = data[offset:offset + 4]
    if len(chunk) < 4:
        chunk += b"\0" * (4 - len(chunk))
    return int.from_bytes(chunk, "little")


def read_signed_bits(data: bytes, bit_index: int, bits: int):
    word_index = bit_index >> 5
    end_shift = 32 - bits
    shift = end_shift - (bit_index & 31)
    word = packed_word(data, word_index * 4)
    if shift < 0:
        value = ((word << -shift) & 0xFFFFFFFF) | (packed_word(data, (word_index + 1) * 4) >> (shift & 31))
    else:
        value = word >> shift
    return sign_extend(value, bits), bit_index + bits


def model_vertex_count(body: bytes, model_header_offset: int) -> int:
    command_offset = read_u32(body, model_header_offset + 0x20)
    cursor = command_offset + 4
    count = 0
    while cursor + 4 <= len(body):
        command = read_u32(body, cursor)
        cursor += 4
        if command == 0xFFFFFFFF:
            return count
        if (command & 0xFFFF0000) == 0:
            continue
        if ((command >> 24) & 4) == 0:
            count += 1
    raise RuntimeError("unterminated model command list")


def parse_model_file(raw: bytes):
    if len(raw) < MODEL_FILE_HEADER_SIZE + 0x18:
        raise RuntimeError("racer model file is too small")
    pointer_map_offset = read_s32(raw, 0)
    if pointer_map_offset < 0 or MODEL_FILE_HEADER_SIZE + pointer_map_offset > len(raw):
        raise RuntimeError("racer model has an invalid pointer map offset")

    body = raw[MODEL_FILE_HEADER_SIZE:]
    num_headers = struct.unpack_from("<h", body, 0x12)[0]
    if num_headers != 1:
        raise RuntimeError(f"template animation retarget currently requires one model header, found {num_headers}")
    model_header_offset = read_u32(body, 0x14)
    if model_header_offset + MODEL_HEADER_SIZE > pointer_map_offset:
        raise RuntimeError("racer model header is outside the DRAM payload")

    vertex_count = model_vertex_count(body, model_header_offset)
    frame_offset = read_u32(body, model_header_offset + 0x24)
    if frame_offset != 0:
        vertex_offset = read_u32(body, frame_offset + 0x18)
        vertex_end = frame_offset + vertex_offset + vertex_count * 3
        if vertex_end > pointer_map_offset:
            raise RuntimeError("racer model frame vertex data is truncated")

    pointer_map_bytes = read_u32(body, pointer_map_offset)
    pointer_count = pointer_map_bytes // 4
    pointer_map_end = pointer_map_offset + 4 + pointer_map_bytes
    if pointer_map_end > len(body):
        raise RuntimeError("racer model pointer map is truncated")
    pointer_slots = [read_u32(body, pointer_map_offset + 4 + i * 4) for i in range(pointer_count)]

    return {
        "body": body,
        "pointer_map_offset": pointer_map_offset,
        "pointer_slots": pointer_slots,
        "model_header_offset": model_header_offset,
        "frame_offset": frame_offset,
        "vertex_count": vertex_count,
    }


def decode_model_frame(body: bytes, frame_offset: int, frame_size: int, delta_offset: int, vertex_count: int):
    pos = struct.unpack_from("<hhh", body, frame_offset)
    vertex_offset = read_u32(body, frame_offset + 0x18)
    stream = body[frame_offset + vertex_offset:frame_offset + frame_size]
    vertices = []

    if delta_offset == 0:
        required = vertex_count * 3
        if len(stream) < required:
            raise RuntimeError("uncompressed animation frame is truncated")
        for i in range(vertex_count):
            vertices.append(tuple(stream[i * 3:i * 3 + 3]))
        return pos, vertices

    bit_index = 0
    x_accum = 0
    y_accum = 0
    z_accum = 0
    for i in range(vertex_count):
        temporal = read_u32(body, delta_offset + i * 4)
        x_bits = (temporal >> 6) & 7
        z_bits = (temporal >> 3) & 7
        y_bits = temporal & 7
        x_base = sign_extend(temporal >> 25, 7) << 1
        z_base = sign_extend(temporal >> 17, 8)
        y_base = sign_extend(temporal >> 9, 8)

        value, bit_index = read_signed_bits(stream, bit_index, x_bits + 1)
        x_accum = signed_byte(value if x_bits == 7 else x_accum + value + x_base)
        value, bit_index = read_signed_bits(stream, bit_index, z_bits + 1)
        y_accum = signed_byte(value if z_bits == 7 else y_accum + value + z_base)
        value, bit_index = read_signed_bits(stream, bit_index, y_bits + 1)
        z_accum = signed_byte(value if y_bits == 7 else z_accum + value + y_base)

        # The compressed decoder stores x/z/y accumulators into x/y/z vertex bytes.
        vertices.append((x_accum & 0xFF, z_accum & 0xFF, y_accum & 0xFF))

    return pos, vertices


def frame_absolute_vertices(pos, vertices):
    # RenderBucket adds vertex.x to pos.x, vertex.z to pos.y, and vertex.y to pos.z.
    return [(pos[0] + v[0], pos[1] + v[2], pos[2] + v[1]) for v in vertices]


def animation_physical_frame_count(num_frames: int) -> int:
    logical_frames = num_frames & 0x7FFF
    if num_frames & 0x8000:
        return (logical_frames + 1) // 2
    return logical_frames


def decode_template_animations(model):
    body = model["body"]
    model_header_offset = model["model_header_offset"]
    vertex_count = model["vertex_count"]
    num_animations = read_u32(body, model_header_offset + 0x34)
    animation_array_offset = read_u32(body, model_header_offset + 0x38)
    if num_animations == 0 or animation_array_offset == 0:
        raise RuntimeError("retail template model has no animations")

    animations = []
    for animation_index in range(num_animations):
        animation_offset = read_u32(body, animation_array_offset + animation_index * 4)
        name = body[animation_offset:animation_offset + 0x10].split(b"\0", 1)[0]
        num_frames, frame_size = struct.unpack_from("<HH", body, animation_offset + 0x10)
        delta_offset = read_u32(body, animation_offset + 0x14)
        physical_frames = animation_physical_frame_count(num_frames)
        if physical_frames <= 0:
            raise RuntimeError(f"template animation {animation_index} has no frames")

        frames = []
        for frame_index in range(physical_frames):
            frame_offset = animation_offset + MODEL_ANIM_SIZE + frame_index * frame_size
            pos, vertices = decode_model_frame(body, frame_offset, frame_size, delta_offset, vertex_count)
            frames.append(frame_absolute_vertices(pos, vertices))

        animations.append({
            "name": name,
            "num_frames": num_frames,
            "frames": frames,
        })
    return animations


def nearest_vertex_mapping(custom_vertices, template_vertices, neighbor_count=TEMPLATE_ANIMATION_NEIGHBORS):
    neighbor_count = min(neighbor_count, len(template_vertices))
    mappings = []
    nearest_distances = []
    for custom_vertex in custom_vertices:
        nearest = heapq.nsmallest(
            neighbor_count,
            (((custom_vertex[0] - template_vertex[0]) ** 2 +
              (custom_vertex[1] - template_vertex[1]) ** 2 +
              (custom_vertex[2] - template_vertex[2]) ** 2, index)
             for index, template_vertex in enumerate(template_vertices)),
        )
        nearest_distances.append(math.sqrt(nearest[0][0]))
        exact = [item for item in nearest if item[0] == 0]
        if exact:
            exact_weight = 1.0 / len(exact)
            mappings.append([(index, exact_weight) for _, index in exact])
            continue
        inverse = [1.0 / distance_sq for distance_sq, _ in nearest]
        total = sum(inverse)
        mappings.append([(index, weight / total) for weight, (_, index) in zip(inverse, nearest)])
    return mappings, nearest_distances


def retarget_template_animations(custom_raw: bytes, template_raw: bytes):
    custom = parse_model_file(custom_raw)
    template = parse_model_file(template_raw)
    custom_body = custom["body"]
    custom_header_offset = custom["model_header_offset"]
    if read_u32(custom_body, custom_header_offset + 0x34) != 0 or read_u32(custom_body, custom_header_offset + 0x38) != 0:
        raise RuntimeError("custom racer already contains animations; refusing to replace them")

    custom_frame = custom["frame_offset"]
    if custom_frame == 0:
        raise RuntimeError("static custom racer model has no frame data to retarget")
    custom_vertex_offset = read_u32(custom_body, custom_frame + 0x18)
    custom_pos = struct.unpack_from("<hhh", custom_body, custom_frame)
    custom_vertices = [
        tuple(custom_body[custom_frame + custom_vertex_offset + i * 3:custom_frame + custom_vertex_offset + i * 3 + 3])
        for i in range(custom["vertex_count"])
    ]
    custom_neutral = frame_absolute_vertices(custom_pos, custom_vertices)

    animations = decode_template_animations(template)
    drive = animations[0]
    drive_logical_midpoint = (drive["num_frames"] & 0x7FFF) >> 1
    if drive["num_frames"] & 0x8000:
        drive_physical_midpoint = drive_logical_midpoint >> 1
    else:
        drive_physical_midpoint = drive_logical_midpoint
    drive_physical_midpoint = min(drive_physical_midpoint, len(drive["frames"]) - 1)
    template_neutral = drive["frames"][drive_physical_midpoint]

    mappings, nearest_distances = nearest_vertex_mapping(custom_neutral, template_neutral)
    frame_size = align(MODEL_FRAME_SIZE + custom["vertex_count"] * 3, 4)
    static_frame_header = bytearray(custom_body[custom_frame:custom_frame + MODEL_FRAME_SIZE])
    struct.pack_into("<I", static_frame_header, 0x18, MODEL_FRAME_SIZE)

    new_body = bytearray(custom_body[:custom["pointer_map_offset"]])
    while len(new_body) & 3:
        new_body.append(0)
    animation_array_offset = len(new_body)
    new_body.extend(b"\0" * (len(animations) * 4))

    animation_offsets = []
    clipped_components = 0
    for animation in animations:
        while len(new_body) & 3:
            new_body.append(0)
        animation_offset = len(new_body)
        animation_offsets.append(animation_offset)
        new_body.extend(animation["name"].ljust(0x10, b"\0")[:0x10])
        new_body.extend(struct.pack("<HHI", animation["num_frames"], frame_size, 0))

        for template_frame in animation["frames"]:
            frame_bytes = bytearray(static_frame_header)
            for custom_index, custom_vertex in enumerate(custom_neutral):
                dx = dy = dz = 0.0
                for template_index, weight in mappings[custom_index]:
                    dx += weight * (template_frame[template_index][0] - template_neutral[template_index][0])
                    dy += weight * (template_frame[template_index][1] - template_neutral[template_index][1])
                    dz += weight * (template_frame[template_index][2] - template_neutral[template_index][2])

                raw_x = round(custom_vertex[0] + dx - custom_pos[0])
                raw_z = round(custom_vertex[1] + dy - custom_pos[1])
                raw_y = round(custom_vertex[2] + dz - custom_pos[2])
                values = [raw_x, raw_y, raw_z]
                for component_index, value in enumerate(values):
                    if value < 0 or value > 255:
                        clipped_components += 1
                    values[component_index] = max(0, min(255, value))
                frame_bytes.extend(bytes(values))

            if len(frame_bytes) < frame_size:
                frame_bytes.extend(b"\0" * (frame_size - len(frame_bytes)))
            new_body.extend(frame_bytes)

    struct.pack_into("<II", new_body, custom_header_offset + 0x34, len(animations), animation_array_offset)
    for animation_index, animation_offset in enumerate(animation_offsets):
        struct.pack_into("<I", new_body, animation_array_offset + animation_index * 4, animation_offset)

    pointer_slots = list(custom["pointer_slots"])
    pointer_slots.append(custom_header_offset + 0x38)
    pointer_slots.extend(animation_array_offset + animation_index * 4 for animation_index in range(len(animations)))
    pointer_map_offset = len(new_body)
    new_body.extend(struct.pack("<I", len(pointer_slots) * 4))
    for pointer_slot in pointer_slots:
        new_body.extend(struct.pack("<I", pointer_slot))

    nearest_distances.sort()
    median_distance = nearest_distances[len(nearest_distances) // 2]
    p90_distance = nearest_distances[min(len(nearest_distances) - 1, int(len(nearest_distances) * 0.9))]
    max_distance = nearest_distances[-1]
    report = {
        "animations": [animation["name"].decode("ascii", errors="replace") for animation in animations],
        "template_vertices": template["vertex_count"],
        "custom_vertices": custom["vertex_count"],
        "median_distance": median_distance,
        "p90_distance": p90_distance,
        "max_distance": max_distance,
        "clipped_components": clipped_components,
    }
    return struct.pack("<i", pointer_map_offset) + bytes(new_body), report


def main() -> int:
    parser = argparse.ArgumentParser(description="Import a CTR racer XDelta as a High Octane .ctrr package")
    parser.add_argument("base", type=Path, help="clean NTSC-U raw ctr-u.bin")
    parser.add_argument("patch", type=Path, help="racer .xdelta patch")
    parser.add_argument("output", type=Path, help="output .ctrr package")
    parser.add_argument("--name", help="display name (defaults to patch filename)")
    parser.add_argument("--author", default="", help="mod author")
    parser.add_argument("--engine-class", type=int, default=-1, choices=(-1, 0, 1, 2, 3), help="override engine class; -1 keeps template")
    parser.add_argument("--template-animations", action="store_true",
                        help="retarget the replaced retail racer's animations onto a static custom racer model")
    parser.add_argument("--xdelta3", help="path to xdelta3 executable")
    args = parser.parse_args()

    xdelta = find_xdelta3(args.xdelta3)
    if not args.base.is_file() or not args.patch.is_file():
        raise RuntimeError("base image or patch does not exist")

    with tempfile.TemporaryDirectory(prefix="ctr-racer-") as temp_dir:
        patched_path = Path(temp_dir) / "patched.bin"
        subprocess.run([xdelta, "-d", "-s", str(args.base), str(args.patch), str(patched_path)], check=True)

        base_iso = RawIso(args.base)
        patched_iso = RawIso(patched_path)
        try:
            base_big = BigFile(base_iso)
            patched_big = BigFile(patched_iso)

            changed_models = [i for i in range(RACER_COUNT) if base_big.changed(patched_big, BI_RACERMODELHI + i)]
            if len(changed_models) != 1:
                raise RuntimeError(f"expected exactly one changed high-LOD racer model, found {changed_models}")

            template_id = changed_models[0]
            model_hi = patched_big.read_entry(BI_RACERMODELHI + template_id)
            animation_report = None
            if args.template_animations:
                template_model_hi = base_big.read_entry(BI_RACERMODELHI + template_id)
                model_hi, animation_report = retarget_template_animations(model_hi, template_model_hi)
            shared_vrm = patched_big.read_entry(BI_SHAREDMPKVRM) if base_big.changed(patched_big, BI_SHAREDMPKVRM) else b""

            clean_xnf = base_iso._read_entry(base_iso.files["XA/ENG.XNF"])
            patched_xnf = patched_iso._read_entry(patched_iso.files["XA/ENG.XNF"])
            voice_xnf = patched_xnf if clean_xnf != patched_xnf else b""
            voice_files = []
            for file_number in range(100):
                relative = f"XA/ENG/GAME/S{file_number:02d}.XA"
                if relative not in base_iso.files or relative not in patched_iso.files:
                    continue
                clean = base_iso.read_raw_extent(relative)
                patched = patched_iso.read_raw_extent(relative)
                if xa_payload(clean) != xa_payload(patched):
                    voice_files.append((file_number, patched))
            if len(voice_files) > MAX_VOICE_FILES:
                raise RuntimeError(f"too many changed GAME XA files: {len(voice_files)}")
        finally:
            base_iso.close()
            patched_iso.close()

    assets = [model_hi, shared_vrm, voice_xnf]
    offsets = []
    cursor = align(PACKAGE_HEADER_STRUCT.size, 16)
    for asset in assets:
        if asset:
            offsets.append((cursor, len(asset)))
            cursor = align(cursor + len(asset), 16)
        else:
            offsets.append((0, 0))

    voice_offsets = []
    for file_number, voice_data in voice_files:
        voice_offsets.append((file_number, cursor, len(voice_data)))
        cursor = align(cursor + len(voice_data), 16)

    display_name = args.name or normalize_name(args.patch.stem)
    source_hash = hashlib.sha256(args.patch.read_bytes()).digest()
    voice_header_values = []
    for i in range(MAX_VOICE_FILES):
        if i < len(voice_offsets):
            file_number, offset, size = voice_offsets[i]
            voice_header_values.extend((file_number, offset, size))
        else:
            voice_header_values.extend((0, 0, 0))

    header = PACKAGE_HEADER_STRUCT.pack(
        int.from_bytes(PACKAGE_MAGIC, "little"), PACKAGE_VERSION, PACKAGE_HEADER_STRUCT.size,
        template_id, args.engine_class, 0, source_hash,
        fixed_string(display_name, 64), fixed_string(args.author, 64),
        offsets[ASSET_MODEL_HI][0], offsets[ASSET_MODEL_HI][1],
        offsets[ASSET_SHARED_VRM][0], offsets[ASSET_SHARED_VRM][1],
        offsets[ASSET_VOICE_XNF][0], offsets[ASSET_VOICE_XNF][1],
        len(voice_offsets), *voice_header_values,
    )

    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("wb") as out:
        out.write(header)
        if out.tell() < align(PACKAGE_HEADER_STRUCT.size, 16):
            out.write(b"\0" * (align(PACKAGE_HEADER_STRUCT.size, 16) - out.tell()))
        for asset, (offset, _) in zip(assets, offsets):
            if not asset:
                continue
            if out.tell() < offset:
                out.write(b"\0" * (offset - out.tell()))
            out.write(asset)
            aligned = align(out.tell(), 16)
            if aligned != out.tell():
                out.write(b"\0" * (aligned - out.tell()))

        for (_, voice_data), (_, offset, _) in zip(voice_files, voice_offsets):
            if out.tell() < offset:
                out.write(b"\0" * (offset - out.tell()))
            out.write(voice_data)
            aligned = align(out.tell(), 16)
            if aligned != out.tell():
                out.write(b"\0" * (aligned - out.tell()))

    print(f"Custom racer: {display_name}")
    print(f"Template character ID: {template_id}")
    print(f"High model: {len(model_hi)} bytes")
    if animation_report is not None:
        print("Template animations: " + ", ".join(animation_report["animations"]))
        print(f"Animation retarget vertices: {animation_report['template_vertices']} -> {animation_report['custom_vertices']}")
        print("Animation mapping distance: "
              f"median {animation_report['median_distance']:.1f}, "
              f"p90 {animation_report['p90_distance']:.1f}, "
              f"max {animation_report['max_distance']:.1f}")
        print(f"Animation clipped components: {animation_report['clipped_components']}")
    print(f"Shared VRM: {len(shared_vrm)} bytes")
    print(f"Voice XNF: {len(voice_xnf)} bytes")
    print(f"Voice XA files: {', '.join(f'S{n:02d}' for n, _ in voice_files) if voice_files else 'none'}")
    print(f"Wrote: {args.output}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f"error: {exc}", file=sys.stderr)
        raise SystemExit(1)
