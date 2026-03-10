#!/usr/bin/env python3
"""
Enforce Scrite QML id convention.

Rules:
1) Top-level object id must be `root`
2) Any other id in the file must start with `_`
"""

from __future__ import annotations

import argparse
import bisect
import json
import pathlib
import re
import sys
from dataclasses import dataclass
from typing import Dict, List, Sequence, Tuple


ID_VALUE_PATTERN = re.compile(r"(?m)^[ \t]*id\s*:\s*([A-Za-z_][A-Za-z0-9_]*)")
IDENTIFIER_PATTERN = re.compile(r"\b[A-Za-z_][A-Za-z0-9_]*\b")


@dataclass
class IdDecl:
    name: str
    start: int
    end: int
    depth: int
    line: int


@dataclass
class Violation:
    line: int
    message: str


@dataclass
class StaleReference:
    line: int
    old_name: str
    new_name: str
    start: int
    end: int
    message: str
    kind: str  # "stale" or "ambiguous"


def mask_comments_and_strings(text: str) -> str:
    chars = list(text)
    out = chars[:]
    state = "normal"
    quote = ""
    i = 0
    while i < len(chars):
        ch = chars[i]
        nxt = chars[i + 1] if i + 1 < len(chars) else ""

        if state == "normal":
            if ch == "/" and nxt == "/":
                out[i] = " "
                out[i + 1] = " "
                i += 2
                state = "line_comment"
                continue
            if ch == "/" and nxt == "*":
                out[i] = " "
                out[i + 1] = " "
                i += 2
                state = "block_comment"
                continue
            if ch in ('"', "'", "`"):
                quote = ch
                out[i] = " "
                i += 1
                state = "string"
                continue
            i += 1
            continue

        if state == "line_comment":
            if ch == "\n":
                state = "normal"
            else:
                out[i] = " "
            i += 1
            continue

        if state == "block_comment":
            if ch == "*" and nxt == "/":
                out[i] = " "
                out[i + 1] = " "
                i += 2
                state = "normal"
            else:
                if ch != "\n":
                    out[i] = " "
                i += 1
            continue

        if state == "string":
            if ch == "\\":
                out[i] = " "
                if i + 1 < len(chars):
                    if chars[i + 1] != "\n":
                        out[i + 1] = " "
                i += 2
                continue
            if ch == quote:
                out[i] = " "
                i += 1
                state = "normal"
                continue
            if ch != "\n":
                out[i] = " "
            i += 1
            continue

    return "".join(out)


def build_depth_map(masked_text: str) -> List[int]:
    depth_before = [0] * (len(masked_text) + 1)
    depth = 0
    for i, ch in enumerate(masked_text):
        depth_before[i] = depth
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth = max(0, depth - 1)
    depth_before[len(masked_text)] = depth
    return depth_before


def compute_line_number(line_starts: Sequence[int], pos: int) -> int:
    return bisect.bisect_right(line_starts, pos)


def line_starts_for(text: str) -> List[int]:
    starts = [0]
    for idx, ch in enumerate(text):
        if ch == "\n":
            starts.append(idx + 1)
    return starts


def parse_ids(text: str) -> List[IdDecl]:
    masked = mask_comments_and_strings(text)
    depth_map = build_depth_map(masked)
    starts = line_starts_for(text)

    decls: List[IdDecl] = []
    for match in ID_VALUE_PATTERN.finditer(masked):
        start, end = match.span(1)
        name = text[start:end]
        decls.append(
            IdDecl(
                name=name,
                start=start,
                end=end,
                depth=depth_map[match.start()],
                line=compute_line_number(starts, start),
            )
        )
    return decls


def find_top_level_decl(decls: Sequence[IdDecl]) -> IdDecl | None:
    if not decls:
        return None
    for decl in decls:
        if decl.depth == 1:
            return decl
    return None


def is_property_key(masked_text: str, token_start: int, token_end: int) -> bool:
    i = token_end
    while i < len(masked_text) and masked_text[i].isspace():
        i += 1
    if not (i < len(masked_text) and masked_text[i] == ":"):
        return False

    j = token_start - 1
    while j >= 0 and masked_text[j].isspace():
        j -= 1
    if j < 0:
        return True
    return masked_text[j] in "{\n;,("


