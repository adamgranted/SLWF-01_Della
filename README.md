# Della Mini Split Control with ESPHome

**This project is a work in progress and is currently broken.**

This _will_ enable control of a Della mini split air conditioner using ESPHome and a SLWF-01 dongle. The implementation is based on reverse-engineered protocol from the [Klima04.txt file](https://github.com/adaasch/AC-hack/issues/3#issuecomment-2346573789).

## Hardware Requirements

- SLWF-01 dongle (made by SMLight, commonly used for Midea AC units)
- Della mini split air conditioner
- Home Assistant setup

## ESPHome Compatibility

This project is compatible with ESPHome 2025.2.2 and later. The configuration uses the new format where the platform is defined as a separate component (e.g., `esp8266:` instead of inside the `esphome:` block).

### Key Configuration Points

- We use the platform-specific `esp8266` component instead of specifying it in the `esphome` block
- We use the `ota` component with the `platform: esphome` format introduced in ESPHome 2024.6.0
- We configure the API encryption key in base64 format as required by ESPHome 2025.2.2

## Successfully Tested

The code successfully compiles with ESPHome 2025.2.2. The UART protocol implementation matches the Della mini split's communication format based on the Klima04.txt reference implementation.

## Installation

1. **Install ESPHome**: If you haven't already, install ESPHome:
   ```bash
   pip install esphome
   ```

2. **Clone this repository**:
   ```bash
   git clone https://github.com/adamgranted/SLWF-01_Della
   cd SLWF-01_Della
   ```

3. **Edit secrets.yaml**: Update the secrets.yaml file with your WiFi credentials and other information:
   ```yaml
   wifi_ssid: "YourWiFiName"
   wifi_password: "YourWiFiPassword"
   fallback_password: "FallbackPassword"
   api_encryption_key: "<base64-encoded-key>"  # Generate with: openssl rand -base64 32
   ota_password: "YourOTAPassword"
   ```

4. **Compile and flash**: Connect your SLWF-01 dongle to your computer and run:
   ```bash
   esphome run della_ac.yaml
   ```

5. **Connect to Home Assistant**: The device should automatically be discovered by Home Assistant. If not, you can manually add it through the "ESPHome" integration.

## Connecting to an Existing ESPHome Instance

If you already have an ESPHome setup and want to integrate this Della AC component:

1. **Clone the Repository into Your ESPHome Directory**:
   ```bash
   # Navigate to your ESPHome config directory (typically ~/.esphome or your Home Assistant config/esphome)
   cd ~/.esphome
   
   # Clone the repository
   git clone https://github.com/adamgranted/SLWF-01_Della
   
   # Create a symbolic link to the components directory
   ln -s SLWF-01_Della/components/della_ac components/della_ac
   ```

2. **Staying Updated**:
   ```bash
   # To update to the latest version
   cd ~/.esphome/SLWF-01_Della
   git pull
   ```

3. **Configure Your Existing ESPHome YAML**:
   Add the following to your existing ESPHome configuration file:
   ```yaml
   # UART Configuration for communicating with the AC
   uart:
     id: ac_uart
     tx_pin: GPIO1  # Verify pins for your SLWF-01 dongle (GPIO 13 in original AC-Hack.txt)
     rx_pin: GPIO3  # Verify pins for your SLWF-01 dongle (GPIO 15 in original AC-Hack.txt)
     baud_rate: 9600
     parity: EVEN
     data_bits: 8
     stop_bits: 1

   # External components directory
   external_components:
     - source: components
       components: [della_ac]

   # Use the della_ac component
   climate:
     - platform: della_ac
       name: "Della Air Conditioner"
       uart_id: ac_uart
   ```

4. **Hardware Connection**:
   - Connect the SLWF-01 dongle to your Della AC unit using the appropriate cables
   - The dongle uses TX/RX for communication with the AC unit
   - The original AC-Hack.txt used GPIO 13 (TX) and GPIO 15 (RX) with 9600 baud rate, 8 data bits, even parity, and 1 stop bit
   - The default configuration for SLWF-01 uses GPIO1 and GPIO3, which may need adjustment based on your specific dongle

5. **Verify Communication**:
   - After flashing your ESPHome configuration, check the logs to ensure proper communication
   - Look for successful initialization of the UART and the climate component
   - You should see log messages showing the AC status being read successfully

6. **Troubleshooting Connection Issues**:
   - If you experience connection issues, try swapping the TX/RX pins
   - Verify the baud rate and parity settings match your AC unit's requirements
   - Check the physical connection between the dongle and AC unit
   - Compare the protocol implementation in `della_ac.cpp` with your specific AC model if commands aren't working

## ESPHome Dashboard Integration

If you're using the ESPHome Dashboard (either standalone or through Home Assistant), you can easily manage your Della AC integration:

1. **Adding to ESPHome Dashboard**:
   - Option 1: Use the installation method described above to add the component to your ESPHome directory
   - Option 2: If using Home Assistant, you can create an external_components directory in your config/esphome folder:
     ```bash
     cd config/esphome
     git clone https://github.com/adamgranted/SLWF-01_Della
     ```
   - Copy the `della_ac.yaml` file from the repository to your ESPHome dashboard configuration directory
   - In the ESPHome Dashboard, click on the three dots next to the newly added device
   - Select "Edit" to customize the configuration for your specific setup

2. **Using Git for Updates**:
   - To update the component when improvements are made:
     ```bash
     cd ~/path/to/your/SLWF-01_Della
     git pull
     ```
   - Rebuild and reflash your device after updating

3. **Using Secrets in Dashboard**:
   - The ESPHome Dashboard automatically uses the `secrets.yaml` file in your ESPHome directory
   - Make sure to update it with your credentials as described in the Installation section

4. **OTA Updates via Dashboard**:
   - Once initially flashed, you can update your device over-the-air through the dashboard
   - Click "Install" and select "Wirelessly" to push new configurations without connecting via USB

5. **Monitoring Logs**:
   - In the ESPHome Dashboard, click on the three dots next to your device
   - Select "Logs" to view real-time logs for debugging

6. **Integration with Home Assistant**:
   - Devices added through the ESPHome Dashboard will automatically appear in Home Assistant
   - Look for your newly added Della AC under the ESPHome integration in Home Assistant

## Usage

Once connected to Home Assistant, you'll be able to control:

- Power on/off
- Temperature settings (including half-degree precision)
- Operation modes (Cooling, Heating, Auto, Fan-only, Dry)
- Fan speeds
- Special modes (ECO and Turbo)

## Troubleshooting

- **Configuration Format**: If you see errors about the `platform` key in the ESPHome block, make sure you're using ESPHome 2025.2.2 or later, or update the configuration format as shown in the della_ac.yaml file.
- **UART Communication**: If you experience communication issues, verify the UART pins in the della_ac.yaml file match your SLWF-01 dongle's configuration.
- **Protocol Mismatch**: If the AC doesn't respond to commands, there might be slight differences in the protocol. Check the logs for any error messages.
- **ESP8266 Pins**: The SLWF-01 may use different GPIO pins than specified in the default configuration. Check the dongle's documentation or try alternative pin configurations if communication fails.
- **API Encryption Key**: If you see "Invalid key format" errors, make sure to generate a proper base64 key using `openssl rand -base64 32`.

## SLWF-01 Configuration

The default configuration assumes the following for the SLWF-01 dongle:
- TX pin: GPIO1
- RX pin: GPIO3
- 9600 baud rate with even parity (8E1)

If your dongle uses different pins, update the `uart` section in the configuration file.

## Credits

This implementation is based on the protocol analysis found in Klima04.txt, which was derived from https://github.com/adaasch/AC-hack.

## License

MIT 
