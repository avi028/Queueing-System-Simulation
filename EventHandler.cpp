#include <bits/stdc++.h>
#include <fstream>
using std::ifstream;

using namespace std;

#define MAX_USER_COUNT 1000 // maximum number of users allowed 
#define CORE_COUNT 4 // number of cores in system
#define CONTEXT_SWITCH_TIME 0 // context switch time
#define MAX_REQUEST_GENERATED 20 // 
#define MAX_THREAD_COUNT 1  // Number of cores per thread
#define MAX_BUFFER_SIZE 100 // Server Buffer storage for incoming messages when no core is free
#define TIME_QUANTUM 1000 // Defined for Round Robin


enum ServerStatus {IDLE = 1, FREE, BUSY}; // 2 states of server
enum EventType {ARRIVAL=1, DEPARTURE, CONTEXTSWITCHIN, CONTEXTSWITCHOUT}; //types of event dealt with in the simulation
enum SchedulingPolicy{FCFS=1, ROUNDROBIN}; //scheduling policies the system supports
enum Distributions { EXPONENTIAL=1, UNIFORM, CONSTANT }; // types pof distribution system supports
enum BufferStatus {FULL=1, AVAILABLE, EMPTY}; // buffer status representation

ofstream reportData;
ofstream outdata; // used to write final output data to file "outfile.csv"
ofstream outTrace; // used to write trace of the system to file "Trace.txt"
double meanWaitingTime = 0.0;
double meanResponseTime = 0.0;
unsigned int request_drops = 0;

/**
 * Class Service Time
 * @brief Generates Random service time.
 *  attributes  :   Dsitribution ds (enum)
 *              :   mean_service (double)
 * 
 *  Methods     :  double getServiceTime() 
 *     
 */
class Service_Time {
    public:

    Distributions ds;
    double mean_service;

    /**
     * @brief Construct a new Service_Time object
     * 
     */
    Service_Time(){
        ds = UNIFORM;
        mean_service = 5;
    }
    /**
     * @brief Construct a new Service_Time object
     * 
     * @param user_mean_service 
     * @param user_ds 
     */
    Service_Time(int user_mean_service, Distributions user_ds){
      this->mean_service  = user_mean_service;
      this->ds = user_ds;  
    }

    /**
     * @brief Get the Service Time object
     * 
     * @return double 
     */
    double getServiceTime() {
        double final_service_time = 0.0;
        std::random_device device;     
        std::mt19937 generator(device());   
        std::uniform_int_distribution<int> dist(2,2*mean_service+2); 
        std::uniform_real_distribution<float> dist1(0, 1);

        switch(ds) {
            case EXPONENTIAL:
                final_service_time = -mean_service * log(dist1(generator));
                break;
            
            case UNIFORM:
                final_service_time = dist(generator);
                break;

            case CONSTANT:
                final_service_time = mean_service;
                break;
        }
        return final_service_time;
    }  
};
/** Class Timeout
 * @brief generates Random timeout time required by events
 * Attributes   :   Distribution ds (enum)
 *              :   mean_time (double)
 *              :   minimum_timeout(int)
 * 
 * Methods      :   double getTimeout()      
 */
class Timeout {
    public:

    Distributions ds;
    double mean_time;
    int minimum_timeout = 200;

    /**
     * @brief Construct a new Timeout object
     * 
     */
    Timeout(){
        ds = UNIFORM;
        mean_time = 3;
    }

    /**
     * @brief Construct a new Timeout object
     * 
     * @param user_mean_time 
     * @param user_ds 
     */
    Timeout(int user_mean_time, Distributions user_ds){
      this->mean_time  = user_mean_time;
      this->ds = user_ds;  
    }

    /**
     * @brief Get the Timeout object
     * 
     * @return double 
     */
    double getTimeout() {
        double final_timeout = 0.0;
        std::random_device device;     
        std::mt19937 generator(device());   
        std::uniform_int_distribution<int> dist(1,2*mean_time+1);
        std::uniform_real_distribution<float> dist1(0, 1);

        switch(ds) {
            case EXPONENTIAL:
                final_timeout = (-mean_time * log(dist1(generator))) + minimum_timeout;
                break;
            
            case UNIFORM:
                final_timeout = dist(generator)+minimum_timeout;
                break;

            case CONSTANT:
                final_timeout = mean_time + minimum_timeout;
                break;
        }
        return final_timeout;
    }  
};


/**
 * @brief used to store User Input values  
 * 
 * Attributes   :   meanTimeoutTime (double)
 *              :   meanServiceTime (double)
 *              :   noOfUsers (unsigned int)
 *              :   naxRequestPerUser(unsigned int)
 *              :   serviceTimeDistribution       (Distributions) 
 *              :   timeoutTimeDistribution       (Distributions) 
 *              :   SchedulingPolicy         (policy)
 */ 
