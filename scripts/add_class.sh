#!/bin/sh
# Simple script to add classes automatically



# Check for no arguments
if [ $# -eq 0 ]; then
    echo "!! [SCRIPT] NO CLASSES PROVIDED. NOTHING TO DO."
    exit 1
fi

src_dir=$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )
parent_dir=$(dirname $src_dir)

# Cycle through all arguments (classnames)
for class in $@
do
    cpp_path=$parent_dir/src/classes/$class.cpp
    header_path=$parent_dir/inc/$class.h


    if [[ -f $cpp_path && -f $header_path ]];
    then

        echo "== [SCRIPT] CLASS ('"$class"') EXISTS, NOT CREATING NEW CLASS FILES"
    else

        echo "== [SCRIPT] CLASS ('"$class"') DOES NOT EXIST, CREATING NEW CLASS FILES"

        search_h="###"
        search_cpp="####"
        replace_h=${class^^}
        replace_cpp=$class

        cp $src_dir/templates/temp.cpp $cpp_path
        cp $src_dir/templates/temp.h $header_path

        sed -i "s/$search_cpp/$replace_cpp/gi" $header_path
        sed -i "s/$search_h/$replace_h/gi" $header_path
        sed -i "s/$search_cpp/$replace_cpp/gi" $cpp_path

    fi 

done
echo "== [SCRIPT] DONE."
