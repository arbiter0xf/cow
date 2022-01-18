#!/bin/bash

set -ex

readonly owner="please_set_owner"

openssl genrsa -out ${owner}.key 2048
openssl req -new -x509 -days 365 -key ${owner}.key -out ${owner}.crt
