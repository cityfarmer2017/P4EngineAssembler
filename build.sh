#!/bin/bash

app_name=P4eAsm;
top_dir=$(pwd);

bin_dir="$top_dir/bin";
bin_file="$bin_dir/$app_name";

obj_dir="$top_dir/build/obj";

if [ "$(ls -A $bin_dir)" ] || [ "$(ls -A $obj_dir)" ]; then
    make clean;
fi

make;

release_with_sub_dir="$bin_dir/release_with_sub";
release_with_sub_file="$release_with_sub_dir/$app_name";
if [ ! -d $release_with_sub_dir ]; then
    mkdir $release_with_sub_dir
elif [ -f $release_with_sub_file ]; then
    rm $release_with_sub_file
fi

mv $bin_file $release_with_sub_dir;

make clean;
make SUB_FLAG=-DWITHOUT_SUB_MODULES;

release_without_sub_dir="$bin_dir/release_without_sub";
release_without_sub_file="$release_without_sub_dir/$app_name";
if [ ! -d $release_without_sub_dir ]; then
    mkdir $release_without_sub_dir
elif [ -f $release_without_sub_file ]; then
    rm $release_without_sub_file
fi

mv $bin_file $release_without_sub_dir;