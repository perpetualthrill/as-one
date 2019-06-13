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

Copy over the mosquitto.conf file to /etc/mosquitto/conf.d/ and reload: `sudo systemctl stop mosquitto`, `sudo systemctl enable mosquitto`, `sudo systemctl start mosquitto`
