#!/bin/bash

############################################################################
# Copyright (c) 2011-2012 Saint-Petersburg Academic University
# All Rights Reserved
# See file LICENSE for details.
############################################################################

set -e
pushd ../../../

target=spades_output/ECOLI_IS220_QUAKE_100K_SAVES
if [ -d $target ]; then
	rm -rf $target #etalon=spades_output/ECOLI_IS220_QUAKE_100K_SAVES/etalon
fi	

etalon=/smallnas/teamcity/etalon_output/ECOLI_IS220_QUAKE_100K_SAVES/etalon_saves

#if [ ! -d $etalon ]; then
#    echo "Error: no etalon saves at $etalon"
#    exit 9
#fi

#./spades_compile.sh
./spades.py --config-file src/test/teamcity/spades_config_mc_is220_100k_saves.info

#pushd spades_output/ECOLI_IS220_QUAKE_100K_SAVES
#diffs=0
#    for f in saves/*
#    do
#        set +e
#        diff $f $etalon/link_K55/$f >> diff_with_etalon.txt
#        errlvl=$?
#        if [ $errlvl -ne 0 ]; then
#            if [ $errlvl -eq 1 ]; then
#                echo "^^^^^^^ it was $f" >> diff_with_etalon.txt
#                echo "BAD: difference found in $f"
#            else
#                echo "BAD: unable to compare with $f"
#            fi
#            (( diffs += 1 ))
#        fi
#        set -e
#    done
#popd
popd

#echo $diffs differences with etalon saves found
./detect_diffs.sh ../../../$target $etalon
exit $?