def replace_identifiers(text: str, rename_map: Dict[str, str]) -> str:
    if not rename_map:
        return text
    masked = mask_comments_and_strings(text)
    replacements: List[Tuple[int, int, str]] = []
    for match in IDENTIFIER_PATTERN.finditer(masked):
        old = text[match.start() : match.end()]
        if old not in rename_map:
            continue
        if is_property_key(masked, match.start(), match.end()):
            continue
        replacements.append((match.start(), match.end(), rename_map[old]))

    if not replacements:
        return text

    out: List[str] = []
    last = 0
    for start, end, rep in replacements:
        out.append(text[last:start])
        out.append(rep)
        last = end
    out.append(text[last:])
    return "".join(out)


def apply_replacements(text: str, replacements: Sequence[Tuple[int, int, str]]) -> str:
    if not replacements:
        return text
    out = text
    for start, end, new_value in sorted(replacements, key=lambda item: item[0], reverse=True):
        out = out[:start] + new_value + out[end:]
    return out


def has_local_declaration(masked_text: str, name: str) -> bool:
    escaped = re.escape(name)
    patterns = [
        rf"\b(?:var|let|const)\s+{escaped}\b",
        rf"\bfunction\s+{escaped}\b",
        rf"\bfunction\b[^\n{{]*\([^)]*\b{escaped}\b",
        rf"\bfor\s*\(\s*(?:var|let|const)\s+{escaped}\b",
    ]
    return any(re.search(pattern, masked_text) for pattern in patterns)


def infer_stale_reference_map(decls: Sequence[IdDecl]) -> Dict[str, str]:
    declared_names = {decl.name for decl in decls}
    stale_map: Dict[str, str] = {}
    for decl in decls:
        if decl.name.startswith("_") and len(decl.name) > 1:
            old_name = decl.name[1:]
            if old_name not in declared_names:
                stale_map[old_name] = decl.name
    return stale_map


def collect_stale_references(text: str) -> Tuple[List[StaleReference], List[StaleReference]]:
    decls = parse_ids(text)
    stale_map = infer_stale_reference_map(decls)
    if not stale_map:
        return [], []

    masked = mask_comments_and_strings(text)
    starts = line_starts_for(text)
    ambiguous_names = {name for name in stale_map if has_local_declaration(masked, name)}

    stale: List[StaleReference] = []
    ambiguous: List[StaleReference] = []

    for match in IDENTIFIER_PATTERN.finditer(masked):
        old_name = text[match.start() : match.end()]
        if old_name not in stale_map:
            continue
        if is_property_key(masked, match.start(), match.end()):
            continue
        line = compute_line_number(starts, match.start())
        ref = StaleReference(
            line=line,
            old_name=old_name,
            new_name=stale_map[old_name],
            start=match.start(),
            end=match.end(),
            message=f"Reference `{old_name}` should be `{stale_map[old_name]}`.",
            kind="stale",
        )
        if old_name in ambiguous_names:
            ref.kind = "ambiguous"
            ref.message = f"Ambiguous `{old_name}` reference; expected `{stale_map[old_name]}` or local symbol."
            ambiguous.append(ref)
        else:
            stale.append(ref)
    return stale, ambiguous


def apply_stale_reference_fixes(text: str) -> Tuple[str, int]:
    stale_refs, _ = collect_stale_references(text)
    replacements = [(ref.start, ref.end, ref.new_name) for ref in stale_refs]
    fixed = apply_replacements(text, replacements)
    return fixed, len(replacements)


def unique_name(base: str, used: set[str]) -> str:
    if base not in used:
        return base
    idx = 2
    while f"{base}_{idx}" in used:
        idx += 1
    return f"{base}_{idx}"


def plan_decl_renames(decls: Sequence[IdDecl]) -> List[Tuple[IdDecl, str]]:
    top = find_top_level_decl(decls)
    if top is None:
        return []

    desired: Dict[int, str] = {}
    for idx, decl in enumerate(decls):
        if decl is top:
            desired[idx] = "root"
        elif decl.name.startswith("_"):
            desired[idx] = decl.name
        else:
            desired[idx] = "_" + decl.name

    used: set[str] = set()
    final: Dict[int, str] = {}
    for idx, decl in enumerate(decls):
        target = desired[idx]
        if target in used:
            if decl is top:
                target = unique_name("root", used)
            else:
                target = unique_name(target, used)
        used.add(target)
        final[idx] = target

    renames: List[Tuple[IdDecl, str]] = []
    for idx, decl in enumerate(decls):
        if decl.name != final[idx]:
            renames.append((decl, final[idx]))
    return renames


