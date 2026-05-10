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

RESVG="./tools/resvg"
JOBS=$(nproc)

mkdir -p generated_icons png

PNG_PATH="./png/${NAME}_${SIZE}x${SIZE}"
HEADER_PATH="./generated_icons/${NAME}_${SIZE}x${SIZE}"
MAIN_HEADER="./generated_icons/${NAME}_${SIZE}x${SIZE}.h"

rm -rf "$PNG_PATH" "$HEADER_PATH" "$MAIN_HEADER"
mkdir -p "$PNG_PATH" "$HEADER_PATH"

sanitize() {
  echo "$1" | tr -cs '[:alnum:]' '_' | sed 's/^_//; s/_$//'
}

export SIZE PNG_PATH RESVG
export -f sanitize

echo "Converting To PNGS: $INPUT_DIR"

find "$INPUT_DIR" -maxdepth 1 -type f | xargs -P "$JOBS" -I {} bash -c '
f="{}"
base=$(basename "$f")
name=$(sanitize "${base%.*}")
out="$PNG_PATH/${name}.png"

if [[ "$f" == *.svg ]]; then
  "$RESVG" "$f" "$out" \
    --width "$SIZE" \
    --height "$SIZE" \
    --shape-rendering crispEdges \
    --text-rendering optimizeSpeed
elif [[ "$f" == *.png ]]; then
  cp "$f" "$out"
fi
'

export GRAY_MODE SIZE PNG_PATH

echo "Cleaning the PNGS up..."

find "$PNG_PATH" -name "*.png" | xargs -P "$JOBS" -I {} bash -c '
f="{}"

convert "$f" \
  -background white \
  -alpha remove \
  -alpha off \
  -filter point \
  -resize "${SIZE}x${SIZE}!" \
  "$f"

if [ "$GRAY_MODE" = "16" ]; then
  convert "$f" -colorspace Gray -posterize 16 "$f"
else
  convert "$f" -colorspace Gray -threshold 50% "$f"
fi
'

export HEADER_PATH NAME SIZE GRAY_MODE INVERT FLIP

echo "Generating headers..."

find "$PNG_PATH" -name "*.png" | xargs -P "$JOBS" -I {} bash -c '
f="{}"
base=$(basename "$f" .png)
name=$(echo "$base" | tr -cs "[:alnum:]" "_" | sed -E "s/_+/_/g; s/^_+|_+$//g")
full_name="${name}_${SIZE}x${SIZE}"
out="${HEADER_PATH}/${full_name}.h"

python3 png_to_header.py \
  -i "$f" -o "$out" \
  --grayscale $([[ "$GRAY_MODE" == "16" ]] && echo 4 || echo 1) \
  --invert "$INVERT" \
  --flip "$FLIP"
'

echo "Generating main header..."

echo "#pragma once" > "$MAIN_HEADER"

for f in "$HEADER_PATH"/*.h; do
  echo "#include \"${NAME}_${SIZE}x${SIZE}/$(basename "$f")\"" >> "$MAIN_HEADER"
done

echo "Done."