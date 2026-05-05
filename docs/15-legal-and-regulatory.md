# Legal and Regulatory Considerations

**This document is not legal advice. It is the engineer-author's best-effort summary of the regulatory pathway. It must be reviewed by counsel familiar with the specific jurisdiction's radiocommunications and surveillance law before any operational deployment.**

This document focuses on Australian law because Australia is the named pilot jurisdiction. Equivalent analyses should be conducted for any other jurisdiction where deployment is contemplated.

---

## Regulatory pathway in Australia

### The Radiocommunications Act 1992 (Cth)

The Radiocommunications Act establishes a regime where transmission of radio waves requires either an apparatus licence, a class licence, or a Ministerial determination of exemption. The Australian Communications and Media Authority (ACMA) administers the Act.

NULLWEAR transmits intermittently in the 2.4 GHz ISM band. The relevant class licence is the **Radiocommunications (Low Interference Potential Devices) Class Licence 2015** (LIPD 2015).

### LIPD 2015 considerations

LIPD 2015 permits operation of low-power devices in the 2.4 GHz band (item 49 in Schedule 1, "Wide-band data transmission systems") subject to a maximum mean equivalent isotropically radiated power (EIRP) of 100 mW (20 dBm) and a maximum EIRP spectral density of 10 mW/MHz.

NULLWEAR's transmissions:

- Operate in the same 2.4 GHz BLE band (2402–2480 MHz).
- Use TX power of +8 dBm (≈ 6 mW), well below the LIPD 2015 limit.
- Are bursty (~30 µs per detection event, with detection rate depending on local Axon traffic).
- Have very low duty cycle (in a typical environment, probably < 0.1%).

**The author's view, subject to legal review**, is that NULLWEAR's emissions fall within the existing LIPD 2015 class licence as a wide-band data transmission system, with no need for additional authorisation. The argument is:

- Power is below the limit.
- Spectrum occupied is the same as any BLE device.
- Frequency hopping (across channels 37/38/39) is consistent with BLE-class operation.

However, ACMA may take a different view because NULLWEAR's *purpose* is to interfere with another transmitter's communication. While the Act does not generally regulate the purpose of an emission as long as power and spectrum conform, intentional interference is treated as a separate matter under sections 197–199 of the Act ("Prohibition of unlicensed transmissions" — N/A here — and "Substantial interference, disruption, etc., to radiocommunications").

### Section 199 — interference

Section 199 of the Act prohibits, in summary, **operating a device in a manner that causes substantial interference, disruption or disturbance to radiocommunications**.

**The interference NULLWEAR causes is, by intent, narrowly targeted at one specific class of broadcast (Axon-OUI BLE advertisements).** It is:

- Not a broad-band jammer.
- Not affecting any licensed service other than the targeted one.
- Time-limited per event to ~30 µs.
- Not interfering with the targeted device's *operation* (the Axon device continues to function for its primary purpose — pairing with its own peer; only its passive discoverability is affected).

**Whether s.199 applies in the strict sense is for ACMA and the Minister to determine**, ideally by way of an explicit Ministerial determination under s.27 of the Act.

### Recommended path

1. **Pre-application meeting with ACMA** — the agency programme sponsor (DHA CICC) approaches ACMA technical staff to explain the device, the purpose, the threat model, and the engineering parameters.
2. **Ministerial determination request** — formal request for a determination under s.27 that NULLWEAR's class of operation is permitted. Justifications: officer safety, narrow targeting, minimal RF impact, consistency with existing emergency-services radio authority precedents.
3. **Class licence variation if needed** — if the Ministerial pathway is not available, request a variation to LIPD 2015 to add a new item explicitly covering NULLWEAR-class devices.
4. **Conditions** — likely conditions on any authorisation include: agency-issued only, not for civilian sale, registered serial numbers, recordkeeping of operational deployment.

This pathway is comparable to the regime established for low-power active jamming of explosive-device triggers in counter-IED operational contexts; the Defence Force has a similar exemption under s.27 for narrowly targeted RF capabilities.

### Timeline expectation

ACMA determinations for novel radio devices typically take 8–12 weeks once formally lodged. The pilot deployment plan (`docs/13-pilot-deployment-plan.md`) assumes a 4-week WP1 stand-up window in which the determination is initiated; the full resolution may overlap into WP2/WP3 depending on ACMA workload. Operational issue (WP5) cannot proceed until the determination is granted.

---

