# Matter C++ component Extended Light example

## Purpose

This repository is an example for Matter C++ component (see below to install submodule). It implement an Extended Color Light as the unique ESP MATTER example available as managed component.

It is intended to run on ESP32-C6 device. Hardware could be configured on menuconfig (by default Led on pin 8 and button on pin 9).

## Get C++ components

The embedded software required `esp-ash-components`:

```
git submodule add https://github.com/akira215/esp-ash-components.git components
```


## Matter OTA using Home Assistant

#### Part 1. On the device side (ESP32)

 * Ensure the Partition Table (partitions.csv) contains at least two app slots (ota_0 and ota_1) large enough to support the compiled Matter binary, alongside an otadata partition

 * `ENABLE_OTA_REQUESTOR` shall be set

 * `DEVICE_SOFTWARE_VERSION_NUMBER` shall be incremented

 * `APP_PROJECT_VER` does not need to be updated for OTA, use it for releases follow up

 * Once the binary is compiled, you can generate the OTA image using the script :

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

#### Part 2: Home Assistant Configuration (OTA Provider)

The official Home Assistant Matter Server core includes local directory scanning capable of serving custom, local OTA payloads directly to paired fabrics.

1. Configure the Matter Server Add-onIn Home Assistant, 

 * Navigate to Settings → Add-ons → Matter Server.

 * Look at your configuration variables. If you are tracking the experimental matter.js engine profile, ensure that enable_test_net_dcl is flagged appropriately to match local sideload requirements.


2. Expose the Firmware Directory

 * Create a dedicated subfolder explicitly named updates: `addon_configs/core_matter_server/updates`.

 * Drop your generated `firmware_v2.ota` file directly into `/addon_configs/core_matter_server/updates/`.

 * Create a file named `firmware_update.json` inside `/addon_configs/core_matter_server/updates/` folder

```json
{
  "modelVersion": {
    "vid": 65521,
    "pid": 32768,
    "softwareVersion": 2,
    "softwareVersionString": "2.0.0",
    "cdVersionNumber": 1,
    "softwareVersionValid": true,
    "otaUrl": "file:///mLightComponent_v2.ota",
    "otaChecksum": "E3w0MMomKm127q9Bol9mY2qotxHLveT3hmzCu+/l88w=",
    "otaChecksumType": 1,
    "minApplicableSoftwareVersion": 1,
    "maxApplicableSoftwareVersion": 1,
    "releaseNotesUrl": ""
  }
}
```

Critical Fields to Change:
  * `softwareVersion`: Must be an integer higher than the version currently running on your ESP32 device.
  * `otaUrl`: Must match the exact filename of your binary file placed in the same folder, prefixed with file:///.
  * `otaChecksum`: Requires a Base64-encoded SHA-256 string of your .ota binary. to get it, run a terminal in the same folder as the image and run this command:
`openssl dgst -sha256 -binary mLightComponent_v2.ota | openssl base64`

 * Restart the Matter Server add-on.

3. Not required: Authorize Access via ACL (Access Control List)

Because Matter isolates endpoints securely, you must explicitly give the device permission to query Home Assistant's local OTA provider node.

 * Use the Home Assistant Matter Panel or a diagnostic utility like chip-tool.
 * Write an Access Control entry to your ESP32-C6 cluster granting Operate Privileges for Cluster 0x0029 (OTA Provider).

#### Part 3: Triggering the Update

 * Once the Matter Server reloads, it checks the `/addon_configs/core_matter_server/updates/` folder, reads your header info, and matches the update payload against your device's current VID/PID. It deletes the file of the folder and store it to its internal storage tree.

 * Navigate to Matter Server - Click on the Node - Click on the `Update` button.

 * Click Install. Home Assistant will send an AnnounceOTAProvider command to your ESP32-C6. The device will then initialize a local network BDX data stream, stream chunks into its alternate flash partition slot, verify the hash, and reboot automatically into the new application code.