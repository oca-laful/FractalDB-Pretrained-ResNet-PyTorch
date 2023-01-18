@echo off
set numof_category=3
set fillrate=0.2
set weight=0.4
set imagesize=362
set numof_point=100000
set numof_ite=200000
set howto_draw='patch_gray'
set arch=resnet50

rem Parameter search
python param_search/ifs_search.py --rate=%fillrate% --category=%numof_category% --numof_point=%numof_point% --save_dir=data
