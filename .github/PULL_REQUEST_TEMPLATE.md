<!--
Thanks for contributing to Project NULLWEAR.

This project protects the safety of serving law-enforcement officers.
Please be careful, especially with anything that touches:
  - The radio jammer code (firmware/nullwear-p/network_core/src/radio_jammer.c)
  - The OUI matcher logic
  - The MCUboot signing chain
  - The acceptance / field test procedures
  - The published validation status

Read CONTRIBUTING.md before opening this PR.
-->

## Summary

<!-- One or two sentences describing the change. -->

## Motivation

<!-- Why this change? What problem does it solve? -->

## Type of change

- [ ] Bug fix (non-breaking)
- [ ] New feature (non-breaking)
- [ ] Breaking change (requires firmware revision bump)
- [ ] Documentation only
- [ ] Refactor / cleanup
- [ ] CI / build / tooling

## Scope

- [ ] Firmware — network core
- [ ] Firmware — application core
- [ ] Verification tooling
- [ ] Hardware specification
- [ ] Documentation
- [ ] Operational procedures
- [ ] Other (describe)

## Validation

<!-- Describe how this change was verified. Field-test results, lab measurements,
     ATP runs, log captures, etc. If unverified, say so explicitly and update
     the Validation Status table in README.md accordingly. -->

- [ ] Code compiles cleanly under NCS v2.5+
- [ ] Existing field-test procedure still passes
- [ ] If touching the radio jammer: lab acceptance (`docs/12-acceptance-test-procedure.md`) re-run with PAR ≥ 0.99
- [ ] If touching the OUI matcher: selective-isolation test (ATP T5) confirms non-target traffic still flows
- [ ] If touching documentation: cross-references checked
- [ ] If introducing new claims: Validation Status updated honestly (Verified / Awaiting verification / Author's interpretation)

## Security

- [ ] This PR does NOT add any private keys, credentials, tokens, asset-register data, or officer-identifying information
- [ ] If touching the signing chain: the change has been reviewed against `docs/16-secrets-and-publishing-policy.md`
- [ ] If touching authentication / authorisation: a security advisory has been considered
- [ ] If introducing a new external dependency: the dependency has been reviewed for supply-chain risk

## Linked issues

<!-- Closes #N, fixes #N, related to #N. -->

## Reviewer notes

<!-- Anything reviewers should know. Areas of uncertainty. Trade-offs you made. -->

## Sign-off

By submitting this pull request:

- I agree the contribution is licensed under the MIT licence (see LICENSE).
- I have read CONTRIBUTING.md.
- I understand the operational sensitivity of NULLWEAR.
