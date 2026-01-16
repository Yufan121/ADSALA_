import pandas as pd
import numpy as np
import os
import shutil

script_template = './real_2.sh'
domains_dir = './domains_2'
out_dirname = 'STDOUT'

######split domain files#####

def mkdir(path):
	folder = os.path.exists(path)
	if not folder:                   #判断是否存在文件夹如果不存在则创建为文件夹
		os.makedirs(path)            #makedirs 创建文件时如果路径不存在会创建这个路径
		print( "---  new folder...  ---")
		print( "---  OK  ---")
	else:
		print( "---  There is this folder!  ---")
#Read file
csv_path = "./z__.d"
data = pd.read_csv(csv_path)
domains = np.array(data)
np.product(domains, axis=1).sum()
path = domains_dir
if os.path.exists(path):
    shutil.rmtree(path)
if not os.path.exists(path):
    os.mkdir(path)
#Save split files
single_file_rows = 100 # control
data_ = data.sample(frac=1).reset_index(drop=True)#randomize entries
n_rows = data_.shape[0]
for i in range(n_rows//single_file_rows):
    new_df = data_.iloc[i*single_file_rows: (i+1)*single_file_rows, :]#save to several portion
    new_df.to_csv(path+"/"+str(i)+".d", index=None)
new_df = data_.iloc[-(n_rows%single_file_rows): , :]
new_df.to_csv(path+"/"+str(n_rows//single_file_rows)+".d", index=None)#save remainder


######create discrete job files#####

domain_name = "real_2.d"

for index in range(n_rows//single_file_rows + 1):
    text = []
    oldStr1 = domain_name
    newStr1 = str(index) + ".d"
    oldStr2 = out_dirname
    newStr2 = out_dirname + "/" + str(index) + ".out"
    with open(script_template,'r',encoding='utf-8') as f:#read file
        text = f.readlines()
    for i in range(len(text)):
        text[i] = text[i].replace(oldStr1, newStr1, -1)#replace str
        text[i] = text[i].replace(oldStr2, newStr2, -1)
    with open(path+'/'+str(index)+'.sh','w',encoding = 'utf-8') as f: #save new file; overwrite
        for i in range(len(text)):
                f.write(text[i])


#######write batch script and STDIO#######
with open(path+'/batch.sh','w',encoding = 'utf-8') as f: #save new file; overwrite
	batch_test = "for i in `seq 0 1 "+str(n_rows//single_file_rows)+"`;\ndo\njob=\"$i.sh\"\nsbatch $job\necho \"Job submitted = $i\"\ndone"
	f.write(batch_test)
# if os.path.exists(path + "/" + out_dirname):
#     shutil.rmtree(path + "/" + out_dirname)
os.mkdir(path + "/" + out_dirname)
