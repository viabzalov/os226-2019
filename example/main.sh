#!/bin/bash
sort | uniq -c | sort -nr | head -1 |  sed -e 's/^ *//' | cut -d ' ' -f2
