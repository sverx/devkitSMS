#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Author: sverx
# Version: 2.0.0

from __future__ import absolute_import, division, generators, unicode_literals, print_function, nested_scopes
import sys
import os.path
import array


class OverWrite:
    def __init__(self, start, length, replace):
        self.start = start
        self.length = length
        self.replace = replace


class Asset:
    def __init__(self, name, size):
        self.name = name
        self.size = size
        self.style = 0
        self.overwrites = []

    def set_style(self, style):
        self.style = style

    def add_overwrite(self, overwrite):
        self.overwrites.append(overwrite)


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
        self.size = size
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

if 2 <= len(sys.argv) <= 4:
    assets_path = sys.argv[1]
    first_bank = 2                                                  # first bank will be number 2
    compile_rel = 0                                                 # RELs not requested
else:
    print("Usage: assets2banks path [--bank1size=<size>][--compile]")
    sys.exit(1)

for n, arg in enumerate(sys.argv):
    if n >= 2:
        if arg[:12] == "--bank1size=":
            try:
                b = Bank(int("+{0!s}".format(arg[12:])))
                BankList.append(b)
                first_bank = 1                                      # first bank should be number 1
                print("Info: bank1 size set at {0!s} bytes".format(b.free))
            except ValueError:
                print("Fatal: invalid bank1 size parameter")
                sys.exit(1)
        elif arg == "--compile":
            compile_rel = 1
            print("Info: compiled output requested")
        else:
            print("Usage: assets2banks path [--bank1size=<size>]")
            sys.exit(1)


# read cfg file (if present) and create assets and assets group accordingly
in_a_group = False
try:
    config_file = open(os.path.join(assets_path, "assets2banks.cfg"), "r").readlines()
    for l in config_file:
        ls = l.strip()
        if len(ls) == 0:                                 # if line is empty or made of just spaces, skip it
            pass
        elif ls[0] == "#":                               # if line starts with # it is a comment, we can skip it
            pass
        elif ls[0] == "{":                               # if line starts with { it means we start a group
            ag = AssetGroup()
            AssetGroupList.append(ag)
            in_a_group = True
        elif ls[0] == "}":                               # if line starts with } it means we close a group
            in_a_group = False
        elif ls[0] == ":":                               # if line starts with : it means we have an attribute
            if ls == ":format unsigned int":
                a.set_style(1)
            elif ls[:11] == ":overwrite ":
                ovp = ls[11:].split()
                try:
                    if len(ovp) == 2:                    # if there are two values only, we overwrite just one value
                        ov = OverWrite(int(ovp[0], 0), 1, ovp[1:])
                    else:
                        ov = OverWrite(int(ovp[0], 0), int(ovp[1], 0), ovp[2:])
                except ValueError:
                    print("Fatal: invalid overwrite attribute parameter(s)")
                    sys.exit(1)
                a.add_overwrite(ov)
            else:
                print("Fatal: unknown attribute '{0}'".format(ls))
                sys.exit(1)
        else:                                                   # else it's an asset
            try:
                st = os.stat(os.path.join(assets_path, str(ls)))
            except OSError:
                print("Fatal: can't find asset '{0}'".format(ls))
                sys.exit(1)
            a = Asset(ls, st.st_size)
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

