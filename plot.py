import matplotlib.pyplot as plt
import pandas as pd
from mplfinance.original_flavor import candlestick_ohlc
# from mplfinance import candlestick_ohlc

figure_count=0

def get_figure_count():
    global figure_count
    figure_count=figure_count+1
    return figure_count

def plot_graphs(data,tag):
    # Plot 
    plt.figure(get_figure_count(),figsize=(6.8, 4.2))
    x = data['noOfUsers']
    plt.plot(x, data['throughput'])
    # plt.xticks(x, data['noOfUsers'])
    plt.xlabel('Number of Users')
    plt.ylabel('Throughput (replies/sec)')
    plt.title('Throughput VS No of users')
    plt.tight_layout()
    plt.grid()
    plt.savefig(tag+'Throughput VS No of users')

    # Plot
    plt.figure(get_figure_count(),figsize=(6.8, 4.2))
    # x = range(len(data['noOfUsers']))
    plt.plot(x, data['meanRes'])
    # plt.xticks(x, data['noOfUsers'])
    plt.xlabel('Number of Users')
    plt.ylabel('Response Time (sec)')
    plt.title('Response Time VS No of users')
    plt.tight_layout()
    plt.grid()
    plt.savefig(tag+'Response Time VS No of users')

    # # Plot
    plt.figure(get_figure_count(),figsize=(6.8, 4.2))
    # x = range(len(data['noOfUsers']))
    plt.plot(x, data['cpu'])
    # plt.xticks(x, data['noOfUsers'])
    plt.xlabel('Number of Users')
    plt.ylabel('cpu utilization')
    plt.title('CPU Utilization VS No of users')
    plt.tight_layout()
    plt.grid()
    plt.savefig(tag+'CPU Utilization VS No of users')

    # Plot
    plt.figure(get_figure_count(),figsize=(6.8, 4.2))
    # x = range(len(data['noOfUsers']))
    plt.plot(x, data['requestDrop'])
    # plt.xticks(x, data['noOfUsers'])
    plt.xlabel('Number of Users')
    plt.ylabel('Requests dropped')
    plt.title('Requests Dropped VS No of users')
    plt.tight_layout()
    plt.grid()
    plt.savefig(tag+'Requests Dropped VS No of users')
    # plt.show()



    plt.show(block=False)

    return

def plot_compareGraphs(rr,fcfs,m,label1,label2,label3,maxUser,tag):

    fcfs = fcfs[fcfs['noOfUsers']<maxUser]
    rr = rr[rr['noOfUsers']<maxUser]
    x = rr['noOfUsers']

    plt.figure(get_figure_count(),figsize=(6.8, 4.2))
    plt.plot(x,rr['meanRes'],label=label1)
    plt.plot(x,fcfs['meanRes'],label=label2)
    plt.plot(x,m['meanRes'],label=label3)
    plt.xlabel('No Of Users')
    plt.ylabel('Response Time')
    plt.title('Response Time VS No of users')
    plt.legend()
    plt.grid()
    plt.tight_layout()
    plt.savefig(tag+'Comparision Response Time')

    plt.figure(get_figure_count(),figsize=(6.8, 4.2))
    plt.plot(x,rr['throughput'],label=label1)
    plt.plot(x,fcfs['throughput'],label=label2)
    plt.plot(x,m['throughput'],label=label3)
    plt.xlabel('No Of Users')
    plt.ylabel('Throughput')
    plt.title('Throughput VS No of users')
    plt.legend()
    plt.grid()
    plt.tight_layout()
    plt.savefig(tag+'Comparision Throughput')


    plt.figure(get_figure_count(),figsize=(6.8, 4.2))
    plt.plot(x,rr['cpu'],label=label1)
    plt.plot(x,fcfs['cpu'],label=label2)
    plt.plot(x,m['cpu'],label=label3)
    plt.xlabel('No Of Users')
    plt.ylabel('CPU Utilization')
    plt.title('CPU Utilization VS No of users')
    plt.legend()
    plt.grid()
    plt.tight_layout()
    plt.savefig(tag+'Comparision CPU Utlization')

    plt.show(block=False)

    return