class UserData{
    public:
        double meanTimeoutTime;
        double meanServiceTime;
        unsigned int noOfUsers;    
        unsigned int maxRequestPerUser;
        Distributions serviceTimeDistribution;
        Distributions timeotTimeDistribution;
        SchedulingPolicy policy;
};
/**
 * @brief the request instance unique to each user 
 * Attributes       :         (int) objectId
 *                  :         (int) eventId
 *                  :         (EventType) type
 *                  :         (double) arrivalTime
 *                  :         (double) timeout
 *                  :         (double) contextSwitchInTime
 *                  :         (double) contextSwitchOutTime
 *                  :         (double) serviceTime
 *                  :         (double) departureTime
 *                  :         (double) waitingTime
 *                  :         (int) core
 *                  :         (int) thread
 *                  :         (int) response_count
 *                  :         (int) request_count
 *                  :         static (int)  object_count
 * 
 * Methods          :       double getRandomThinkTime()
 *                  :       double getRemainingServiceTime()
 */
class Event {
    public:
       int objectId;
       int eventId;
       EventType type;
       double arrivalTime;
       double timeout;
       double contextSwitchInTime;
       double contextSwitchOutTime;
       double serviceTime;
       double eventServiceTime;
       double departureTime;
       double waitingTime;
       int core;
       int thread;
       int response_count;
       int request_count;
       static int  object_count;

       /**
        * @brief Gets a Random Think Time 
        * 
        * @return * double 
        */
       double getRandomThinkTime();
       /**
        * @brief Get the Remaining Service Time of the request
        * 
        * @return double 
        */
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

/**
 * @brief Used to schedule the requests on the core
 * 
 * Attributes   :   type (SchedulingPolicy)
 *              :   contextSwitchTime (int)
 */
class Scheduler{
    public:
        
        SchedulingPolicy type;
        int contextSwitchTIme;

        Scheduler(){}
        Scheduler(SchedulingPolicy p);
        void setPolicy(SchedulingPolicy p);
};

/**
 * @brief Construct a new Scheduler:: Scheduler object
 * 
 * @param p 
 */
Scheduler::Scheduler(SchedulingPolicy p){
    type=p;
    contextSwitchTIme = CONTEXT_SWITCH_TIME;
}

/**
 * @brief Set the Policy
 * 
 * @param p 
 */
void Scheduler::setPolicy(SchedulingPolicy p){
    this->type = p;
}

/**
 * @brief Each represents a core of the system
 * Attributes : threads (queue of Event Class instance Pointers)
 *            : status (ServerStatus)  
 *            : threadBusyCount (int)
 *            : schedulerObj (Scheduler Class Instance)
 *            : coreId (int identification of the core)
 *            : coreIdIterator (static Int to assign new Id to each new instance of Core Class)        
 * 
 * 
 * Methods      :  double getNextCoreAvailableTime()
 *              :  ServerStatus getCoreStatus()
 *              :  void setCoreStatus(ServerStatus status)
 *              :  int addToThread(Event obj)
 *              :  Event* removeFromThread()
 *              :  int getBUsyThreadCount()
 *              :  void setBusyThreadCount(int count)
 */
class Core{
    public:
    
        std::queue<Event*> threads;
        ServerStatus status;
        int threadBusyCount;
        Scheduler schedulerObj;
        int coreId;
        static int coreIdIterator;
    
