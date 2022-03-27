# Source Code File Structure

Main Folder
    
    - EventHandler.cpp
    - infile.txt
    - Trace.txt
    - Report.csv
    - README.md

# Instructions of Compilation and Execution of Code

Compile the code using

    g++ Simulation.cpp -o Simulation

Execute the code using

    ./Simulation

# Execution Instructions

The program accepts following inputs

        - Action
            - 1: Single Simulation Run
            - 2: Metric Calculation
        - Mean Service Time (Non-Negative Real Number)
        - Mean TimeOut Time (Non-Negative Real Number)
        - Service Time Distribution (Integer)
            - 1: Exponential
            - 2: Uniform
            - 3: Constant
        - TimeOut Time Distribution (Integer)
            - 1: Exponential
            - 2: Uniform
            - 3: Constant
        - Number of Users (Positive Integer)
        - Maximum Requests Per User (Positive Integer)

There are two ways to give input
    
    - From Terminal: Execute the above given commands and give inputs wherever asked.
    - From File: Put the inputs in a file names "infile.txt" in the same order mentioned above and every input on different line.

# Trace and Output

The program produces the trace whenever global time reaches some next event time. The trace is generated only when program is executed in single simulation mode. The trace is outputed on terminal as well as in a file called Trace.txt. The trace contains following data in every row in same order.
    
    - Simulation Timer
    - All Core Status: The status of all the 4 cores whether they are idle or doing something
    - Server Buffer Top Element: The top element in the server queue
    - Next Event Type: The next event that is going to be processed
    - Next Event Time: The time when next event is going to be processed.

# Metrics Calculation

To calculate the metric, select Metirc Calculation mode. Give maximum number of users till till which you want to calculate metrics. The program will generate the metrics while incrementing number of users by 10 every time. The metric data is stored in a file named "Report.csv". The python script provided can be used to plot the graphs.