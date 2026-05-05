# NULLWEAR — User Manual (for the officer)

**Audience:** the sworn officer who has been issued a NULLWEAR/P device for daily wear.

**Intended length of read:** 5 minutes.

---

## What it is

NULLWEAR is a small electronic device about the size of a credit card, issued to you in addition to your existing duty equipment. Its only function is to **prevent your Axon body camera, holster, Taser and other Axon equipment from being passively tracked by third parties using off-the-shelf Bluetooth scanners**.

It does this without changing how your Axon equipment works. Your body camera still records when you press the button. Your Taser still works. Your holster still wakes the camera. The pairing between your phone and your equipment is unaffected.

What changes is that someone listening for your equipment from outside a roughly 30-metre bubble around you no longer sees it. The signal is destroyed in the air before they can read it.

## What it looks like

- Plastic case, matte black, soft-touch coating.
- About 50 × 35 mm and 11 mm thick. Roughly the footprint of a debit card, somewhat thicker.
- Weighs about 28 g.
- One small LED visible on the top edge.
- One small button next to the LED.
- A USB-C charging port on one short edge.
- Two M3 mounting holes on the back for a belt clip or MOLLE attachment.

## How to wear it

Three options, in order of preference:

1. **Belt clip** — the supplied stainless steel belt clip attaches via the two M3 holes on the back. Clip it to your duty belt, ideally near your body camera.
2. **Body-camera mount** — you may attach the device directly to your body-camera mount using the supplied bracket.
3. **Pocket** — acceptable but less effective if pocketed under heavy fabric layers.

Do not wear the device inside a metal-lined pouch or inside a Faraday-equipped vest pocket — that defeats the radio function.

## Daily routine

| When | What you do |
|---|---|
| Start of shift | Take it from the dock. Briefly press the button. Confirm the LED shows **steady green**. Clip on. |
| During shift | Nothing. It runs autonomously. |
| End of shift | Clip off. Place it back on the dock. The dock charges it. No power-down required. |

That's the whole user-side process. Total interaction time per shift: under 30 seconds.

## What the LED tells you

| LED state | Meaning | Action |
|---|---|---|
| Steady green (dim) | Battery > 50%, normal operation | None |
| Steady amber | Battery 20–50%, still operational | Charge before next shift |
| Slow red blink | Battery < 20%, charge soon | Charge as soon as practical |
| Pulsing green ("breathing") | On the dock, charging | None |
| Rapid red blink (fast) | Fault — the device may not be working | Report at end of shift; obtain replacement |
| LED off entirely | Either deeply discharged or off | Charge for 10 minutes; if still off, report and get a replacement |
| Brief blue flicker | A nearby Axon device's transmission was just annihilated | None — informational only; means the device is working |

## What the button does

- **Short press (< 1 second)** — confirms LED is currently working. Useful at start of shift.
- **Long press (held for 3 seconds)** — initiates manual deep sleep. The device stops protecting equipment until next clipped to the dock or until the button is short-pressed again. Use this only if you have been instructed to.

## Common questions

### Will it interfere with my body camera or Taser?

No. The device is designed to operate within a few microseconds, on a different protocol layer, with no impact on your own Axon equipment's pairing, recording or operation. The very design constraint that drove its development is that it must not affect the equipment it is protecting.

### Will it interfere with my radio, my phone, or other officers' equipment?

No. It transmits only when it detects a specific kind of Bluetooth packet from Axon equipment, and only on Bluetooth's 2.4 GHz band, in pulses lasting microseconds. Your radio (UHF/VHF), your phone (cellular), Wi-Fi, GPS, and the Bluetooth equipment of every officer near you are unaffected.

### What if I forget to clip it on?

You're back to the pre-NULLWEAR state — your equipment is detectable from a distance. Wear it on every shift.

### What if I lose it?

Report immediately. The device contains no information about you, no encrypted material, no operational data. It is not a security risk if recovered by a third party. But it is your issued equipment and is logged against your badge number.

### Can I use it off-duty?

Off-duty equipment policy is a matter for your agency, not for this manual. Defer to your supervising sergeant.

### Can I take it through airport security?

Yes. It contains a small lithium battery within the limits of standard carry-on. There are no restricted radio capabilities. If asked, describe it as a personal Bluetooth device with manufacturer documentation in the supplied carry pouch.

### What if the LED is off and I can't tell if it's working?

If you can't confirm green-on at the start of a shift, do not deploy. Get a replacement from the watch commander or the equipment locker. There are spares.

### What should I tell the public if asked?

You don't need to discuss it. If pressed, "It's a personal radio safety device" is sufficient.

### What if it gets wet?

It is rated IP67 — submersible in 1 m of water for 30 minutes. Rain, sweat, splashes, brief immersion: no problem. Don't put it through a washing machine.

### What if it gets dropped?

It is rated for a 1.5 m drop onto concrete on any face. If you drop it from greater height, inspect for case damage before continuing to use it.

## When to escalate

| If... | ...escalate to |
|---|---|
| LED is red rapid blink | Equipment locker / watch commander |
| LED stays off after 10 min on dock | Equipment locker / watch commander |
| Device is physically damaged | Equipment locker / watch commander |
| You suspect the device is no longer working but the LED looks normal | Watch commander → field test technician (per `docs/10-field-testing-protocol.md`) |
| You lose the device | Immediately, watch commander |

## What you do not have to do

- You do not have to power it on or off (the dock and the LED handle that).
- You do not have to update its firmware (only the depot does that).
- You do not have to know what an OUI is, what a CRC is, what a BLE advertising packet is, or how reactive interference works. The device handles all of that on its own.

## What you should know

You are wearing it because, without it, an unaccountable third party with a $5 Bluetooth listener can identify and locate you to within ~10 metres anywhere they have a sensor. That capability has been demonstrated to be deployable across an entire city for the cost of a second-hand car. The device on your belt makes you invisible to that capability for as long as you wear it. That is the entire point.

---

**Issued by:** _________________________

**To officer (badge number):** _________________________

**Serial number:** _________________________

**Date:** _________________________
