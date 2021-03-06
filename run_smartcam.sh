#!/bin/bash
export DISPLAY=:0
cd build/bin
sh setup_vcan.sh
./CarSmartCam
while [ $? -e 42 ]; do
    ./CarSmartCam
done
# sudo update-rc.d /etc/init.d/run_smartcam.sh defaults