<p align="center">
  <img src="docs/img/logo.png" alt="Project NULLWEAR" width="800"/>
</p>

# Contributing to Project NULLWEAR

This project exists to protect the safety of law-enforcement officers from a real, demonstrated radio-frequency surveillance threat. Contributions are welcome from engineers, security researchers, manufacturing partners, regulators, agency staff, and anyone with relevant expertise.

## Before you contribute

Read the documentation. In particular:

- [`README.md`](README.md) — project overview and validation status.
- [`docs/01-overview.md`](docs/01-overview.md) — what the project is and is not.
- [`docs/14-security-considerations.md`](docs/14-security-considerations.md) — threat model.
- [`SECURITY.md`](SECURITY.md) — vulnerability disclosure policy.

If your contribution is a security-relevant finding, follow [`SECURITY.md`](SECURITY.md) first. Do not open a public issue.

## Types of contribution welcomed

- **Firmware bug fixes.** Especially around radio timing, OUI matching, BLE PHY edge cases, power management.
- **Build-system improvements.** NCS workspace setup, cross-platform toolchain hints, CI integration.
- **Test plans and test results.** Empirical lab and field measurements. We need data to replace assertions.
- **Hardware design improvements.** PCB layout, antenna tuning, power-supply optimisation.
- **Mechanical design.** Enclosure variants, alternative belt clips, vehicle mount kits.
- **Documentation.** Translations, clarifications, corrections.
- **Reference receiver / test source enhancements.** Better verification tooling.
- **Regulatory pathway research** for jurisdictions outside Australia.
- **Manufacturing process feedback** from contract manufacturers who have actually built units.

## Types of contribution that need maintainer review

- Adding additional target OUIs (e.g. for other vendors' equipment with similar characteristics).
- Changes to the cryptographic signing model.
- Changes to the bootloader.
- Changes to the regulatory documentation (`docs/15-legal-and-regulatory.md`) — these need legal review, not just engineering review.

## How to submit

1. Fork the repository.
2. Create a feature branch named after the change (e.g. `fix/radio-timing-edge-case`, `docs/build-instructions-windows-clarification`).
3. Make your changes. Keep commits focused; one logical change per commit.
4. Update tests where applicable.
5. Update documentation where applicable.
6. Submit a pull request.
7. The maintainer will review. Allow up to 14 days.

## Commit message conventions

Use the [Conventional Commits](https://www.conventionalcommits.org/) format:

```
type(scope): subject

body

footer
```

Examples:

```
fix(radio_jammer): correct BCC value for 64-bit OUI detection
docs(user-manual): clarify LED meaning during charge
feat(test-source): add multi-MAC mode for 5-officer scenario testing
chore(build): bump NCS minimum version to v2.5.2
```

## Code style

### C (firmware)

- Follow the existing style in `firmware/nullwear-p/`. Roughly:
  - 4-space indent, no tabs.
  - Opening brace on the same line for functions and control flow.
  - `snake_case` for functions and variables.
  - `UPPER_SNAKE_CASE` for constants.
  - Comments above non-trivial blocks explaining *why*, not *what*.
- Use the `nrfx` HAL for peripheral access, not the legacy nrf_drv layer.
- Annotate ISRs clearly (`/* runs in interrupt context */`).
- No dynamic allocation in interrupt context.

### Python (tooling)

- PEP 8.
- Black-formatted (`black -l 100`).
- Type hints where useful.
- Docstrings on public functions.

### Markdown (documentation)

- Use the existing voice — direct, technically grounded, occasionally informal.
- Section headings consistent with the existing numbering.
- Cross-link between docs liberally.

## Validation status discipline

This is important. Every claim in a NULLWEAR document falls into one of:

- **Verified** — empirically demonstrated or sourced from a primary specification.
- **Awaiting verification** — engineering target or design intent, not yet measured.
- **Author's interpretation** — opinion, must be reviewed by an appropriate expert.

When you contribute new documentation, label your claims accordingly. If you change something from "Awaiting verification" to "Verified", include the test report that justifies the change.

## Sign-off

By submitting a contribution, you agree that:

- The contribution is your original work, or you have the right to contribute it.
- It is licensed under the project's MIT licence.
- You understand the operational sensitivity of the project (it protects serving officers).

## Maintainer

**Benjamin Jack Leighton** — Tester Present Specialist Automotive Solutions.

For security-relevant contact: see [`SECURITY.md`](SECURITY.md). For everything else: GitHub issues / pull requests.