## The Telecommunications (Interception and Access) Act 1979 (Cth)

The TIAA prohibits, broadly, the interception of communications passing over a telecommunications system without lawful authority.

NULLWEAR:

- Does not store, decode, or relay any received communication.
- Does not interpret content of any received packet beyond a 24-bit OUI prefix match.
- Causes the targeted communications to fail to reach their intended receiver — but it does not itself "intercept" them in the TIAA sense (it does not "listen to or otherwise extract the content").

**Subject to counsel review, NULLWEAR should not engage TIAA.**

---

## The Surveillance Devices Act 2004 (Cth) and equivalent state Acts (e.g. Surveillance Devices Act 1999 (Vic))

These Acts regulate "surveillance devices" — listening devices, optical surveillance devices, tracking devices, data surveillance devices — and prohibit their use without warrant or other lawful authority for the purpose of monitoring a private activity.

NULLWEAR:

- Is not a listening device (no audio capture).
- Is not an optical surveillance device.
- Is not a tracking device (it does not track anything; it makes the wearer LESS trackable).
- Is not a data surveillance device (it does not capture or record data).

**Subject to counsel review, NULLWEAR should not engage SDA.**

---

## Officer-evidence considerations

A potentially complex question: does NULLWEAR's annihilation of the Axon body camera's BLE pairing advertisements affect the integrity or admissibility of body-camera evidence?

**Short answer: no, because NULLWEAR does not affect the body camera's primary function (recording video to internal storage).** The annihilated packets are advertising packets used for discoverability and peer-pairing — they are not part of the recording chain.

**However**, certain Axon Aware features (live streaming to peers, holster-trigger auto-record activation by Signal Sidearm) may rely on BLE pairing being possible. NULLWEAR may degrade those features within its protection bubble. The agency's procedures for activating recording must therefore not depend on BLE-mediated pairing inside the bubble; the body camera's own button must be the source of truth.

This must be tested as part of WP4 field testing and reviewed by the agency's evidence custodian before operational issue.

---

## Inter-jurisdictional considerations

If NULLWEAR is exported to another jurisdiction (Five Eyes partner, for example):

- **Defence and Strategic Goods List (DSGL)** — verify whether the device or its components fall under export-control provisions. The Nordic nRF5340 SoC is a commercial-grade part with no specific export control. The integrated assembly may be of interest to the Defence Export Controls system; obtain an export-control determination before any cross-border transfer.
- **Recipient jurisdiction's regulatory pathway** — each jurisdiction has its own equivalent of ACMA (FCC in US, Ofcom in UK, ISED in Canada, RSM in NZ) and its own radio-interference statute. The recipient agency is responsible for obtaining their own authorisation.

---

## Liability framework

The agency General Counsel should establish a liability framework before issue covering:

- Officer claims arising from device malfunction (e.g. officer alleges injury because their location was disclosed when the device should have been protecting them).
- Public claims arising from device operation (e.g. claim that NULLWEAR interfered with a third party's BLE device).
- Vendor liability (the contract manufacturer's warranty and indemnity).
- Insurance coverage.

The author's view, subject to counsel: standard agency-issued-equipment liability frameworks should apply with no modification. NULLWEAR is not categorically different from any other agency-issued radio device.

---

## Open questions for legal review

Compile this list and present to General Counsel:

1. Does s.199 of the Radiocommunications Act apply to selective per-packet interference targeting a specific OUI?
2. Is a Ministerial determination under s.27 the correct authorisation pathway, or is a variation to LIPD 2015 preferred?
3. Are there any privacy, evidentiary, or procedural-fairness implications when an Axon body camera worn by an officer is rendered selectively undetectable?
4. Are there any licensing or registration requirements for the depot's operational receivers (used during field testing)?
5. Does the use of the ESP32 emulator (broadcasting `00:25:DF` for test purposes) require any specific authorisation, or is it simply low-power BLE?
6. Are there any disclosure obligations to Axon Enterprise, the OUI registrant, before an agency operationally deploys a device that targets their OUI?

---

## Summary

NULLWEAR's regulatory pathway is novel but not unprecedented. The principal authorisation needed is a Ministerial determination from ACMA, which the agency programme sponsor should pursue at the start of the pilot. All other Acts (TIAA, SDA) are unlikely to engage, but should be confirmed by counsel.

Once the determination is granted, NULLWEAR's operation is on the same footing as any other agency-issued radio device.
