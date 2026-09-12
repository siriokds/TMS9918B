#!/usr/bin/env python3
"""Keeps the README index consistent with the lessons in this directory.

Checks that every ROM source has a row in the lesson index and a section under
"Lessons", that both are in numerical order, and that no entry refers to a
source that does not exist. Run it before committing a new lesson.
"""

import glob
import os
import re
import sys

os.chdir(os.path.dirname(os.path.abspath(__file__)))
readme = open('README.md').read()
problems = []

sources = sorted(os.path.basename(p)[:-4] for p in glob.glob('[0-9][0-9][0-9]_*.asm'))
numbers = [int(name[:3]) for name in sources]

index = re.findall(r'^\| (\d{3}) \| `([^`]+)` \|', readme, re.M)
indexed = [int(n) for n, _ in index]
if indexed != sorted(indexed):
    problems.append('the lesson index is not in numerical order')
for number, program in index:
    if program not in sources:
        problems.append('the index lists %s, which has no source' % program)
    if program[:3] != number:
        problems.append('index row %s names %s' % (number, program))

sections = [int(n) for n in re.findall(r'^### (\d{3})', readme, re.M)]
if sections != sorted(sections):
    problems.append('the lesson sections are not in numerical order')

described = set()
for heading in re.findall(r'^### ([\d, and]+) —', readme, re.M):
    described.update(int(n) for n in re.findall(r'\d{3}', heading))

for name, number in zip(sources, numbers):
    if number not in indexed:
        problems.append('%s is missing from the lesson index' % name)
    if number not in described:
        problems.append('%s has no section under Lessons' % name)

for line in problems:
    print('README:', line)
print('%d lessons, %d problems' % (len(sources), len(problems)))
sys.exit(1 if problems else 0)
