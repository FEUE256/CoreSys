#!/usr/bin/env python3
import yaml
from pathlib import Path

CONFIG_PATH = Path("../../.config/cfg.yaml")
OUTPUT_CFG = Path("../../.config/config.cfg")


def is_list(v):
    return isinstance(v, list)


def join_flags(v):
    return " ".join(v)


def to_make_value(v):
    if is_list(v):
        return join_flags(v)
    return str(v)


def load_yaml(path: Path):
    with path.open("r", encoding="utf-8") as f:
        return yaml.safe_load(f)


def generate_make(cfg: dict) -> str:
    lines = []

    for k, v in cfg.items():
        lines.append(f"{k} := {to_make_value(v)}")

    return "\n".join(lines)


def main():
    cfg = load_yaml(CONFIG_PATH)
    out = generate_make(cfg)

    OUTPUT_CFG.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT_CFG.write_text(out + "\n", encoding="utf-8")

    print(f"Wrote: {OUTPUT_CFG}")


if __name__ == "__main__":
    main()
    