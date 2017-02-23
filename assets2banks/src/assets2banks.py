#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import absolute_import, division, generators, unicode_literals, print_function, nested_scopes
import sys
import os.path
import glob
import array


__author__  = "sverx"
__version__ = "1.1.1"


class Overwrite(object):
    def __init__(self, start, length, replace):
        self.start = start
        self.length = length
        self.replace = replace


class Asset(object):
    def __init__(self, name, size):
        self.name = name
        self.size = size
        self.style = 0
        self.overwrites = []

    def add_overwrite(self, overwrite):
        self.overwrites.append(overwrite)


class AssetGroup(object):
    def __init__(self):
        self.assets = []
        self.size = 0

    def add_asset(self, asset):
        self.assets.append(asset)
        self.size += asset.size


class Bank(object):
    def __init__(self, size):
        self.assetgroups = []
        self.free = size

    def add_assetgroup(self, assetgroup):
        self.assetgroups.append(assetgroup)
        self.free -= assetgroup.size


def fix_name(name):
    '''Replace special character with underscore'''
    return "".join(map(lambda ch: ch if ch not in ('(', ')', ' ', '.') else '_', self.name))

def find(fun, seq):
    for item in seq:
        if fun(item):
            return item

def main():
    # list of the groups (we will sort this)
    AssetGroupList = [] 
    # list of the assets (for inspection)    
    AssetList = []  
    # list of the banks    
    BankList = []          

    print("*** sverx's assets2banks converter ***")

    if 2 <= len(sys.argv) <= 3:
        assets_path = sys.argv[1]
        # first bank will be number 2
        first_bank = 2                                              
    else:
        print("Usage: assets2banks path [--bank1size=<size>]", file=sys.stderr)
        sys.exit(1)

    if len(sys.argv) == 3:
        if sys.argv[2].startswith("--bank1size="):
            try:
                b = Bank(int("+{0!s}".format(sys.argv[2][len("--bank1size="):])))
                BankList.append(b)
                # first bank should be number 1
                first_bank = 1                                      
                print("Info: bank1 size set at {0!s} bytes".format(b.free))
            except ValueError:
                print("Fatal: invalid bank1 size parameter", file=sys.stderr)
                sys.exit(1)
        else:
            print("Usage: assets2banks path [--bank1size=<size>]", file=sys.stderr)
            sys.exit(1)

    # read cfg file (if present) and create assets and assets group accordingly
    in_a_group = False
    try:
        for l in open(os.path.join(assets_path, "assets2banks.cfg")):
            ls = l.strip()
            if not len:
                # if line is empty or made of just spaces, skip it
                pass
            elif ls.startswith("#"):                               
                # if line starts with "#", it is a comment, we can skip it
                pass
            elif ls.startswith("{"):                               
                # if line starts with "{", it means we start a group
                ag = AssetGroup()
                AssetGroupList.append(ag)
                in_a_group = True
            elif ls.startswith("}"):
                # if line starts with "}", it means we close a group
                in_a_group = False
            elif ls.startswith(":"):                               
                # if line starts with ":", it means we have an attribute
                if ls == ":format unsigned int":
                    a.style = 1
                elif ls.startswith(":overwrite "):
                    ovp = ls[len(":overwrite "):].split()
                    try:
                        if len(ovp) == 2:                    
                            # if there are two values only, we overwrite just one value
                            ov = Overwrite(int(ovp[0], 0), 1, ovp[1:])
                        else:
                            ov = Overwrite(int(ovp[0], 0), int(ovp[1], 0), ovp[2:])
                    except ValueError:
                        print("Fatal: invalid overwrite attribute parameter(s)", file=sys.stderr)
                        sys.exit(1)
                    a.add_overwrite(ov)
                else:
                    print("Fatal: unknown attribute '{0}'".format(ls), file=sys.stderr)
                    sys.exit(1)
            else:                                                   
                # it's an asset
                try:
                    st = os.stat(os.path.join(assets_path, str(ls)))
                except OSError:
                    print("Fatal: can't find asset '{0}'".format(ls), file=sys.stderr)
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
        st = os.stat(os.path.join(assets_path, "assets2banks.cfg"))  
        # fake asset, to skip config file, if present in folder
        a = Asset("assets2banks.cfg", st.st_size)
        AssetList.append(a)
    except OSError:
        pass

    try:
        for f in glob.glob(os.path.join(assets_path, "") + "*"):  
            # read directory content and create missing assets and assetgroup (one per asset)
            st = os.stat(os.path.join(assets_path, f))
            a = Asset(str(f), st.st_size)
            if not find(lambda asset: asset.name == str(f), AssetList):
                AssetList.append(a)
                ag = AssetGroup()
                ag.add_asset(a)
                AssetGroupList.append(ag)
    except OSError:
        print("Fatal: can't access '{0}' folder".format(assets_path), file=sys.stderr)
        sys.exit(1)

    AssetGroupList.sort(key=lambda g: g.size, reverse=True)    
    # sort the AssetGroupList by size, descending

    for ag in AssetGroupList:                                 
        # now find a place for each assetgroup
        for b in BankList:
            if b.free >= ag.size:
                # we've found a place for this assetgroup
                b.add_assetgroup(ag)
                break
        else:  
             # we've found no place for this assetgroup, allocate a new bank
            if ag.size <= 16 * 1024:
                b = Bank(16 * 1024)
                b.add_assetgroup(ag)
                BankList.append(b)
            else:
                print("Fatal: asset/assetgroup too big to fit ({0!s} bytes are needed)".format(ag.size), file=sys.stderr)
                sys.exit(1)

    for bank_n, b in enumerate(BankList):
        with open("bank{0!s}.h".format(bank_n + first_bank), 'w') as out_file_h, \
            open("bank{0!s}.c".format(bank_n + first_bank), 'w') as  out_file_c:
            for ag in b.assetgroups:
                for a in ag.assets:
                    with open(os.path.join(assets_path, a.name), 'rb') as in_file:
                        if a.style == 0:
                            out_file_c.write("const unsigned char {0}[{1!s}]={{\n".format(fix_name(a.name), a.size))
                            out_file_h.write("extern const unsigned char\t{0}[{1!s}];\n".format(fix_name(a.name), a.size))
                        else:
                            out_file_c.write("const unsigned int {0}[{1!s}]={{\n".format(fix_name(a.name), a.size // 2))
                            out_file_h.write("extern const unsigned int\t{0}[{1!s}];\n".format(fix_name(a.name), a.size // 2))

                        out_file_h.write("#define\t\t\t\t{0}_size {1!s}\n".format(fix_name(a.name), a.size))
                        out_file_h.write("#define\t\t\t\t{0}_bank {1!s}\n".format(fix_name(a.name), bank_n + first_bank))

                        if a.style == 0:
                            ar = array.array('B')
                            ar.fromfile(in_file, a.size)
                        else:
                            # 'I' was trying to load 32 bits integers, of course!
                            ar = array.array('H')                  
                            ar.fromfile(in_file, a.size // 2)
                            if a.size % 2:
                                # odd file size... as this shouldn't happen and last byte won't be read, return a warning
                                print("Warning: asset '{0}' has odd size but declared as 'unsigned int'".format(a.name))
                                print("         so the last byte has been discarded")

                        for o in a.overwrites:
                            for cnt in range(o.length):
                                ar[o.start + cnt] = int(o.replace[cnt % len(o.replace)], 0)

                        cnt = 0
                        for i, _ in enumerate(ar):
                            if a.style == 0:
                                out_file_c.write("0x{0}".format(format(ar[i], "02x")))
                            else:
                                out_file_c.write("0x{0}".format(format(ar[i], '04x')))
                            cnt += 1
                            if i < len(ar) - 1:
                                out_file_c.write(",")
                                if a.style == 0 and cnt == 16:
                                    out_file_c.write("\n")
                                    cnt = 0
                                elif a.style != 0 and cnt == 8:
                                    out_file_c.write("\n")
                                    cnt = 0

                        out_file_c.write("};\n\n")
    else:
        if not BankList:
            print("Fatal: no banks generated", file=sys.stderr)
            sys.exit(1)
        else:
            print("Info: {0!s} bank(s) generated".format(len(BankList)), end="")
            for bank_n, b in enumerate(BankList):
                print(" [bank{0!s}:{1!s}]".format(bank_n + first_bank, b.free), end=""),
            print(" bytes free")
        
if __name__ == "__main__":
    main()