#!/usr/bin/python
import json
from collections import Counter
from pandas import DataFrame, Series
import pandas as pd
import numpy as np

records = [json.loads(line) for line in open('usagov_bitly_data2012-03-16-1331923249.txt')]

#collection
timezone = [re['tz'] for re in records  if 'tz' in re]
counts = Counter(timezone)
print counts.most_common(10)


#pandas
frame = DataFrame(records)
tz_counts = frame['tz'].value_counts()
print tz_counts[:10]

tz_v2 = frame['tz'].fillna('Missing')
tz_v2[tz_v2 == ''] = 'Unknown'
tz_counts_v2 = tz_v2.value_counts()
print tz_counts_v2[:10]



