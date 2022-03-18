#include <bits/stdc++.h>
#include <fstream>
using std::ifstream;

using namespace std;

#define MAX_USER_COUNT 100
#define CORE_COUNT 2
#define CONTEXT_SWITCH_TIME 2
#define MAX_REQUEST_GENERATED 20
#define MAX_THREAD_COUNT 1
#define MAX_BUFFER_SIZE 1000

enum serverStatus {Idle = 1, Busy=2};
enum eventType {arrival=1,departure=2,contextSwitchOutEvent=3};
enum schedulingPolicy{FCFS=1,roundRobin=2};
enum Distributions { Exponential=1, Uniform, Constant };
enum bufferStatus {Full=1,Available=2,Empty=3};

ofstream outdata;

class Service_Time {
    public:

    Distributions ds;
    int mean_service;

    Service_Time(){
        ds = Uniform;
        mean_service = 5;
    }

    Service_Time(int user_mean_service, Distributions user_ds){
      this->mean_service  = user_mean_service;
      this->ds = user_ds;  
    }

    double getServiceTime() {
        double final_service_time = 0.0;
        srand(time(0));
        int random_number = rand();
        double random_number_new = (double)random_number/RAND_MAX;
        std::random_device device;     
        std::mt19937 generator(device());   
        std::uniform_int_distribution<int> dist(2,2*mean_service+2); 

        switch(ds) {
            case Exponential:
                final_service_time = -mean_service * log(random_number_new);
                break;
            
            case Uniform:
                final_service_time = dist(generator);
                break;

            case Constant:
                final_service_time = mean_service;
                break;
        }
        return final_service_time;
    }  
};

class Timeout {
    public:

    Distributions ds;
    int mean_time;
    int minimum_timeout = 100;

    Timeout(){
        ds = Uniform;
        mean_time = 3;
    }

    Timeout(int user_mean_time, Distributions user_ds){
      this->mean_time  = user_mean_time;
      this->ds = user_ds;  
    }

    double getTimeout() {
        double final_timeout = 0.0;
        srand(time(0));
        int random_number = rand();
        double random_number_new = (double)random_number/RAND_MAX;
        std::random_device device;     
        std::mt19937 generator(device());   
        std::uniform_int_distribution<int> dist(1,2*mean_time+1);

        switch(ds) {
            case Exponential:
                final_timeout = (-mean_time * log(random_number_new)) + minimum_timeout;
                break;
            
            case Uniform:
                final_timeout = dist(generator)+minimum_timeout;
                break;

            case Constant:
                final_timeout = mean_time + minimum_timeout;
                break;
        }
        return final_timeout;
    }  
};

class UserData{
    public:
        double meanTimeoutTime;
        double meanServiceTime;
        double noOfUsers;    
        Distributions serviceTimeDistribution;
        Distributions timeotTimeDistribution;
        schedulingPolicy policy;
};

class Event {
    public:
       int eventId;
       eventType type;
       double arrivalTime;
       double timeout;
       int contextSwitchOutTime;
       double serviceTime;
       double departureTime;
       double waitingTime;

       int core;
       int thread;
       int response_count;
       double getRandomThinkTime();
       double getRemainingServiceTime(); 
};

double Event::getRandomThinkTime(){
    std::random_device device;     
    std::mt19937 generator(device());    
    std::uniform_int_distribution<int> dist(4,10);
    auto thinkTime = dist(generator);
    return (double)thinkTime;
}

double Event::getRemainingServiceTime(){
    return serviceTime;
}

class Scheduler{
    public:
        
        schedulingPolicy type;
        int contextSwitchTIme;

        Scheduler(){}
        Scheduler(schedulingPolicy p);
        void setPolicy(schedulingPolicy p);
        void threadSwitch(Event obj1,Event obj2);
};

Scheduler::Scheduler(schedulingPolicy p){
    type=p;
    contextSwitchTIme = CONTEXT_SWITCH_TIME;
}

void Scheduler::threadSwitch(Event obj1,Event obj2){
    // to be written
}

void Scheduler::setPolicy(schedulingPolicy p){
    this->type = p;
}

class Core{
    public:
    
        std::queue<Event> threads;
        serverStatus status;
        int threadBusyCount;
        Scheduler schedulerObj;
        int coreId;
        static int coreIdIterator;
    
        Core(){}
        Core(UserData);
        serverStatus getCoreStatus();
        void setCoreStatus(serverStatus status);
        int addToThread(Event obj);
        void removeFromThread();
        int getBUsyThreadCount();
        void setBusyThreadCount(int count);
};