try:
    for f in os.listdir(assets_path):  # read directory content and create missing assets and assetgroup (one per asset)
        if os.path.isfile(os.path.join(assets_path, f)):
            st = os.stat(os.path.join(assets_path, f))
            a = Asset(str(f), st.st_size)
            if not find(lambda asset: asset.name == str(f), AssetList):
                AssetList.append(a)
                ag = AssetGroup()
                ag.add_asset(a)
                AssetGroupList.append(ag)
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
    out_file_h = open("bank{0!s}.h".format(bank_n + first_bank), 'w')
    if compile_rel == 0:
        out_file_c = open("bank{0!s}.c".format(bank_n + first_bank), 'w')
    else:
        out_file_rel = open("bank{0!s}.rel".format(bank_n + first_bank), 'w')
        out_file_rel.write("XL2\n")

        # count the global symbols
        global_symbols = 1                              # there's already one global symbol in the header
        for ag in b.assetgroups:
            for a in ag.assets:
                global_symbols += 1

        out_file_rel.write("H 1 areas {0!s} global symbols\n".format(format(global_symbols, "X")))
        out_file_rel.write("M bank{0!s}\n".format(bank_n + first_bank))
        out_file_rel.write("S .__.ABS. Def0000\n")
        if bank_n + first_bank == 1:
            out_file_rel.write("A _CODE size {0!s} flags 0 addr 0\n".format(format(b.size - b.free, "X")))
        else:
            out_file_rel.write("A _BANK{0!s} size {1!s} flags 0 addr 0\n".format(bank_n + first_bank,
                                                                                 format(b.size - b.free, "X")))

        # list all the  global symbols
        asset_addr = 0
        for ag in b.assetgroups:
            for a in ag.assets:
                out_file_rel.write("S _{0!s} Def{1!s}\n".format(fix_name(a.name), format(asset_addr, "04X")))
                asset_addr += a.size

    asset_addr = 0
    for ag in b.assetgroups:
        for a in ag.assets:
            in_file = open(os.path.join(assets_path, a.name), 'rb')
            if a.style == 0:
                out_file_h.write("extern const unsigned char\t{0}[{1!s}];\n".format(fix_name(a.name), a.size))
                if compile_rel == 0:
                    out_file_c.write("const unsigned char {0}[{1!s}]={{\n".format(fix_name(a.name), a.size))
            else:
                out_file_h.write("extern const unsigned int\t{0}[{1!s}];\n".format(fix_name(a.name), a.size//2))
                if compile_rel == 0:
                    out_file_c.write("const unsigned int {0}[{1!s}]={{\n".format(fix_name(a.name), a.size // 2))

            out_file_h.write("#define\t\t\t\t{0}_size {1!s}\n".format(fix_name(a.name), a.size))
            out_file_h.write("#define\t\t\t\t{0}_bank {1!s}\n".format(fix_name(a.name), bank_n + first_bank))

            if compile_rel == 1:
                out_file_rel.write("T {0!s} {1!s}\n".format(format(asset_addr % 256, "02X"),
                                                            format(asset_addr // 256, "02X")))
                out_file_rel.write("R 00 00 00 00\n")     # this is because we mapped assets in area number 0

            if a.style == 0:
                ar = array.array('B')
                ar.fromfile(in_file, a.size)
            else:
                ar = array.array('H')                  # 'I' was trying to load 32 bits integers, of course!
                ar.fromfile(in_file, a.size//2)
                if a.size % 2:
                    # odd file size... as this shouldn't happen and last byte won't be read, return a warning
                    print("Warning: asset '{0}' has odd size but declared as 'unsigned int'".format(a.name))
                    print("         so the last byte has been discarded")

            for o in a.overwrites:
                for cnt in range(o.length):
                    ar[o.start+cnt] = int(o.replace[cnt % len(o.replace)], 0)

            if compile_rel == 1:
                out_file_rel.write("T {0!s} {1!s}".format(format(asset_addr % 256, "02X"),
                                                          format(asset_addr // 256, "02X")))

            cnt = 0
            for i in range(len(ar)):
                if compile_rel == 0:
                    if a.style == 0:
                        out_file_c.write("0x{0}".format(format(ar[i], "02x")))
                    else:
                        out_file_c.write("0x{0}".format(format(ar[i], '04x')))
                else:
                    if a.style == 0:
                        out_file_rel.write(" {0!s}".format(format(ar[i], "02X")))
                        asset_addr += 1
                    else:
                        out_file_rel.write(" {0!s} {1!s}".format(format(ar[i] % 256, "02X"),
                                                                 format(ar[i] // 256, "02X")))
                        asset_addr += 2

                if compile_rel == 0:
                    cnt += 1
                    if i < len(ar)-1:
                        out_file_c.write(",")
                        if a.style == 0 and cnt == 16:
                            out_file_c.write("\n")
                            cnt = 0
                        elif a.style != 0 and cnt == 8:
                            out_file_c.write("\n")
                            cnt = 0
                else:
                    cnt += 1
                    if (a.style == 0 and cnt == 14) or (a.style != 0 and cnt == 7):   # wrap every 14 bytes
                        cnt = 0

                    if cnt == 0 or i == (len(ar)-1):
                        out_file_rel.write("\n")               # EOL
                        out_file_rel.write("R 00 00 00 00\n")  # this is because we mapped assets in area number 0
                        if i < (len(ar)-1):
                            out_file_rel.write("T {0!s} {1!s}".format(format(asset_addr % 256, "02X"),
                                                                      format(asset_addr // 256, "02X")))
            if compile_rel == 0:
                out_file_c.write("};\n\n")
            in_file.close()
    out_file_h.close()

    if compile_rel == 0:
        out_file_c.close()
    else:
        out_file_rel.close()
else:
    if len(BankList) == 0:
        print("Fatal: no banks generated")
        sys.exit(1)
    else:
        print("Info: {0!s} bank(s) generated".format(len(BankList)), end="")
        for bank_n, b in enumerate(BankList):
            print(" [bank{0!s}:{1!s}]".format(bank_n + first_bank, b.free), end=""),
        print(" bytes free")