def apply_decl_renames(text: str, renames: Sequence[Tuple[IdDecl, str]]) -> str:
    if not renames:
        return text
    out = text
    for decl, new_name in sorted(renames, key=lambda item: item[0].start, reverse=True):
        out = out[:decl.start] + new_name + out[decl.end:]
    return out


def build_unambiguous_reference_map(decls: Sequence[IdDecl], renames: Sequence[Tuple[IdDecl, str]]) -> Dict[str, str]:
    if not renames:
        return {}

    count_by_name: Dict[str, int] = {}
    for decl in decls:
        count_by_name[decl.name] = count_by_name.get(decl.name, 0) + 1

    ref_map: Dict[str, str] = {}
    for decl, new_name in renames:
        if count_by_name.get(decl.name, 0) == 1:
            ref_map[decl.name] = new_name
    return ref_map


def find_first_object_open_brace(text: str) -> int:
    masked = mask_comments_and_strings(text)
    return masked.find("{")


def infer_child_indent(text: str, brace_pos: int) -> str:
    line_start = text.rfind("\n", 0, brace_pos)
    line_start = 0 if line_start == -1 else line_start + 1
    line = text[line_start:brace_pos]
    parent_indent_match = re.match(r"[ \t]*", line)
    parent_indent = parent_indent_match.group(0) if parent_indent_match else ""
    return parent_indent + "    "


def insert_root_id_if_missing(text: str) -> str:
    brace_pos = find_first_object_open_brace(text)
    if brace_pos == -1:
        return text
    child_indent = infer_child_indent(text, brace_pos)
    insertion = "\n" + child_indent + "id: root"
    return text[: brace_pos + 1] + insertion + text[brace_pos + 1 :]


def collect_violations(text: str) -> List[Violation]:
    decls = parse_ids(text)
    violations: List[Violation] = []
    top = find_top_level_decl(decls)
    if top is None:
        violations.append(Violation(line=1, message="Missing top-level id declaration (expected `id: root`)."))
        return violations

    if top.name != "root":
        violations.append(Violation(line=top.line, message=f"Top-level id is `{top.name}`; expected `root`."))

    for decl in decls:
        if decl is top:
            continue
        if not decl.name.startswith("_"):
            violations.append(Violation(line=decl.line, message=f"Nested id `{decl.name}` must start with `_`."))
    return violations


def apply_fixes(text: str) -> str:
    decls = parse_ids(text)
    top = find_top_level_decl(decls)
    if top is None:
        text = insert_root_id_if_missing(text)
        decls = parse_ids(text)
    renames = plan_decl_renames(decls)
    if not renames:
        return text
    with_decl_updates = apply_decl_renames(text, renames)
    reference_map = build_unambiguous_reference_map(decls, renames)
    if not reference_map:
        return with_decl_updates
    return replace_identifiers(with_decl_updates, reference_map)


def parse_qml_paths_from_cmake(cmake_file: pathlib.Path) -> List[pathlib.Path]:
    base_dir = cmake_file.parent.parent
    content = cmake_file.read_text(encoding="utf-8")
    rel_paths = re.findall(r'"([^"]+\.qml)"', content)
    files = [base_dir / rel for rel in rel_paths]
    unique_files = sorted(set(path.resolve() for path in files))
    return unique_files


def discover_qml_files(paths: Sequence[pathlib.Path], recursive: bool) -> List[pathlib.Path]:
    found: List[pathlib.Path] = []
    for path in paths:
        if path.is_file() and path.suffix == ".qml":
            found.append(path.resolve())
            continue
        if path.is_dir():
            iterator = path.rglob("*.qml") if recursive else path.glob("*.qml")
            for item in iterator:
                if item.is_file():
                    found.append(item.resolve())
    return sorted(set(found))