Core::Core(UserData obj){
    status = Idle;
    threadBusyCount=0;
    coreId=coreIdIterator++;
    schedulerObj = Scheduler(obj.policy);
}

serverStatus Core::getCoreStatus(){
    if(this->threadBusyCount==MAX_THREAD_COUNT)
        return Busy;
    return Idle;
}

void Core::setCoreStatus(serverStatus status){
    this->status = status;
}

int Core::addToThread(Event obj){
    threadBusyCount = threadBusyCount + 1;
    if(threadBusyCount >= MAX_THREAD_COUNT)
        this->status = Busy;     
    return 1;
}

void Core::removeFromThread(){
    this->threadBusyCount = this->threadBusyCount -1;
    if(this->threadBusyCount < MAX_THREAD_COUNT)
        this->status = Idle;             
}

int Core::getBUsyThreadCount(){
    return this->threadBusyCount;
}

void Core::setBusyThreadCount(int count){
    this->threadBusyCount+=count;
}

class Server{
    public:
        Core coreObj[CORE_COUNT];
        Service_Time serviceTimeObj;
        std::queue<Event> buffer;

        Server(){}
        Server(UserData);
        Event getNextEvent();
        void addEventToBuffer(Event);
        serverStatus getServerStatus();
        Core getCore(int coreCount);
        int getFreeCore(); 
        bufferStatus getBufferStatus();

};

Server::Server(UserData obj){
    for(int i=0;i<CORE_COUNT;i++){
        coreObj[i]= Core(obj);
    }
    serviceTimeObj = Service_Time(obj.meanServiceTime,obj.serviceTimeDistribution);
}

// true == empty false = data in
bufferStatus Server::getBufferStatus(){
    if(this->buffer.empty())
        return Empty;
    if(this->buffer.size()==MAX_BUFFER_SIZE)
        return Full;
    return Available;
}
// coreId 1,2,3,4

int Server::getFreeCore(){
    int maxVal = MAX_THREAD_COUNT;
    int coreId = 0;
    for (int i=0;i<CORE_COUNT;i++){
        if(this->coreObj[i].getBUsyThreadCount()<maxVal){
            maxVal = this->coreObj[i].getBUsyThreadCount();
            coreId = this->coreObj[i].coreId;
        }
    }
    return coreId;    
}

void Server::addEventToBuffer(Event obj){
    this->buffer.push(obj);
}

Event Server::getNextEvent(){
    Event obj=  this->buffer.front();
    this->buffer.pop();
    return obj;
}

serverStatus Server::getServerStatus(){
    int busyCount=0;
    for (int i=0;i<CORE_COUNT;i++){
        if(this->coreObj[i].getCoreStatus()==Busy && this->coreObj[i].threadBusyCount==MAX_THREAD_COUNT){
            busyCount++;
        }
    }
    if(busyCount==CORE_COUNT)
        return Busy;

    return Idle;
}

Core Server::getCore(int core){
    return this->coreObj[core];
}

struct timeEventTuple{
    double eventTime;
    Event eventObj;
};

struct comparatorTimeEventTuple{
    bool operator()(struct timeEventTuple const&t1 , struct timeEventTuple const&t2){
        return t1.eventTime > t2.eventTime;
    }
};

class EventHandler {
    public:
        Server serverObj;
        UserData userDataObj;
        
        double gblSystemTime;
        int eventIdSeed ;
        priority_queue < timeEventTuple, vector<timeEventTuple>, comparatorTimeEventTuple > nextEventTime;
        Timeout timeoutObj;

        EventHandler(){}
        EventHandler(UserData);
        void setUserData();
        int genNewEventId();
        timeEventTuple getNextEvent();
        void manageEvent(Event event);
        Server getServerObj();
        void setEvent(double,Event);
        void arrive(Event);
        void depart(Event);
        void contextSwitchOut(Event);
        bool IsNextEventTimeBufferEmpty();
        void report(Event);
        void printState(timeEventTuple);
};

bool EventHandler::IsNextEventTimeBufferEmpty() {
        return this->nextEventTime.empty();
}

void EventHandler::printState(timeEventTuple te) {
    cout << gblSystemTime << "\t\t";
	if (this->serverObj.getServerStatus() == Idle) {
        cout << "Idle\t\t";
    }
    else {
        cout << "Busy\t\t";
    }
	if (this->serverObj.buffer.empty()) {
		cout << "Empty\t\t";
	}
	else {
        cout << serverObj.buffer.front().arrivalTime << "\t\t";
    }
	 	

	cout << te.eventObj.type << "\t\t" << te.eventTime << endl;
	cout << "==================================================================================================" << endl;
}

void EventHandler::report(Event e) {
    outdata << e.eventId << "," << e.arrivalTime << "," << e.departureTime << "," << e.waitingTime << endl;
}

