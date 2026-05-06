# Threat Intelligence — complementary attacker-side data sources

Project NULLWEAR's primary threat model is **passive Bluetooth-Low-Energy detection of officers via Axon equipment broadcasts**. NULLWEAR addresses that threat layer.

This subfolder documents **complementary intelligence sources** that an attacker can stack alongside (or in place of) BLE surveillance. NULLWEAR does **not** mitigate any of these — they are documented here so that defenders understand the full picture and don't overestimate the protection NULLWEAR provides.

## Why this matters

A sophisticated attacker doesn't pick one intelligence source — they layer them. A real attacker stack could combine:

| Layer | Coverage | Latency | Cost | NULLWEAR mitigates? |
|---|---|---|---|---|
| **BLE OUI/payload mesh** (NULLWEAR's threat) | Per-officer, ~30 m radius | Sub-second | $5–100 per scanner node | **Yes** |
| **Crowd-sourced map apps** (this folder's analysis) | City-wide | 1–10 minutes | $0 (public API) | **No** |
| ALPR / static cameras | Per-vehicle, choke points | Seconds–minutes | Per-camera | No |
| Telco-side cell movement (lawful or unlawful) | National | Real-time | High (insider/legal access) | No |
| Visual / human surveillance | Per-target, high cost | Real-time | Per-watcher | No |

NULLWEAR cleanly removes one layer (the cheapest and easiest one to deploy at scale). The other layers remain. **Defending the officer means defending across all layers**, and this subfolder's job is to document what the other layers look like in practice so that strategic decisions can be made about them.

## What's in here

```
threat-intelligence/
├── README.md                              ← this file
├── 01-waze-crowd-source-analysis.md       ← findings memo
└── output/                                 ← generated charts + JSON summaries
    ├── waze_temporal_hourly.png
    ├── waze_temporal_dow.png
    ├── waze_geographic_heatmap.png
    ├── waze_top_hotspots.png
    └── waze_summary.json
```

## Source data

The analysis in `01-waze-crowd-source-analysis.md` is based on a 12-month dataset of crowd-sourced police-presence reports from a publicly-available consumer mapping platform. The dataset comprises **1,008,227 individual reports** across Greater Melbourne, July 2024 – ~July 2025.

The dataset is held by the maintainer and is NOT included in this repository. The analysis derives only aggregated statistics and anonymised geographic offsets — no specific report records are republished.

## Operational discipline

Same handling rules as the BLE telemetry analysis: source data PROTECTED, only aggregated/anonymised outputs in the published documentation, no specific MAC / coordinate / officer information in any committed file.

## Cross-reference

- For the BLE-side threat and NULLWEAR's mitigation, see the strategic *Mitigation Report* PDF (companion document) and the main repository documentation.
- For the empirical BLE telemetry that validates the BLE threat, see the *Threat Validation Report* PDF (Rev B).
