#!/bin/sh
set -e # fail the script if a command fails

PATH=$PATH:/usr/local/bin # This should make node and npm accessible on OSX.
firmware_image=`pwd`/$1
usb_dir=../../../lib/agent/usb
usb_binding=$usb_dir/node_modules/usb/build/Release/usb_bindings.node
blhost="../../../lib/bootloader/bin/Tools/blhost/linux/amd64/blhost --usb 0x1d50,0x6120"

set -x # echo on

if [ ! -f $usb_binding ]; then
   cd $usb_dir
   npm install
fi

$usb_dir/jump-to-bootloader.js
$blhost flash-security-disable 0403020108070605
$blhost flash-erase-region 0xc000 475136
$blhost flash-image $firmware_image
$blhost reset
