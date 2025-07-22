#!/bin/bash

set -e

MODULE_NAME="ehlo"
KERNEL_VERSIONS=("6.1.130":"6.1.130" "vcpkg":$(uname -r))

for version in "${KERNEL_VERSIONS[@]}"; do
    echo "=== Building for kernel $version ==="
    name="${version%%:*}"
    version="${version##*:}"
    toolbox run -c "$name" make KERNEL_VERSION="$version"
    formatted_string="APP-${name}.${version}"
    echo "$formatted_string"
done

echo "=== All builds completed! ==="
ls -lh build/
