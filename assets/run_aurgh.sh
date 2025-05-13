#!/usr/bin/env bash
xdg_cache_home=$XDG_CACHE_HOME

rm $XDG_CACHE_HOME/aurgh/log.txt
aurgh -l $XDG_CACHE_HOME/aurgh/log.txt