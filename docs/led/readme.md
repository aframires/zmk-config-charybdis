# Charybdis RGB LED Notes

This document covers the optional SK6812 RGB LED support for this ZMK config, including firmware flags, LED data pins, LED counts, nice!nano v2 power rails, and the known external-power caveat.

RGB parts are optional. Use non-RGB firmware when LEDs are not installed or the LED chain is incomplete.

## Optional Parts

The optional LED parts are listed in the [BOM](/docs/bom/readme.md):

- SK6812 Mini-E LEDs: `56` total, `29` left and `27` right.
- 1uF capacitors, size 1206.
- 330 Ohm resistors, size 1206.
- Wire/ribbon cable as needed for LED power, ground, and data.

## Firmware Overview

RGB support is opt-in through [`config/charybdis.conf`](/config/charybdis.conf):

```kconfig
CONFIG_CHARYBDIS_RGB=y
```

This flag enables the LED driver only on the existing keyboard-half shields:

- `charybdis_left`
- `charybdis_right_standalone`
- `dongle_charybdis_right`

Dongle display shields, including `dongle_nice_64 dongle_display`, keep `CONFIG_ZMK_RGB_UNDERGLOW` disabled because they do not have a local LED strip. They still build the Charybdis RGB wrapper so raise-layer RGB keys can proxy commands to connected keyboard halves.

The RGB key bindings are always present in the keymap because devicetree/keymap preprocessing cannot reliably branch on `CONFIG_CHARYBDIS_RGB`. When RGB is disabled, the wrapper remains a no-op and the LED driver stack is not enabled.

## Native ZMK Behavior

This repo uses ZMK's RGB underglow implementation for the keyboard halves. The keymap calls a small Charybdis RGB wrapper so LED-less dongles can send RGB commands to the left/right peripherals without requiring an LED strip on the dongle itself.

ZMK's stock `&rgb_ug` behavior is global on normal split keyboards. The wrapper keeps that same global-locality model, but returns no-op on LED-less dongles and applies the stock RGB actions on halves that have `CONFIG_ZMK_RGB_UNDERGLOW=y`.

RGB controls use ZMK-style relative commands. If the halves already have different saved RGB settings, reset settings or clear both halves before testing so effect, color, and brightness start from the same state.

## Defaults

RGB firmware starts with LEDs on and uses the rainbow/spectrum effect at 25% brightness after settings reset. Existing saved ZMK settings can override this, so flash `settings_reset` to each half once if a newly flashed RGB build still starts with LEDs off.

Current defaults are set in [`boards/shields/charybdis/Kconfig.defconfig`](/boards/shields/charybdis/Kconfig.defconfig):

```kconfig
CONFIG_ZMK_RGB_UNDERGLOW_ON_START=y
CONFIG_ZMK_RGB_UNDERGLOW_EXT_POWER=n
CONFIG_ZMK_RGB_UNDERGLOW_EFF_START=2
CONFIG_ZMK_RGB_UNDERGLOW_BRT_START=25
```

ZMK's default brightness step is currently 10% per brightness keypress:

```kconfig
CONFIG_ZMK_RGB_UNDERGLOW_BRT_STEP=10
```

To use a smaller brightness jump, add a default under the RGB block in [`boards/shields/charybdis/Kconfig.defconfig`](/boards/shields/charybdis/Kconfig.defconfig), for example:

```kconfig
config ZMK_RGB_UNDERGLOW_BRT_STEP
    default 5
```

## Raise Layer Controls

Hold the right thumb `RAISE/BSPC` key, then use these base-layer keys:

```text
K       brightness up
L       brightness down
;       next effect
,       RGB on
.       RGB off
/       hue up
```

## Data Pin

The RGB data pin is configured in [`boards/shields/charybdis/charybdis_rgb.dtsi`](/boards/shields/charybdis/charybdis_rgb.dtsi).

Current default:

```dts
psels = <NRF_PSEL(SPIM_MOSI, 1, 13)>;
```

This maps to:

```text
nice!nano v2: P1.13
Pro Micro:    D15
```

Update both `NRF_PSEL(SPIM_MOSI, <port>, <pin>)` values in `charybdis_rgb.dtsi` if the data pin changes.

## LED Count

The per-side LED count is set where the shared RGB include is used:

- Left side: [`boards/shields/charybdis/charybdis_left.overlay`](/boards/shields/charybdis/charybdis_left.overlay) uses `29`.
- Right side: [`boards/shields/charybdis/charybdis_right_common.dtsi`](/boards/shields/charybdis/charybdis_right_common.dtsi) uses `27`.

