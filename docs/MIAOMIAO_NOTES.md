# MiaoMiao Charybdis 4x6 — notes

This branch of `zmk-config-charybdis` is adapted to the wireless **MiaoMiao**
Charybdis 4x6 (AliExpress / 喵喵猫 / "wireless Charybdis") that ships with two
`nice!nano v2` controllers and a PMW3610 trackball on the right side.

Upstream (`carrefinho/zmk-config-charybdis`) targets the official BastardKB
PCB, which wires the right-hand columns and the right thumb cluster
differently from the MiaoMiao PCB. The deltas below restore compatibility.

## Hardware-specific changes vs upstream

| File | Change |
|------|--------|
| `boards/shields/charybdis/charybdis_right_common.dtsi` | Right-side `col-gpios` set to the **same order** as the left (`pro_micro 19, 20, 10, 6, 7, 8`); upstream had them reversed for BastardKB. |
| `boards/shields/charybdis/charybdis.dtsi` | Row 4 of the matrix transform: right thumbs `RC(4,9) RC(4,10) RC(4,11)` (MiaoMiao wiring) instead of `RC(4,6) RC(4,8) RC(4,10)` (BastardKB wiring). |
| `boards/shields/charybdis/charybdis_right_common.dtsi` | Trackball: `swap-xy; invert-x;` only — `invert-y;` is commented out to match the seller's proven orientation. |
| `boards/shields/charybdis/charybdis_layers.h` | Layer scheme reduced to `BASE / NAV / SYM / ADJ / SCROLL / SNIPING`. |
| `boards/shields/charybdis/charybdis_trackball_processors.dtsi` | `move` processor activated on `BASE NAV SYM ADJ` (no `POINTER` layer in this scheme). |
| `config/charybdis.keymap` | Replaced with a vendor-style BastardKB `charybdis_4x6` port (Option B — no home-row mods). |
| `build.yaml` | Trimmed to standalone-only (`charybdis_left`, `charybdis_right_standalone`, `settings_reset`). |

The PMW3610 driver, ZMK Studio integration, conditional-layer (NAV+SYM → ADJ),
combo for Studio unlock, and snipe/scroll input processors all remain upstream.

## First-flash checklist (standalone mode)

1. Build via GitHub Actions (or `manual_build/build.py`). Artefacts:
   - `charybdis_left-nice_nano-zmk.uf2`
   - `charybdis_right_standalone-nice_nano-zmk.uf2`
   - `settings_reset-nice_nano-zmk.uf2`
2. For **each half**: USB-connect → double-tap reset → drag
   `settings_reset-nice_nano-zmk.uf2` onto the mounted drive → wait for
   reboot. This wipes any old pairing / Studio state.
3. Flash the actual firmware:
   - Left half  ← `charybdis_left-nice_nano-zmk.uf2`
   - Right half ← `charybdis_right_standalone-nice_nano-zmk.uf2`
4. Power both halves on; halves auto-pair, then the right half (central)
   advertises Bluetooth — pair it with your computer in the usual way.

## Validation checklist

Run these in a plain text editor (or the
[ZMK Studio web app](https://zmk.studio/)) with both halves connected:

- **Main grid (48 keys)** — type top to bottom, left to right, confirm
  every key registers and matches the printed legend on its cap.
- **Left thumbs (5 keys)** — should produce, in physical order:
  TAB (hold = NAV layer), BSPC, LGUI, SPACE, DEL. The exact physical
  order depends on PCB silk; if any are swapped, reorder the 5 left-thumb
  bindings on positions 48–52 in `config/charybdis.keymap`.
- **Right thumbs (3 keys)** — should produce ENTER, SPACE, BSPC (hold = SYM).
  Same caveat: if swapped, reorder positions 53–55.
- **Layers**
  - Hold the leftmost left-thumb key → NAV layer (arrows/numpad/F-keys).
  - Hold the rightmost right-thumb key → SYM layer (symbols/brackets).
  - Hold BOTH simultaneously → ADJ layer (BT, output, `&bootloader`,
    `&sys_reset`, `&mkp MB1/2/3`, sniping/scroll toggles).
- **Studio unlock combo** — hold all three right-thumb keys (positions
  53/54/55) at once; ZMK Studio should unlock.
- **Trackball**
  - Cursor moves on the BASE layer. If X or Y axis feels inverted,
    flip the corresponding `invert-x;` / `invert-y;` in
    `boards/shields/charybdis/charybdis_right_common.dtsi`.
  - From ADJ, momentary-hold "SCROLL" → trackball acts as scroll wheel.
  - From ADJ, momentary-hold "SNIP" → trackball moves at 1/3 speed.
  - Mouse buttons MB1/MB2/MB3 fire from the ADJ layer (and from
    SCROLL/SNIP layers on positions 31/32/33).

## Switching to Option A (home-row mods) later

If you want the BastardKB-vendor home-row mods after living with the plain
layout for a week or two, add this `behaviors` block to the top of
`config/charybdis.keymap` (inside `/ { ... }`):

```dts
behaviors {
    hml: home_row_mod_left {
        compatible = "zmk,behavior-hold-tap";
        #binding-cells = <2>;
        flavor = "balanced";
        require-prior-idle-ms = <150>;
        tapping-term-ms = <280>;
        quick-tap-ms = <175>;
        bindings = <&kp>, <&kp>;
        hold-trigger-key-positions = <6 7 8 9 10 11 18 19 20 21 22 23 30 31 32 33 34 35 42 43 44 45 46 47 53 54 55>;
        hold-trigger-on-release;
    };
    hmr: home_row_mod_right {
        compatible = "zmk,behavior-hold-tap";
        #binding-cells = <2>;
        flavor = "balanced";
        require-prior-idle-ms = <150>;
        tapping-term-ms = <280>;
        quick-tap-ms = <175>;
        bindings = <&kp>, <&kp>;
        hold-trigger-key-positions = <0 1 2 3 4 5 12 13 14 15 16 17 24 25 26 27 28 29 36 37 38 39 40 41 48 49 50 51 52>;
        hold-trigger-on-release;
    };
};
```

…and change the BASE layer's home row from

```
&kp ESC &kp A &kp S &kp D &kp F &kp G   &kp H &kp J &kp K &kp L &kp SEMI &kp SQT
```

to

```
&kp ESC &hml LCTRL A &hml LALT S &hml LGUI D &hml LSHFT F &kp G   &kp H &hmr RSHFT J &hmr RGUI K &hmr LALT L &hmr RCTRL SEMI &kp SQT
```
