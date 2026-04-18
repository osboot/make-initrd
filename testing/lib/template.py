# SPDX-License-Identifier: GPL-3.0-or-later

from __future__ import annotations

import re
from pathlib import Path


TEMPLATE_DIR = Path(__file__).resolve().parent.parent / "templates"


def render_template(name: str, values: dict[str, str]) -> str:
    result = (TEMPLATE_DIR / name).read_text()
    for key, value in values.items():
        result = result.replace(f"@{key}@", value)

    missing = sorted(set(re.findall(r"@[A-Z0-9_]+@", result)))
    if missing:
        raise KeyError(f"missing template values for {name}: {', '.join(missing)}")
    return result
