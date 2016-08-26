#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Author: sverx
# Version: 1.0.2

from __future__ import absolute_import, division, generators, unicode_literals, print_function, nested_scopes
import sys
import os.path


class Asset:
    def __init__(self, name, size):
        self.name = name
        self.size = size
        self.style = 0

    def set_style(self, style):
        self.style = style


class AssetGroup:
    def __init__(self):
        self.assets = []
        self.size = 0

    def add_asset(self, asset):
        self.assets.append(asset)
        self.size += asset.size


class Bank:
    def __init__(self, size):
        self.assetgroups = []
        self.free = size

    def add_assetgroup(self, assetgroup):
        self.assetgroups.append(assetgroup)
        self.free -= assetgroup.size


def fix_name(name):
    return name.replace('(', '_').replace(')', '_').replace(' ', '_').replace('.', '_')


def find(fun, seq):
    for item in seq:
        if fun(item):
            return item

AssetGroupList = []    # list of the groups (we will sort this)
AssetList = []         # list of the assets (for inspection)
BankList = []          # list of the banks

print("*** sverx's assets2banks converter ***")

if 2 <= len(sys.argv) <= 3:
    assets_path = sys.argv[1]
    first_bank = 2                                              # first bank will be number 2
else:
    print("Usage: assets2banks path [--bank1size=<size>]")
    sys.exit(1)

if len(sys.argv) == 3:
    if sys.argv[2][:12] == "--bank1size=":
        try:
            b = Bank(int("+{0!s}".format(sys.argv[2][12:])))
            BankList.append(b)
            first_bank = 1                                          # first bank should be number 1
            print("Info: bank1 size set at {0!s} bytes".format(b.free))
        except ValueError:
            print("Fatal: invalid bank1 size parameter")
            sys.exit(1)
    else:
        print("Usage: assets2banks path [--bank1size=<size>]")
        sys.exit(1)

# read cfg file (if present) and create assets and assets group accordingly
in_a_group = False
try:
    config_file = open(os.path.join(assets_path, "assets2banks.cfg"), "r").readlines()
    for l in config_file:
        if len(l.strip()) == 0:                                 # if line is empty or made of just spaces, skip it
            pass
        elif l[0] == "#":                                       # if line starts with #, it is a comment, we can skip it
            pass
        elif l[0] == "{":                                       # if line starts with {, it means we start a group
            ag = AssetGroup()
            AssetGroupList.append(ag)
            in_a_group = True
        elif l[0] == "}":                                       # if line starts with }, it means we close a group
            in_a_group = False
        elif l[0] == ":":                                       # if line starts with :, it means we have an attribute
            if l.strip('\n') == ":format unsigned int":
                a.set_style(1)
        else:                                                   # else it's an asset
            st = os.stat(os.path.join(assets_path, str(l.strip('\n'))))
            a = Asset(str(l.strip('\n')), st.st_size)
            AssetList.append(a)
            if not in_a_group:
                ag = AssetGroup()
                AssetGroupList.append(ag)
            ag.add_asset(a)
except OSError:
    pass

try:
    st = os.stat(os.path.join(assets_path, "assets2banks.cfg"))  # fake asset, to skip config file, if present in folder
    a = Asset("assets2banks.cfg", st.st_size)
    AssetList.append(a)
except OSError:
    pass

# for a in AssetList:
#    print (a.name, a.size, len(a.name))

try:
    for f in os.listdir(assets_path):  # read directory content and create missing assets and assetgroup (one per asset)
        if os.path.isfile(os.path.join(assets_path, f)):
            st = os.stat(os.path.join(assets_path, f))
            a = Asset(str(f), st.st_size)
            if not find(lambda asset: asset.name == str(f), AssetList):
                # print (a.name, a.size, "not in list!")
                AssetList.append(a)
                ag = AssetGroup()
                ag.add_asset(a)
                AssetGroupList.append(ag)
#        else:
#            print (a.name, a.size, "already in list!")
except OSError:
    print("Fatal: can't access '{0}' folder".format(assets_path))
    sys.exit(1)

AssetGroupList.sort(key=lambda g: g.size, reverse=True)    # sort the AssetGroupList by size, descending

for ag in AssetGroupList:                                  # now find a place for each assetgroup
    for b in BankList:
        if b.free >= ag.size:                              # we've found a place for this assetgroup
            b.add_assetgroup(ag)
            break
    else:                                                # we've found no place for this assetgroup, allocate a new bank
        if ag.size <= 16384:
            b = Bank(16384)
            b.add_assetgroup(ag)
            BankList.append(b)
        else:
            print("Fatal: asset/assetgroup too big to fit ({0!s} bytes are needed)".format(ag.size))
            sys.exit(1)

for bank_n, b in enumerate(BankList):
    out_file_h = open("bank{0!s}.h".format(bank_n + first_bank), "w")
    out_file_c = open("bank{0!s}.c".format(bank_n + first_bank), "w")
    for ag in b.assetgroups:
        for a in ag.assets:
            in_file = open(os.path.join(assets_path, a.name), "rb")
            if a.style == 0:
                out_file_c.write("const unsigned char {0}[{1!s}]={{\n".format(fix_name(a.name), a.size))
                out_file_h.write("extern const unsigned char\t{0}[{1!s}];\n".format(fix_name(a.name), a.size))
            else:
                out_file_c.write("const unsigned int {0}[{1!s}]={{\n".format(fix_name(a.name), a.size // 2))
                out_file_h.write("extern const unsigned int\t{0}[{1!s}];\n".format(fix_name(a.name), a.size // 2))

            out_file_h.write("#define\t\t\t\t{0}_size {1!s}\n".format(fix_name(a.name), a.size))
            out_file_h.write("#define\t\t\t\t{0}_bank {1!s}\n".format(fix_name(a.name), bank_n + first_bank))

            if a.style == 0:
                cnt = 0
                byte = in_file.read(1)
                while byte:
                    out_file_c.write("0x{0}".format(format(ord(byte), '02x')))
                    cnt += 1
                    byte = in_file.read(1)
                    if byte:
                        out_file_c.write(",")
                    if byte and cnt == 16:
                        out_file_c.write("\n")
                        cnt = 0
            else:
                cnt = 0
                word = in_file.read(2)
                while word:
                    if len(word) == 2:
                        out_file_c.write("0x{0}{1}".format(format(word[1], '02x'), format(word[0], '02x')))
                    else:
                        out_file_c.write("0x{0}".format(format(word[0], '02x')))
                        # a lone byte... this shouldn't happen, return a warning
                        print("Warning: odd number of bytes in asset '{0}' declared as 'unsigned int'".format(a.name))
                    cnt += 1
                    word = in_file.read(2)
                    if word:
                        out_file_c.write(",")
                    if word and cnt == 8:
                        out_file_c.write("\n")
                        cnt = 0

            out_file_c.write("};\n\n")
            in_file.close()
    out_file_h.close()
    out_file_c.close()
else:
    if len(BankList) == 0:
        print("Fatal: no banks generated")
        sys.exit(1)
    else:
        print("Info: {0!s} bank(s) generated".format(len(BankList)), end="")
        for bank_n, b in enumerate(BankList):
            print(" [bank{0!s}:{1!s}]".format(bank_n + first_bank, b.free), end=""),
        print(" bytes free")