        Core(){}
        Core(UserData);
        double getNextCoreAvailableTime();
        ServerStatus getCoreStatus();
        void setCoreStatus(ServerStatus status);
        int addToThread(Event obj);
        Event* removeFromThread();
        int getBUsyThreadCount();
        void setBusyThreadCount(int count);
};

/**
 * @brief Construct a new Core:: Core object
 * 
 * @param obj 
 */
Core::Core(UserData obj){
    status = IDLE;
    threadBusyCount=0;
    coreId=coreIdIterator++;
    schedulerObj = Scheduler(obj.policy);
}

/**
 * @brief returns next time stamp when the core is available
 * 
 * @return double 
 */
double Core::getNextCoreAvailableTime(){
    double nextFreeTime;
    if(schedulerObj.type == FCFS){
        if(threads.empty()==true)
            nextFreeTime = 0.0;
        else    
            nextFreeTime =  threads.back()->departureTime;
    }
    else{
        if(threads.empty()==true)
            nextFreeTime = 0.0;
        else    
            nextFreeTime =  max(threads.back()->departureTime, threads.back()->contextSwitchOutTime);
    }
    return nextFreeTime;
}
/**
 * @brief 
 * 
 * @return ServerStatus 
 */
ServerStatus Core::getCoreStatus(){
    if(this->threadBusyCount==MAX_THREAD_COUNT)
        return BUSY;
    if(this->threadBusyCount > 0 )
        return FREE;
    return IDLE;
}

/**
 * @brief sets core status
 * 
 * @param status 
 */
void Core::setCoreStatus(ServerStatus status){
    this->status = status;
}

/**
 * @brief  adds the event to the core process queue
 * 
 * @param obj 
 * @return int 
 */
int Core::addToThread(Event obj){
    threads.push(&obj);
    threadBusyCount = threadBusyCount + 1;
    if(threadBusyCount >= MAX_THREAD_COUNT)
        this->status = BUSY;     
    else if(threadBusyCount >0)
        this->status =FREE;
    return 1;
}

/**
 * @brief removes event entity from the core process queue
 * 
 * @return Event* 
 */
Event* Core::removeFromThread(){
    Event *obj = threads.front();
    threads.pop();
    this->threadBusyCount = this->threadBusyCount -1;
    if(this->threadBusyCount == 0)
        this->status = IDLE;
    else if(this->threadBusyCount < MAX_THREAD_COUNT)
        this->status = FREE;             
    return obj;
}

/**
 * @brief get the number of thread busy on the core
 * 
 * @return int 
 */
int Core::getBUsyThreadCount(){
    return this->threadBusyCount;
}

/**
 * @brief set the busy therad count of core
 * 
 * @param count 
 */
void Core::setBusyThreadCount(int count){
    this->threadBusyCount+=count;
}

/**
 * @brief class represensts the Server with CORE_COUNT number of cores
 * 
 * Attributes   :   coreObj (Array of core Class instances)
 *              :   serviceTimeObj (instance of ServiceTime class)
 *              :   buffer (queue of events requried to store the events in case system is full)
 * 
 * Methods      :       Event getNextEvent()
 *              :       void addEventToBuffer(Event)
 *              :       ServerStatus getServerStatus()
 *              :       Core getCore(int coreCount)
 *              :       int getFreeCore()
 *              :       BufferStatus getBufferStatus()
 */
class Server{
    public:
        Core coreObj[CORE_COUNT];
        Service_Time serviceTimeObj;
        std::queue<Event> buffer;

