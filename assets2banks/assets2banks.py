import sys, os, os.path

class Asset: 
     def __init__(self, name, size): 
         self.name = name
         self.size = size
         self.style = 0
     def set_style(self, style):
         self.style = style
     def fixed_name(self):
         return self.name.replace('(', '_').replace(')', '_').replace(' ', '_').replace('.', '_') 
         
class AssetGroup: 
     def __init__(self): 
         self.assets = [] 
         self.size = 0 
     def add_asset(self, asset): 
         self.assets.append(asset) 
         self.size += asset.size

class Bank:
     def __init__(self): 
         self.assetgroups = [] 
         self.size = 0
     def add_assetgroup(self, assetgroup): 
         self.assetgroups.append(assetgroup) 
         self.size += assetgroup.size

def find(f, seq):
  for item in seq:
    if f(item): 
      return item         
       
AssetGroupList = []    # list of the groups (we will sort this)
AssetList = []         # list of the assets (for inspection)
BankList = []          # list of the banks

print ("*** sverx's assets2banks converter ***")

if len(sys.argv)==2:
    assets_path=sys.argv[1]
else:
    print ("Usage: assets2banks path")
    exit(1)

in_a_group = False                                          # read config and create assets and assets group accordingly
config_file = open (os.path.join(assets_path, "assets2banks.cfg"),"r").readlines()
for l in config_file:
    if l[0] == "#":                                         # if line starts with #, it is a comment, we can skip it
        pass
    elif l[0] == "{":                                       # if line starts with {, it means we are beginning a group
        ag = AssetGroup()
        AssetGroupList.append(ag)
        in_a_group = True;
    elif l[0] == "}":                                       # if line starts with }, it means we are closing an open group
        in_a_group = False;
    elif l[0] == ":":                                       # if line starts with :, it means we have an attribute
        if l.strip('\n')==":format unsigned int":
          a.set_style(1)
    else:                                                   # else it's an asset
        st = os.stat(os.path.join(assets_path, str(l.strip('\n')))) 
        a = Asset(str(l.strip('\n')),st.st_size)
        AssetList.append(a)
        if not in_a_group:
            ag = AssetGroup()
            AssetGroupList.append(ag)
        ag.add_asset(a)

st = os.stat(os.path.join(assets_path, "assets2banks.cfg")) # fake asset, to skip config file, if present in folder
a = Asset("assets2banks.cfg",st.st_size)                    
AssetList.append(a)

#for a in AssetList:
#    print (a.name, a.size, len(a.name))
    
for f in os.listdir(assets_path):                          # read directory content and create missing assets and assetgroup (one per asset)
    if os.path.isfile(os.path.join(assets_path, f)):
        st = os.stat(os.path.join(assets_path, f))
        a = Asset(str(f),st.st_size)
        if not find(lambda asset: asset.name == str(f), AssetList):
#            print (a.name, a.size, "not in list!")  
            AssetList.append(a)
            ag = AssetGroup()
            ag.add_asset(a)
            AssetGroupList.append(ag)
#        else:             
#            print (a.name, a.size, "already in list!")



            

AssetGroupList.sort(key=lambda g: g.size, reverse=True)    # sort the AssetGroupList by size, descending

for ag in AssetGroupList:                                  # now find a place for each assetgroup
#    print (ag.assets, ag.size)
    for b in BankList:
#        print (b.size) 
        if (b.size+ag.size)<=16384:                        # we've found a place for this assetgroup
#            print ("added to existing bank") 
            b.add_assetgroup(ag)
            break;                       
    else:                                                  # we've found no place for this assetgroup, allocate a new bank
        b = Bank()
        b.add_assetgroup(ag)
        BankList.append(b)
#        print ("added to new bank") 

for bank_n, b in enumerate (BankList):
    out_file_h = open ("bank" + str(bank_n + 2) + ".h","w")
    out_file_c = open ("bank" + str(bank_n + 2) + ".c","w")
    for ag in b.assetgroups:
        for a in ag.assets:
            in_file = open (os.path.join(assets_path, a.name),"rb")
            if a.style==0:
                out_file_c.write("const unsigned char " + a.fixed_name() + "[" + str(a.size) + "]={\n") 
                out_file_h.write("extern const unsigned char\t" + a.fixed_name() + "[" + str(a.size) + "];\n")
            else:
                out_file_c.write("const unsigned int " + a.fixed_name() + "[" + str(a.size//2) + "]={\n") 
                out_file_h.write("extern const unsigned int\t" + a.fixed_name() + "[" + str(a.size//2) + "];\n")
                 
            out_file_h.write("#define\t\t\t\t" + a.fixed_name() + "_size " + str(a.size) + "\n")
            out_file_h.write("#define\t\t\t\t" + a.fixed_name() + "_bank " + str(bank_n + 2) + "\n")

            if a.style==0:
                cnt = 0
                byte = in_file.read(1)
                while byte:
                    out_file_c.write("0x" + format(ord(byte), '02x'))
                    cnt += 1;
                    byte = in_file.read(1)
                    if byte:
                        out_file_c.write(",")
                    if byte and cnt==16:
                        out_file_c.write("\n")
                        cnt = 0
            else:
                cnt = 0
                word = in_file.read(2)
                while word:
                    out_file_c.write("0x" + format(word[1],'02x') + format(word[0],'02x'))
                    cnt += 1;
                    word = in_file.read(2)
                    if word:
                        out_file_c.write(",")
                    if word and cnt==8:
                        out_file_c.write("\n")
                        cnt = 0

            out_file_c.write("};\n\n")
            in_file.close()
    out_file_h.close()
    out_file_c.close()
else:
    print ("Info: " + str(bank_n + 1) + " banks generated\n")
    
