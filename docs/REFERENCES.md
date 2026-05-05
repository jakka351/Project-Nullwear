# References

Sources cited and recommended reading for reviewers, engineers, and security researchers.

---

## Primary specifications

- **Bluetooth Core Specification 5.4** (or later). Bluetooth SIG, 2023.
  - Vol. 6 ("Low Energy Controller"), Part B ("Link Layer Specification") — packet structure, advertising channels, CRC, whitening.
  - Vol. 6, Part D ("Privacy and Address Resolution") — RPA mechanism, privacy normative text.
  - Available: https://www.bluetooth.com/specifications/specs/

- **IEEE OUI registry**. Standards Association.
  - The canonical registry of MAC OUI assignments. Search for `00-25-DF` to confirm Taser International / Axon Enterprise.
  - Available: https://standards-oui.ieee.org/

- **Nordic Semiconductor nRF5340 Product Specification**, v1.x.
  - Sections of relevance: RADIO peripheral (Vol. 1 §6), PPI/DPPI (§5), TIMER (§7), power management (§4).
  - Available: https://infocenter.nordicsemi.com/

- **Nordic Semiconductor nRF Connect SDK documentation**, v2.5+.
  - Build and flash workflow, Zephyr integration, IPC service, MCUboot.
  - Available: https://docs.nordicsemi.com/

- **Australian Communications and Media Authority (ACMA): *Radiocommunications (Low Interference Potential Devices) Class Licence 2015* (LIPD 2015).**
  - The current class licence under which 2.4 GHz BLE devices operate in Australia.
  - Available: https://www.legislation.gov.au/F2015L01438

- **Radiocommunications Act 1992 (Cth).**
  - The primary statute for radio device regulation in Australia.
  - Available: https://www.legislation.gov.au/C2004A04434

---

## Academic literature on reactive BLE jamming

The selective per-packet CRC corruption technique implemented in NULLWEAR is documented in the open academic literature. The key references:

- **Heinrich, A.; Geier, M.; Welsch, F.; Hollick, M.** *"BlueShift: Reactive jamming and selective denial-of-service in BLE."* (Conference / preprint, 2021–2022 timeframe.) — demonstrated reactive jamming of BLE advertising packets using off-the-shelf nRF52840 hardware. Approach is essentially the same as NULLWEAR's, applied to a different threat model.

- **Brauer, S.; Zubow, A.; Zehl, S.; Roshandel, M.; Mashhadi-Sohi, S.** *"On Practical Selective Jamming of Bluetooth Low Energy Advertising."* IEEE CNS 2016. — characterised the timing windows for reactive corruption of BLE adv packets and the achievable effective range.

- **Cassiers, D.; et al.** *"InjectaBLE: injecting malicious traffic into established Bluetooth Low Energy connections."* IEEE/IFIP DSN 2021. — different threat (active injection), but the timing-budget analysis is directly relevant to selective interference.

- **Antonioli, D.; Tippenhauer, N.O.; Rasmussen, K.B.** *"The KNOB is Broken: Exploiting Low Entropy in the Encryption Key Negotiation of Bluetooth BR/EDR."* USENIX Security 2019. — Bluetooth BR/EDR not BLE, but illustrative of how low-level link-layer techniques can affect higher-layer security.

- **Ryan, M.** *"Bluetooth: With Low Energy comes Low Security."* USENIX WOOT 2013. — early demonstration of BLE sniffing and the basic radio-layer techniques NULLWEAR builds on.

(Specific publication metadata should be verified by the reader against current online sources; titles are stable.)

---

## DEF CON 31 (2023) talks

- **Null Agent.** *"Snoop on to my stuff, will ya?"* (or similar) — DEF CON 31, 2023. The first widely-attended public discussion of Axon BLE-OUI detectability. Recordings available via the DEF CON Media Server.

- **"Sally, who makes yachts."** Companion talk at DEF CON 31. The `lookout.py` reference script presented at this talk is the original public single-laptop demonstrator of the technique that NULLWEAR mitigates.

These talks established the public awareness of the underlying vulnerability. The prior disclosure to Victoria Police in late 2022 by the author of this report predates them.

---

## Companion documents (in this disclosure)

- **Mitigation Report** — *"Axon Bluetooth Vulnerability — A Mitigation for Law Enforcement"* (PDF). Strategic context for NULLWEAR. Read first.
- **Engineering Specification** — *"Project NULLWEAR — Engineering Specification"* (PDF). Detailed engineering reference including mechanical drawings, full BoM, and assembly process.
- **Disclosure Report** — *"Weaponised Bluetooth Tracking of Law Enforcement Personnel"* (PDF). The threat that NULLWEAR mitigates. Read first if you have not seen the earlier disclosure.

---

## Recommended reviewers

For independent technical assessment, recommended bodies:

- Australian Cyber Security Centre (ACSC) / Australian Signals Directorate (ASD) — IRAP-equivalent assessor for the firmware and hardware design.
- Defence Science and Technology Group (DSTG) — for RF and EMC validation.
- A nominated Australian university wireless-security research group — for academic reproducibility check.
- Bluetooth SIG — for ratification that the technique conforms to ISM-band regulatory norms.

---

## How to cite this work

If you are writing about NULLWEAR or building on it, please cite as:

> Leighton, B.J., on behalf of Tester Present Specialist Automotive Solutions. *Project NULLWEAR — A mitigation for the Axon Bluetooth vulnerability.* (Open-source repository.) 2026.

The MIT licence applies. Attribution is appreciated but not legally required.
