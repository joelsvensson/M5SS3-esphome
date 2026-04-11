#!/bin/bash

ESPYAML="${1:-m5ss3.yaml}"
VENVDIR="../m5ss3-dev.venv"

[ -e "${VENVDIR}" ] || python3 -m venv "${VENVDIR}"

source "${VENVDIR}/bin/activate"

which esphome || pip install esphome

