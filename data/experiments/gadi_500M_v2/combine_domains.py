import numpy as np
import pandas as pd

batch_domains=pd.DataFrame()
for i in range(0, 18): #change this when collect data!
    csv_path = "./domains_1/" + str(i) + ".d"
    domains = pd.read_csv(csv_path)
    batch_domains = pd.concat([batch_domains, domains])

batch_domains.describe()
batch_domains.to_csv("batch_domains.csv", index=None)
