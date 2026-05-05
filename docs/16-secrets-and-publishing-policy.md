# Secrets, Keys, and Publishing Policy

What goes in the public repository, what doesn't, and the operational handling of cryptographic material across the project's lifecycle.

This is the most important non-technical document in the repository. Get this wrong and you can fully compromise every fielded NULLWEAR unit silently. Get it right and you give away the design without giving away the integrity.

---

## TL;DR — the rule in one paragraph

**Publish everything that helps a defender build, verify, deploy, or audit the system. Withhold only the cryptographic material whose secrecy is the only thing that distinguishes a legitimate firmware update from a malicious one, and the operational data that links specific officers to specific equipment.** Everything else, including all source code, all documentation, all hardware design, all test procedures, all bill-of-materials, all reference receivers, and the Axon OUI itself, is publishable. The OUI is on every Axon product on earth and was already documented at DEF CON 31; keeping it secret here protects nobody.

---

## 1. Decision tree

When you are about to commit, push, or publish anything, ask:

```
Is the thing you want to publish a credential, a private key, or a way
for someone to forge a legitimate-looking artefact?

  ├── YES → DO NOT PUBLISH. (See §3 for handling.)
  │
  └── NO  → Is it information about a specific officer, a specific
            deployment, a specific facility, or a specific operational
            target?
            │
            ├── YES → DO NOT PUBLISH. (See §4 for handling.)
            │
            └── NO  → Is it information that materially helps a
                     defender build, verify, deploy, or audit the
                     system?
                     │
                     ├── YES → PUBLISH. (See §2 for what goes here.)
                     │
                     └── NO  → Don't publish unnecessarily, but it's
                              not a secrecy concern. (See §5.)
```

---

## 2. What MUST be public

These items belong in the public repository. The strategic value of this project depends on them being freely accessible to any agency in any friendly jurisdiction.

| Category | Items | Why public |
|---|---|---|
| Source code | All firmware, all Python tooling, all build scripts | Reviewability is the entire integrity model |
| Documentation | All `docs/*.md`, all manuals, all procedures | The fix is useless if nobody can build, deploy or use it |
| Hardware design | BoM, component MPNs, reference layout constraints, mechanical specifications | Required for any contract manufacturer to build |
| The target OUI | `00:25:DF` and the fact it belongs to Axon | Public IEEE record; published at DEF CON 31; pretending otherwise is theatre |
| Test procedures | ATP, field-test protocol, acceptance criteria | Empirical verification is mandatory; everyone needs the procedure |
| Verification tools | Reference receiver, ESP32 emulator | Defender needs these to validate units |
| Architecture diagrams | All SVGs in `docs/img/` | Communication aid |
| Public signing keys | The `.pub` half of the firmware-signing keypair | Allows anyone to verify a built image came from the owning agency |
| Test source firmware | `axon_emulator.ino` | It's a $5 ESP32 — the value is the technique not the artefact |
| The cost model, the timeline, the regulatory pathway | All of it | Decision-makers need it to decide |
| The threat model and the residual risks | The full security analysis | Defenders deserve to know what NULLWEAR does NOT protect against |

**A useful heuristic:** if Axon Enterprise themselves read this repository, would the disclosure of any of these items make a NULLWEAR unit easier to defeat? **No.** Defeating NULLWEAR requires either:
1. Axon's own equipment to switch to RPA-based MAC addressing (the actual fix), OR
2. A receiver capable of reconstructing CRC-failed packets (custom SDR, expensive, not widely deployed) — and even then, the recovered information is "an Axon-OUI packet was being sent somewhere within range of this scanner", which is uselessly vague.

Publishing the design does not lower the bar to attacking it. The bar is set by physics.

---

## 3. What must NEVER be public

These items must never enter the public repository. Treat them with the same care as a kernel-module signing key, a code-signing certificate, or a TLS private key.

### 3.1 Firmware signing private key

The MCUboot bootloader on every NULLWEAR unit verifies signed images before booting. The agency holds the private half of the signing keypair. Anyone with that key can produce a malicious firmware image that the bootloader will accept.

A compromised key allows an attacker to:

- Distribute a "NULLWEAR" firmware that LED-states "green / working" but does not actually annihilate.
- Distribute a firmware that exfiltrates data via USB-CDC when connected to a host.
- Distribute a firmware that disables the radio entirely.

This is the single most destructive failure mode of the project. The private key compromise is not equivalent to losing a device — it is equivalent to losing every device.

**Storage:** generate the key in a hardware security module (HSM). Never let it leave the HSM in cleartext. Use multi-person ceremony for any operation involving the key. Rotate the key per major release.

**Backup:** Shamir's Secret Sharing across at least three custodians. Recovery requires a quorum. Never store full key material in a single location.

**Procedure for build releases:** the build artefact is unsigned in CI; signing happens out-of-band on a controlled signing workstation that talks to the HSM.

**What goes in the repo:** the **public** half of the keypair, committed as `keys/release-public.pem` (or per-agency, if multi-agency keys are in use). This allows any reviewer to verify a delivered binary.

### 3.2 Production credentials

| Type | Examples | Why secret |
|---|---|---|
| Cloud / CM portal credentials | Mouser, Digi-Key, Element14 supplier accounts | Procurement-action capability |
| Code-signing certificates (CI) | If adopting Sigstore / cosign for binary transparency | Same risk as §3.1 |
| HSM PIN / unlock material | The thing that controls access to §3.1 | Single point of total compromise |
| Test-rig telemetry endpoints | If the CM streams ATP results back | Authentication credentials |
| Internal agency programme-specific URLs | Asset-management API keys, dock-management tokens | Network privilege |

### 3.3 The agency asset register linking serials to officers

