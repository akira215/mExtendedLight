# Matter C++ component Extended Light example

## Purpose

This repository is an example for Matter C++ component (see below to install submodule). It implement an Extended Color Light as the unique ESP MATTER example available as managed component.

It is intended to run on ESP32-C6 device. Hardware could be configured on menuconfig (by default Led on pin 8 and button on pin 9).

## Get C++ components

The embedded software required `esp-ash-components`:

```
git submodule add https://github.com/akira215/esp-ash-components.git components
```


## Producing an OTA

 * Ensure the Partition Table (partitions.csv) contains at least two app slots (ota_0 and ota_1) large enough to support the compiled Matter binary, alongside an otadata partition

 * `ENABLE_OTA_REQUESTOR` shall be set

 * `DEVICE_SOFTWARE_VERSION_NUMBER` shall be incremented

 * Check if `APP_PROJECT_VER` should be updated

 * Generatinf the OTA image using the script :

 ```
 python managed_components/espressif__esp_matter/connectedhomeip/connectedhomeip/src/app/ota_image_tool.py create \
  --vendor-id 0xFFF1 \
  --product-id 0x8000 \
  --version 1 \
  --version-str "v1.0" \
  -da sha256 \
  build/mLightComponent.bin \
  build/mLightComponent.ota
 ```

Part 2: Home Assistant Configuration (OTA Provider)

The official Home Assistant Matter Server core includes local directory scanning capable of serving custom, local OTA payloads directly to paired fabrics.

1. Expose the Firmware Directory

* Locate your Home Assistant /config share directory (using the Samba share or Studio Code Server add-on).
* Create a dedicated subfolder explicitly named updates: /config/updates/.

2. Configure the Matter Server Add-onIn Home Assistant, 

* navigate to Settings → Add-ons → Matter Server.

* Look at your configuration variables. If you are tracking the experimental matter.js engine profile, ensure that enable_test_net_dcl is flagged appropriately to match local sideload requirements.

* Drop your generated firmware_v2.ota file directly into /config/updates/.

* Restart the Matter Server add-on.

3. Authorize Access via ACL (Access Control List)

ecause Matter isolates endpoints securely, you must explicitly give the device permission to query Home Assistant's local OTA provider node.
* Use the Home Assistant Matter Panel or a diagnostic utility like chip-tool.
* Write an Access Control entry to your ESP32-C6 cluster granting Operate Privileges for Cluster 0x0029 (OTA Provider).

Part 3: Triggering the Update

* Once the Matter Server reloads, it checks the /config/updates/ folder, reads your header info, and matches the update payload against your device's current VID/PID.
* Navigate to Settings → Devices & Services → Entities.
* Locate the Update Entity matching your device node. It will transition from Up to date to Update Available.
* Click Install. Home Assistant will send an AnnounceOTAProvider command to your ESP32-C6. The device will then initialize a local network BDX data stream, stream chunks into its alternate flash partition slot, verify the hash, and reboot automatically into the new application code.