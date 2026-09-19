#!/usr/bin/env bash

set -u

required_commands=(
  git
  make
  gcc
  python3
  arm-none-eabi-as
  arm-none-eabi-gcc
  arm-none-eabi-ld
  arm-none-eabi-objcopy
)

missing=0

printf 'Finite Fusion development environment check\n\n'

if grep -qi microsoft /proc/version 2>/dev/null; then
  printf '[ok] Running inside WSL\n'
else
  printf '[note] WSL was not detected. This is expected on native Linux or macOS.\n'
fi

case "$PWD" in
  /mnt/*)
    printf '[warning] Repository is under /mnt/. WSL2 builds are much faster under ~/projects.\n'
    ;;
  *)
    printf '[ok] Repository is on the Linux filesystem\n'
    ;;
esac

for command_name in "${required_commands[@]}"; do
  if command -v "$command_name" >/dev/null 2>&1; then
    printf '[ok] %s\n' "$command_name"
  else
    printf '[missing] %s\n' "$command_name"
    missing=1
  fi
done

if [ ! -f Makefile ]; then
  printf '[error] Makefile not found. Run this script from the repository root.\n'
  missing=1
else
  printf '[ok] Makefile found\n'
fi

if [ "$missing" -ne 0 ]; then
  printf '\nSetup is incomplete. Follow GETTING_STARTED_WINDOWS.md, then run this check again.\n'
  exit 1
fi

printf '\nEnvironment looks ready. Build with: make -j"$(nproc)"\n'
