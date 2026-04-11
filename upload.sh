#!/bin/bash

source  ./activate.sh
#esphome run "${ESPYAML}"
esphome compile "${ESPYAML}" && esphome upload "${ESPYAML}"


