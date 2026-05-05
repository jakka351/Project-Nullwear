# Project NULLWEAR — reproducible firmware build
#
# Pinned to a known-good Zephyr CI image so every build of the same
# source produces a byte-identical artefact. Use:
#
#   make docker-build
#
# or directly:
#
#   docker build -t nullwear-build:latest -f docker/Dockerfile .
#   docker run --rm -v $(pwd):/workspace -w /workspace/firmware/nullwear-p \
#       nullwear-build:latest \
#       west build -b nrf5340dk_nrf5340_cpuapp -p always
#
# Output appears in firmware/nullwear-p/build/zephyr/merged.hex on the host.

FROM ghcr.io/zephyrproject-rtos/ci:v0.29.2

# Pinned NCS revision. Bump when project moves to a newer NCS.
ARG NCS_REVISION=v2.5-branch

USER user
WORKDIR /workdir

# Initialise an isolated NCS workspace inside the image so subsequent
# builds reuse the resolved manifest (faster).
RUN mkdir -p ncs && cd ncs && \
    west init -m https://github.com/nrfconnect/sdk-nrf --mr ${NCS_REVISION} && \
    west update --narrow -o=--depth=1 && \
    west zephyr-export

ENV ZEPHYR_BASE=/workdir/ncs/zephyr
ENV PATH="/workdir/ncs/zephyr/scripts:$PATH"

# Default workdir for builds is mounted at runtime.
WORKDIR /workspace
CMD ["bash"]
