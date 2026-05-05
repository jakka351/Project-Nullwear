#
# Project NULLWEAR — top-level Makefile (convenience entry points)
#
# This is NOT the build system. The build system is Zephyr / west /
# CMake / NCS. This Makefile is a thin wrapper that documents and
# automates the most common workflows for new contributors.
#

.DEFAULT_GOAL := help

NCS_BOARD ?= nrf5340dk_nrf5340_cpuapp
FW_DIR    := firmware/nullwear-p
BUILD_DIR := $(FW_DIR)/build

.PHONY: help
help:               ## Show this help.
	@echo "Project NULLWEAR — convenience targets"
	@echo ""
	@echo "Firmware:"
	@grep -E '^(build|flash|attach|clean):.*?##' $(MAKEFILE_LIST) | awk 'BEGIN{FS=":.*?## "}{printf "  %-22s %s\n", $$1, $$2}'
	@echo ""
	@echo "Verification tools:"
	@grep -E '^(receiver|test-source|atp):.*?##' $(MAKEFILE_LIST) | awk 'BEGIN{FS=":.*?## "}{printf "  %-22s %s\n", $$1, $$2}'
	@echo ""
	@echo "Quality:"
	@grep -E '^(lint|format|secrets-scan|test-python):.*?##' $(MAKEFILE_LIST) | awk 'BEGIN{FS=":.*?## "}{printf "  %-22s %s\n", $$1, $$2}'
	@echo ""
	@echo "Diagrams / docs:"
	@grep -E '^(diagrams|logo|docs-check):.*?##' $(MAKEFILE_LIST) | awk 'BEGIN{FS=":.*?## "}{printf "  %-22s %s\n", $$1, $$2}'
	@echo ""
	@echo "Reproducible build via Docker:"
	@grep -E '^(docker-build|docker-shell):.*?##' $(MAKEFILE_LIST) | awk 'BEGIN{FS=":.*?## "}{printf "  %-22s %s\n", $$1, $$2}'

# ---------- Firmware ----------

.PHONY: build
build:              ## Build the firmware (assumes NCS environment is sourced).
	cd $(FW_DIR) && west build -b $(NCS_BOARD) -p always

.PHONY: flash
flash:              ## Flash the most recent build via J-Link.
	cd $(FW_DIR) && west flash --erase

.PHONY: attach
attach:             ## Attach to the device via J-Link RTT for log capture.
	cd $(FW_DIR) && west attach

.PHONY: clean
clean:              ## Remove the firmware build directory.
	rm -rf $(BUILD_DIR)

# ---------- Verification tools ----------

.PHONY: receiver
receiver:           ## Run the reference receiver for 60 s.
	python firmware/tools/reference-receiver/ref_receiver.py --duration 60

.PHONY: test-source
test-source:        ## Print instructions for flashing the ESP32 emulator.
	@echo "ESP32 emulator firmware: firmware/tools/test-source/axon_emulator.ino"
	@echo "See firmware/tools/test-source/README.md for build/flash instructions."

.PHONY: atp
atp:                ## Run the automated ATP harness on a connected DUT.
	@echo "Usage: make atp SERIAL=<sn> FW=<rev> RTT_LOG=<path> OUT=<report.json>"
	python firmware/tools/atp/run_atp.py \
		--serial $(SERIAL) \
		--firmware-revision $(FW) \
		--rtt-log $(RTT_LOG) \
		--out $(OUT) \
		--skip-manual

# ---------- Quality ----------

.PHONY: lint
lint:               ## Lint Python code with ruff and check formatting with black.
	black --check firmware/tools/
	ruff check firmware/tools/

.PHONY: format
format:             ## Auto-format Python code with black + ruff --fix.
	black firmware/tools/
	ruff check --fix firmware/tools/

.PHONY: secrets-scan
secrets-scan:       ## Scan working tree for accidental secrets (gitleaks).
	gitleaks detect --source . --no-git --verbose

.PHONY: test-python
test-python:        ## Smoke-test Python tools (--help works, schema parses).
	python firmware/tools/reference-receiver/ref_receiver.py --help > /dev/null
	python firmware/tools/atp/run_atp.py --help > /dev/null
	python -c "import json; json.load(open('firmware/tools/atp/atp_schema.json'))"
	@echo "Python tools OK."

# ---------- Docs ----------

.PHONY: diagrams
diagrams:           ## Regenerate all SVG diagrams from their Python sources.
	cd docs/img && python _make_logo.py && python _make_diagrams.py

.PHONY: logo
logo:               ## Regenerate just the logo SVG.
	cd docs/img && python _make_logo.py

.PHONY: docs-check
docs-check:         ## Markdown link check (requires markdown-link-check installed).
	npx --yes markdown-link-check docs/*.md README.md SECURITY.md CONTRIBUTING.md CONTACT.md

# ---------- Docker (reproducible build) ----------

.PHONY: docker-build
docker-build:       ## Build the firmware in a reproducible Docker container.
	docker build -t nullwear-build:latest -f docker/Dockerfile .
	docker run --rm -v $(PWD):/workspace -w /workspace/firmware/nullwear-p \
		nullwear-build:latest \
		west build -b $(NCS_BOARD) -p always

.PHONY: docker-shell
docker-shell:       ## Drop into the build container interactively.
	docker run --rm -it -v $(PWD):/workspace -w /workspace nullwear-build:latest /bin/bash