EventHandler::EventHandler(UserData obj){
    serverObj = Server(obj);
    userDataObj = obj;
    timeoutObj = Timeout(obj.meanTimeoutTime,obj.timeotTimeDistribution);
    eventIdSeed = 1;
    gblSystemTime=0;
}

int EventHandler::genNewEventId(){
    return this->eventIdSeed++;
}

timeEventTuple EventHandler::getNextEvent(){
    timeEventTuple obj = this->nextEventTime.top();
    this->nextEventTime.pop();
    return obj;
}

void EventHandler::manageEvent(Event event){

   switch (event.type)
   {
    case arrival:
        this->arrive(event);
        break;
        
    case departure:
        this->depart(event);
        break;

    case contextSwitchOutEvent:
        /* code */ // woth round robin
        break;

    default:
        break;
   } 
}

void EventHandler::setEvent(double timeOfEvent, Event obj){
    this->nextEventTime.push({timeOfEvent,obj});
}

Server EventHandler::getServerObj(){
    return this->serverObj;
}

void EventHandler::arrive(Event X){
    if( this->userDataObj.noOfUsers < MAX_USER_COUNT ){
        int requestCount = this->genNewEventId();
        if(requestCount < MAX_REQUEST_GENERATED){
            Event obj = Event();
            obj.arrivalTime = gblSystemTime+obj.getRandomThinkTime();
            obj.eventId = requestCount;
            obj.type = arrival;
            obj.serviceTime = this->serverObj.serviceTimeObj.getServiceTime();
            obj.timeout = gblSystemTime + this->timeoutObj.getTimeout();
            this->setEvent(obj.arrivalTime,obj);  
        }
    }
    if(this->serverObj.getServerStatus()==Busy){

        if(this->serverObj.getBufferStatus()==Full){
            printf("System Exit due ot overFlow!!");
            exit(0);
        }
        this->serverObj.addEventToBuffer(X);

    }
    else{

        int freeCore = this->serverObj.getFreeCore();
        X.core = freeCore;
        serverObj.coreObj[freeCore].addToThread(X);
        X.thread = this->serverObj.coreObj[freeCore].getBUsyThreadCount();
        X.departureTime = gblSystemTime + X.serviceTime;
        X.waitingTime = gblSystemTime - X.arrivalTime;
        X.type = departure;
        this->setEvent(X.departureTime,X);
    }
} 

void EventHandler::depart(Event X){

    this->serverObj.coreObj[X.core].removeFromThread();

    if(this->serverObj.getBufferStatus() != Empty){
        Event A =  this->serverObj.getNextEvent();
        int freeCore = this->serverObj.getFreeCore();
        A.core = freeCore;
        serverObj.coreObj[freeCore].addToThread(A);
        A.thread = this->serverObj.coreObj[freeCore].getBUsyThreadCount();
        A.departureTime = gblSystemTime + A.serviceTime;
        A.waitingTime = gblSystemTime - A.arrivalTime;
        A.type = departure;
        this->setEvent(A.departureTime,A);
    }

    if(X.departureTime < X.timeout){
        report(X);
        X.response_count++;
        int requestCount = this->genNewEventId();
        if(requestCount < MAX_REQUEST_GENERATED){
            X.eventId = requestCount;
            X.type = arrival;
            X.timeout = gblSystemTime + this->timeoutObj.getTimeout();
            X.arrivalTime = gblSystemTime+X.getRandomThinkTime();
            X.serviceTime = this->serverObj.serviceTimeObj.getServiceTime();
            X.departureTime = 0;
            X.waitingTime =0 ;
            X.core =0 ;
            X.thread =0 ;
            this->setEvent(X.arrivalTime,X);  
        }
    }
    else{
        X.type = arrival;
        X.arrivalTime = gblSystemTime+X.getRandomThinkTime();
        X.timeout = gblSystemTime + this->timeoutObj.getTimeout();
        X.departureTime = 0;
        X.waitingTime =0 ;
        X.core =0 ;
        X.thread =0 ;
        this->setEvent(X.arrivalTime,X);  
    }
}

//Round robin
void EventHandler::contextSwitchOut(Event X){

}

class Simulation {
    public:
        EventHandler eventHandlerObj;
        Simulation(){}
        Simulation(UserData);
        void time(timeEventTuple X);
        void initialize();
};

Simulation::Simulation(UserData obj){
    eventHandlerObj = EventHandler(obj);
};

void Simulation::time(timeEventTuple X){
    eventHandlerObj.gblSystemTime = X.eventTime;
}

