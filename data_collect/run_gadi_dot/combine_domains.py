import numpy as np
import pandas as pd
import glob

batch_domains=pd.DataFrame()
# collect all files end with .d using glob
fn_list =  glob.glob("./domains_1/*.d")

for fn in fn_list: #change this when collect data!
    domains = pd.read_csv(fn)
    batch_domains = pd.concat([batch_domains, domains])
    pass
# obtain column names in fn_list[0]
col_names = list(pd.read_csv(fn_list[0]).columns)
col_num = len(col_names)
# remove all elements containing "Unnamed"
col_names = [x for x in col_names if "Unnamed" not in x]
# fill the rest with positive integers until len is col_num
col_names = col_names + list(range(len(col_names), col_num)) # two params of range means start and end

print(col_names)
batch_domains.columns = col_names
# ditch empty and nan rows
batch_domains = batch_domains.dropna()
# save to csv
batch_domains.to_csv("batch_domains.csv", index=None)
print(batch_domains.describe())
