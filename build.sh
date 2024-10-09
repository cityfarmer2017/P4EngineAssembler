#!/bin/bash

app_name=P4eAsm;
top_dir=$(pwd);

bin_dir="$top_dir/bin";
bin_file="$bin_dir/$app_name";

obj_dir="$top_dir/build/obj";

# if [ -d $bin_dir ]; then
#     if [ "$(ls -A $bin_dir)" ] || [ "$(ls -A $obj_dir)" ]; then
#         make clean;
#     fi
# fi

make dist-clean
make PRE_FLAG=-DNO_PRE_PROC TBL_FLAG=-DNO_TBL_PROC;

release_without_sub_dir="$bin_dir/release_without_sub";
release_without_sub_file="$release_without_sub_dir/$app_name";
if [ ! -d $release_without_sub_dir ]; then
    mkdir $release_without_sub_dir
elif [ -f $release_without_sub_file ]; then
    rm $release_without_sub_file
fi

mv $bin_file $release_without_sub_dir;

make clean
make TBL_FLAG=-DNO_TBL_PROC;

release_with_preproc_dir="$bin_dir/release_with_preproc";
release_with_preproc_file="$release_with_preproc_dir/$app_name";
if [ ! -d $release_with_preproc_dir ]; then
    mkdir $release_with_preproc_dir
elif [ -f $release_with_preproc_file ]; then
    rm $release_with_preproc_file
fi

mv $bin_file $release_with_preproc_dir;

make clean
make PRE_FLAG=-DNO_PRE_PROC;

release_with_tblproc_dir="$bin_dir/release_with_tblproc";
release_with_tblproc_file="$release_with_tblproc_dir/$app_name";
if [ ! -d $release_with_tblproc_dir ]; then
    mkdir $release_with_tblproc_dir
elif [ -f $release_with_tblproc_file ]; then
    rm $release_with_tblproc_file
fi

mv $bin_file $release_with_tblproc_dir;

make clean
make;

release_with_allsub_dir="$bin_dir/release_with_allsub";
release_with_allsub_file="$release_with_allsub_dir/$app_name";
if [ ! -d $release_with_allsub_dir ]; then
    mkdir $release_with_allsub_dir
elif [ -f $release_with_allsub_file ]; then
    rm $release_with_allsub_file
fi

mv $bin_file $release_with_allsub_dir;
