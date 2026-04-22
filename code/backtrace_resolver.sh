#!/bin/bash

XTENSA_ADDR2LINE="$HOME/.platformio/packages/toolchain-riscv32-esp/bin/riscv32-esp-elf-addr2line"
ELF_FILE=".pio/build/esp32c6/firmware.elf"

for BACKTRACE in "$@"; do
    echo "Processing backtrace: $BACKTRACE"

    while IFS= read -r LINE; do
        ADDR="$(echo "$LINE" | awk '{print $1}')"
        OFFSET="$(echo "$LINE" | awk '{print $2}')"
        if [ "$OFFSET" = "0x0" ]; then
            continue
        fi
        echo -n "$ADDR: "
        "$XTENSA_ADDR2LINE" -e "$ELF_FILE" "$ADDR"
    done < <(echo "$BACKTRACE")
    echo
done
