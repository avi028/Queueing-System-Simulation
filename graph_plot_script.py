import matplotlib.pyplot as plt
import pandas as pd



# Load data
data = pd.read_csv('Report - Report.csv')

# Plot
plt.figure(figsize=(6.8, 4.2))
x = range(len(data['noOfUsers']))
plt.plot(x, data['badput'])
plt.xticks(x, data['noOfUsers'])
plt.xlabel('Number of Users')
plt.ylabel('Badput (replies/sec)')
plt.show()

# Plot
# plt.figure(figsize=(6.8, 4.2))
x = range(len(data['noOfUsers']))
plt.plot(x, data['goodput'])
plt.xticks(x, data['noOfUsers'])
plt.xlabel('Number of Users')
plt.ylabel('Goodput (replies/sec)')
plt.show()

# Plot
# plt.figure(figsize=(6.8, 4.2))
x = range(len(data['noOfUsers']))
plt.plot(x, data['throughput'])
plt.xticks(x, data['noOfUsers'])
plt.xlabel('Number of Users')
plt.ylabel('Throughput (replies/sec)')
plt.show()


# Plot
# plt.figure(figsize=(6.8, 4.2))
x = range(len(data['noOfUsers']))
plt.plot(x, data['requestDrop'])
plt.xticks(x, data['noOfUsers'])
plt.xlabel('Number of Users')
plt.ylabel('Number of Request Drops')
plt.show()

# Plot
plt.figure(figsize=(6.8, 4.2))
x = range(len(data['noOfUsers']))
plt.plot(x, data['core1'])
plt.plot(x, data['core2'])
plt.plot(x, data['core3'])
plt.plot(x, data['core4'])
plt.xticks(x, data['noOfUsers'])
plt.xlabel('Number of Users')
plt.ylabel('CPU Utilisation (%)')
plt.show()


import plotly.graph_objects as go

fig = go.Figure(data1=[go.Candlestick(x=data['noOfUsers'],
                open=data['minRes'],
                high=data['maxRes'],
                low=data['minRes'],
                close=data['maxRes'])])

fig.show()