def parse_args(argv: Sequence[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Enforce QML id naming convention.")
    parser.add_argument(
        "--path",
        action="append",
        default=[],
        help="QML file or directory to scan. Can be repeated.",
    )
    parser.add_argument(
        "--cmake-list",
        help="Path to DesktopQmlAndAssetFiles.cmake to derive QML files from.",
    )
    parser.add_argument(
        "--recursive",
        action="store_true",
        help="When --path points to a directory, scan recursively.",
    )
    parser.add_argument("--fix", action="store_true", help="Rewrite files to satisfy convention when possible.")
    parser.add_argument(
        "--check-stale-refs",
        action="store_true",
        help="Check for stale references to pre-rename ids (for example `foo` when only `_foo` exists).",
    )
    parser.add_argument(
        "--fix-stale-refs",
        action="store_true",
        help="Rewrite stale id references conservatively where unambiguous.",
    )
    parser.add_argument(
        "--stale-report-json",
        help="Optional path to write stale reference findings as JSON.",
    )
    parser.add_argument(
        "--fail-on-ambiguous",
        action="store_true",
        help="Return non-zero when ambiguous stale-reference candidates are found.",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Show what would fail without writing files.",
    )
    return parser.parse_args(argv)


def main(argv: Sequence[str]) -> int:
    args = parse_args(argv)

    qml_files: List[pathlib.Path] = []
    if args.cmake_list:
        qml_files.extend(parse_qml_paths_from_cmake(pathlib.Path(args.cmake_list)))
    if args.path:
        qml_files.extend(discover_qml_files([pathlib.Path(p) for p in args.path], args.recursive))
    qml_files = sorted(set(qml_files))

    if not qml_files:
        print("No QML files found.")
        return 1

    files_with_violations = 0
    total_violations = 0
    changed_files = 0
    stale_changed_files = 0
    total_stale = 0
    total_ambiguous = 0
    files_with_stale = 0
    stale_report_rows: List[Dict[str, object]] = []

    for file_path in qml_files:
        original_text = file_path.read_text(encoding="utf-8")
        text_for_validation = original_text

        if args.fix:
            fixed_text = apply_fixes(original_text)
            text_for_validation = fixed_text

        if args.fix_stale_refs:
            stale_fixed_text, replaced_count = apply_stale_reference_fixes(text_for_validation)
            if replaced_count > 0:
                stale_changed_files += 1
            text_for_validation = stale_fixed_text

        if text_for_validation != original_text:
            changed_files += 1
            if not args.dry_run:
                file_path.write_text(text_for_validation, encoding="utf-8")

        violations = collect_violations(text_for_validation)
        if violations:
            files_with_violations += 1
            total_violations += len(violations)
            for violation in violations:
                print(f"{file_path}:{violation.line}: {violation.message}")

        if args.check_stale_refs or args.fix_stale_refs:
            stale_refs, ambiguous_refs = collect_stale_references(text_for_validation)
            if stale_refs or ambiguous_refs:
                files_with_stale += 1
            total_stale += len(stale_refs)
            total_ambiguous += len(ambiguous_refs)

            for ref in stale_refs:
                print(f"{file_path}:{ref.line}: {ref.message}")
                stale_report_rows.append(
                    {
                        "file": str(file_path),
                        "line": ref.line,
                        "old": ref.old_name,
                        "new": ref.new_name,
                        "kind": ref.kind,
                        "message": ref.message,
                    }
                )
            for ref in ambiguous_refs:
                print(f"{file_path}:{ref.line}: {ref.message}")
                stale_report_rows.append(
                    {
                        "file": str(file_path),
                        "line": ref.line,
                        "old": ref.old_name,
                        "new": ref.new_name,
                        "kind": ref.kind,
                        "message": ref.message,
                    }
                )

    if args.fix or args.fix_stale_refs:
        mode = "Dry-run fix" if args.dry_run else "Fix"
        print(f"{mode} touched {changed_files} file(s).")
    if args.fix_stale_refs:
        mode = "Dry-run stale-ref fix" if args.dry_run else "Stale-ref fix"
        print(f"{mode} touched {stale_changed_files} file(s).")

    print(f"Checked {len(qml_files)} file(s).")
    if args.stale_report_json:
        report_path = pathlib.Path(args.stale_report_json)
        report_path.parent.mkdir(parents=True, exist_ok=True)
        report_path.write_text(json.dumps(stale_report_rows, indent=2), encoding="utf-8")
        print(f"Wrote stale reference report to {report_path}.")

    has_naming_issues = total_violations > 0
    has_stale_issues = total_stale > 0 or (args.fail_on_ambiguous and total_ambiguous > 0)

    if not has_naming_issues and not has_stale_issues:
        if (args.check_stale_refs or args.fix_stale_refs) and total_ambiguous > 0:
            print(
                f"No stale references found; {total_ambiguous} ambiguous candidate(s) remain for manual review."
            )
            return 0
        print("No convention violations found.")
        return 0

    if has_naming_issues:
        print(f"Found {total_violations} naming violation(s) in {files_with_violations} file(s).")
    if args.check_stale_refs or args.fix_stale_refs:
        print(
            f"Found {total_stale} stale reference(s) and {total_ambiguous} ambiguous reference(s) in {files_with_stale} file(s)."
        )
    return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
