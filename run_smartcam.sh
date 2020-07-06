#!/bin/bash
export DISPLAY=:0
cd /home/smartcam/Works/source/carsmartcam/build/bin
sh setup_vcan.sh
./CarSmartCam
sudo update-rc.d /etc/init.d/run_smartcam.sh defaults