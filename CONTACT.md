# Secure Contact

How to reach the author for matters that should not go through the public GitHub issue tracker — in particular, distribution of cryptographic material (signing keys, code-signing certificates) to law-enforcement agencies and authorised contract manufacturers.

---

## Author

**Benjamin Jack Leighton**
Tester Present Specialist Automotive Solutions

---

## When to use which channel

| Purpose | Channel |
|---|---|
| General questions, feature requests, documentation issues | Public GitHub Issues |
| Security vulnerability in NULLWEAR | Private GitHub Security Advisory (preferred) — see [`SECURITY.md`](SECURITY.md) |
| **Cryptographic material handoff** (firmware-signing keys for an agency or CM) | **PGP-encrypted email**, after identity verification (see §3 below) |
| Confidential operational matter from a serving agency | Your agency's existing liaison channel to the author, or PGP-encrypted email |
| Press / media | PGP-encrypted email; verbal-only calls after identity verification |

---

## 1. PGP / GnuPG (preferred for any sensitive matter)

Author's PGP public key is published at:

- This repository: [`keys/author-public.asc`](keys/author-public.asc) — fingerprint must be verified out-of-band before use.
- Public keyservers: search for the fingerprint below.

**Public key fingerprint (placeholder — to be replaced with the author's real fingerprint at first publication):**

```
TBD  TBD  TBD  TBD  TBD  TBD  TBD  TBD  TBD  TBD
```

**Email address (placeholder — to be replaced):**

```
nullwear-secure@<author-domain>
```

The fingerprint must be verified through at least one trusted out-of-band channel before any sensitive material is exchanged. Acceptable verification channels:

- A signed message from the author through this repository (commits signed with the same key).
- A signed message from the author at the conference / forum / agency briefing where you met.
- A short phone call to a number obtained independently of the email.

**If a key fingerprint anywhere claims to be the author's but does not match the value above and the fingerprint published with this repository's git-signed commits, treat it as an impostor.**

## 2. Other end-to-end-encrypted channels (acceptable for routine communications)

After PGP-verified identity exchange, follow-on conversation can move to any of:

- **Signal** (Open Whisper Systems) — preferred messenger; safety numbers must be verified.
- **Wire** — acceptable; verify safety numbers.
- **ProtonMail / Tutanota** — acceptable for email; native E2E if both parties use the platform.

**Channels NOT acceptable for sensitive material:**
- Plain email (SMTP without PGP).
- Telegram (server-side keys; not E2E by default).
- WhatsApp (E2E but Meta-controlled metadata; treat as out-of-scope for cryptographic-material distribution).
- Slack, Discord, Microsoft Teams, or any other workplace-collaboration platform without explicit E2E channel agreed in advance.
- Voice-call audio of cryptographic material (do not read keys aloud).

## 3. Identity verification before key handoff

Before any firmware-signing key, code-signing certificate, or equivalent cryptographic credential is shared:

### For an agency

1. Initial contact via agency's existing liaison to the author, or via PGP-encrypted email.
2. The author requests:
   - A formal request on agency letterhead, signed by an officer at superintendent rank or above (or equivalent), naming the specific programme NULLWEAR is being deployed under.
   - The PGP public key of the agency-authorised key custodian who will receive the material.
3. The author cross-checks the agency contact via independent channel (call the agency's published switchboard, ask for the named officer, confirm the request).
4. Once confirmed, the cryptographic material is transferred either:
   - PGP-encrypted to the agency custodian's public key, or
   - Hand-delivered on a hardware token (YubiKey, Feitian, similar) by a courier with chain-of-custody documentation.

### For a contract manufacturer

1. Initial contact via the agency procurement office, copying the author.
2. The CM provides:
   - Their PGP public key.
   - Confirmation of the receiving custodian (named individual + role).
   - Confirmation of how the key will be stored (HSM model, access control).
3. The author cross-checks the CM via the agency procurement office.
4. Material is transferred PGP-encrypted to the named custodian.

### For an independent security researcher (vulnerability reports, NOT key handoff)

Researchers are not given keys. Vulnerability reports follow [`SECURITY.md`](SECURITY.md). Keys are not in scope for researcher handoff.

## 4. What is in scope for secure transfer

| Item | Distribution scope |
|---|---|
| Firmware-signing private key (per-agency) | Receiving agency only, via §3 process |
| Code-signing certificate (CI side) | Receiving agency, possibly CM, via §3 process |
| HSM unlock material | Receiving agency only |
| Asset register schema (no real data) | Anyone — public artefact |
| Asset register data (real serial-to-officer mapping) | Receiving agency only; not author-held |
| Pilot test source MAC patterns linking to real officers | Receiving agency only; sanitised for any public release |
| Detailed regulatory determination from ACMA (when issued) | Receiving agency to manage publication |

## 5. After-key-handoff hygiene

Once cryptographic material has been handed off:

- The author retains no copy of the agency's private key.
- The author retains the public key only, in [`keys/`](keys/), so signed artefacts can be verified.
- If the agency wishes to rotate its key (per §16 of the docs), the rotation procedure is the same as the initial handoff.
- The agency is responsible for revocation in event of suspected compromise.

## 6. If you suspect impersonation

If you receive any communication claiming to be from the author or from the project that:
- Asks for credentials or keys to flow toward the sender,
- Provides a key fingerprint different from the one in this repository,
- Pressures you to bypass the §3 verification process,

then it is an impostor. Confirm via an independent channel (this repository, a known phone number, an in-person meeting). Report attempted impersonation as a security advisory per [`SECURITY.md`](SECURITY.md).

---

## TL;DR

- General matters → public GitHub Issues.
- Vulnerabilities → private GitHub Security Advisory.
- **Cryptographic material → PGP-encrypted email after fingerprint verification, then optionally Signal for follow-up. Identity is verified independently before any key crosses the wire.**

The cryptographic posture is conservative on purpose. The signing key controls every fielded NULLWEAR unit. Treat its distribution with the same care you would treat the master key to your own building.
