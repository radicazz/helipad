"""Documentation tooling helpers."""

from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path
import shutil


def _repo_root() -> Path:
    return Path(__file__).resolve().parents[2]


def _run(command: list[str], *, cwd: Path) -> None:
    process = subprocess.run(command, cwd=cwd, check=False)
    if process.returncode != 0:
        joined = " ".join(command)
        raise SystemExit(f"command failed: {joined}")


def _prepare_doxygen_config(root: Path) -> Path:
    source = root / "Doxyfile"
    text = source.read_text(encoding="utf-8")

    output_dir = root / "build" / "docs" / "doxygen"
    output_dir.mkdir(parents=True, exist_ok=True)

    replacements = {
        "@CMAKE_SOURCE_DIR@": root.as_posix(),
        "@PROJECT_NAME@": "Helipad",
        "@PROJECT_VERSION@": "dev",
        "@PROJECT_DESCRIPTION@": "Helipad SDL3 engine",
    }

    for placeholder, value in replacements.items():
        text = text.replace(placeholder, value)

    text = re.sub(
        r"^OUTPUT_DIRECTORY\s*=.*$",
        f"OUTPUT_DIRECTORY       = {output_dir.as_posix()}",
        text,
        flags=re.MULTILINE,
    )

    generated = output_dir / "Doxyfile.generated"
    generated.write_text(text, encoding="utf-8")
    return generated


def generate_source_docs() -> None:
    root = _repo_root()
    doxyfile_src = root / "Doxyfile"

    if not doxyfile_src.exists():
        raise SystemExit(f"missing Doxygen config: {doxyfile_src}")

    doxyfile = _prepare_doxygen_config(root)

    print(f"[docs] running doxygen ({doxyfile})", file=sys.stderr)
    _run(["doxygen", str(doxyfile)], cwd=root)


def generate_user_docs() -> None:
    root = _repo_root()
    mkdocs_config = root / "mkdocs.yml"

    if not mkdocs_config.exists():
        raise SystemExit(f"missing MkDocs config: {mkdocs_config}")

    print("[docs] running mkdocs build", file=sys.stderr)
    _run(["mkdocs", "build", "--config-file", str(mkdocs_config)], cwd=root)

    # TODO: Query the user if they want to serve the docs locally.

    # Start a local MkDocs development server so the generated site can be
    # inspected immediately. Use Popen and wait so we can intercept Ctrl-C
    # and shut down the server cleanly without a traceback.
    serve_cmd = ["mkdocs", "serve", "--config-file", str(mkdocs_config)]
    print("[docs] starting mkdocs serve (press Ctrl-C to stop)", file=sys.stderr)
    try:
        proc = subprocess.Popen(serve_cmd, cwd=root)
        try:
            proc.wait()
        except KeyboardInterrupt:
            # User requested shutdown; terminate mkdocs server gracefully.
            print("[docs] received Ctrl-C, stopping mkdocs server...", file=sys.stderr)
            proc.terminate()
            try:
                proc.wait(timeout=5)
            except Exception:
                proc.kill()
    except Exception as exc:  # pragma: no cover - platform/process errors
        raise SystemExit(f"failed to start mkdocs serve: {exc}")


def main() -> None:
    generate_source_docs()
    generate_user_docs()
