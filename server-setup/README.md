## Setting up box as WiFi router

Presumes Ubuntu 18.04

1) Dependency: `sudo apt install hostapd`
1) In create_ap issue `sudo make install`
1) This command will start a temporary network, where enp4s0 is the WAN interface and wlx9cefd5fc7572 is the wifi interface. Add `--mkconfig create_ap.conf` to create a config file, though it should already exist in this directory. `sudo create_ap -m nat -c 6 -w 2 wlx9cefd5fc7572 enp4s0 AsOne purplemotion`
1) Install the configuration: `sudo cp create_ap.conf /etc/`
1) Fire up create_ap daemon: `sudo systemctl enable create_ap && sudo systemctl start create_ap`
1) Set hostname, so the embedded devices can find you: `hostnamectl set-hostname asone-console`

This should leave you in the state where the wifi nat is running and will run automatically when rebooted.

If wifi has been turned off somehow, `rfkill unblock wifi` may fix it.

If there are unknown errors, `systemctl status create_ap.service` and `systemctl status hostapd.service` may be of use.

## Configuring MQTT

Unfortunately, Ubuntu 18.04 ships with an older version of Mosquitto that does not support the necessary configuration options. Do `sudo apt-add-repository ppa:mosquitto-dev/mosquitto-ppa` to add newer versions to apt, then `sudo apt install mosquitto` which will install a current one.

For Mac, some mosquitto instructions may be found in a text file in this directory. That text file was copied from https://gist.github.com/KazChe/6bcafbaf29e10a7f309d3ca2e2a0f706 on 20190620. Just copy mosquitto.conf to /etc first and you should be golden.

Using sudo:
1) Copy over the mosquitto.conf file to /etc/mosquitto/conf.d/ to enable one tcp packet per message.
1) Copy the mosquitto.service file to /usr/lib/systemd/system/ -- not sure what the issue is, but start-on-boot does not work with whatever comes out of the box. At least with 1.6.2 which is the current as of this edit.
1) Unload the service in case it is running, tell it to run at boot, and fire it up: `sudo systemctl stop mosquitto`, `sudo systemctl enable mosquitto`, `sudo systemctl start mosquitto`