        Server(){}
        Server(UserData);
        Event getNextEvent();
        void addEventToBuffer(Event);
        ServerStatus getServerStatus();
        Core getCore(int coreCount);
        int getFreeCore(); 
        BufferStatus getBufferStatus();

};

/**
 * @brief Construct a new Server:: Server object
 * 
 * @param obj 
 */
Server::Server(UserData obj){
    Core::coreIdIterator = 0;
    for(int i=0;i<CORE_COUNT;i++){
        coreObj[i]= Core(obj);
    }
    serviceTimeObj = Service_Time(obj.meanServiceTime,obj.serviceTimeDistribution);
}

// true == empty false = data in
/**
 * @brief get status of the server buffer queue
 * 
 * @return BufferStatus 
 */
BufferStatus Server::getBufferStatus(){
    if(this->buffer.empty())
        return EMPTY;
    if(this->buffer.size()==MAX_BUFFER_SIZE)
        return FULL;
    return AVAILABLE;
}
// coreId 0,1,2,3

/**
 * @brief get the core id of free core if any otherwise -1
 * 
 * @return int 
 */
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

/**
 * @brief adds event to the buffer queue
 * 
 * @param obj 
 */
void Server::addEventToBuffer(Event obj){
    this->buffer.push(obj);
}

/**
 * @brief removes the event from queue and returns
 * 
 * @return Event 
 */
Event Server::getNextEvent(){
    Event obj=  this->buffer.front();
    this->buffer.pop();
    return obj;
}

/**
 * @brief return server status
 * 
 * @return ServerStatus 
 */
ServerStatus Server::getServerStatus(){
    int busyCount=0;
    for (int i=0;i<CORE_COUNT;i++){
        if(this->coreObj[i].getCoreStatus()==BUSY && this->coreObj[i].threadBusyCount==MAX_THREAD_COUNT){
            busyCount++;
        }
    }
    if(busyCount==CORE_COUNT)
        return BUSY;

    busyCount=0;
    for (int i=0;i<CORE_COUNT;i++){
        if(this->coreObj[i].getCoreStatus()==FREE){
            busyCount++;
        }
    }
    if(busyCount > 0)
        return FREE;
        
    return IDLE;
}

/**
 * @brief return instance of the core requested
 * 
 * @param core // core id
 * @return Core 
 */
Core Server::getCore(int core){
    return this->coreObj[core];
}

/**
 * @brief struct to combine event time and corresponding event object
 * 
 */
struct timeEventTuple{
    double eventTime;
    Event eventObj;
};

/**
 * @brief function defination for comparision of ghe timeevent Tuple structure
 * 
 */
struct comparatorTimeEventTuple{
    bool operator()(struct timeEventTuple const&t1 , struct timeEventTuple const&t2){
        return t1.eventTime > t2.eventTime;
    }
};

/**
 * @brief handles all the request events generated in the system
 * Attributes       :       serverObj (Server class instance)
 *                  :       userDataObj(UserData class Instance)
 *                  :       gblSystemTime (double)
 *                  :       eventIdSeed (int ) used to generate event id's
 *                  :       userCount (int) states the current user count in the system
 *                  :       maxRequestCount (unsigned int) maximum number of requests being generated in the system
 *                  :       timeoutObj (Timeout class instance)
 *                  :       nextEventQueue (a priority queue to hold the list of next events)
 * 
 * Methods          :        void setUserData();
 *                  :        int genNewEventId();
 *                  :        timeEventTuple getNextEvent();
 *                  :        void manageEvent(Event event);
 *                  :        Server getServerObj();
 *                  :        void setEvent(double,Event);
 *                  :        void arrive(Event);
 *                  :        void depart(Event);
 *                  :        void contextSwitchIn(Event);
 *                  :        void contextSwitchOut(Event);
 *                  :        bool IsNextEventTimeBufferEmpty();
 *                  :        void report(Event);
 *                  :        void printState(timeEventTuple);

 */
class EventHandler {
    public:
        Server serverObj;
        UserData userDataObj;
        double gblSystemTime;
        int eventIdSeed ;
        int userCount;
        unsigned int maxRequestCount;
        priority_queue < timeEventTuple, vector<timeEventTuple>, comparatorTimeEventTuple > nextEventTime;
        Timeout timeoutObj;
        int eventCount[MAX_USER_COUNT][2];

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
        void contextSwitchIn(Event);
        void contextSwitchOut(Event);
        bool IsNextEventTimeBufferEmpty();
        void report(Event);
        void printState(timeEventTuple);
};

/**
 * @brief returns status of the eventTime buffer
 * 
 * @return true  if buffer is empty
 * @return false  if not empty
 */
bool EventHandler::IsNextEventTimeBufferEmpty() {
        return this->nextEventTime.empty();
}

/**
 * @brief prints the current state of the system 
 * 
 * @param timeEventTuple
 */
void EventHandler::printState(timeEventTuple te) {
    cout << gblSystemTime << "\t\t";
    outTrace << gblSystemTime << "\t\t";
	
    cout << "[";
    outTrace << "[";
    for (int i = 0; i < 4; i++) {
        switch(this->serverObj.coreObj[i].getCoreStatus()) {
        case IDLE:
            cout << "I ";
            outTrace << "I ";
            break;
        case BUSY:
            cout << "B ";
            outTrace << "B ";
            break;
        case AVAILABLE:
            cout << "A ";
            outTrace << "A ";
            break; 
        }
    }
    cout << "]\t"; outTrace << "]\t";


	if (this->serverObj.buffer.empty()) {
		cout << "EMPTY\t\t";
        outTrace << "EMPTY\t\t";
	}
	else {
        cout << serverObj.buffer.front().arrivalTime << "\t\t";
        outTrace << serverObj.buffer.front().arrivalTime << "\t\t";
    }
	 	
    switch(te.eventObj.type) {
        case ARRIVAL:
            cout << "Arrival      \t\t";
            outTrace << "Arrival      \t\t";
            break;
        case DEPARTURE:
            cout << "Departure    \t\t";
            outTrace << "Departure    \t\t";
            break;
        case CONTEXTSWITCHIN:
            cout << "Cntx Swtch In\t\t";
            outTrace << "Cntx Swtch In\t\t";
            break;
        case CONTEXTSWITCHOUT:
            cout << "Cntx Swtch Out\t\t";
            outTrace << "Cntx Swtch Out\t\t";
            break; 
    }
    
    cout << "\t\t" << te.eventTime << "\t\t" << endl;
    outTrace << "\t\t" << te.eventTime << "\t\t" << endl;
	cout << "==================================================================================================" << endl;
    outTrace << "==================================================================================================" << endl;
}

/**
 * @brief logs the event details at exit 
 * 
 * @param Event 
 */
void EventHandler::report(Event e) {
    outdata << e.eventId << "," << e.arrivalTime << "," << e.departureTime << "," << e.waitingTime<< "," << e.departureTime - e.arrivalTime << "," << e.request_count << "," << e.response_count<<","<<e.objectId << endl;
}

/**
 * @brief Construct a new Event Handler:: Event Handler object
 * 
 * @param obj 
 */
EventHandler::EventHandler(UserData obj){
    serverObj = Server(obj);
    
    obj.noOfUsers = (obj.noOfUsers < MAX_USER_COUNT) ? obj.noOfUsers : MAX_USER_COUNT;
    userDataObj = obj;
    
    timeoutObj = Timeout(obj.meanTimeoutTime,obj.timeotTimeDistribution);
    eventIdSeed = 1;
    userCount=0;
    gblSystemTime=0;
    maxRequestCount = obj.noOfUsers * obj.maxRequestPerUser;
}

/**
 * @brief generates new event ID
 * 
 * @return int 
 */
int EventHandler::genNewEventId(){
    return this->eventIdSeed++;
}

/**
 * @brief returns next event in the event queue
 * 
 * @return timeEventTuple 
 */
timeEventTuple EventHandler::getNextEvent(){
    timeEventTuple obj = this->nextEventTime.top();
    this->nextEventTime.pop();
    return obj;
}

/**
 * @brief calls the appropriate function as per the state of the event
 * 
 * @param event 
 */
void EventHandler::manageEvent(Event event){

   switch (event.type)
   {
    case ARRIVAL:
        this->arrive(event);
        break;
        
    case DEPARTURE:
        this->depart(event);
        break;

    case CONTEXTSWITCHIN:
        contextSwitchIn(event);
        break;
    
    case CONTEXTSWITCHOUT:
        contextSwitchOut(event);
        break;

    default:
        break;
   } 
}

/**
 * @brief adds event to the time event queue
 * 
 * @param timeOfEvent 
 * @param obj 
 */
void EventHandler::setEvent(double timeOfEvent, Event obj){
    this->nextEventTime.push({timeOfEvent,obj});
}

/**
 * @brief returns Instance of server class
 * 
 * @return Server 
 */
Server EventHandler::getServerObj(){
    return this->serverObj;
}

/**
 * @brief Arrival event handler
 * 
 * @param Event X
 */
void EventHandler::arrive(Event X){

    // creates new Event instances if not reached the required count
    if( userCount <= this->userDataObj.noOfUsers  ){

        int requestCount = this->genNewEventId();
        if(requestCount <= maxRequestCount){
            // make the Event object ready of ARRIVAL
            Event obj = Event();
            obj.objectId=++userCount;
            obj.arrivalTime = gblSystemTime+obj.getRandomThinkTime();
            obj.eventId = requestCount;
            obj.type = ARRIVAL;
            obj.serviceTime = this->serverObj.serviceTimeObj.getServiceTime();
            obj.eventServiceTime = this->serverObj.serviceTimeObj.getServiceTime();
            obj.timeout = gblSystemTime + this->timeoutObj.getTimeout();
            obj.request_count  =1;
            obj.response_count =0;
            //push the event object in the event queue
            this->setEvent(obj.arrivalTime,obj);  
        }
    }
    // in case server is busy push the event X in the wait buffer of the server
    if(this->serverObj.getServerStatus()==BUSY){

        if(this->serverObj.getBufferStatus()==FULL){
            request_drops++;
        }
        else {
            this->serverObj.addEventToBuffer(X);
        }
    }
    // if the server is not fully  busy , get a free core and assign the event to the core
    else{
        int freeCore =-1;
        freeCore = this->serverObj.getFreeCore();
        double nextAvailableTime = 0.0;
        nextAvailableTime = max(serverObj.coreObj[freeCore].getNextCoreAvailableTime(), gblSystemTime);
        X.core = freeCore;
        X.contextSwitchInTime = nextAvailableTime + CONTEXT_SWITCH_TIME;
        X.type = CONTEXTSWITCHIN;
        serverObj.coreObj[freeCore].addToThread(X);
        X.thread = this->serverObj.coreObj[freeCore].getBUsyThreadCount();
        // send the event to the event queue
        this->setEvent(X.contextSwitchInTime,X);  
    }
} 

/**
 * @brief Departure event handle 
 * 
 * @param Event X 
 */
void EventHandler::depart(Event X){

    // removes the event from the core process thread
    this->serverObj.coreObj[X.core].removeFromThread();

    // get next request in the server buffer and assign it to a core
    if(this->serverObj.getBufferStatus() != EMPTY){
        Event A =  this->serverObj.getNextEvent();
        int freeCore = -1;
        freeCore = this->serverObj.getFreeCore();
        double nextAvialableTime = 0.0;
        nextAvialableTime = max(serverObj.coreObj[freeCore].getNextCoreAvailableTime(), gblSystemTime);
        A.core = freeCore;
        A.contextSwitchInTime = nextAvialableTime + CONTEXT_SWITCH_TIME;
        A.type = CONTEXTSWITCHIN;
        serverObj.coreObj[freeCore].addToThread(A);
        A.thread = this->serverObj.coreObj[freeCore].getBUsyThreadCount();
        this->setEvent(A.contextSwitchInTime,A);
    }

    // if response is under the timeout of the request then generate new request
    if(X.departureTime < X.timeout){
        X.response_count++;
        // report(X);
        this->eventCount[X.objectId-1][0] = X.request_count;
        this->eventCount[X.objectId-1][1] = X.response_count;
        int requestCount = this->genNewEventId();
        if(requestCount <= maxRequestCount){
            X.eventId = requestCount;
            X.type = ARRIVAL;
            X.timeout = gblSystemTime + this->timeoutObj.getTimeout();
            X.arrivalTime = gblSystemTime+X.getRandomThinkTime();
            X.serviceTime = this->serverObj.serviceTimeObj.getServiceTime();
            X.eventServiceTime = this->serverObj.serviceTimeObj.getServiceTime();
            X.departureTime = 0;
            X.waitingTime =0 ;
            X.core = 0;
            X.thread = 0;
            X.request_count +=1;
            this->setEvent(X.arrivalTime,X);  
        }
    }
    // if DEPARTURE of event X happens after the timeout value the user will wont respond to the request and will regenrate the request
    else{
        X.type = ARRIVAL;
        X.arrivalTime = gblSystemTime+X.getRandomThinkTime();
        X.timeout = gblSystemTime + this->timeoutObj.getTimeout();
        X.departureTime = 0;
        X.waitingTime = 0;
        X.core = 0;
        X.thread = 0;
        X.request_count += 1;
        this->setEvent(X.arrivalTime,X);  
    }
}

/**
 * @brief Handles the Context Switch Out event
 * 
 * @param X 
 */
void EventHandler::contextSwitchOut(Event X){

    // remove the core from the process thread of the core
    this->serverObj.coreObj[X.core].removeFromThread();
    double nextAvailableTime = 0.0;
    nextAvailableTime = max(serverObj.coreObj[X.core].getNextCoreAvailableTime(), gblSystemTime);
    X.contextSwitchInTime = nextAvailableTime + CONTEXT_SWITCH_TIME;
    X.type = CONTEXTSWITCHIN;
    this->serverObj.coreObj[X.core].addToThread(X);
    // assign it next context switch in time and push it back in the queue.
    this->setEvent(X.contextSwitchInTime, X);
}

/**
 * @brief handles the context Switch In event
 * 
 * @param X 
 */
void EventHandler::contextSwitchIn(Event X){

    // if the cervice time left is more than time quantum then assign next context switch out time
    if (X.serviceTime > TIME_QUANTUM) {
        X.serviceTime -= TIME_QUANTUM;
        X.contextSwitchOutTime = gblSystemTime + TIME_QUANTUM;
        X.type = CONTEXTSWITCHOUT;
        this->setEvent(X.contextSwitchOutTime, X);
    }
    // of the remaining service time is less than the time quantum assign the DEPARTURE time 
    else {
        X.departureTime = gblSystemTime + X.eventServiceTime;
        X.waitingTime = (X.departureTime - X.arrivalTime) - X.eventServiceTime;
        meanWaitingTime += X.waitingTime;
        meanResponseTime += X.departureTime - X.arrivalTime; 
        X.type = DEPARTURE;
        this->setEvent(X.departureTime,X);
    } 
}

/**
* @brief Handles the simulation
* Attributes   :    eventHandlerObj (Event Handler instance)
* Methods      :     void time(timeEventTuple X)
*              :     void initialize()
*/
class Simulation {
    public:
        EventHandler eventHandlerObj;
        Simulation(){}
        Simulation(UserData);
        void time(timeEventTuple X);
        void initialize();
};

/**
 * @brief Construct a new Simulation:: Simulation object
 * 
 * @param obj 
 */
Simulation::Simulation(UserData obj){
    eventHandlerObj = EventHandler(obj);
};

/**
 * @brief sets the global system time
 * 
 * @param X 
 */
void Simulation::time(timeEventTuple X){
    eventHandlerObj.gblSystemTime = X.eventTime;
}

/**
 * @brief Initialize the simulation by pushing teh first request Event
 * 
 */
void Simulation::initialize(){
    Event obj = Event();
    obj.eventId = eventHandlerObj.genNewEventId();
    obj.arrivalTime = eventHandlerObj.gblSystemTime;
    obj.type = ARRIVAL;
    obj.timeout = eventHandlerObj.gblSystemTime + eventHandlerObj.timeoutObj.getTimeout();
    obj.serviceTime = eventHandlerObj.serverObj.serviceTimeObj.getServiceTime();
    obj.eventServiceTime = eventHandlerObj.serverObj.serviceTimeObj.getServiceTime();
    obj.contextSwitchInTime = 0.0;
    obj.contextSwitchOutTime = 0.0;
    obj.request_count=1;
    obj.objectId = ++eventHandlerObj.userCount;
    obj.response_count =0;
    eventHandlerObj.userCount++;
   eventHandlerObj.setEvent(0, obj);
}

// core Id Iterator Initialization
int Core::coreIdIterator = 0;

/**
 * @brief reads input from file 
 * 
 * @param obj 
 */
void readFromFile (UserData * obj) {
    string file_name = "infile.txt";
    // cout << "Enter file name" << endl;
    // cin >> file_name;
    ifstream indata; 
    int num; 
    indata.open(file_name); 
    if(!indata) { 
        cerr << "Error: file could not be opened" << endl;
        exit(1);
    }
    indata >> obj->meanServiceTime;
    indata >> obj->meanTimeoutTime;
    indata >> num;
    switch(num) {
        case EXPONENTIAL:
            obj->serviceTimeDistribution = EXPONENTIAL;
            break;
        case UNIFORM:
            obj->serviceTimeDistribution = UNIFORM;
            break;
        case CONSTANT:
            obj->serviceTimeDistribution = CONSTANT;
            break;
        default:
            cout << "Wrong Distribution" << endl;
            exit(0);
            break;
    }
    indata >> num;
    switch(num) {
        case EXPONENTIAL:
            obj->timeotTimeDistribution = EXPONENTIAL;
            break;
        case UNIFORM:
            obj->timeotTimeDistribution = UNIFORM;
            break;
        case CONSTANT:
            obj->timeotTimeDistribution = CONSTANT;
            break;
        default:
            cout << "Wrong Distribution" << endl;
            exit(0);
            break;
    }
    indata >> obj->noOfUsers;
    indata >> obj->maxRequestPerUser;
    // indata >> num;
    // switch(num) {
    //     case FCFS:
    //         obj.policy = FCFS;
    //         break;
    //     case ROUNDROBIN:
    //         obj.policy = ROUNDROBIN;
    //         break;
    //     default:
    //         cout << "Wrong Policy";
    //         break;
    // }
    indata.close();
    // return obj;
}

/**
 * @brief manual read of input from the user
 * 
 * @param obj 
 */
void manualRead(UserData * obj) {
    cout << "Enter mean service time of server" << endl;
    cin >> obj->meanServiceTime;
    cout << "Enter mean Timeout time of request" << endl;
    cin >> obj->meanTimeoutTime;
    cout << "Enter service time distribution eg. 1" << endl;
    cout << "1. EXPONENTIAL" << endl << "2. UNIFORM" << endl << "3. CONSTANT" << endl;
    int a = 0;
    cin >> a;
    switch(a) {
        case EXPONENTIAL:
            obj->serviceTimeDistribution = EXPONENTIAL;
            break;
        case UNIFORM:
            obj->serviceTimeDistribution = UNIFORM;
            break;
        case CONSTANT:
            obj->serviceTimeDistribution = CONSTANT;
            break;
        default:
            cout << "Wrong Distribution" << endl;
            exit(0);
            break;
    }
    cout << "Enter Timeout distribution" << endl;
    cout << "1. EXPONENTIAL" << endl << "2. UNIFORM" << endl << "3. CONSTANT" << endl << "Enter numbers" << endl;
    cin >> a;
    switch(a) {
        case EXPONENTIAL:
            obj->timeotTimeDistribution = EXPONENTIAL;
            break;
        case UNIFORM:
            obj->timeotTimeDistribution = UNIFORM;
            break;
        case CONSTANT:
            obj->timeotTimeDistribution = CONSTANT;
            break;
        default:
            cout << "Wrong Distribution" << endl;
            exit(0);
            break;
    }
    cout << "Enter number of users in the system" << endl;
    cin >> obj->noOfUsers;
    cout << "Enter number of requests per user in the system" << endl;
    cin >> obj->maxRequestPerUser;
    // cout << "Enter scheduling policy" << endl << "1. FCFS" << endl << "2. Round Robin" << endl;
    // cin >> a;
    // switch(a) {
    //     case FCFS:
    //         obj.policy = FCFS;
    //         break;
    //     case ROUNDROBIN:
    //         obj.policy = ROUNDROBIN;
    //         break;
    //     default:
    //         cout << "Wrong Policy";
    //         break;
    // }
}

/**
 * @brief read user input
 * 
 * @param obj 
 */
void read(UserData *obj) {
    cout << "Enter input type eg. 1" << endl << "1. Manual" << endl << "2. From a file" << endl;
    int input_type = 0;
    cin >> input_type;

    if (input_type == 1) {
        manualRead(obj);
    }
    else {
        readFromFile(obj);
    } 
    // return obj;
}

/**
 * @brief Main Function
 * 
 * @return int 
 */
int main(){
    UserData obj = UserData();
    // read(&obj);
    obj.meanServiceTime = 2;
    obj.meanTimeoutTime = 10;
    obj.serviceTimeDistribution = EXPONENTIAL;
    obj.timeotTimeDistribution = EXPONENTIAL;
    obj.noOfUsers = 0;
    obj.maxRequestPerUser = 8;
    obj.policy = ROUNDROBIN;
    
    outdata.open("outfile.csv");
    outTrace.open("Trace.txt");
    reportData.open("Report.csv");

    // outdata << "Even_Id,Arrival_Time,Departure_Time,Waiting_Time,Response_Time,Request_Count,Response_Count,Object_Id" << endl;

    
    
    // cout << "Parameters: " << endl << "1. No. of Cores: 4" << endl << "2. No. of Threads Per Core" << MAX_THREAD_COUNT << endl;
    // cout << "3. Buffer Size: " << MAX_BUFFER_SIZE << endl << "4. Context Switch Time: " << CONTEXT_SWITCH_TIME << endl << "5. Time Quanta: " << TIME_QUANTUM << endl;
    // cout << "6. Mean Serive Time: " << obj.meanServiceTime << endl << "7. Mean Timeout Time: " << obj.meanTimeoutTime << endl;
    // cout << "8. No. of Users: " << obj.noOfUsers << endl << "9. Number of Requests per User: " << obj.maxRequestPerUser << endl << "10. Scheduling Policy: Round Robin" << endl;  

    // outTrace << "Parameters: " << endl << "1. No. of Cores: 4" << endl << "2. No. of Threads Per Core" << MAX_THREAD_COUNT << endl;
    // outTrace << "3. Buffer Size: " << MAX_BUFFER_SIZE << endl << "4. Context Switch Time: " << CONTEXT_SWITCH_TIME << endl << "5. Time Quanta: " << TIME_QUANTUM << endl;
    // outTrace << "6. Mean Serive Time: " << obj.meanServiceTime << endl << "7. Mean Timeout Time: " << obj.meanTimeoutTime << endl;
    // outTrace << "8. No. of Users: " << obj.noOfUsers << endl << "9. Number of Requests per User: " << obj.maxRequestPerUser << endl << "10. Scheduling Policy: Round Robin" << endl;  


    // cout << "Global System Time\t" << "Core Status\t" << "Server Buffer Top Element\t" << "Next Event Type\t" << "Next Event Time" << endl;

    

    for (int l = 0; l < 30; l++) {
        double finalMeanWaitingTime = 0.0;
        double finalResponseTime = 0.0;
        double finalBadput = 0.0;
        double finalGoodput = 0.0;
        double finalThroughput = 0.0;
        double mean_request_drops = 0;
        double per_core_util[CORE_COUNT] = {0.0, 0.0, 0.0, 0.0};
        obj.noOfUsers += 10;

        // double waitingArray[5];

        for (int k = 0; k<5; k++) {
            meanWaitingTime = 0.0;
            meanResponseTime = 0.0;
            request_drops = 0;
            Simulation simObj = Simulation(obj);
            simObj.initialize();
            double prev_time = 0.0;
            double core_util = 0.0;
            double core_thread_util[CORE_COUNT] = {0.0, 0.0, 0.0, 0.0};

            while(simObj.eventHandlerObj.IsNextEventTimeBufferEmpty()==false){
                timeEventTuple x = simObj.eventHandlerObj.getNextEvent();
                // simObj.eventHandlerObj.printState(x);
                simObj.time(x);
                simObj.eventHandlerObj.manageEvent(x.eventObj);
                double diff = x.eventTime - prev_time;
                prev_time = x.eventTime;
                for (int i = 0; i<CORE_COUNT; i++) {
                    // if (simObj.eventHandlerObj.serverObj.coreObj[i].getCoreStatus() != IDLE) {
                        core_thread_util[i] += diff * (simObj.eventHandlerObj.serverObj.coreObj[i].getBUsyThreadCount()/MAX_THREAD_COUNT);
                    // }
                }
            }

            int total_request_count = 0, total_response_count = 0;

            for (int i = 0; i < obj.noOfUsers; i++) {
                total_request_count += simObj.eventHandlerObj.eventCount[i][0];
                total_response_count += simObj.eventHandlerObj.eventCount[i][1];
            } 

            int retries = total_request_count - total_response_count;
            int not_timedout = total_response_count - retries;

            double badput = (retries*1.0)/simObj.eventHandlerObj.gblSystemTime;
            double goodput = (not_timedout*1.0)/simObj.eventHandlerObj.gblSystemTime; 
            double throughput = (total_response_count * 1.0)/simObj.eventHandlerObj.gblSystemTime;

            // // waitingArray[k] = meanResponseTime/simObj.eventHandlerObj.maxRequestCount;
            // // finalMeanWaitingTime += meanWaitingTime/simObj.eventHandlerObj.eventIdSeed;
            // // finalResponseTime += meanResponseTime/simObj.eventHandlerObj.maxRequestCount;

            finalBadput += badput; finalGoodput += goodput; finalThroughput += throughput;

            mean_request_drops += request_drops;

            for (int i = 0; i<CORE_COUNT; i++) {
                per_core_util[i] += core_thread_util[i]/simObj.eventHandlerObj.gblSystemTime;
            }

        }
        
        reportData << obj.noOfUsers << ",";

        // for (int p = 0; p < 5; p++) {
        //     reportData << waitingArray[p] << ",";
        // }
        reportData << mean_request_drops/5 << "," << finalBadput/5 << "," << finalGoodput/5 << "," << finalThroughput/5 << ",";
        for (int i = 0; i<CORE_COUNT; i++) {
            reportData << per_core_util[i]/5 << ",";
        }
        reportData << endl;
    }
         


    // outTrace << "Mean Waiting Time = " << meanWaitingTime/simObj.eventHandlerObj.eventIdSeed << endl;
    // outTrace << "Mean Response Time = " << meanResponseTime/simObj.eventHandlerObj.eventIdSeed << endl;
    // outTrace << "Total Request Drops = " << request_drops << endl;
    // cout << "Total Request Drops = " << request_drops << endl;
    // cout << "Mean Waiting Time = " << meanWaitingTime/simObj.eventHandlerObj.eventIdSeed << endl;
    // cout << "Mean Response Time = " << meanResponseTime/simObj.eventHandlerObj.eventIdSeed << endl;
    return 0;
}