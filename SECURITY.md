# Security Policy

## Reporting a vulnerability

Project NULLWEAR is a security-relevant device. If you discover a vulnerability or weakness — in the firmware, in the hardware design, in the documented procedures, in the regulatory analysis, or in the supply-chain assumptions — please report it responsibly.

### How to report

1. **Do not** open a public GitHub issue containing the details of the vulnerability.
2. Open a private security advisory on GitHub (Security tab → Advisories → New draft security advisory), or
3. Contact the maintainer via PGP-encrypted email — see [`CONTACT.md`](CONTACT.md) for the public key fingerprint and the verification process, or
4. If you are an officer of an authorised disclosure recipient, contact your agency's NULLWEAR programme liaison.

Use the subject line: `NULLWEAR — security report`.

### What to include

- A clear description of the vulnerability.
- The conditions under which it manifests.
- The potential impact (e.g. "unit fails to annihilate target packets in scenario X" / "device can be made to over-jam non-target traffic in scenario Y" / "supply-chain compromise vector at stage Z").
- Suggested mitigation if you have one.
- Your name and (optionally) affiliation, for credit. Anonymous reports are accepted.

### What you can expect from us

- Acknowledgement within 5 business days.
- Initial assessment and severity classification within 14 days.
- A coordinated disclosure plan, if the issue is confirmed, within 30 days.
- Credit in the patched release notes (with your consent), unless you prefer anonymity.

### What not to do

- **Do not test on live operational deployments.** If you wish to test in the field, request a controlled-test environment from the agency programme liaison.
- **Do not disclose publicly until coordinated.** This is a defensive capability used by serving officers; uncoordinated disclosure can expose them.
- **Do not modify deployed units.** Field modification can cause an officer to wear what they believe is a working device while it is not.

## Threat model

NULLWEAR's threat model is set out in detail in [`docs/14-security-considerations.md`](docs/14-security-considerations.md). Summary:

- **In scope:** passive third-party detection of officers via the BLE-OUI signature of their issued Axon equipment.
- **Out of scope:** active attacks on Axon equipment, agency network compromise, visual identification of officers, ALPR-based tracking, cellular triangulation, supply-chain attacks against components other than the NULLWEAR unit itself.

## Scope of acceptable testing

The following is acceptable:

- Bench testing of NULLWEAR units against the ESP32 emulator (`firmware/tools/test-source/`).
- Reference-receiver verification on your own equipment.
- Source-code review.
- Hardware reverse-engineering of an obtained NULLWEAR unit.
- Discovery testing in jurisdictions where you have appropriate radio licences.

The following is not acceptable:

- Testing against units issued to serving officers without authorisation.
- Operating against real Axon equipment outside controlled environments.
- Broadcasting `00:25:DF` packets in public spaces without an experimental radio licence.
- Any activity that would interfere with active law-enforcement operations.

## Cryptographic key handling

If your report concerns the firmware-signing key:

- The signing key compromises the entire fleet's update channel.
- Treat reports involving signing keys as **CRITICAL** and use the agency programme liaison channel directly, not a GitHub draft advisory.

## Coordinated disclosure timeline

Default disclosure timeline:

- **Days 0–14:** triage and confirmation.
- **Days 14–60:** mitigation development and review.
- **Day 60:** coordinated public disclosure with patch.
- **Special cases:** for vulnerabilities affecting fielded units in active operational use, the maintainer reserves the right to extend the timeline by up to 90 days, with the reporter's agreement.

## Hall of fame

A list of researchers who have responsibly disclosed will be maintained here once the project enters operational use.

---

This Security Policy is adapted from common open-source-project SECURITY.md conventions, adjusted for the operational sensitivity of a defensive capability used by sworn officers.
