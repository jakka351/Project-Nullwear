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

**Public key:**

```
-----BEGIN PGP PUBLIC KEY BLOCK-----

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
hEPgGUQTZffYRE0=
=DmKk
-----END PGP PUBLIC KEY BLOCK-----

```

**Email address (placeholder — to be replaced):**

```
contact@testerpresent.com.au
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
