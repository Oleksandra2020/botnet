#!/bin/sh

src_dir=$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )
parent_dir=$(dirname "$src_dir")


if [ ! -d $parent_dir/build/ ];
then
    echo "== [SCRIPT] CREATING BUILD DIRECTORY"
    mkdir $parent_dir/build
fi

echo "== [SCRIPT] STARTING BUILD"
cmake $parent_dir -B $parent_dir/build/ -DCMAKE_BUILD_TYPE=Release
cd $parent_dir/build/ && make
