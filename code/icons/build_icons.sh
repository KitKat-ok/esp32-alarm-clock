#!/bin/bash

set -e
shopt -s nullglob

if [ $# -lt 4 ]; then
  echo "Usage: $0 <input_dir> <output_name> <size> <gray_mode(1|16)> [invert] [flip]"
  exit 1
fi

INPUT_DIR="$1"
NAME="$2"
SIZE="$3"
GRAY_MODE="${4:-1}"

INVERT="${5:-0}"
FLIP="${6:-0}"

INVERT=$((INVERT))
FLIP=$((FLIP))

mkdir -p icons png

PNG_PATH="./png/${NAME}_${SIZE}x${SIZE}"
HEADER_PATH="./icons/${NAME}_${SIZE}x${SIZE}"
MAIN_HEADER="./icons/${NAME}_${SIZE}x${SIZE}.h"

echo "Cleaning..."
rm -rf "$PNG_PATH" "$HEADER_PATH" "$MAIN_HEADER"
mkdir -p "$PNG_PATH" "$HEADER_PATH"

sanitize() {
  echo "$1" | tr -cs '[:alnum:]' '_' | sed 's/^_//; s/_$//'
}

echo "Processing: $INPUT_DIR"

for f in "$INPUT_DIR"/*; do
  [ -e "$f" ] || continue

  base=$(basename "$f")
  name=$(sanitize "${base%.*}")
  out="$PNG_PATH/${name}.png"

  if [[ "$f" == *.svg ]]; then
    echo "SVG -> PNG: $f"
    rsvg-convert -w "$SIZE" -h "$SIZE" --background-color=white "$f" -o "$out"

  elif [[ "$f" == *.png ]]; then
    echo "PNG -> PNG: $f"
    convert "$f" -resize "${SIZE}x${SIZE}" "$out"

  else
    continue
  fi

  # 🔴 CRITICAL: remove alpha and force white background
  convert "$out" -background white -alpha remove -alpha off "$out"

  # grayscale conversion WITHOUT noise
  if [ "$GRAY_MODE" = "16" ]; then
    convert "$out" -alpha remove -alpha off \
  -colorspace Gray \
  -posterize 16 \
  -contrast-stretch 0 \
  "$out"
  else
    convert "$out" -colorspace Gray -threshold 50% -dither None "$out"
  fi
done

echo "Generating headers..."

for f in "$PNG_PATH"/*.png; do
  [ -e "$f" ] || continue

  base=$(basename "$f" .png)
  name=$(sanitize "$base")

  if [[ "$name" != *"${SIZE}x${SIZE}"* ]]; then
    name="${name}_${SIZE}x${SIZE}"
  fi

  out="${HEADER_PATH}/${name}.h"

  if [ "$GRAY_MODE" = "16" ]; then
    python3 png_to_header.py \
      -i "$f" -o "$out" \
      --grayscale 4 \
      --invert "$INVERT" \
      --flip "$FLIP"
  else
    python3 png_to_header.py \
      -i "$f" -o "$out" \
      --grayscale 1 \
      --invert "$INVERT" \
      --flip "$FLIP"
  fi
done

echo "Generating main include header..."

echo "#ifndef __${NAME^^}_${SIZE}x${SIZE}_H__" > "$MAIN_HEADER"
echo "#define __${NAME^^}_${SIZE}x${SIZE}_H__" >> "$MAIN_HEADER"

for f in "$HEADER_PATH"/*.h; do
  [ -e "$f" ] || continue
  echo "#include \"${NAME}_${SIZE}x${SIZE}/$(basename "$f")\"" >> "$MAIN_HEADER"
done

echo "#endif" >> "$MAIN_HEADER"

echo "Done."