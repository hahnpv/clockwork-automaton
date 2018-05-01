The .h264 files are the native output from the Raspberry Pi. They can be converted into MP4 files by using ffmpeg - check out the /analysis/ folder for an example of how this is done.

The LIDAR spins at approximately 5 revolutions per second and should report 5 scans per second, unfortunately, this is not the case. I do not have an official diagnosis yet but there are two culprits
1) The LIDAR was not spinning at full speed, and only putting out intermittant results. This is probable, I was supplying 3V3 from the Pi and not checking RPM / using a PID voltage control
2) My script sucks