def plot_confident_graph(data,tag):

    data['Open']=data['minRes']
    data['Low']=data['minRes']
    data['High']=data['maxRes']
    data['Close']=data['maxRes']

    data1 = data[['noOfUsers', 'Open', 'High',
         'Low', 'Close']]

    plt.figure(get_figure_count())
    
    candlestick_ohlc(plt.axes(), data1.values, width = 0.6,
                    colorup = 'green', colordown = 'red',
                    alpha = 0.8)
    
    plt.grid(True)
    
    plt.xlabel('No Of Users')
    plt.ylabel('Response Time')    
    plt.title('Confidence Plot Response Time ')
    plt.tight_layout()
    plt.savefig(tag+'Confidence Interval Response Time')
    plt.show(block=False)

    return

def plot_tp_gp_bp(data,tag):
    plt.figure(get_figure_count())
    x = data['noOfUsers']
    plt.plot(x,data['badput'],label='badput')
    plt.plot(x,data['goodput'],label='goodput')
    plt.plot(x,data['throughput'],label='throughput')
    plt.xlabel('No Of Users')
    plt.ylabel('throughput')
    plt.grid()
    # plt.tight_layout()
    plt.legend()
    plt.title('Throughput GoodPut Badput')
    plt.savefig(tag+'GBBP')
    plt.show(block = False)

    return

def plot_curiocity(data,tag):
    data = data[data['meanRes']<2]
    plt.figure(get_figure_count(),figsize=(6.8, 4.2))
    plt.plot(data['meanRes'], data['throughput'])
    plt.xlabel('Response time (sec)')
    plt.ylabel('throughput')
    plt.title('Throughput Vs Response time')
    plt.tight_layout()
    plt.grid()
    plt.savefig(tag+'Throughput Vs Response time')
    plt.show(block=False)

    plt.figure(get_figure_count(),figsize=(6.8, 4.2))
    plt.plot(data['cpu'], data['throughput'])
    plt.xlabel('cpu')
    plt.ylabel('throughput')
    plt.title('Throughput Vs cpu')
    plt.tight_layout()
    plt.grid()
    plt.savefig(tag+'Throughput Vs cpu')
    plt.show(block=False)
    return

if __name__ == '__main__':

    fcfs = "data/Assignment 2 data - FCFS comparision.csv"
    fcfs1000 = "data/Assignment 2 data - FCFS 1000 data.csv"

    roundRobin = 'data/Assignment 2 data - RR comparision.csv'

    measureFile = 'data/Assignment 2 data - Measurement Closed.csv'

    rr001 = "data/Assignment 2 data - 1000 RR.csv"
    rr_01 = "data/Assignment 2 data - RR 0.01.csv"
    rr_1 = "data/Assignment 2 data - rr1.csv"

    gpbp = "data/Assignment 2 data - BadGoodThroughPut variation.csv"

    fcfs1000_data = pd.read_csv(fcfs1000)

    fcfs_data = pd.read_csv(fcfs)
    rr_data = pd.read_csv(roundRobin)
    measureData = pd.read_csv(measureFile)

    rr001_data = pd.read_csv(rr001)
    rr01_data = pd.read_csv(rr_01)
    rr1_data = pd.read_csv(rr_1)

    gpbp_data = pd.read_csv(gpbp)

    measureData = measureData[['noOfUsers','meanRes','cpu','throughput']]
    measureData = measureData[measureData['noOfUsers']>1]
    measureData['cpu'] = measureData['cpu']/100

    # plot_curiocity(fcfs_data,label2)
    # plot_graphs(gpbp_data,'gpbp_')
    # plot_confident_graph(gpbp_data,'gpbp_')
    # plot_tp_gp_bp(gpbp_data,'rr')

    plot_compareGraphs(rr_data,fcfs_data,measureData,'Round Robin','FCFS','MEASURED DATA',210,'m_cmp_')
    # plot_compareGraphs(rr001_data,rr01_data,rr1_data,'ctx time : 0.001','ctx time : 0.01','ctx time : 0.1',840,'ctx_Cmp_')


    plt.pause(1.0) # Pause for interval seconds.
    input("hit[enter] to end.")
    plt.close('all')
