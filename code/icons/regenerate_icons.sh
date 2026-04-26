#!/bin/bash

./build_icons.sh ./svg icons 18 1 &
./build_icons.sh ./svg icons 24 1 &
./build_icons.sh ./svg icons 32 1 &
./build_icons.sh ./svg icons 48 1 &

./build_icons.sh ./svg_weather weather 50 1 &
./build_icons.sh ./svg_weather weather 40 1 &
./build_icons.sh ./svg_weather weather 21 1 &

wait
echo "All builds done."