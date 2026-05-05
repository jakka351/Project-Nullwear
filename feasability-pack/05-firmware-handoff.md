# Firmware Handoff — What the CM Needs from the Agency

The Agency holds the MCUboot signing private key in an HSM. **The Manufacturer never receives the private key.** What the Manufacturer receives:

## At contract execution

| Item | From | Via | Format |
|---|---|---|---|
| Agency's MCUboot signing public key | Agency security officer | PGP-encrypted email per `CONTACT.md` §3 | `.pem` file |
| Initial signed firmware image (release artefact) | Agency engineering | PGP-encrypted email or signed git release | `.hex` (and `.sha256`) |
| Asset-management endpoint URL + auth credentials | Agency IT | PGP-encrypted email | URL + bearer token |
| Per-unit serial-number range | Agency procurement | Plain document | Spreadsheet or range definition |

## Per-firmware-revision

When the Agency updates the firmware (typically annually):

| Item | From | Via | Format |
|---|---|---|---|
| New signed firmware image | Agency engineering | Same channel | `.hex` + `.sha256` |
| Release notes | Agency engineering | Same | Markdown |
| New ATP harness if changed | Agency engineering | Same | Python source |

## What the Manufacturer does

1. **Verify** the received signed image's SHA-256 matches the published value.
2. **Verify** the image's MCUboot signature against the Agency public key (use `imgtool verify --key agency-public.pem image.hex`).
3. **Flash** every unit with the verified image.
4. **Run** the ATP T9 firmware-integrity test, which re-confirms the SHA-256 matches.
5. **Log** the firmware revision in the per-unit ATP report.
6. **Archive** the public key, the signed image, and the SHA-256 manifest for the contract retention period.

## What the Manufacturer must NOT do

- Hold or back up the private key (never receives it).
- Modify the firmware source and re-sign with their own key.
- Flash any firmware not signed by the Agency's key.
- Ship a unit whose ATP T9 hash does not match the expected value.

## Cyber-hygiene expectations

The Manufacturer's firmware-flashing workstation should:

- Be air-gapped or on a network segment isolated from the corporate LAN.
- Have only the flashing software installed (J-Link, nrfjprog, the agency-supplied verification scripts).
- Be physically secured against unauthorised access.
- Not run general-purpose user accounts (login restricted to operators on the production line).

The Agency may audit the Manufacturer's firmware-flashing workflow as part of pre-award due diligence.

## Reference

- Bootloader specification: MCUboot v1.x with Ed25519 signature
- Verification tool: `imgtool` (Zephyr's reference signing tool)
- Public-key file: `keys/agency-<name>-public.pem` (Agency-supplied)
- Build environment: nRF Connect SDK v2.5+
- Toolchain: GNU ARM Embedded 12.x

For the firmware build itself, see `docs/07-build-instructions.md`. The Agency may produce the build artefact in-house and ship the signed `.hex` to the CM, OR the CM may build the firmware themselves from the open-source repository and submit unsigned binaries to the Agency for signing — agreed at contract execution.