## nice!nano v2 Pinout

Top view, USB at top:

```text
        +---------------------------+
 GND ---|                           |--- BATTERY+
 D1  ---| P0.06                     |--- BATTERY+
 D0  ---| P0.08                     |--- GND
 GND ---|                           |--- RESET
 GND ---|                           |--- VCC / 3.3V  (switchable by P0.13)
 D2  ---| P0.17                     |--- D21  P0.31
 D3  ---| P0.20                     |--- D20  P0.29
 D4  ---| P0.22                     |--- D19  P0.02
 D5  ---| P0.24                     |--- D18  P1.15
 D6  ---| P1.00                     |--- D15  P1.13  (RGB data in this config)
 D7  ---| P0.11                     |--- D14  P1.11
 D8  ---| P1.04                     |--- D16  P0.10
 D9  ---| P1.06                     |--- D10  P0.09
        +---------------------------+
```

Relevant Pro Micro to nRF52840 mapping:

```text
D0      P0.08
D1      P0.06
D2      P0.17
D3      P0.20
D4      P0.22
D5      P0.24
D6      P1.00
D7      P0.11
D8      P1.04
D9      P1.06
D10     P0.09
D14     P1.11
D15     P1.13
D16     P0.10
D18/A0  P1.15
D19/A1  P0.02
D20/A2  P0.29
D21/A3  P0.31
```

## Power Rails

The nice!nano v2 has a regulated 3.3V external output, but that rail is switchable by the controller.

```text
BATTERY+ / RAW
     |
     | raw LiPo, not regulated 3.3V
     v
nice!nano regulator
     |
     v
VCC / 3.3V output
     |
     +-- PMW3610 VDD   <- should stay powered if trackball must stay alive
     |
     +-- RGB LED VDD   <- shared on this PCB

P0.13 -- controls nice!nano external/VCC cutoff MOSFET
```

Practical hardware guidance:

- Do not wire PMW3610 directly to battery.
- PMW3610 should be powered from a stable regulated 3.3V logic rail.
- VCC is shared on this PCB, so firmware external-power cutoff affects more than just RGB LEDs.
- Add a physical switch only on the RGB LED VDD wire if hard LED power-off is desired.
- Do not switch LED GND or RGB data; keep grounds shared and switch only LED voltage.
- The LED switch must not cut power to PMW3610, keyboard matrix pullups, or other critical logic.
- Keep PMW3610 I/O voltage aligned with the MCU logic voltage.

Recommended physical switch wiring:

```text
nice!nano VCC / 3.3V ----> PMW3610 VDD
nice!nano VCC / 3.3V ----> LED power switch ----> SK6812 VDD

GND ---------------------> PMW3610 GND
GND ---------------------> SK6812 GND

D15 / P1.13 -------------> SK6812 DIN
```

## External Power Caveat

This branch intentionally uses:

```kconfig
CONFIG_ZMK_RGB_UNDERGLOW_EXT_POWER=n
```

With `EXT_POWER=n`, `RGB_OFF` turns LEDs off through the LED driver while leaving the board power rail enabled. This may use more battery than hard-cutting LED power, but it avoids destabilizing the PMW3610 trackball or requiring reset before `RGB_ON` works again.

The behavior with `EXT_POWER=y` is unsafe for this PCB because it can cut more than the LED rail:

```text
CONFIG_ZMK_RGB_UNDERGLOW_EXT_POWER=y
        |
        v
RGB_OFF calls external power off
        |
        v
P0.13 toggles VCC cutoff
        |
        v
Anything powered from VCC may lose power:
RGB LEDs, PMW3610, displays, pullups, etc.
```

Observed failure mode with external power enabled:

- Trackball stops responding.
- Right side behaves strangely.
- Dongle display may stop reflecting layer/input state correctly.
- Reset recovers because the rail and PMW3610 are initialized again.

If a future hardware revision isolates RGB LED VDD behind a dedicated MOSFET/load switch, `CONFIG_ZMK_RGB_UNDERGLOW_EXT_POWER=y` can be retested for better battery life.

## References

- [nice!nano documentation](https://nicekeyboards.com/docs/nice-nano/)
- [nice!nano pinout and schematic](https://nicekeyboards.com/docs/nice-nano/pinout-schematic/)
- [ZMK power configuration](https://zmk.dev/docs/config/power)
- [ZMK RGB underglow hardware integration](https://zmk.dev/docs/development/hardware-integration/lighting/underglow)
