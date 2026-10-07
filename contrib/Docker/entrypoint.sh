#!/bin/sh
# Seeds configs/ from the .conf.dist files on first start, then runs the
# given command (worldserver, bnetserver or one of the extraction tools).
set -e

mkdir -p configs data logs

for dist in /opt/arguscore/configs/*.conf.dist; do
    conf="configs/$(basename "${dist%.dist}")"
    [ -f "$conf" ] || cp "$dist" "$conf"
done

exec "$@"