void Simulation::initialize(){
    Event obj = Event();
    obj.eventId = eventHandlerObj.genNewEventId();
    obj.arrivalTime = eventHandlerObj.gblSystemTime;
    obj.type = arrival;
    obj.timeout = eventHandlerObj.gblSystemTime + eventHandlerObj.timeoutObj.getTimeout();
    obj.serviceTime = eventHandlerObj.serverObj.serviceTimeObj.getServiceTime();
    eventHandlerObj.setEvent(0, obj);
}

int Core::coreIdIterator = 0;

UserData readFromFile (UserData obj) {
    string file_name = "";
    cout << "Enter file name" << endl;
    cin >> file_name;
    ifstream indata; 
    int num; 
    indata.open(file_name); 
    if(!indata) { 
        cerr << "Error: file could not be opened" << endl;
        exit(1);
    }
    indata >> obj.meanServiceTime;
    indata >> obj.meanTimeoutTime;
    indata >> num;
    switch(num) {
        case Exponential:
            obj.serviceTimeDistribution = Exponential;
            break;
        case Uniform:
            obj.serviceTimeDistribution = Uniform;
            break;
        case Constant:
            obj.serviceTimeDistribution = Constant;
            break;
        default:
            cout << "Wrong Distribution" << endl;
            exit(0);
            break;
    }
    indata >> num;
    switch(num) {
        case Exponential:
            obj.timeotTimeDistribution = Exponential;
            break;
        case Uniform:
            obj.timeotTimeDistribution = Uniform;
            break;
        case Constant:
            obj.timeotTimeDistribution = Constant;
            break;
        default:
            cout << "Wrong Distribution" << endl;
            exit(0);
            break;
    }
    indata >> obj.noOfUsers;
    //indata >> obj.policy;
    indata.close();
    return obj;
}

UserData manualRead(UserData obj) {
    cout << "Enter mean service time of server" << endl;
    cin >> obj.meanServiceTime;
    cout << "Enter mean Timeout time of request" << endl;
    cin >> obj.meanTimeoutTime;
    cout << "Enter service time distribution eg. 1" << endl;
    cout << "1. Exponential" << endl << "2. Uniform" << endl << "3. Constant" << endl;
    int a = 0;
    cin >> a;
    switch(a) {
        case Exponential:
            obj.serviceTimeDistribution = Exponential;
            break;
        case Uniform:
            obj.serviceTimeDistribution = Uniform;
            break;
        case Constant:
            obj.serviceTimeDistribution = Constant;
            break;
        default:
            cout << "Wrong Distribution" << endl;
            exit(0);
            break;
    }
    cout << "Enter Timeout distribution" << endl;
    cout << "1. Exponential" << endl << "2. Uniform" << endl << "3. Constant" << endl << "Enter numbers" << endl;
    cin >> a;
    switch(a) {
        case Exponential:
            obj.timeotTimeDistribution = Exponential;
            break;
        case Uniform:
            obj.timeotTimeDistribution = Uniform;
            break;
        case Constant:
            obj.timeotTimeDistribution = Constant;
            break;
        default:
            cout << "Wrong Distribution" << endl;
            exit(0);
            break;
    }
    cout << "Enter number of users in the system" << endl;
    cin >> obj.noOfUsers;
    // cout << "Enter number scheduling policy" << endl;
    // cin >> obj.policy;
}

UserData read(UserData obj) {
    cout << "Enter input type eg. 1" << endl << "1. Manual" << endl << "2. From a file" << endl;
    int input_type = 0;
    cin >> input_type;

    if (input_type == 1) {
        obj = manualRead(obj);
    }
    else {
        obj = readFromFile(obj);
    } 
    return obj;
}

int main(){
    UserData obj = UserData();
    obj =  read(obj);
    // obj.meanServiceTime = 20;
    // obj.meanTimeoutTime =10;
    // obj.serviceTimeDistribution = Exponential;
    // obj.timeotTimeDistribution = Uniform;
    // obj.noOfUsers = 10;

    
    outdata.open("outfile.csv");
    outdata << "EventId,Arrival Time,Departure Time,Waiting Time" << endl;

    Simulation simObj = Simulation(obj);
    simObj.initialize();
    
    cout << "Global System Time\t" << "Server Status\t" << "Server Buffer Top Element\t" << "Next Event Type\t" << "Next Event Time" << endl;

    while(simObj.eventHandlerObj.IsNextEventTimeBufferEmpty()==false){
        timeEventTuple x = simObj.eventHandlerObj.getNextEvent();
        simObj.eventHandlerObj.printState(x);
        simObj.time(x);
        simObj.eventHandlerObj.manageEvent(x.eventObj);
    }
    return 0;
}