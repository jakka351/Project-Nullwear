# Before You Publish

A pre-flight checklist of every placeholder in this repository that must be replaced with real values before public release. Doing this list saves an embarrassing post-publish patch commit.

---

## 1. GitHub owner placeholders

`REPLACE-ME-OWNER` appears in the following files. Find/replace globally with your actual GitHub username or organisation.

```
.github/CODEOWNERS
.github/ISSUE_TEMPLATE/config.yml
.github/mlc.json
firmware/tools/atp/atp_schema.json
```

Quick command from the repo root:

```bash
grep -rl "REPLACE-ME-OWNER" . | xargs sed -i 's/REPLACE-ME-OWNER/YOUR-GH-USERNAME/g'
```

(macOS: use `sed -i ''` with an empty string after `-i`.)

## 2. PGP key + fingerprint

The author's PGP material in [`CONTACT.md`](CONTACT.md) and [`keys/author-public.asc`](keys/author-public.asc) is placeholder.

- Generate (or use existing) PGP keypair.
- Export the public key in ASCII-armoured form: `gpg --armor --export YOUR_KEY_ID > keys/author-public.asc`
- Replace the file content with the real key.
- Update `CONTACT.md`:
  - Replace the `TBD TBD TBD ...` fingerprint line with the real fingerprint (`gpg --fingerprint YOUR_KEY_ID`).
  - Replace the placeholder email `nullwear-secure@<author-domain>` with your real address.

## 3. Firmware-signing public key (per agency)

[`keys/example-release-public.pem`](keys/example-release-public.pem) is a placeholder. For each adopting agency:

- Generate keypair in HSM (see `keys/README.md` for the `imgtool` recipe).
- Export the public half: commit as `keys/agency-<name>-public.pem`.
- Keep the private half in the HSM. Never commit.

For the project's own reference firmware (used during pilot only), generate `keys/release-public.pem` and commit it — but the corresponding private key still lives only in your signing-server HSM.

## 4. Repository description / topics on GitHub

When you create the GitHub repo:

- **Description:** suggested — *"Open-source mitigation for the Axon Bluetooth (BLE OUI 00:25:DF) tracking vulnerability. Per-officer reactive radio jammer. Reference firmware, hardware spec, deployment plan."*
- **Topics:** suggested — `bluetooth`, `ble`, `nrf5340`, `axon`, `law-enforcement`, `officer-safety`, `radio`, `embedded`, `zephyr`, `responsible-disclosure`.
- **Website:** if a project website is later created, link it.

## 5. Repository settings

Recommended GitHub repo settings before opening to the public:

- **General → Default branch:** `main`
- **General → Disable wiki** (use docs/ instead — single source of truth).
- **General → Disable projects** unless you intend to use them.
- **Branch protection on `main`:**
  - Require pull request before merging.
  - Require at least 1 review from CODEOWNERS.
  - Require status checks: `lint-docs`, `lint-python`, `secrets-scan`, `build-firmware`, `python-tests`.
  - Require linear history.
  - Require signed commits.
- **Code security:**
  - Enable Private Vulnerability Reporting (Settings → Code security and analysis).
  - Enable Dependabot security updates.
  - Enable Dependabot version updates (the `.github/dependabot.yml` config is already in the repo).
  - Enable secret scanning + push protection.
- **Actions:**
  - Restrict to selected actions and reusable workflows.
  - Do not allow CI to push back to the repo.

## 6. First release tag

Once everything is in place:

```bash
# Sign the tag with your PGP key
git tag -s v0.0.1-rc1 -m "Initial release candidate — reference implementation"
git push origin v0.0.1-rc1
```

Update [`CHANGELOG.md`](CHANGELOG.md) to reflect the release date.

## 7. Optional but recommended

- Mirror the repo to a backup git host (Codeberg, GitLab, your own Forgejo).
- Pin the maintainer's GitHub profile to this repo.
- Tweet/post about the release with a link to the strategic Mitigation Report PDF as the entry point for non-technical readers.
- Notify the disclosure recipients listed in the Mitigation Report §10 of the public release date.

---

## Summary

Estimated time for all placeholder replacements: **15 minutes**.

The longest single step is generating the PGP key if you don't already have one. Everything else is mechanical find-and-replace.

When this checklist is done, publish.
