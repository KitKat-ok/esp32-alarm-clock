#!/bin/bash

./build_icons.sh ./svg_icons/phosphor_icons/SVGs/duotone icons 18 16 &
./build_icons.sh ./svg_icons/phosphor_icons/SVGs/duotone icons 24 16 &
./build_icons.sh ./svg_icons/phosphor_icons/SVGs/duotone icons 32 16 &
./build_icons.sh ./svg_icons/phosphor_icons/SVGs/duotone icons 48 16 &

./build_icons.sh ./svg_icons/svg_weather weather 50 1 &
./build_icons.sh ./svg_icons/svg_weather weather 40 1 &
./build_icons.sh ./svg_icons/svg_weather weather 21 1 &

wait
echo "All builds done."