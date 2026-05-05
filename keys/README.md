# `keys/` — Public-key material only

This directory contains **public key material only**. Private keys are NEVER committed to this repository under any circumstances. See [`docs/16-secrets-and-publishing-policy.md`](../docs/16-secrets-and-publishing-policy.md) for the full policy and [`CONTACT.md`](../CONTACT.md) for the secure handoff process.

---

## What goes here

| File | Purpose | Who creates it |
|---|---|---|
| `author-public.asc` | The project author's PGP public key. Used for: verifying signed git commits and signed releases; encrypting communications to the author. | Author, on first publication |
| `release-public.pem` | The MCUboot firmware-signing public key for project-issued reference firmware. Used for: bootloader verification of project-issued binary releases. | Author, on first signed release |
| `agency-<name>-public.pem` | Per-agency MCUboot firmware-signing public key. Used for: bootloader verification of agency-built binaries. Each agency that adopts NULLWEAR generates their own keypair and only their public half is committed here. | Each adopting agency |

---

## What MUST NOT go here

- Any file ending in `.priv`, `.key`, `-private.*`, `secret*`.
- Any PEM file whose header begins `-----BEGIN PRIVATE KEY-----`, `-----BEGIN RSA PRIVATE KEY-----`, `-----BEGIN EC PRIVATE KEY-----`, etc.
- Any file containing HSM unlock material, PIN, or seed phrase.
- Any file containing API tokens, OAuth client secrets, or signing-server credentials.

The repository's `.gitignore` (top-level) is configured to ignore the most common patterns — but **mechanical exclusion is not a substitute for not having those files in the working tree in the first place.**

---

## Trust model

The project's overall trust model is described in [`docs/16-secrets-and-publishing-policy.md`](../docs/16-secrets-and-publishing-policy.md). Summary:

1. **Each agency has its own firmware-signing keypair.** A compromise in one agency does not compromise units issued by another.
2. **Private keys live in HSMs**, never on developer laptops.
3. **Multi-person ceremony** is required for any operation involving a private key.
4. **Shamir's Secret Sharing** (3-of-5) backup of the private key, custodians distributed across the agency's senior security personnel.
5. **Per-release rotation** so any unknown long-term compromise has a bounded blast radius.

---

## Verifying a signature

For any signed artefact in this repository:

```bash
# Git commits (after importing the author's public key)
git log --show-signature

# Signed release artefact (e.g. nullwear-p-v1.0.0.hex.sig)
gpg --verify nullwear-p-v1.0.0.hex.sig nullwear-p-v1.0.0.hex

# Firmware image, if released signed via cosign / sigstore
cosign verify-blob \
  --certificate nullwear-p-v1.0.0.hex.cert \
  --signature nullwear-p-v1.0.0.hex.sig \
  nullwear-p-v1.0.0.hex
```

The fingerprint of the author's PGP key is published in [`CONTACT.md`](../CONTACT.md); verify it through at least one out-of-band channel before trusting anything signed by it.

---

## Generating an agency keypair

If your agency is adopting NULLWEAR and you need to generate your own firmware-signing keypair:

```bash
# Generate inside the HSM (example for SoftHSM/PKCS#11 — adapt to your HSM model)
imgtool keygen --key release.pem --type ed25519
imgtool getpub --key release.pem > agency-<name>-public.pem

# Verify the keypair
imgtool sign --key release.pem --header-size 0x200 --align 4 \
  --version 1.0.0 --slot-size 0xE0000 \
  test-image.bin test-image-signed.bin

# Commit ONLY the public key
git add keys/agency-<name>-public.pem
# DO NOT git add release.pem — it contains the private half
```

Hand-off the public key half to the project maintainer per [`CONTACT.md`](../CONTACT.md) §3 if you would like it referenced in upstream documentation.

---

## Placeholder files in this directory

The placeholder files committed here are templates and are NOT operational keys. They have the correct file structure so reviewers can see what should be present, but they cannot be used to verify anything real. They will be replaced with real public keys when the project enters operational use.