Discussed in `09-operations-manual.md` §11 and `14-security-considerations.md`. The asset register is **operationally sensitive** — it is the map from a NULLWEAR serial number to a specific badge number. If an adversary can read the asset register and physically observe a unit, they can identify the wearer.

This is per-agency data, not project data. It does not belong in this public repository. It does not belong in any system not specifically authorised to hold sensitive officer information.

### 3.4 Test-source emulator MAC patterns used to identify pilot officers

If the pilot uses additional fake MACs on test sources (e.g. `00:25:DF:DE:AD:01` through `0n` for officer-correlation studies), and those MACs are correlated with specific officer identities in test reports, those reports must be sanitised before publication.

The test-source MACs themselves (`00:25:DF:DE:AD:BE` etc.) are public — they're in the firmware source. What is sensitive is any link between a specific test MAC and a specific real human.

---

## 4. Per-deployment confidentiality

These items are not secret in a cryptographic sense, but they are operationally sensitive on a per-deployment basis. They should be controlled within each agency's information-classification framework rather than published.

| Item | Suggested classification (Australian PSPF) |
|---|---|
| Asset register (serial → officer) | PROTECTED |
| Specific deployment locations | PROTECTED |
| Field-test results identifying real Axon devices in known agency facilities | OFFICIAL: Sensitive |
| Officer feedback containing personally-identifying detail | OFFICIAL: Sensitive |
| ATP per-unit reports (just the engineering measurements, not officer mapping) | OFFICIAL |
| Aggregate fleet metrics (active units, fault rates) | OFFICIAL |
| Acknowledgement of programme existence in any specific jurisdiction | (depends on jurisdiction's announcement strategy) |

Under Five Eyes information-sharing arrangements, equivalent classifications apply (e.g. UK OFFICIAL, US FOUO/CUI, Canadian Protected B).

---

## 5. The grey zone — release timing

Two categories of information are in principle public but are released on a coordinated timeline:

### 5.1 Recovered attacker stack details

The original disclosure report describes a recovered productionised weaponisation of the underlying vulnerability. The strategic Mitigation Report and the Engineering Specification PDFs reference it; this repository implements the answer.

**Specific URLs, API keys, GPS coordinates, suburb names, vehicle identifiers from the recovered system are NOT to be published.** The disclosure report has been progressively scrubbed to remove all such material; do not add any of it to this repository.

### 5.2 Vulnerabilities discovered in NULLWEAR itself

If a security researcher reports a vulnerability in the firmware (per `SECURITY.md`), the vulnerability detail is held back until a coordinated disclosure date with a patch in hand. Until then it is not public.

---

## 6. Practical handling

### 6.1 `.gitignore` enforcement

The repository's `.gitignore` already excludes:

```
*.pem
!*-public.pem
!example*.pem
keys/
*.key
*.priv
secrets.env
```

This prevents accidental commit. **It is not a substitute for not having the keys in the working tree in the first place.** Always store private keys outside the project directory.

### 6.2 Pre-commit hook

Use `pre-commit` with a secrets-detection hook (e.g. `detect-secrets`, `gitleaks`) on every commit. Reject any commit containing high-entropy strings, base64 blobs of suspicious shape, or `BEGIN PRIVATE KEY` markers.

Sample `.pre-commit-config.yaml` (to be added to the repo if adopting):

```yaml
repos:
  - repo: https://github.com/gitleaks/gitleaks
    rev: v8.18.1
    hooks:
      - id: gitleaks
```

### 6.3 CI

Treat CI as untrusted with respect to secrets. CI builds the firmware unsigned; signing is performed on the controlled signing workstation. Public-side verification of CI builds is by reviewer action, not by automated trust.

### 6.4 Backup

The firmware-signing private key is backed up via Shamir's Secret Sharing (threshold 3-of-5, custodians distributed across the agency's senior security personnel). Other secrets follow agency standard practice.

### 6.5 Rotation

| Asset | Rotation cadence |
|---|---|
| Firmware-signing key | Per major release (typically annually) or on suspected compromise |
| HSM PINs / unlock material | Per personnel change in custodians |
| CM portal credentials | Per agency password policy; minimum quarterly |
| Test-rig telemetry creds | Per pilot phase change |

---

## 7. What to do if a secret is exposed

If a private key, credential, or sensitive officer-identifying data is committed or otherwise exposed publicly:

1. **Immediately** rotate the affected secret. Do not wait for confirmation of compromise — assume worst case.
2. For a firmware-signing key: revoke the public key from the bootloader allowlist (requires a firmware update flow that has the new public key already in place). All fielded units must be re-flashed with a firmware that trusts the new key.
3. Force-push removal from git history is not sufficient — assume the data is gone. Treat as if it had been read.
4. Document the incident, root-cause it, and patch the process gap that allowed the exposure.
5. Report to the agency security officer and (if applicable) to the contract manufacturer.

For asset-register exposure: the affected officer cohort needs a fresh device-serial assignment with new units. The old serials should be flagged in any future asset-register query so that "this serial corresponds to officer X" no longer resolves.

---



Cross-references:

- [`CONTACT.md`](../CONTACT.md) — secure-contact channel for cryptographic-material handoff.
- [`SECURITY.md`](../SECURITY.md) — vulnerability reporting policy.
- [`keys/README.md`](../keys/README.md) — public-key material in the repo.
- [`docs/14-security-considerations.md`](14-security-considerations.md) — the threat model NULLWEAR addresses.
- [`docs/15-legal-and-regulatory.md`](15-legal-and-regulatory.md) — regulatory pathway for deployment.
- [`docs/09-operations-manual.md`](09-operations-manual.md) §11 — operational security posture for fielded units.
